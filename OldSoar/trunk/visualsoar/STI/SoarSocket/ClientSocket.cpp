// ClientSocket.cpp: implementation of the CTClientSocket class.
//
//////////////////////////////////////////////////////////////////////

#include "Debug.h"
#include "ClientSocket.h"
#include "OSspecific.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTClientSocket::CTClientSocket()
{

}

CTClientSocket::~CTClientSocket()
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
	if (address.s_addr != -1)
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
// Function name  : CTClientSocket::ConnectToServer
// 
// Return type    : bool 	
// Argument       : char* pNetAddress	
// Argument       : int port	
// 
// Description	  : Connect to a server
//
/////////////////////////////////////////////////////////////////////
bool CTClientSocket::ConnectToServer(char const* pNetAddress, unsigned short port)
{
	CTDEBUG_ENTER_METHOD("CTClientSocket::ConnectToServer");

	if (pNetAddress == NULL)
		pNetAddress = kLocalHost ;

	// Get the address
	in_addr* pAddress = ConvertAddress(pNetAddress) ;

	if (pAddress == NULL)
	{
		PrintDebug("Error: Unable to convert entered address to socket address") ;
		return false ;
	}

	// Specify the host address and port to connect to.
	sockaddr_in address ;
	memset(&address, 0, sizeof(address)) ;

	address.sin_family = AF_INET ;
	address.sin_port   = port ;
	address.sin_addr.s_addr = pAddress->s_addr ;

	// Create the socket
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0) ;

	if (sock == INVALID_SOCKET)
	{
		PrintDebug("Error: Error creating client connection socket") ;
		return false ;
	}

	// Record the sock so it's cleaned up correctly on exit
	m_hConnection = sock ;

	// Try to connect to the server
	int res = connect(sock, (sockaddr*)&address, sizeof(address)) ;

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
