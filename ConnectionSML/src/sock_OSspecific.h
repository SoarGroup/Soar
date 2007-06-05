/////////////////////////////////////////////////////////////////
// OSSpecific class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : ~2001
//
// The parts of the socket code that are specific to the operating system.
// 
/////////////////////////////////////////////////////////////////

#ifndef CT_OS_SPECIFIC_H
#define CT_OS_SPECIFIC_H

#include "sock_SocketHeader.h"	// For SOCKET definition

namespace sock {

bool InitializeOperatingSystemSocketLibrary() ;
bool TerminateOperatingSystemSocketLibrary() ;
bool MakeSocketNonBlocking(SOCKET hSock) ;
bool SleepSocket(long secs, long msecs) ;
} // Namespace

// Map certain functions depending on the OS
#ifdef _WIN32
#define STRICMP	   stricmp
#define VSNSPRINTF _vsnprintf
#define ENABLE_NAMED_PIPES
#else
#define STRICMP    strcasecmp
#define VSNSPRINTF vsnprintf
//#define ENABLE_LOCAL_SOCKETS
#endif

#endif // CT_OS_SPECIFIC_H

