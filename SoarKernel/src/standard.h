/* standard.h */

#if defined(MACINTOSH)
#include <utime.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#elif defined(WIN32)
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>

#else
#include <sys/time.h>
#endif

#include <stdio.h>
#include <stddef.h>

/*  kjc:  I don't understand what's up with all this __hpux stuff.
 *  when I took out all the embedded old stuff about THINK_C and
 *  __SC__, I'm left with __hpux defines and undefines that cancel
 *  each other out.  so this should be tested in HP's and this
 *  stuff should be removed if not needed...
 */
/*  I changed the goofy stuff to this from agent.c : */
#if defined(__hpux) || defined(UNIX)
#ifndef _INCLUDE_POSIX_SOURCE
#define _INCLUDE_POSIX_SOURCE
#endif
#define _INCLUDE_XOPEN_SOURCE
#define _INCLUDE_HPUX_SOURCE
#include <sys/types.h>
#undef  _INCLUDE_POSIX_SOURCE
#undef  _INCLUDE_XOPEN_SOURCE
#undef  _INCLUDE_HPUX_SOURCE
#endif /* __hpux */

typedef char Bool;
