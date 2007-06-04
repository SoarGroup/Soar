#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
//FIXME: #include <portability.h>

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
#include "sock_Debug.h"
#include "sock_ClientSocket.h"
#include "sock_OSspecific.h"
#include <assert.h>

using namespace sock ;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ClientSocket::ClientSocket()
{

}

ClientSocket::~ClientSocket()
{

}

/////////////////////////////////////////////////////////////////////
// Function name  : ConvertAddress
// 
// Return type    : static in_addr* 	
// Argument       : char* pNetAddress	
// 
// Description	  : Converts from ip address or hostname to
//					socket address data.
//
//					NOTE: Uses static structures so only valid
//					until next call.  (This is gethostbyname's approach
//					so we have just extended it).
//
/////////////////////////////////////////////////////////////////////
static in_addr* ConvertAddress(char const* pNetAddress)
{
	static in_addr address ;

	assert(pNetAddress) ;

	// Try it as aaa.bbb.ccc.ddd first
	address.s_addr = inet_addr(pNetAddress) ;

	// Check if this method worked
	if (address.s_addr != INADDR_NONE)
		return &address ;

	// Then try it as a hostname
	hostent* pHost = gethostbyname(pNetAddress) ;

	// Check if this method worked
	if (pHost != NULL)
	{
		return (in_addr*) *pHost->h_addr_list ;
	}

	return NULL ;
}

const char* kLocalHost = "127.0.0.1" ; // Special IP address meaning "this machine"

/////////////////////////////////////////////////////////////////////
// Function name  : ClientSocket::ConnectToServer
// 
// Return type    : bool 	
// Argument       : char* pNetAddress	// Can be NULL -- in which case connect to "this machine"
// Argument       : int port	
// 
// Description	  : Connect to a server
//
/////////////////////////////////////////////////////////////////////
bool ClientSocket::ConnectToServer(char const* pNetAddress, unsigned short port)
{
	CTDEBUG_ENTER_METHOD("ClientSocket::ConnectToServer");

#ifdef _WIN32
	if (pNetAddress == NULL)
		pNetAddress = kLocalHost ;
#endif

	in_addr* pAddress = NULL;

	if(pNetAddress) {
		// Get the address
		pAddress = ConvertAddress(pNetAddress) ;

		if (pAddress == NULL)
		{
			PrintDebug("Error: Unable to convert entered address to socket address") ;
			return false ;
		}
	}

	// Specify the host address and port to connect to.
	sockaddr_in address ;
	SOCKET sock;

	int res;

#ifndef _WIN32

	sockaddr_un local_address;

	if(!pNetAddress) {
		memset(&local_address, 0, sizeof(local_address));
		local_address.sun_family = AF_UNIX;
		sprintf(local_address.sun_path, "%s%u", "/var/tmp/", port);
		int len = SUN_LEN(&local_address);

		//BADBAD? Check if in use?
		unlink(local_address.sun_path); // in case it already exists

		// Create the socket
		sock = socket(AF_UNIX, SOCK_STREAM, 0) ;

		if (sock == INVALID_SOCKET)
		{
			PrintDebug("Error: Error creating client local connection socket") ;
			return false ;
		}

		/*if(chmod(local_address.sun_path, "S_IRWXU") < 0)
		{
			PrintDebug("Error: Error setting permissions for client local connection socket") ;
			return false ;
		}*/

		// Try to connect to the server
		res = connect(sock, (sockaddr*)&local_address, len)) ;

	} else 
#endif
	{
		memset(&address, 0, sizeof(address)) ;

		address.sin_family = AF_INET ;
		address.sin_port   = htons(port) ;
		address.sin_addr.s_addr = pAddress->s_addr ;

		// Create the socket
		sock = socket(AF_INET, SOCK_STREAM, 0) ;

		if (sock == INVALID_SOCKET)
		{
			PrintDebug("Error: Error creating client connection socket") ;
			return false ;
		}

		// Try to connect to the server
		res = connect(sock, (sockaddr*)&address, sizeof(address)) ;
	}

	// Record the sock so it's cleaned up correctly on exit
	m_hSocket = sock ;

	if (res != 0)
	{
		PrintDebug("Unable to connect to server") ;
		return false ;
	}

#ifdef NON_BLOCKING
	// Make the socket a non-blocking socket
	bool ok = MakeSocketNonBlocking(sock) ;

	if (!ok)
	{
		PrintDebug("Error: Error setting the connection socket to be non-blocking") ;
		return false ;
	}
#endif

	return true ;
}
