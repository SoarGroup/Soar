// Socket.h: interface for the CTSocket class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SOCKET_H__E6E162BD_90B4_4C63_BDC7_7C6A180152AD__INCLUDED_)
#define AFX_SOCKET_H__E6E162BD_90B4_4C63_BDC7_7C6A180152AD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SocketHeader.h"
#include "Msg.h"
#include "Utils.h"

#include <assert.h>

// Define NON_BLOCKING to make all sockets non-blocking sockets.
#define NON_BLOCKING

// Useful utility functions.
unsigned long GetLocalIP() ;
char*		  GetLocalIPAddress() ;

#define NO_CONNECTION	0

// Report info on the error than just occurred.
void	ReportErrorCode() ;

class CTSocket  
{
protected:
	SOCKET	m_hConnection ;

	// These fields are not required to establish a socket
	// connection, but are useful information about the socket
	// that the owner can use.
	bool	m_bIsEnabled ;
	short	m_Port ;			// Port the other side is listening on
	char	m_Name[256] ;		// Name the other side is using
	bool	m_bSentName ;		// Have we sent our name+port to them yet?

public:
	CTSocket();
	CTSocket(SOCKET hSocket) ;

	// Destructor closes the socket
	virtual		~CTSocket();

	// Access to member variables
	void		SetName(char const* pName)	{ SafeStrncpy(m_Name, pName, sizeof(m_Name)) ; }
	char const* GetName() const				{ return m_Name ; }

	void		Enable(bool bEnable) 		{ m_bIsEnabled = bEnable ; }
	bool		IsEnabled()			 		{ return m_bIsEnabled ; }

	void		SetSentName(bool bSent)		{ m_bSentName = bSent ; }
	bool		IsNameSent()				{ return m_bSentName ; }

	void		SetPort(short port)  		{ m_Port = port ; }
	short		GetPort()			 		{ return m_Port ; }

	bool		IsNamed()					{ return (m_Port != 0 || m_Name[0] != '\0') ; }

	// Note: When we try to read/write to the socket next we may
	// find this is no longer true--it's just the last state we know about.
	bool		IsAlive()					{ return m_hConnection != NO_CONNECTION ; }

	// Returns handle for socket
	SOCKET		GetSocketHandle()			{ return m_hConnection ; }

	// Send Msg, Receive Msg
	bool		SendMsg(CTMsg const& msg) ;
	bool		ReceiveMsg(CTMsg& msg) ;

	// Check if data is waiting to be read
	// Returns true if socket is closed--but then receiveMsg will know it's closed.
	bool		IsReadDataAvailable() ;

	// Close down our side of the socket
	void		CloseSocket() ;
		
protected:
	// Lower level buffer send and receive calls.
	bool		SendBuffer(char const* pSendBuffer, size_t bufferSize) ;
	bool		ReceiveBuffer(char* pRecvBuffer, size_t bufferSize) ;
};

#endif // !defined(AFX_SOCKET_H__E6E162BD_90B4_4C63_BDC7_7C6A180152AD__INCLUDED_)
