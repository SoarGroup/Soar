// ClientSocket.h: interface for the CTClientSocket class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CLIENTSOCKET_H__992AB033_8F59_4587_B150_E7F835464EB3__INCLUDED_)
#define AFX_CLIENTSOCKET_H__992AB033_8F59_4587_B150_E7F835464EB3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Socket.h"

class CTClientSocket : public CTSocket  
{
public:
	CTClientSocket();
	virtual ~CTClientSocket();

	// Connect to the server at specific address and port number
	bool	ConnectToServer(char const* netAddress, unsigned short port) ;
};

#endif // !defined(AFX_CLIENTSOCKET_H__992AB033_8F59_4587_B150_E7F835464EB3__INCLUDED_)
