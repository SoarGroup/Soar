#ifdef __GW32__
#ifndef __WINX_SYS_STATX_H__
#define __WINX_SYS_STATX_H__

#if defined(_STAT_H_) || defined(__STRICT_ANSI__)
# define _SYS_STAT_H
#endif

#include <features.h>
//#include <winx/unistdx.h>  /* for large-file support */
#include <bits/stat.h>
//#include <windows.h>
#include <io.h>

#ifdef _S_IFBLK
#undef _S_IFBLK
#endif
#ifdef S_IFBLK
#undef S_IFBLK
#endif
#define	_S_IFBLK	0x6000	
#define	_S_IFLNK	0xA000	/* symbolic link */
#define	_S_IFSOCK	0xC000	/* socket */
#define	_IFLNK		_S_IFLNK
#define	_IFSOCK		_S_IFSOCK
#define	_S_ISLNK(m)		(((m) & _S_IFMT) == _S_IFLNK)
#define	_S_ISSOCK(m)	(((m) & _S_IFMT) == _S_IFSOCK)

#define __S_IFMT     _S_IFMT
#define __S_IFDIR    _S_IFDIR
#define __S_IFCHR    _S_IFCHR
#define __S_IFBLK    _S_IFBLK
#define __S_IFREG    _S_IFREG
#define __S_IFIFO   _S_IFIFO
#define __S_IFLNK   _S_IFLNK
#define __S_IFSOCK  _S_IFSOCK

#define	_IFMT		_S_IFMT	/* File type mask */
#define	_S_ISUID		0004000	/* set user id on execution */
#define	_S_ISGID		0002000	/* set group id on execution */
#define	_S_ISVTX		0001000	/* save swapped text even after use */
#define	_S_IRWXG	(_S_IRWXU >> 3)
#define	_S_IXGRP	(_S_IXUSR >> 3) /* read permission, group */
#define	_S_IWGRP	(_S_IWUSR >> 3) /* write permission, grougroup */
#define	_S_IRGRP	(_S_IRUSR >> 3) /* execute/search permission, group */
#define	_S_IRWXO	(_S_IRWXG >> 3) 
#define	_S_IXOTH	(_S_IXGRP >> 3) /* read permission, other */
#define	_S_IWOTH	(_S_IWGRP >> 3) /* write permission, other */
#define	_S_IROTH	(_S_IRGRP  >> 3) /* execute/search permission, other */

#ifndef _NO_OLDNAMES

#define	S_ISUID	_S_ISUID
#define	S_ISGID	_S_ISGID
#define	S_ISVTX	_S_ISVTX

#define	S_IRWXG		_S_IRWXG
#define	S_IXGRP		_S_IXGRP
#define	S_IWGRP		_S_IWGRP
#define	S_IRGRP		_S_IRGRP
#define	S_IRWXO		_S_IRWXO
#define	S_IXOTH		_S_IXOTH
#define	S_IWOTH		_S_IWOTH
#define	S_IROTH		_S_IROTH

#define	S_IFBLK		_S_IFBLK
#define	S_IFLNK		_S_IFLNK
#define	S_IFSOCK	_S_IFSOCK

#define	S_ISLNK(m)	_S_ISLNK(m)
#define	S_ISSOCK(m)	_S_ISSOCK(m)

#endif	/* Not _NO_OLDNAMES */

//#ifdef __USE_LARGEFILE64
//struct __stat64
//  {
//    __mode_t st_mode;         /* File mode.  */
//    __ino64_t st_ino;         /* File serial number.   */
//    __dev_t st_dev;      /* Device.  */
//    __nlink_t st_nlink;       /* Link count.  */
//
//    __uid_t st_uid;      /* User ID of the file's owner.    */
//    __gid_t st_gid;      /* Group ID of the file's group.*/
//    __off64_t st_size;        /* Size of file, in bytes.  */
//
//    __time_t st_atime;        /* Time of last access.  */
//    __time_t st_mtime;        /* Time of last modification.  */
//    __time_t st_ctime;        /* Time of last status change.  */
//  };
//#endif

////# define lstat stat
//
//#ifdef  __USE_LARGEFILE64
////# define __lxstat64(v,f,b) _stati64(f,b)
////# define lxstat64(v,f,b) _stati64(f,b)
//# define stat64 _stati64
//# define fstat64 _fstati64
//# define ino64_t __ino64_t
//# define off64_t __off64_t
//#endif /* __USE_LARGEFILE64 */
//
//#ifdef  __USE_FILE_OFFSET64
//# define lstat _stati64
//# define __lxstat64(v,f,b) _stati64(f,b)
//# define lxstat64(v,f,b) _stati64(f,b)
//# define stat _stati64
//# define fstat _fstati64
//# define stat64 _stati64
//# define fstat64 _fstati64
//# define ino_t __ino64_t
//# define ino64_t __ino64_t
//# define off_t __off64_t
//# define off64_t __off64_t
//#endif /* __USE_FILE_OFFSET64 */

/* To allow the `struct stat' structure and the file type `mode_t'
   bits to vary without changing shared library major version number,
   the `stat' family of functions and `mknod' are in fact inline
   wrappers around calls to `xstat', `fxstat', `lxstat', and `xmknod',
   which all take a leading version-number argument designating the
   data structure and bits used.  <bits/stat.h> defines _STAT_VER with
   the version number corresponding to `struct stat' as defined in
   that file; and _MKNOD_VER with the version number corresponding to
   the S_IF* macros defined therein.  It is arranged that when not
   inlined these function are always statically linked; that way a
   dynamically-linked executable always encodes the version number
   corresponding to the data structures it uses, so the `x' functions
   in the shared library can adapt without needing to recompile all
   callers.  */

#ifndef _STAT_VER
# define _STAT_VER  0
#endif
#ifndef _MKNOD_VER
# define _MKNOD_VER 0
#endif

/* Wrappers for stat and mknod system calls.  */
#ifndef __USE_FILE_OFFSET64
extern int __fxstat (int __ver, int __fildes, struct stat *__stat_buf) __THROW;
extern int __xstat (int __ver, __const char *__filename,
              struct stat *__stat_buf) __THROW;
extern int __lxstat (int __ver, __const char *__filename,
               struct stat *__stat_buf) __THROW;
#else
# ifdef __REDIRECT
extern int __REDIRECT (__fxstat, (int __ver, int __fildes,
                      struct stat *__stat_buf) __THROW,
                 __fxstat64);
extern int __REDIRECT (__xstat, (int __ver, __const char *__filename,
                     struct stat *__stat_buf) __THROW, __xstat64);
extern int __REDIRECT (__lxstat, (int __ver, __const char *__filename,
                      struct stat *__stat_buf) __THROW,
                 __lxstat64);

# else
#  define __fxstat __fxstat64
#  define __xstat __xstat64
#  define __lxstat __lxstat64
# endif
#endif

#ifdef __USE_LARGEFILE64
extern int __fxstat64 (int __ver, int __fildes, struct stat64 *__stat_buf)
     __THROW;
extern int __xstat64 (int __ver, __const char *__filename,
                struct stat64 *__stat_buf) __THROW;
//extern int __hxstat64 (HANDLE __fh, __const char *__filename,
//                struct stat64 *__stat_buf) __THROW;
extern int __lxstat64 (int __ver, __const char *__filename,
                 struct stat64 *__stat_buf) __THROW;
#endif
extern int __xmknod (int __ver, __const char *__path, __mode_t __mode,
               __dev_t *__dev) __THROW;

#if defined __GNUC__ && __GNUC__ >= 2
/* Inlined versions of the real stat and mknod functions.  */

#ifndef __USE_FILE_OFFSET64
extern __inline__ int stat (__const char *__path,
                    struct stat *__statbuf) __THROW
{
//  fprintf(stderr, "stat -> __xstat\n");
  return __xstat (_STAT_VER, __path, __statbuf);
}
#else
# ifdef __REDIRECT
extern int __REDIRECT (stat,
                 (__const char *__restrict __file,
               struct stat *__restrict __buf) __THROW,
                 stat64);
# else
#   define stat stat64
# endif
#endif

#if defined __USE_BSD || defined __USE_XOPEN_EXTENDED
# ifndef __USE_FILE_OFFSET64
extern __inline__ int lstat (__const char *__path,
                    struct stat *__statbuf) __THROW
{
//  fprintf(stderr, "lstat -> __lxstat\n");
  return __lxstat (_STAT_VER, __path, __statbuf);
}
# else
#  ifdef __REDIRECT
extern int __REDIRECT (lstat,
                 (__const char *__restrict __file,
               struct stat *__restrict __buf) __THROW,
                 lstat64);
#  else
#   define lstat lstat64
#  endif
# endif
#endif


extern
__inline__ int fstat (int __fd, struct stat *__statbuf) __THROW
{
  return __fxstat (_STAT_VER, __fd, __statbuf);
}

# if defined __USE_MISC || defined __USE_BSD
extern __inline__ int mknod (__const char *__path, __mode_t __mode,
                    __dev_t __dev) __THROW
{
  return __xmknod (_MKNOD_VER, __path, __mode, &__dev);
}
# endif

# if defined __USE_LARGEFILE64 \
  && (! defined __USE_FILE_OFFSET64 \
      || (defined __REDIRECT && defined __OPTIMIZE__))
extern __inline__ int stat64 (__const char *__path,
                     struct stat64 *__statbuf) __THROW
{
  return __xstat64 (_STAT_VER, __path, __statbuf);
}

#  if defined __USE_BSD || defined __USE_XOPEN_EXTENDED
extern __inline__ int lstat64 (__const char *__path,
                      struct stat64 *__statbuf) __THROW
{
  return __lxstat64 (_STAT_VER, __path, __statbuf);
}
#  endif

extern __inline__ int fstat64 (int __fd, struct stat64 *__statbuf) __THROW
{
  return __fxstat64 (_STAT_VER, __fd, __statbuf);
}
# endif

#endif

/* Create a new directory named PATH, with permission bits MODE.  */
extern __mkdir (__const char *__path, __mode_t __mode) __THROW;

/* Set file access permissions of the file FD is open on to MODE.  */
#if defined __USE_BSD || defined __USE_XOPEN_EXTENDED
extern int fchmod (int __fd, __mode_t __mode) __THROW;
#endif

#endif /* __WINX_SYS_STATX_H__ */

#endif /* __GW32__ */
