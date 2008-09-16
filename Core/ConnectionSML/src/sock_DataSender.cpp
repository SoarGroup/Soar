#include <portability.h>

#include "sock_DataSender.h"

using namespace sock ;

/////////////////////////////////////////////////////////////////////
// Function name  : DataSender::SendBuffer
// 
// Return type    : bool 	
// Argument       : char* String	
// 
// Description	  : Send a string of data to a socket.
//					The outgoing format on the socket will be
//					a 4-byte length followed by the string of characters.
//
/////////////////////////////////////////////////////////////////////
bool DataSender::SendString(char const* pString)
{
	unsigned long len = (unsigned long)strlen(pString) ;

	// Convert the value into network byte ordering (so it's compatible if we send it
	// from a big-endian machine to a little endian one or vice-versa).
	unsigned long netLen = htonl(len) ;

	// Send the length out first
	bool ok = SendBuffer((char*)&netLen, sizeof(netLen)) ;

	// Now send the string of characters
	ok = ok && SendBuffer(pString, len) ;

	return ok ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : DataSender::SendBuffer
// 
// Return type    : bool 	
// Argument       : char* String	
// 
// Description	  : Send a string of data to a socket.
//					The outgoing format on the socket will be
//					a 4-byte length followed by the string of characters.
//
/////////////////////////////////////////////////////////////////////
bool DataSender::ReceiveString()
{
	unsigned long netLen = 0 ;

	// Read the length of the string (the first 4 bytes)
	bool ok = ReceiveBuffer((char*)&netLen, sizeof(netLen)) ;

	// Convert the length from network byte ordering back to our local order
	unsigned long len = ntohl(netLen) ;

	// If we got a zero length string.
	if (len == 0)
		return ok ;

	bufferCurrentSize = 0;
	if ( len > bufferCapacity - 1 ) {
		delete [] buffer;
		buffer = new char[ len + 1 ];
		bufferCapacity = len + 1;
	}

	if ( !ReceiveBuffer( buffer, len ) )
	{
		return false;
	}

	buffer[ len ] = 0;

	return true;
}

void DataSender::Close()
{
   soar_thread::Lock lock( &m_CloseMutex );

   CloseInternal();
}