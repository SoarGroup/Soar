#ifndef _WINX_STRINGX_H
#define _WINX_STRINGX_H
#ifdef __GW32__

#include <features.h>

/* char*	_strerror (const char *); */
/* char*	_strerror (int); */
/*
#ifndef index
#define index(s,c) strchr(s,c)
#endif
#ifndef rindex
#define rindex(s,c) strrchr(s,c)
#endif
*/
#ifndef    __cplusplus
extern __inline__ int index (__const char *s, int c) __THROW
{
  return strchr (s, c);
}
extern __inline__ int rindex (__const char *s, int c) __THROW
{
  return strrchr (s, c);
}
#endif /* __cplusplus */

#ifndef __strdup
# define __strdup _strdup
#endif

#ifndef __memcpy
# define __memcpy _memcpy
#endif

#ifndef strcasecmp
# define strcasecmp _stricmp
#endif

#ifndef __strcasecmp
# define __strcasecmp _stricmp
#endif

#ifndef strcasencmp
# define strcasencmp _strnicmp
#endif

#ifdef    __cplusplus
extern "C" {
#endif

#ifdef __USE_GNU
/* Find the first occurrence of NEEDLE in HAYSTACK.
   NEEDLE is NEEDLELEN bytes long;
   HAYSTACK is HAYSTACKLEN bytes long.  */
extern void *memmem (__const void *__haystack, size_t __haystacklen,
               __const void *__needle, size_t __needlelen)
     __THROW __attribute_pure__;

/* Copy N bytes of SRC to DEST, return pointer to bytes after the
   last written byte.  */
extern void *__mempcpy (void *__restrict __dest,
               __const void *__restrict __src, size_t __n) __THROW;
extern void *mempcpy (void *__restrict __dest,
                __const void *__restrict __src, size_t __n) __THROW;
#endif

/* Divide S into tokens separated by characters in DELIM.  Information
   passed between calls are stored in SAVE_PTR.  */
extern char *__strtok_r (char *__restrict __s,
                __const char *__restrict __delim,
                char **__restrict __save_ptr) __THROW;
#if defined __USE_POSIX || defined __USE_MISC
extern char *strtok_r (char *__restrict __s, __const char *__restrict __delim,
                 char **__restrict __save_ptr) __THROW;
#endif


#ifdef __MSVCRT__
_CRTIMP int  _strncoll(const char*, const char*, size_t);
_CRTIMP int  _strnicoll(const char*, const char*, size_t);
#endif

#ifdef __MSVCRT__
_CRTIMP int  _wcsncoll(const wchar_t*, const wchar_t*, size_t);
_CRTIMP int  _wcsnicoll(const wchar_t*, const wchar_t*, size_t);
#endif

extern char * __strsep (char ** , const char * );

/* Duplicate S, returning an identical alloca'd string.  */
# define strdupa(s)							      \
  (__extension__							      \
    ({									      \
      __const char *__old = (s);					      \
      size_t __len = strlen (__old) + 1;				      \
      char *__new = (char *) __builtin_alloca (__len);			      \
      (char *) memcpy (__new, __old, __len);				      \
    }))

/* Return an alloca'd copy of at most N bytes of string.  */
# define strndupa(s, n)							      \
  (__extension__							      \
    ({									      \
      __const char *__old = (s);					      \
      size_t __len = strnlen (__old, (n));				      \
      char *__new = (char *) __builtin_alloca (__len + 1);		      \
      __new[__len] = '\0';						      \
      (char *) memcpy (__new, __old, __len);				      \
    }))

#ifdef    __USE_GNU
/* Search in S for C.  This is similar to `memchr' but there is no
   length limit.  */
extern void *rawmemchr (__const void *__s, int __c) __THROW __attribute_pure__;

/* Compare S1 and S2 as strings holding name & indices/version numbers.  */
extern int strverscmp (__const char *__s1, __const char *__s2)
     __THROW __attribute_pure__;

/* Return a malloc'd copy of at most N bytes of STRING.  The
   resultant string is terminated even if no null terminator
   appears before STRING[N].  */
extern char *strndup (__const char *__string, size_t __n)
     __THROW __attribute_malloc__;

	 /* Find the length of STRING, but scan at most MAXLEN characters.
   If no '\0' terminator is found in that many characters, return MAXLEN.  */
extern size_t strnlen (__const char *__string, size_t __maxlen)
     __THROW __attribute_pure__;

#endif	/* __USE_GNU */

#ifdef    __cplusplus
}
#endif

#endif /* __GW32__ */

#endif /* _WINX_STRINGX_H */
