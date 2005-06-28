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

#ifndef CONNECTION_MANAGER_H
#define CONNECTION_MANAGER_H

#include "thread_Thread.h"
#include "thread_Lock.h"
#include "sock_SocketLib.h"
#include "sml_Connection.h"

#include <list>

namespace sml {

// Forward declarations
class KernelSML ;
class ListenerThread ;
class ReceiverThread ;

class ConnectionManager
{
protected:
	// Initialize the socket library when we create this manager
	// which we do by creating a socket lib object.
	sock::SocketLib				m_SocketLib ;

	// The listener socket, wrapped in a thread
	ListenerThread*				m_ListenerThread ;

	// Used to check for incoming messages and execute them.
	ReceiverThread*				m_ReceiverThread ;

	// This mutex is used to control access to the m_Connections list
	soar_thread::Mutex			m_ConnectionsMutex ;

	// The list of connections.  One may be an embedded connection (part of same process).
	// Rest will be remote connections over a socket, wrapped in a thread.
	std::list< Connection* >		m_Connections ;
	std::list< Connection* >		m_ClosedConnections ;
	typedef std::list< Connection* >::iterator	ConnectionsIter ;

	// If true dump out details about messages sent and received over sockets
	// (and perhaps embedded connections too?)
	bool	m_bTraceCommunications ;

public:
	ConnectionManager(unsigned short port) ;
	~ConnectionManager() ;

	// Add a new incoming connection to our list
	void AddConnection(Connection* pConnection) ;

	// Remove a connection from our list
	void RemoveConnection(Connection* pConnection) ;

	// Get the i-th connection (it's ok to call with i >= number of connections as this returns NULL in that case)
	// So provides easy and thread safe way to get each connection in turn.
	Connection* GetConnectionByIndex(int index) ;

	// Set the agent status value for all existing connections
	void SetAgentStatus(char const* pStatus) ;

	/*************************************************************
	* @brief Turning this on means we'll start dumping output about messages
	*		 being sent and received.  Currently this only applies to remote connections.
	*************************************************************/
	void SetTraceCommunications(bool state) ;
	bool IsTracingCommunications() { return m_bTraceCommunications ; }

	// Go through all connections and read any incoming commands from the sockets.
	// The messages are sent to the callback registered with the connection when it was created (ReceivedCall currently).
	// Returning false indicates we should stop checking
	// for more messages (and presumably shutdown completely).
	bool ReceiveAllMessages() ;

	// Cause the receiver thread to quit.
	void StopReceiverThread() ;

	// Close all connections
	void Shutdown() ;
} ;

} // Namespace

#endif	// CONNECTION_MANAGER_H
