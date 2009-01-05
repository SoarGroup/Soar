/////////////////////////////////////////////////////////////////
// ClientNamedPipe class
//
// Author: Bob Marinier
// Date  : 5/2007
//
// Based on ClientSocket.
//
// Creates a named pipe by connecting to a server at a known
// pipe name.
// 
/////////////////////////////////////////////////////////////////

#ifndef CLIENT_NAMED_PIPE_H
#define CLIENT_NAMED_PIPE_H

#include "sock_NamedPipe.h"

namespace sock {

class ClientNamedPipe : public NamedPipe
{
public:
	ClientNamedPipe();
	virtual ~ClientNamedPipe();

	/////////////////////////////////////////////////////////////////////
	// Function name  : ClientNamedPipe::ConnectToServer
	// 
	// Return type    : bool 	
	// Argument       : char* pPipeName
	// 
	// Description	  : Connect to a server
	//
	/////////////////////////////////////////////////////////////////////
	bool	ConnectToServer(char const* pipeName) ;
};

} // Namespace

#endif // CLIENT_NAMED_PIPE_H
