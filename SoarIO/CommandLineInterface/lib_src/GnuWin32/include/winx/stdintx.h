#ifndef __WINX_STDINTX_H__
#define __WINX_STDINTX_H__
#ifdef __GW32__

#ifndef   _BITS_TYPES_H

/*
typedef int8_t __int8_t;
typedef uint8_t __uint8_t;
typedef int16_t __int16_t;
typedef uint16_t __uint16_t;
typedef int32_t __int32_t;
typedef uint32_t __uint32_t;
typedef int64_t __int64_t;
typedef uint64_t __uint64_t;

typedef uint8_t u_int8_t;
typedef uint16_t u_int16_t;
typedef uint32_t u_int32_t;
typedef uint64_t u_int64_t;

*/
#define __BIT_TYPES_DEFINED__  1

#ifndef NOMINMAX
#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif
#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif
#endif

#ifdef __intptr_t_defined
# define _INTPTR_T_DEFINED
# define _UINTPTR_T_DEFINED
#endif /* __intptr_t_defined */

#endif  /* _BITS_TYPES_H */

#endif /* __GW32__ */

#endif /* __WINX_STDINTX_H__ */
