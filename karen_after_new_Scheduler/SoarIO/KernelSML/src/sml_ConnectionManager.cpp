#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

/////////////////////////////////////////////////////////////////
// ConnectionManager class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : October 2004
//
// Used by a server (the kernel in our case usually) to manager
// a listener socket and then keep a list of connections that
// come in over that listener.
//
/////////////////////////////////////////////////////////////////

#include "sml_ConnectionManager.h"
#include "sml_ListenerThread.h"
#include "sml_ReceiverThread.h"
#include "sml_KernelSML.h"
#include "sock_Debug.h"

#include <time.h>	// To get clock

using namespace sml ;
using namespace sock ;

ConnectionManager::ConnectionManager(unsigned short port)
{
	// Start the thread that wraps the listener socket running.
	// (Unless passed port 0 -- which suppresses this)
	m_ListenerThread = NULL ;
	if (port != 0)
	{
		m_ListenerThread = new ListenerThread(this, port) ;
		m_ListenerThread->Start() ;
	}

	// Start the thread that checks for incoming messages on the remote connections
	// (once the listener has been used to create them).
	m_ReceiverThread = new ReceiverThread(this) ;
	m_ReceiverThread->Start() ;

	m_bTraceCommunications = false ;
}
ConnectionManager::~ConnectionManager()
{
	// If we still have an active listener, shut down the connections.
	// This isn't recommended.  It seems if we do this when the
	// main process is executing, other threads may already have been forced to exit
	// so this shutdown won't be as clean.  Nothing should crash, but we could cause
	// timeouts etc.
	if (m_ListenerThread)
		Shutdown() ;
}

// Cause the receiver thread to quit.
void ConnectionManager::StopReceiverThread()
{
	// Stop the receiver thread (and wait until is has stopped)
	if (m_ReceiverThread)
		m_ReceiverThread->Stop(true) ;
}

/*************************************************************
* @brief Turning this on means we'll start dumping output about messages
*		 being sent and received.  Currently this only applies to remote connections.
*************************************************************/
void ConnectionManager::SetTraceCommunications(bool state)
{
	m_bTraceCommunications = state ;

	// Serialize thread access to the connections list
	soar_thread::Lock lock(&m_ConnectionsMutex) ;

	// Set the trace state on any existing connections
	// (this list will usually be empty when we set this variable, but this is for completeness).
	for (ConnectionsIter iter = m_Connections.begin() ; iter != m_Connections.end() ; iter++)
	{
		Connection* pConnection = *iter ;
		pConnection->SetTraceCommunications(state) ;
	}
}

// Close all connections
void ConnectionManager::Shutdown()
{
//	PrintDebug("Shutting down connection manager") ;

	// Stop the thread (and wait until it has stopped)
	if (m_ListenerThread)
	{
		m_ListenerThread->Stop(true) ;

		// Remove the listener
		delete m_ListenerThread ;
		m_ListenerThread = NULL ;
	}

//	PrintDebug("Listener stopped") ;

	// Stop the receiver thread (and wait until is has stopped)
	if (m_ReceiverThread)
	{
		m_ReceiverThread->Stop(true) ;

	//	PrintDebug("Receiver stopped") ;

		// Remove the thread
		delete m_ReceiverThread ;
		m_ReceiverThread = NULL ;
	}

	// Serialize thread access to the connections list
	soar_thread::Lock lock(&m_ConnectionsMutex) ;

	for (ConnectionsIter iter = m_Connections.begin() ; iter != m_Connections.end() ; iter++)
	{
		Connection* pConnection = *iter ;
		pConnection->CloseConnection() ;

		// Remove any events that this connection is listening to
		KernelSML* pKernelSML = (KernelSML*)pConnection->GetUserData() ;
		pKernelSML->RemoveAllListeners(pConnection) ;

		// Not clear that we can just delete connections as we might be inside a connection callback
		// when the shutdown comes.  So for safety, move connections to a closed list instead of deleting.
		// For now this will leak the connection object.  Later if we are confident we can delete this list.
		//delete pConnection ;
		m_ClosedConnections.push_back(pConnection) ;
	}

	m_Connections.clear() ;

	// Now clean up all closed connections.
	for (ConnectionsIter iter = m_ClosedConnections.begin() ; iter != m_ClosedConnections.end() ; iter++)
	{
		Connection* pConnection = *iter ;
		delete pConnection ;
	}
	m_ClosedConnections.clear() ;

//	PrintDebug("Completed shutdown") ;
}

void ConnectionManager::AddConnection(Connection* pConnection)
{
	// Serialize thread access to the connections list
	soar_thread::Lock lock(&m_ConnectionsMutex) ;

	// Assign a unique ID for this connection
	std::ostringstream buffer;
	buffer << "id_0x" << pConnection;
	std::string bufferStdString = buffer.str();
	const char* bufferCString = bufferStdString.c_str();
	pConnection->SetID(bufferCString) ;
	pConnection->SetName("unknown") ;
	pConnection->SetStatus(sml_Names::kStatusCreated) ;

	m_Connections.push_back(pConnection) ;
}

Connection* ConnectionManager::GetConnectionByIndex(int i)
{
	// Just to be consistent with allowing out of range values to be passed in here.
	if (i < 0)
		return NULL ;

	// Serialize thread access to the connections list
	// which is shared with the listener socket/thread
	soar_thread::Lock lock(&m_ConnectionsMutex) ;

	// Get the n'th connection
	ConnectionsIter iter;
	for (iter = m_Connections.begin() ; iter != m_Connections.end() && i > 0 ; iter++)
	{
		i-- ;
	}

	if (iter == m_Connections.end())
		return NULL ;

	return *iter ;
}

void ConnectionManager::RemoveConnection(Connection* pConnection)
{
	// Serialize thread access to the connections list
	// which is shared with the listener socket/thread
	soar_thread::Lock lock(&m_ConnectionsMutex) ;

	// Remove the connection from our list
	m_Connections.remove(pConnection) ;
}

void ConnectionManager::SetAgentStatus(char const* pStatus)
{
	int index = 0 ;
	Connection* pConnection = NULL ;

	while ( (pConnection = GetConnectionByIndex(index)) != NULL)
	{
		pConnection->SetAgentStatus(pStatus) ;
		index++ ;
	}
}

// Go through all connections and read any incoming commands from the sockets.
// The messages are sent to the callback registered with the connection when it was created (ReceivedCall currently).
// Those calls could take a long time to execute (e.g. a call to Run Soar).
// Returns true if we received at least one message.
bool ConnectionManager::ReceiveAllMessages()
{
	int index = 0 ;
	bool receivedOneMessage = false ;

	// We need to search this list of connections and call each in turn.
	// But we also want to allow the listener thread to add new connections while we're doing this.
	// (E.g. we might want to attach a debugger to a process that's already executing a "run" command inside "receiveMessages").
	// So we use this slightly cumbersome approach of looking up the connection based on an integer index.
	// The lookup is thread safe and if the connection list changes between lookups that's fine as this function
	// will return NULL once we go out of bounds.  (You could argue that we might miss calling a connection on a particular pass using this method
	// but that should be ok).
	Connection* pConnection = GetConnectionByIndex(index++) ;

	while (pConnection)
	{
		// Check to see if this connection has already been closed
		// (which includes if the other side has dropped its half of the socket)
		if (!pConnection->IsClosed())
		{
			receivedOneMessage = pConnection->ReceiveMessages(true) || receivedOneMessage ;
		}
		else
		{
			// If the connection has closed, delete it from the list of active connections
			RemoveConnection(pConnection) ;
			
			// Remove any events that this connection is listening to
			KernelSML* pKernelSML = (KernelSML*)pConnection->GetUserData() ;
			pKernelSML->RemoveAllListeners(pConnection) ;

			// Not clear that we can just delete connections as we might be inside a connection callback
			// when the shutdown comes.  So for safety, move connections to a closed list instead of deleting.
			// For now this will leak the connection object.  Later if we are confident we can delete this list.
			//delete pConnection ;
			m_ClosedConnections.push_back(pConnection) ;

			// Since we just removed this connection we should look up the same index value again
			// so we don't skip any.  This isn't really that important.
			index-- ;
		}

		// Get the next connection (will return NULL once we reach the end of the list)
		pConnection = GetConnectionByIndex(index++) ;
	}

	// So far we don't have some sort of shutdown message the remote connections
	// can send, but if we decide to implement it this allows us to return it.
	return receivedOneMessage ;
}
