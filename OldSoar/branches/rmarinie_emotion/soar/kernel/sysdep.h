
#include <signal.h>

/*
 *   OS Specific includes
 */

#if (defined (_WINDOWS) || defined (WIN32) )    /* excludeFromBuildInfo */
#include <stdlib.h>
#include <malloc.h>
#include <string.h>

#if !defined(WIN32)

#define UNIX                    /* excludeFromBuildInfo */
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>

#endif                          /* !WIN32 */
#endif                          /* _WINDOWS */

/*
 *    sys_random()
 *    sys_srandom()
 */

#if defined(THINK_C) || defined(__hpux) || defined(_WINDOWS) || defined(WIN32) || defined(MACINTOSH)

#define sys_random()     rand()
#define sys_srandom(x)   srand(x)
#else

#define sys_random()     random()
#define sys_srandom(x)   srandom(x)

#endif

/*
 *     sys_getwd
 *     sys_chdir - added by voigtjr since this differs on win32
 */

#if defined(WIN32)
    /* #include <direct.h> */
    /* #define sys_getwd(arg) getcwd(arg, 9999) voigtjr - getcwd deprecated as of Win3.1 */
#define sys_getwd(arg, len) GetCurrentDirectory(len, arg)
#define sys_chdir(arg) SetCurrentDirectory(arg)

#elif defined(MACINTOSH)
#define sys_getwd(arg, len) getcwd(arg, len)
#define sys_chdir(arg) chdir(arg)

#elif defined(__hpux)
#include <sys/syscall.h>
#include <unistd.h>
#define getrusage(a, b) syscall(SYS_GETRUSAGE, a, b)
#define sys_getwd(arg, len) getcwd(arg, (size_t) len)
#define sys_chdir(arg) chdir(arg)

#else

#define sys_getwd(arg, len) getcwd(arg, (size_t) len)
#define sys_chdir(arg) chdir(arg)
#endif

/*
 *  sys_abort
 */

#ifdef _WINDOWS
#define sys_abort() Terminate(1)
#else
#define sys_abort() abort()
#endif

/*
 *   timersub
 */

#ifdef WIN32

#ifndef PII_TIMERS
/* macro taken from linux time.h file since not in VC++ !!! */
#define timersub(a, b, result)                                                \
  do {                                                                        \
    (result)->tv_sec = (a)->tv_sec - (b)->tv_sec;                             \
    (result)->tv_usec = (a)->tv_usec - (b)->tv_usec;                          \
    if ((result)->tv_usec < 0) {                                              \
      --(result)->tv_sec;                                                     \
      (result)->tv_usec += 1000000;                                           \
    }                                                                         \
  } while (0)
#else                           /* PII_TIMERS */

#define timersub( a, b, result ) (*(result) = *(a) - *(b));

#endif                          /* PII_TIMERS */

#endif                          /* WIN32 */

/*
 *		sys_sleep
 */

#ifdef WIN32

#define sys_sleep( seconds )    _sleep( seconds )

#else                           /* WIN32 */

#define sys_sleep( seconds )    sleep( seconds )

#endif                          /* !WIN32 */
