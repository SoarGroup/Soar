// ServerSocket.h: interface for the CTServerSocket class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SERVERSOCKET_H__F885641C_A9DF_4E1A_9C3B_75DCA7C6C124__INCLUDED_)
#define AFX_SERVERSOCKET_H__F885641C_A9DF_4E1A_9C3B_75DCA7C6C124__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Socket.h"

class CTServerSocket : public CTSocket  
{
public:
	CTServerSocket();
	virtual ~CTServerSocket();

	// Creates a listener socket -- used by the server to create connections
	bool	CreateListener(unsigned short port) ;

	// Check for an incoming client connection
	CTSocket* CheckForClientConnection() ;
};

#endif // !defined(AFX_SERVERSOCKET_H__F885641C_A9DF_4E1A_9C3B_75DCA7C6C124__INCLUDED_)
