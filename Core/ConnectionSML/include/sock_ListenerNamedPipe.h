/////////////////////////////////////////////////////////////////
// ListenerNamedPipe class
//
// Author: Bob Marinier
// Date  : 5/2007
//
// Based on ListenerSocket class
//
// A server application creates a listener pipe with a specific name.
// Clients then connect through this name to create a pipe which is
// actually used to send data.
// 
/////////////////////////////////////////////////////////////////
#ifndef LISTENER_NAMED_PIPE_H
#define LISTENER_NAMED_PIPE_H

#include "sock_NamedPipe.h"

namespace sock {

class ListenerNamedPipe : public NamedPipe  
{
public:
	ListenerNamedPipe();
	virtual ~ListenerNamedPipe();

	// Creates a listener socket -- used by the server to create connections
	bool	CreateListener(const char* name) ;

	// Check for an incoming client connection
	// This call does not block.  If there is no pending connection it returns NULL immediately.
	NamedPipe* CheckForClientConnection() ;
};

} // Namespace

#endif // LISTENER_NAMED_PIPE_H

