#ifndef __WINX_WINDEFX_H__
#define __WINX_WINDEFX_H__
#ifdef __GW32__

#include <limits.h>

//#define MAKEDWORDLONG(a,b) \
//    (DWORDLONG) ((a) + UInt32x32To64((b), (DWORDLONG) ULONG_MAX+1))
#define LODWORD(l)   ((DWORD)((DWORDLONG)(l)))
#define HIDWORD(l)   ((DWORD)(((DWORDLONG)(l)>>32)&0xFFFFFFFF))

#define MAKEDWORD(a,b)    ((DWORD)(((WORD)(a))|(((DWORD)((WORD)(b)))<<16)))
#define MAKEDWORDLONG(a,b)    ((DWORDLONG)(((DWORD)(a))|(((DWORDLONG)((DWORD)(b)))<<32)))

typedef DWORD   *LPCOLORREF;

#endif /* __GW32__ */

#endif /* __WINX_WINDEFX_H__ */
