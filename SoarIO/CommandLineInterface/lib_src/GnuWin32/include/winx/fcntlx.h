#ifndef __WINX_FCNTLX_H__
#define __WINX_FCNTLX_H__
#ifdef __GW32__

#if defined(_FCNTL_H_) || defined(__STRICT_ANSI__)
# define _FCNTL_H
#endif /* _FCNTL_H_ */

#include <bits/fcntl.h>

/* bits/fcntl.h may define these different from MSVC */
#define   O_CREAT        _O_CREAT
#define   O_TRUNC        _O_TRUNC
#define   O_EXCL         _O_EXCL

#define O_LARGEFILE 0

#define F_OK 0
#ifdef X_OK
#undef X_OK
#endif
#define X_OK 1
#define W_OK 2
#define R_OK 4
#define RW_OK 6

#define L_SET 1 
#define L_INCR SEEK_CUR
#define L_XTND SEEK_END

#define valloc malloc

#ifdef __cplusplus
extern "C" {
#endif

/* Open FILE and return a new file descriptor for it, or -1 on error.
   OFLAG determines the type of access used.  If O_CREAT is on OFLAG,
   the third argument is taken as a `mode_t', the mode of the created file.  */
#ifdef __REDIRECT
extern int __REDIRECT (__open, (__const char *__file, int __oflag, ...) __THROW,
                 _open);
#else
# define __open _open
#endif
#ifdef __USE_LARGEFILE64
# ifdef __REDIRECT
extern int __REDIRECT (__open64, (__const char *__file, int __oflag, ...) __THROW,
                 _open);
extern int __REDIRECT (open64, (__const char *__file, int __oflag, ...) __THROW,
                 _open);
# else
#  define __open64 _open
#  define   open64 _open
# endif
#endif


#ifdef __cplusplus
}
#endif

#endif /* __GW32__ */

#endif /* __WINX_FCNTLX_H__ */
