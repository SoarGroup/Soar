#ifndef _WINX_WINSOCKX_H_
#define	_WINX_WINSOCKX_H_
#ifdef __GW32__
/* Declarations of various socket errors: */

#define EWOULDBLOCK             WSAEWOULDBLOCK
#define EINPROGRESS             WSAEINPROGRESS
#define EALREADY                WSAEALREADY
#define ENOTSOCK                WSAENOTSOCK
#define EDESTADDRREQ            WSAEDESTADDRREQ
#define EMSGSIZE                WSAEMSGSIZE
#define EPROTOTYPE              WSAEPROTOTYPE
#define ENOPROTOOPT             WSAENOPROTOOPT
#define EPROTONOSUPPORT         WSAEPROTONOSUPPORT
#define ESOCKTNOSUPPORT         WSAESOCKTNOSUPPORT
#define EOPNOTSUPP              WSAEOPNOTSUPP
#define EPFNOSUPPORT            WSAEPFNOSUPPORT
#define EAFNOSUPPORT            WSAEAFNOSUPPORT
#define EADDRINUSE              WSAEADDRINUSE
#define EADDRNOTAVAIL           WSAEADDRNOTAVAIL
#define ENETDOWN                WSAENETDOWN
#define ENETUNREACH             WSAENETUNREACH
#define ENETRESET               WSAENETRESET
#define ECONNABORTED            WSAECONNABORTED
#define ECONNRESET              WSAECONNRESET
#define ENOBUFS                 WSAENOBUFS
#define EISCONN                 WSAEISCONN
#define ENOTCONN                WSAENOTCONN
#define ESHUTDOWN               WSAESHUTDOWN
#define ETOOMANYREFS            WSAETOOMANYREFS
#define ETIMEDOUT               WSAETIMEDOUT
#define ECONNREFUSED            WSAECONNREFUSED
#define ELOOP                   WSAELOOP
#define EHOSTDOWN               WSAEHOSTDOWN
#define EHOSTUNREACH            WSAEHOSTUNREACH
#define EPROCLIM                WSAEPROCLIM
#define EUSERS                  WSAEUSERS
#define EDQUOT                  WSAEDQUOT
#define ESTALE                  WSAESTALE
#define EREMOTE                 WSAEREMOTE

#include <sys/types.h>

#ifndef socklen_t
typedef __socklen_t socklen_t;
#endif

//#define gethostname __wingethostname
//#define gethostbyname __wingethostbyname
//#define gethostbyaddr __wingethostbyaddr

#ifndef _STRUCT_TIMEVAL
#define _STRUCT_TIMEVAL
#endif

#ifndef __TIMEVAL__
#define __TIMEVAL__
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern void WSAErr(const char *Msg);
extern void ws_cleanup(void);
extern int ws_init (void);
extern int __w32_ingethostname (char *__name, size_t __len);
extern long int gethostid (void);
extern int getdomainname (char *name, size_t len);
extern struct hostent *__w32_gethostbyaddr (__const void *__addr, __socklen_t __len,
	int __type);
extern struct hostent *__w32_gethostbyname (__const char *__name);

#ifdef __cplusplus
}
#endif

#endif /* __GW32__ */

#endif /* _WINX_WINSOCKX_H_ */
