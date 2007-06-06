#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
//FIXME: #include <portability.h>

/////////////////////////////////////////////////////////////////
// ClientNamedPipe class
//
// Author: Bob Marinier
// Date  : 5/2007
//
// Based on ClientSocket
//
// Creates a named pipe by connecting to a server at a known
// pipe name.
// 
/////////////////////////////////////////////////////////////////

#ifdef _WIN32

#include "sock_Debug.h"
#include "sock_NamedPipeHeader.h"
#include "sock_ClientNamedPipe.h"
#include <assert.h>

using namespace sock ;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ClientNamedPipe::ClientNamedPipe()
{

}

ClientNamedPipe::~ClientNamedPipe()
{

}

/////////////////////////////////////////////////////////////////////
// Function name  : ClientNamedPipe::ConnectToServer
// 
// Return type    : bool 	
// Argument       : char* pPipeName
// 
// Description	  : Connect to a server
//
/////////////////////////////////////////////////////////////////////
bool ClientNamedPipe::ConnectToServer(char const* pPipeName)
{
	CTDEBUG_ENTER_METHOD("ClientNamedPipe::ConnectToServer");

	if (pPipeName == NULL) { pPipeName = "12121"; }

	// Get the address
	std::string name = "\\\\.\\pipe\\";
	name.append(pPipeName);

	HANDLE hPipe;

	while(1) {
		// Create the pipe
		hPipe = CreateFile( 
			name.c_str(),	// pipe name 
			GENERIC_READ |	// read and write access 
			GENERIC_WRITE, 
			0,				// no sharing 
			NULL,			// default security attributes
			OPEN_EXISTING,	// opens existing pipe 
			0,				// default attributes 
			NULL);			// no template file 

		if(hPipe != INVALID_HANDLE_VALUE) break;
		
		if (PIPE_ERROR_NUMBER != PIPE_BUSY)
		{
			PrintDebug("Error: Error creating client connection pipe") ;
			ReportErrorCode();
			return false ;
		}

		// All pipe instances are busy, so wait for 20 seconds. 
		if (!WaitNamedPipe(name.c_str(), 20000)) 
		{ 
			PrintDebug("Error: Error opening client connection pipe") ;
			ReportErrorCode();
			return false;
		}
	}

	// Record the sock so it's cleaned up correctly on exit
	m_hPipe = hPipe ;

	return true ;
}

#endif // _WIN32