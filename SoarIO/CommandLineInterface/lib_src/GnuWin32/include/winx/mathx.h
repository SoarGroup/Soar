#ifndef _WINX_MATHX_H
# define _WINX_MATHX_H
#ifdef __GW32__

//#include <winx/bits/mathdefx.h>

#ifdef    __cplusplus
extern "C" {
#endif

#define M_E			2.7182818284590452354	/* e */
#define M_LOG2E		1.4426950408889634074	/* log_2 e */
#define M_LOG10E	0.43429448190325182765	/* log_10 e */
#define M_LN2		0.69314718055994530942	/* log_e 2 */
#define M_LN10		2.30258509299404568402	/* log_e 10 */
#define M_PI		3.14159265358979323846	/* pi */
#define M_PI_2		1.57079632679489661923	/* pi/2 */
#define M_PI_4		0.78539816339744830962	/* pi/4 */
#define M_1_PI		0.31830988618379067154	/* 1/pi */
#define M_2_PI		0.63661977236758134308	/* 2/pi */
#define M_2_SQRTPI	1.12837916709551257390	/* 2/sqrt(pi) */
#define M_SQRT2		1.41421356237309504880	/* sqrt(2) */
#define M_SQRT1_2	0.70710678118654752440	/* 1/sqrt(2) */
#define M_TWOPI     (M_PI * 2.0)
#define M_3PI_4		2.3561944901923448370E0
#define M_SQRTPI    1.77245385090551602792981
#define M_LN2LO     1.9082149292705877000E-10
#define M_LN2HI     6.9314718036912381649E-1
#define M_SQRT3   	1.73205080756887719000
#define M_IVLN10    0.43429448190325182765 /* 1 / log(10) */
#define M_LOG2_E    0.693147180559945309417
#define M_INVLN2    1.4426950408889633870E0  /* 1 / log(2) */
/* #define log2(x) (log (x) / M_LOG2_E) */

/* The above constants are not adequate for computation using `long double's.
   Therefore we provide as an extension constants with similar names as a
   GNU extension.  Provide enough digits for the 128-bit IEEE quad.  */
#define M_El		2.7182818284590452353602874713526625L  /* e */
#define M_LOG2El	1.4426950408889634073599246810018922L  /* log_2 e */
#define M_LOG10El	0.4342944819032518276511289189166051L  /* log_10 e */
#define M_LN2l		0.6931471805599453094172321214581766L  /* log_e 2 */
#define M_LN10l		2.3025850929940456840179914546843642L  /* log_e 10 */
#define M_PIl		3.1415926535897932384626433832795029L  /* pi */
#define M_PI_2l		1.5707963267948966192313216916397514L  /* pi/2 */
#define M_PI_4l		0.7853981633974483096156608458198757L  /* pi/4 */
#define M_1_PIl		0.3183098861837906715377675267450287L  /* 1/pi */
#define M_2_PIl		0.6366197723675813430755350534900574L  /* 2/pi */
#define M_2_SQRTPIl	1.1283791670955125738961589031215452L  /* 2/sqrt(pi) */
#define M_SQRT2l	1.4142135623730950488016887242096981L  /* sqrt(2) */
#define M_SQRT1_2l	0.7071067811865475244008443621048490L  /* 1/sqrt(2) */

#ifndef HUGE
#define HUGE		HUGE_VAL
#endif

#include <sys/types.h>
#include <stdint.h>

/* Get the architecture specific values describing the floating-point
   evaluation.  The following symbols will get defined:

    float_t	floating-point type at least as wide as `float' used
		to evaluate `float' expressions
    double_t	floating-point type at least as wide as `double' used
		to evaluate `double' expressions

    FLT_EVAL_METHOD
		Defined to
		  0	if `float_t' is `float' and `double_t' is `double'
		  1	if `float_t' and `double_t' are `double'
		  2	if `float_t' and `double_t' are `long double'
		  else	`float_t' and `double_t' are unspecified

    INFINITY	representation of the infinity value of type `float'

    FP_FAST_FMA
    FP_FAST_FMAF
    FP_FAST_FMAL
		If defined it indicates that the `fma' function
		generally executes about as fast as a multiply and an add.
		This macro is defined only iff the `fma' function is
		implemented directly with a hardware multiply-add instructions.

    FP_ILOGB0	Expands to a value returned by `ilogb (0.0)'.
    FP_ILOGBNAN	Expands to a value returned by `ilogb (NAN)'.

    DECIMAL_DIG	Number of decimal digits supported by conversion between
		decimal and all internal floating-point formats.

*/

/* All floating-point numbers can be put in one of these categories.  */
//enum
//  {
//    FP_NAN,
//# define FP_NAN FP_NAN
//    FP_INFINITE,
//# define FP_INFINITE FP_INFINITE
//    FP_ZERO,
//# define FP_ZERO FP_ZERO
//    FP_SUBNORMAL,
//# define FP_SUBNORMAL FP_SUBNORMAL
//    FP_NORMAL
//# define FP_NORMAL FP_NORMAL
//  };

/* Bitmasks for the math_errhandling macro.  */
# define MATH_ERRNO	1	/* errno set by math functions.  */
# define MATH_ERREXCEPT	2	/* Exceptions raised by math functions.  */

/* Support for various different standard error handling behaviors.  */
enum __fdlibm_version
{
  __fdlibm_ieee = -1, /* According to IEEE 754/IEEE 854.  */
  __fdlibm_svid,	  /* According to System V, release 4.  */
  __fdlibm_xopen,	  /* Nowadays also Unix98.  */
  __fdlibm_posix,
  __fdlibm_isoc       /* Actually this is ISO C99.  */
};

#define _LIB_VERSION_TYPE enum __fdlibm_version
#define _LIB_VERSION __fdlib_version

/* if global variable _LIB_VERSION is not desirable, one may 
 * change the following to be a constant by: 
 *	#define _LIB_VERSION_TYPE const enum version
 * In that case, after one initializes the value _LIB_VERSION (see
 * s_lib_version.c) during compile time, it cannot be modified
 * in the middle of a program
 */ 
extern  _LIB_VERSION_TYPE  _LIB_VERSION;

#define _IEEE_  __fdlibm_ieee
#define _SVID_  __fdlibm_svid
#define _XOPEN_ __fdlibm_xopen
#define _POSIX_ __fdlibm_posix

union __dmath
{
  uint32_t i[2];
  double d;
};

/* The exception structure passed to the matherr routine.  */
#ifdef __cplusplus
struct __exception 
#else
struct exception 
#endif
{
  int type;
  char *name;
  double arg1;
  double arg2;
  double retval;
  int err;
};

#define complex _complex

float atanf (float);
float cosf (float);
float sinf (float);
float tanf (float);
float tanhf (float);
float frexpf (float, int *);
float modff (float, float *);
float ceilf (float);
float fabsf (float);
float floorf (float);
float acosf (float);
float asinf (float);
float atan2f (float, float);
float coshf (float);
float sinhf (float);
float expf (float);
float ldexpf (float, int);
float logf (float);
float log10f (float);
float powf (float, float);
float sqrtf (float);
float fmodf (float, float);
float infinityf (void);
//float nanf (void);
int isnanf (float);
int isinff (float);
int finitef (float);
float copysignf (float, float);
int ilogbf (float);

float asinhf (float);
float cbrtf (float);
float nextafterf (float, float);
float rintf (float);
float scalbnf (float, int);
float log1pf (float);
float expm1f (float);
float acoshf (float);
float atanhf (float);
float remainderf (float, float);
float gammaf (float);
float gammaf_r (float, int *);
float lgammaf (float);
float lgammaf_r (float, int *);
float erff (float);
float erfcf (float);
float y0f (float);
float y1f (float);
float ynf (int, float);
float j0f (float);
float j1f (float);
float jnf (int, float);
/* #define log2f(x) (logf (x) / (float) M_LOG2_E) */ 
float hypotf (float, float);


float cabsf();
float dremf (float, float);
/*
double cabs (struct _complex);
double cabs ();
*/
double log1p (double);
double expm1 (double);
double acosh (double);
double asinh (double);
double atanh (double);
double remainder (double, double);
double gamma (double);
double gamma_r (double, int *);
double lgamma (double);
double lgamma_r (double, int *);
double erf (double);
double erfc (double);
double infinity (void);
//double nan (void);
//int isinf (double);
//int isnan (double);
int finite (double);

double log2 (double);
float log2f (float );

/* sign of the result of the gamma functions */
/* int signgam; */

/* ISO C99 defines some macros to compare number while taking care
   for unordered numbers.  Since many FPUs provide special
   instructions to support these operations and these tests are
   defined in <bits/mathinline.h>, we define the generic macros at
   this late point and only if they are not defined yet.  */

/* Return nonzero value if X is greater than Y.  */
# ifndef isgreater
#  define isgreater(x, y) \
  (__extension__							      \
   ({ __typeof__(x) __x = (x); __typeof__(y) __y = (y);			      \
      !isunordered (__x, __y) && __x > __y; }))
# endif

/* Return nonzero value if X is greater than or equal to Y.  */
# ifndef isgreaterequal
#  define isgreaterequal(x, y) \
  (__extension__							      \
   ({ __typeof__(x) __x = (x); __typeof__(y) __y = (y);			      \
      !isunordered (__x, __y) && __x >= __y; }))
# endif

/* Return nonzero value if X is less than Y.  */
# ifndef isless
#  define isless(x, y) \
  (__extension__							      \
   ({ __typeof__(x) __x = (x); __typeof__(y) __y = (y);			      \
      !isunordered (__x, __y) && __x < __y; }))
# endif

/* Return nonzero value if X is less than or equal to Y.  */
# ifndef islessequal
#  define islessequal(x, y) \
  (__extension__							      \
   ({ __typeof__(x) __x = (x); __typeof__(y) __y = (y);			      \
      !isunordered (__x, __y) && __x <= __y; }))
# endif

/* Return nonzero value if either X is less than Y or Y is less than X.  */
# ifndef islessgreater
#  define islessgreater(x, y) \
  (__extension__							      \
   ({ __typeof__(x) __x = (x); __typeof__(y) __y = (y);			      \
      !isunordered (__x, __y) && (__x < __y || __y < __x); }))
# endif

/* Return nonzero value if arguments are unordered.  */
# ifndef isunordered
#  define isunordered(u, v) \
  (__extension__							      \
   ({ __typeof__(u) __u = (u); __typeof__(v) __v = (v);			      \
      fpclassify (__u) == FP_NAN || fpclassify (__v) == FP_NAN; }))
# endif


#ifndef NOMINMAX
#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif
#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif
#endif

#ifdef    __cplusplus
}
#endif

#endif /* __GW32__ */

#endif /* _WINX_MATHX_H */
