#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include "portability.h"

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

#include "gSKI_AgentPerformanceMonitor.h"

#include <sstream>

#include "agent.h"
#include "init_soar.h" // for timer_value
#include "print.h"
#include "rete.h" // for get_node_count_statistics

#include "gSKI_Agent.h"

using namespace gSKI;

// a hack so I don't have to rewrite all the current_agent code below...
#define current_agent(x) a->x

#ifdef WIN32 
// Sleep on windows is in milliseconds, hence the multiplication by 1000
#define sys_sleep( seconds )    Sleep( seconds * 1000 )
#else /* WIN32 */
#include <unistd.h>
#define sys_sleep( seconds )    sleep( seconds )
#endif /* !WIN32 */

#ifdef _WIN32
#define safeSprintf _snprintf
#else
#define safeSprintf snprintf
#endif

namespace {

   bool string_match_up_to (const char * string1, 
                            const char * string2, 
                            unsigned int positions)
   {
      unsigned int i,num;

      /*  what we really want is to require a match over the length of
      the shorter of the two strings, with positions being a minimum */

      num = (unsigned)strlen(string1);
      if (num > strlen(string2)) num = (unsigned)strlen(string2);
      if (positions < num)  positions = num;

      for (i = 0; i < positions; i++)
      {
         if (string1[i] != string2[i])
            return false;
      }

      return true;  
   }
#ifndef NO_TIMING_STUFF
/*
 *----------------------------------------------------------------------
 *
 * soar_cDetermineTimerResolution
 *
 *   check the resolution of the system timers.     
 *
 *----------------------------------------------------------------------
 */
double soar_cDetermineTimerResolution( double *min, double *max) {
#define ONE_MILLION 1000000
  double delta, max_delta, min_delta, min_nz_delta;
  float q;  
  int i,j, top;
#ifdef PII_TIMERS
  unsigned long long int start, end, total;
#else
  struct timeval start, end, total;
#endif
  
  top = ONE_MILLION;
  min_delta = ONE_MILLION;
  min_nz_delta = ONE_MILLION;
  max_delta = -1;
  reset_timer( &total );

  for( i = 0; i < ONE_MILLION; i = (i+1)*2 ) {
    reset_timer( &end );
    start_timer( NULL, &start );
    for( j = 0; j < i*top; j++ ) {
      q = (float)j*i;
    }
    stop_timer( NULL, &start, &end );
    stop_timer( NULL, &start, &total );
    delta = timer_value( &end );
    
    if ( delta < min_delta ) min_delta = delta;
    if ( delta && delta < min_nz_delta ) min_nz_delta = delta;
    if ( delta > max_delta ) max_delta = delta;
    
    /* when we have gone through this loop for 2 seconds, stop */
    if ( timer_value( &total ) >= 2 ) { break; }

  }

  if ( min_nz_delta == ONE_MILLION ) min_nz_delta = -1;
  if ( min_delta == ONE_MILLION ) min_delta = -1;

  if ( min != NULL ) *min = min_delta;
  if ( max != NULL ) *max = max_delta;
  return min_nz_delta;

}
#endif  

   // A helper to conver numbers to strings...
   template <typename T> std::string ToString(const T& v)
   {
      std::ostringstream oss;
      oss << v;
      return oss.str();
   }
}


AgentPerformanceMonitor::AgentPerformanceMonitor(Agent* pAgent) :
   m_pAgent(pAgent)
{
}

AgentPerformanceMonitor::~AgentPerformanceMonitor()
{
}

bool AgentPerformanceMonitor::GetStatsString(int argc, 
                                             char* argv[], 
                                             const char** result)
{
   m_result = "";
   bool r = false;
   if (argc == 1 || string_match_up_to("-system", argv[1], 2))
   {
      r = parse_system_stats(argc, argv);
   } 
   else if (string_match_up_to("-memory", argv[1], 2))
   {
      r = parse_memory_stats(argc, argv);
   }
   else if (string_match_up_to("-rete", argv[1], 2))
   {
      r = parse_rete_stats(argc, argv);
   }
//#ifdef DC_HISTOGRAM
//   else if (string_match_up_to("-dc_histogram", argv[1], 2))
//   {
//      r = soar_ecPrintDCHistogram();
//   }
//#endif
//#ifdef KT_HISTOGRAM
//   else if (string_match_up_to("-kt_histogram", argv[1], 2)) 
//   {
//      r = soar_ecPrintKTHistogram();
//   }
//#endif
#ifndef NO_TIMING_STUFF
   else if (string_match_up_to("-timers", argv[1], 2))
   {
      printTimingInfo();
      r = true;
   }
#endif
   else 
   {
      m_result = std::string("Unrecognized argument to stats: ") + argv[1];
      r = false;
   }

   *result = m_result.c_str();
   return r;
}


/*
 *----------------------------------------------------------------------
 *
 * parse_system_stats --
 *
 *	This procedure parses an argv array and prints the selected
 *      statistics.
 *
 *      The syntax being parsed is:
 *         stats -system <stype>
 *         <stype> ::= -default-production-count
 *                     -user-production-count
 *                     -chunk-count
 *                     -justification-count
 *                     -all-productions-count
 *                     -dc-count
 *                     -ec-count
 *                     -ecs/dc
 *                     -firings-count
 *                     -firings/ec
 *                     -wme-change-count
 *                     -wme-addition-count
 *                     -wme-removal-count
 *                     -wme-count
 *                     -wme-avg-count
 *                     -wme-max-count
 *                     -total-time             |T
 *                     -ms/dc                  |T
 *                     -ms/ec                  |T
 *                     -ms/firing              |T
 *                     -ms/wme-change          |T
 *                     -match-time             |D
 *                     -ownership-time         |D
 *                     -chunking-time          |D
 *
 *         The items marked |T are available when Soar has been
 *         compiled with the NO_TIMING_STUFF flag NOT SET and 
 *         the items marked |D are available when Soar has been
 *         compiled with the DETAILED_TIMING_STATS flag SET.
 *
 * Results:
 *	Returns the statistic and Tcl return code.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

bool AgentPerformanceMonitor::parse_system_stats(int argc, char *argv[])
{
   agent* a = m_pAgent->GetSoarAgent();

#ifndef NO_TIMING_STUFF
    double total_kernel_time, total_kernel_msec;

#ifdef DETAILED_TIMING_STATS
    double time;
#endif
#endif

    if (argc > 3) 
    {
        m_result = "Too many arguments, should be: stats -system [<type>]";
        return false;
    }
#ifndef NO_TIMING_STUFF
    total_kernel_time = timer_value(&a->total_kernel_time);
    total_kernel_msec = total_kernel_time * 1000.0;
#endif
    if (argc <= 2)  /* Invoked as stats or stats -system */
    {
        soar_ecPrintSystemStatistics();
        return true;
    } 
    else 
    {
        if (!strcmp("-default-production-count", argv[2])) 
        {
           m_result = ToString(a->num_productions_of_type[DEFAULT_PRODUCTION_TYPE]);
        } 
        else if (!strcmp("-user-production-count", argv[2])) 
        {
            m_result = ToString(a->num_productions_of_type[USER_PRODUCTION_TYPE]);

        } 
        else if (!strcmp("-chunk-count", argv[2])) 
        {
            m_result = ToString(a->num_productions_of_type[CHUNK_PRODUCTION_TYPE]);
        } 
        else if (!strcmp("-justification-count", argv[2])) 
        {
            m_result = ToString(a->num_productions_of_type[JUSTIFICATION_PRODUCTION_TYPE]);
        } 
        else if (!strcmp("-all-productions-count", argv[2])) 
        {
            m_result = ToString(a->num_productions_of_type[DEFAULT_PRODUCTION_TYPE]
                                + a->num_productions_of_type[USER_PRODUCTION_TYPE]
                                + a->num_productions_of_type[CHUNK_PRODUCTION_TYPE]);

        } 
        else if (!strcmp("-dc-count", argv[2])) 
        {
			/* v8.6.2: print out decisions executed, not # full cycles */
            m_result = ToString(a->decision_phases_count);
        } 
        else if (!strcmp("-ec-count", argv[2])) 
        {
            m_result = ToString(a->e_cycle_count);
        } 
        else if (!strcmp("-ecs/dc", argv[2])) 
        {
			/* v8.6.2: print out decisions executed, not # full cycles */
            m_result = ToString((a->decision_phases_count
                                              ? ((double) a->e_cycle_count
                                                 / a->decision_phases_count)
                                              : 0.0));
        } 
        else if (!strcmp("-firings-count", argv[2])) 
        {
            m_result = ToString(a->production_firing_count);
        } 
        else if (!strcmp("-firings/ec", argv[2])) 
        {
            m_result = ToString((a->e_cycle_count
                                              ? ((double) a->production_firing_count
                                                 / a->e_cycle_count)
                                              : 0.0));
        } 
        else if (!strcmp("-wme-change-count", argv[2])) 
        {
            m_result = ToString(a->wme_addition_count
                                + a->wme_removal_count);
        } 
        else if (!strcmp("-wme-addition-count", argv[2])) 
        {
            m_result = ToString(a->wme_addition_count);
        } 
        else if (!strcmp("-wme-removal-count", argv[2])) 
        {
            m_result = ToString(a->wme_removal_count);
        } 
        else if (!strcmp("-wme-count", argv[2])) 
        {
            m_result = ToString(a->num_wmes_in_rete);
        } 
        else if (!strcmp("-wme-avg-count", argv[2])) 
        {
            m_result = ToString((a->num_wm_sizes_accumulated
                                              ? (a->cumulative_wm_size
                                                 / a->num_wm_sizes_accumulated)
                                              : 0.0));
        } 
        else if (!strcmp("-wme-max-count", argv[2])) 
        {
            m_result = ToString(a->max_wm_size);
        }
#ifndef NO_TIMING_STUFF
        else if (!strcmp("-total-time", argv[2])) 
        {
            m_result = ToString(total_kernel_time);
        } 
        else if (!strcmp("-ms/dc", argv[2])) 
        {
			/* v8.6.2: print out decisions executed, not # full cycles */
            m_result = ToString((a->decision_phases_count
                                              ? total_kernel_msec / a->decision_phases_count
                                              : 0.0));
        } 
        else if (!strcmp("-ms/ec", argv[2])) 
        {
            m_result = ToString((a->e_cycle_count
                                              ? total_kernel_msec / a->e_cycle_count
                                              : 0.0));
        } 
        else if (!strcmp("-ms/firing", argv[2])) 
        {
            m_result = ToString((a->production_firing_count
                                              ? total_kernel_msec / a->production_firing_count
                                              : 0.0));
        }
#endif                          /* NO_TIMING_STUFF */
#ifdef DETAILED_TIMING_STATS
        else if (!strcmp("-ms/wme-change", argv[2])) 
        {
            long wme_changes;
            time = timer_value(&a->match_cpu_time[INPUT_PHASE])
                + timer_value(&a->match_cpu_time[DETERMINE_LEVEL_PHASE])
                + timer_value(&a->match_cpu_time[PREFERENCE_PHASE])
                + timer_value(&a->match_cpu_time[WM_PHASE])
                + timer_value(&a->match_cpu_time[OUTPUT_PHASE])
                + timer_value(&a->match_cpu_time[DECISION_PHASE]);

            time *= 1000;       /* convert to msec */

            wme_changes = a->wme_addition_count
                + a->wme_removal_count;

            m_result = ToString((wme_changes ? time / wme_changes : 0.0));
        } 
        else if (!strcmp("-match-time", argv[2])) 
        {

            time = timer_value(&a->match_cpu_time[INPUT_PHASE])
                + timer_value(&a->match_cpu_time[DETERMINE_LEVEL_PHASE])
                + timer_value(&a->match_cpu_time[PREFERENCE_PHASE])
                + timer_value(&a->match_cpu_time[WM_PHASE])
                + timer_value(&a->match_cpu_time[OUTPUT_PHASE])
                + timer_value(&a->match_cpu_time[DECISION_PHASE]);

            m_result = ToString(time);

        } 
        else if (!strcmp("-ownership-time", argv[2])) 
        {
            time = timer_value(&a->ownership_cpu_time[INPUT_PHASE])
                + timer_value(&a->ownership_cpu_time[DETERMINE_LEVEL_PHASE])
                + timer_value(&a->ownership_cpu_time[PREFERENCE_PHASE])
                + timer_value(&a->ownership_cpu_time[WM_PHASE])
                + timer_value(&a->ownership_cpu_time[OUTPUT_PHASE])
                + timer_value(&a->ownership_cpu_time[DECISION_PHASE]);

            m_result = ToString(time);

        } 
        else if (!strcmp("-chunking-time", argv[2])) 
        {
            time = timer_value(&a->chunking_cpu_time[INPUT_PHASE])
                + timer_value(&a->chunking_cpu_time[DETERMINE_LEVEL_PHASE])
                + timer_value(&a->chunking_cpu_time[PREFERENCE_PHASE])
                + timer_value(&a->chunking_cpu_time[WM_PHASE])
                + timer_value(&a->chunking_cpu_time[OUTPUT_PHASE])
                + timer_value(&a->chunking_cpu_time[DECISION_PHASE]);

            m_result = ToString(time);

        }
#endif                          /* DETAILED_TIMING_STATS */
        else 
        {
           m_result = std::string("Unrecognized argument to stats: -system ") + argv[2];
        }
    }

    return true;
}

void AgentPerformanceMonitor::soar_ecPrintSystemStatistics()
{

   agent* a = m_pAgent->GetSoarAgent();

    unsigned long wme_changes;

    /* REW: begin 28.07.96 */
#ifndef NO_TIMING_STUFF
    double total_kernel_time, total_kernel_msec, derived_kernel_time, monitors_sum, input_function_time, input_phase_total_time, output_function_time, output_phase_total_time, determine_level_phase_total_time,       /* REW: end   05.05.97 */
     propose_phase_total_time, apply_phase_total_time, preference_phase_total_time, wm_phase_total_time, decision_phase_total_time, derived_total_cpu_time;

#ifdef DETAILED_TIMING_STATS
    double match_time, match_msec;
    double ownership_time, chunking_time;
    double other_phase_kernel_time[6], other_total_kernel_time;
#endif
#endif
    /* REW: end 28.07.96 */

    /* MVP 6-8-94 */
    char hostname[MAX_LEXEME_LENGTH + 1];
    time_t current_time;

#if !defined (THINK_C) && !defined (__SC__) && !defined(MACINTOSH) && !defined(WIN32) && !defined(_WINDOWS)
    if (gethostname(hostname, MAX_LEXEME_LENGTH)) {
#endif

        strncpy(hostname, "[host name unknown]", MAX_LEXEME_LENGTH + 1);
        hostname[MAX_LEXEME_LENGTH] = 0;

#if !defined (THINK_C) && !defined (__SC__) && !defined(MACINTOSH) && !defined(WIN32) && !defined(_WINDOWS)
    }
#endif

    current_time = time(NULL);

/* REW: begin 28.07.96 */
/* See note in soarkernel.h for a description of the timers */
#ifndef NO_TIMING_STUFF
    total_kernel_time = timer_value(&current_agent(total_kernel_time));
    total_kernel_msec = total_kernel_time * 1000.0;

    /* derived_kernel_time := Total of the time spent in the phases of the decision cycle, 
       excluding Input Function, Output function, and pre-defined callbacks. 
       This computed time should be roughly equal to total_kernel_time, 
       as determined above. */

#ifndef KERNEL_TIME_ONLY
    derived_kernel_time = timer_value(&current_agent(decision_cycle_phase_timers[INPUT_PHASE]))
        + timer_value(&current_agent(decision_cycle_phase_timers[PROPOSE_PHASE]))
        + timer_value(&current_agent(decision_cycle_phase_timers[APPLY_PHASE]))
        + timer_value(&current_agent(decision_cycle_phase_timers[PREFERENCE_PHASE]))
        + timer_value(&current_agent(decision_cycle_phase_timers[WM_PHASE]))
        + timer_value(&current_agent(decision_cycle_phase_timers[OUTPUT_PHASE]))
        + timer_value(&current_agent(decision_cycle_phase_timers[DECISION_PHASE]));

    input_function_time = timer_value(&current_agent(input_function_cpu_time));

    output_function_time = timer_value(&current_agent(output_function_cpu_time));

    /* Total of the time spent in callback routines. */
    monitors_sum = timer_value(&current_agent(monitors_cpu_time[INPUT_PHASE]))
        + timer_value(&current_agent(monitors_cpu_time[PROPOSE_PHASE]))
        + timer_value(&current_agent(monitors_cpu_time[APPLY_PHASE]))
        + timer_value(&current_agent(monitors_cpu_time[PREFERENCE_PHASE]))
        + timer_value(&current_agent(monitors_cpu_time[WM_PHASE]))
        + timer_value(&current_agent(monitors_cpu_time[OUTPUT_PHASE]))
        + timer_value(&current_agent(monitors_cpu_time[DECISION_PHASE]));

    derived_total_cpu_time = derived_kernel_time + monitors_sum + input_function_time + output_function_time;

    /* Total time spent in the input phase */
    input_phase_total_time = timer_value(&current_agent(decision_cycle_phase_timers[INPUT_PHASE]))
        + timer_value(&current_agent(monitors_cpu_time[INPUT_PHASE]))
        + timer_value(&current_agent(input_function_cpu_time));


    /* REW: begin 10.30.97 */
    //determine_level_phase_total_time = timer_value(&current_agent(decision_cycle_phase_timers[DETERMINE_LEVEL_PHASE]))
    //    + timer_value(&current_agent(monitors_cpu_time[DETERMINE_LEVEL_PHASE]));
	determine_level_phase_total_time = 0;
    /* REW: end   10.30.97 */

    /* Total time spent in the preference phase */
    preference_phase_total_time = timer_value(&current_agent(decision_cycle_phase_timers[PREFERENCE_PHASE]))
        + timer_value(&current_agent(monitors_cpu_time[PREFERENCE_PHASE]));

    /* Total time spent in the working memory phase */
    wm_phase_total_time = timer_value(&current_agent(decision_cycle_phase_timers[WM_PHASE]))
        + timer_value(&current_agent(monitors_cpu_time[WM_PHASE]));

	    /* Total time spent in the propose phase */
    propose_phase_total_time = timer_value(&current_agent(decision_cycle_phase_timers[PROPOSE_PHASE]))
        + timer_value(&current_agent(monitors_cpu_time[PROPOSE_PHASE]));

    /* Total time spent in the apply phase */
    apply_phase_total_time = timer_value(&current_agent(decision_cycle_phase_timers[APPLY_PHASE]))
        + timer_value(&current_agent(monitors_cpu_time[APPLY_PHASE]));

    /* Total time spent in the output phase */
    output_phase_total_time = timer_value(&current_agent(decision_cycle_phase_timers[OUTPUT_PHASE]))
        + timer_value(&current_agent(monitors_cpu_time[OUTPUT_PHASE]))
        + timer_value(&current_agent(output_function_cpu_time));

    /* Total time spent in the decision phase */
    decision_phase_total_time = timer_value(&current_agent(decision_cycle_phase_timers[DECISION_PHASE]))
        + timer_value(&current_agent(monitors_cpu_time[DECISION_PHASE]));

    /* The sum of these phase timers is exactly equal to the 
     * derived_total_cpu_time
     */

#ifdef DETAILED_TIMING_STATS

	// KJC removed DETERMINE_LEVEL_PHASE.  Using Propose and Apply  04.05

    match_time = timer_value(&current_agent(match_cpu_time[INPUT_PHASE]))
        + timer_value(&current_agent(match_cpu_time[PROPOSE_PHASE]))
        + timer_value(&current_agent(match_cpu_time[APPLY_LEVEL_PHASE]))
        + timer_value(&current_agent(match_cpu_time[PREFERENCE_PHASE]))
        + timer_value(&current_agent(match_cpu_time[WM_PHASE]))
        + timer_value(&current_agent(match_cpu_time[OUTPUT_PHASE]))
        + timer_value(&current_agent(match_cpu_time[DECISION_PHASE]));

    match_msec = 1000 * match_time;

    ownership_time = timer_value(&current_agent(ownership_cpu_time[INPUT_PHASE]))
        + timer_value(&current_agent(ownership_cpu_time[PROPOSE_PHASE]))
        + timer_value(&current_agent(ownership_cpu_time[APPLY_LEVEL_PHASE]))
        + timer_value(&current_agent(ownership_cpu_time[PREFERENCE_PHASE]))
        + timer_value(&current_agent(ownership_cpu_time[WM_PHASE]))
        + timer_value(&current_agent(ownership_cpu_time[OUTPUT_PHASE]))
        + timer_value(&current_agent(ownership_cpu_time[DECISION_PHASE]));

    chunking_time = timer_value(&current_agent(chunking_cpu_time[INPUT_PHASE]))
        + timer_value(&current_agent(chunking_cpu_time[PROPOSE_PHASE]))
        + timer_value(&current_agent(chunking_cpu_time[APPLY_PHASE]))
        + timer_value(&current_agent(chunking_cpu_time[PREFERENCE_PHASE]))
        + timer_value(&current_agent(chunking_cpu_time[WM_PHASE]))
        + timer_value(&current_agent(chunking_cpu_time[OUTPUT_PHASE]))
        + timer_value(&current_agent(chunking_cpu_time[DECISION_PHASE]));

    /* O-support time should go to 0 with o-support-mode 2 */
    /* o_support_time = 
       timer_value (&current_agent(o_support_cpu_time[INPUT_PHASE])) 
       + timer_value (&current_agent(o_support_cpu_time[DETERMINE_LEVEL_PHASE])) 
       + timer_value (&current_agent(o_support_cpu_time[PREFERENCE_PHASE])) 
       + timer_value (&current_agent(o_support_cpu_time[WM_PHASE])) 
       + timer_value (&current_agent(o_support_cpu_time[OUTPUT_PHASE])) 
       + timer_value (&current_agent(o_support_cpu_time[DECISION_PHASE])); */

    other_phase_kernel_time[INPUT_PHASE] = timer_value(&current_agent(decision_cycle_phase_timers[INPUT_PHASE]))
        - timer_value(&current_agent(match_cpu_time[INPUT_PHASE]))
        - timer_value(&current_agent(ownership_cpu_time[INPUT_PHASE]))
        - timer_value(&current_agent(chunking_cpu_time[INPUT_PHASE]));

	/* Removed 04.05 for Soar 8.6.0
     * other_phase_kernel_time[DETERMINE_LEVEL_PHASE] =
     *    timer_value(&current_agent(decision_cycle_phase_timers[DETERMINE_LEVEL_PHASE]))
     *   - timer_value(&current_agent(match_cpu_time[DETERMINE_LEVEL_PHASE]))
     *   - timer_value(&current_agent(ownership_cpu_time[DETERMINE_LEVEL_PHASE]))
     *   - timer_value(&current_agent(chunking_cpu_time[DETERMINE_LEVEL_PHASE]));
     */
    other_phase_kernel_time[PROPOSE_PHASE] =
        timer_value(&current_agent(decision_cycle_phase_timers[PROPOSE_PHASE]))
        - timer_value(&current_agent(match_cpu_time[PROPOSE_PHASE]))
        - timer_value(&current_agent(ownership_cpu_time[PROPOSE_PHASE]))
        - timer_value(&current_agent(chunking_cpu_time[PROPOSE_PHASE]));

    other_phase_kernel_time[APPLY_PHASE] =
        timer_value(&current_agent(decision_cycle_phase_timers[APPLY_PHASE]))
        - timer_value(&current_agent(match_cpu_time[APPLY_PHASE]))
        - timer_value(&current_agent(ownership_cpu_time[APPLY_PHASE]))
        - timer_value(&current_agent(chunking_cpu_time[APPLY_PHASE]));

    other_phase_kernel_time[PREFERENCE_PHASE] =
        timer_value(&current_agent(decision_cycle_phase_timers[PREFERENCE_PHASE]))
        - timer_value(&current_agent(match_cpu_time[PREFERENCE_PHASE]))
        - timer_value(&current_agent(ownership_cpu_time[PREFERENCE_PHASE]))
        - timer_value(&current_agent(chunking_cpu_time[PREFERENCE_PHASE]));

    other_phase_kernel_time[WM_PHASE] = timer_value(&current_agent(decision_cycle_phase_timers[WM_PHASE]))
        - timer_value(&current_agent(match_cpu_time[WM_PHASE]))
        - timer_value(&current_agent(ownership_cpu_time[WM_PHASE]))
        - timer_value(&current_agent(chunking_cpu_time[WM_PHASE]));

    other_phase_kernel_time[OUTPUT_PHASE] = timer_value(&current_agent(decision_cycle_phase_timers[OUTPUT_PHASE]))
        - timer_value(&current_agent(match_cpu_time[OUTPUT_PHASE]))
        - timer_value(&current_agent(ownership_cpu_time[OUTPUT_PHASE]))
        - timer_value(&current_agent(chunking_cpu_time[OUTPUT_PHASE]));

    other_phase_kernel_time[DECISION_PHASE] = timer_value(&current_agent(decision_cycle_phase_timers[DECISION_PHASE]))
        - timer_value(&current_agent(match_cpu_time[DECISION_PHASE]))
        - timer_value(&current_agent(ownership_cpu_time[DECISION_PHASE]))
        - timer_value(&current_agent(chunking_cpu_time[DECISION_PHASE]));

    other_total_kernel_time = other_phase_kernel_time[INPUT_PHASE]
        + other_phase_kernel_time[PROPOSE_PHASE]
        + other_phase_kernel_time[APPLY_PHASE]
        + other_phase_kernel_time[PREFERENCE_PHASE]
        + other_phase_kernel_time[WM_PHASE]
        + other_phase_kernel_time[OUTPUT_PHASE]
        + other_phase_kernel_time[DECISION_PHASE];

#endif
#endif
#endif
/* REW: end 28.07.96 */

	char buf[128];

    //print(a, "Soar %s on %s at %s\n", soar_version_string, hostname, ctime((const time_t *) &current_time));
	safeSprintf(buf, 127, "Soar %s on %s at %s\n", soar_version_string, hostname, ctime(&current_time));
	m_result += buf;

    //print(a, "%lu productions (%lu default, %lu user, %lu chunks)\n",
    //      current_agent(num_productions_of_type)[DEFAULT_PRODUCTION_TYPE] +
    //      current_agent(num_productions_of_type)[USER_PRODUCTION_TYPE] +
    //      current_agent(num_productions_of_type)[CHUNK_PRODUCTIN_TYPE],
    //      current_agent(num_productions_of_type)[DEFAULT_PRODUCTION_TYPE],
    //      current_agent(num_productions_of_type)[USER_PRODUCTION_TYPE],
    //      current_agent(num_productions_of_type)[CHUNK_PRODUCTION_TYPE]);
	safeSprintf(buf, 127, "%lu productions (%lu default, %lu user, %lu chunks)\n",
          current_agent(num_productions_of_type)[DEFAULT_PRODUCTION_TYPE] +
          current_agent(num_productions_of_type)[USER_PRODUCTION_TYPE] +
          current_agent(num_productions_of_type)[CHUNK_PRODUCTION_TYPE],
          current_agent(num_productions_of_type)[DEFAULT_PRODUCTION_TYPE],
          current_agent(num_productions_of_type)[USER_PRODUCTION_TYPE],
          current_agent(num_productions_of_type)[CHUNK_PRODUCTION_TYPE]);
	m_result += buf;
    //print(a, "   + %lu justifications\n", current_agent(num_productions_of_type)[JUSTIFICATION_PRODUCTION_TYPE]);
	safeSprintf(buf, 127, "   + %lu justifications\n", current_agent(num_productions_of_type)[JUSTIFICATION_PRODUCTION_TYPE]);
	m_result += buf;

    /* REW: begin 28.07.96 */
#ifndef NO_TIMING_STUFF
    /* The fields for the timers are 8.3, providing an upper limit of 
       approximately 2.5 hours the printing of the run time calculations.  
       Obviously, these will need to be increased if you plan on needing 
       run-time data for a process that you expect to take longer than 
       2 hours. :) */

#ifndef KERNEL_TIME_ONLY
	if (current_agent(operand2_mode)) {
    	safeSprintf(buf, 127, "                                                        |   Computed\n");
    	m_result += buf;
 		safeSprintf(buf, 127, "Phases:      Input   Propose   Decide   Apply    Output |     Totals\n");
		m_result += buf;
 		safeSprintf(buf, 127, "========================================================|===========\n");
		m_result += buf;

		safeSprintf(buf, 127, "Kernel:   %8.3f %8.3f %8.3f %8.3f %8.3f  | %10.3f\n",
          timer_value(&current_agent(decision_cycle_phase_timers[INPUT_PHASE])),
          timer_value(&current_agent(decision_cycle_phase_timers[PROPOSE_PHASE])),
          timer_value(&current_agent(decision_cycle_phase_timers[DECISION_PHASE])),
          timer_value(&current_agent(decision_cycle_phase_timers[APPLY_PHASE])),
          timer_value(&current_agent(decision_cycle_phase_timers[OUTPUT_PHASE])), derived_kernel_time);
		m_result += buf;
	} else { 
		//print(a, "                                                                |    Derived\n");
		safeSprintf(buf, 127, "                                                        |   Computed\n");
		//print(a, "Phases:      Input      DLP     Pref      W/M   Output Decision |     Totals\n");
		safeSprintf(buf, 127, "Phases:      Input      Pref      W/M   Output Decision |     Totals\n");
		m_result += buf;
		//print(a, "================================================================|===========\n");
		safeSprintf(buf, 127, "========================================================|===========\n");
		m_result += buf;

		safeSprintf(buf, 127, "Kernel:   %8.3f %8.3f %8.3f %8.3f %8.3f | %10.3f\n",
          timer_value(&current_agent(decision_cycle_phase_timers[INPUT_PHASE])),
          timer_value(&current_agent(decision_cycle_phase_timers[PREFERENCE_PHASE])),
          timer_value(&current_agent(decision_cycle_phase_timers[WM_PHASE])),
          timer_value(&current_agent(decision_cycle_phase_timers[OUTPUT_PHASE])),
          timer_value(&current_agent(decision_cycle_phase_timers[DECISION_PHASE])), derived_kernel_time);
		m_result += buf;

	}

#ifdef DETAILED_TIMING_STATS

    //print(a, "====================  Detailed Timing Statistics  ==============|===========\n");
	safeSprintf(buf, 127, "====================  Detailed Timing Statistics  ==============|===========\n");
	m_result += buf;

    //print(a, "   Match: %8.3f %8.3f %8.3f %8.3f %8.3f %8.3f | %10.3f\n",
    //      timer_value(&current_agent(match_cpu_time[INPUT_PHASE])),
    //      timer_value(&current_agent(match_cpu_time[DETERMINE_LEVEL_PHASE])),
    //      timer_value(&current_agent(match_cpu_time[PREFERENCE_PHASE])),
    //      timer_value(&current_agent(match_cpu_time[WM_PHASE])),
    //      timer_value(&current_agent(match_cpu_time[OUTPUT_PHASE])),
    //      timer_value(&current_agent(match_cpu_time[DECISION_PHASE])), match_time);
	safeSprintf(buf, 127, "   Match: %8.3f %8.3f %8.3f %8.3f %8.3f %8.3f | %10.3f\n",
          timer_value(&current_agent(match_cpu_time[INPUT_PHASE])),
          timer_value(&current_agent(match_cpu_time[DETERMINE_LEVEL_PHASE])),
          timer_value(&current_agent(match_cpu_time[PREFERENCE_PHASE])),
          timer_value(&current_agent(match_cpu_time[WM_PHASE])),
          timer_value(&current_agent(match_cpu_time[OUTPUT_PHASE])),
          timer_value(&current_agent(match_cpu_time[DECISION_PHASE])), match_time);
	m_result += buf;

    //print(a, "Own'ship: %8.3f %8.3f %8.3f %8.3f %8.3f %8.3f | %10.3f\n",
    //      timer_value(&current_agent(ownership_cpu_time[INPUT_PHASE])),
    //      timer_value(&current_agent(ownership_cpu_time[DETERMINE_LEVEL_PHASE])),
    //      timer_value(&current_agent(ownership_cpu_time[PREFERENCE_PHASE])),
    //      timer_value(&current_agent(ownership_cpu_time[WM_PHASE])),
    //      timer_value(&current_agent(ownership_cpu_time[OUTPUT_PHASE])),
    //      timer_value(&current_agent(ownership_cpu_time[DECISION_PHASE])), ownership_time);
	safeSprintf(buf, 127, "Own'ship: %8.3f %8.3f %8.3f %8.3f %8.3f %8.3f | %10.3f\n",
          timer_value(&current_agent(ownership_cpu_time[INPUT_PHASE])),
          timer_value(&current_agent(ownership_cpu_time[DETERMINE_LEVEL_PHASE])),
          timer_value(&current_agent(ownership_cpu_time[PREFERENCE_PHASE])),
          timer_value(&current_agent(ownership_cpu_time[WM_PHASE])),
          timer_value(&current_agent(ownership_cpu_time[OUTPUT_PHASE])),
          timer_value(&current_agent(ownership_cpu_time[DECISION_PHASE])), ownership_time);
	m_result += buf;

    //print(a, "Chunking: %8.3f %8.3f %8.3f %8.3f %8.3f %8.3f | %10.3f\n",
    //      timer_value(&current_agent(chunking_cpu_time[INPUT_PHASE])),
    //      timer_value(&current_agent(chunking_cpu_time[DETERMINE_LEVEL_PHASE])),
    //      timer_value(&current_agent(chunking_cpu_time[PREFERENCE_PHASE])),
    //      timer_value(&current_agent(chunking_cpu_time[WM_PHASE])),
    //      timer_value(&current_agent(chunking_cpu_time[OUTPUT_PHASE])),
    //      timer_value(&current_agent(chunking_cpu_time[DECISION_PHASE])), chunking_time);
	safeSprintf(buf, 127, "Chunking: %8.3f %8.3f %8.3f %8.3f %8.3f %8.3f | %10.3f\n",
          timer_value(&current_agent(chunking_cpu_time[INPUT_PHASE])),
          timer_value(&current_agent(chunking_cpu_time[DETERMINE_LEVEL_PHASE])),
          timer_value(&current_agent(chunking_cpu_time[PREFERENCE_PHASE])),
          timer_value(&current_agent(chunking_cpu_time[WM_PHASE])),
          timer_value(&current_agent(chunking_cpu_time[OUTPUT_PHASE])),
          timer_value(&current_agent(chunking_cpu_time[DECISION_PHASE])), chunking_time);
	m_result += buf;

    //print(a, "   Other: %8.3f %8.3f %8.3f %8.3f %8.3f %8.3f | %10.3f\n",
    //      other_phase_kernel_time[INPUT_PHASE],
    //      other_phase_kernel_time[DETERMINE_LEVEL_PHASE],
    //      other_phase_kernel_time[PREFERENCE_PHASE],
    //      other_phase_kernel_time[WM_PHASE],
    //      other_phase_kernel_time[OUTPUT_PHASE], other_phase_kernel_time[DECISION_PHASE], other_total_kernel_time);
	safeSprintf(buf, 127, "   Other: %8.3f %8.3f %8.3f %8.3f %8.3f %8.3f | %10.3f\n",
          other_phase_kernel_time[INPUT_PHASE],
          other_phase_kernel_time[DETERMINE_LEVEL_PHASE],
          other_phase_kernel_time[PREFERENCE_PHASE],
          other_phase_kernel_time[WM_PHASE],
          other_phase_kernel_time[OUTPUT_PHASE], other_phase_kernel_time[DECISION_PHASE], other_total_kernel_time);
	m_result += buf;

    /* REW: begin 11.25.96 */
    //print(a, "Operand2: %8.3f %8.3f %8.3f %8.3f %8.3f %8.3f | %10.3f\n",
    //      timer_value(&current_agent(gds_cpu_time[INPUT_PHASE])),
    //      timer_value(&current_agent(gds_cpu_time[DETERMINE_LEVEL_PHASE])),
    //      timer_value(&current_agent(gds_cpu_time[PREFERENCE_PHASE])),
    //      timer_value(&current_agent(gds_cpu_time[WM_PHASE])),
    //      timer_value(&current_agent(gds_cpu_time[OUTPUT_PHASE])),
    //      timer_value(&current_agent(gds_cpu_time[DECISION_PHASE])),
    //      timer_value(&current_agent(gds_cpu_time[INPUT_PHASE])) +
    //      timer_value(&current_agent(gds_cpu_time[DETERMINE_LEVEL_PHASE])) +
    //      timer_value(&current_agent(gds_cpu_time[PREFERENCE_PHASE])) +
    //      timer_value(&current_agent(gds_cpu_time[WM_PHASE])) +
    //      timer_value(&current_agent(gds_cpu_time[OUTPUT_PHASE])) +
    //      timer_value(&current_agent(gds_cpu_time[DECISION_PHASE])));
	safeSprintf(buf, 127, "Operand2: %8.3f %8.3f %8.3f %8.3f %8.3f %8.3f | %10.3f\n",
          timer_value(&current_agent(gds_cpu_time[INPUT_PHASE])),
          timer_value(&current_agent(gds_cpu_time[DETERMINE_LEVEL_PHASE])),
          timer_value(&current_agent(gds_cpu_time[PREFERENCE_PHASE])),
          timer_value(&current_agent(gds_cpu_time[WM_PHASE])),
          timer_value(&current_agent(gds_cpu_time[OUTPUT_PHASE])),
          timer_value(&current_agent(gds_cpu_time[DECISION_PHASE])),
          timer_value(&current_agent(gds_cpu_time[INPUT_PHASE])) +
          timer_value(&current_agent(gds_cpu_time[DETERMINE_LEVEL_PHASE])) +
          timer_value(&current_agent(gds_cpu_time[PREFERENCE_PHASE])) +
          timer_value(&current_agent(gds_cpu_time[WM_PHASE])) +
          timer_value(&current_agent(gds_cpu_time[OUTPUT_PHASE])) +
          timer_value(&current_agent(gds_cpu_time[DECISION_PHASE])));
	m_result += buf;

    /* REW: end   11.25.96 */

#endif  // DETAILED_TIMING_STATS

    //print(a, "================================================================|===========\n");
	safeSprintf(buf, 127, "========================================================|===========\n");
	m_result += buf;

    //print(a, "Input fn: %8.3f                                              | %10.3f\n",
    //      input_function_time, input_function_time);
	safeSprintf(buf, 127, "Input fn: %8.3f                                      | %10.3f\n",
          input_function_time, input_function_time);
	m_result += buf;

    //print(a, "================================================================|===========\n");
	safeSprintf(buf, 127, "========================================================|===========\n");
    //print(a, "Outpt fn:                                     %8.3f          | %10.3f\n",
    //      output_function_time, output_function_time);
	m_result += buf;
	safeSprintf(buf, 127, "Outpt fn:                                     %8.3f  | %10.3f\n",
          output_function_time, output_function_time);
	m_result += buf;

    //print(a, "================================================================|===========\n");
	safeSprintf(buf, 127, "========================================================|===========\n");
	m_result += buf;

	//print(a, "Callbcks: %8.3f %8.3f %8.3f %8.3f %8.3f %8.3f | %10.3f\n",
 //         timer_value(&current_agent(monitors_cpu_time[INPUT_PHASE])),
 //         timer_value(&current_agent(monitors_cpu_time[DETERMINE_LEVEL_PHASE])),
 //         timer_value(&current_agent(monitors_cpu_time[PREFERENCE_PHASE])),
 //         timer_value(&current_agent(monitors_cpu_time[WM_PHASE])),
 //         timer_value(&current_agent(monitors_cpu_time[OUTPUT_PHASE])),
 //         timer_value(&current_agent(monitors_cpu_time[DECISION_PHASE])), monitors_sum);
	safeSprintf(buf, 127, "Callbcks: %8.3f %8.3f %8.3f %8.3f %8.3f  | %10.3f\n",
          timer_value(&current_agent(monitors_cpu_time[INPUT_PHASE])),
          timer_value(&current_agent(monitors_cpu_time[PROPOSE_PHASE])),
          timer_value(&current_agent(monitors_cpu_time[DECISION_PHASE])),
          timer_value(&current_agent(monitors_cpu_time[APPLY_PHASE])),
          timer_value(&current_agent(monitors_cpu_time[OUTPUT_PHASE])), monitors_sum);
	m_result += buf;

    //print(a, "================================================================|===========\n");
	safeSprintf(buf, 127, "========================================================|===========\n");
 	m_result += buf;
   //print(a, "Derived---------------------------------------------------------+-----------\n");
	safeSprintf(buf, 127, "Computed------------------------------------------------+-----------\n");
	m_result += buf;
    //print(a, "Totals:   %8.3f %8.3f %8.3f %8.3f %8.3f %8.3f | %10.3f\n\n",
    //      input_phase_total_time,
    //      determine_level_phase_total_time,
    //      preference_phase_total_time,
    //      wm_phase_total_time, output_phase_total_time, decision_phase_total_time, derived_total_cpu_time);
	safeSprintf(buf, 127, "Totals:   %8.3f %8.3f %8.3f %8.3f %8.3f  | %10.3f\n\n",
          input_phase_total_time,
          propose_phase_total_time,
          decision_phase_total_time,
          apply_phase_total_time, output_phase_total_time, derived_total_cpu_time);
	m_result += buf;

    //print(a, "Values from single timers:\n");
	safeSprintf(buf, 127, "Values from single timers:\n");
	m_result += buf;
#endif  // KERNEL_TIME_ONLY

#ifdef WARN_IF_TIMERS_REPORT_ZERO
    /* If a warning has occured since the last init-soar, the warn flag will
     * have been set to FALSE, so such a warning is not spammed to the screen
     * But lets repeat it here.
     */
	if (!current_agent(warn_on_zero_timers)) {
        //print(a, " Warning: one or more timers have reported zero during this run\n");
		safeSprintf(buf, 127, " Warning: one or more timers have reported zero during this run\n");
		m_result += buf;
	}

#endif

   //print(a, " Kernel CPU Time: %11.3f sec. \n", total_kernel_time);
   safeSprintf(buf, 127, " Kernel CPU Time: %11.3f sec. \n", total_kernel_time);
	m_result += buf;

   //print(a, " Total  CPU Time: %11.3f sec.\n\n", timer_value(&current_agent(total_cpu_time)));
   safeSprintf(buf, 127, " Total  CPU Time: %11.3f sec.\n\n", timer_value(&current_agent(total_cpu_time)));
	m_result += buf;

#ifdef COUNT_KERNEL_TIMER_STOPS
    //print(a, " Kernel CPU Timer Stops: %d\n", current_agent(kernelTimerStops));
	safeSprintf(buf, 127, " Kernel CPU Timer Stops: %d\n", current_agent(kernelTimerStops));
	m_result += buf;
    //print(a, " Non-Kernel Timer Stops: %d\n", current_agent(nonKernelTimerStops));
	safeSprintf(buf, 127, " Non-Kernel Timer Stops: %d\n", current_agent(nonKernelTimerStops));
	m_result += buf;
#endif

#endif  // #ifndef NO_TIMING_STUFF
		
	/* v8.6.2: print out decisions executed, not # full cycles */

#if !defined(NO_TIMING_STUFF)
    //print(a, "%lu decision cycles (%.3f msec/dc)\n",
    //      current_agent(d_cycle_count),
    //      current_agent(d_cycle_count) ? total_kernel_msec / current_agent(d_cycle_count) : 0.0);
	safeSprintf(buf, 127, "%lu decisions (%.3f msec/decision)\n",
          current_agent(decision_phases_count),
          current_agent(decision_phases_count) ? total_kernel_msec / current_agent(decision_phases_count) : 0.0);
	m_result += buf;
    //print(a, "%lu elaboration cycles (%.3f ec's per dc, %.3f msec/ec)\n",
    //      current_agent(e_cycle_count),
    //      current_agent(d_cycle_count) ? (double) current_agent(e_cycle_count) / current_agent(d_cycle_count) : 0,
    //      current_agent(e_cycle_count) ? total_kernel_msec / current_agent(e_cycle_count) : 0);
	safeSprintf(buf, 127, "%lu elaboration cycles (%.3f ec's per dc, %.3f msec/ec)\n",
          current_agent(e_cycle_count),
          current_agent(decision_phases_count) ? (double) current_agent(e_cycle_count) / current_agent(decision_phases_count) : 0,
          current_agent(e_cycle_count) ? total_kernel_msec / current_agent(e_cycle_count) : 0);
	m_result += buf;
    /* REW: begin 09.15.96 */

// voigtjr: removed because we don't use this
//#ifndef SOAR_8_ONLY
	if (current_agent(operand2_mode)) {
//#endif

		//print(a, "%lu p-elaboration cycles (%.3f pe's per dc, %.3f msec/pe)\n",
		//      current_agent(pe_cycle_count),
		//      current_agent(d_cycle_count) ? (double) current_agent(pe_cycle_count) / current_agent(d_cycle_count) : 0,
		//      current_agent(pe_cycle_count) ? total_kernel_msec / current_agent(pe_cycle_count) : 0);
		safeSprintf(buf, 127, "%lu p-elaboration cycles (%.3f pe's per dc, %.3f msec/pe)\n",
				current_agent(pe_cycle_count),
				current_agent(decision_phases_count) ? (double) current_agent(pe_cycle_count) / current_agent(decision_phases_count) : 0,
				current_agent(pe_cycle_count) ? total_kernel_msec / current_agent(pe_cycle_count) : 0);
		m_result += buf;
	}

    /* REW: end 09.15.96 */
    //print(a, "%lu production firings (%.3f pf's per ec, %.3f msec/pf)\n",
    //      current_agent(production_firing_count),
    //      current_agent(e_cycle_count) ? (double) current_agent(production_firing_count) /
    //      current_agent(e_cycle_count) : 0.0,
    //      current_agent(production_firing_count) ? total_kernel_msec / current_agent(production_firing_count) : 0.0);
	safeSprintf(buf, 127, "%lu production firings (%.3f pf's per ec, %.3f msec/pf)\n",
          current_agent(production_firing_count),
          current_agent(e_cycle_count) ? (double) current_agent(production_firing_count) /
          current_agent(e_cycle_count) : 0.0,
          current_agent(production_firing_count) ? total_kernel_msec / current_agent(production_firing_count) : 0.0);
	m_result += buf;

#else
    //print(a, "%lu decision cycles\n", current_agent(d_cycle_count));
	safeSprintf(buf, 127, "%lu decisions\n", current_agent(decision_phases_count));
	m_result += buf;
    //print(a, "%lu elaboration cycles \n", current_agent(e_cycle_count));
	safeSprintf(buf, 127, "%lu elaboration cycles \n", current_agent(e_cycle_count));
	m_result += buf;
    //print(a, "%lu production firings \n", current_agent(production_firing_count));
	safeSprintf(buf, 127, "%lu production firings \n", current_agent(production_firing_count));
	m_result += buf;
#endif                          /* !NO_TIMING_STUFF */

    wme_changes = current_agent(wme_addition_count) + current_agent(wme_removal_count);
    //print(a, "%lu wme changes (%lu additions, %lu removals)\n",
    //      wme_changes, current_agent(wme_addition_count), current_agent(wme_removal_count));
	safeSprintf(buf, 127, "%lu wme changes (%lu additions, %lu removals)\n",
          wme_changes, current_agent(wme_addition_count), current_agent(wme_removal_count));
	m_result += buf;

#ifdef DETAILED_TIMING_STATS
    //print(a, "    match time: %.3f msec/wm change\n", wme_changes ? match_msec / wme_changes : 0.0);
	safeSprintf(buf, 127, "    match time: %.3f msec/wm change\n", wme_changes ? match_msec / wme_changes : 0.0);
	m_result += buf;
#endif

    //print(a, "WM size: %lu current, %.3f mean, %lu maximum\n",
    //      current_agent(num_wmes_in_rete),
    //      (current_agent(num_wm_sizes_accumulated) ?
    //       (current_agent(cumulative_wm_size) / current_agent(num_wm_sizes_accumulated)) :
    //       0.0), current_agent(max_wm_size));
   safeSprintf(buf, 127, "WM size: %lu current, %.3f mean, %lu maximum\n",
          current_agent(num_wmes_in_rete),
          (current_agent(num_wm_sizes_accumulated) ?
           (current_agent(cumulative_wm_size) / current_agent(num_wm_sizes_accumulated)) :
           0.0), current_agent(max_wm_size));
	m_result += buf;

#ifndef NO_TIMING_STUFF
    //print(a, "\n");
	safeSprintf(buf, 127, "\n");
	m_result += buf;
    //print(a, "    *** Time/<x> statistics use the total kernel time from a ***\n");
	safeSprintf(buf, 127, "    *** Time/<x> statistics use the total kernel time from a    ***\n");
	m_result += buf;
    //print(a, "    *** single kernel timer.  Differences between this value ***\n");
	safeSprintf(buf, 127, "    *** single kernel timer.  Differences between this value    ***\n");
	m_result += buf;
    //print(a, "    *** and the derived total kernel time  are expected. See ***\n");
	safeSprintf(buf, 127, "    *** and the computed total kernel time  are expected. See   ***\n");
	m_result += buf;
    //print(a, "    *** help  for the  stats command  for more  information. ***\n");
	safeSprintf(buf, 127, "    *** help  for the  stats command  for more  information.    ***\n");
	m_result += buf;
	safeSprintf(buf, 127, "    *** The format fields for the timers are 8.3f, limiting     ***\n");
	m_result += buf;
	safeSprintf(buf, 127, "    *** printable values to a maximum of approximately 2.5 hrs. ***\n");
	m_result += buf;
	safeSprintf(buf, 127, "    *** See gSKI_AgentPerformanceMonitor.cpp to change the      ***\n");
	m_result += buf;
	safeSprintf(buf, 127, "    *** fieldwidths of the timers for runtime data > 2 hrs      ***\n");
	m_result += buf;
#endif
    /* REW: end 28.07.96 */

	// voigtjr: Uncomment to send to print callback
	//print(a, m_result.c_str());
}

/*
 *----------------------------------------------------------------------
 *
 * parse_memory_stats --
 *
 *	This procedure parses an argv array and prints the selected
 *      statistics.
 *
 *      The syntax being parsed is:
 *         stats -rete <mtype>
 *         <mtype> ::= -total
 *                     -overhead
 *                     -strings
 *                     -hash-table
 *                     -pool [-total | pool-name [<aspect>]]
 *                     -misc
 *        <aspect> ::= -used                   |M
 *                     -free                   |M
 *                     -item-size
 *                     -total-bytes
 *
 *          The items marked |M are available only when Soar has
 *          been compiled with the MEMORY_POOL_STATS option.
 *
 * Results:
 *	Returns the statistic and Tcl return code.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

bool AgentPerformanceMonitor::parse_memory_stats(int argc, char *argv[])
{
   agent* a = m_pAgent->GetSoarAgent();
    if (argc == 2) 
    {
        soar_ecPrintMemoryStatistics();
        soar_ecPrintMemoryPoolStatistics();

        return true;
    }

    if (!strcmp("-total", argv[2])) 
    {
        unsigned long total;
        int i;

        total = 0;
        for (i = 0; i < NUM_MEM_USAGE_CODES; i++)
            total += current_agent(memory_for_usage)[i];

        m_result = ToString(total);
    } 
    else if (!strcmp("-overhead", argv[2])) 
    {
        m_result = ToString(current_agent(memory_for_usage)[STATS_OVERHEAD_MEM_USAGE]);
    } 
    else if (!strcmp("-strings", argv[2])) 
    {
        m_result = ToString(current_agent(memory_for_usage)[STRING_MEM_USAGE]);
    } 
    else if (!strcmp("-hash-table", argv[2])) 
    {
        m_result = ToString(current_agent(memory_for_usage)[HASH_TABLE_MEM_USAGE]);
    } 
    else if (!strcmp("-pool", argv[2])) 
    {     /* Handle pool stats */
        if (argc == 3) 
        {
            soar_ecPrintMemoryPoolStatistics();
        } 
        else if (!strcmp("-total", argv[3])) 
        {
            m_result = ToString(current_agent(memory_for_usage)[POOL_MEM_USAGE]);
        } 
        else {                /* Match pool name or invalid item */
            memory_pool *p;
            memory_pool *pool_found = NIL;

            for (p = current_agent(memory_pools_in_use); p != NIL; p = p->next) 
            {
                if (!strcmp(argv[3], p->name)) 
                {
                    pool_found = p;
                    break;
                }
            }

            if (!pool_found) 
            {
               m_result = std::string("Unrecognized pool name: stats -memory -pool ") + argv[4];
                return false;
            }

            if (argc == 4) 
            {
#ifdef MEMORY_POOL_STATS
                long total_items;
#endif
                print(a, "Memory pool statistics:\n\n");
#ifdef MEMORY_POOL_STATS
                print(a, "Pool Name        Used Items  Free Items  Item Size  Total Bytes\n");
                print(a, "---------------  ----------  ----------  ---------  -----------\n");
#else
                print(a, "Pool Name        Item Size  Total Bytes\n");
                print(a, "---------------  ---------  -----------\n");
#endif

                print_string(a, pool_found->name);
                print_spaces(a, MAX_POOL_NAME_LENGTH - (int)strlen(pool_found->name));
#ifdef MEMORY_POOL_STATS
                print(a, "  %10lu", pool_found->used_count);
                total_items = pool_found->num_blocks * pool_found->items_per_block;
                print(a, "  %10lu", total_items - pool_found->used_count);
#endif
                print(a, "  %9lu", pool_found->item_size);
                print(a, "  %11lu\n", pool_found->num_blocks * pool_found->items_per_block * pool_found->item_size);
            } 
            else if (argc == 5) 
            {     /* get pool attribute */
                long total_items;

                total_items = pool_found->num_blocks * pool_found->items_per_block;

                if (!strcmp("-item-size", argv[4])) 
                {
                    m_result = ToString(pool_found->item_size);
                }
#ifdef MEMORY_POOL_STATS
                else if (!strcmp("-used", argv[4])) 
                {
                    m_result = ToString(pool_found->used_count);
                } 
                else if (!strcmp("-free", argv[4])) 
                {
                    m_result = ToString(total_items - pool_found->used_count);
                }
#endif
                else if (!strcmp("-total-bytes", argv[4])) 
                {
                    m_result = ToString(pool_found->num_blocks * pool_found->items_per_block * pool_found->item_size);
                } 
                else 
                {
                   m_result = std::string("Unrecognized argument to stats: -memory -pool ") + argv[3] + " " + argv[4];
                    return false;
                }
            } 
            else 
            {
                m_result = "Too many arguments, should be: stats -memory -pool [-total | pool-name [<aspect>]]";
                return false;
            }
        }
    } 
    else if (!strcmp("-misc", argv[2])) 
    {
        m_result = ToString(current_agent(memory_for_usage)[MISCELLANEOUS_MEM_USAGE]);
    } 
    else 
    {
        m_result = std::string("Unrecognized argument to stats: -memory ") + argv[2];
        return false;
    }

    return true;
}

/*
 *----------------------------------------------------------------------
 *
 * parse_rete_stats --
 *
 *	This procedure parses an argv array and prints the selected
 *      statistics.
 *
 *      The syntax being parsed is:
 *          stats -rete <rtype> <qualifier>
 *          <rtype> ::= unhashed memory
 *                      memory
 *                      unhashed mem-pos
 *                      mem-pos
 *                      unhashed negative
 *                      negative
 *                      unhashed positive
 *                      positive
 *                      dummy top
 *                      dummy matches
 *                      nconj. neg.
 *                      conj. neg. partner
 *                      production
 *                      total
 *          <qualifier> ::= -actual | -if-no-merging | -if-no-sharing
 *
 * Results:
 *	Returns the statistic and Tcl return code.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

bool AgentPerformanceMonitor::parse_rete_stats(int argc, char *argv[])
{
   agent* a = m_pAgent->GetSoarAgent();
    unsigned long data;

    if (argc == 2) 
    {
        soar_ecPrintReteStatistics();
        return true;
    }

    if (argc == 3) 
    {
        m_result = "Too few arguments, should be: stats -rete [type qualifier]";
        return false;
    }

    if (argc > 4) 
    {
        m_result = "Too many arguments, should be: stats -rete [type qualifier]";
        return false;
    }

    if (get_node_count_statistic(a, (char*) argv[2], (char *) argv[3] + 1, &data)) 
    {
        m_result = ToString(data);
    } 
    else 
    {
       m_result = std::string("Unrecognized argument to stats: -rete ") + argv[2] + " " + argv[3];
        return false;
    }
    return true;
}

void AgentPerformanceMonitor::soar_ecPrintReteStatistics()
{
   agent* a = m_pAgent->GetSoarAgent();

#ifdef TOKEN_SHARING_STATS
    print(a, "Token additions: %lu   If no sharing: %lu\n",
          current_agent(token_additions), current_agent(token_additions_without_sharing));
#endif

    print_node_count_statistics(a);
    print_null_activation_stats();
}

void AgentPerformanceMonitor::soar_ecPrintMemoryStatistics()
{
   agent* a = m_pAgent->GetSoarAgent();
    unsigned long total;
    int i;

    total = 0;
    for (i = 0; i < NUM_MEM_USAGE_CODES; i++)
        total += current_agent(memory_for_usage)[i];

    print(a, "%8lu bytes total memory allocated\n", total);
    print(a, "%8lu bytes statistics overhead\n", current_agent(memory_for_usage)[STATS_OVERHEAD_MEM_USAGE]);
    print(a, "%8lu bytes for strings\n", current_agent(memory_for_usage)[STRING_MEM_USAGE]);
    print(a, "%8lu bytes for hash tables\n", current_agent(memory_for_usage)[HASH_TABLE_MEM_USAGE]);
    print(a, "%8lu bytes for various memory pools\n", current_agent(memory_for_usage)[POOL_MEM_USAGE]);
    print(a, "%8lu bytes for miscellaneous other things\n", current_agent(memory_for_usage)[MISCELLANEOUS_MEM_USAGE]);
}

void AgentPerformanceMonitor::soar_ecPrintMemoryPoolStatistics()
{
   agent* a = m_pAgent->GetSoarAgent();
    memory_pool *p;
#ifdef MEMORY_POOL_STATS
    long total_items;
#endif

    print(a, "Memory pool statistics:\n\n");
#ifdef MEMORY_POOL_STATS
    print(a, "Pool Name        Used Items  Free Items  Item Size  Total Bytes\n");
    print(a, "---------------  ----------  ----------  ---------  -----------\n");
#else
    print(a, "Pool Name        Item Size  Total Bytes\n");
    print(a, "---------------  ---------  -----------\n");
#endif

    for (p = current_agent(memory_pools_in_use); p != NIL; p = p->next) {
        print_string(a, p->name);
        print_spaces(a, MAX_POOL_NAME_LENGTH - (int)strlen(p->name));
#ifdef MEMORY_POOL_STATS
        print(a, "  %10lu", p->used_count);
        total_items = p->num_blocks * p->items_per_block;
        print(a, "  %10lu", total_items - p->used_count);
#endif
        print(a, "  %9lu", p->item_size);
        print(a, "  %11lu\n", p->num_blocks * p->items_per_block * p->item_size);
    }
}

void AgentPerformanceMonitor::print_null_activation_stats()
{
   agent* a = m_pAgent->GetSoarAgent();
    print(a, "\nActivations: %lu right (%lu null), %lu left (%lu null)\n",
          current_agent(num_right_activations),
          current_agent(num_null_right_activations),
          current_agent(num_left_activations), current_agent(num_null_left_activations));
}

void AgentPerformanceMonitor::printTimingInfo()
{
   agent* a = m_pAgent->GetSoarAgent();
#ifdef PII_TIMERS
    unsigned long long int start, stop;
#else
    struct timeval start, stop;
#endif

    double min=0.0, max=0.0, min_nz=0.0;
#ifndef NO_TIMING_STUFF
    min_nz = soar_cDetermineTimerResolution(&min, &max);
#endif

#ifdef PII_TIMERS
    print(a, "Using Pentium II Time Stamp -- Assuming %d MHZ Processor...\n", MHZ);
#else
    print(a, "Using system timers...\n");
#endif

    print(a, "---------------------------------------------------------------\n");
    print(a, "A timing loop is used to obtain the following values.\n");
    print(a, "At least one additional instruction takes place between\n");
    print(a, "starting and stopping the timers, thus a perfectly accurate\n");
    print(a, "timer which costs nothing to invoke would still accumulate\n");
    print(a, "non-zero value. The loop runs for a total of ~2 seconds as \n");
    print(a, "a result, the Maximum timer value is likely to be relatively \n");
    print(a, "large, but should be < 2 seconds\n");
    print(a, "** NOTE: If the code was optimized, the timing loop may yield\n");
    print(a, "         unanticipated results.  If both minimum values are\n");
    print(a, "         zero, it is unclear what the timer resolution is...\n");
    print(a, "---------------------------------------------------------------\n");
    print(a, "Minimum (Non-Zero) Timer Value: %11.5e sec\n", min_nz);
    print(a, "Minimum Timer Value           : %11.5e sec\n", min);
    print(a, "Maximum Timer Value           : %11.5e sec\n\n", max);

    print(a, "---------------------------------------------------------------\n");
    print(a, "A short delay will be issued using the sleep() command, and \n");
    print(a, "timed using Soar's timers....\n");
    reset_timer(&stop);
    start_timer(a, &start);
    sys_sleep(3);
    stop_timer(a, &start, &stop);
    print(a, "Sleep interval  -->   3 seconds\n");
#ifndef NO_TIMING_STUFF
    print(a, "Timers report   -->  %8.5f seconds\n", timer_value(&stop));
#endif
    print(a, "---------------------------------------------------------------\n");

}

void AgentPerformanceMonitor::GetStats(AgentPerformanceData* pStats)
{
	if (!pStats) return;

	agent* a = m_pAgent->GetSoarAgent();

	pStats->productionCountDefault = a->num_productions_of_type[DEFAULT_PRODUCTION_TYPE];
	pStats->productionCountUser = a->num_productions_of_type[USER_PRODUCTION_TYPE];
	pStats->productionCountChunk = a->num_productions_of_type[CHUNK_PRODUCTION_TYPE];
	pStats->productionCountJustification = a->num_productions_of_type[JUSTIFICATION_PRODUCTION_TYPE];
	pStats->cycleCountDecision = a->decision_phases_count;
	pStats->cycleCountElaboration = a->e_cycle_count;
	pStats->productionFiringCount = a->production_firing_count;
	pStats->wmeCountAddition = a->wme_addition_count;
	pStats->wmeCountRemoval = a->wme_removal_count;
	pStats->wmeCount = a->num_wmes_in_rete;
	pStats->wmeCountAverage = a->num_wm_sizes_accumulated ? (a->cumulative_wm_size / a->num_wm_sizes_accumulated) : 0.0;
	pStats->wmeCountMax = a->max_wm_size;

	pStats->kernelCPUTime = timer_value(&a->total_kernel_time);
	pStats->totalCPUTime = timer_value(&a->total_cpu_time);

    pStats->phaseTimeInputPhase = timer_value(&a->decision_cycle_phase_timers[INPUT_PHASE]);
    pStats->phaseTimeProposePhase = timer_value(&a->decision_cycle_phase_timers[PROPOSE_PHASE]);
    pStats->phaseTimeDecisionPhase = timer_value(&a->decision_cycle_phase_timers[DECISION_PHASE]);
    pStats->phaseTimeApplyPhase = timer_value(&a->decision_cycle_phase_timers[APPLY_PHASE]);
    pStats->phaseTimeOutputPhase = timer_value(&a->decision_cycle_phase_timers[OUTPUT_PHASE]);
    pStats->phaseTimePreferencePhase = timer_value(&a->decision_cycle_phase_timers[PREFERENCE_PHASE]);
    pStats->phaseTimeWorkingMemoryPhase = timer_value(&a->decision_cycle_phase_timers[WM_PHASE]);

    pStats->monitorTimeInputPhase = timer_value(&a->monitors_cpu_time[INPUT_PHASE]);
    pStats->monitorTimeProposePhase = timer_value(&a->monitors_cpu_time[PROPOSE_PHASE]);
    pStats->monitorTimeDecisionPhase = timer_value(&a->monitors_cpu_time[DECISION_PHASE]);
    pStats->monitorTimeApplyPhase = timer_value(&a->monitors_cpu_time[APPLY_PHASE]);
    pStats->monitorTimeOutputPhase = timer_value(&a->monitors_cpu_time[OUTPUT_PHASE]);
    pStats->monitorTimePreferencePhase = timer_value(&a->monitors_cpu_time[PREFERENCE_PHASE]);
    pStats->monitorTimeWorkingMemoryPhase = timer_value(&a->monitors_cpu_time[WM_PHASE]);
 
    pStats->inputFunctionTime = timer_value(&a->input_function_cpu_time) ;
    pStats->outputFunctionTime = timer_value(&a->output_function_cpu_time) ;
 
  // Match_cpu_time, ownership, and chunking are only calculated when
  // DETAILED_TIMING_STATS is defined.
	pStats->matchTimeInputPhase = timer_value(&a->match_cpu_time[INPUT_PHASE]);
	pStats->matchTimeProposePhase = timer_value(&a->match_cpu_time[PROPOSE_PHASE]);
	pStats->matchTimeApplyPhase = timer_value(&a->match_cpu_time[APPLY_PHASE]);
	pStats->matchTimePreferencePhase = timer_value(&a->match_cpu_time[PREFERENCE_PHASE]);
	pStats->matchTimeWorkingMemoryPhase = timer_value(&a->match_cpu_time[WM_PHASE]);
	pStats->matchTimeOutputPhase = timer_value(&a->match_cpu_time[OUTPUT_PHASE]);
	pStats->matchTimeDecisionPhase = timer_value(&a->match_cpu_time[DECISION_PHASE]);

	pStats->ownershipTimeInputPhase = timer_value(&a->ownership_cpu_time[INPUT_PHASE]);
	pStats->ownershipTimeProposePhase = timer_value(&a->ownership_cpu_time[PROPOSE_PHASE]);
	pStats->ownershipTimeApplyPhase = timer_value(&a->ownership_cpu_time[APPLY_PHASE]);
	pStats->ownershipTimePreferencePhase = timer_value(&a->ownership_cpu_time[PREFERENCE_PHASE]);
	pStats->ownershipTimeWorkingMemoryPhase = timer_value(&a->ownership_cpu_time[WM_PHASE]);
	pStats->ownershipTimeOutputPhase = timer_value(&a->ownership_cpu_time[OUTPUT_PHASE]);
	pStats->ownershipTimeDecisionPhase = timer_value(&a->ownership_cpu_time[DECISION_PHASE]);

	pStats->chunkingTimeInputPhase = timer_value(&a->chunking_cpu_time[INPUT_PHASE]);
	pStats->chunkingTimeProposePhase = timer_value(&a->chunking_cpu_time[PROPOSE_PHASE]);
	pStats->chunkingTimeApplyPhase = timer_value(&a->chunking_cpu_time[APPLY_PHASE]);
	pStats->chunkingTimePreferencePhase = timer_value(&a->chunking_cpu_time[PREFERENCE_PHASE]);
	pStats->chunkingTimeWorkingMemoryPhase = timer_value(&a->chunking_cpu_time[WM_PHASE]);
	pStats->chunkingTimeOutputPhase = timer_value(&a->chunking_cpu_time[OUTPUT_PHASE]);
	pStats->chunkingTimeDecisionPhase = timer_value(&a->chunking_cpu_time[DECISION_PHASE]);

	pStats->memoryUsageMiscellaneous = a->memory_for_usage[MISCELLANEOUS_MEM_USAGE];
	pStats->memoryUsageHash = a->memory_for_usage[HASH_TABLE_MEM_USAGE];
	pStats->memoryUsageString = a->memory_for_usage[STRING_MEM_USAGE];
	pStats->memoryUsagePool = a->memory_for_usage[POOL_MEM_USAGE];
	pStats->memoryUsageStatsOverhead = a->memory_for_usage[STATS_OVERHEAD_MEM_USAGE];

	// TODO: pool and rete stats
}


