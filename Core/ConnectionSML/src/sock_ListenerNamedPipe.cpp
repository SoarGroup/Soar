#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
//FIXME: #include <portability.h>

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
#include "sock_ListenerNamedPipe.h"
#include "sock_Debug.h"

using namespace sock ;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ListenerNamedPipe::ListenerNamedPipe()
{

}

ListenerNamedPipe::~ListenerNamedPipe()
{

}


/////////////////////////////////////////////////////////////////////
// Function name  : ListenerNamedPipe::CreateListener
// 
// Return type    : bool 	
// Argument       : const char* name	
// 
// Description	  : Create a non-blocking socket that listens
//					on a specific port.
//
/////////////////////////////////////////////////////////////////////
bool ListenerNamedPipe::CreateListener(const char* name)
{
	CTDEBUG_ENTER_METHOD("ListenerNamedPipe::CreateListener");

	/*// Should only call this once
	if (m_hPipe != INVALID_HANDLE_VALUE)
	{
		PrintDebug("Error: Already listening--closing the existing listener") ;

		Close();
	}*/

	// Create the listener socket
//FIXME: 
#ifdef _WIN32
	HANDLE hListener = CreateNamedPipe( 
          name,						// pipe name 
          PIPE_ACCESS_DUPLEX,		// read/write access 
          PIPE_TYPE_MESSAGE |		// message type pipe 
          PIPE_READMODE_MESSAGE |	// message-read mode 
          PIPE_NOWAIT,				// blocking mode 
          PIPE_UNLIMITED_INSTANCES,	// max. instances  
          4096,						// output buffer size 
          4096,						// input buffer size 
          NMPWAIT_USE_DEFAULT_WAIT,	// client time-out 
          NULL);					// default security attribute 
#else
	HANDLE hListener = 0;
#endif


	if (hListener == INVALID_HANDLE_VALUE)
	{
		PrintDebug("Error: Error creating the listener pipe") ;
		return false ;
	}

	// Record the listener pipe so we'll clean
	// up properly when the object is destroyed.
	m_hPipe = hListener ;

	return true ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : ListenerNamedPipe::CheckForClientConnection
// 
// Return type    : NamedPipe* 	
// 
// Description	  : This function creates a new
//					pipe (wrapped by NamedPipe) if there is a connection
//					by a client to the listener named pipe.
//
//					NULL is returned if there is no new connection.
//
/////////////////////////////////////////////////////////////////////
NamedPipe* ListenerNamedPipe::CheckForClientConnection()
{
	CTDEBUG_ENTER_METHOD("ListenerNamedPipe::CheckForClientConnection");

	// If this is a blocking socket make sure
	// a connection is available before attempting to accept.
/*#ifndef NON_BLOCKING
	if (!IsReadDataAvailable())
		return NULL ;
#endif*/

	//PrintDebug("About to check for a connection") ;
//FIXME:
#ifdef _WIN32
	int connected = ConnectNamedPipe(m_hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED); 
#else
	int connected = 0;
#endif
	// If we failed to find a valid socket we're done.
	if (!connected)
		return NULL ;

	PrintDebug("Received a connection") ;

	// Create a generic NamedPipe because once the connection has been
	// made all pipes are both servers and clients.  No need to distinguish.
	NamedPipe* pConnection = new NamedPipe(m_hPipe) ;

	return pConnection ;
}
