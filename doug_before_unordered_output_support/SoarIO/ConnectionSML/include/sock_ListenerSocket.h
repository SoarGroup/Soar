/////////////////////////////////////////////////////////////////
// ListenerSocket class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : ~2001
//
// A server application creates a listener socket on a specific port.
// Clients then connect through this port to create a socket which is
// actually used to send data.
// 
/////////////////////////////////////////////////////////////////
#ifndef CT_LISTENER_SOCKET_H
#define CT_LISTENER_SOCKET_H

#include "sock_Socket.h"

namespace sock {

class ListenerSocket : public Socket  
{
public:
	ListenerSocket();
	virtual ~ListenerSocket();

	// Creates a listener socket -- used by the server to create connections
	bool	CreateListener(unsigned short port) ;

	// Check for an incoming client connection
	// This call does not block.  If there is no pending connection it returns NULL immediately.
	Socket* CheckForClientConnection() ;
};

} // Namespace

#endif // CT_LISTENER_SOCKET_H

