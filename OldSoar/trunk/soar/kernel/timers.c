#include "soarkernel.h"
#include "sysdep.h"
#if !defined(__SC__) && !defined(THINK_C) && !defined(WIN32) && !defined(MACINTOSH)
#include <sys/time.h>           /* used for timing stuff */
#include <sys/resource.h>       /* used for timing stuff */
#endif                          /* !__SC__ && !THINK_C && !WIN32 */

/* voigtjr 10/17/2003 - all defined in sysdep.h
#ifdef __hpux
#include <sys/syscall.h>
#include <unistd.h>
#define getrusage(a, b) syscall(SYS_GETRUSAGE, a, b)
#define getwd(arg) getcwd(arg, (size_t) 9999)
#endif
*/

/* ===================================================================

                       Timer Utility Routines

   These are utility routines for using timers.  We use (struct timeval)'s
   (defined in a system include file) for keeping track of the cumulative
   time spent in one part of the system or another.  Reset_timer()
   clears a timer to 0.  Start_timer() and stop_timer() are used for
   timing an interval of code--the usage is:
   
     start_timer (&timeval_to_record_the_start_time_in); 
     ... other code here ...
     stop_timer (&timeval_to_record_the_start_time_in,
                 &timeval_holding_accumulated_time_for_this_code);

   Finally, timer_value() returns the accumulated value of a timer
   (in seconds).
=================================================================== */
#ifndef NO_TIMING_STUFF
#define ONE_MILLION (1000000)

void reset_timer (struct timeval *tv_to_reset) {
  tv_to_reset->tv_sec = 0;
  tv_to_reset->tv_usec = 0;
}

#if defined(WIN32)

/* A fake implementation of rusage for WIN32. Taken from cygwin. */
#define RUSAGE_SELF 0
struct rusage {
   struct timeval ru_utime;
   struct timeval ru_stime;
};
#define NSPERSEC 10000000L
#define FACTOR (0x19db1ded53e8000L)

static unsigned __int64
__to_clock_t (FILETIME * src, int flag)
{
  unsigned __int64 total = ((unsigned __int64) src->dwHighDateTime << 32) + ((unsigned)src->dwLowDateTime);

  /* Convert into clock ticks - the total is in 10ths of a usec.  */
  if (flag)
    total -= FACTOR;

  total /= (unsigned __int64) (NSPERSEC / CLOCKS_PER_SEC);
  return total;
}
static void totimeval (struct timeval *dst, FILETIME *src, int sub, int flag)
{
  __int64 x = __to_clock_t (src, flag);

  x *= (int) (1e6) / CLOCKS_PER_SEC; /* Turn x into usecs */
  x -= (__int64) sub * (int) (1e6);

  dst->tv_usec = (long)(x % (__int64) (1e6)); /* And split */
  dst->tv_sec = (long)(x / (__int64) (1e6));
}

int getrusage(int who, struct rusage* r)
{
   FILETIME creation_time = {0,0};
   FILETIME exit_time = {0,0};
   FILETIME kernel_time = {0,0};
   FILETIME user_time = {0,0};

   who = who; /* shuts up compiler */

   memset (r, 0, sizeof (*r));
   GetProcessTimes (GetCurrentProcess(), &creation_time, &exit_time, &kernel_time, &user_time);
   totimeval (&r->ru_stime, &kernel_time, 0, 0);
   totimeval (&r->ru_utime, &user_time, 0, 0);
   return 0;
}

#endif /* WIN32 */

void get_cputime_from_rusage (struct rusage *r, struct timeval *dest_tv) {
  dest_tv->tv_sec = r->ru_utime.tv_sec + r->ru_stime.tv_sec;
  dest_tv->tv_usec = r->ru_utime.tv_usec + r->ru_stime.tv_usec;
  if (dest_tv->tv_usec >= ONE_MILLION) {
    dest_tv->tv_usec -= ONE_MILLION;
    dest_tv->tv_sec++;
  }
}

void start_timer (struct timeval *tv_for_recording_start_time) {
  struct rusage temp_rusage;
  
  getrusage (RUSAGE_SELF, &temp_rusage);
  get_cputime_from_rusage (&temp_rusage, tv_for_recording_start_time);
}

void stop_timer (struct timeval *tv_with_recorded_start_time,
                 struct timeval *tv_with_accumulated_time) {
  struct rusage end_rusage;
  struct timeval end_tv;
  long delta_sec, delta_usec;
  
  getrusage (RUSAGE_SELF, &end_rusage);
  get_cputime_from_rusage (&end_rusage, &end_tv);

  delta_sec = end_tv.tv_sec - tv_with_recorded_start_time->tv_sec;
  delta_usec = end_tv.tv_usec - tv_with_recorded_start_time->tv_usec;
  if (delta_usec < 0) {
    delta_usec += ONE_MILLION;
    delta_sec--;
  }

  tv_with_accumulated_time->tv_sec += delta_sec;
  tv_with_accumulated_time->tv_usec += delta_usec;
  if (tv_with_accumulated_time->tv_usec >= ONE_MILLION) {
    tv_with_accumulated_time->tv_usec -= ONE_MILLION;
    tv_with_accumulated_time->tv_sec++;
  }
}

double timer_value (struct timeval *tv) {
  return (double)(tv->tv_sec) + (double)(tv->tv_usec)/(double)ONE_MILLION;
}
#endif

#ifdef REAL_TIME_BEHAVIOR
/* RMJ */
void init_real_time (void) {
   thisAgent->real_time_tracker =
         (struct timeval *) malloc(sizeof(struct timeval));
   timerclear(thisAgent->real_time_tracker);
   thisAgent->real_time_idling = FALSE;
   current_real_time =
         (struct timeval *) malloc(sizeof(struct timeval));
}
#endif

#ifdef ATTENTION_LAPSE
/* RMJ */

void wake_from_attention_lapse (void) {
   /* Set tracker to last time we woke up */
   start_timer (thisAgent->attention_lapse_tracker);
   thisAgent->attention_lapsing = FALSE;
}

void init_attention_lapse (void) {
   thisAgent->attention_lapse_tracker =
         (struct timeval *) malloc(sizeof(struct timeval));
   wake_from_attention_lapse();
#ifndef REAL_TIME_BEHAVIOR
   current_real_time =
         (struct timeval *) malloc(sizeof(struct timeval));
#endif
}

void start_attention_lapse (long duration) {
   /* Set tracker to time we should wake up */
   start_timer (thisAgent->attention_lapse_tracker);
   thisAgent->attention_lapse_tracker->tv_usec += 1000 * duration;
   if (thisAgent->attention_lapse_tracker->tv_usec >= 1000000) {
      thisAgent->attention_lapse_tracker->tv_sec +=
            thisAgent->attention_lapse_tracker->tv_usec / 1000000;
      thisAgent->attention_lapse_tracker->tv_usec %= 1000000;
   }
   thisAgent->attention_lapsing = TRUE;
}
   
#endif
