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
 * Copyright 1995-2004 Carnegie Mellon University,
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

#include "soarkernel.h"
#include <signal.h>             /* used for control-c handler */

#if !defined(__SC__) && !defined(THINK_C) && !defined(WIN32) && !defined(MACINTOSH)
#include <sys/time.h>           /* used for timing stuff */
#include <sys/resource.h>       /* used for timing stuff */
#endif                          /* !__SC__ && !THINK_C && !WIN32 */

/* REW: begin 08.20.97   these defined in consistency.c  */
extern void determine_highest_active_production_level_in_stack();
extern void determine_highest_active_production_level_in_stack_apply();
extern void determine_highest_active_production_level_in_stack_propose();
extern void initialize_consistency_calculations_for_new_decision();

/* REW: end   08.20.97 */

int soar_agent_ids[MAX_SIMULTANEOUS_AGENTS];
soar_global_callback_array soar_global_callbacks;
unsigned long soar_global_callback_error;

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

void just_before_exit_soar(void)
{
    cons *c;

    /* 
     * This function should now deal with multiple agents
     * as well as no agents.
     */
    for (c = all_soar_agents; c != NULL; c = c->rest) {
        soar_invoke_callbacks((agent *) c->first, SYSTEM_TERMINATION_CALLBACK, (soar_call_data) TRUE);

        if (((agent *) c->first)->logging_to_file)
            stop_log_file();
    }
}

void exit_soar(void)
{

    just_before_exit_soar();
    exit(0);

}

void abort_with_fatal_error(char *msg)
{
    FILE *f;

    print("%s", msg);
    print("Soar cannot recover from this error.  Aborting...\n");
    fprintf(stderr, "%s", msg);
    fprintf(stderr, "Soar cannot recover from this error.  Aborting...\n");
    f = fopen("soarerror", "w");
    fprintf(f, "%s", msg);
    fprintf(f, "Soar cannot recover from this error.  Aborting...\n");
    fclose(f);

    soar_invoke_callbacks(soar_agent, SYSTEM_TERMINATION_CALLBACK, (soar_call_data) FALSE);

    if (current_agent(logging_to_file))
        stop_log_file();

    sys_abort();

}

#ifdef REAL_TIME_BEHAVIOR
/* RMJ */
void init_real_time(void)
{
    current_agent(real_time_tracker) = (struct timeval *) malloc(sizeof(struct timeval));
    timerclear(current_agent(real_time_tracker));
    current_agent(real_time_idling) = FALSE;
    current_real_time = (struct timeval *) malloc(sizeof(struct timeval));
}
#endif

#ifdef ATTENTION_LAPSE
/* RMJ */

void wake_from_attention_lapse(void)
{
    /* Set tracker to last time we woke up */
    start_timer(current_agent(attention_lapse_tracker));
    current_agent(attention_lapsing) = FALSE;
}

void init_attention_lapse(void)
{
    current_agent(attention_lapse_tracker) = (struct timeval *) malloc(sizeof(struct timeval));
    wake_from_attention_lapse();
#ifndef REAL_TIME_BEHAVIOR
    current_real_time = (struct timeval *) malloc(sizeof(struct timeval));
#endif
}

void start_attention_lapse(long duration)
{
    /* Set tracker to time we should wake up */
    start_timer(current_agent(attention_lapse_tracker));
    current_agent(attention_lapse_tracker)->tv_usec += 1000 * duration;
    if (current_agent(attention_lapse_tracker)->tv_usec >= 1000000) {
        current_agent(attention_lapse_tracker)->tv_sec += current_agent(attention_lapse_tracker)->tv_usec / 1000000;
        current_agent(attention_lapse_tracker)->tv_usec %= 1000000;
    }
    current_agent(attention_lapsing) = TRUE;
}

#endif

/* ===================================================================
   
                            Sysparams

=================================================================== */

void init_sysparams(void)
{
    int i;

    for (i = 0; i < HIGHEST_SYSPARAM_NUMBER + 1; i++)
        current_agent(sysparams)[i] = 0;

    /* --- set all params to zero, except the following: --- */
    current_agent(sysparams)[TRACE_CONTEXT_DECISIONS_SYSPARAM] = TRUE;

#ifndef TRACE_CONTEXT_DECISIONS_ONLY

    current_agent(sysparams)[TRACE_FIRINGS_OF_CHUNKS_SYSPARAM] = TRUE;
    current_agent(sysparams)[TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM] = NONE_WME_TRACE;
    current_agent(sysparams)[TRACE_CHUNK_NAMES_SYSPARAM] = TRUE;
    current_agent(sysparams)[TRACE_JUSTIFICATION_NAMES_SYSPARAM] = TRUE;

#endif
    current_agent(sysparams)[TRACE_LOADING_SYSPARAM] = TRUE;    /* KJC 8/96 */

    current_agent(sysparams)[MAX_ELABORATIONS_SYSPARAM] = 100;
    current_agent(sysparams)[MAX_CHUNKS_SYSPARAM] = 50;

    current_agent(sysparams)[RESPOND_TO_LOAD_ERRORS_SYSPARAM] = TRUE;

#ifdef ATTENTION_LAPSE
    /* RMJ */
    current_agent(sysparams)[ATTENTION_LAPSE_ON_SYSPARAM] = FALSE;
#endif                          /* ATTENTION_LAPSE */

    current_agent(sysparams)[LEARNING_ON_SYSPARAM] = FALSE;  /* bugzilla bug 338 */
    current_agent(sysparams)[LEARNING_ONLY_SYSPARAM] = FALSE;   /* AGR MVL1 */
    current_agent(sysparams)[LEARNING_EXCEPT_SYSPARAM] = FALSE; /* KJC 8/96 */
    current_agent(sysparams)[LEARNING_ALL_GOALS_SYSPARAM] = TRUE;
    current_agent(sysparams)[USER_SELECT_MODE_SYSPARAM] = USER_SELECT_RANDOM;
    current_agent(sysparams)[PRINT_WARNINGS_SYSPARAM] = TRUE;
    current_agent(sysparams)[PRINT_ALIAS_SYSPARAM] = TRUE;      /* AGR 627 */
    current_agent(sysparams)[EXPLAIN_SYSPARAM] = FALSE; /* KJC 7/96 */
    current_agent(sysparams)[USE_LONG_CHUNK_NAMES] = TRUE;      /* kjh(B14) */
    current_agent(sysparams)[TRACE_OPERAND2_REMOVALS_SYSPARAM] = FALSE;
}

/* ===================================================================
   
                     Adding and Removing Pwatchs

   Productions_being_traced is a (consed) list of all productions
   on which a pwatch has been set.  Pwatchs are added/removed via
   calls to add_pwatch() and remove_pwatch().
=================================================================== */
/* list of production structures */

#ifndef TRACE_CONTEXT_DECISIONS_ONLY

void add_pwatch(production * prod)
{
    if (prod->trace_firings)
        return;
    prod->trace_firings = TRUE;
    push(prod, current_agent(productions_being_traced));
}

production *prod_to_remove_pwatch_of;

bool remove_pwatch_test_fn(cons * c)
{
    return (bool) (c->first == prod_to_remove_pwatch_of);
}

void remove_pwatch(production * prod)
{
    if (!prod->trace_firings)
        return;
    prod->trace_firings = FALSE;
    prod_to_remove_pwatch_of = prod;
    free_list(extract_list_elements(&current_agent(productions_being_traced), remove_pwatch_test_fn));
}

#endif

/* ===================================================================
   
                         Reinitializing Soar

   Reset_statistics() resets all the statistics (except the firing counts
   on each individual production).  Reinitialize_soar() does all the 
   work for an init-soar.
=================================================================== */

void reset_production_firing_counts(void)
{
    int t;
    production *p;

    for (t = 0; t < NUM_PRODUCTION_TYPES; t++) {
        for (p = current_agent(all_productions_of_type)[t]; p != NIL; p = p->next)
            p->firing_count = 0;
    }
}

void reset_statistics(void)
{

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
    reset_timer(&current_agent(total_cpu_time));

/* REW: begin 28.07.96 */

    reset_timer(&current_agent(total_kernel_time));

    reset_timer(&current_agent(input_function_cpu_time));
    reset_timer(&current_agent(output_function_cpu_time));

#ifndef KERNEL_TIME_ONLY

    reset_timer(&current_agent(decision_cycle_phase_timers[INPUT_PHASE]));
    reset_timer(&current_agent(decision_cycle_phase_timers[DETERMINE_LEVEL_PHASE]));
    reset_timer(&current_agent(decision_cycle_phase_timers[PREFERENCE_PHASE]));
    reset_timer(&current_agent(decision_cycle_phase_timers[WM_PHASE]));
    reset_timer(&current_agent(decision_cycle_phase_timers[OUTPUT_PHASE]));
    reset_timer(&current_agent(decision_cycle_phase_timers[DECISION_PHASE]));

    reset_timer(&current_agent(monitors_cpu_time[INPUT_PHASE]));
    reset_timer(&current_agent(monitors_cpu_time[DETERMINE_LEVEL_PHASE]));
    reset_timer(&current_agent(monitors_cpu_time[PREFERENCE_PHASE]));
    reset_timer(&current_agent(monitors_cpu_time[WM_PHASE]));
    reset_timer(&current_agent(monitors_cpu_time[OUTPUT_PHASE]));
    reset_timer(&current_agent(monitors_cpu_time[DECISION_PHASE]));

#ifdef DETAILED_TIMING_STATS
    reset_timer(&current_agent(match_cpu_time[INPUT_PHASE]));
    reset_timer(&current_agent(match_cpu_time[DETERMINE_LEVEL_PHASE]));
    reset_timer(&current_agent(match_cpu_time[PREFERENCE_PHASE]));
    reset_timer(&current_agent(match_cpu_time[WM_PHASE]));
    reset_timer(&current_agent(match_cpu_time[OUTPUT_PHASE]));
    reset_timer(&current_agent(match_cpu_time[DECISION_PHASE]));

    reset_timer(&current_agent(ownership_cpu_time[INPUT_PHASE]));
    reset_timer(&current_agent(ownership_cpu_time[DETERMINE_LEVEL_PHASE]));
    reset_timer(&current_agent(ownership_cpu_time[PREFERENCE_PHASE]));
    reset_timer(&current_agent(ownership_cpu_time[WM_PHASE]));
    reset_timer(&current_agent(ownership_cpu_time[OUTPUT_PHASE]));
    reset_timer(&current_agent(ownership_cpu_time[DECISION_PHASE]));

    reset_timer(&current_agent(chunking_cpu_time[INPUT_PHASE]));
    reset_timer(&current_agent(chunking_cpu_time[DETERMINE_LEVEL_PHASE]));
    reset_timer(&current_agent(chunking_cpu_time[PREFERENCE_PHASE]));
    reset_timer(&current_agent(chunking_cpu_time[WM_PHASE]));
    reset_timer(&current_agent(chunking_cpu_time[OUTPUT_PHASE]));
    reset_timer(&current_agent(chunking_cpu_time[DECISION_PHASE]));

/* REW: begin 11.25.96 */
    reset_timer(&current_agent(total_gds_time));

    reset_timer(&current_agent(gds_cpu_time[INPUT_PHASE]));
    reset_timer(&current_agent(gds_cpu_time[DETERMINE_LEVEL_PHASE]));
    reset_timer(&current_agent(gds_cpu_time[PREFERENCE_PHASE]));
    reset_timer(&current_agent(gds_cpu_time[WM_PHASE]));
    reset_timer(&current_agent(gds_cpu_time[OUTPUT_PHASE]));
    reset_timer(&current_agent(gds_cpu_time[DECISION_PHASE]));
/* REW: end   11.25.96 */
#endif
#endif
#ifdef DC_HISTOGRAM
    {
        int i;
        for (i = 0; i < current_agent(dc_histogram_sz); i++) {
            reset_timer(&current_agent(dc_histogram_tv)[i]);
        }
    }
#endif                          /* DC_HISTOGRAM */
#ifdef KT_HISTOGRAM
    {
        int i;
        for (i = 0; i < current_agent(kt_histogram_sz); i++) {
            reset_timer(&current_agent(kt_histogram_tv)[i]);
        }
    }
#endif                          /* KT_HISTOGRAM */

#endif
/* REW: end 28.07.96 */
#ifdef COUNT_KERNEL_TIMER_STOPS
    current_agent(kernelTimerStops) = 0;
    current_agent(nonKernelTimerStops) = 0;
#endif

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

void do_one_top_level_phase(void)
{

    if (current_agent(system_halted)) {
        print("\nSystem halted.  Use (init-soar) before running Soar again.");
        current_agent(stop_soar) = TRUE;
        current_agent(reason_for_stopping) = "System halted.";
        return;
    }

    if (!current_agent(top_goal)) {
        create_top_goal();
        if (current_agent(sysparams)[TRACE_CONTEXT_DECISIONS_SYSPARAM]) {
            print_string("\n");
            print_lowest_slot_in_context_stack();
        }
        current_agent(current_phase) = INPUT_PHASE;

#ifndef SOAR_8_ONLY
        if (current_agent(operand2_mode))
#endif
            /*
             * SW 112999 was:
             * current_agent(d_cycle_count)++;
             */
            increment_current_agent_d_cycle_count;
#ifdef DC_HISTOGRAM
        /*  SW NOTE
         *  I beleive this is executed for the very first decision cycle only.
         *  Assuming this holds true, then this should be right.  We start
         *  the dc_histogram timer here, and we will stop it every 
         *  dc_histogram_freq decision cycles when dc_histogram_now is TRUE
         *  when it is stopped (at the end of OUTPUT) it will also be restarted
         *  thus, we only need to bootstrap for the 1st dc.
         */
        start_timer(&current_agent(start_dc_tv));
#endif

    }

    switch (current_agent(current_phase)) {

    case INPUT_PHASE:
#ifdef REAL_TIME_BEHAVIOR
        /* RMJ; For real-time behavior, don't start any new decision phase
           until the specified "artificial" time step has passed */
        start_timer(current_real_time);
        if (timercmp(current_real_time, current_agent(real_time_tracker), <)) {
            if (!(current_agent(real_time_idling))) {
                current_agent(real_time_idling) = TRUE;
                if (current_agent(sysparams)[TRACE_PHASES_SYSPARAM]) {
                    print("\n--- Real-time Idle Phase ---\n");
                }
            }
            break;
        }

        /* Artificial time delay has passed.  Reset new delay and start the
           decision phase with input */
        current_agent(real_time_tracker)->tv_sec = current_real_time->tv_sec;
        current_agent(real_time_tracker)->tv_usec =
            current_real_time->tv_usec + 1000 * current_agent(sysparams)[REAL_TIME_SYSPARAM];
        if (current_agent(real_time_tracker)->tv_usec >= 1000000) {
            current_agent(real_time_tracker)->tv_sec += current_agent(real_time_tracker)->tv_usec / 1000000;
            current_agent(real_time_tracker)->tv_usec %= 1000000;
        }
        current_agent(real_time_idling) = FALSE;
#endif

#ifdef ATTENTION_LAPSE
        /* RMJ; decide whether to start or finish an attentional lapse */
        if (current_agent(sysparams)[ATTENTION_LAPSE_ON_SYSPARAM]) {
            if (current_agent(attention_lapsing)) {
                /* If lapsing, is it time to stop? */
                start_timer(current_real_time);
                if (timercmp(current_real_time, current_agent(attention_lapse_tracker), >)) {
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

#ifndef KERNEL_TIME_ONLY
#ifndef NO_TIMING_STUFF         /* REW: begin 28.07.96 */
        start_timer(&current_agent(start_phase_tv));
#endif
#endif

        /* for Operand2 mode using the new decision cycle ordering,
           we need to do some initialization in the INPUT PHASE, which
           now comes first.  e_cycles are also zeroed before the APPLY Phase.
         */
#ifndef SOAR_8_ONLY
        if (current_agent(operand2_mode) == TRUE) {
#endif
            current_agent(chunks_this_d_cycle) = 0;
            current_agent(e_cycles_this_d_cycle) = 0;
#ifndef SOAR_8_ONLY
        }
#endif

#ifndef FEW_CALLBACKS

        if (current_agent(e_cycles_this_d_cycle) == 0) {
            soar_invoke_callbacks(soar_agent, BEFORE_DECISION_CYCLE_CALLBACK, (soar_call_data) NULL);
        }
#endif

#ifndef DONT_DO_IO_CYCLES

        if (current_agent(input_cycle_flag) == TRUE) {  /* AGR REW1 */

#ifndef FEW_CALLBACKS

            soar_invoke_callbacks(soar_agent, BEFORE_INPUT_PHASE_CALLBACK, (soar_call_data) NULL);
#endif

            /* SW : 120199 timer bug fix */
#ifndef NO_TIMING_STUFF
#ifndef KERNEL_TIME_ONLY
            stop_timer(&current_agent(start_phase_tv),
                       &current_agent(decision_cycle_phase_timers[current_agent(current_phase)]));
#endif
            stop_timer(&current_agent(start_kernel_tv), &current_agent(total_kernel_time));
            start_timer(&current_agent(start_kernel_tv));
#endif
            /* SW : 120199 end */

            do_input_cycle();

            /* SW : 120199 timer bug fix */
#ifndef NO_TIMING_STUFF
            stop_timer(&current_agent(start_kernel_tv), &current_agent(input_function_cpu_time));
            start_timer(&current_agent(start_kernel_tv));
#ifndef KERNEL_TIME_ONLY
            start_timer(&current_agent(start_phase_tv));
#endif
#endif
            /* SW : 120199 end */

#ifndef FEW_CALLBACKS
            soar_invoke_callbacks(soar_agent, AFTER_INPUT_PHASE_CALLBACK, (soar_call_data) NULL);
#endif
            if (current_agent(input_period))
                current_agent(input_cycle_flag) = FALSE;
        }
        /* AGR REW1 this line and 1 previous line */
#endif                          /* DONT_DO_IO_CYCLES */

        /* REW: begin 09.15.96 */
#ifndef SOAR_8_ONLY
        if (current_agent(operand2_mode) == TRUE) {
#endif
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
#ifndef SOAR_8_ONLY
        } else {
            if (any_assertions_or_retractions_ready())
                current_agent(current_phase) = PREFERENCE_PHASE;
            else
                current_agent(current_phase) = DECISION_PHASE;
        }
#endif

        /* REW: end   09.15.96 */

        /* SW : 120199 timer bug fix */
        /* REW: begin 28.07.96 */
#ifndef KERNEL_TIME_ONLY
#ifndef NO_TIMING_STUFF
        stop_timer(&current_agent(start_phase_tv), &current_agent(decision_cycle_phase_timers[INPUT_PHASE]));
#endif
#endif
        /* REW: end 28.07.96 */
        /* SW 120199 End */

        break;

    case DETERMINE_LEVEL_PHASE:

#ifndef NO_TIMING_STUFF
#ifndef KERNEL_TIME_ONLY
        start_timer(&current_agent(start_phase_tv));
#endif
#endif

        /* Still need to register callbacks for both before and after
           the determine_level procedure call. */

        if (current_agent(applyPhase))
            determine_highest_active_production_level_in_stack_apply();
        else
            determine_highest_active_production_level_in_stack_propose();
#ifndef NO_TIMING_STUFF
#ifndef KERNEL_TIME_ONLY
        stop_timer(&current_agent(start_phase_tv), &current_agent(decision_cycle_phase_timers[DETERMINE_LEVEL_PHASE]));
#endif
#endif

        break;

    case PREFERENCE_PHASE:

        /* REW: begin 28.07.96 */
#ifndef NO_TIMING_STUFF
#ifndef KERNEL_TIME_ONLY
        start_timer(&current_agent(start_phase_tv));
#endif
#endif
        /* REW: end 28.07.96 */

#ifndef FEW_CALLBACKS
        soar_invoke_callbacks(soar_agent, BEFORE_PREFERENCE_PHASE_CALLBACK, (soar_call_data) NULL);
#endif
        do_preference_phase();

#ifndef FEW_CALLBACKS
        soar_invoke_callbacks(soar_agent, AFTER_PREFERENCE_PHASE_CALLBACK, (soar_call_data) NULL);
#endif
        current_agent(current_phase) = WM_PHASE;

        /* REW: begin 28.07.96 */
#ifndef NO_TIMING_STUFF
#ifndef KERNEL_TIME_ONLY
        stop_timer(&current_agent(start_phase_tv), &current_agent(decision_cycle_phase_timers[PREFERENCE_PHASE]));
#endif
#endif
        /* REW: end 28.07.96 */

#ifndef SOAR_8_ONLY
        if (current_agent(operand2_mode) == FALSE)
            break;
#endif
        /* if we're in Soar8, then go right to WM_PHASE without stopping */

    case WM_PHASE:
        /* REW: begin 28.07.96 */
#ifndef NO_TIMING_STUFF
#ifndef KERNEL_TIME_ONLY
        start_timer(&current_agent(start_phase_tv));
#endif
#endif
        /* REW: end 28.07.96 */

#ifndef FEW_CALLBACKS
        soar_invoke_callbacks(soar_agent, BEFORE_WM_PHASE_CALLBACK, (soar_call_data) NULL);
#endif
        do_working_memory_phase();

#ifndef FEW_CALLBACKS
        soar_invoke_callbacks(soar_agent, AFTER_WM_PHASE_CALLBACK, (soar_call_data) NULL);
#endif

#ifndef SOAR_8_ONLY
        if (current_agent(operand2_mode) == TRUE) {
#endif
            /* KJC - New Order: Always follow WM PHASE with DETERMINE_LEVEL_PHASE */
            current_agent(current_phase) = DETERMINE_LEVEL_PHASE;

            /* Update accounting.  Moved here by KJC 10-02-98 */
            current_agent(e_cycle_count)++;
            current_agent(e_cycles_this_d_cycle)++;

            if (current_agent(FIRING_TYPE) == PE_PRODS) {
                /* Keep track of each pe_phase for reporting in stats */
                current_agent(pe_cycle_count)++;
                current_agent(pe_cycles_this_d_cycle)++;
            }
#ifndef SOAR_8_ONLY
        }
#endif
        /* REW: end   10.29.97 */

        else

            current_agent(current_phase) = OUTPUT_PHASE;

        /* REW: begin 28.07.96 */
#ifndef NO_TIMING_STUFF
#ifndef KERNEL_TIME_ONLY
        stop_timer(&current_agent(start_phase_tv), &current_agent(decision_cycle_phase_timers[WM_PHASE]));
#endif
#endif
        /* REW: end 28.07.96 */
        break;

    case OUTPUT_PHASE:
        /* REW: begin 28.07.96 */
#ifndef NO_TIMING_STUFF
#ifndef KERNEL_TIME_ONLY
        start_timer(&current_agent(start_phase_tv));
#endif
#endif
        /* REW: end 28.07.96 */

#ifndef FEW_CALLBACKS
        soar_invoke_callbacks(soar_agent, BEFORE_OUTPUT_PHASE_CALLBACK, (soar_call_data) NULL);
#endif

#ifndef DONT_DO_IO_CYCLES

        /* REW: begin 28.07.96 */
#ifndef NO_TIMING_STUFF
#ifndef KERNEL_TIME_ONLY
        stop_timer(&current_agent(start_phase_tv),
                   &current_agent(decision_cycle_phase_timers[current_agent(current_phase)]));
#endif
        stop_timer(&current_agent(start_kernel_tv), &current_agent(total_kernel_time));
        start_timer(&current_agent(start_kernel_tv));
#endif
        /* REW: end 28.07.96 */

        do_output_cycle();

        /* REW: begin 28.07.96 */
#ifndef NO_TIMING_STUFF
        stop_timer(&current_agent(start_kernel_tv), &current_agent(output_function_cpu_time));
        start_timer(&current_agent(start_kernel_tv));
#ifndef KERNEL_TIME_ONLY
        start_timer(&current_agent(start_phase_tv));
#endif
#endif

#endif                          /* DONT_DO_IO_CYCLES */

        /* REW: end 28.07.96 */

#ifndef FEW_CALLBACKS
        soar_invoke_callbacks(soar_agent, AFTER_OUTPUT_PHASE_CALLBACK, (soar_call_data) NULL);
#endif

        /* REW: begin 09.15.96 */
#ifndef SOAR_8_ONLY
        if (current_agent(operand2_mode) == TRUE) {
#endif
            /* After each OUTPUT_PHASE in Operand2/Waterfall, always return to the 
               DETERMINE_LEVEL_PHASE */
            /* changed to INPUT LEVEL. KJC 10-04-98 */
            current_agent(current_phase) = INPUT_PHASE;

#ifdef DC_HISTOGRAM
            if (current_agent(dc_histogram_now)) {
                /* We check this value /before/ incrementing the d_cycle_count
                 * because we want to know when we have /finished/ the dc
                 * which is amultiple of dc_histogram_freq.
                 * now, we store that value, restart the timer, and incr the d.c. count
                 */
                stop_timer(&current_agent(start_dc_tv),
                           &current_agent(dc_histogram_tv)[(current_agent(d_cycle_count) /
                                                            current_agent(dc_histogram_freq)) - 1]);
                start_timer(&current_agent(start_dc_tv));
                current_agent(dc_histogram_now) = FALSE;
            }
#endif                          /* DC_HISTOGRAM */

            /*
             * SW 112999 was:
             * current_agent(d_cycle_count)++;
             */
            increment_current_agent_d_cycle_count;

            /* timers stopped KJC 10-04-98 */
#ifndef NO_TIMING_STUFF
#ifndef KERNEL_TIME_ONLY
            stop_timer(&current_agent(start_phase_tv), &current_agent(decision_cycle_phase_timers[OUTPUT_PHASE]));
#endif
#endif
            break;

#ifndef SOAR_8_ONLY
        }
#endif
        /* REW: end 09.15.96 */

        /* otherwise we're in Soar7 mode ...  */

        current_agent(e_cycle_count)++;
        current_agent(e_cycles_this_d_cycle)++;

        /* MVP 6-8-94 */
        if (current_agent(e_cycles_this_d_cycle) >= (unsigned long) current_agent(sysparams)[MAX_ELABORATIONS_SYSPARAM]) {
            if (current_agent(sysparams)[PRINT_WARNINGS_SYSPARAM])
                print("\nWarning: reached max-elaborations; proceeding to decision phase.");
            current_agent(current_phase) = DECISION_PHASE;
        } else
            current_agent(current_phase) = INPUT_PHASE;

        /* REW: begin 28.07.96 */
#ifndef NO_TIMING_STUFF
#ifndef KERNEL_TIME_ONLY
        stop_timer(&current_agent(start_phase_tv), &current_agent(decision_cycle_phase_timers[OUTPUT_PHASE]));
#endif
#endif
        /* REW: end 28.07.96 */
        break;

    case DECISION_PHASE:
        /* REW: begin 28.07.96 */
#ifndef NO_TIMING_STUFF
#ifndef KERNEL_TIME_ONLY
        start_timer(&current_agent(start_phase_tv));
#endif
#endif
        /* REW: end 28.07.96 */

        /* d_cycle_count moved to input phase for Soar 8 new decision cycle */
#ifndef SOAR_8_ONLY
        if (current_agent(operand2_mode) == FALSE) {
            /*
             * SW 11299 was:
             * current_agent(d_cycle_count)++;
             */
            increment_current_agent_d_cycle_count;
        }
#endif

/* AGR REW1 begin */
        if (!current_agent(input_period))
            current_agent(input_cycle_flag) = TRUE;
        else if ((current_agent(d_cycle_count) % current_agent(input_period)) == 0)
            current_agent(input_cycle_flag) = TRUE;

#ifndef FEW_CALLBACKS
/* AGR REW1 end */
        soar_invoke_callbacks(soar_agent, BEFORE_DECISION_PHASE_CALLBACK, (soar_call_data) NULL);
#endif
        do_decision_phase();

#ifndef NO_ADP_CALLBACK
        soar_invoke_callbacks(soar_agent, AFTER_DECISION_PHASE_CALLBACK, (soar_call_data) NULL);
#endif

#ifndef NO_ADC_CALLBACK
        soar_invoke_callbacks(soar_agent, AFTER_DECISION_CYCLE_CALLBACK, (soar_call_data) NULL);
#endif

        if (current_agent(sysparams)[TRACE_CONTEXT_DECISIONS_SYSPARAM]) {

            /* 
               The following statement was previously used only in conjunction
               with the TCL interface, however, the new formating is valid
               for all Soar8 derivitives, and thus we will use this,
               as opposed to the next (commented out) block.
               081699 SW
             */
            print_string("\n");

            /* Unnecessary as of 081699 SW change */
            /*
               if(current_agent(printer_output_column) != 1)
               print_string ("\n");
             */

            print_lowest_slot_in_context_stack();
        }
#ifndef SOAR_8_ONLY
        if (current_agent(operand2_mode) == FALSE) {
            current_agent(chunks_this_d_cycle) = 0;
        }
#endif

        current_agent(e_cycles_this_d_cycle) = 0;
        current_agent(current_phase) = INPUT_PHASE;

        /* REW: begin 09.15.96 */
#ifndef SOAR_8_ONLY
        if (current_agent(operand2_mode) == TRUE) {
#endif

#ifdef AGRESSIVE_ONC
            /* test for ONC, if TRUE, generate substate and go to OUTPUT */
            if ((current_agent(ms_o_assertions) == NIL) && (current_agent(bottom_goal)->id.operator_slot->wmes != NIL)) {

#ifndef FEW_CALLBACKS
                soar_invoke_callbacks(soar_agent, BEFORE_DECISION_PHASE_CALLBACK, (soar_call_data) NULL);
#endif
                do_decision_phase();

#ifndef FEW_CALLBACKS
                soar_invoke_callbacks(soar_agent, AFTER_DECISION_PHASE_CALLBACK, (soar_call_data) NULL);
#endif

                if (current_agent(sysparams)[TRACE_CONTEXT_DECISIONS_SYSPARAM]) {
                    /* 
                       The following statement was previously used only in conjunction
                       with the TCL interface, however, the new formating is valid
                       for all Soar8 derivitives, and thus we will use this,
                       as opposed to the next (commented out) block.
                       081699 SW
                     */
                    print_string("\n");

                    /* Unneeded as of 081699 SW changes */
                    /*
                       if(current_agent(printer_output_column) != 1) print_string ("\n");
                     */

                    print_lowest_slot_in_context_stack();
                }

                /* set phase to OUTPUT */
                current_agent(current_phase) = OUTPUT_PHASE;

                /* REW: begin 28.07.96 */
#ifndef NO_TIMING_STUFF
#ifndef KERNEL_TIME_ONLY
                stop_timer(&current_agent(start_phase_tv), &current_agent(decision_cycle_phase_timers[DECISION_PHASE]));
#endif
#endif
                /* REW: end 28.07.96 */

                break;

            } else
/* end AGRESSIVE_ONC	*/
#endif

            {
                /* print("\nSetting next phase to APPLY following a decision...."); */
                current_agent(applyPhase) = TRUE;
                current_agent(FIRING_TYPE) = PE_PRODS;
                current_agent(current_phase) = DETERMINE_LEVEL_PHASE;
                /* 'prime' the cycle for a new round of production firings 
                   in the APPLY (pref/wm) phase */
                initialize_consistency_calculations_for_new_decision();
            }
#ifndef SOAR_8_ONLY
        }
#endif

        /* REW: begin 28.07.96 */
#if !defined(NO_TIMING_STUFF) && !defined(KERNEL_TIME_ONLY)
        stop_timer(&current_agent(start_phase_tv), &current_agent(decision_cycle_phase_timers[DECISION_PHASE]));
#endif
        /* REW: end 28.07.96 */

        break;                  /* end DECISION phase */

    }                           /* end switch stmt for current_phase */

    /* --- update WM size statistics --- */
    if (current_agent(num_wmes_in_rete) > current_agent(max_wm_size))
        current_agent(max_wm_size) = current_agent(num_wmes_in_rete);
    current_agent(cumulative_wm_size) += current_agent(num_wmes_in_rete);
    current_agent(num_wm_sizes_accumulated)++;

    if (current_agent(system_halted)) {
        current_agent(stop_soar) = TRUE;
        current_agent(reason_for_stopping) = "System halted.";

        soar_invoke_callbacks(soar_agent, AFTER_HALT_SOAR_CALLBACK, (soar_call_data) NULL);
    }

    if (current_agent(stop_soar)) {

        /* (voigtjr)
           this old test is nonsense, it compares pointers:

           if (current_agent(reason_for_stopping) != "")

           what really should happen here is reason_for_stopping should be
           set to NULL in the cases where nothing should be printed, instead 
           of being assigned a pointer to a zero length (NULL) string, then
           we could simply say:

           if (current_agent(reason_for_stopping)) 
         */
        if (current_agent(reason_for_stopping)) {
            if (strcmp(current_agent(reason_for_stopping), "") != 0) {
                print("\n%s", current_agent(reason_for_stopping));
            }
        }
    }
}

void run_forever(void)
{
#ifndef NO_TIMING_STUFF
    start_timer(&current_agent(start_total_tv));
    start_timer(&current_agent(start_kernel_tv));
#endif
    current_agent(stop_soar) = FALSE;
    current_agent(reason_for_stopping) = "";
    while (!current_agent(stop_soar)) {
        do_one_top_level_phase();
    }
#ifndef NO_TIMING_STUFF
    stop_timer(&current_agent(start_kernel_tv), &current_agent(total_kernel_time));
    stop_timer(&current_agent(start_total_tv), &current_agent(total_cpu_time));
#endif
}

void run_for_n_phases(long n)
{
    if (n == -1) {
        run_forever();
        return;
    }
    if (n < -1)
        return;
#ifndef NO_TIMING_STUFF
    start_timer(&current_agent(start_total_tv));
    start_timer(&current_agent(start_kernel_tv));
#endif
    current_agent(stop_soar) = FALSE;
    current_agent(reason_for_stopping) = "";
    while (!current_agent(stop_soar) && n) {
        do_one_top_level_phase();
        n--;
    }
#ifndef NO_TIMING_STUFF
    stop_timer(&current_agent(start_total_tv), &current_agent(total_cpu_time));
    stop_timer(&current_agent(start_kernel_tv), &current_agent(total_kernel_time));
#endif
}

void run_for_n_elaboration_cycles(long n)
{
    long e_cycles_at_start, d_cycles_at_start, elapsed_cycles;

    if (n == -1) {
        run_forever();
        return;
    }
    if (n < -1)
        return;
#ifndef NO_TIMING_STUFF
    start_timer(&current_agent(start_total_tv));
    start_timer(&current_agent(start_kernel_tv));
#endif
    current_agent(stop_soar) = FALSE;
    current_agent(reason_for_stopping) = "";
    e_cycles_at_start = current_agent(e_cycle_count);
    d_cycles_at_start = current_agent(d_cycle_count);
    /* need next line or runs only the input phase for "d 1" after init-soar */
#ifndef SOAR_8_ONLY
    if (current_agent(operand2_mode) && (d_cycles_at_start == 0))
#else
    if (d_cycles_at_start == 0)
#endif
        d_cycles_at_start++;
    while (!current_agent(stop_soar)) {
        elapsed_cycles = (current_agent(d_cycle_count) - d_cycles_at_start) +
            (current_agent(e_cycle_count) - e_cycles_at_start);
        if (n == elapsed_cycles)
            break;
        do_one_top_level_phase();
    }
#ifndef NO_TIMING_STUFF
    stop_timer(&current_agent(start_total_tv), &current_agent(total_cpu_time));
    stop_timer(&current_agent(start_kernel_tv), &current_agent(total_kernel_time));
#endif
}

void run_for_n_modifications_of_output(long n)
{
    bool was_output_phase;
    long count = 0;

    if (n == -1) {
        run_forever();
        return;
    }
    if (n < -1)
        return;
#ifndef NO_TIMING_STUFF
    start_timer(&current_agent(start_total_tv));
    start_timer(&current_agent(start_kernel_tv));
#endif
    current_agent(stop_soar) = FALSE;
    current_agent(reason_for_stopping) = "";

    while (!current_agent(stop_soar) && n) {
        was_output_phase = (bool) (current_agent(current_phase) == OUTPUT_PHASE);
        do_one_top_level_phase();

        if (was_output_phase) {
            if (current_agent(output_link_changed)) {

                n--;
            } else {
                count++;
            }
        }
        if (count > 15) {
            current_agent(stop_soar) = TRUE;
            current_agent(reason_for_stopping) = "exceeded 15 cycles with no output";
        }
    }
#ifndef NO_TIMING_STUFF
    stop_timer(&current_agent(start_kernel_tv), &current_agent(total_kernel_time));
    stop_timer(&current_agent(start_total_tv), &current_agent(total_cpu_time));
#endif
}

void run_for_n_decision_cycles(long n)
{
    long d_cycles_at_start;

    if (n == -1) {
        run_forever();
        return;
    }
    if (n < -1)
        return;
#ifndef NO_TIMING_STUFF
    start_timer(&current_agent(start_total_tv));
    start_timer(&current_agent(start_kernel_tv));
#endif
    current_agent(stop_soar) = FALSE;
    current_agent(reason_for_stopping) = "";
    d_cycles_at_start = current_agent(d_cycle_count);
    /* need next line or runs only the input phase for "d 1" after init-soar */
#ifndef SOAR_8_ONLY
    if (current_agent(operand2_mode) && (d_cycles_at_start == 0))
#else
    if (d_cycles_at_start == 0)
#endif
        d_cycles_at_start++;
    while (!current_agent(stop_soar)) {
        if (n == (long) (current_agent(d_cycle_count) - d_cycles_at_start))
            break;
        do_one_top_level_phase();
    }

#ifndef NO_TIMING_STUFF
    stop_timer(&current_agent(start_total_tv), &current_agent(total_cpu_time));
    stop_timer(&current_agent(start_kernel_tv), &current_agent(total_kernel_time));
#endif
}

Symbol *attr_of_slot_just_decided(void)
{
    if (current_agent(bottom_goal)->id.operator_slot->wmes)
        return current_agent(operator_symbol);
    return current_agent(state_symbol);
}

void run_for_n_selections_of_slot(long n, Symbol * attr_of_slot)
{
    long count;
    bool was_decision_phase;

    if (n == -1) {
        run_forever();
        return;
    }
    if (n < -1)
        return;
#ifndef NO_TIMING_STUFF
    start_timer(&current_agent(start_total_tv));
    start_timer(&current_agent(start_kernel_tv));
#endif
    current_agent(stop_soar) = FALSE;
    current_agent(reason_for_stopping) = "";
    count = 0;
    while (!current_agent(stop_soar) && (count < n)) {
        was_decision_phase = (bool) (current_agent(current_phase) == DECISION_PHASE);
        do_one_top_level_phase();
        if (was_decision_phase)
            if (attr_of_slot_just_decided() == attr_of_slot)
                count++;
    }
#ifndef NO_TIMING_STUFF
    stop_timer(&current_agent(start_total_tv), &current_agent(total_cpu_time));
    stop_timer(&current_agent(start_kernel_tv), &current_agent(total_kernel_time));
#endif
}

void run_for_n_selections_of_slot_at_level(long n, Symbol * attr_of_slot, goal_stack_level level)
{
    long count;
    bool was_decision_phase;

    if (n == -1) {
        run_forever();
        return;
    }
    if (n < -1)
        return;
#ifndef NO_TIMING_STUFF
    start_timer(&current_agent(start_total_tv));
    start_timer(&current_agent(start_kernel_tv));
#endif
    current_agent(stop_soar) = FALSE;
    current_agent(reason_for_stopping) = "";
    count = 0;
    while (!current_agent(stop_soar) && (count < n)) {
        was_decision_phase = (bool) (current_agent(current_phase) == DECISION_PHASE);
        do_one_top_level_phase();
        if (was_decision_phase) {
            if (current_agent(bottom_goal)->id.level < level)
                break;
            if (current_agent(bottom_goal)->id.level == level) {
                if (attr_of_slot_just_decided() == attr_of_slot)
                    count++;
            }
        }
    }
#ifndef NO_TIMING_STUFF
    stop_timer(&current_agent(start_total_tv), &current_agent(total_cpu_time));
    stop_timer(&current_agent(start_kernel_tv), &current_agent(total_kernel_time));
#endif
}

/* ===================================================================

                     Print the Startup Banner

=================================================================== */

char *soar_news_string = "\
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

void print_startup_banner(void)
{
    print(soar_version_string);
    print(soar_news_string);
}

/* ===================================================================
   
             Loading the Initialization File ".init.soar"

   This routine looks for a file ".init.soar" in either the current
   directory or $HOME, and if found, loads it.
=================================================================== */

extern char *getenv();

int terminate_soar(void)
{
    /* Shouldn't we free *all* agents here? */
    free((void *) soar_agent);

    exit_soar();
    return 0;                   /* unreachable, but without it, gcc -Wall warns here */
}
