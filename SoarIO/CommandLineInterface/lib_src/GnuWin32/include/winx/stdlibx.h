#ifndef _WINX_STDLIBX_H
#define _WINX_STDLIBX_H
#ifdef __GW32__

#include <features.h>
#include <gw32.h>
//#include <libiberty.h>

#define __environ _environ

#ifndef _IO
# define _IO(x,y)     (((x)<<8)|y)
#endif /* _IO */

#ifdef __cplusplus
extern "C" {
#endif

/*
int random (void);
long srandom(unsigned __seed);


#if defined(__cplusplus)
#define __STDLIB_H_USING_LIST
     using std::_argc;
     using std::_argv;
#endif
*/

/* Generate a unique temporary file name from TEMPLATE.
   The last six characters of TEMPLATE must be "XXXXXX";
   they are replaced with a string that makes the filename unique.
   Returns a file descriptor open on the file for reading and writing,
   or -1 if it cannot create a uniquely-named file.  */
extern int mkstemps (char *__template, int __suffix_len);
#ifndef __USE_FILE_OFFSET64
extern __inline__ int mkstemp (char *__template) __THROW
{
	return mkstemps	(__template, 0);
}
#else
# ifdef __REDIRECT
extern int __REDIRECT (mkstemp, (char *__template) __THROW, mkstemp64);
# else
#   define mkstemp mkstemp64
# endif
#endif
#ifdef __USE_LARGEFILE64
extern int mkstemp64 (char *__template) __THROW;
#endif

#define ecvtf ecvt
#define fcvtf fcvt
#define gcvtf gcvt
#define atoff atof
#define strtodf strtod

#define __alloca malloc
#define alloca __alloca

#define environ _environ

#include <stdint.h>

#ifndef NOMINMAX
# ifndef max
#  define max(a,b) ((a)>(b)?(a):(b))
# endif
# ifndef min
#  define min(a,b) ((a)<(b)?(a):(b))
# endif
#endif
#ifndef NOMINMAX
# ifndef MAX
#  define MAX(a,b) ((a)>(b)?(a):(b))
# endif
# ifndef MIN
#  define MIN(a,b) ((a)<(b)?(a):(b))
# endif
#endif

extern char *canonicalize_file_name (const char *name);
extern char *realpath (const char *name, char *resolved);
extern __inline__ char *resolvepath (const char *name, char *resolved, size_t bufsiz) __THROW
{
	return realpath (name, resolved);
}
#define getfullname realpath
#define getfullpath realpath

#ifdef __MSVCRT__
_CRTIMP __int64 _strtoi64(const char *nptr, char **endptr, int base);
_CRTIMP __int64 _wcstoi64(const wchar_t *nptr, wchar_t **endptr, int base);
_CRTIMP unsigned __int64 _strtoui64(const char *nptr, char **endptr, int base);
_CRTIMP unsigned __int64 _wcstoui64(const wchar_t *nptr, wchar_t **endptr, int base);
#endif /* __MSVCRT__ */

//#if defined __USE_ISOC99 || (defined __GLIBC_HAVE_LONG_LONG && defined __USE_MISC)
//__BEGIN_NAMESPACE_C99
///* Convert a string to a quadword integer.  */
//__extension__
//extern __inline__ long long int strtoll (__const char *__restrict __nptr,
//                     char **__restrict __endptr, int __base) __THROW
//{
//	return (long long int) _strtoi64 (__nptr, __endptr, __base);
//}
///* Convert a string to an unsigned quadword integer.  */
//__extension__
//extern __inline__ unsigned long long int strtoull (__const char *__restrict __nptr,
//                         char **__restrict __endptr, int __base) __THROW
//{
//	return (unsigned long long int) _strtoui64 (__nptr, __endptr, __base);
//}
//__END_NAMESPACE_C99
//#endif /* ISO C99 or GCC and use MISC.  */


/* Shorthand for type of comparison functions.  */
#ifndef __COMPAR_FN_T
# define __COMPAR_FN_T
typedef int (*__compar_fn_t) (__const void *, __const void *);

# ifdef   __USE_GNU
typedef __compar_fn_t comparison_fn_t;
# endif
#endif

__BEGIN_NAMESPACE_STD
/* Do a binary search for KEY in BASE, which consists of NMEMB elements
   of SIZE bytes each, using COMPAR to perform the comparisons.  */
extern void *bsearch (__const void *__key, __const void *__base,
                size_t __nmemb, size_t __size, __compar_fn_t __compar);

/* Sort NMEMB elements of BASE, of SIZE bytes each,
   using COMPAR to perform the comparisons.  */
extern void qsort (void *__base, size_t __nmemb, size_t __size,
             __compar_fn_t __compar);


#if defined __USE_SVID || defined __USE_XOPEN

/* Global state for non-reentrant functions.  Defined in drand48-iter.c.  */
extern struct drand48_data __libc_drand48_data;


/* These are the functions that actually do things.  The `random', `srandom',
   `initstate' and `setstate' functions are those from BSD Unices.
   The `rand' and `srand' functions are required by the ANSI standard.
   We provide both interfaces to the same random number generator.  */
/* Return a random long integer between 0 and RAND_MAX inclusive.  */
extern long int random (void) __THROW;

/* Seed the random number generator with the given number.  */
extern void srandom (unsigned int __seed) __THROW;

/* Initialize the random number generator to use state buffer STATEBUF,
   of length STATELEN, and seed it with SEED.  Optimal lengths are 8, 16,
   32, 64, 128 and 256, the bigger the better; values less than 8 will
   cause an error and values greater than 256 will be rounded down.  */
extern char *initstate (unsigned int __seed, char *__statebuf,
               size_t __statelen) __THROW;

/* Switch the random number generator to state buffer STATEBUF,
   which should have been previously initialized by `initstate'.  */
extern char *setstate (char *__statebuf) __THROW;


# ifdef __USE_MISC
/* Reentrant versions of the `random' family of functions.
   These functions all use the following data structure to contain
   state, rather than global state variables.  */

struct random_data
  {
    int32_t *fptr;       /* Front pointer.  */
    int32_t *rptr;       /* Rear pointer.  */
    int32_t *state;      /* Array of state values.  */
    int rand_type;       /* Type of random number generator.  */
    int rand_deg;        /* Degree of random number generator.  */
    int rand_sep;        /* Distance between front and rear.  */
    int32_t *end_ptr;         /* Pointer behind state table.  */
  };

extern int random_r (struct random_data *__restrict __buf,
               int32_t *__restrict __result) __THROW;

extern int srandom_r (unsigned int __seed, struct random_data *__buf) __THROW;

extern int initstate_r (unsigned int __seed, char *__restrict __statebuf,
               size_t __statelen,
               struct random_data *__restrict __buf) __THROW;

extern int setstate_r (char *__restrict __statebuf,
                 struct random_data *__restrict __buf) __THROW;
# endif   /* Use misc.  */
#endif    /* Use SVID || extended X/Open.  */


__BEGIN_NAMESPACE_STD
/* Return a random integer between 0 and RAND_MAX inclusive.  */
extern int rand (void) __THROW;
/* Seed the random number generator with the given number.  */
extern void srand (unsigned int __seed) __THROW;
__END_NAMESPACE_STD

#ifdef __USE_POSIX
/* Reentrant interface according to POSIX.1.  */
extern int rand_r (unsigned int *__seed) __THROW;
#endif


#if defined __USE_SVID || defined __USE_XOPEN
/* System V style 48-bit random number generator functions.  */

/* Return non-negative, double-precision floating-point value in [0.0,1.0).  */
extern double drand48 (void) __THROW;
extern double erand48 (unsigned short int __xsubi[3]) __THROW;

/* Return non-negative, long integer in [0,2^31).  */
extern long int lrand48 (void) __THROW;
extern long int nrand48 (unsigned short int __xsubi[3]) __THROW;

/* Return signed, long integers in [-2^31,2^31).  */
extern long int mrand48 (void) __THROW;
extern long int jrand48 (unsigned short int __xsubi[3]) __THROW;

/* Seed random number generator.  */
extern void srand48 (long int __seedval) __THROW;
extern unsigned short int *seed48 (unsigned short int __seed16v[3]) __THROW;
extern void lcong48 (unsigned short int __param[7]) __THROW;

# ifdef __USE_MISC
/* Data structure for communication with thread safe versions.  This
   type is to be regarded as opaque.  It's only exported because users
   have to allocate objects of this type.  */
struct drand48_data
  {
    unsigned short int __x[3];     /* Current state.  */
    unsigned short int __old_x[3]; /* Old state.  */
    unsigned short int __c;   /* Additive const. in congruential formula.  */
    unsigned short int __init;     /* Flag for initializing.  */
    unsigned long long int __a;    /* Factor in congruential formula.  */
  };

/* Return non-negative, double-precision floating-point value in [0.0,1.0).  */
extern int drand48_r (struct drand48_data *__restrict __buffer,
                double *__restrict __result) __THROW;
extern int erand48_r (unsigned short int __xsubi[3],
                struct drand48_data *__restrict __buffer,
                double *__restrict __result) __THROW;

/* Return non-negative, long integer in [0,2^31).  */
extern int lrand48_r (struct drand48_data *__restrict __buffer,
                long int *__restrict __result) __THROW;
extern int nrand48_r (unsigned short int __xsubi[3],
                struct drand48_data *__restrict __buffer,
                long int *__restrict __result) __THROW;

/* Return signed, long integers in [-2^31,2^31).  */
extern int mrand48_r (struct drand48_data *__restrict __buffer,
                long int *__restrict __result) __THROW;
extern int jrand48_r (unsigned short int __xsubi[3],
                struct drand48_data *__restrict __buffer,
                long int *__restrict __result) __THROW;

/* Seed random number generator.  */
extern int srand48_r (long int __seedval, struct drand48_data *__buffer)
     __THROW;

extern int seed48_r (unsigned short int __seed16v[3],
               struct drand48_data *__buffer) __THROW;

extern int lcong48_r (unsigned short int __param[7],
                struct drand48_data *__buffer) __THROW;
# endif   /* Use misc.  */
#endif    /* Use SVID or X/Open.  */


#ifdef __cplusplus
}
#endif

#endif /* __GW32__ */

#endif /* _WINX_STDLIBX_H */
