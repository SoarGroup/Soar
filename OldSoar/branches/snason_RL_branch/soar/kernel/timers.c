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

#ifdef PII_TIMERS

#ifndef MHZ
#error Must define MHZ as the Clock Speed on the CPU in MegaHertz
#endif

#define RDTSC(llptr) ({ \
__asm__ __volatile__ ( \
        ".byte 0x0f; .byte 0x31" \
        : "=A" (llptr) \
        : : "eax", "edx"); })

#define CPUID ({ \
__asm__ __volatile__ ( \
        ".byte 0x0f; .byte 0xa2" \
        : : : "eax", "ebx", "ecx", "edx" ); })

void SafeRDTSC(unsigned long long int *ullptr)
{

    CPUID;
    RDTSC(*ullptr);

}

int test_timers()
{

    unsigned long long int start, stop, base, baseii;

    double temp;
    struct timeval tv_start, tv_stop;

    start_timer(&start);
    start_timer(&start);
    start_timer(&start);
    start_timer(&start);

    reset_timer(&base);
    start_timer(&start);
    stop_timer(&start, &base);

    reset_timer(&baseii);
    start_timer(&start);
    stop_timer(&start, &baseii);

    print("Finding overhead of RDTSC call\n");
    print("Attempt 1  --> %ld  ", (long) base);
    print("Attempt 2  --> %ld  ", (long) baseii);
    base = (base > baseii) ? baseii : base;

    reset_timer(&baseii);
    start_timer(&start);
    stop_timer(&start, &baseii);
    base = (base > baseii) ? baseii : base;
    print("Attempt 3  --> %ld  ", (long) baseii);

    reset_timer(&baseii);
    start_timer(&start);
    stop_timer(&start, &baseii);
    base = (base > baseii) ? baseii : base;
    print("Attempt 4  --> %ld\n", (long) baseii);

    print("Minimum number of Clock Cycles for RDTSC --> %ld\n", (long) base);
    print("Overhead is:  %8.4e seconds per timer switch\n", timer_value(&base));

    print("Timing 3 second Sleep\n");
    reset_timer(&stop);

    gettimeofday(&tv_start, NULL);
    start_timer(&start);
    sys_sleep(3);
    stop_timer(&start, &stop);
    gettimeofday(&tv_stop, NULL);

    print("RDTSC Returns        --> %f\n", timer_value(&stop));

    temp = tv_stop.tv_sec - tv_start.tv_sec;
    temp += (double) ((tv_stop.tv_usec - tv_start.tv_usec) / (double) ONE_MILLION);
    print("gettimeofday Returns --> %f\n\n\n", temp);

    return (int) base;

}

void reset_timer(unsigned long long int *tv_to_reset)
{
    *tv_to_reset = 0ULL;
}

double timer_value(unsigned long long int *tv)
{

    return (double) ((*tv) / ((double) (MHZ * ONE_MILLION)));
}

void start_timer(unsigned long long int *tv_for_recording_start_time)
{

    SafeRDTSC(tv_for_recording_start_time);

}

void stop_timer(unsigned long long int *tv_with_start_time, unsigned long long int *tv_with_accumulated_time)
{

    unsigned long long int end_time;

    SafeRDTSC(&end_time);
    *tv_with_accumulated_time += (end_time - *tv_with_start_time);

}

#else

/* ===================================================================
 *
 *  OS Independent Functions (non PII specific) 
 *
 * =================================================================== */
void reset_timer(struct timeval *tv_to_reset)
{
    tv_to_reset->tv_sec = 0;
    tv_to_reset->tv_usec = 0;
}

double timer_value(struct timeval *tv)
{
    return (double) (tv->tv_sec) + (double) (tv->tv_usec) / (double) ONE_MILLION;
}

/* ===================================================================
 *
 *  Macintosh and Windows Timing Functions (non PII specific) 
 *
 * =================================================================== */

#if defined(MACINTOSH) || defined(WIN32)

void get_cputime_from_clock(clock_t t, struct timeval *dt)
{
    dt->tv_sec = t / CLOCKS_PER_SEC;
    dt->tv_usec = (long) (((t % CLOCKS_PER_SEC) / (float) CLOCKS_PER_SEC) * ONE_MILLION);
}

void start_timer(struct timeval *tv_for_recording_start_time)
{
    clock_t ticks;

    ticks = clock();
    get_cputime_from_clock(ticks, tv_for_recording_start_time);
}

void stop_timer(struct timeval *tv_with_recorded_start_time, struct timeval *tv_with_accumulated_time)
{
    clock_t ticks;
    struct timeval end_tv;
    long delta_sec, delta_usec;

    ticks = clock();
    get_cputime_from_clock(ticks, &end_tv);

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
#ifdef WARN_IF_TIMERS_REPORT_ZERO
    if (current_agent(warn_on_zero_timers) &&
        delta_usec == 0 && delta_sec == 0 && current_agent(sysparams)[PRINT_WARNINGS_SYSPARAMS]) {

        print("\nWarning:  A timer has reported zero.\n");
        print("  This is likely the result of poor system timer resolution.\n");
        print("  (This warning will not be repeated)\n");
        current_agent(warn_on_zero_timers) = FALSE;
    }
#endif

#ifdef COUNT_KERNEL_TIMER_STOPS
    if (tv_with_accumulated_time == &(current_agent(total_kernel_time))) {
        current_agent(kernelTimerStops)++;
    } else {
        current_agent(nonKernelTimerStops)++;
    }

#endif

#ifdef KT_HISTOGRAM
    if (tv_with_accumulated_time == &(current_agent(total_kernel_time)) &&
        current_agent(d_cycle_count) < current_agent(kt_histogram_sz)) {

        current_agent(kt_histogram_tv)[current_agent(d_cycle_count)].tv_sec += delta_sec;
        current_agent(kt_histogram_tv)[current_agent(d_cycle_count)].tv_usec += delta_usec;

        if (current_agent(kt_histogram_tv)[current_agent(d_cycle_count)].tv_usec >= ONE_MILLION) {

            current_agent(kt_histogram_tv)[current_agent(d_cycle_count)].tv_usec -= ONE_MILLION;
            current_agent(kt_histogram_tv)[current_agent(d_cycle_count)].tv_sec++;
        }

    }
#endif

}

#else

/* ===================================================================
 *
 *  Unix Timing Functions
 *
 * =================================================================== */

void get_cputime_from_rusage(struct rusage *r, struct timeval *dest_tv)
{
    dest_tv->tv_sec = r->ru_utime.tv_sec + r->ru_stime.tv_sec;
    dest_tv->tv_usec = r->ru_utime.tv_usec + r->ru_stime.tv_usec;
    if (dest_tv->tv_usec >= ONE_MILLION) {
        dest_tv->tv_usec -= ONE_MILLION;
        dest_tv->tv_sec++;
    }
}

void start_timer(struct timeval *tv_for_recording_start_time)
{
    struct rusage temp_rusage;

    getrusage(RUSAGE_SELF, &temp_rusage);
    get_cputime_from_rusage(&temp_rusage, tv_for_recording_start_time);
}

void stop_timer(struct timeval *tv_with_recorded_start_time, struct timeval *tv_with_accumulated_time)
{
    struct rusage end_rusage;
    struct timeval end_tv;
    long delta_sec, delta_usec;

    getrusage(RUSAGE_SELF, &end_rusage);
    get_cputime_from_rusage(&end_rusage, &end_tv);

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

#ifdef WARN_IF_TIMERS_REPORT_ZERO
    if (current_agent(warn_on_zero_timers) && delta_usec == 0 && delta_sec == 0) {

        print("\nWarning:  A timer has reported zero.\n");
        print("  This is likely the result of poor system timer resolution.\n");
        print("  (This warning will not be repeated)\n");
        current_agent(warn_on_zero_timers) = FALSE;
    }
#endif

#ifdef COUNT_KERNEL_TIMER_STOPS
    if (tv_with_accumulated_time == &(current_agent(total_kernel_time))) {
        current_agent(kernelTimerStops)++;
    } else {
        current_agent(nonKernelTimerStops)++;
    }

#endif

#ifdef KT_HISTOGRAM
    if (tv_with_accumulated_time == &(current_agent(total_kernel_time)) &&
        current_agent(d_cycle_count) < current_agent(kt_histogram_sz)) {

        current_agent(kt_histogram_tv)[current_agent(d_cycle_count)].tv_sec += delta_sec;
        current_agent(kt_histogram_tv)[current_agent(d_cycle_count)].tv_usec += delta_usec;

        if (current_agent(kt_histogram_tv)[current_agent(d_cycle_count)].tv_usec >= ONE_MILLION) {

            current_agent(kt_histogram_tv)[current_agent(d_cycle_count)].tv_usec -= ONE_MILLION;
            current_agent(kt_histogram_tv)[current_agent(d_cycle_count)].tv_sec++;
        }

    }
#endif

}

#endif                          /* UNIX */

#endif                          /* ndef PII_TIMERS */

#endif                          /* ndef NO_TIMING_STUFF */
