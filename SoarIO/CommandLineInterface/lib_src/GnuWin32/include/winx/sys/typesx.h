#ifndef _WINX_SYS_TYPESX_H_
#define _WINX_SYS_TYPESX_H_

#include <features.h>
//#include <winx/unistdx.h> /* for large-file support */

#ifdef _LARGEFILE_SOURCE
# define __USE_LARGEFILE 1
#endif

#ifdef _LARGEFILE64_SOURCE
# define __USE_LARGEFILE64    1
#endif

#if defined _FILE_OFFSET_BITS && _FILE_OFFSET_BITS == 64
# define _INTEGRAL_MAX_BITS 64
# define __USE_FILE_OFFSET64  1
#endif

typedef unsigned long vm_offset_t;
typedef unsigned long vm_size_t;
typedef long ssize_t;

#ifndef _TIME_T_DEFINED
typedef long time_t;
# define _TIME_T_DEFINED
#endif

#include <bits/types.h>

typedef __int8_t int8_t;
typedef __quad_t quad_t;
typedef __uint8_t u_int8_t;
typedef __int16_t int16_t;
typedef __uint16_t u_int16_t;
#ifndef __SYS_CONFIG_H__
typedef int __int32_t;
typedef unsigned int __uint32_t;
#endif
typedef __int32_t int32_t;
typedef __uint32_t u_int32_t;
typedef __u_quad_t u_quad_t;
typedef __uint32_t uint32_t;
typedef __int32_t __register_t;
typedef int32_t register_t;
typedef __int64_t int64_t;
typedef __uint64_t u_int64_t;

#define __sigset_t_defined 1

#ifdef _sigset_t
typedef _sigset_t __sigset_t;
#endif

typedef __caddr_t caddr_t;
typedef long daddr_t;
typedef	long key_t;
typedef	char *	addr_t;
#ifndef   __useconds_t
 typedef unsigned int useconds_t;
#endif /*  __useconds_t */

#if !defined ( _BSDTYPES_DEFINED )
/* also defined in gmon.h and in cygwin's sys/types */
typedef unsigned char    u_char;
typedef unsigned short   u_short;
typedef unsigned int     u_int;
typedef unsigned long    u_long;
typedef u_long		    ulong;
#define _BSDTYPES_DEFINED
#endif /* !defined  _BSDTYPES_DEFINED */

/* This is not a typedef so `const __ptr_t' does the right thing.  */
#ifndef __ptr_t
#define __ptr_t void *
#endif

#ifndef _CLOCK_T_DEFINED
#undef clock_t
typedef   int64_t clock_t;
#define _CLOCK_T_DEFINED
#endif

#if defined (_MODE_T_) && !defined(_NO_OLDNAMES)
# define __mode_t_defined
#endif

#if defined (_OFF_T_) && !defined(_NO_OLDNAMES)
# define __off_t_defined
#endif

#if defined (_INO_T_) && !defined(_NO_OLDNAMES)
# define __ino_t_defined
#endif

#if defined (_DEV_T_) && !defined(_NO_OLDNAMES)
# define __dev_t_defined
#endif

#if defined (_PID_T_) && !defined(_NO_OLDNAMES)
# define __pid_t_defined
#endif

#if defined (_GID_T_) && !defined(_NO_OLDNAMES)
# define __gid_t_defined
#endif

#if defined (_UID_T_) && !defined(_NO_OLDNAMES)
# define __uid_t_defined
#endif

#ifndef __dev_t_defined
typedef __dev_t dev_t;
# define __dev_t_defined
#else
# ifdef __USE_FILE_OFFSET64
#  define dev_t __dev_t
# endif
#endif

#ifndef __gid_t_defined
typedef __gid_t gid_t;
# define __gid_t_defined
#endif

#ifndef __ino_t_defined
# ifndef __USE_FILE_OFFSET64
typedef __ino_t ino_t;
# else
typedef __ino64_t ino_t;
# endif
# define __ino_t_defined
#else
# ifdef __USE_FILE_OFFSET64
#  define ino_t __ino64_t
# endif
#endif

#ifndef __mode_t_defined
typedef __mode_t mode_t;
# define __mode_t_defined
#endif

# ifndef __nlink_t_defined
typedef __nlink_t nlink_t;
#  define __nlink_t_defined
# endif

#ifndef __off_t_defined
# ifndef __USE_FILE_OFFSET64
typedef __off_t off_t;
# else
typedef __off64_t off_t;
# endif
# define __off_t_defined
#else
# ifdef __USE_FILE_OFFSET64
#  define off_t __off64_t
# endif
#endif

# ifndef __uid_t_defined
typedef __uid_t uid_t;
#  define __uid_t_defined
# endif

# ifndef __blkcnt_t_defined
#  ifndef __USE_FILE_OFFSET64
typedef __blkcnt_t blkcnt_t;
#  else
typedef __blkcnt64_t blkcnt_t;
#  endif
#  define __blkcnt_t_defined
# endif

# ifndef __blksize_t_defined
typedef __blksize_t blksize_t;
#  define __blksize_t_defined
# endif

#ifdef  __USE_LARGEFILE64
typedef __ino64_t ino64_t;
typedef __off64_t off64_t;
#endif /* __USE_LARGEFILE64 */

#ifdef  __USE_FILE_OFFSET64
//# define ino_t __ino64_t
//# define dev_t __dev_t
//# define off_t __off64_t
#endif /* __USE_FILE_OFFSET64 */

#endif /* _WINX_SYS_TYPESX_H_ */
