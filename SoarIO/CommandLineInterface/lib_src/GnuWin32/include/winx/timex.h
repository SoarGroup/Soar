#ifndef _WINX_TIMEX_H_
#define _WINX_TIMEX_H_

#ifdef __GW32__

#ifndef   _TIME_H
# define _TIME_H    1
# include <features.h>
#endif

#include <bits/time.h>
#include <winx/stdlibx.h>
#include <winx/windowsx.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FILETIME_1970		(116444736000000000LL)	/* 100-nanoseconds between 1/1/1601 and 1/1/1970 */
#define ft70sec				(11644473600)			/* seconds between 1/1/1601 and 1/1/1970 */
#define ft8070sec			(315532800)				/* seconds between 1/1/1970 and 1/1/1980 */
#define ft80sec				(ft70sec + ft8070sec) 	/* seconds between 1/1/1601 and 1/1/1980 */

/*
#define tzname _tzname
#define __tzname _tzname
*/

#ifdef CLOCKS_PER_SEC
# undef	CLOCKS_PER_SEC
# define	CLOCKS_PER_SEC	(1000000)
# define   CLK_TCK        CLOCKS_PER_SEC
#endif /* CLOCKS_PER_SEC */

/* 1 sec = 1000 millisec = 1,000,000 microsec = 1,000,000,000 nanosec = 10,000,000 100-nanosec 
   1 microsec = 1000 nanosec = 10 100-nanosec
   100 nanosec = 0.1 microsec
*/
#define MILLISEC_PER_SEC		(1000)
#define MICROSEC_PER_SEC		(1000000)
#define NANOSEC_PER_SEC			(1000000000)
#define HECTONANOSEC_PER_SEC	(10000000)
#define NANOSEC_PER_MILLISEC	(1000000)
#define NANOSEC_PER_MICROSEC	(1000)

#define UM_FILETIME_TO_TIME(FT)	\
	((__time_t)	(	(MAKEDWORDLONG (FT.dwLowDateTime, FT.dwHighDateTime) \
						- ((DWORDLONG) FILETIME_1970) \
					) / HECTONANOSEC_PER_SEC \
				) \
	)
#define FILETIME_TO_TIME(FT) MAX(0,UM_FILETIME_TO_TIME(FT))

/*
 * A type for measuring processor time (in clock ticks).
 */
typedef   int64_t clock_t;
#define _CLOCK_T_DEFINED

#if !defined __clockid_t_defined && \
   ((defined _TIME_H && defined __USE_POSIX199309) || defined __need_clockid_t)
# define __clockid_t_defined  1

# include <bits/types.h>

/* Clock ID used in clock and timer functions.  */
typedef __clockid_t clockid_t;

#endif /* clockid_t not defined and <time.h> or need clockid_t.  */
#undef    __clockid_time_t

#if !defined __timer_t_defined && \
    ((defined _TIME_H && defined __USE_POSIX199309) || defined __need_timer_t)
# define __timer_t_defined    1

# include <bits/types.h>

/* Timer ID returned by `timer_create'.  */
typedef __timer_t timer_t;

#endif /* timer_t not defined and <time.h> or need timer_t.  */
#undef    __need_timer_t


/*
  time structures
  */

#if !defined __timespec_defined &&                \
    ((defined _TIME_H &&                     \
      (defined __USE_POSIX199309 || defined __USE_MISC)) || \
      defined __need_timespec)
# define __timespec_defined   1
# define HAVE_STRUCT_TIMESPEC 1

/* POSIX.1b structure for a time value.  This is like a `struct timeval' but
   has nanoseconds instead of microseconds.  */
struct timespec
  {
    __time_t tv_sec;          /* Seconds.  */
    long int tv_nsec;         /* Nanoseconds.  */
  };

#endif /* timespec not defined and <time.h> or need timespec.  */
#undef    __need_timespec
#ifdef __USE_POSIX199309
/* POSIX.1b structure for timer start values and intervals.  */
struct itimerspec
  {
    struct timespec it_interval;
    struct timespec it_value;
  };
#endif    /* POSIX.1b */

/* needed for timeval */
#ifdef _TIMEVAL_DEFINED /* also in winsock[2].h */
# undef __TIMEVAL__ 
# define __TIMEVAL__ 1
# undef _STRUCT_TIMEVAL
# define _STRUCT_TIMEVAL     1
#endif /* _TIMEVAL_DEFINED */

#if !defined(_TIMEVAL_DEFINED) && !defined(_STRUCT_TIMEVAL) /* also in sys/time.h */
#define _TIMEVAL_DEFINED
#define _STRUCT_TIMEVAL     1
struct timeval {
     long    tv_sec;
     long    tv_usec;
};
#define timerisset(tvp)   ((tvp)->tv_sec || (tvp)->tv_usec)
#define timercmp(tvp, uvp, cmp) \
     (((tvp)->tv_sec != (uvp)->tv_sec) ? \
     ((tvp)->tv_sec cmp (uvp)->tv_sec) : \
     ((tvp)->tv_usec cmp (uvp)->tv_usec))
#define timerclear(tvp)   (tvp)->tv_sec = (tvp)->tv_usec = 0
#endif /* _TIMEVAL_DEFINED */

/* Return the `struct tm' representation of *TIMER in local time,
   using *TP to store the result.  */
extern struct tm *__localtime_r (__const time_t *__restrict __timer,
                      struct tm *__restrict __tp) __THROW;

# ifdef __USE_XOPEN
/* Parse S according to FORMAT and store binary time information in TP.
   The return value is a pointer to the first unparsed character in S.  */
extern char *strptime (__const char *__restrict __s,
                 __const char *__restrict __fmt, struct tm *__tp)
     __THROW;
# endif

# ifdef __USE_XOPEN_EXTENDED
/* Set to one of the following values to indicate an error.
     1  the DATEMSK environment variable is null or undefined,
     2  the template file cannot be opened for reading,
     3  failed to get file status information,
     4  the template file is not a regular file,
     5  an error is encountered while reading the template file,
     6  memory allication failed (not enough memory available),
     7  there is no line in the template that matches the input,
     8  invalid input specification Example: February 31 or a time is
        specified that can not be represented in a time_t (representing
     the time in seconds since 00:00:00 UTC, January 1, 1970) */
extern int getdate_err;

/* Parse the given string as a date specification and return a value
   representing the value.  The templates from the file identified by
   the environment variable DATEMSK are used.  In case of an error
   `getdate_err' is set.  */
extern struct tm *getdate (__const char *__string) __THROW;
# endif

/* Make the process sleep for SECONDS seconds, or until a signal arrives
   and is not ignored.  The function returns the number of seconds less
   than SECONDS which it actually slept (thus zero if it slept the full time).
   If a signal handler does a `longjmp' or modifies the handling of the
   SIGALRM signal while inside `sleep' call, the handling of the SIGALRM
   signal afterwards is undefined.  There is no return value to indicate
   error, but if `sleep' returns SECONDS, it probably didn't work.  */
extern unsigned int __sleep (unsigned int __seconds) __THROW;
extern unsigned int sleep (unsigned int __seconds) __THROW;

# ifdef __USE_POSIX199309
/* Pause execution for a number of nanoseconds.  */
extern int nanosleep (__const struct timespec *__requested_time,
                struct timespec *__remaining) __THROW;


/* Get resolution of clock CLOCK_ID.  */
extern int clock_getres (clockid_t __clock_id, struct timespec *__res) __THROW;

/* Get current value of clock CLOCK_ID and store it in TP.  */
extern int clock_gettime (clockid_t __clock_id, struct timespec *__tp) __THROW;

/* Set clock CLOCK_ID to value TP.  */
extern int clock_settime (clockid_t __clock_id, __const struct timespec *__tp)
     __THROW;

#  ifdef __USE_XOPEN2K
/* High-resolution sleep with the specified clock.  */
extern int clock_nanosleep (clockid_t __clock_id, int __flags,
                   __const struct timespec *__req,
                   struct timespec *__rem) __THROW;

/* Return clock ID for CPU-time clock.  */
extern int clock_getcpuclockid (pid_t __pid, clockid_t *__clock_id) __THROW;
#  endif


/* Create new per-process timer using CLOCK_ID.  */
extern int timer_create (clockid_t __clock_id,
                struct sigevent *__restrict __evp,
                timer_t *__restrict __timerid) __THROW;

/* Delete timer TIMERID.  */
extern int timer_delete (timer_t __timerid) __THROW;

/* Set timer TIMERID to VALUE, returning old value in OVLAUE.  */
extern int timer_settime (timer_t __timerid, int __flags,
                 __const struct itimerspec *__restrict __value,
                 struct itimerspec *__restrict __ovalue) __THROW;

/* Get current value of timer TIMERID and store it in VLAUE.  */
extern int timer_gettime (timer_t __timerid, struct itimerspec *__value)
     __THROW;

/* Get expiration overrun for timer TIMERID.  */
extern int timer_getoverrun (timer_t __timerid) __THROW;
# endif


# ifdef __USE_XOPEN_EXTENDED
/* Set to one of the following values to indicate an error.
     1  the DATEMSK environment variable is null or undefined,
     2  the template file cannot be opened for reading,
     3  failed to get file status information,
     4  the template file is not a regular file,
     5  an error is encountered while reading the template file,
     6  memory allication failed (not enough memory available),
     7  there is no line in the template that matches the input,
     8  invalid input specification Example: February 31 or a time is
        specified that can not be represented in a time_t (representing
     the time in seconds since 00:00:00 UTC, January 1, 1970) */
extern int getdate_err;

/* Parse the given string as a date specification and return a value
   representing the value.  The templates from the file identified by
   the environment variable DATEMSK are used.  In case of an error
   `getdate_err' is set.  */
extern struct tm *getdate (__const char *__string) __THROW;
# endif

# ifdef __USE_GNU
/* Since `getdate' is not reentrant because of the use of `getdate_err'
   and the static buffer to return the result in, we provide a thread-safe
   variant.  The functionality is the same.  The result is returned in
   the buffer pointed to by RESBUFP and in case of an error the return
   value is != 0 with the same values as given above for `getdate_err'.  */
extern int getdate_r (__const char *__restrict __string,
                struct tm *__restrict __resbufp) __THROW;
# endif

#ifdef __cplusplus
}
#endif

#endif /* __GW32__ */

#endif /* _WINX_TIMEX_H_ */
