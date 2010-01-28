/////////////////////////////////////////////////////////////////
// SocketLib class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : ~2001
//
// Handles initilization and termination of the socket library for
// the appropriate platform.
//
// Need to do this before we can use the socket code.
// BADBAD: We should figure out how to initialize this automatically.
// 
/////////////////////////////////////////////////////////////////
#ifndef CT_SOCKET_LIB_H
#define CT_SOCKET_LIB_H

namespace sock {

class SocketLib  
{
public:
	SocketLib();
	virtual ~SocketLib();
};

} // Namespace

#endif // CT_SOCKET_LIB_H

