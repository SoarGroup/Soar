/////////////////////////////////////////////////////////////////
// ClientSocket class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : ~2001
//
// Creates a socket by connecting to a server at a known
// IP address and port number.
// 
/////////////////////////////////////////////////////////////////

#ifndef CT_CLIENT_SOCKET_H
#define CT_CLIENT_SOCKET_H

#include "sock_Socket.h"

namespace sock {

class ClientSocket : public Socket  
{
public:
	ClientSocket();
	virtual ~ClientSocket();

	/////////////////////////////////////////////////////////////////////
	// Function name  : CTClientSocket::ConnectToServer
	// 
	// Return type    : bool 	
	// Argument       : char* pNetAddress	// Can be NULL -- in which case connect to "this machine"
	// Argument       : int port	
	// 
	// Description	  : Connect to a server
	//
	/////////////////////////////////////////////////////////////////////
	bool	ConnectToServer(char const* netAddress, unsigned short port) ;
};

} // Namespace

#endif // CT_CLIENT_SOCKET_H
