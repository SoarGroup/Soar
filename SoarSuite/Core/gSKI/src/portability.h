#ifndef PORTABILITY_H
#define PORTABILITY_H

//////////////////////////////////////////////
// TODO
//typedef struct _iobuf FILE;

//////////////////////////////////////////////
// UNCONDITIONAL BOOTSTRAP
#include <stdio.h>

#ifdef HAVE_CONFIG_H
//////////////////////////////////////////////
// PROCESSED WITH AUTOCONF

#if HAVE_UTIME_H
# include <utime.h>
#endif

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
# include <sys/time.h>
# else
# include <time.h>
# endif
#endif

#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif

#if HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif

#if STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# if HAVE_STDLIB_H
# include <stdlib.h>
# endif
#endif

#if HAVE_STRING_H
# if !STDC_HEADERS && HAVE_MEMORY_H
# include <memory.h>
# endif
# include <string.h>
#endif

#if HAVE_STRINGS_H
# include <strings.h>
#endif

#if HAVE_INTTYPES_H
# include <inttypes.h>
#else
# if HAVE_STDINT_H
# include <stdint.h>
# endif
#endif

#if HAVE_UNISTD_H
# include <unistd.h>
#endif

#if HAVE_SYS_SYSCALL_H
#include <sys/syscall.h>
#endif

#if HAVE_ASSERT_H
#include <assert.h>
#else // not HAVE_ASSERT_H
typedef void assert;
#endif // not HAVE_ASSERT_H

#if HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif

#if HAVE_ERRNO_H
#include <errno.h>
#endif

#else // not HAVE_CONFIG_H
//////////////////////////////////////////////
// PROCESSED WITHOUT AUTOCONF

#include <windows.h>
#include <direct.h>
#include <time.h>
#include <assert.h>

#endif // not HAVE_CONFIG_H
//////////////////////////////////////////////
// UNCONDITIONAL WRAP-UP

#include <ctype.h>
#include <math.h>
#include <signal.h>

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024   /* AGR 536  - from sys/param.h */
#endif

#endif // PORTABILITY_H
