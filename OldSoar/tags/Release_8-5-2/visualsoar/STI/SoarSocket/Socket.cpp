// Socket.cpp: implementation of the CTSocket class.
//
//////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include "Socket.h"
#include "Check.h"
#include "Debug.h"
#include "OSspecific.h"	// For sleep

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTSocket::CTSocket()
{
	m_hConnection = NO_CONNECTION ;
	m_Name[0] = '\0' ;
	m_Port    = 0 ;
	m_bSentName = false ;
	m_bIsEnabled = true ;
}

CTSocket::CTSocket(SOCKET hSocket)
{
	m_hConnection = hSocket ;
	m_Name[0] = '\0' ;
	m_Port    = 0 ;
	m_bSentName = false ;
	m_bIsEnabled = true ;
}

CTSocket::~CTSocket()
{
	CloseSocket();
}

/////////////////////////////////////////////////////////////////////
// Function name  : GetLocalIPAddress
// 
// Return type    : char* 	
// 
// Description	  : Get the IP address as a string "aaaa.bbbb.cccc.dddd"
//
/////////////////////////////////////////////////////////////////////
char* GetLocalIPAddress()
{
	// Look up the local host's IP address
	unsigned long hostID = GetLocalIP() ;
	
	in_addr addr ;
	addr.s_addr = hostID ;

	// Convert to the string form of the IP address
	char* pHost = inet_ntoa(addr) ;

	return pHost ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : GetLocalIP
// 
// Return type    : unsigned long 	
// 
// Description	  : Function taken from the net.
//					Uses two approaches to get local IP address:
//					1) Use gethostname and then gethostbyname
//					2) Create UDP socket and call getsockname
//
/////////////////////////////////////////////////////////////////////
unsigned long GetLocalIP()
{
   char szLclHost[1024];
   HOSTENT* lpstHostent;
   SOCKADDR_IN stLclAddr;
   SOCKADDR_IN stRmtAddr;
   int nAddrSize = sizeof(SOCKADDR);
   SOCKET hSock;
   int nRet;
   
   /* Init local address (to zero) */
   stLclAddr.sin_addr.s_addr = INADDR_ANY;
   
   /* Get the local hostname */
   nRet = gethostname(szLclHost, sizeof(szLclHost));
   if (nRet != SOCKET_ERROR)
      {
      /* Resolve hostname for local address */
      lpstHostent = gethostbyname(szLclHost);
      if (lpstHostent)
         stLclAddr.sin_addr.s_addr = *((u_long*)
(lpstHostent->h_addr));
      }
   
   /* If still not resolved, then try second strategy */
   if (stLclAddr.sin_addr.s_addr == INADDR_ANY)
      {
      /* Get a UDP socket */
      hSock = socket(AF_INET, SOCK_DGRAM, 0);
      if (hSock != INVALID_SOCKET)
         {
         /* Connect to arbitrary port and address (NOT loopback) */
         stRmtAddr.sin_family = AF_INET;
         stRmtAddr.sin_port   = htons(IPPORT_ECHO);
         stRmtAddr.sin_addr.s_addr = inet_addr("128.127.50.1");
         nRet = connect(hSock, (SOCKADDR*)&stRmtAddr,
sizeof(SOCKADDR));
         if (nRet != SOCKET_ERROR)
            {
            /* Get local address */
#ifdef _WIN32
			// The Windows call takes a signed int, the Linux an unsigned int for nAddrSize.
            getsockname(hSock, (SOCKADDR*)&stLclAddr, &nAddrSize);
#else
			unsigned int addrSize ;
            getsockname(hSock, (SOCKADDR*)&stLclAddr, &addrSize);
			nAddrSize = addrSize ;
#endif
            }
         NET_CLOSESOCKET(hSock);   /* we're done with the socket */
         }
      }
   return stLclAddr.sin_addr.s_addr;
}


/////////////////////////////////////////////////////////////////////
// Function name  : IsErrorWouldBlock
// 
// Return type    : static bool 	
// 
// Description	  : Returns true if the error from the socket
//					is that making the call would cause it to block.
//
/////////////////////////////////////////////////////////////////////
static bool IsErrorWouldBlock()
{
	int error = NET_ERROR_NUMBER ;

	return (error == NET_EWOULDBLOCK) ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : CTSocket::SendMsg
// 
// Return type    : bool 				// true if send succeeded
// Argument       : CTMsg& msg	
// 
// Description	  : Send a message to the socket.
//
/////////////////////////////////////////////////////////////////////
bool CTSocket::SendMsg(CTMsg const& msg)
{
	CTDEBUG_ENTER_METHOD("CTSocket::SendMsg");

	// Check that we are packing the data as we expect.
	assert(sizeof(MsgStruct) == kMsg1Size) ;

	// BUGBUG: If this data can be between two different machines
	// (e.g. PC and Linux box) then we need to convert the data to
	// network layer format using htonl, ntohl etc.

	MsgStruct const* pMsgStruct = &msg.GetReadStruct() ;

	// Make sure the total size is filled in correctly.
	CHECK_RET_FALSE(pMsgStruct->m_TotalMessageSize == pMsgStruct->m_StructSize + pMsgStruct->m_DataSize) ;

	// Send the message
	bool ok = SendBuffer((char const*)pMsgStruct, sizeof(MsgStruct)) ;

	// Send the data section as well (if present)
	if (ok && msg.GetData() && msg.GetDataSize() > 0)
	{
		PrintDebugFormat("Sending extra data buffer of size %d",msg.GetDataSize()) ;

		ok = ok && SendBuffer(msg.GetData(), msg.GetDataSize()) ;
	}

	return ok ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : CTSocket::SendBuffer
// 
// Return type    : bool 	
// Argument       : char* pSendBuffer	
// Argument       : size_t bufferSize	
// 
// Description	  : Send a buffer of data to a socket.
//					This may require repeated calls to the low level "send" call.
//
/////////////////////////////////////////////////////////////////////
bool CTSocket::SendBuffer(char const* pSendBuffer, size_t bufferSize)
{
	CTDEBUG_ENTER_METHOD("CTSocket::SendBuffer");

	CHECK_RET_FALSE(pSendBuffer && bufferSize > 0) ;

	SOCKET hSock = m_hConnection ;

	if (!hSock)
	{
		PrintDebug("Error: Can't send because this socket is closed") ;
		return false; 
	}

	size_t bytesSent = 0 ;
	int    thisSend = 0 ;

	// May need repeated calls to send all of the data.
	while (bytesSent < bufferSize)
	{
		long tries = 0 ;

		do
		{
			tries++ ;
			thisSend = send(hSock, pSendBuffer, bufferSize - bytesSent, 0) ;

			// Check if there was an error
			if (thisSend == SOCKET_ERROR)
			{
#ifdef NON_BLOCKING
				// On a non-blocking socket, the socket can return "would block" -- in which case
				// we need to wait for it to clear.  A blocking socket would not return in this case
				// so this would always be an error.
				if (IsErrorWouldBlock())
				{
					PrintDebug("Waiting for socket to unblock") ;
					SleepMillisecs(100) ;
				}
				else
#endif
				{
					PrintDebug("Error: Error sending message") ;
					ReportErrorCode() ;
					return false ;
				}
			}
		} while (thisSend == SOCKET_ERROR) ;

		PrintDebugFormat("Sent %d bytes",thisSend) ;

		bytesSent   += thisSend ;
		pSendBuffer += thisSend ;
	}

	return true ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : CTSocket::IsReadDataAvailable
// 
// Return type    : bool 	
// 
// Description	  : Returns true if data is waiting to be read on this socket.
//					Also returns true if the socket is closed.
//					In that case the next read will return 0 bytes w/o an error
//					indicating that the socket is closed.
//
/////////////////////////////////////////////////////////////////////
bool CTSocket::IsReadDataAvailable()
{
	CTDEBUG_ENTER_METHOD("CTSocket::IsReadDataAvailable");

	SOCKET hSock = m_hConnection ;

	if (!hSock)
	{
		PrintDebug("Error: Can't check for read data because this socket is closed") ;
		return false;
	}

	fd_set set ;
	FD_ZERO(&set) ;

	#ifdef _MSC_VER
	#pragma warning(push, 3)
	#endif

	// Add hSock to the set of descriptors to check
	// This generates a warning on level 4 in VC++ 6.
	FD_SET(hSock, &set) ;

	#ifdef _MSC_VER
	#pragma warning(pop)
	#endif

	// Don't wait--just poll
	TIMEVAL zero ;
	zero.tv_sec = 0 ;
	zero.tv_usec = 0 ;

	// Check if anything is waiting to be read
	int res = select(hSock + 1, &set, NULL, NULL, &zero) ;

	// Did an error occur?
	if (res == SOCKET_ERROR)
	{
		PrintDebug("Error: Error checking if data is available to be read") ;
		ReportErrorCode() ;
		return false ;
	}

	bool bIsSet = FD_ISSET(hSock, &set) ? true : false ;
/*
	if (bIsSet)
		PrintDebug("Read data IS available") ;
	else
		PrintDebug("Read data is not available") ;
*/
	return bIsSet ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : CTSocket::ReceiveMsg
// 
// Return type    : bool 	
// Argument       : SOCKET hSock	
// Argument       : CTMsg& msg	
// 
// Description	  : Get a message if one is waiting.
//
/////////////////////////////////////////////////////////////////////
bool CTSocket::ReceiveMsg(CTMsg& msg)
{
	CTDEBUG_ENTER_METHOD("CTSocket::ReceiveMsg");

	// Check that we are packing the data as we expect.
	assert(sizeof(MsgStruct) == kMsg1Size) ;

	// Get the buffer into which we will receive the data
	MsgStruct* pMsgStruct = &msg.GetWriteStruct() ;

	// Receive the message
	bool ok = ReceiveBuffer((char*)pMsgStruct, sizeof(MsgStruct)) ;

	// Then check for the additional data buffer
	long dataSize = msg.GetDataSize() ;

	if (ok && dataSize > 0)
	{
		PrintDebugFormat("Receiving extra data buffer of size %d",dataSize) ;

		// Don't delete existing memory--data field we just received points to nowhere.
		const bool kDelete = false ;

		// Allocate enough space for data buffer and a trailing null--as we'll often want
		// to treat this as a string.
		char* pDataBuffer = msg.AllocateDataBuffer(kDelete, dataSize) ;

		if (pDataBuffer)
		{
			// Read the extra data into the data buffer
			ok = ReceiveBuffer(pDataBuffer, dataSize) ;

			// If there's a problem, clear the buffer
			if (!ok)
			{
				PrintDebug("Error: There was an error receiving the extra data buffer") ;
				msg.DeleteData() ;
			}
		}
		else
		{
			// Failed to allocate the memory.
			PrintDebug("Error: Failed to allocate the extra data buffer") ;

			ok = false ;
		}
	}

	// If there was a problem, need to reset the data fields
	// or others might think they're valid...when they're not.
	if (!ok)
	{
		if (IsAlive())
			PrintDebug("Error: Reseting the data fields in the received message due to an error") ;
		else
			PrintDebug("Socket has closed.  Cleaning up data fields") ;

		// Reset data fields to 0, but don't free them--they're not valid.
		msg.DeleteData(false /* don't free */) ;
	}

	// BUGBUG: If this data can be between two different machines
	// (e.g. PC and Linux box) then we need to convert the data back from
	// network layer format using htonl, ntohl etc.

	return ok ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : ReceiveBuffer
// 
// Return type    : bool 	
// Argument       : char* pRecvBuffer	
// Argument       : size_t bufferSize	
// 
// Description	  : Receive a buffer of data.
//
/////////////////////////////////////////////////////////////////////
bool CTSocket::ReceiveBuffer(char* pRecvBuffer, size_t bufferSize)
{
	CTDEBUG_ENTER_METHOD("CTSocket::ReceiveBuffer");

	CHECK_RET_FALSE(pRecvBuffer && bufferSize > 0) ;

	SOCKET hSock = m_hConnection ;

	if (!hSock)
	{
		PrintDebug("Error: Can't read because this socket is closed") ;
		return false;
	}

	size_t bytesRead = 0 ;
	int	   thisRead  = 0 ;

	// Check our incoming data is valid
	if (!pRecvBuffer || !hSock)
	{
		assert(pRecvBuffer && hSock) ;
		return false ;
	}

	// May need to make repeated calls to read all of the data
	while (bytesRead < bufferSize)
	{
		long tries = 0 ;

		do
		{
			tries++ ;
			thisRead = recv(hSock, pRecvBuffer, bufferSize - bytesRead, 0) ;

			// Check if there was an error
			if (thisRead == SOCKET_ERROR)
			{
#ifdef NON_BLOCKING
				// On a non-blocking socket, the socket can return "would block" -- in which case
				// we need to wait for it to clear.  A blocking socket would not return in this case
				// so this would always be an error.
				if (IsErrorWouldBlock())
				{
					PrintDebug("Waiting for socket to unblock") ;
					SleepMillisecs(100) ;
				}
				else
#endif
				{
					PrintDebug("Error: Error receiving message") ;

					ReportErrorCode() ;
					return false ;
				}
			}

			// Check for 0 bytes read--which is the behavior if the remote socket is
			// closed gracefully.
			if (thisRead == 0)
			{
				PrintDebug("Remote socket has closed gracefully") ;

				// Now close down our socket
				PrintDebug("Closing our side of the socket") ;

				CloseSocket() ;

				return false ;	// No message received.
			}
		} while (thisRead == SOCKET_ERROR) ;

		PrintDebugFormat("Received %d bytes",thisRead) ;

		bytesRead   += thisRead ;
		pRecvBuffer += thisRead ;
	}

	return true ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : ReportErrorCode
// 
// Return type    : void 	
// 
// Description	  : Convert the error code to useful text.
//
/////////////////////////////////////////////////////////////////////
void ReportErrorCode()
{
	CTDEBUG_ENTER_METHOD("SoarSocket - ReportErrorCode");

	int error = NET_ERROR_NUMBER ;

	switch (error)
	{
	case NET_NOTINITIALISED:PrintDebug("Error: WSA Startup needs to be called first") ; break ;
	case NET_ENETDOWN:		PrintDebug("Error: Underlying network is down") ; break ;
	case NET_EFAULT:		PrintDebug("Error: Buffer is not in a valid address space") ; break ;
	case NET_ENOTCONN:		PrintDebug("Error: The socket is not connected") ; break ;
	case NET_EINTR:			PrintDebug("Error: The blocking call was cancelled") ; break ;
	case NET_EINPROGRESS:	PrintDebug("Error: A blocking call is in progress") ; break ;
	case NET_ENETRESET:		PrintDebug("Error: The connection has been broken") ; break ;
	case NET_ENOTSOCK:		PrintDebug("Error: The descriptor is not a socket") ; break ;
	case NET_EOPNOTSUPP:	PrintDebug("Error: OOB data is not supported on this socket") ; break ;
	case NET_ESHUTDOWN:		PrintDebug("Error: The socket has been shutdown") ; break ;
	case NET_EWOULDBLOCK:	PrintDebug("Error: The operation would block") ; break ;
	case NET_EMSGSIZE:		PrintDebug("Error: The message is too large") ; break ;
	case NET_EINVAL:		PrintDebug("Error: Need to bind the socket") ; break ;
	case NET_ECONNABORTED:	PrintDebug("Error: The circuit was terminated") ; break ;
	case NET_ETIMEDOUT:		PrintDebug("Error: The conection timed out") ; break ;
	case NET_ECONNRESET:	PrintDebug("Error: The circuit was reset by the remote side") ; break ;

	default:
		{
			PrintDebugFormat("Error: Unknown error %d",error) ;
			break ;
		}
	}
}

/////////////////////////////////////////////////////////////////////
// Function name  : CTSocket::CloseSocket
// 
// Return type    : void 	
// 
// Description	  : Close down the socket.
//
/////////////////////////////////////////////////////////////////////
void CTSocket::CloseSocket()
{
	// BUGBUG: Do we need a parameter to CloseSocket to say
	// whether or not to call shutdown first?  I think that's
	// the correct protocol if we're closing down the connection
	// (and the other side is still alive).

	if (m_hConnection)
	{
		NET_CLOSESOCKET(m_hConnection) ;
		m_hConnection = NO_CONNECTION ;
	}
}
