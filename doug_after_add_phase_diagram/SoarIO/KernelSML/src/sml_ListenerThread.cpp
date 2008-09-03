#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

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

#include "sml_ConnectionManager.h"
#include "sml_ListenerThread.h"
#include "sml_KernelSML.h"
#include "sock_Debug.h"

#include <time.h>	// To get clock

using namespace sml ;
using namespace sock ;

// From KernelSMLInterface (which handles embedded connections)
extern ElementXML* ReceivedCall(Connection* pConnection, ElementXML* pIncoming, void* pUserData) ;

void ListenerThread::Run()
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

			// For debugging record that this is on the kernel side
			pConnection->SetIsKernelSide(true) ;

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
		Sleep(0, 50) ;
	}

	// Shut down our listener socket
	m_ListenerSocket.CloseSocket() ;
}
