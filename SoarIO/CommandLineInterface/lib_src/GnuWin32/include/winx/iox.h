#ifndef __WINX_IOX_H__
#define __WINX_IOX_H__
#ifdef __GW32__

/* #define __need_getopt */

#include <features.h>
//#include <winx/unistdx.h> /* for large-file support */
#include <stdlib.h>
#include <getopt.h>
#ifndef _O_BINARY
# undef _FCNTL_H_
# ifdef __STRICT_ANSI__
#  define __U_STRICT_ANSI__
#  undef __STRICT_ANSI__
# endif
# include <fcntl.h>
# ifdef __U_STRICT_ANSI__
#  define __STRICT_ANSI__
#  undef __U_STRICT_ANSI__
# endif
#endif
#include <sys/types.h>

#ifdef	__cplusplus
extern "C" {
#endif

#define D_OK	0x10

/* Truncate FILE to LENGTH bytes.  */
#ifndef __USE_FILE_OFFSET64
extern int truncate (__const char *__file, __off_t __length) __THROW;
#else
# ifdef __REDIRECT
extern int __REDIRECT (truncate,
                 (__const char *__file, __off64_t __length) __THROW,
                 truncate64);
# else
#   define truncate truncate64
# endif
#endif
#ifdef __USE_LARGEFILE64
extern int truncate64 (__const char *__file, __off64_t __length) __THROW;
#endif

/* Truncate the file FD is open on to LENGTH bytes.  */
#ifndef __USE_FILE_OFFSET64
extern __inline__ int ftruncate (int __fd, __off_t __length) __THROW
{
	return _chsize(__fd, (long) __length);
}
#else
# ifdef __REDIRECT
extern int __REDIRECT (ftruncate, (int __fd, __off64_t __length) __THROW,
                 ftruncate64);
# else
#  define ftruncate ftruncate64
# endif
#endif
#ifdef __USE_LARGEFILE64
extern int ftruncate64 (int __fd, __off64_t __length) __THROW;
#endif

/* Create a one-way communication channel (pipe).
   If successful, two file descriptors are stored in PIPEDES;
   bytes written on PIPEDES[1] can be read from PIPEDES[0].
   Returns 0 if successful, -1 if not.  */
extern __inline__ int pipe (int __pipedes[2]) __THROW
{
	return _pipe(__pipedes, 0, _O_BINARY | _O_NOINHERIT);
}
#if defined __USE_BSD || defined __USE_XOPEN
/* Make all changes done to FD actually appear on disk.  */
extern __inline__ int fsync (int __fd) __THROW
{
	return _commit(__fd);
}
#endif /* Use BSD || X/Open.  */

/* Get the pathname of the current working directory,
   and put it in SIZE bytes of BUF.  Returns NULL if the
   directory couldn't be determined or SIZE was too small.
   If successful, returns BUF.  In GNU, if BUF is NULL,
   an array is allocated with `malloc'; the array is SIZE
   bytes long, unless SIZE == 0, in which case it is as
   big as necessary.  */
extern char *__getcwd (char *__buf, size_t __size);

/* Move FD's file position to OFFSET bytes from the
   beginning of the file (if WHENCE is SEEK_SET),
   the current position (if WHENCE is SEEK_CUR),
   or the end of the file (if WHENCE is SEEK_END).
   Return the new file position.  */
#ifdef __REDIRECT
extern char * __REDIRECT (getcwd,
                    (char *buf, int size) __THROW,
                    __getcwd);
#else
# define getcwd __getcwd
#endif
#ifndef __USE_FILE_OFFSET64
extern __inline__ __off_t lseek (int __fd, __off_t __offset, int __whence) __THROW
{
	return (__off_t) _lseek (__fd, (long) __offset, __whence);
}
#else
//# ifdef __REDIRECT
//extern __off64_t __REDIRECT (lseek,
//                    (int __fd, __off64_t __offset, int __whence)
//                    __THROW,
//                    lseek64);
//# else
#  define lseek lseek64
//# endif
#endif
#ifdef __USE_LARGEFILE64
extern __inline__ __off64_t lseek64 (int __fd, __off64_t __offset, int __whence) __THROW
{
	return (__off64_t) _lseeki64 (__fd, (__int64) __offset, __whence);
}
#endif

/* pid_t        fork(void); */

int mkstemps(char *, int );

#ifdef	__cplusplus
}
#endif

#endif /* __GW32__ */

#endif /* __WINX_IOX_H__ */
