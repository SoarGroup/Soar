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

	m_Buffer.resize( len );

	char chunk[ 512 ];
	unsigned long read = 0;

	while ( len )
	{
		unsigned long chunkLength = min( len, 512 );
		assert( chunkLength <= len );

		if ( !ReceiveBuffer( chunk, chunkLength ) )
		{
			return false;
		}
		m_Buffer.replace( read, chunkLength, chunk, chunkLength );

		len -= chunkLength;
		read += chunkLength;
	}

	return true;
}

void DataSender::Close()
{
   soar_thread::Lock lock( &m_CloseMutex );

   CloseInternal();
}