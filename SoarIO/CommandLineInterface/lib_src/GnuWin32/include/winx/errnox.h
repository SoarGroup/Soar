#ifndef _WINX_ERRNOX_H
#define _WINX_ERRNOX_H
#ifdef __GW32__

#include <sys/types.h>

#define   ETXTBSY 26     					/* Text file busy */
#define   EWOULDBLOCK                EAGAIN /* Operation would block */
#define   EINPROGRESS                35     /* Operation now in progress */
#define   EALREADY                   37     /* Operation already in progress */
#define   ENOTSOCK                   63     /* Socket operation on non-socket */
#define   EMSGSIZE                   77     /* Message too long */
#define   EPROTOTYPE                 78     /* Protocol wrong type for socket */
#define   ENOPROTOOPT                106     /* Protocol not available */
#define   EPROTONOSUPPORT            43     /* Protocol not supported */
#define   ESOCKTNOSUPPORT            44     /* Socket type not supported */
#define   EOPNOTSUPP                 45     /* Operation not supported */
#define   EPFNOSUPPORT               46     /* Protocol family not supported */
#define   EAFNOSUPPORT               47     /* Address family not supported by protocol */
#define   EADDRINUSE                 48     /* Address already in use */
#define   EADDRNOTAVAIL              49     /* Cannot assign requested address */
#define   ENETDOWN                   50     /* Network is down */
#define   ENETUNREACH                51     /* Network is unreachable */
#define   ENETRESET                  52     /* Network dropped connection on reset */
#define   ECONNABORTED               53     /* Software caused connection abort */
#define   ECONNRESET                 54     /* Connection reset by peer */
#define   ENOBUFS                    55     /* No buffer space available */
#define   EISCONN                    56     /* Transport endpoint is already connected */
#define   ENOTCONN                   57     /* Transport endpoint is not connected */
#define   EDESTADDRREQ               39     /* Destination address required */
#define   ESHUTDOWN                  58     /* Cannot send after transport endpoint shutdown */
#define   ETOOMANYREFS               59     /* Too many references: cannot splice */
#define   ETIMEDOUT                  60     /* Connection timed out */
#define   ECONNREFUSED               61     /* Connection refused */
#define   ELOOP                      62     /* Too many levels of symbolic links */
//#define ENAMETOOLONG               63     /* File name too long */
#define   EHOSTDOWN                  64     /* Host is down */
#define   EHOSTUNREACH               65     /* No route to host */
//#define ENOTEMPTY                  66     /* Directory not empty */
#define   EPROCLIM                   67     /* Too many processes */
#define   EUSERS                     68     /* Too many users */
#define   EDQUOT                     69     /* Disk quota exceeded */
#define   ESTALE                     70     /* Stale NFS file handle */
#define   EREMOTE                    71     /* Object is remote */
#define   EBADRPC                    72     /* RPC struct is bad */
#define   ERPCMISMATCH               73     /* RPC version wrong */
#define   EPROGUNAVAIL               74     /* RPC program not available */
#define   EPROGMISMATCH              75     /* RPC program version wrong */
#define   EPROCUNAVAIL               76     /* RPC bad procedure for program */
//#define ENOLCK                     77     /* No locks available */
//#define ENOSYS                     78     /* Function not implemented */
#define   EFTYPE                     79     /* Inappropriate file type or format */
#define   EAUTH                      80     /* Authentication error */
#define   ENEEDAUTH                  81     /* Need authenticator */
#define   EBACKGROUND                100     /* Inappropriate operation for background process */
#define   EDIED                      101     /* Translator died */
#define   ED                         102     /* ? */
#define   EGREGIOUS                  103     /* You really blew it this time */
#define   EIEIO                      104     /* Computer bought the farm */
#define   EGRATUITOUS                105     /* Gratuitous error */
//#define EILSEQ                     106     /* Invalid or incomplete multibyte	or wide character */
#define   EBADMSG                    107     /* Bad message */
#define   EIDRM                      108     /* Identifier removed */
#define   EMULTIHOP                  109     /* Multihop attempted */
#define   ENODATA                    110     /* No data available */
#define   ENOLINK                    111     /* Link has been severed */
#define   ENOMSG                     112     /* No message of desired type */
#define   ENOSR                      113     /* Out of streams resources */
#define   ENOSTR                     114     /* Device not a stream */
#define   EOVERFLOW                  115     /* Value too large for defined data type */
#define   EPROTO                     116     /* Protocol error */
#define   ETIME                      117     /* Timer expired */
#define   ENOTSUP                    118     /* Not supported */

#define   ERR_MAX                    118     

#define __set_errno(a)		(errno = (a))

#ifdef    __cplusplus
extern "C" {
#endif

//#if defined(__USE_GNU) && defined(_LIBC)
///* The full and simple forms of the name with which the program was
//   invoked.  These variables are set up automatically at startup based on
//   the value of ARGV[0] (this works only if you use GNU ld).  */
//char *program_invocation_name, *program_invocation_short_name, *program_name;
//#endif /* __USE_GNU && _LIBC */
//
extern char *__strerror_r (int errnum, char *buf, size_t buflen);
extern int werrno (void);
extern int __get_errno(int oserr);
#define set_werrno		(__set_errno (werrno()) )

#ifdef    __cplusplus
}
#endif

#endif /* __GW32__ */

#endif /* _WINX_ERRNOX_H */
