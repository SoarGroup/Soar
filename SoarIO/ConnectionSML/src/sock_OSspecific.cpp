/////////////////////////////////////////////////////////////////
// OSSpecific class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : ~2001
//
// The parts of the socket code that are specific to the operating system.
// 
/////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"

#ifndef HAVE_UNISTD_H
#error "missing required unistd.h header"
#endif // HAVE_UNISTD_H

#endif // HAVE_CONFIG_H

#ifdef _WIN32
//////////////////////////////////////////////////////////////////////
// Windows Versions
//////////////////////////////////////////////////////////////////////
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>

#include "sock_OSspecific.h"
#include "sock_SocketHeader.h"

bool sock::InitializeOperatingSystemSocketLibrary()
{
	WSADATA wsaData ;

	// Start the windows socket library.
	int result = WSAStartup(MAKEWORD(1,1), &wsaData) ;

	// Result should be 0 if we found the windows socket DLL
	if (result != 0)
		return false ;

	// Check that we got the version we asked for.
	if ( LOBYTE( wsaData.wVersion ) != 1 ||
		 HIBYTE( wsaData.wVersion ) != 1 )
	{
		WSACleanup();
		return false ;
    } 

	return true ;
}

bool sock::TerminateOperatingSystemSocketLibrary()
{
	// Clean up the windows socket library
	WSACleanup() ;

	return true ;
}

bool sock::MakeSocketNonBlocking(SOCKET hSock)
{
	// Now set the socket to be non-blocking
	u_long num = 1 ;	// Set non-blocking to 1 (true)
	int res = ioctlsocket(hSock, FIONBIO, &num) ;

	return (res == 0) ;
}

bool sock::SleepMillisecs(long msecs)
{
	Sleep(msecs) ;

	return true ;
}

#else	// _WIN32
//////////////////////////////////////////////////////////////////////
// Linux Versions
//////////////////////////////////////////////////////////////////////
#include "sock_OSspecific.h"
#include "sock_SocketHeader.h"

#include <unistd.h>			// For sleep

// Nothing needs to be initialized on Linux
bool sock::InitializeOperatingSystemSocketLibrary()
{
	return true ;
}

// Nothing needs to be initialized on Linux
bool sock::TerminateOperatingSystemSocketLibrary()
{
	return true ;
}

bool sock::MakeSocketNonBlocking(SOCKET hSock)
{
	int res = fcntl(hSock, F_SETFL, O_NONBLOCK) ;

	return (res == 0) ;
}

bool sock::SleepMillisecs(long msecs)
{
	// usleep takes microseconds
	usleep(msecs * 1000) ;

	return true ;
}
#endif	// _WIN32
