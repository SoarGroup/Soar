/*************************************************************************
 *
 *  file:  init_soar.c
 *
 * =======================================================================
 *  Routines for initializing Soar, signal handling (ctrl-c interrupt),
 *  exiting Soar (cleanup and error msgs), setting sysparams, and
 *  the core routines for "running" Soar (do_one_top_level_phase, etc.)
 * =======================================================================
 *
 * Copyright 1995-2003 Carnegie Mellon University,
 *										 University of Michigan,
 *										 University of Southern California/Information
 *										 Sciences Institute. All rights reserved.
 *										
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1.	Redistributions of source code must retain the above copyright notice,
 *		this list of conditions and the following disclaimer. 
 * 2.	Redistributions in binary form must reproduce the above copyright notice,
 *		this list of conditions and the following disclaimer in the documentation
 *		and/or other materials provided with the distribution. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE SOAR CONSORTIUM ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE SOAR CONSORTIUM  OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * The views and conclusions contained in the software and documentation are
 * those of the authors and should not be interpreted as representing official
 * policies, either expressed or implied, of Carnegie Mellon University, the
 * University of Michigan, the University of Southern California/Information
 * Sciences Institute, or the Soar consortium.
 * =======================================================================
 */


#include <signal.h>         /* used for control-c handler */
#include "soarkernel.h"

#if !defined(__SC__) && !defined(THINK_C) && !defined(WIN32) && !defined(MACINTOSH)
#include <sys/time.h>       /* used for timing stuff */
#include <sys/resource.h>   /* used for timing stuff */
#endif /* !__SC__ && !THINK_C && !WIN32 */

#ifdef __hpux
#include <sys/syscall.h>
#include <unistd.h>
#define getrusage(a, b) syscall(SYS_GETRUSAGE, a, b)
#define getwd(arg) getcwd(arg, (size_t) 9999)
#endif /* __hpux */

#ifdef _WINDOWS
#define INIT_FILE       "init.soa"
#else
#define INIT_FILE       ".init.soar"
#endif


/* REW: begin 08.20.97   these defined in consistency.c  */
extern void determine_highest_active_production_level_in_stack_propose();
extern initialize_consistency_calculations_for_new_decision();
extern void determine_highest_active_production_level_in_stack_apply();
/* REW: end   08.20.97 */

#if (defined(REAL_TIME_BEHAVIOR) || defined(ATTENTION_LAPSE))
/* RMJ; just a temporary variable, but we don't want to
      reallocate it every time we process a phase, so we make it global
      and allocate memory for it in init_soar() (init agent.c) */
struct timeval *current_real_time;
#endif

#ifdef ATTENTION_LAPSE
/* RMJ; just a temporary variable, but we don't want to
      reallocate it every time we process a phase, so we make it global */
long lapse_duration;
#endif

/* ===================================================================

                            Exiting Soar

   Exit_soar() and abort_with_fatal_error(msg) both terminate Soar, closing
   the log file before exiting.  Abort_with_fatal_error(msg) also prints
   an error message and tries to write a file before exiting.
=================================================================== */

void just_before_exit_soar (void) {
  soar_invoke_callbacks(soar_agent, 
			SYSTEM_TERMINATION_CALLBACK,
			(soar_call_data) TRUE);
  if (current_agent(logging_to_file)) stop_log_file ();
}

void exit_soar (void) {
#ifdef _WINDOWS
  print("Cannot exit from Soar via the command line.\n");
#else
  just_before_exit_soar();
  exit (0);
#endif
}

void old_abort_with_fatal_error (void) {
  print ("Soar cannot recover from this error.  Aborting...\n");
  soar_invoke_callbacks(soar_agent, 
			SYSTEM_TERMINATION_CALLBACK,
			(soar_call_data) FALSE);		       
  if (current_agent(logging_to_file)) stop_log_file ();
#ifdef _WINDOWS
  Terminate(1);
#else
  exit (1);
#endif
}

void abort_with_fatal_error (char *msg) {
  FILE *f;

  print ("%s",msg);
  print ("Soar cannot recover from this error.  Aborting...\n");
  fprintf (stderr,"%s",msg);
  fprintf (stderr,"Soar cannot recover from this error.  Aborting...\n");
  f = fopen("soarerror", "w");
  fprintf (f,"%s",msg);
  fprintf (f,"Soar cannot recover from this error.  Aborting...\n");
  fclose(f);
  soar_invoke_callbacks(soar_agent, 
			SYSTEM_TERMINATION_CALLBACK,
			(soar_call_data) FALSE);		       
  if (current_agent(logging_to_file)) stop_log_file ();
#ifdef _WINDOWS
  Terminate(1);
#else
  abort ();
#endif
}

/* ===================================================================
   
                        Signal Handling

   Setup things so control_c_handler() gets control whenever the program
   receives a SIGINT (e.g., from a ctrl-c at the keyboard).  The handler
   just sets the stop_soar flag.
=================================================================== */

char * c_interrupt_msg = "*** Ctrl-C Interrupt ***";

/* AGR 581  The variable the_signal is not used at all, so I thought I
   would remove it.  Unfortunately, the signal command at the end of this
   function requires a function name that has a single integer parameter.
   It's probably some unix thing.  So I left the variable in the parameter
   list and instead changed the calling functions to use a parameter.
   94.11.15 (although this was done a month or two earlier--this comment
   was placed here in retrospect.)  */

void control_c_handler (int the_signal) {
/* Windows 3.1 can't do ^C handling */
#ifndef _WINDOWS

  cons * c;
  agent * the_agent;

  for(c = all_soar_agents; c != NIL; c = c->rest) {
    the_agent = ((agent *) c->first);
    the_agent->stop_soar = TRUE;
    the_agent->reason_for_stopping =  c_interrupt_msg;
  }

  /* --- reinstall this signal handler -- some brain-damaged OS's uninstall
     it after delivering the signal --- */
  signal (SIGINT, control_c_handler);

#endif
}

void setup_signal_handling (void) {
#ifndef _WINDOWS
  if (signal(SIGINT, control_c_handler) == SIG_ERR) {
    fprintf(stderr, "setup_signal_handling: unable to install signal handler.\n");
    fprintf(stderr, "                       Ctrl-C will not interrupt Soar.\n");
  }

#endif /* _WINDOWS */
}

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

#if defined(MACINTOSH) || defined(WIN32)

void get_cputime_from_clock(clock_t t, struct timeval *dt) {
  dt->tv_sec = t / CLOCKS_PER_SEC;
  dt->tv_usec = (long)(((t % CLOCKS_PER_SEC) / (float) CLOCKS_PER_SEC) * ONE_MILLION);
}

void start_timer (struct timeval *tv_for_recording_start_time) {
  clock_t ticks;
  
  ticks = clock();
  get_cputime_from_clock(ticks,tv_for_recording_start_time);
}

void stop_timer (struct timeval *tv_with_recorded_start_time,
                 struct timeval *tv_with_accumulated_time) {
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
}

/* stolen from linux time.h file for attention lapse and real time stuff */
#define timersub(a, b, result)                                                \
  do {                                                                        \
    (result)->tv_sec = (a)->tv_sec - (b)->tv_sec;                             \
    (result)->tv_usec = (a)->tv_usec - (b)->tv_usec;                          \
    if ((result)->tv_usec < 0) {                                              \
      --(result)->tv_sec;                                                     \
      (result)->tv_usec += 1000000;                                           \
    }                                                                         \
  } while (0)


#else

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

#endif /* MACINTOSH */

double timer_value (struct timeval *tv) {
  return (double)(tv->tv_sec) + (double)(tv->tv_usec)/(double)ONE_MILLION;
}
#endif

#ifdef REAL_TIME_BEHAVIOR
/* RMJ */
void init_real_time (void) {
   current_agent(real_time_tracker) =
         (struct timeval *) malloc(sizeof(struct timeval));
   timerclear(current_agent(real_time_tracker));
   current_agent(real_time_idling) = FALSE;
   current_real_time =
         (struct timeval *) malloc(sizeof(struct timeval));
}
#endif

#ifdef ATTENTION_LAPSE
/* RMJ */

void wake_from_attention_lapse (void) {
   /* Set tracker to last time we woke up */
   start_timer (current_agent(attention_lapse_tracker));
   current_agent(attention_lapsing) = FALSE;
}

void init_attention_lapse (void) {
   current_agent(attention_lapse_tracker) =
         (struct timeval *) malloc(sizeof(struct timeval));
   wake_from_attention_lapse();
#ifndef REAL_TIME_BEHAVIOR
   current_real_time =
         (struct timeval *) malloc(sizeof(struct timeval));
#endif
}

void start_attention_lapse (long duration) {
   /* Set tracker to time we should wake up */
   start_timer (current_agent(attention_lapse_tracker));
   current_agent(attention_lapse_tracker)->tv_usec += 1000 * duration;
   if (current_agent(attention_lapse_tracker)->tv_usec >= 1000000) {
      current_agent(attention_lapse_tracker)->tv_sec +=
            current_agent(attention_lapse_tracker)->tv_usec / 1000000;
      current_agent(attention_lapse_tracker)->tv_usec %= 1000000;
   }
   current_agent(attention_lapsing) = TRUE;
}
   
#endif

/* ===================================================================
   
                            Sysparams

=================================================================== */


void set_sysparam (int param_number, long new_value) {
	if ((param_number < 0) || (param_number > HIGHEST_SYSPARAM_NUMBER)) {
		print ("Internal error: tried to set bad sysparam #: %d\n", param_number);
		return;
	}
	current_agent(sysparams)[param_number] = new_value;
	
    #ifndef NO_CALLBACKS /* kjc 1/00 */
	soar_invoke_callbacks(soar_agent, 
		SYSTEM_PARAMETER_CHANGED_CALLBACK,
		(soar_call_data) param_number);		
    #endif
}

void init_sysparams (void) {
  int i;

  for (i=0; i<HIGHEST_SYSPARAM_NUMBER+1; i++) current_agent(sysparams)[i] = 0;
  
  /* --- set all params to zero, except the following: --- */
  current_agent(sysparams)[TRACE_CONTEXT_DECISIONS_SYSPARAM] = TRUE;
  current_agent(sysparams)[TRACE_FIRINGS_OF_CHUNKS_SYSPARAM] = TRUE;
  current_agent(sysparams)[TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM] = NONE_WME_TRACE;
  current_agent(sysparams)[TRACE_CHUNK_NAMES_SYSPARAM] = TRUE;
  current_agent(sysparams)[TRACE_JUSTIFICATION_NAMES_SYSPARAM] = TRUE;
  current_agent(sysparams)[TRACE_LOADING_SYSPARAM] = TRUE; /* KJC 8/96 */
  current_agent(sysparams)[MAX_ELABORATIONS_SYSPARAM] = 100;
  current_agent(sysparams)[MAX_CHUNKS_SYSPARAM] = 50;
  current_agent(sysparams)[MAX_NIL_OUTPUT_CYCLES_SYSPARAM] = 15;

#ifdef USE_X_DISPLAY
  current_agent(sysparams)[RESPOND_TO_LOAD_ERRORS_SYSPARAM] = FALSE;
#else
  current_agent(sysparams)[RESPOND_TO_LOAD_ERRORS_SYSPARAM] = TRUE;
#endif

#ifdef ATTENTION_LAPSE
  /* RMJ */
  current_agent(sysparams)[ATTENTION_LAPSE_ON_SYSPARAM] = FALSE;
#endif /* ATTENTION_LAPSE */

  current_agent(sysparams)[LEARNING_ON_SYSPARAM] = TRUE;
  current_agent(sysparams)[LEARNING_ONLY_SYSPARAM] = FALSE;  /* AGR MVL1 */
  current_agent(sysparams)[LEARNING_EXCEPT_SYSPARAM] = FALSE;  /* KJC 8/96 */
  current_agent(sysparams)[LEARNING_ALL_GOALS_SYSPARAM] = TRUE;
  current_agent(sysparams)[USER_SELECT_MODE_SYSPARAM] = USER_SELECT_RANDOM;
  current_agent(sysparams)[PRINT_WARNINGS_SYSPARAM] = TRUE;
  current_agent(sysparams)[PRINT_ALIAS_SYSPARAM] = TRUE;  /* AGR 627 */
  current_agent(sysparams)[EXPLAIN_SYSPARAM] = FALSE; /* KJC 7/96 */
  current_agent(sysparams)[USE_LONG_CHUNK_NAMES] = TRUE;  /* kjh(B14) */
  current_agent(sysparams)[TRACE_OPERAND2_REMOVALS_SYSPARAM] = FALSE;
}

/* ===================================================================
   
                     Adding and Removing Pwatchs

   Productions_being_traced is a (consed) list of all productions
   on which a pwatch has been set.  Pwatchs are added/removed via
   calls to add_pwatch() and remove_pwatch().
=================================================================== */
/* list of production structures */


void add_pwatch (production *prod) {
  if (prod->trace_firings) return;
  prod->trace_firings = TRUE;
  push (prod, current_agent(productions_being_traced));
}

production *prod_to_remove_pwatch_of;

bool remove_pwatch_test_fn (cons *c) {
  return (c->first == prod_to_remove_pwatch_of);
}

void remove_pwatch (production *prod) {
  if (! prod->trace_firings) return;
  prod->trace_firings = FALSE;
  prod_to_remove_pwatch_of = prod;
  free_list (extract_list_elements (&current_agent(productions_being_traced),
                                    remove_pwatch_test_fn));
}

/* ===================================================================
   
                         Reinitializing Soar

   Reset_statistics() resets all the statistics (except the firing counts
   on each individual production).  Reinitialize_soar() does all the 
   work for an init-soar.
=================================================================== */

void reset_production_firing_counts(void) {
  int t;
  production * p;

  for (t = 0; t < NUM_PRODUCTION_TYPES; t++) {
    for (p = current_agent(all_productions_of_type)[t]; 
	 p != NIL; 
	 p = p->next)
      p->firing_count = 0;
  }
}

void reset_statistics (void) {

  current_agent(d_cycle_count) = 0;
  current_agent(e_cycle_count) = 0;
  current_agent(e_cycles_this_d_cycle) = 0;
  current_agent(chunks_this_d_cycle) = 0;
  current_agent(production_firing_count) = 0;
  current_agent(wme_addition_count) = 0;
  current_agent(wme_removal_count) = 0;
  current_agent(max_wm_size) = 0;
  current_agent(cumulative_wm_size) = 0.0;
  current_agent(num_wm_sizes_accumulated) = 0;
/* REW: begin 09.15.96 */
  current_agent(pe_cycle_count) = 0;
  current_agent(pe_cycles_this_d_cycle) = 0;
/* REW: end   09.15.96 */


  reset_production_firing_counts();

#ifndef NO_TIMING_STUFF
  reset_timer (&current_agent(total_cpu_time));


/* REW: begin 28.07.96 */

  reset_timer (&current_agent(total_kernel_time));

  reset_timer (&current_agent(decision_cycle_phase_timers[INPUT_PHASE]));
  reset_timer (&current_agent(decision_cycle_phase_timers[DETERMINE_LEVEL_PHASE]));
  reset_timer (&current_agent(decision_cycle_phase_timers[PREFERENCE_PHASE]));
  reset_timer (&current_agent(decision_cycle_phase_timers[WM_PHASE]));
  reset_timer (&current_agent(decision_cycle_phase_timers[OUTPUT_PHASE]));
  reset_timer (&current_agent(decision_cycle_phase_timers[DECISION_PHASE]));

  reset_timer (&current_agent(monitors_cpu_time[INPUT_PHASE]));
  reset_timer (&current_agent(monitors_cpu_time[DETERMINE_LEVEL_PHASE]));
  reset_timer (&current_agent(monitors_cpu_time[PREFERENCE_PHASE]));
  reset_timer (&current_agent(monitors_cpu_time[WM_PHASE]));
  reset_timer (&current_agent(monitors_cpu_time[OUTPUT_PHASE]));
  reset_timer (&current_agent(monitors_cpu_time[DECISION_PHASE]));

  reset_timer (&current_agent(input_function_cpu_time));
  reset_timer (&current_agent(output_function_cpu_time));


 #ifdef DETAILED_TIMING_STATS
  reset_timer (&current_agent(match_cpu_time[INPUT_PHASE]));
  reset_timer (&current_agent(match_cpu_time[DETERMINE_LEVEL_PHASE]));
  reset_timer (&current_agent(match_cpu_time[PREFERENCE_PHASE]));
  reset_timer (&current_agent(match_cpu_time[WM_PHASE]));
  reset_timer (&current_agent(match_cpu_time[OUTPUT_PHASE]));
  reset_timer (&current_agent(match_cpu_time[DECISION_PHASE]));

  reset_timer (&current_agent(ownership_cpu_time[INPUT_PHASE]));
  reset_timer (&current_agent(ownership_cpu_time[DETERMINE_LEVEL_PHASE]));
  reset_timer (&current_agent(ownership_cpu_time[PREFERENCE_PHASE]));
  reset_timer (&current_agent(ownership_cpu_time[WM_PHASE]));
  reset_timer (&current_agent(ownership_cpu_time[OUTPUT_PHASE]));
  reset_timer (&current_agent(ownership_cpu_time[DECISION_PHASE]));

  reset_timer (&current_agent(chunking_cpu_time[INPUT_PHASE]));
  reset_timer (&current_agent(chunking_cpu_time[DETERMINE_LEVEL_PHASE]));
  reset_timer (&current_agent(chunking_cpu_time[PREFERENCE_PHASE]));
  reset_timer (&current_agent(chunking_cpu_time[WM_PHASE]));
  reset_timer (&current_agent(chunking_cpu_time[OUTPUT_PHASE]));
  reset_timer (&current_agent(chunking_cpu_time[DECISION_PHASE]));

/* REW: begin 11.25.96 */ 
  reset_timer (&current_agent(total_gds_time));

  reset_timer (&current_agent(gds_cpu_time[INPUT_PHASE]));     
  reset_timer (&current_agent(gds_cpu_time[DETERMINE_LEVEL_PHASE]));
  reset_timer (&current_agent(gds_cpu_time[PREFERENCE_PHASE]));
  reset_timer (&current_agent(gds_cpu_time[WM_PHASE]));        
  reset_timer (&current_agent(gds_cpu_time[OUTPUT_PHASE]));    
  reset_timer (&current_agent(gds_cpu_time[DECISION_PHASE]));  
/* REW: end   11.25.96 */ 

 #endif
#endif
/* REW: end 28.07.96 */
}

void reinitialize_soar (void) {

  /* kjh (CUSP-B4) begin */
  long cur_TRACE_CONTEXT_DECISIONS_SYSPARAM;
  long cur_TRACE_PHASES_SYSPARAM;
  long cur_TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM;
  long cur_TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM;
  long cur_TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM;
  long cur_TRACE_FIRINGS_PREFERENCES_SYSPARAM;
  long cur_TRACE_WM_CHANGES_SYSPARAM;
  /* kjh (CUSP-B4) end */

  current_agent(did_PE) = FALSE;    /* RCHONG:  10.11 */

  #ifndef NO_CALLBACKS /* kjc 1/00 */
  soar_invoke_callbacks(soar_agent, 
		       BEFORE_INIT_SOAR_CALLBACK,
		       (soar_call_data) NULL);		 
  #endif

  /* kjh (CUSP-B4) begin */
  /* Stash trace state: */
  cur_TRACE_CONTEXT_DECISIONS_SYSPARAM        = current_agent(sysparams)[TRACE_CONTEXT_DECISIONS_SYSPARAM];
  cur_TRACE_PHASES_SYSPARAM                   = current_agent(sysparams)[TRACE_PHASES_SYSPARAM];
  cur_TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM = current_agent(sysparams)[TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM];
  cur_TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM    = current_agent(sysparams)[TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM];
  cur_TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM   = current_agent(sysparams)[TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM];
  cur_TRACE_FIRINGS_PREFERENCES_SYSPARAM      = current_agent(sysparams)[TRACE_FIRINGS_PREFERENCES_SYSPARAM];
  cur_TRACE_WM_CHANGES_SYSPARAM               = current_agent(sysparams)[TRACE_WM_CHANGES_SYSPARAM];

  /* Temporarily disable tracing: */
  set_sysparam(TRACE_CONTEXT_DECISIONS_SYSPARAM,        FALSE);
  set_sysparam(TRACE_PHASES_SYSPARAM,                   FALSE);
  set_sysparam(TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM, FALSE);
  set_sysparam(TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM,    FALSE);
  set_sysparam(TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM,   NONE_WME_TRACE);
  set_sysparam(TRACE_FIRINGS_PREFERENCES_SYSPARAM,      FALSE);
  set_sysparam(TRACE_WM_CHANGES_SYSPARAM,               FALSE);
  /* kjh (CUSP-B4) end */

  clear_goal_stack ();

  if (current_agent(operand2_mode) == TRUE) {
     current_agent(active_level) = 0; /* Signal that everything should be retracted */
     current_agent(FIRING_TYPE) = IE_PRODS;
     do_preference_phase ();   /* allow all i-instantiations to retract */

     /* REW: begin  09.22.97 */

     /* In Operand2,  any retractions, regardless of i-instantitations or
	o-instantitations, are retracted at the saem time (in an IE_PRODS
	phase).  So one call to the preference phase should be sufficient. */

     /* DELETED code to set FIRING_TYPE to PE and call preference phase. */

     /* REW: end    09.22.97 */
  }

  /* REW: end  09.15.96 */
  else
  do_preference_phase ();   /* allow all instantiations to retract */

  reset_explain();
  reset_id_counters ();
  reset_wme_timetags ();
  reset_statistics ();
  current_agent(system_halted) = FALSE;
  current_agent(go_number) = 1;
  current_agent(go_type) = GO_DECISION;

  /* kjh (CUSP-B4) begin */
  /* Restore trace state: */
  set_sysparam(TRACE_CONTEXT_DECISIONS_SYSPARAM,        cur_TRACE_CONTEXT_DECISIONS_SYSPARAM);
  set_sysparam(TRACE_PHASES_SYSPARAM,                   cur_TRACE_PHASES_SYSPARAM);
  set_sysparam(TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM, cur_TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM);
  set_sysparam(TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM,    cur_TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM);
  set_sysparam(TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM,   cur_TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM);
  set_sysparam(TRACE_FIRINGS_PREFERENCES_SYSPARAM,      cur_TRACE_FIRINGS_PREFERENCES_SYSPARAM);
  set_sysparam(TRACE_WM_CHANGES_SYSPARAM,               cur_TRACE_WM_CHANGES_SYSPARAM);
  /* kjh (CUSP-B4) end */

  #ifndef NO_CALLBACKS  /* kjc 1/00 */
  soar_invoke_callbacks(soar_agent, 
		       AFTER_INIT_SOAR_CALLBACK,
		       (soar_call_data) NULL);
  #endif
  current_agent(input_cycle_flag) = TRUE;  /* reinitialize flag  AGR REW1 */

  /* REW: begin 09.15.96 */
  if (current_agent(operand2_mode) == TRUE) {
     current_agent(FIRING_TYPE) = IE_PRODS;  /* KJC 10.05.98 was PE */
     current_agent(current_phase) = INPUT_PHASE;
     current_agent(did_PE) = FALSE;
  }
  /* REW: end 09.15.96 */

}

/* ===================================================================
   
                            Running Soar

   Do_one_top_level_phase() runs Soar one top-level phase.  Note that
   this does not start/stop the total_cpu_time timer--the caller must
   do this.

   Each of the following routines runs Soar for a certain duration,
   or until stop_soar gets set to TRUE.
     - Run_forever() runs Soar forever.
     - Run_for_n_phases() runs Soar for a given number (n) of top-level
       phases.  (If n==-1, it runs forever.)
     - Run_for_n_elaboration_cycles() runs Soar for a given number (n)
       of elaboration cycles.  (Here, decision phase is counted as
       an elaboration cycle.)  (If n==-1, it runs forever.)
     - Run_for_n_decision_cycles() runs Soar for a given number (n) of
       decision cycles.  (If n==-1, it runs forever.)
     - Run_for_n_selections_of_slot (long n, Symbol *attr_of_slot): this
       runs Soar until the nth time a selection is made for a given
       type of slot.  Attr_of_slot should be either state_symbol or 
       operator_symbol.
     - Run_for_n_selections_of_slot_at_level (long n, Symbol *attr_of_slot,
       goal_stack_level level):  this runs Soar for n selections of the
       given slot at the given level, or until the goal stack is popped
       so that level no longer exists.
=================================================================== */

void do_one_top_level_phase (void) {

  /*  Symbol *iterate_goal_sym;  kjc commented /* RCHONG: end 10.11 */

  if (current_agent(system_halted)) {
    print ("\nSystem halted.  Use (init-soar) before running Soar again.");
    current_agent(stop_soar) = TRUE;
    current_agent(reason_for_stopping) = "System halted.";
    return;
  }
  
  if (! current_agent(top_goal)) {
    create_top_goal();
    if (current_agent(sysparams)[TRACE_CONTEXT_DECISIONS_SYSPARAM]) {
      print_string ("\n");
      print_lowest_slot_in_context_stack ();
    }
    current_agent(current_phase) = INPUT_PHASE;
    if (current_agent(operand2_mode)) current_agent(d_cycle_count)++;
  }

  switch (current_agent(current_phase)) {

  case INPUT_PHASE:

#ifdef REAL_TIME_BEHAVIOR
   /* RMJ; For real-time behavior, don't start any new decision phase
      until the specified "artificial" time step has passed */
   start_timer (current_real_time);
   if (timercmp(current_real_time, current_agent(real_time_tracker), <)) {
      if (!(current_agent(real_time_idling))) {
         current_agent(real_time_idling) = TRUE;
         if (current_agent(sysparams)[TRACE_PHASES_SYSPARAM]) {
            print ("\n--- Real-time Idle Phase ---\n");
         }
      }
      break;
   }

   /* Artificial time delay has passed.  Reset new delay and start the
      decision phase with input */
   current_agent(real_time_tracker)->tv_sec = current_real_time->tv_sec;
   current_agent(real_time_tracker)->tv_usec =
         current_real_time->tv_usec +
         1000 * current_agent(sysparams)[REAL_TIME_SYSPARAM];
   if (current_agent(real_time_tracker)->tv_usec >= 1000000) {
      current_agent(real_time_tracker)->tv_sec +=
            current_agent(real_time_tracker)->tv_usec / 1000000;
      current_agent(real_time_tracker)->tv_usec %= 1000000;
   }
   current_agent(real_time_idling) = FALSE;
#endif

#ifdef ATTENTION_LAPSE
   /* RMJ; decide whether to start or finish an attentional lapse */
   if (current_agent(sysparams)[ATTENTION_LAPSE_ON_SYSPARAM]) {
      if (current_agent(attention_lapsing)) {
         /* If lapsing, is it time to stop? */
         start_timer (current_real_time);
         if (timercmp(current_real_time,
                      current_agent(attention_lapse_tracker), >)) {
            wake_from_attention_lapse();
         }
      } else {
         /* If not lapsing, should we start? */
         lapse_duration = init_lapse_duration(current_agent(attention_lapse_tracker));
         if (lapse_duration > 0) {
            start_attention_lapse(lapse_duration);
         }
      }
   }
#endif

#ifndef NO_TIMING_STUFF   /* REW: begin 28.07.96 */
  start_timer (&current_agent(start_phase_tv));
#endif

  /* for Operand2 mode using the new decision cycle ordering,
     we need to do some initialization in the INPUT PHASE, which
     now comes first.  e_cycles are also zeroed before the APPLY Phase.
     */
    if (current_agent(operand2_mode) == TRUE) {
      current_agent(chunks_this_d_cycle) = 0;
      current_agent(e_cycles_this_d_cycle) = 0;
    }
    
    #ifndef NO_CALLBACKS /* kjc 1/00 */
    if (current_agent(e_cycles_this_d_cycle)==0) {
		soar_invoke_callbacks(soar_agent, 
			     BEFORE_DECISION_CYCLE_CALLBACK,
			     (soar_call_data) NULL);
    }
    #endif

    if (current_agent(input_cycle_flag) == TRUE) {  /* AGR REW1 */
      #ifndef NO_CALLBACKS /* kjc 1/00 */
      soar_invoke_callbacks(soar_agent, 
		  BEFORE_INPUT_PHASE_CALLBACK,
		  (soar_call_data) NULL);
      #endif
      do_input_cycle();

      #ifndef NO_CALLBACKS /* kjc 1/00 */
      soar_invoke_callbacks(soar_agent, 
		  AFTER_INPUT_PHASE_CALLBACK,
		  (soar_call_data) NULL);
      #endif
	  if (current_agent(input_period)) current_agent(input_cycle_flag) = FALSE;
    }  /* AGR REW1 this line and 1 previous line */

/* REW: begin 28.07.96 */
#ifndef NO_TIMING_STUFF
  stop_timer (&current_agent(start_phase_tv), 
              &current_agent(decision_cycle_phase_timers[INPUT_PHASE]));
#endif
/* REW: end 28.07.96 */


    /* REW: begin 09.15.96 */
    if (current_agent(operand2_mode) == TRUE) {
       /* REW: begin 05.05.97 */
       current_agent(current_phase) = DETERMINE_LEVEL_PHASE;
       current_agent(FIRING_TYPE) = IE_PRODS;
       /* Pref and WM are now done twice: for propose and apply.
	  We always need to know which "superphase" we are in. */
       current_agent(applyPhase) = FALSE;
       /* We now do input only once per decision so we can 'prime' the
	  decision for a new round of production firings at the end of
	  the input phase */
       initialize_consistency_calculations_for_new_decision();
       /* REW: end   05.05.97 */
    }
    /* REW: end   09.15.96 */
	else {
		if (any_assertions_or_retractions_ready()) 
			current_agent(current_phase) = PREFERENCE_PHASE;
		else
			current_agent(current_phase) = DECISION_PHASE;
	}
  


    break;
    
  case DETERMINE_LEVEL_PHASE:

    #ifndef NO_TIMING_STUFF
	  start_timer (&current_agent(start_phase_tv));
    #endif
     
      /* Still need to register callbacks for both before and after
       the determine_level procedure call. */
    
	  if (current_agent(applyPhase)) 
		  determine_highest_active_production_level_in_stack_apply();
	  else 
		  determine_highest_active_production_level_in_stack_propose();
          
      #ifndef NO_TIMING_STUFF
	  stop_timer (&current_agent(start_phase_tv), 
                     &current_agent(decision_cycle_phase_timers[DETERMINE_LEVEL_PHASE]));
      #endif

	  break;

  case PREFERENCE_PHASE:

	  /* REW: begin 28.07.96 */
     
      #ifndef NO_TIMING_STUFF
      start_timer (&current_agent(start_phase_tv));
      #endif
      /* REW: end 28.07.96 */
 
      #ifndef NO_CALLBACKS
	  soar_invoke_callbacks(soar_agent, 
			 BEFORE_PREFERENCE_PHASE_CALLBACK,
			 (soar_call_data) NULL);
      #endif

	  do_preference_phase();

      #ifndef NO_CALLBACKS
      soar_invoke_callbacks(soar_agent, 
			 AFTER_PREFERENCE_PHASE_CALLBACK,
			 (soar_call_data) NULL);
      #endif

	  current_agent(current_phase) = WM_PHASE;

      /* REW: begin 28.07.96 */
      #ifndef NO_TIMING_STUFF
          stop_timer (&current_agent(start_phase_tv), 
                      &current_agent(decision_cycle_phase_timers[PREFERENCE_PHASE]));
      #endif
      /* REW: end 28.07.96 */

      if (current_agent(operand2_mode) == FALSE) break;
      /* if we're in Soar8, then go right to WM_PHASE without stopping */
    

  case WM_PHASE:

	  /* REW: begin 28.07.96 */
      #ifndef NO_TIMING_STUFF
	  start_timer (&current_agent(start_phase_tv));
      #endif
      /* REW: end 28.07.96 */
	
      #ifndef NO_CALLBACKS /* kjc 1/00 */
	  soar_invoke_callbacks(soar_agent, 
			 BEFORE_WM_PHASE_CALLBACK,
			 (soar_call_data) NULL);
      #endif
    
	  do_working_memory_phase();
    
      #ifndef NO_CALLBACKS
	  soar_invoke_callbacks(soar_agent, 
			 AFTER_WM_PHASE_CALLBACK,
			 (soar_call_data) NULL);
      #endif

    if (current_agent(operand2_mode) == TRUE) {
      /* KJC - New Order: Always follow WM PHASE with DETERMINE_LEVEL_PHASE */
      current_agent(current_phase) = DETERMINE_LEVEL_PHASE;

      /* Update accounting.  Moved here by KJC 10-02-98*/
      current_agent(e_cycle_count)++;
      current_agent(e_cycles_this_d_cycle)++;

      if (current_agent(FIRING_TYPE) == PE_PRODS) {  
	/* Keep track of each pe_phase for reporting in stats */
	current_agent(pe_cycle_count)++;
	current_agent(pe_cycles_this_d_cycle)++;
      }

    }
    /* REW: end   10.29.97 */

    else

    current_agent(current_phase) = OUTPUT_PHASE;

    /* REW: begin 28.07.96 */
     #ifndef NO_TIMING_STUFF
         stop_timer (&current_agent(start_phase_tv), 
                     &current_agent(decision_cycle_phase_timers[WM_PHASE]));
     #endif
     /* REW: end 28.07.96 */
    break;

    
  case OUTPUT_PHASE:

      /* REW: begin 28.07.96 */
      #ifndef NO_TIMING_STUFF
	  start_timer (&current_agent(start_phase_tv));
      #endif
      /* REW: end 28.07.96 */
    
      #ifndef NO_CALLBACKS /* kjc 1/00 */
	  soar_invoke_callbacks(soar_agent, 
			 BEFORE_OUTPUT_PHASE_CALLBACK,
			 (soar_call_data) NULL);
      #endif
   
	  /* REW: begin 28.07.96 */
      #ifndef NO_TIMING_STUFF
      stop_timer (&current_agent(start_phase_tv), 
                   &current_agent(decision_cycle_phase_timers[current_agent(current_phase)]));
      stop_timer (&current_agent(start_kernel_tv), &current_agent(total_kernel_time));
      start_timer (&current_agent(start_kernel_tv));
      #endif
      /* REW: end 28.07.96 */

	  do_output_cycle();

      /* REW: begin 28.07.96 */
      #ifndef NO_TIMING_STUFF
      stop_timer (&current_agent(start_kernel_tv), &current_agent(output_function_cpu_time));
      start_timer (&current_agent(start_kernel_tv));
      start_timer (&current_agent(start_phase_tv));
      #endif
      /* REW: end 28.07.96 */

      #ifndef NO_CALLBACKS /* kjc 1/00 */
	  soar_invoke_callbacks(soar_agent, 
			 AFTER_OUTPUT_PHASE_CALLBACK,
			 (soar_call_data) NULL);
      #endif

      /* REW: begin 09.15.96 */
      if (current_agent(operand2_mode) == TRUE) {
          /* After each OUTPUT_PHASE in Operand2/Waterfall, always return to the 
	         DETERMINE_LEVEL_PHASE */
          /* changed to INPUT LEVEL. KJC 10-04-98 */
		  current_agent(current_phase) = INPUT_PHASE;
          current_agent(d_cycle_count)++;

          /* timers stopped KJC 10-04-98 */
          #ifndef NO_TIMING_STUFF
          stop_timer (&current_agent(start_phase_tv), 
                     &current_agent(decision_cycle_phase_timers[OUTPUT_PHASE]));
          #endif
     
		  break;
	  }
      /* REW: end 09.15.96 */

    
	  /* otherwise we're in Soar7 mode ...  */

	  current_agent(e_cycle_count)++;
	  current_agent(e_cycles_this_d_cycle)++;

      /* MVP 6-8-94 */
      if (current_agent(e_cycles_this_d_cycle) >=
		  (unsigned long)(current_agent(sysparams)[MAX_ELABORATIONS_SYSPARAM])) {
		  if (current_agent(sysparams)[PRINT_WARNINGS_SYSPARAM])
			  print ("\nWarning: reached max-elaborations; proceeding to decision phase.");
		  current_agent(current_phase) = DECISION_PHASE;
	  } else
		  current_agent(current_phase) = INPUT_PHASE;
     
	  /* REW: begin 28.07.96 */
      #ifndef NO_TIMING_STUFF   
	  stop_timer (&current_agent(start_phase_tv), 
		  &current_agent(decision_cycle_phase_timers[OUTPUT_PHASE]));
      #endif
      /* REW: end 28.07.96 */

	  break;
    
  case DECISION_PHASE:
 
	  /* REW: begin 28.07.96 */
      #ifndef NO_TIMING_STUFF
	  start_timer (&current_agent(start_phase_tv));
      #endif
      /* REW: end 28.07.96 */

      /* d_cycle_count moved to input phase for Soar 8 new decision cycle */
      if (current_agent(operand2_mode) == FALSE) current_agent(d_cycle_count)++;

      /* AGR REW1 begin */
	  if (!current_agent(input_period)) 
		  current_agent(input_cycle_flag) = TRUE;
	  else if ((current_agent(d_cycle_count) % current_agent(input_period)) == 0)
		  current_agent(input_cycle_flag) = TRUE;
      /* AGR REW1 end */

      #ifndef NO_CALLBACKS /* kjc 1/00 */
      soar_invoke_callbacks(soar_agent, 
	 		 BEFORE_DECISION_PHASE_CALLBACK,
			 (soar_call_data) NULL);
      #endif
	  
	  do_decision_phase();

      /* kjc 1/00: always want this CB, so can do matches cmd here */
	  soar_invoke_callbacks(soar_agent, 
			 AFTER_DECISION_PHASE_CALLBACK,
			 (soar_call_data) NULL);


      #ifdef _WINDOWS
	  {
		  char *str;
		  if (soar_is_halted(&str)) {
			  current_agent(stop_soar)=TRUE;
			  current_agent(reason_for_stopping)=str;
		  }
	  }
      #endif

	  /* kjc 1/00: leave this one too in case confusion, but really the
	   * end of the DC for Soar 8 is after output phase... */
	  soar_invoke_callbacks(soar_agent, 
			 AFTER_DECISION_CYCLE_CALLBACK,
			 (soar_call_data) NULL);
    
	  if (current_agent(sysparams)[TRACE_CONTEXT_DECISIONS_SYSPARAM]) {
          #ifdef USE_TCL
		  print_string ("\n");
          #else
		  if(current_agent(printer_output_column) != 1)
			  print_string ("\n");
          #endif /* USE_TCL */
		  print_lowest_slot_in_context_stack ();
	  }


	  if (current_agent(operand2_mode) == FALSE) {
		  current_agent(chunks_this_d_cycle) = 0;
 
	  }
	  current_agent(e_cycles_this_d_cycle) = 0;
	  current_agent(current_phase) = INPUT_PHASE;

	  /* REW: begin 09.15.96 */
	  if (current_agent(operand2_mode) == TRUE) {
		  /* test for ONC, if TRUE, generate substate and go to OUTPUT */
		  if ((current_agent(ms_o_assertions) == NIL) &&
			  (current_agent(bottom_goal)->id.operator_slot->wmes != NIL)) {
    
              #ifndef NO_CALLBACKS
			  soar_invoke_callbacks(soar_agent, BEFORE_DECISION_PHASE_CALLBACK,
				                    (soar_call_data) NULL);
              #endif

			  do_decision_phase();
     
			  soar_invoke_callbacks(soar_agent, AFTER_DECISION_PHASE_CALLBACK,
                                    (soar_call_data) NULL);

			  if (current_agent(sysparams)[TRACE_CONTEXT_DECISIONS_SYSPARAM]) {
                  #ifdef USE_TCL
				  print_string ("\n");
                  #else
				  if(current_agent(printer_output_column) != 1) print_string ("\n");
                  #endif /* USE_TCL */
				  print_lowest_slot_in_context_stack ();
			  }

			  /* set phase to OUTPUT */
			  current_agent(current_phase) = OUTPUT_PHASE;

			  /* REW: begin 28.07.96 */
              #ifndef NO_TIMING_STUFF
              stop_timer (&current_agent(start_phase_tv), 
				  &current_agent(decision_cycle_phase_timers[DECISION_PHASE]));
              #endif
	          /* REW: end 28.07.96 */

			  /* kjc 1/00:  do we need AFTER_DECISION_CYCLE callback here too?? */

			  break;
   
		  } else {

			  /* printf("\nSetting next phase to APPLY following a decision...."); */
			  current_agent(applyPhase) = TRUE;
			  current_agent(FIRING_TYPE) = PE_PRODS;
			  current_agent(current_phase) = DETERMINE_LEVEL_PHASE;
			  /* 'prime' the cycle for a new round of production firings 
			      in the APPLY (pref/wm) phase */
			  initialize_consistency_calculations_for_new_decision();
		  }
	  }
 
	  /* REW: begin 28.07.96 */
      #ifndef NO_TIMING_STUFF
	  stop_timer (&current_agent(start_phase_tv), 
		  &current_agent(decision_cycle_phase_timers[DECISION_PHASE]));
      #endif
	  /* REW: end 28.07.96 */
	  
	  break;  /* end DECISION phase */
	  
  }  /* end switch stmt for current_phase */
  
  /* --- update WM size statistics --- */
  if (current_agent(num_wmes_in_rete) > current_agent(max_wm_size)) 
      current_agent(max_wm_size) = current_agent(num_wmes_in_rete);
  current_agent(cumulative_wm_size) += current_agent(num_wmes_in_rete);
  current_agent(num_wm_sizes_accumulated)++;
  
  if (current_agent(system_halted)) {
	  current_agent(stop_soar) = TRUE;
	  current_agent(reason_for_stopping) = "System halted.";
	  soar_invoke_callbacks(soar_agent, 
		  AFTER_HALT_SOAR_CALLBACK,
		  (soar_call_data) NULL);
  }
  
  if (current_agent(stop_soar))
	  if (current_agent(reason_for_stopping) != "")
		  print ("\n%s", current_agent(reason_for_stopping));
}

void run_forever (void) {
    #ifndef NO_TIMING_STUFF
	start_timer (&current_agent(start_total_tv));
	start_timer (&current_agent(start_kernel_tv));
    #endif

	current_agent(stop_soar) = FALSE;
	current_agent(reason_for_stopping) = "";
	while (! current_agent(stop_soar)) {
		do_one_top_level_phase();
	}

    #ifndef NO_TIMING_STUFF
	stop_timer (&current_agent(start_kernel_tv), &current_agent(total_kernel_time));
	stop_timer (&current_agent(start_total_tv), &current_agent(total_cpu_time));
    #endif
}

void run_for_n_phases (long n) {
  if (n == -1) { run_forever(); return; }
  if (n < -1) return;
#ifndef NO_TIMING_STUFF
  start_timer (&current_agent(start_total_tv));
  start_timer (&current_agent(start_kernel_tv));
#endif
  current_agent(stop_soar) = FALSE;
  current_agent(reason_for_stopping) = "";
  while (!current_agent(stop_soar) && n) {
    do_one_top_level_phase();
    n--;
  }
#ifndef NO_TIMING_STUFF
  stop_timer (&current_agent(start_total_tv), &current_agent(total_cpu_time));
  stop_timer (&current_agent(start_kernel_tv), &current_agent(total_kernel_time));
#endif
}

void run_for_n_elaboration_cycles (long n) {
  long e_cycles_at_start, d_cycles_at_start, elapsed_cycles;
  
  if (n == -1) { run_forever(); return; }
  if (n < -1) return;
#ifndef NO_TIMING_STUFF
  start_timer (&current_agent(start_total_tv));
  start_timer (&current_agent(start_kernel_tv));
#endif
  current_agent(stop_soar) = FALSE;
  current_agent(reason_for_stopping) = "";
  e_cycles_at_start = current_agent(e_cycle_count);
  d_cycles_at_start = current_agent(d_cycle_count);
  /* need next line or runs only the input phase for "d 1" after init-soar */
  if ( current_agent(operand2_mode) && (d_cycles_at_start == 0) )
    d_cycles_at_start++;
  while (!current_agent(stop_soar)) {
    elapsed_cycles = (current_agent(d_cycle_count)-d_cycles_at_start) +
                     (current_agent(e_cycle_count)-e_cycles_at_start);
    if (n==elapsed_cycles) break;
    do_one_top_level_phase();
  }
#ifndef NO_TIMING_STUFF
  stop_timer (&current_agent(start_total_tv), &current_agent(total_cpu_time));
  stop_timer (&current_agent(start_kernel_tv), &current_agent(total_kernel_time));
#endif
}

void run_for_n_modifications_of_output (long n) {
  bool was_output_phase;
  long count = 0;

  if (n == -1) { run_forever(); return; }
  if (n < -1) return;
#ifndef NO_TIMING_STUFF
  start_timer (&current_agent(start_total_tv));
  start_timer (&current_agent(start_kernel_tv));
#endif
  current_agent(stop_soar) = FALSE;
  current_agent(reason_for_stopping) = "";
  while (!current_agent(stop_soar) && n) {
    was_output_phase = (current_agent(current_phase)==OUTPUT_PHASE);
    do_one_top_level_phase();
    if (was_output_phase) {	
		if (current_agent(output_link_changed)) {
		  n--;
		} else {
		  count++;
	} }
	if (count > current_agent(sysparams)[MAX_NIL_OUTPUT_CYCLES_SYSPARAM]) {
		current_agent(stop_soar) = TRUE;
		current_agent(reason_for_stopping) = "exceeded max_nil_output_cycles with no output";
	}
  }
#ifndef NO_TIMING_STUFF
  stop_timer (&current_agent(start_kernel_tv), &current_agent(total_kernel_time));
  stop_timer (&current_agent(start_total_tv), &current_agent(total_cpu_time));
#endif
}

void run_for_n_decision_cycles (long n) {
  long d_cycles_at_start;
  
  if (n == -1) { run_forever(); return; }
  if (n < -1) return;
#ifndef NO_TIMING_STUFF
  start_timer (&current_agent(start_total_tv));
  start_timer (&current_agent(start_kernel_tv));
#endif
  current_agent(stop_soar) = FALSE;
  current_agent(reason_for_stopping) = "";
  d_cycles_at_start = current_agent(d_cycle_count);
  /* need next line or runs only the input phase for "d 1" after init-soar */
  if ( current_agent(operand2_mode) && (d_cycles_at_start == 0) )
    d_cycles_at_start++;
  while (!current_agent(stop_soar)) {
    if (n==(long)(current_agent(d_cycle_count)-d_cycles_at_start)) break;
    do_one_top_level_phase();
  }
#ifndef NO_TIMING_STUFF
  stop_timer (&current_agent(start_total_tv), &current_agent(total_cpu_time));
  stop_timer (&current_agent(start_kernel_tv), &current_agent(total_kernel_time));
#endif
}

Symbol *attr_of_slot_just_decided (void) {
  if (current_agent(bottom_goal)->id.operator_slot->wmes) 
    return current_agent(operator_symbol);
  return current_agent(state_symbol);
}

void run_for_n_selections_of_slot (long n, Symbol *attr_of_slot) {
  long count;
  bool was_decision_phase;
  
  if (n == -1) { run_forever(); return; }
  if (n < -1) return;
#ifndef NO_TIMING_STUFF
  start_timer (&current_agent(start_total_tv));
  start_timer (&current_agent(start_kernel_tv));
#endif
  current_agent(stop_soar) = FALSE;
  current_agent(reason_for_stopping) = "";
  count = 0;
  while (!current_agent(stop_soar) && (count < n)) {
    was_decision_phase = (current_agent(current_phase)==DECISION_PHASE);
    do_one_top_level_phase();
    if (was_decision_phase)
      if (attr_of_slot_just_decided()==attr_of_slot) count++;
  }
#ifndef NO_TIMING_STUFF
  stop_timer (&current_agent(start_total_tv), &current_agent(total_cpu_time));
  stop_timer (&current_agent(start_kernel_tv), &current_agent(total_kernel_time));
#endif
}

void run_for_n_selections_of_slot_at_level (long n,
                                            Symbol *attr_of_slot,
                                            goal_stack_level level) {
  long count;
  bool was_decision_phase;
  
  if (n == -1) { run_forever(); return; }
  if (n < -1) return;
#ifndef NO_TIMING_STUFF
  start_timer (&current_agent(start_total_tv));
  start_timer (&current_agent(start_kernel_tv));
#endif
  current_agent(stop_soar) = FALSE;
  current_agent(reason_for_stopping) = "";
  count = 0;
  while (!current_agent(stop_soar) && (count < n)) {
    was_decision_phase = (current_agent(current_phase)==DECISION_PHASE);
    do_one_top_level_phase();
    if (was_decision_phase) {
      if (current_agent(bottom_goal)->id.level < level) break;
      if (current_agent(bottom_goal)->id.level==level) {
        if (attr_of_slot_just_decided()==attr_of_slot) count++;
      }
    }
  }
#ifndef NO_TIMING_STUFF
  stop_timer (&current_agent(start_total_tv), &current_agent(total_cpu_time));
  stop_timer (&current_agent(start_kernel_tv), &current_agent(total_kernel_time));
#endif
}

/* ===================================================================

                     Print the Startup Banner

=================================================================== */

char * soar_news_string = "\
General questions and topics for discussion should be sent to\n\
soar-group@umich.edu. Bug reports should be sent to soar-bugs@umich.edu\n\
The current bug-list may be obtained by sending mail to\n\
soar-bugs@umich.edu with the Subject: line \"bug list\".\n\
The Soar Home Page URL is:  http://ai.eecs.umich.edu/soar\n\
\n\
Copyright (c) 1995-1999 Carnegie Mellon University,\n\
                         University of Michigan,\n\
                         University of Southern California/Information\n\
                         Sciences Institute.  All rights reserved.\n\
The Soar consortium proclaims this software is in the public domain, and\n\
is made available AS IS.  Carnegie Mellon University, The University of \n\
Michigan, and The University of Southern California/Information Sciences \n\
Institute make no warranties about the software or its performance,\n\
implied or otherwise.\n\
\n\
Type \"help\" for information on various topics.\n\
Type \"quit\" to exit Soar.  Use ctrl-c to stop a Soar run.\n\
Type \"soarnews\" to repeat this information.\n\
Type \"version\" for Soar version information.\
";

void print_startup_banner (void) {
  print(soar_version_string);
  print(soar_news_string);
}

/* ===================================================================
   
             Loading the Initialization File ".init.soar"

   This routine looks for a file ".init.soar" in either the current
   directory or $HOME, and if found, loads it.
=================================================================== */

extern char *getenv();

/* AGR 536  Soar core dumped when it used filenames longer than 1000 chars
   but shorter than MAXPATHLEN (from sys/param.h).  4-May-94  */

void load_init_file (void) {
  char filename[1000];   /* AGR 536 */
  char *home_directory;
  FILE *initfile;

  strcpy (filename, INIT_FILE);
  initfile = fopen (filename, "r");
  if (!initfile) {
    home_directory = getenv ("HOME");
    if (home_directory) {
      strcpy (filename, home_directory);
      strcat (filename, "/");
      strcat (filename, INIT_FILE);
      initfile = fopen (filename, "r");
    }
  }

  print_startup_banner();

  if (initfile) {
    print ("\nLoading %s\n",filename);
    load_file (filename, initfile);
    fclose (initfile);
  }
}

int terminate_soar (void)
{
  /* Shouldn't we free *all* agents here? */
  free((void *) soar_agent);

  exit_soar();  
  return 0; /* unreachable, but without it, gcc -Wall warns here */
}

