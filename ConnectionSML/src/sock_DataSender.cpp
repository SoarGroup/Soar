#include "WinSock2.h" // for htonl

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
bool DataSender::ReceiveString(std::string* pString)
{
	unsigned long netLen = 0 ;

	// Make sure we return an empty string if we get an error
	pString->clear() ;

	// Read the length of the string (the first 4 bytes)
	bool ok = ReceiveBuffer((char*)&netLen, sizeof(netLen)) ;

	// Convert the length from network byte ordering back to our local order
	unsigned long len = ntohl(netLen) ;

	// If we got a zero length string.
	if (len == 0)
		return ok ;

	// Create the buffer into which we'll receive data
	char* buffer = new char[len+1] ;

	// Receive the string
	ok = ok && ReceiveBuffer(buffer, len) ;

	// Make it null terminated
	buffer[len] = 0 ;

	// Return the result in the string
	if (ok)
	{
		pString->assign(buffer) ;
	}

	// Release our temp buffer
	delete buffer ;

	return ok ;
}