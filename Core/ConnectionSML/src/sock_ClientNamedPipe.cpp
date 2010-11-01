#include <portability.h>

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

#ifdef ENABLE_NAMED_PIPES

#include "sml_Utils.h"
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

	DWORD usernamesize = UNLEN+1;
	char username[UNLEN+1];
	GetUserName(username,&usernamesize);

	// Get the address
	std::string name = "\\\\.\\pipe\\";
	name.append(username);
	name.append("-");
	name.append(pPipeName);

	// set the name of this datasender
	this->name = "pipe ";
	this->name.append(name);

	HANDLE hPipe;

// silence warning about constant conditional expression
#pragma warning( push ) 
#pragma warning (disable : 4127)
	while(1) {
#pragma warning( pop ) 
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
		
		if (GetLastError() != ERROR_PIPE_BUSY)
		{
			sml::PrintDebug("Error: Error creating client connection pipe") ;
			sml::ReportSystemErrorMessage();
			return false ;
		}

		// All pipe instances are busy, so wait for 20 seconds. 
		if (!WaitNamedPipe(name.c_str(), 20000)) 
		{ 
			sml::PrintDebug("Error: Error opening client connection pipe") ;
			sml::ReportSystemErrorMessage();
			return false;
		}
	}

	// Record the sock so it's cleaned up correctly on exit
	m_hPipe = hPipe ;

	return true ;
}

#endif // ENABLE_NAMED_PIPES
