// OS specific header files

// True when running the Windows compiler
#ifdef _WIN32
#define WIN_HEADERS
#endif

// True on Windows, unless building a command line app
#ifdef _WINDOWS
#define WIN_HEADERS
#endif

#ifdef WIN_HEADERS
#ifndef __STDC__ 
#include "WinSock2.h"

#define NET_CLOSESOCKET		closesocket
#define NET_ERROR_NUMBER	WSAGetLastError()

#define NET_EWOULDBLOCK		WSAEWOULDBLOCK
#define NET_ENETDOWN		WSAENETDOWN
#define NET_EFAULT			WSAEFAULT
#define NET_ENOTCONN		WSAENOTCONN
#define NET_EINTR			WSAEINTR
#define NET_EINPROGRESS		WSAEINPROGRESS
#define NET_ENETRESET		WSAENETRESET
#define NET_ENOTSOCK		WSAENOTSOCK
#define NET_EOPNOTSUPP		WSAEOPNOTSUPP
#define NET_ESHUTDOWN		WSAESHUTDOWN
#define NET_WOULDBLOCK		WSAEWOULDBLOCK
#define NET_EMSGSIZE		WSAEMSGSIZE
#define NET_EINVAL			WSAEINVAL
#define NET_ECONNABORTED	WSAECONNABORTED
#define NET_ETIMEDOUT		WSAETIMEDOUT
#define NET_ECONNRESET		WSAECONNRESET
#define NET_NOTINITIALISED	WSANOTINITIALISED

#endif

#else	// WIN_HEADERS

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <unistd.h>

#include <netinet/in.h>

// For some reason, Darwin (MacOSX) doesn't have this
// It should be defined in netinet/in.h
#ifndef IPPORT_ECHO
#define IPPORT_ECHO 7
#endif // IPPORT_ECHO

#include <limits.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h> // FIXME: redundant?  see up about 6 lines
#include <fcntl.h>

#define NET_CLOSESOCKET		close
#define NET_ERROR_NUMBER	errno

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

#endif

