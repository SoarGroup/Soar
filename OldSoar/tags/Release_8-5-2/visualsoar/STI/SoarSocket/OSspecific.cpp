// OSspecific.cpp: implementation of the OSspecific functions.
//
//////////////////////////////////////////////////////////////////////

#ifdef _WIN32
//////////////////////////////////////////////////////////////////////
// Windows Versions
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "OSspecific.h"
#include "SocketHeader.h"

// Bring in the windows socket library
// This linker option is Windows specific.
#pragma comment (lib, "ws2_32.lib")

bool InitializeOperatingSystemSocketLibrary()
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

bool TerminateOperatingSystemSocketLibrary()
{
	// Clean up the windows socket library
	WSACleanup() ;

	return true ;
}

bool MakeSocketNonBlocking(SOCKET hSock)
{
	// Now set the socket to be non-blocking
	u_long num = 1 ;	// Set non-blocking to 1 (true)
	int res = ioctlsocket(hSock, FIONBIO, &num) ;

	return (res == 0) ;
}

bool SleepMillisecs(long msecs)
{
	Sleep(msecs) ;

	return true ;
}

#else	// _WIN32
//////////////////////////////////////////////////////////////////////
// Linux Versions
//////////////////////////////////////////////////////////////////////
#include "OSspecific.h"
#include "SocketHeader.h"
#include <unistd.h>			// For sleep

// Nothing needs to be initialized on Linux
bool InitializeOperatingSystemSocketLibrary()
{
	return true ;
}

// Nothing needs to be initialized on Linux
bool TerminateOperatingSystemSocketLibrary()
{
	return true ;
}

bool MakeSocketNonBlocking(SOCKET hSock)
{
	int res = fcntl(hSock, F_SETFL, O_NONBLOCK) ;

	return (res == 0) ;
}

bool SleepMillisecs(long msecs)
{
	// usleep takes microseconds
	usleep(msecs * 1000) ;

	return true ;
}

#endif	// _WIN32