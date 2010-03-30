#include <portability.h>

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
#include "sml_Utils.h"
#include "sock_ClientSocket.h"
#include "sock_OSspecific.h"

#include <sstream>
#include <stdio.h>
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
bool ClientSocket::ConnectToServer(char const* pNetAddress, int port)
{
	CTDEBUG_ENTER_METHOD("ClientSocket::ConnectToServer");

	SOCKET sock;

	int res = 1; // if any of this fails, fall back on creating an internet socket

#ifdef ENABLE_LOCAL_SOCKETS

	sockaddr_un local_address;

	if(!pNetAddress) {
		memset(&local_address, 0, sizeof(local_address));
		local_address.sun_family = AF_UNIX;
		sprintf(local_address.sun_path, "%s%d", sock::GetLocalSocketDir().c_str(), port);

		// set the name of the datasender
		this->name = "file ";
		this->name.append(local_address.sun_path);

		int len = SUN_LEN(&local_address);

		// Create the socket
		sock = socket(AF_UNIX, SOCK_STREAM, 0) ;

		res = 1; 
		if (sock == INVALID_SOCKET)
		{
			sml::PrintDebug("Error: Error creating client local connection socket") ;
		} 
		else
		{
			if(chmod(local_address.sun_path, S_IRWXU) < 0)
			{
				sml::PrintDebug("Error: Error setting permissions for client local connection socket") ;
			}
			else
			{
				// Try to connect to the server
				res = connect(sock, (sockaddr*)&local_address, len) ;
			}
		}

	} 
#endif
	if (res != 0) 
	{
		in_addr* pAddress = NULL;

		if (pNetAddress == NULL)
			pNetAddress = kLocalHost ;

		// Get the address
		pAddress = ConvertAddress(pNetAddress) ;

		if (pAddress == NULL)
		{
			sml::PrintDebug("Error: Unable to convert entered address to socket address") ;
			return false ;
		}

		// Specify the host address and port to connect to.
		sockaddr_in address ;

		// set the name of the datasender
		std::stringstream name;
		name << "port " << port;
		this->name = name.str();

		memset(&address, 0, sizeof(address)) ;

		address.sin_family = AF_INET ;
		address.sin_port   = htons(static_cast<unsigned short>(port)) ;
		address.sin_addr.s_addr = pAddress->s_addr ;

		// Create the socket
		sock = socket(AF_INET, SOCK_STREAM, 0) ;

		if (sock == INVALID_SOCKET)
		{
			sml::PrintDebug("Error: Error creating client connection socket") ;
			return false ;
		}

		// Try to connect to the server
		res = connect(sock, (sockaddr*)&address, sizeof(address)) ;
	}

	// Record the sock so it's cleaned up correctly on exit
	m_hSocket = sock ;

	if (res != 0)
	{
		sml::PrintDebug("Unable to connect to server") ;
		return false ;
	}

#ifdef NON_BLOCKING
	// Make the socket a non-blocking socket
	bool ok = MakeSocketNonBlocking(sock) ;

	if (!ok)
	{
		sml::PrintDebug("Error: Error setting the connection socket to be non-blocking") ;
		return false ;
	}
#endif

	return true ;
}
