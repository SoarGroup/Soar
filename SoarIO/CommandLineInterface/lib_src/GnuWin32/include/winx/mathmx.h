#ifndef __WINX_MATHMX_H__
#define __WINX_MATHMX_H__
#ifdef __GW32__

#include <sys/types.h>
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

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
float nanf (void);
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
#define log2f(x) (logf (x) / (float) M_LOG2_E)
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
double nan (void);
int isinf (double);
int isnan (double);
int finite (double);

union __dmath
{
  uint32_t i[2];
  double d;
};

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

extern int signgam;

enum __fdlibm_version
{
  __fdlibm_ieee = -1,
  __fdlibm_svid,
  __fdlibm_xopen,
  __fdlibm_posix
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

__MINGW_IMPORT double	_HUGE;
#define	HUGE_VAL	_HUGE

#include <winx/mathx.h>

#ifdef __cplusplus
}
#endif

#endif /* __GW32__ */

#endif /* __WINX_MATHMX_H__ */
