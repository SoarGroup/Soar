/////////////////////////////////////////////////////////////////
// Socket class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : ~2001
//
// Represents a socket.
//
// Instances of this class are not created directly.
//
// A server creates a listener socket (a derived class)
// which is used to listen for incoming connections on a particular port.
//
// A client then connects to that listener socket (it needs to know the IP address
// and port to connect to) through the "client socket" class.
//
// The client continues to use the client socket object it created.
// The server is passed a new socket when it checks for incoming connections
// on the listener socket.
// 
/////////////////////////////////////////////////////////////////

#ifndef CT_SOCKET_H
#define CT_SOCKET_H

#include <string>

// Define the SOCKET type without pulling in "sock_SocketHeader.h" which would
// pull in all of the Windows headers...that's a lot of work for one typedef.
#ifdef _WIN32
typedef	unsigned int	SOCKET ;
#else
typedef int				SOCKET ;
#endif

#ifdef _WIN32
// Bring in the windows socket library.
// By doing this in the code like this, each client doesn't need to add the winsock library
// as an explicit dependency.  When it includes this header, it gets the winsock library linked in.
#pragma comment (lib, "ws2_32.lib")
#endif

namespace sock {

// Define NON_BLOCKING to make all sockets non-blocking sockets.
//#define NON_BLOCKING

// Useful utility functions.
unsigned long GetLocalIP() ;
char*		  GetLocalIPAddress() ;

#define NO_CONNECTION	0

// Report info on the error than just occurred.
void	ReportErrorCode() ;

class ListenerSocket ;
class ClientSocket ;

class Socket  
{
	// Allow these classes access to our constructor
	friend class ListenerSocket ;
	friend class ClientSocket ;

protected:
	SOCKET	m_hSocket ;

	// These fields are not required to establish a socket
	// connection, but are useful information about the socket
	// that the owner can use.
	bool	m_bIsEnabled ;
	short	m_Port ;			// Port the other side is listening on
	char	m_Name[256] ;		// Name the other side is using
	bool	m_bSentName ;		// Have we sent our name+port to them yet?

	// These objects are created through the ListenerSocket or ClientSocket classes.
protected:
	Socket();
	Socket(SOCKET hSocket) ;

public:
	// Destructor closes the socket
	virtual		~Socket();

	// Note: When we try to read/write to the socket next we may
	// find this is no longer true--it's just the last state we know about.
	bool		IsAlive()					{ return m_hSocket != NO_CONNECTION ; }

	// Returns handle for socket
	SOCKET		GetSocketHandle()			{ return m_hSocket ; }

	// Check if data is waiting to be read
	// Returns true if socket is closed--but then receiveMsg will know it's closed.
	bool		IsReadDataAvailable() ;

	// Close down our side of the socket
	void		CloseSocket() ;
		
public:
	// Send a string of characters.  Outgoing format will be "<4-byte length>"+string data
	bool		SendString(char const* pString) ;

	// Receive a string of characters.  Incoming format on socket should be "<4-byte length>"+string data
	bool		ReceiveString(std::string* pString) ;

protected:
	// Lower level buffer send and receive calls.
	bool		SendBuffer(char const* pSendBuffer, size_t bufferSize) ;
	bool		ReceiveBuffer(char* pRecvBuffer, size_t bufferSize) ;

protected:
	// I think these methods should all be removed, together with their members variables
	// but I'm going to wait for a little to see that they're definitely not needed first...
//	void		SetName(char const* pName)	{ SafeStrncpy(m_Name, pName, sizeof(m_Name)) ; }
//	char const* GetName() const				{ return m_Name ; }

	void		Enable(bool bEnable) 		{ m_bIsEnabled = bEnable ; }
	bool		IsEnabled()			 		{ return m_bIsEnabled ; }

	void		SetSentName(bool bSent)		{ m_bSentName = bSent ; }
	bool		IsNameSent()				{ return m_bSentName ; }

	void		SetPort(short port)  		{ m_Port = port ; }
	short		GetPort()			 		{ return m_Port ; }

	bool		IsNamed()					{ return (m_Port != 0 || m_Name[0] != '\0') ; }
};

} // Namespace

#endif // CT_SOCKET_H
