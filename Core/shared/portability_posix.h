#ifndef PORTABILITY_POSIX_H
#define PORTABILITY_POSIX_H

/* This file contains code specific to the posix platforms */

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stddef.h>
#include <stdint.h>
#include <strings.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <utime.h>
#include <pthread.h>
#include <dirent.h>
#include <regex.h>

/* this needs to be defined */
#ifndef MAXPATHLEN
#define MAXPATHLEN 1024   /* AGR 536  - from sys/param.h */
#endif // MAXPATHLEN

// Use local sockets instead of internet sockets for same-machine interprocess communication
#define ENABLE_LOCAL_SOCKETS

#include <dlfcn.h>      // Needed for dlopen and dlsym
#define GetProcAddress dlsym

#define SNPRINTF snprintf
#define VSNPRINTF vsnprintf

/* socket support stuff */

///////
// This maps some constants to values that can be used on any platform
#ifndef SUN_LEN
/* This system is not POSIX.1g.         */
#define SUN_LEN(ptr) ((size_t) (((struct sockaddr_un *) 0)->sun_path) + strlen ((ptr)->sun_path))
#endif

// For some reason, Darwin (MacOSX) doesn't have this
// It should be defined in netinet/in.h
#ifndef IPPORT_ECHO
#define IPPORT_ECHO 7
#endif // IPPORT_ECHO

#define NET_CLOSESOCKET		close
#define ERROR_NUMBER		errno

#define SOCKET			int
#define HOSTENT			hostent
#define SOCKADDR_IN		sockaddr_in
#define SOCKADDR		sockaddr
#define TIMEVAL			timeval

#define INVALID_SOCKET	-1
#define SOCKET_ERROR	-1

#define NET_EWOULDBLOCK		EWOULDBLOCK
#define NET_ENETDOWN		ENETDOWN
#define NET_EFAULT			EFAULT
#define NET_ENOTCONN		ENOTCONN
#define NET_EINTR			EINTR
#define NET_EINPROGRESS		EINPROGRESS
#define NET_ENETRESET		ENETRESET
#define NET_ENOTSOCK		ENOTSOCK
#define NET_EOPNOTSUPP		EOPNOTSUPP
#define NET_ESHUTDOWN		ESHUTDOWN
#define NET_EWOULDBLOCK		EWOULDBLOCK
#define NET_EMSGSIZE		EMSGSIZE
#define NET_EINVAL			EINVAL
#define NET_ECONNABORTED	ECONNABORTED
#define NET_ETIMEDOUT		ETIMEDOUT
#define NET_ECONNRESET		ECONNRESET

// Don't think this error maps to Unix error codes.
// Leave it with original Windows value just so we can compile.
#define NET_NOTINITIALISED	10093
// 
///////

#define NET_SD_BOTH			SHUT_RDWR


#endif // PORTABILITY_POSIX_H

