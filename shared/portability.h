#ifndef PORTABILITY_H
#define PORTABILITY_H

#ifdef SCONS
/* New SCons section */

/* unconditional includes */
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <signal.h>

#ifdef WIN32
#include <windows.h>
#include <direct.h>
#include <time.h>
#include <assert.h>

// Visual Studio 2005 requires these:
#define getcwd _getcwd
#define chdir _chdir
#define strcasecmp _stricmp

#else // WIN32
/* posix includes */
#include <arpa/inet.h>
#include <errno.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <utime.h>

#endif // not WIN32

/* unconditional includes */
#include <assert.h>
#include <time.h>

/* this needs to be defined */
#ifndef MAXPATHLEN
#define MAXPATHLEN 1024   /* AGR 536  - from sys/param.h */
#endif // MAXPATHLEN

/* necessary in connectionsml socket code */
#define STRICMP    strcasecmp
#define VSNSPRINTF vsnprintf
#define ENABLE_LOCAL_SOCKETS

#else // SCONS

#include <stdio.h>

#include <winsock2.h>
#include <windows.h>
#include <direct.h>
#include <time.h>
#include <assert.h>

// Visual Studio 2005 requires these:
#define getcwd _getcwd
#define chdir _chdir
#define strcasecmp _stricmp

#include <ctype.h>
#include <math.h>
#include <signal.h>

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024   /* AGR 536  - from sys/param.h */
#endif

// Map certain functions depending on the OS
#define STRICMP	   stricmp
#define VSNSPRINTF _vsnprintf
#define ENABLE_NAMED_PIPES

#endif // not SCONS
#endif // PORTABILITY_H
