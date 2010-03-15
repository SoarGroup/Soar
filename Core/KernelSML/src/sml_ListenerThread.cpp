#include <portability.h>

/////////////////////////////////////////////////////////////////
// ListenerThread class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : October 2004
//
// Listens for incoming remote connections and creates
// a new connection object for each one.
//
/////////////////////////////////////////////////////////////////

#include "sml_ListenerThread.h"

#include "sml_Utils.h"
#include "sml_ConnectionManager.h"
#include "sml_KernelSML.h"

#include <time.h>	// To get clock
#include <stdio.h> // To concat file name for socket file deletion

using namespace sml ;
using namespace sock ;

// From KernelSMLInterface (which handles embedded connections)
extern soarxml::ElementXML* ReceivedCall(Connection* pConnection, soarxml::ElementXML* pIncoming, void* pUserData) ;

void ListenerThread::Run()
{
	bool ok = m_ListenerSocket.CreateListener(m_Port) ;
	if (ok)
	{
		//sml::PrintDebugFormat("Internet listener socket: %s", m_ListenerSocket.GetName().c_str()) ;
	}
	else 
	{
		sml::PrintDebug("Failed to create the internet listener socket.  Shutting down listener thread.") ;
		return ;
	}

#ifdef ENABLE_LOCAL_SOCKETS
	ok = m_LocalListenerSocket.CreateListener(m_Port, true);
	if (ok)
	{
		//sml::PrintDebugFormat("Local listener socket: %s", m_LocalListenerSocket.GetName().c_str()) ;
	}
	else
	{
		sml::PrintDebug("Failed to create the local listener socket.  Shutting down thread.") ;
		m_ListenerSocket.Close();
		return ;
	}
#endif

#ifdef ENABLE_NAMED_PIPES
	ok = m_ListenerNamedPipe.CreateListener(m_Port) ;
	if (ok)
	{
		//sml::PrintDebugFormat("Local listener pipe: %s", m_ListenerNamedPipe.GetName().c_str()) ;
	}
	else
	{
		sml::PrintDebug("Failed to create the listener pipe.  Shutting down thread.") ;
		m_ListenerSocket.Close();
		return ;
	}
#endif

	m_Port = m_ListenerSocket.GetPort();

	while (!m_QuitNow)
	{
		//sml::PrintDebug("Check for incoming connection") ;

		// Check for an incoming client connection
		// This doesn't block.
		Socket* pSocket = m_ListenerSocket.CheckForClientConnection() ;

		Socket* pLocalSocket = 0;
#ifdef ENABLE_LOCAL_SOCKETS
		pLocalSocket = m_LocalListenerSocket.CheckForClientConnection();
#endif

#ifdef ENABLE_NAMED_PIPES
		NamedPipe* pNamedPipe = m_ListenerNamedPipe.CheckForClientConnection();
#endif
		if (pSocket) CreateConnection(pSocket);

		if (pLocalSocket) CreateConnection(pLocalSocket);
#ifdef ENABLE_NAMED_PIPES
		if (pNamedPipe) {
			CreateConnection(pNamedPipe);
			ok = m_ListenerNamedPipe.CreateListener(m_Port) ;

			if (!ok)
			{
				sml::PrintDebug("Failed to create the listener pipe.  Shutting down thread.") ;
				return ;
			}
		}
#endif

		// Sleep for a little before checking for a new connection
		// New connections will come in very infrequently so this doesn't
		// have to be very rapid.
		sml::Sleep(0, 50) ;
	}

	// Shut down our listener socket
	m_ListenerSocket.Close() ;
#ifdef ENABLE_LOCAL_SOCKETS
	m_LocalListenerSocket.Close();

// This code shouldn't be necessary since the we unlink existing files on start
// and since the file is in the user's home, permissions for that won't be an issue
/*
	char f_name[256];
	sprintf( f_name, "%s%d", sock::GetLocalSocketDir().c_str(), m_Port );
	sml::PrintDebugFormat( "Attempting to delete %s", f_name );
	int del_status = unlink( f_name );
	if ( del_status == 0 )
		sml::PrintDebug( "Delete succeeded!" );
	else
		sml::PrintDebugFormat( "Error occurred during delete attempt, error code %d", errno );
*/
#endif
#ifdef ENABLE_NAMED_PIPES
	m_ListenerNamedPipe.Close();
#endif
}

void ListenerThread::CreateConnection(DataSender* pSender)
{
	//sml::PrintDebugFormat("Got new connection on %s", pSender->GetName().c_str()) ;

	// Create a new connection object for this socket
	Connection* pConnection = Connection::CreateRemoteConnection(pSender) ;

	// Record our kernel object with this connection.  I think we only want one kernel
	// object even if there are many connections (because there's only one kernel) so for now
	// that's how things are set up.
	pConnection->SetUserData(m_pKernel) ;

	// For debugging record that this is on the kernel side
	pConnection->SetIsKernelSide(true) ;

	// Register for "calls" from the client.
	pConnection->RegisterCallback(ReceivedCall, NULL, sml_Names::kDocType_Call, true) ;

	// Set up our tracing state to match the user's preference
	pConnection->SetTraceCommunications(m_pKernel->IsTracingCommunications()) ;

	// Record the new connection in our list of connections
	m_pKernel->AddConnection(pConnection) ;
}
