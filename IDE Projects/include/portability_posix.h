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
#include <stdio.h>
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
#include <limits.h>

#if (defined(__APPLE__) && defined(__MACH__))
#include <mach-o/dyld.h>
#endif

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

#define NET_CLOSESOCKET     close
#define ERROR_NUMBER        errno

#define SOCKET          int
#define HOSTENT         hostent
#define SOCKADDR_IN     sockaddr_in
#define SOCKADDR        sockaddr
#define TIMEVAL         timeval

#define INVALID_SOCKET  -1
#define SOCKET_ERROR    -1

#define NET_EWOULDBLOCK     EWOULDBLOCK
#define NET_ENETDOWN        ENETDOWN
#define NET_EFAULT          EFAULT
#define NET_ENOTCONN        ENOTCONN
#define NET_EINTR           EINTR
#define NET_EINPROGRESS     EINPROGRESS
#define NET_ENETRESET       ENETRESET
#define NET_ENOTSOCK        ENOTSOCK
#define NET_EOPNOTSUPP      EOPNOTSUPP
#define NET_ESHUTDOWN       ESHUTDOWN
#define NET_EWOULDBLOCK     EWOULDBLOCK
#define NET_EMSGSIZE        EMSGSIZE
#define NET_EINVAL          EINVAL
#define NET_ECONNABORTED    ECONNABORTED
#define NET_ETIMEDOUT       ETIMEDOUT
#define NET_ECONNRESET      ECONNRESET

// Don't think this error maps to Unix error codes.
// Leave it with original Windows value just so we can compile.
#define NET_NOTINITIALISED  10093
//
///////

#define NET_SD_BOTH         SHUT_RDWR

#if (__GNUC__ > 4 || (__GNUC__ == 4 && (__GNUC_MINOR__ > 2 || (__GNUC_MINOR__ == 2)))) && (!defined(__i386__) && !defined(__i486__) && !defined(__i586__) && !defined(__i686__))
// requires GCC>=4.2.0
// additionally fails at link time on i386/i486/i586/i686
static inline long atomic_inc(volatile long*  v)
{
    return __sync_add_and_fetch(v, 1);
}

static inline long atomic_dec(volatile long* v)
{
    return __sync_sub_and_fetch(v, 1);
}

#define HAVE_ATOMICS 1

#endif // GCC<4.2.0
static inline int set_working_directory_to_executable_path()
{
    char application_path[FILENAME_MAX];
    int length;
    
#if (defined(__APPLE__) && defined(__MACH__))
    uint32_t size = sizeof(application_path);
    length = _NSGetExecutablePath(application_path, &size) ? -1 : int(strlen(application_path));
#else
    length = readlink("/proc/self/exe", application_path, FILENAME_MAX);
#endif
    
    for (; length != -1; --length)
    {
        if (application_path[length] == '/')
        {
            application_path[length] = '\0';
            break;
        }
    }
    
    if (length == -1)
    {
        fprintf(stderr, "Detecting working directory failed.\n");
        return -1;
    }
    
    int rv = chdir(application_path);
    if (rv)
    {
        fprintf(stderr, "Failed to set working directory.\n");
    }
    
    return rv;
}

#if (defined(__APPLE__) && defined(__MACH__))
#include <mach/mach.h>
#include <mach/mach_time.h>

inline uint64_t get_raw_time()
{
    return mach_absolute_time();
}

inline double get_raw_time_per_usec()
{
    mach_timebase_info_data_t ti;
    mach_timebase_info(&ti);
    return (1e3 * ti.denom) / ti.numer;
}

#else
#include <ctime>

inline uint64_t get_raw_time()
{
    timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1e9 + ts.tv_nsec;
}

inline double get_raw_time_per_usec()
{
    return 1000; // 1000 nanosecs per microsec
}

#endif

#endif // PORTABILITY_POSIX_H

