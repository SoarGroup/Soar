#ifndef _WINX_SYS_TIMESX_H_
#define _WINX_SYS_TIMESX_H_

#include <time.h>
#include <sys/timeb.h>

/*
 Operations on timevals.

 NB: timercmp does not work for >= or <=.
 */
#define	timerisset(tvp)		((tvp)->tv_sec || (tvp)->tv_usec)
#define	timercmp(tvp, uvp, cmp)	\
    (((tvp)->tv_sec == (uvp)->tv_sec && (tvp)->tv_usec cmp (uvp)->tv_usec) \
    || (tvp)->tv_sec cmp (uvp)->tv_sec)
#define	timerclear(tvp)		((tvp)->tv_sec = (tvp)->tv_usec = 0)

struct tms {
  clock_t tms_utime;
  clock_t tms_cstime;
  clock_t tms_cutime;
  clock_t tms_stime;
};

#endif /*_WINX_SYS_TIMESX_H_*/
