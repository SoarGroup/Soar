#ifndef __WINX_STDIOX_H__
#define __WINX_STDIOX_H__
#ifdef __GW32__

#include <features.h>
//#include <winx/unistdx.h> /* for large-file support */
#include <sys/types.h> 
#include <stdio_ext.h> 

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MAX_PATH
# define MAX_PATH 260
#endif /* MAX_PATH */

#ifdef L_tmpnam
# undef L_tmpnam
#endif
#define L_tmpnam MAX_PATH

#ifndef P_tmpdir
# define P_tmpdir (getenv("TEMP"))
#endif
#ifdef _P_tmpdir
# undef _P_tmpdir
# define _P_tmpdir P_tmpdir
#endif
#ifndef wP_tmpdir
# define wP_tmpdir  (_wgetenv("TEMP"))
#endif
#ifdef _wP_tmpdir
# undef _wP_tmpdir
# define _wP_tmpdir wP_tmpdir
#endif

#ifndef __FILE
# define __FILE FILE
#endif

#define snprintf _snprintf
#define __snprintf _snprintf
#define vsnprintf  _vsnprintf
#define __vsnprintf  _vsnprintf

#define ISDIRSEP(c) ((c) == '/' || (c) == '\\')

#ifdef __REDIRECT
extern FILE * __REDIRECT (__fdopen, (int __fd, __const char *__modes) __THROW,
                 _fdopen);
#else
# define __fdopen _fdopen
#endif

/* Flush all line-buffered files.  */
extern void __REDIRECT (_flushlbf, (void) __THROW,
                 _flushall);
extern void __REDIRECT (flushall, (void) __THROW,
                 _flushall);
/* Return non-zero value iff the stream FP is line-buffered.  */
extern __inline__ int __flbf (FILE *__fp) __THROW
{
	return (int) (__fp->_flag & _IOLBF);
}
/* Return amount of output in bytes pending on a stream FP.  */
extern __inline__ size_t __fpending (FILE *__fp) __THROW
{ 
	return (size_t) (__fp->_ptr - __fp->_base);
}
/* Return the size of the buffer of FP in bytes currently in use by
   the given stream.  */
extern __inline__ size_t __fbufsize (FILE *__fp) __THROW
{
	return (size_t) (__fp->_bufsiz);
}

__BEGIN_NAMESPACE_STD
/* If BUF is NULL, make STREAM unbuffered.
   Else make it use buffer BUF, of size BUFSIZ.  */
extern __inline__ void setbuf (FILE *__restrict __stream, char *__restrict __buf) __THROW
{
	(__buf == NULL ? setvbuf(__stream, __buf, _IONBF, BUFSIZ)  \
	: setvbuf(__stream, __buf, _IOFBF, BUFSIZ));
}
__END_NAMESPACE_STD

#ifdef    __USE_BSD
/* If BUF is NULL, make STREAM unbuffered.
   Else make it use SIZE bytes of BUF for buffering.  */
extern  __inline__ void setbuffer (FILE *__restrict __stream, char *__restrict __buf,
                 size_t __size) __THROW
{
	(__buf == NULL ? setvbuf(__stream, __buf, _IONBF, __size)  \
	: setvbuf(__stream, __buf, _IOFBF, __size));
}
/* Make STREAM line-buffered.  */
extern __inline__ void setlinebuf (FILE *__stream) __THROW
{
	setvbuf(__stream, NULL, _IOLBF, 512);
}
#endif
extern __inline__ int __validfp (FILE *__stream) __THROW
{
	return 	__stream != NULL;
}

#define fflush_unlocked fflush
#define fgets_unlocked fgets
#define fwrite_unlocked fwrite
#define putc_unlocked putc
#define fputs_unlocked fputs


#ifdef __USE_LARGEFILE64
extern __inline__ FILE *fopen64 (__const char *__restrict __filename,
                __const char *__restrict __modes) __THROW
{
	return fopen (__filename, __modes);
}
extern  __inline__ FILE *freopen64 (__const char *__restrict __filename,
               __const char *__restrict __modes,
               FILE *__restrict __stream) __THROW
{
	return freopen (__filename, __modes, __stream);
}
#endif



/* The Single Unix Specification, Version 2, specifies an alternative,
   more adequate interface for the two functions above which deal with
   file offset.  `long int' is not the right type.  These definitions
   are originally defined in the Large File Support API.  */

#ifdef __USE_LARGEFILE
# define fseek fseeko
# define ftell ftello
# ifndef __USE_FILE_OFFSET64
/* Seek to a certain position on STREAM.  */
extern int fseeko (FILE *__stream, __off_t __off, int __whence) __THROW;
/* Return the current position of STREAM.  */
extern __off_t ftello (FILE *__stream) __THROW;
# else
#  ifdef __REDIRECT
extern int __REDIRECT (fseeko,
                 (FILE *__stream, __off64_t __off, int __whence) __THROW,
                 fseeko64);
extern __off64_t __REDIRECT (ftello, (FILE *__stream) __THROW, ftello64);
#  else
#   define fseeko fseeko64
#   define ftello ftello64
#  endif
# endif
#endif


#ifdef __USE_LARGEFILE64
# define _G_LSEEK64 1
typedef fpos_t fpos64_t;
extern int fseeko64 (FILE *__stream, __off64_t __off, int __whence) __THROW;
extern __off64_t ftello64 (FILE *__stream) __THROW;
#  ifdef __REDIRECT
extern __inline__ int __REDIRECT (fgetpos64,
		(FILE *__restrict __stream, fpos64_t *__restrict __pos) __THROW,
		fgetpos);
extern __inline__ int __REDIRECT (fsetpos64,
		(FILE *__stream, __const fpos64_t *__pos) __THROW,
		fsetpos);
#  else
#   define fgetpos64 fgetpos
#   define fsetpos64 fsetpos
#  endif
#endif

extern __const char *__const _sys_errlist_internal[];
extern int _sys_nerr_internal ;

#ifdef __USE_LARGEFILE64
extern FILE *tmpfile64 (void) __THROW;
#endif

#ifdef __USE_MISC
/* This is the reentrant variant of `tmpnam'.  The only difference is
   that it does not allow S to be NULL.  */
extern char *tmpnam_r (char *__s) __THROW;
#endif

#define __need_size_t
#include <stddef.h>
/* Generate a unique file name (and possibly open it).  */
extern int __path_search (char *__tmpl, size_t __tmpl_len,
                 __const char *__dir, __const char *__pfx,
                 int __try_tempdir);

extern int __gen_tempname (char *__tmpl, int __kind);
/* The __kind argument to __gen_tempname may be one of: */
#define __GT_FILE 0    /* create a file */
#define __GT_BIGFILE   1    /* create a file, using open64 */
#define __GT_DIR  2    /* create a directory */
#define __GT_NOCREATE  3    /* just find a name not currently in use */

#ifndef  __cplusplus
# define feof(__F)     ((__F)->_flag & _IOEOF)
# define ferror(__F)   ((__F)->_flag & _IOERR)
# define _fileno(__F)  ((__F)->_file)
# define getc(__F)     (--(__F)->_cnt >= 0 \
                ? 0xFF & *(__F)->_ptr++ : _filbuf(__F))
# define putc(__c,__F)  (--(__F)->_cnt >= 0 \
                ? 0xFF & (*(__F)->_ptr++ = (char)(__c)) :  _flsbuf((__c),(__F)))
# define getchar()         getc(stdin)
# define putchar(__c)       putc((__c),stdout)

# ifdef  _MT
#  undef  getc
#  undef  putc
#  undef  getchar
#  undef  putchar
# endif

#endif /* __cplusplus */


/* libc_hidden_proto(perror) */

#ifdef __cplusplus
}
#endif

#endif /* __GW32__ */

#endif /* __WINX_STDIOX_H__ */
