#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
//FIXME: #include <portability.h>

/////////////////////////////////////////////////////////////////
// OSSpecific class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : ~2001
//
// The parts of the socket code that are specific to the operating system.
// 
/////////////////////////////////////////////////////////////////

#include <assert.h>

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

bool sock::SleepSocket(long secs, long msecs)
{
	assert(msecs < 1000 && "Specified milliseconds too large; use seconds argument to specify part of time >= 1000 milliseconds");
	
	Sleep((secs * 1000) + msecs) ;

	return true ;
}

#else	// _WIN32
//////////////////////////////////////////////////////////////////////
// Linux Versions
//////////////////////////////////////////////////////////////////////
#include "sock_OSspecific.h"
#include "sock_SocketHeader.h"

#include <time.h>			// For sleep

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

bool sock::SleepSocket(long secs, long msecs)
{
	assert(msecs < 1000 && "Specified milliseconds too large; use seconds argument to specify part of time >= 1000 milliseconds");
	
	struct timespec sleeptime;
	sleeptime.tv_sec = secs;
	sleeptime.tv_nsec = msecs * 1000000;
	nanosleep(&sleeptime, 0);

	return true ;
}
#endif	// _WIN32
