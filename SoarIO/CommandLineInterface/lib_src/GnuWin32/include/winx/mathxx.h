#ifdef __GW32__
///* Return number of classification appropriate for X.  */
//# ifdef __NO_LONG_DOUBLE_MATH
//#  define fpclassify(x) \
//     (sizeof (x) == sizeof (float) ? __fpclassifyf (x) : __fpclassify (x))
//# else
//#  define fpclassify(x) \
//     (sizeof (x) == sizeof (float)					      \
//      ? __fpclassifyf (x)						      \
//      : sizeof (x) == sizeof (double)					      \
//      ? __fpclassify (x) : __fpclassifyl (x))
//# endif
//
///* Return nonzero value if sign of X is negative.  */
//# ifdef __NO_LONG_DOUBLE_MATH
//#  define signbit(x) \
//     (sizeof (x) == sizeof (float) ? __signbitf (x) : __signbit (x))
//# else
//#  define signbit(x) \
//     (sizeof (x) == sizeof (float)					      \
//      ? __signbitf (x)							      \
//      : sizeof (x) == sizeof (double)					      \
//      ? __signbit (x) : __signbitl (x))
//# endif
//
///* Return nonzero value if X is not +-Inf or NaN.  */
//# ifdef __NO_LONG_DOUBLE_MATH
//#  define isfinite(x) \
//     (sizeof (x) == sizeof (float) ? __finitef (x) : __finite (x))
//# else
//#  define isfinite(x) \
//     (sizeof (x) == sizeof (float)					      \
//      ? __finitef (x)							      \
//      : sizeof (x) == sizeof (double)					      \
//      ? __finite (x) : __finitel (x))
//# endif
//
///* Return nonzero value if X is neither zero, subnormal, Inf, nor NaN.  */
//# define isnormal(x) (fpclassify (x) == FP_NORMAL)
//
///* Return nonzero value if X is a NaN.  We could use `fpclassify' but
//   we already have this functions `__isnan' and it is faster.  */
//# ifdef __NO_LONG_DOUBLE_MATH
//#  define isnan(x) \
//     (sizeof (x) == sizeof (float) ? __isnanf (x) : __isnan (x))
//# else
//#  define isnan(x) \
//     (sizeof (x) == sizeof (float)					      \
//      ? __isnanf (x)							      \
//      : sizeof (x) == sizeof (double)					      \
//      ? __isnan (x) : __isnanl (x))
//# endif
//
///* Return nonzero value is X is positive or negative infinity.  */
//# ifdef __NO_LONG_DOUBLE_MATH
//#  define isinf(x) \
//     (sizeof (x) == sizeof (float) ? __isinff (x) : __isinf (x))
//# else
//#  define isinf(x) \
//     (sizeof (x) == sizeof (float)					      \
//      ? __isinff (x)							      \
//      : sizeof (x) == sizeof (double)					      \
//      ? __isinf (x) : __isinfl (x))
//# endif

#endif /* __GW32__ */
