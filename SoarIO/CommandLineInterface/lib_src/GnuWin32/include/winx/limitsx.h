#ifndef __WINX_LIMITSX_H__
#define __WINX_LIMITSX_H__
#ifdef __GW32__

//#define NGROUPS_MAX 0
//#define ARG_MAX	4096
//#define NAME_MAX	255
//
/* POSIX values */
/* These should never vary from one system type to another */
/* They represent the minimum values that POSIX systems must support.
   POSIX-conforming apps must not require larger values. */
#include <bits/posix1_lim.h>

//#define	_POSIX_ARG_MAX		4096
//#define _POSIX_CHILD_MAX	6
//#define _POSIX_LINK_MAX		8
//#define _POSIX_MAX_CANON	255
//#define _POSIX_MAX_INPUT	255
//#define _POSIX_NAME_MAX		14
//#define _POSIX_NGROUPS_MAX	0
//#define _POSIX_OPEN_MAX		16
//#define _POSIX_PATH_MAX		255
//#define _POSIX_PIPE_BUF		512
//#define _POSIX_SSIZE_MAX	32767
//#define _POSIX_STREAM_MAX	8
//#define _POSIX_TZNAME_MAX       3
//
#ifdef    __USE_XOPEN
# include <bits/xopen_lim.h>
#endif

#endif /* __GW32__ */

#endif /* __WINX_LIMITSX_H__ */
