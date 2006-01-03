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
#else // _WIN32
#ifndef SOCKET	 // voigtjr: SOCKET seems to be already defined on linux
typedef int				SOCKET ;
#endif  // SOCKET
#endif // _WIN32

#ifdef _WIN32
// Bring in the windows socket library.
// By doing this in the code like this, each client doesn't need to add the winsock library
// as an explicit dependency.  When it includes this header, it gets the winsock library linked in.
#pragma comment (lib, "ws2_32.lib")
#endif

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif 

#include <sys/types.h>

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

	// Controls whether we dump out the messages we're sending and receiving.
	bool m_bTraceCommunications ;

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
	// The timeout for waiting for data is secondsWait + millisecondsWait, where millisecondsWait < 1000
	bool		IsReadDataAvailable(long secondsWait = 0, long millisecondsWait = 0) ;

	// Close down our side of the socket
	void		CloseSocket() ;
		
public:
	// Print out debug information about the messages we are sending and receiving.
	// NOTE: We still print out information about start up/shut down, errors etc. without this flag being true.
	void		SetTraceCommunications(bool state) { m_bTraceCommunications = state ; }

	// Send a string of characters.  Outgoing format will be "<4-byte length>"+string data
	bool		SendString(char const* pString) ;

	// Receive a string of characters.  Incoming format on socket should be "<4-byte length>"+string data
	bool		ReceiveString(std::string* pString) ;

protected:
	// Lower level buffer send and receive calls.
	bool		SendBuffer(char const* pSendBuffer, size_t bufferSize) ;
	bool		ReceiveBuffer(char* pRecvBuffer, size_t bufferSize) ;

};

} // Namespace

#endif // CT_SOCKET_H
