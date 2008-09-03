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
#include "sml_KernelSML.h"
#include "sock_Debug.h"

#include <time.h>	// To get clock

using namespace sml ;
using namespace sock ;

// From KernelSMLInterface (which handles embedded connections)
extern ElementXML* ReceivedCall(Connection* pConnection, ElementXML* pIncoming, void* pUserData) ;

void ListenerManager::Run()
{
	// Create the listener
	PrintDebugFormat("Listening on port %d", m_Port) ;
	bool ok = m_ListenerSocket.CreateListener(m_Port) ;
	
	if (!ok)
	{
		PrintDebug("Failed to create the listener socket.  Shutting down thread.") ;
		return ;
	}

	while (!m_QuitNow)
	{
		//PrintDebug("Check for incoming connection") ;

		// Check for an incoming client connection
		// This doesn't block.
		Socket* pSocket = m_ListenerSocket.CheckForClientConnection() ;

		if (pSocket)
		{
			PrintDebug("Got new connection") ;

			// Create a new connection object for this socket
			Connection* pConnection = Connection::CreateRemoteConnection(pSocket) ;

			// Record our kernel object with this connection.  I think we only want one kernel
			// object even if there are many connections (because there's only one kernel) so for now
			// that's how things are set up.
			pConnection->SetUserData(KernelSML::GetKernelSML()) ;

			// Register for "calls" from the client.
			pConnection->RegisterCallback(ReceivedCall, NULL, sml_Names::kDocType_Call, true) ;

			// Set up our tracing state to match the user's preference
			pConnection->SetTraceCommunications(KernelSML::GetKernelSML()->IsTracingCommunications()) ;

			// Record the new connection in our list of connections
			m_Parent->AddConnection(pConnection) ;
		}

		// Sleep for a little before checking for a new connection
		// New connections will come in very infrequently so this doesn't
		// have to be very rapid.
		Sleep(50) ;
	}

	// Shut down our listener socket
	m_ListenerSocket.CloseSocket() ;
}

void ReceiverThread::Run()
{
	// While this thread is alive, keep checking for incoming messages
	// and executing them.  These messages could be coming in from a remote socket
	// or from an embedded connection.  If we're executing on the client thread
	// for an embedded connection (the "sycnhronous" model) then this thread is shut down
	// and it's the client's responsibility to check for incoming messages by calling
	// "GetIncomingCommands" periodically.

	// Record the time of the last message we received and keep polling
	// for a little after at a high speed, before sleeping.
	// The reason we track this is because calling "Sleep(0)" gives us maximum
	// performance (on towers-of-hanoi calling Sleep(1) instead slows execution down
	// by a factor of 80!), but calling "Sleep(0)" means we're stuck in a tight loop checking
	// messages continually.  If we're using Soar through a command line interface (for instance)
	// then the CPU would be 100% loaded all of the time, even if the user wasn't typing anything.
	// This method of switching between Sleep(0) and a true sleep means we get maximum performance
	// during a run (when messages are flying back and forth) but when the user stops doing anything
	// the CPU quickly drops off to 0% usage as we sleep a lot.
	clock_t last = 0 ;

	// How long to wait before sleeping (currently 1 sec)
	clock_t delay = CLOCKS_PER_SEC ;

//	clock_t report = 0 ;

	while (!m_QuitNow)
	{
#ifdef _DEBUG
		/*
		// Every so often report that we're alive
		if (clock() - report > CLOCKS_PER_SEC * 3)
		{
			report = clock() ;
			PrintDebugFormat("ReceiverThread::Run alive\n") ;
		}
		*/
#endif
		// Receive any incoming commands and execute them
		bool receivedMessage = m_ConnectionManager->ReceiveAllMessages() ;

		if (receivedMessage)
		{
			// Record the time of the last incoming message
			last = clock() ;
		}

		clock_t current = clock() ;

		// If it's been a while since the last incoming message
		// then start sleeping a reasonable amount.
		// Sleep(0) just allows other threads to run before we continue
		// to execute.
		if (current - last > delay)
			Sleep(5) ;
		else
			Sleep(0) ;
	}
}

ConnectionManager::ConnectionManager(unsigned short port)
{
	// Start the thread that wraps the listener socket running.
	m_ListenerManager = new ListenerManager(this, port) ;
	m_ListenerManager->Start() ;

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
	if (m_ListenerManager)
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
	if (m_ListenerManager)
	{
		m_ListenerManager->Stop(true) ;

		// Remove the listener
		delete m_ListenerManager ;
		m_ListenerManager = NULL ;
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

		delete pConnection ;
	}

	m_Connections.clear() ;

//	PrintDebug("Completed shutdown") ;
}

void ConnectionManager::AddConnection(Connection* pConnection)
{
	// Serialize thread access to the connections list
	soar_thread::Lock lock(&m_ConnectionsMutex) ;

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

			delete pConnection ;

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
