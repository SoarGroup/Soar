#include <portability.h>
#include "soar_rand.h" // provides SoarRand, a better random number generator (see bug 595)

/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/*************************************************************************
 *
 *  file:  init_soar.cpp
 *
 * =======================================================================
 *  Routines for initializing Soar, signal handling (ctrl-c interrupt),
 *  exiting Soar (cleanup and error msgs), setting sysparams, and
 *  the core routines for "running" Soar (do_one_top_level_phase, etc.)
 * =======================================================================
 */

/* 
   RDF 20020710: Added this define because some needed timer 
   functionality has been hidden behind the __USE_BSD define in sys/time.h
   (see sys/features.h for more information)
*/
#ifndef _BSD_SOURCE
# define _BSD_SOURCE
#endif

#include "init_soar.h"
#include "agent.h"
#include "consistency.h"
#include "callback.h"
#include "agent.h"
#include "print.h"
#include "production.h"
#include "decide.h"
#include "recmem.h"
#include "explain.h"
#include "symtab.h"
#include "wmem.h"
#include "io_soar.h"
#include "rete.h"
#include "gdatastructs.h"
#include "xml.h"
#include "utilities.h"

#include <assert.h>
#include <time.h>

#include "reinforcement_learning.h"

#include "episodic_memory.h"

#include "wma.h"


#define INIT_FILE       "init.soar"

/* REW: begin 08.20.97   these defined in consistency.c  */
extern void determine_highest_active_production_level_in_stack_propose(agent* thisAgent);
extern void determine_highest_active_production_level_in_stack_apply(agent* thisAgent);
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

// JRV: these functions are no longer used with SML
//void just_before_exit_soar (agent* thisAgent) {
//  soar_invoke_callbacks(thisAgent, 
//			SYSTEM_TERMINATION_CALLBACK,
//			(soar_call_data) TRUE);
//}

//void exit_soar (agent* thisAgent) {
////#ifdef _WINDOWS
////  print(thisAgent, "Cannot exit from Soar via the command line.\n");
////#else
//  just_before_exit_soar(thisAgent);
//  exit (0);
////#endif
//}

void abort_with_fatal_error (agent* thisAgent, char *msg) {
  FILE *f;
  char* warning = "Soar cannot recover from this error. \nYou will have to restart Soar to run an agent.\nData is still available for inspection, but may be corrupt.\nIf a log was open, it has been closed for safety.";
  
  print (thisAgent, "%s", msg);
  print (thisAgent, "%s", warning);
  
  fprintf (stderr,"%s",msg);
  fprintf (stderr,warning);
  
  xml_generate_error(thisAgent, msg);
  xml_generate_error(thisAgent, warning);

  f = fopen("soarerror", "w");
  fprintf (f,"%s",msg);
  fprintf (f,warning);
  fclose(f);

  assert(false);

  // Since we're no longer terminating, should we be invoking this event?
  // Note that this is a soar callback, not a gSKI callback, so it isn't being used for now anyway
  //soar_invoke_callbacks(thisAgent, 
		//	SYSTEM_TERMINATION_CALLBACK,
		//	(soar_call_data) FALSE);     
}

/* ===================================================================
   
                        Signal Handling

   Setup things so control_c_handler() gets control whenever the program
   receives a SIGINT (e.g., from a ctrl-c at the keyboard).  The handler
   just sets the stop_soar flag.
=================================================================== */

/* This is deprecated. -AJC (8/9/02) */
//char * c_interrupt_msg = "*** Ctrl-C Interrupt ***";

/* AGR 581  The variable the_signal is not used at all, so I thought I
   would remove it.  Unfortunately, the signal command at the end of this
   function requires a function name that has a single integer parameter.
   It's probably some unix thing.  So I left the variable in the parameter
   list and instead changed the calling functions to use a parameter.
   94.11.15 (although this was done a month or two earlier--this comment
   was placed here in retrospect.)  */

/* Removed these because they are deprecated -AJC (8/6/02) */

//void control_c_handler (int the_signal) {
///* Windows 3.1 can't do ^C handling */
//#ifndef _WINDOWS
//
//  cons * c;
//  agent * the_agent;
///*
//  for(c = thisKernel->all_soar_agents; c != NIL; c = c->rest) {
//    the_agent = ((agent *) c->first);
//    the_agent->stop_soar = TRUE;
//    the_agent->reason_for_stopping =  c_interrupt_msg;
//  }
//*/
//  /* --- reinstall this signal handler -- some brain-damaged OS's uninstall
//     it after delivering the signal --- */
//  signal (SIGINT, control_c_handler);
//
//#endif


//void setup_signal_handling (void) {
//#ifndef _WINDOWS
//  if (signal(SIGINT, control_c_handler) == SIG_ERR) {
//    fprintf(stderr, "setup_signal_handling: unable to install signal handler.\n");
//    fprintf(stderr, "                       Ctrl-C will not interrupt Soar.\n");
//  }
//
//#endif /* _WINDOWS */

/* ===================================================================
   
                            Sysparams

=================================================================== */


void set_sysparam (agent* thisAgent, int param_number, long new_value) {
	if ((param_number < 0) || (param_number > HIGHEST_SYSPARAM_NUMBER)) {
		print (thisAgent, "Internal error: tried to set bad sysparam #: %d\n", param_number);
		return;
	}
	thisAgent->sysparams[param_number] = new_value;
	
	soar_invoke_callbacks(thisAgent, 
		SYSTEM_PARAMETER_CHANGED_CALLBACK,
		(soar_call_data) param_number);		
}

void init_sysparams (agent* thisAgent) {
  int i;

  for (i=0; i<HIGHEST_SYSPARAM_NUMBER+1; i++) thisAgent->sysparams[i] = 0;
  
  /* --- set all params to zero, except the following: --- */
  thisAgent->sysparams[TRACE_CONTEXT_DECISIONS_SYSPARAM] = TRUE;
  thisAgent->sysparams[TRACE_FIRINGS_OF_CHUNKS_SYSPARAM] = FALSE;
  thisAgent->sysparams[TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM] = NONE_WME_TRACE; /* RPM 6/05 Changed from timetag to none */
  thisAgent->sysparams[TRACE_CHUNK_NAMES_SYSPARAM] = FALSE;
  thisAgent->sysparams[TRACE_JUSTIFICATION_NAMES_SYSPARAM] = FALSE;
  thisAgent->sysparams[TRACE_LOADING_SYSPARAM] = TRUE; /* KJC 8/96 */
  thisAgent->sysparams[MAX_ELABORATIONS_SYSPARAM] = 100;
  thisAgent->sysparams[MAX_CHUNKS_SYSPARAM] = 50;
  thisAgent->sysparams[MAX_NIL_OUTPUT_CYCLES_SYSPARAM] = 15;
  thisAgent->sysparams[MAX_GOAL_DEPTH] = 100;  /* generate an interrupt so users can recover before exceed program stack*/
  thisAgent->sysparams[MAX_MEMORY_USAGE_SYSPARAM] = 100000000; /* default to 100MB.  Event generated when exceeded*/

//#ifdef USE_X_DISPLAY
//  thisAgent->sysparams[RESPOND_TO_LOAD_ERRORS_SYSPARAM] = FALSE;
//#else
  thisAgent->sysparams[RESPOND_TO_LOAD_ERRORS_SYSPARAM] = TRUE;
//#endif

#ifdef ATTENTION_LAPSE
  /* RMJ */
  thisAgent->sysparams[ATTENTION_LAPSE_ON_SYSPARAM] = FALSE;
#endif /* ATTENTION_LAPSE */

  // voigtjr:  turning learning off by default
  thisAgent->sysparams[LEARNING_ON_SYSPARAM] = FALSE;

  thisAgent->sysparams[LEARNING_ONLY_SYSPARAM] = FALSE;  /* AGR MVL1 */
  thisAgent->sysparams[LEARNING_EXCEPT_SYSPARAM] = FALSE;  /* KJC 8/96 */
  thisAgent->sysparams[LEARNING_ALL_GOALS_SYSPARAM] = TRUE;
  thisAgent->sysparams[USER_SELECT_MODE_SYSPARAM] = USER_SELECT_SOFTMAX;  
  thisAgent->sysparams[USER_SELECT_REDUCE_SYSPARAM] = FALSE;
  thisAgent->sysparams[PRINT_WARNINGS_SYSPARAM] = TRUE;
  thisAgent->sysparams[PRINT_ALIAS_SYSPARAM] = TRUE;  /* AGR 627 */
  thisAgent->sysparams[EXPLAIN_SYSPARAM] = FALSE; /* KJC 7/96 */
  thisAgent->sysparams[USE_LONG_CHUNK_NAMES] = TRUE;  /* kjh(B14) */
  thisAgent->sysparams[TRACE_OPERAND2_REMOVALS_SYSPARAM] = FALSE;
  thisAgent->sysparams[TIMERS_ENABLED] = TRUE;
  
  // JRV: Chunk through local negations by default
  thisAgent->sysparams[CHUNK_THROUGH_LOCAL_NEGATIONS_SYSPARAM] = TRUE;
}

/* ===================================================================
   
                     Adding and Removing Pwatchs

   Productions_being_traced is a (consed) list of all productions
   on which a pwatch has been set.  Pwatchs are added/removed via
   calls to add_pwatch() and remove_pwatch().
=================================================================== */
/* list of production structures */


void add_pwatch (agent* thisAgent, production *prod) 
{
  if (prod->trace_firings) return;
  prod->trace_firings = TRUE;
  push (thisAgent, prod, thisAgent->productions_being_traced);
}

Bool remove_pwatch_test_fn (agent* /*thisAgent*/, cons *c,
							       void *prod_to_remove_pwatch_of) 
{
  return (c->first == static_cast<production *>(prod_to_remove_pwatch_of));
}

void remove_pwatch (agent* thisAgent, production *prod) {
  if (! prod->trace_firings) return;
  prod->trace_firings = FALSE;
  free_list (thisAgent, 
             extract_list_elements ( thisAgent, 
                                    &thisAgent->productions_being_traced,
                                     remove_pwatch_test_fn, prod));
}

/* ===================================================================
   
                         Reinitializing Soar

   Reset_statistics() resets all the statistics (except the firing counts
   on each individual production).  Reinitialize_soar() does all the 
   work for an init-soar.
=================================================================== */

void reset_production_firing_counts(agent* thisAgent) {
  int t;
  production * p;

  for (t = 0; t < NUM_PRODUCTION_TYPES; t++) {
    for (p = thisAgent->all_productions_of_type[t]; 
	 p != NIL; 
	 p = p->next)
      p->firing_count = 0;
  }
}

void reset_statistics (agent* thisAgent) {

  thisAgent->d_cycle_count = 0;
  thisAgent->decision_phases_count = 0;
  thisAgent->e_cycle_count = 0;
  thisAgent->e_cycles_this_d_cycle = 0;
  thisAgent->chunks_this_d_cycle = 0;
  thisAgent->production_firing_count = 0;
  thisAgent->wme_addition_count = 0;
  thisAgent->wme_removal_count = 0;
  thisAgent->max_wm_size = 0;
  thisAgent->cumulative_wm_size = 0.0;
  thisAgent->num_wm_sizes_accumulated = 0;
/* REW: begin 09.15.96 */
  thisAgent->pe_cycle_count = 0;
  thisAgent->pe_cycles_this_d_cycle = 0;
/* REW: end   09.15.96 */
  thisAgent->d_cycle_last_output = 0;   // KJC 11/17/05

  thisAgent->run_phase_count = 0 ;
  thisAgent->run_elaboration_count = 0 ;
  thisAgent->run_last_output_count = 0 ;
  thisAgent->run_generated_output_count = 0 ;

  thisAgent->inner_e_cycle_count = 0;

  reset_production_firing_counts(thisAgent);

/* These are ALWAYS created in create_soar_agent, so might as 
 * well remove the compiler directives and always reset them.
 * KJC June 2005
 */
  /* Initializing all the timer structures */
  reset_timer (&thisAgent->start_total_tv);
  reset_timer (&thisAgent->total_cpu_time);
  reset_timer (&thisAgent->start_kernel_tv);
  reset_timer (&thisAgent->start_phase_tv);
  reset_timer (&thisAgent->total_kernel_time);

  reset_timer (&thisAgent->input_function_cpu_time);
  reset_timer (&thisAgent->output_function_cpu_time);

  reset_timer (&thisAgent->start_gds_tv);
  reset_timer (&thisAgent->total_gds_time);

  for (int ii=0;ii < NUM_PHASE_TYPES; ii++) {
     reset_timer (&thisAgent->decision_cycle_phase_timers[ii]);
     reset_timer (&thisAgent->monitors_cpu_time[ii]);
     reset_timer (&thisAgent->ownership_cpu_time[ii]);
     reset_timer (&thisAgent->chunking_cpu_time[ii]);
     reset_timer (&thisAgent->match_cpu_time[ii]);
     reset_timer (&thisAgent->gds_cpu_time[ii]);
  }

  thisAgent->epmem_timers->reset();  
}

bool reinitialize_soar (agent* thisAgent) {

  /* kjh (CUSP-B4) begin */
  long cur_TRACE_CONTEXT_DECISIONS_SYSPARAM;
  long cur_TRACE_PHASES_SYSPARAM;
  long cur_TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM;
  long cur_TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM;
  long cur_TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM;
  long cur_TRACE_FIRINGS_PREFERENCES_SYSPARAM;
  long cur_TRACE_WM_CHANGES_SYSPARAM;
  /* kjh (CUSP-B4) end */

  thisAgent->did_PE = FALSE;    /* RCHONG:  10.11 */

  soar_invoke_callbacks(thisAgent, 
		       BEFORE_INIT_SOAR_CALLBACK,
		       (soar_call_data) NULL);		 

  /* kjh (CUSP-B4) begin */
  /* Stash trace state: */
  cur_TRACE_CONTEXT_DECISIONS_SYSPARAM        = thisAgent->sysparams[TRACE_CONTEXT_DECISIONS_SYSPARAM];
  cur_TRACE_PHASES_SYSPARAM                   = thisAgent->sysparams[TRACE_PHASES_SYSPARAM];
  cur_TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM = thisAgent->sysparams[TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM];
  cur_TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM    = thisAgent->sysparams[TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM];
  cur_TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM   = thisAgent->sysparams[TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM];
  cur_TRACE_FIRINGS_PREFERENCES_SYSPARAM      = thisAgent->sysparams[TRACE_FIRINGS_PREFERENCES_SYSPARAM];
  cur_TRACE_WM_CHANGES_SYSPARAM               = thisAgent->sysparams[TRACE_WM_CHANGES_SYSPARAM];

  /* Temporarily disable tracing: */
  set_sysparam(thisAgent, TRACE_CONTEXT_DECISIONS_SYSPARAM,        FALSE);
  set_sysparam(thisAgent, TRACE_PHASES_SYSPARAM,                   FALSE);
  set_sysparam(thisAgent, TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM, FALSE);
  set_sysparam(thisAgent, TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM,    FALSE);
  set_sysparam(thisAgent, TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM,   NONE_WME_TRACE);
  set_sysparam(thisAgent, TRACE_FIRINGS_PREFERENCES_SYSPARAM,      FALSE);
  set_sysparam(thisAgent, TRACE_WM_CHANGES_SYSPARAM,               FALSE);
  /* kjh (CUSP-B4) end */

  rl_reset_data( thisAgent );
  wma_deinit( thisAgent );
  clear_goal_stack (thisAgent);  
  thisAgent->rl_stats->reset();
  thisAgent->wma_stats->reset();
  thisAgent->epmem_stats->reset();

  if (thisAgent->operand2_mode == TRUE) {
     thisAgent->active_level = 0; /* Signal that everything should be retracted */
     thisAgent->FIRING_TYPE = IE_PRODS;
     do_preference_phase (thisAgent);   /* allow all i-instantiations to retract */
  }
  /* REW: end  09.15.96 */
  else
  do_preference_phase (thisAgent);   /* allow all instantiations to retract */

  reset_explain(thisAgent);
  bool ok = reset_id_counters (thisAgent);
  reset_wme_timetags (thisAgent);
  reset_statistics (thisAgent);

  // should come after reset statistics
  wma_init( thisAgent );

  // JRV: For XML generation
  xml_reset( thisAgent );

  /* RDF 01282003: Reinitializing the various halt and stop flags */
  thisAgent->system_halted = FALSE;
  thisAgent->stop_soar = FALSE;			// voigtjr:  this line doesn't exist in other kernel
  thisAgent->reason_for_stopping = "";  // voigtjr: nor does this one

  thisAgent->go_number = 1;
  thisAgent->go_type = GO_DECISION;

  /* kjh (CUSP-B4) begin */
  /* Restore trace state: */
  set_sysparam(thisAgent, TRACE_CONTEXT_DECISIONS_SYSPARAM,        cur_TRACE_CONTEXT_DECISIONS_SYSPARAM);
  set_sysparam(thisAgent, TRACE_PHASES_SYSPARAM,                   cur_TRACE_PHASES_SYSPARAM);
  set_sysparam(thisAgent, TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM, cur_TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM);
  set_sysparam(thisAgent, TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM,    cur_TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM);
  set_sysparam(thisAgent, TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM,   cur_TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM);
  set_sysparam(thisAgent, TRACE_FIRINGS_PREFERENCES_SYSPARAM,      cur_TRACE_FIRINGS_PREFERENCES_SYSPARAM);
  set_sysparam(thisAgent, TRACE_WM_CHANGES_SYSPARAM,               cur_TRACE_WM_CHANGES_SYSPARAM);
  /* kjh (CUSP-B4) end */

  soar_invoke_callbacks(thisAgent, 
		       AFTER_INIT_SOAR_CALLBACK,
		       (soar_call_data) NULL);

  thisAgent->input_cycle_flag = TRUE;  /* reinitialize flag  AGR REW1 */
  thisAgent->current_phase = INPUT_PHASE;  /* moved here June 05 from loop below.  KJC */

  /* REW: begin 09.15.96 */
  if (thisAgent->operand2_mode == TRUE) {
     thisAgent->FIRING_TYPE = IE_PRODS;  /* KJC 10.05.98 was PE */
     thisAgent->did_PE = FALSE;
  }
  /* REW: end 09.15.96 */

  // voigtjr: WARN_IF_TIMERS_REPORT_ZERO block goes here in other kernel
  return ok ;
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

void do_one_top_level_phase (agent* thisAgent) 
{
  //  Symbol *iterate_goal_sym;  kjc commented /* RCHONG: end 10.11 */

  if (thisAgent->system_halted) 
  {
    print (thisAgent,
	   "\nSystem halted.  Use (init-soar) before running Soar again.");
	xml_generate_error(thisAgent, "System halted.  Use (init-soar) before running Soar again.");
    thisAgent->stop_soar = TRUE;
    thisAgent->reason_for_stopping = "System halted.";
    return;
  }

  /*
   *  This code was commented by SoarTech with the addition of gSKI
   *  because gSKI requires the existence of a ^state and ^io link
   *  before the first Run cmd is issued.  
   *  But what if we uncommented this code anyway, then if ever
   *  gSKI isn't wrapped around the kernel, this code will 
   *  execute and create the links.  KJC 4/05
   *
   * if (! thisAgent->top_goal) 
   * {
   *  create_top_goal(thisAgent);
   *  if (thisAgent->sysparams[TRACE_CONTEXT_DECISIONS_SYSPARAM]) 
   *    {
   *      print_string (thisAgent, "\n");
   *      print_lowest_slot_in_context_stack (thisAgent);
   *    }
   *  thisAgent->current_phase = INPUT_PHASE;
   *  if (thisAgent->operand2_mode) 
   *    thisAgent->d_cycle_count++;
   * }
   */

  switch (thisAgent->current_phase) {

  case INPUT_PHASE:

	 if (thisAgent->sysparams[TRACE_PHASES_SYSPARAM])
         print_phase (thisAgent, "\n--- Input Phase --- \n",0);

	  /* for Operand2 mode using the new decision cycle ordering,
       * we need to do some initialization in the INPUT PHASE, which
       * now comes first.  e_cycles are also zeroed before the APPLY Phase.
       */
	 if (thisAgent->operand2_mode == TRUE) {
		  thisAgent->chunks_this_d_cycle = 0;
          thisAgent->e_cycles_this_d_cycle = 0;
	 }
	 #ifndef NO_TIMING_STUFF   /* REW:  28.07.96 */
     start_timer (thisAgent, &thisAgent->start_phase_tv);
     #endif

	  /* we check e_cycle_count because Soar 7 runs multiple input cycles per decision */
	  /* always true for Soar 8 */
	 if (thisAgent->e_cycles_this_d_cycle==0) {
	   soar_invoke_callbacks(thisAgent, 
			     BEFORE_DECISION_CYCLE_CALLBACK,
			     (soar_call_data) INPUT_PHASE);
	 }  /* end if e_cycles_this_d_cycle == 0 */

     #ifdef REAL_TIME_BEHAVIOR  /* RM Jones */
	 test_for_input_delay(thisAgent);
     #endif
     #ifdef ATTENTION_LAPSE  /* RM Jones */
	 determine_lapsing(thisAgent);
     #endif
    
    if (thisAgent->input_cycle_flag == TRUE) { /* Soar 7 flag, but always true for Soar8 */
      soar_invoke_callbacks(thisAgent, 
		  BEFORE_INPUT_PHASE_CALLBACK,
		  (soar_call_data) INPUT_PHASE);

      do_input_cycle(thisAgent);

	  thisAgent->run_phase_count++ ;
	  thisAgent->run_elaboration_count++ ;	// All phases count as a run elaboration
      soar_invoke_callbacks(thisAgent, 
		  AFTER_INPUT_PHASE_CALLBACK,
		  (soar_call_data) INPUT_PHASE);

	  if (thisAgent->input_period) 
		  thisAgent->input_cycle_flag = FALSE;
	 }  /* END if (input_cycle_flag==TRUE) AGR REW1 this line and 1 previous line */
  
	 if (thisAgent->sysparams[TRACE_PHASES_SYSPARAM])
        print_phase (thisAgent, "\n--- END Input Phase --- \n",1);

     #ifndef NO_TIMING_STUFF  /* REW:  28.07.96 */
        stop_timer (thisAgent, &thisAgent->start_phase_tv, 
                   &thisAgent->decision_cycle_phase_timers[INPUT_PHASE]);
     #endif

  	 if (thisAgent->operand2_mode == TRUE)  {
		thisAgent->current_phase = PROPOSE_PHASE;
	 }
	 else {  /* we're running in Soar7 mode */
        if (any_assertions_or_retractions_ready(thisAgent)) 
           thisAgent->current_phase = PREFERENCE_PHASE;
        else
           thisAgent->current_phase = DECISION_PHASE;    	
	 }

    break;  /* END of INPUT PHASE */

  /////////////////////////////////////////////////////////////////////////////////

  case PROPOSE_PHASE:   /* added in 8.6 to clarify Soar8 decision cycle */
	  
      #ifndef NO_TIMING_STUFF
      start_timer (thisAgent, &thisAgent->start_phase_tv);
      #endif

	  /* e_cycles_this_d_cycle will always be zero UNLESS we are 
	   * running by ELABORATIONS.
	   * We only want to do the following if we've just finished INPUT and are
	   * starting PROPOSE.  If this is the second elaboration for PROPOSE, then 
	   * just do the while loop below.   KJC  June 05
	   */
	  if (thisAgent->e_cycles_this_d_cycle < 1) {
			if (thisAgent->sysparams[TRACE_PHASES_SYSPARAM])
				print_phase(thisAgent, "\n--- Proposal Phase ---\n",0);

			soar_invoke_callbacks(thisAgent, 
									BEFORE_PROPOSE_PHASE_CALLBACK,
									(soar_call_data) PROPOSE_PHASE);
		 
			// We need to generate this event here in case no elaborations fire...
			// FIXME return the correct enum top_level_phase constant in soar_call_data?
			/*(soar_call_data)((thisAgent->applyPhase == TRUE)? gSKI_K_APPLY_PHASE: gSKI_K_PROPOSAL_PHASE)*/
			soar_invoke_callbacks(thisAgent, BEFORE_ELABORATION_CALLBACK, NULL ) ; 
			
  		   /* 'Prime the decision for a new round of production firings at the end of
			* REW:   05.05.97   */  /*  KJC 04.05 moved here from INPUT_PHASE for 8.6.0 */
			initialize_consistency_calculations_for_new_decision(thisAgent);

			thisAgent->FIRING_TYPE = IE_PRODS;
			thisAgent->applyPhase = FALSE;   /* KJC 04/05: do we still need this line?  gSKI does*/
			determine_highest_active_production_level_in_stack_propose(thisAgent);

			if (thisAgent->current_phase == DECISION_PHASE) 
			{  // no elaborations will fire this phase
				thisAgent->run_elaboration_count++ ;	// All phases count as a run elaboration
				// FIXME return the correct enum top_level_phase constant in soar_call_data?
				/*(soar_call_data)((thisAgent->applyPhase == TRUE)? gSKI_K_APPLY_PHASE: gSKI_K_PROPOSAL_PHASE)*/
				soar_invoke_callbacks(thisAgent, AFTER_ELABORATION_CALLBACK, NULL ) ;
			}
	  }

		/* max-elaborations are checked in determine_highest_active... and if they
		* are reached, the current phase is set to DECISION.  phase is also set
		* to DECISION when PROPOSE is done.
		*/

	  while (thisAgent->current_phase != DECISION_PHASE) {
		  if (thisAgent->e_cycles_this_d_cycle) 
		  {  // only for 2nd cycle or higher.  1st cycle fired above
			  // FIXME return the correct enum top_level_phase constant in soar_call_data? /*(soar_call_data)((thisAgent->applyPhase == TRUE)? gSKI_K_APPLY_PHASE: gSKI_K_PROPOSAL_PHASE)*/
			  soar_invoke_callbacks(thisAgent, BEFORE_ELABORATION_CALLBACK, NULL ) ;
		  }

		  do_preference_phase(thisAgent);
		  do_working_memory_phase(thisAgent);

		  /* Update accounting.  Moved here by KJC 04/05/05 */
          thisAgent->e_cycle_count++;
          thisAgent->e_cycles_this_d_cycle++;
		  thisAgent->run_elaboration_count++ ;
		  determine_highest_active_production_level_in_stack_propose(thisAgent);
		    // FIXME return the correct enum top_level_phase constant in soar_call_data? /*(soar_call_data)((thisAgent->applyPhase == TRUE)? gSKI_K_APPLY_PHASE: gSKI_K_PROPOSAL_PHASE)*/
			soar_invoke_callbacks(thisAgent, AFTER_ELABORATION_CALLBACK, NULL ) ;
          if (thisAgent->go_type == GO_ELABORATION) break;
	  }
 
      /*  If we've finished PROPOSE, then current_phase will be equal to DECISION
	   *  otherwise, we're only stopping because we're running by ELABORATIONS, so
	   *  don't do the end-of-phase updating in that case.
	   */
	  if (thisAgent->current_phase == DECISION_PHASE) {
		   /* This is a HACK for Soar 8.6.0 beta release... KCoulter April 05
			* We got here, because we should move to DECISION, so PROPOSE is done
			* Set phase back to PROPOSE, do print_phase, callbacks, and then
			* reset phase to DECISION
			*/
			thisAgent->current_phase = PROPOSE_PHASE;
			if (thisAgent->sysparams[TRACE_PHASES_SYSPARAM]) {
				print_phase(thisAgent, "\n--- END Proposal Phase ---\n",1);
			}

			thisAgent->run_phase_count++ ;
 			soar_invoke_callbacks(thisAgent, 
									AFTER_PROPOSE_PHASE_CALLBACK,
								(soar_call_data) PROPOSE_PHASE);
			thisAgent->current_phase = DECISION_PHASE;
	  }

	  #ifndef NO_TIMING_STUFF
	  stop_timer (thisAgent, &thisAgent->start_phase_tv, 
                 &thisAgent->decision_cycle_phase_timers[PROPOSE_PHASE]);  
      #endif

	  break;  /* END of Soar8 PROPOSE PHASE */

  /////////////////////////////////////////////////////////////////////////////////
  case PREFERENCE_PHASE:
      /* starting with 8.6.0, PREFERENCE_PHASE is only Soar 7 mode -- applyPhase not valid here */
      /* needs to be updated for gSKI interface, and gSKI needs to accommodate Soar 7 */
	
      /* JC ADDED: Tell gski about elaboration phase beginning */
	  // FIXME return the correct enum top_level_phase constant in soar_call_data? /*(soar_call_data)((thisAgent->applyPhase == TRUE)? gSKI_K_APPLY_PHASE: gSKI_K_PROPOSAL_PHASE)*/
	 soar_invoke_callbacks(thisAgent, BEFORE_ELABORATION_CALLBACK, NULL ) ;

      #ifndef NO_TIMING_STUFF       /* REW: 28.07.96 */
      start_timer (thisAgent, &thisAgent->start_phase_tv);
      #endif

 	   soar_invoke_callbacks(thisAgent, 
			                 BEFORE_PREFERENCE_PHASE_CALLBACK,
			                 (soar_call_data) PREFERENCE_PHASE);
 
	  do_preference_phase(thisAgent);

	  thisAgent->run_phase_count++ ;
	  thisAgent->run_elaboration_count++ ;	// All phases count as a run elaboration
       soar_invoke_callbacks(thisAgent, 
			 AFTER_PREFERENCE_PHASE_CALLBACK,
			 (soar_call_data) PREFERENCE_PHASE);
 	  thisAgent->current_phase = WM_PHASE;

      #ifndef NO_TIMING_STUFF       /* REW:  28.07.96 */
      stop_timer (thisAgent, &thisAgent->start_phase_tv, 
                 &thisAgent->decision_cycle_phase_timers[PREFERENCE_PHASE]);
      #endif

	  /* tell gSKI PREF_PHASE ending... 
	  */
      break;      /* END of Soar7 PREFERENCE PHASE */
 
  /////////////////////////////////////////////////////////////////////////////////
  case WM_PHASE:
      /* starting with 8.6.0, WM_PHASE is only Soar 7 mode; see PROPOSE and APPLY */
      /* needs to be updated for gSKI interface, and gSKI needs to accommodate Soar 7 */

    /*  we need to tell gSKI WM Phase beginning... */

      #ifndef NO_TIMING_STUFF  	  /* REW: begin 28.07.96 */
	  start_timer (thisAgent, &thisAgent->start_phase_tv);
      #endif	

 	  soar_invoke_callbacks(thisAgent, 
			                BEFORE_WM_PHASE_CALLBACK,
			                (soar_call_data) WM_PHASE);
 
	  do_working_memory_phase(thisAgent);

	  thisAgent->run_phase_count++ ;
	  thisAgent->run_elaboration_count++ ;	// All phases count as a run elaboration
 	  soar_invoke_callbacks(thisAgent, 
			 AFTER_WM_PHASE_CALLBACK,
			 (soar_call_data) WM_PHASE);
 
	  thisAgent->current_phase = OUTPUT_PHASE;
 
      #ifndef NO_TIMING_STUFF      /* REW:  28.07.96 */
      stop_timer (thisAgent, &thisAgent->start_phase_tv, 
                 &thisAgent->decision_cycle_phase_timers[WM_PHASE]);
      #endif

	  // FIXME return the correct enum top_level_phase constant in soar_call_data? /*(soar_call_data)((thisAgent->applyPhase == TRUE)? gSKI_K_APPLY_PHASE: gSKI_K_PROPOSAL_PHASE)*/
	  soar_invoke_callbacks(thisAgent, AFTER_ELABORATION_CALLBACK, NULL ) ;

     break;     /* END of Soar7 WM PHASE */

  /////////////////////////////////////////////////////////////////////////////////
  case APPLY_PHASE:   /* added in 8.6 to clarify Soar8 decision cycle */

      #ifndef NO_TIMING_STUFF
      start_timer (thisAgent, &thisAgent->start_phase_tv);
      #endif

	  /* e_cycle_count will always be zero UNLESS we are running by ELABORATIONS.
	   * We only want to do the following if we've just finished DECISION and are
	   * starting APPLY.  If this is the second elaboration for APPLY, then 
	   * just do the while loop below.   KJC  June 05
	   */
	  if (thisAgent->e_cycles_this_d_cycle < 1) {

			if (thisAgent->sysparams[TRACE_PHASES_SYSPARAM]) 
				print_phase (thisAgent, "\n--- Application Phase ---\n",0);

 			soar_invoke_callbacks(thisAgent, 
									BEFORE_APPLY_PHASE_CALLBACK,
									(soar_call_data) APPLY_PHASE);

			// We need to generate this event here in case no elaborations fire...
			// FIXME return the correct enum top_level_phase constant in soar_call_data? /*(soar_call_data)((thisAgent->applyPhase == TRUE)? gSKI_K_APPLY_PHASE: gSKI_K_PROPOSAL_PHASE)*/
			soar_invoke_callbacks(thisAgent, BEFORE_ELABORATION_CALLBACK, NULL ) ;
		 
			/* 'prime' the cycle for a new round of production firings 
			* in the APPLY (pref/wm) phase *//* KJC 04.05 moved here from end of DECISION */
			initialize_consistency_calculations_for_new_decision(thisAgent);

			thisAgent->FIRING_TYPE = PE_PRODS;  /* might get reset in det_high_active_prod_level... */
			thisAgent->applyPhase = TRUE;       /* KJC 04/05: do we still need this line?  gSKI does*/
			determine_highest_active_production_level_in_stack_apply(thisAgent);
			if (thisAgent->current_phase == OUTPUT_PHASE) 
			{  // no elaborations will fire this phase	
				thisAgent->run_elaboration_count++ ;	// All phases count as a run elaboration
				// FIXME return the correct enum top_level_phase constant in soar_call_data? /*(soar_call_data)((thisAgent->applyPhase == TRUE)? gSKI_K_APPLY_PHASE: gSKI_K_PROPOSAL_PHASE)*/
				soar_invoke_callbacks(thisAgent, AFTER_ELABORATION_CALLBACK, NULL ) ;
			}
	  }
	  /* max-elaborations are checked in determine_highest_active... and if they
	   * are reached, the current phase is set to OUTPUT.  phase is also set
	   * to OUTPUT when APPLY is done.
	   */

      while (thisAgent->current_phase != OUTPUT_PHASE) {
          /* JC ADDED: Tell gski about elaboration phase beginning */
		  if (thisAgent->e_cycles_this_d_cycle) 
		  {  // only for 2nd cycle or higher.  1st cycle fired above     
			  // FIXME return the correct enum top_level_phase constant in soar_call_data? /*(soar_call_data)((thisAgent->applyPhase == TRUE)? gSKI_K_APPLY_PHASE: gSKI_K_PROPOSAL_PHASE)*/
			soar_invoke_callbacks(thisAgent, BEFORE_ELABORATION_CALLBACK, NULL ) ;
		  }

		  do_preference_phase(thisAgent);
		  do_working_memory_phase(thisAgent);

		  /* Update accounting.  Moved here by KJC 04/05/05 */
          thisAgent->e_cycle_count++;
          thisAgent->e_cycles_this_d_cycle++;
		  thisAgent->run_elaboration_count++ ;

		  if (thisAgent->FIRING_TYPE == PE_PRODS) { 
			  thisAgent->pe_cycle_count++;
	  		  thisAgent->pe_cycles_this_d_cycle++;
	  	  }
		  determine_highest_active_production_level_in_stack_apply(thisAgent);
		  // FIXME return the correct enum top_level_phase constant in soar_call_data? /*(soar_call_data)((thisAgent->applyPhase == TRUE)? gSKI_K_APPLY_PHASE: gSKI_K_PROPOSAL_PHASE)*/
		  soar_invoke_callbacks(thisAgent, AFTER_ELABORATION_CALLBACK, NULL ) ;

		  if (thisAgent->go_type == GO_ELABORATION) break;
      }

      /*  If we've finished APPLY, then current_phase will be equal to OUTPUT
	   *  otherwise, we're only stopping because we're running by ELABORATIONS, so
	   *  don't do the end-of-phase updating in that case.
	   */
	  if (thisAgent->current_phase == OUTPUT_PHASE) {
		   /* This is a HACK for Soar 8.6.0 beta release... KCoulter April 05
			* We got here, because we should move to OUTPUT, so APPLY is done
			* Set phase back to APPLY, do print_phase, callbacks and reset phase to OUTPUT
			*/
 			thisAgent->current_phase = APPLY_PHASE;
 			if (thisAgent->sysparams[TRACE_PHASES_SYSPARAM]) {
				print_phase(thisAgent, "\n--- END Application Phase ---\n",1);
			}
			thisAgent->run_phase_count++ ;
 			soar_invoke_callbacks(thisAgent, 
					AFTER_APPLY_PHASE_CALLBACK,
					(soar_call_data) APPLY_PHASE);
 
			thisAgent->current_phase = OUTPUT_PHASE;
	  }

	  #ifndef NO_TIMING_STUFF
	  stop_timer (thisAgent, &thisAgent->start_phase_tv, 
                 &thisAgent->decision_cycle_phase_timers[APPLY_PHASE]);  
      #endif

      break;  /* END of Soar8 APPLY PHASE */

  /////////////////////////////////////////////////////////////////////////////////    
  case OUTPUT_PHASE:

	  if (thisAgent->sysparams[TRACE_PHASES_SYSPARAM])
          print_phase (thisAgent, "\n--- Output Phase ---\n",0);
      
      #ifndef NO_TIMING_STUFF      /* REW:  28.07.96 */
	  start_timer (thisAgent, &thisAgent->start_phase_tv);
      #endif   

 	  soar_invoke_callbacks(thisAgent, 
			 BEFORE_OUTPUT_PHASE_CALLBACK,
			 (soar_call_data) OUTPUT_PHASE);
 
	  /** KJC June 05:  moved output function timers into do_output_cycle ***/

	  do_output_cycle(thisAgent);

	  if ( wma_enabled( thisAgent ) )
		  wma_move_and_remove_wmes( thisAgent );

	  if ( epmem_enabled( thisAgent ) && ( thisAgent->epmem_params->phase->get_value() == epmem_param_container::phase_output ) )
		  epmem_go( thisAgent );

	  // Count the outputs the agent generates (or times reaching max-nil-outputs without sending output)
	  if (thisAgent->output_link_changed || ((++(thisAgent->run_last_output_count)) >= (unsigned long)thisAgent->sysparams[MAX_NIL_OUTPUT_CYCLES_SYSPARAM]))
	  {
		  thisAgent->run_last_output_count = 0 ;
		  thisAgent->run_generated_output_count++ ;
	  }

	  thisAgent->run_phase_count++ ;
	  thisAgent->run_elaboration_count++ ;	// All phases count as a run elaboration
 	  soar_invoke_callbacks(thisAgent, 
			 AFTER_OUTPUT_PHASE_CALLBACK,
			 (soar_call_data) OUTPUT_PHASE);
 
      /* REW: begin 09.15.96 */
      if (thisAgent->operand2_mode == TRUE) {
		  // JRV: Get rid of the cached XML after every decision but before the after-decision-phase callback
		  xml_invoke_callback( thisAgent ); // invokes XML_GENERATION_CALLBACK, clears XML state

		  /* KJC June 05:  moved here from DECISION Phase */
 	      soar_invoke_callbacks(thisAgent, 
		                    	AFTER_DECISION_CYCLE_CALLBACK,
		 	                    (soar_call_data) OUTPUT_PHASE);
          #ifndef NO_TIMING_STUFF    /* timers stopped KJC 10-04-98 */
          stop_timer (thisAgent, &thisAgent->start_phase_tv, 
                     &thisAgent->decision_cycle_phase_timers[OUTPUT_PHASE]);
          #endif

		  if (thisAgent->sysparams[TRACE_PHASES_SYSPARAM])
              print_phase (thisAgent, "\n--- END Output Phase ---\n",1);
    	  thisAgent->current_phase = INPUT_PHASE;
          thisAgent->d_cycle_count++;	  
	  break;
	  }     /* REW: end 09.15.96 */

    
	  /* ******************* otherwise we're in Soar7 mode ...  */

	  thisAgent->e_cycle_count++;
	  thisAgent->e_cycles_this_d_cycle++;
	  thisAgent->run_elaboration_count++ ;	// All phases count as a run elaboration

      if (thisAgent->sysparams[TRACE_PHASES_SYSPARAM])
          print_phase (thisAgent, "\n--- END Output Phase ---\n",1);

      /* MVP 6-8-94 */
      if (thisAgent->e_cycles_this_d_cycle >=
		  (unsigned long)(thisAgent->sysparams[MAX_ELABORATIONS_SYSPARAM])) {
			  if (thisAgent->sysparams[PRINT_WARNINGS_SYSPARAM]) {			
				  print (thisAgent, "\nWarning: reached max-elaborations; proceeding to decision phase.");
				  xml_generate_warning(thisAgent, "Warning: reached max-elaborations; proceeding to decision phase.");
			  }
		  thisAgent->current_phase = DECISION_PHASE;
	  } else
		  thisAgent->current_phase = INPUT_PHASE;
     
	  /* REW: begin 28.07.96 */
      #ifndef NO_TIMING_STUFF   
	  stop_timer (thisAgent, &thisAgent->start_phase_tv, 
		  &thisAgent->decision_cycle_phase_timers[OUTPUT_PHASE]);
      #endif
      /* REW: end 28.07.96 */

     break;
    
  /////////////////////////////////////////////////////////////////////////////////
  case DECISION_PHASE:
      /* not yet cleaned up for 8.6.0 release */

	  if (thisAgent->sysparams[TRACE_PHASES_SYSPARAM])
		  print_phase (thisAgent, "\n--- Decision Phase ---\n",0);
	  	 
      #ifndef NO_TIMING_STUFF   /* REW:  28.07.96 */
	  start_timer (thisAgent, &thisAgent->start_phase_tv);
      #endif

      /* d_cycle_count moved to input phase for Soar 8 new decision cycle */
      if (thisAgent->operand2_mode == FALSE) 
         thisAgent->d_cycle_count++;
	  thisAgent->decision_phases_count++;  /* counts decisions, not cycles, for more accurate stats */

      /* AGR REW1 begin */
	  if (!thisAgent->input_period) 
		  thisAgent->input_cycle_flag = TRUE;
	  else if ((thisAgent->d_cycle_count % thisAgent->input_period) == 0)
		  thisAgent->input_cycle_flag = TRUE;
      /* AGR REW1 end */
 
       soar_invoke_callbacks(thisAgent, 
	 		 BEFORE_DECISION_PHASE_CALLBACK,
			 (soar_call_data) DECISION_PHASE);
 
	  do_decision_phase(thisAgent);

	  thisAgent->run_phase_count++ ;
	  thisAgent->run_elaboration_count++ ;	// All phases count as a run elaboration

	  soar_invoke_callbacks(thisAgent, 
			 AFTER_DECISION_PHASE_CALLBACK,
			 (soar_call_data) DECISION_PHASE);

	  if (thisAgent->sysparams[TRACE_CONTEXT_DECISIONS_SYSPARAM]) {
     //     #ifdef USE_TCL
		  print_string (thisAgent, "\n");
    //      #else
		  //if(thisAgent->printer_output_column != 1)
			 // print_string ("\n");
    //      #endif /* USE_TCL */
		  print_lowest_slot_in_context_stack (thisAgent);
	  }

	  if (thisAgent->operand2_mode == FALSE) {
		  // JRV: Get rid of the cached XML after every decision but before the after-decision-phase callback
		  xml_invoke_callback( thisAgent ); // invokes XML_GENERATION_CALLBACK, clears XML state

          /* KJC June 05: Soar8 - moved AFTER_DECISION_CYCLE_CALLBACK to proper spot in OUTPUT */
 	      soar_invoke_callbacks(thisAgent, 
		                    	AFTER_DECISION_CYCLE_CALLBACK,
		 	                    (soar_call_data) DECISION_PHASE);
		  thisAgent->chunks_this_d_cycle = 0;
		  if (thisAgent->sysparams[TRACE_PHASES_SYSPARAM])
			  print_phase (thisAgent, "\n--- END Decision Phase ---\n",1);
    	  thisAgent->current_phase = INPUT_PHASE;
 	  }
	  /* reset elaboration counter */
      thisAgent->e_cycles_this_d_cycle = 0;
      thisAgent->pe_cycles_this_d_cycle = 0;

	  /* REW: begin 09.15.96 */
	  if (thisAgent->operand2_mode == TRUE) 
     {
#ifdef AGRESSIVE_ONC
		  /* test for Operator NC, if TRUE, generate substate and go to OUTPUT */
		  if ((thisAgent->ms_o_assertions == NIL) &&
			  (thisAgent->bottom_goal->id.operator_slot->wmes != NIL)) 
        {

 			  soar_invoke_callbacks(thisAgent, thisAgent, 
				                    BEFORE_DECISION_PHASE_CALLBACK,
				                    (soar_call_data) thisAgent->current_phase);
 
			  do_decision_phase(thisAgent);

			  soar_invoke_callbacks(thisAgent, thisAgent, AFTER_DECISION_PHASE_CALLBACK,
                                    (soar_call_data) thisAgent->current_phase);

			  if (thisAgent->sysparams[TRACE_CONTEXT_DECISIONS_SYSPARAM]) {
//                  #ifdef USE_TCL
				  print_string (thisAgent, "\n");
//                  #else
//				  if(thisAgent->printer_output_column != 1) print_string ("\n");
//                  #endif /* USE_TCL */
				  print_lowest_slot_in_context_stack (thisAgent);
			  }
			  if (thisAgent->sysparams[TRACE_PHASES_SYSPARAM])			 
				  print_phase (thisAgent, "\n--- END Decision Phase ---\n",1);

			  /* set phase to OUTPUT */
			  thisAgent->current_phase = OUTPUT_PHASE;

			  /* REW: begin 28.07.96 */
              #ifndef NO_TIMING_STUFF
              stop_timer (thisAgent, &thisAgent->start_phase_tv, 
				  &thisAgent->decision_cycle_phase_timers[DECISION_PHASE]);
              #endif
	          /* REW: end 28.07.96 */

			  break;
   
		  } else 
#endif //AGRESSIVE_ONC
		  {
			  if (thisAgent->sysparams[TRACE_PHASES_SYSPARAM])			 
				  print_phase (thisAgent, "\n--- END Decision Phase ---\n",1);

			  /* printf("\nSetting next phase to APPLY following a decision...."); */
			  thisAgent->applyPhase = TRUE;
			  thisAgent->FIRING_TYPE = PE_PRODS;
			  thisAgent->current_phase = APPLY_PHASE;
		  }
	  }
 
	  /* REW: begin 28.07.96 */
      #ifndef NO_TIMING_STUFF
	  stop_timer (thisAgent, &thisAgent->start_phase_tv, 
		  &thisAgent->decision_cycle_phase_timers[DECISION_PHASE]);
      #endif
	  /* REW: end 28.07.96 */

	  if ( epmem_enabled( thisAgent ) && ( thisAgent->epmem_params->phase->get_value() == epmem_param_container::phase_selection ) )
		epmem_go( thisAgent );

	  break;  /* end DECISION phase */
	  
  /////////////////////////////////////////////////////////////////////////////////

  default: // 2/24/05: added default case to quell gcc compile warning
	  assert(false && "Invalid phase enumeration value!");
	  break;

  }  /* end switch stmt for current_phase */
  
  /* --- update WM size statistics --- */
  if (thisAgent->num_wmes_in_rete > thisAgent->max_wm_size) 
      thisAgent->max_wm_size = thisAgent->num_wmes_in_rete;
  thisAgent->cumulative_wm_size += thisAgent->num_wmes_in_rete;
  thisAgent->num_wm_sizes_accumulated++;
  
  if (thisAgent->system_halted) {
	  thisAgent->stop_soar = TRUE;
	  thisAgent->reason_for_stopping = "System halted.";
	  soar_invoke_callbacks(thisAgent, 
		  AFTER_HALT_SOAR_CALLBACK,
		  (soar_call_data) thisAgent->current_phase);

	  // To model episodic task, after halt, perform RL update with next-state value 0
	  if ( rl_enabled( thisAgent ) )
	  {
		  for ( Symbol *g = thisAgent->bottom_goal; g; g = g->id.higher_goal)
		  {
			  rl_tabulate_reward_value_for_goal( thisAgent, g );
			  rl_perform_update( thisAgent, 0, true, g );
		  }
	  }
  }
  
  if (thisAgent->stop_soar) {
        /* (voigtjr)
           this old test is nonsense, it compares pointers:

           if (thisAgent->reason_for_stopping != "")

           what really should happen here is reason_for_stopping should be
           set to NULL in the cases where nothing should be printed, instead 
           of being assigned a pointer to a zero length (NULL) string, then
           we could simply say:

           if (thisAgent->reason_for_stopping) 
         */
        if (thisAgent->reason_for_stopping) {
            if (strcmp(thisAgent->reason_for_stopping, "") != 0) {
                print(thisAgent, "\n%s", thisAgent->reason_for_stopping);
            }
        }
  }
}

void run_forever (agent* thisAgent) {
    #ifndef NO_TIMING_STUFF
	start_timer (thisAgent, &thisAgent->start_total_tv);
	start_timer (thisAgent, &thisAgent->start_kernel_tv);
    #endif

	thisAgent->stop_soar = FALSE;
	thisAgent->reason_for_stopping = "";
	while (! thisAgent->stop_soar) {
		do_one_top_level_phase(thisAgent);
	}

    #ifndef NO_TIMING_STUFF
	stop_timer (thisAgent, &thisAgent->start_kernel_tv, &thisAgent->total_kernel_time);
	stop_timer (thisAgent, &thisAgent->start_total_tv, &thisAgent->total_cpu_time);
    #endif
}

void run_for_n_phases (agent* thisAgent, long n) {
  if (n == -1) { run_forever(thisAgent); return; }
  if (n < -1) return;
#ifndef NO_TIMING_STUFF
  start_timer (thisAgent, &thisAgent->start_total_tv);
  start_timer (thisAgent, &thisAgent->start_kernel_tv);
#endif
  thisAgent->stop_soar = FALSE;
  thisAgent->reason_for_stopping = "";
  while (!thisAgent->stop_soar && n) {
    do_one_top_level_phase(thisAgent);
    n--;
  }
#ifndef NO_TIMING_STUFF
  stop_timer (thisAgent, &thisAgent->start_kernel_tv, &thisAgent->total_kernel_time);
  stop_timer (thisAgent, &thisAgent->start_total_tv, &thisAgent->total_cpu_time);
#endif
}

void run_for_n_elaboration_cycles (agent* thisAgent, long n) {
  long e_cycles_at_start, d_cycles_at_start, elapsed_cycles = 0;
  go_type_enum save_go_type = GO_PHASE;
  
  if (n == -1) { run_forever(thisAgent); return; }
  if (n < -1) return;
#ifndef NO_TIMING_STUFF
  start_timer (thisAgent, &thisAgent->start_total_tv);
  start_timer (thisAgent, &thisAgent->start_kernel_tv);
#endif
  thisAgent->stop_soar = FALSE;
  thisAgent->reason_for_stopping = "";
  e_cycles_at_start = thisAgent->e_cycle_count;
  d_cycles_at_start = thisAgent->d_cycle_count;
  if ( thisAgent->operand2_mode ) {
     elapsed_cycles = -1; 
	 save_go_type = thisAgent->go_type;
	 thisAgent->go_type = GO_ELABORATION;
     /* need next line or runs only the input phase for "d 1" after init-soar */
     if (d_cycles_at_start == 0) d_cycles_at_start++;
  }
  while (!thisAgent->stop_soar) {
    if ( thisAgent->operand2_mode ) {
		elapsed_cycles++;
	} else {
		elapsed_cycles = (thisAgent->d_cycle_count-d_cycles_at_start) +
					     (thisAgent->e_cycle_count-e_cycles_at_start);
	}
    if (n==elapsed_cycles) break;
    do_one_top_level_phase(thisAgent);
  }
  if ( thisAgent->operand2_mode ) {thisAgent->go_type = save_go_type;}

#ifndef NO_TIMING_STUFF
  stop_timer (thisAgent, &thisAgent->start_total_tv, &thisAgent->total_cpu_time);
  stop_timer (thisAgent, &thisAgent->start_kernel_tv, &thisAgent->total_kernel_time);
#endif
}

void run_for_n_modifications_of_output (agent* thisAgent, long n) {
  Bool was_output_phase;
  long count = 0;

  if (n == -1) { run_forever(thisAgent); return; }
  if (n < -1) return;
#ifndef NO_TIMING_STUFF
  start_timer (thisAgent, &thisAgent->start_total_tv);
  start_timer (thisAgent, &thisAgent->start_kernel_tv);
#endif
  thisAgent->stop_soar = FALSE;
  thisAgent->reason_for_stopping = "";
  while (!thisAgent->stop_soar && n) {
    was_output_phase = (thisAgent->current_phase==OUTPUT_PHASE);
    do_one_top_level_phase(thisAgent);
    if (was_output_phase) {	
		if (thisAgent->output_link_changed) {
		  n--;
		} else {
		  count++;
	} }
	if (count >= thisAgent->sysparams[MAX_NIL_OUTPUT_CYCLES_SYSPARAM]) {
		thisAgent->stop_soar = TRUE;
		thisAgent->reason_for_stopping = "exceeded max_nil_output_cycles with no output";
	}
  }
#ifndef NO_TIMING_STUFF
  stop_timer (thisAgent, &thisAgent->start_kernel_tv, &thisAgent->total_kernel_time);
  stop_timer (thisAgent, &thisAgent->start_total_tv, &thisAgent->total_cpu_time);
#endif
}

void run_for_n_decision_cycles (agent* thisAgent, long n) {
  long d_cycles_at_start;
  
  if (n == -1) { run_forever(thisAgent); return; }
  if (n < -1) return;
#ifndef NO_TIMING_STUFF
  start_timer (thisAgent, &thisAgent->start_total_tv);
  start_timer (thisAgent, &thisAgent->start_kernel_tv);
#endif
  thisAgent->stop_soar = FALSE;
  thisAgent->reason_for_stopping = "";
  d_cycles_at_start = thisAgent->d_cycle_count;
  /* need next line or runs only the input phase for "d 1" after init-soar */
  if ( thisAgent->operand2_mode && (d_cycles_at_start == 0) )
    d_cycles_at_start++;
  while (!thisAgent->stop_soar) {
    if (n==(long)(thisAgent->d_cycle_count-d_cycles_at_start)) break;
    do_one_top_level_phase(thisAgent);
  }
#ifndef NO_TIMING_STUFF
  stop_timer (thisAgent, &thisAgent->start_total_tv, &thisAgent->total_cpu_time);
  stop_timer (thisAgent, &thisAgent->start_kernel_tv, &thisAgent->total_kernel_time);
#endif
}

Symbol *attr_of_slot_just_decided (agent* thisAgent) {
  if (thisAgent->bottom_goal->id.operator_slot->wmes) 
    return thisAgent->operator_symbol;
  return thisAgent->state_symbol;
}

void run_for_n_selections_of_slot (agent* thisAgent, long n, Symbol *attr_of_slot) {
  long count;
  Bool was_decision_phase;
  
  if (n == -1) { run_forever(thisAgent); return; }
  if (n < -1) return;
#ifndef NO_TIMING_STUFF
  start_timer (thisAgent, &thisAgent->start_total_tv);
  start_timer (thisAgent, &thisAgent->start_kernel_tv);
#endif
  thisAgent->stop_soar = FALSE;
  thisAgent->reason_for_stopping = "";
  count = 0;
  while (!thisAgent->stop_soar && (count < n)) {
    was_decision_phase = (thisAgent->current_phase==DECISION_PHASE);
    do_one_top_level_phase(thisAgent);
    if (was_decision_phase)
      if (attr_of_slot_just_decided(thisAgent)==attr_of_slot) count++;
  }
#ifndef NO_TIMING_STUFF
  stop_timer (thisAgent, &thisAgent->start_total_tv, &thisAgent->total_cpu_time);
  stop_timer (thisAgent, &thisAgent->start_kernel_tv, &thisAgent->total_kernel_time);
#endif
}

void run_for_n_selections_of_slot_at_level (agent* thisAgent, long n,
                                            Symbol *attr_of_slot,
                                            goal_stack_level level) {
  long count;
  Bool was_decision_phase;
  
  if (n == -1) { run_forever(thisAgent); return; }
  if (n < -1) return;
#ifndef NO_TIMING_STUFF
  start_timer (thisAgent, &thisAgent->start_total_tv);
  start_timer (thisAgent, &thisAgent->start_kernel_tv);
#endif
  thisAgent->stop_soar = FALSE;
  thisAgent->reason_for_stopping = "";
  count = 0;
  while (!thisAgent->stop_soar && (count < n)) {
    was_decision_phase = (thisAgent->current_phase==DECISION_PHASE);
    do_one_top_level_phase(thisAgent);
    if (was_decision_phase) {
      if (thisAgent->bottom_goal->id.level < level) break;
      if (thisAgent->bottom_goal->id.level==level) {
        if (attr_of_slot_just_decided(thisAgent)==attr_of_slot) count++;
      }
    }
  }
#ifndef NO_TIMING_STUFF
  stop_timer (thisAgent, &thisAgent->start_total_tv, &thisAgent->total_cpu_time);
  stop_timer (thisAgent, &thisAgent->start_kernel_tv, &thisAgent->total_kernel_time);
#endif
}

/* ===================================================================
   
             Loading the Initialization File ".init.soar"

   This routine looks for a file ".init.soar" in either the current
   directory or $HOME, and if found, loads it.
=================================================================== */

extern char *getenv();

/* AGR 536  Soar core dumped when it used filenames longer than 1000 chars
   but shorter than MAXPATHLEN (from sys/param.h).  4-May-94  */

// KJC Nov 05:  moved here from old interface.cpp, so could remove interface.* files
void load_file (agent* thisAgent, char *file_name, FILE *already_open_file) {
Bool old_print_prompt_flag;

  old_print_prompt_flag = thisAgent->print_prompt_flag;
  thisAgent->print_prompt_flag = FALSE;

  start_lex_from_file (thisAgent, file_name, already_open_file);
  //repeatedly_read_and_dispatch_commands (thisKernel, thisAgent);
  stop_lex_from_file (thisAgent);

  thisAgent->print_prompt_flag = old_print_prompt_flag;
}


//void load_init_file (Kernel* thisKernel, agent* thisAgent) {
//#define LOAD_INIT_FILE_BUFFER_SIZE 1000
//  char filename[LOAD_INIT_FILE_BUFFER_SIZE];   /* AGR 536 */
//  char *home_directory;
//  FILE *initfile;
//
//  strncpy (filename, INIT_FILE, LOAD_INIT_FILE_BUFFER_SIZE);
//  initfile = fopen (filename, "r");
//  if (!initfile) {
//    home_directory = getenv ("HOME");
//    if (home_directory) {
//      strncpy (filename, home_directory, LOAD_INIT_FILE_BUFFER_SIZE);
//      filename[LOAD_INIT_FILE_BUFFER_SIZE - 1] = 0; /* ensure null termination */
//      strncat (filename, "/",LOAD_INIT_FILE_BUFFER_SIZE - strlen(filename));
//      filename[LOAD_INIT_FILE_BUFFER_SIZE - 1] = 0; /* ensure null termination */
//      strncat (filename, INIT_FILE, LOAD_INIT_FILE_BUFFER_SIZE - strlen(filename));
//      filename[LOAD_INIT_FILE_BUFFER_SIZE - 1] = 0; /* ensure null termination */
//      initfile = fopen (filename, "r");
//    }
//  }
//
//  print_startup_banner(thisAgent);
//
//  if (initfile) {
//    print (thisAgent, "\nLoading %s\n",filename);
//    load_file (thisKernel, thisAgent, filename, initfile);
//    fclose (initfile);
//  }
//}
//
//int terminate_soar (agent* thisAgent)
//{
//  /* Shouldn't we free *all* agents here? */
//  free((void *) thisAgent);
//
//  exit_soar(thisAgent);  
//  return 0; /* unreachable, but without it, gcc -Wall warns here */
//}

/*
  RDF: 20020706 Added this for the gSKI project. This makes it so that
                created agents have a top state and the appropriate io
                header symbols and wme's even before the first decision
                cycle. This helps keep the gSKI interface a little saner.
*/

void init_agent_memory(agent* thisAgent)
{

  /* The following code was taken from the do_one_top_level_phase function
     near the top of this file */
  // If there is already a top goal this function should probably not be called
  assert( thisAgent->top_goal == 0 && 
          "There should be no top goal when init_agent_memory is called!");
  if ( thisAgent->top_goal) return;

  thisAgent->io_header = get_new_io_identifier (thisAgent, 'I');
  thisAgent->io_header_input = get_new_io_identifier (thisAgent, 'I');
  thisAgent->io_header_output = get_new_io_identifier (thisAgent, 'I');

  create_top_goal(thisAgent);
  if (thisAgent->sysparams[TRACE_CONTEXT_DECISIONS_SYSPARAM]) 
    {
      print_string (thisAgent, "\n");
      print_lowest_slot_in_context_stack (thisAgent);
    }
  thisAgent->current_phase = INPUT_PHASE;
  if (thisAgent->operand2_mode) 
    thisAgent->d_cycle_count++;

  /* The following code was taken from the do_input_cycle function of io.cpp */
  // Creating the io_header and adding the top state io header wme
  thisAgent->io_header_link = add_input_wme (thisAgent, 
                                             thisAgent->top_state,
                                             thisAgent->io_symbol,
                                             thisAgent->io_header);
  // Creating the input and output header symbols and wmes
  // RPM 9/06 changed to use thisAgent->input/output_link_symbol
  // Note we don't have to save these wmes for later release since their parent
  //  is already being saved (above), and when we release it they will automatically be released
  add_input_wme (thisAgent, thisAgent->io_header,
                 thisAgent->input_link_symbol,
                 thisAgent->io_header_input);
  add_input_wme (thisAgent, thisAgent->io_header,
                 thisAgent->output_link_symbol,
                 thisAgent->io_header_output);
  
  // KJC & RPM 10/06
  // A lot of stuff isn't initialized properly until the input and output cycles are run the first time.
  // Because of this, SW had to put in a hack to work around changes made to the output-link in the first
  //   dc not being visible. (see comment near end of update_for_top_state_wme_addition).  This change added
  //   an item to the associated_output_links list.
  // But the ol->ids_in_tc is still not initialized until the first output phase, so if we exit before that,
  //   remove_output_link_tc_info doesn't see it and doesn't clean up the associated_output_links list.
  // If we do run an output phase, though, the same item is added to the associated_output_links list twice.
  //   ol->ids_in_tc gets initialized, so remove_output_link_tc_info -- but it only cleans up the first copy
  //   of the item.
  // All of these problems come back to things not being initialized properly, so we run the input and output
  //   phases here to force proper initialization (and have commented out SW's changes to
  //   update_for_top_state_wme_addition).  This will cause somecallbacks to be triggered, but we don't think
  //   this is a problem for two reasons:
  //   1) these events are currently not exposed through SML, so no clients will see them
  //   2) even if these events were exposed, they are being fired during agent creation.  Since the agent
  //      hasn't been created yet, no client could have registered for the events anyway.
  // Note that this change replaces the do_buffered_wm_and_ownership_changes call which attempted to do some
  //   initialization (including triggering SW's changes).
  do_input_cycle(thisAgent);
  do_output_cycle(thisAgent);
  //do_buffered_wm_and_ownership_changes(thisAgent);

  /* executing the IO cycles above, increments the timers, so reset */
  /* Initializing all the timer structures */
  reset_timer (&thisAgent->start_total_tv);
  reset_timer (&thisAgent->total_cpu_time);
  reset_timer (&thisAgent->start_kernel_tv);
  reset_timer (&thisAgent->start_phase_tv);
  reset_timer (&thisAgent->total_kernel_time);

  reset_timer (&thisAgent->input_function_cpu_time);
  reset_timer (&thisAgent->output_function_cpu_time);

  reset_timer (&thisAgent->start_gds_tv);
  reset_timer (&thisAgent->total_gds_time);

  for (int ii=0;ii < NUM_PHASE_TYPES; ii++) {
     reset_timer (&thisAgent->decision_cycle_phase_timers[ii]);
     reset_timer (&thisAgent->monitors_cpu_time[ii]);
     reset_timer (&thisAgent->ownership_cpu_time[ii]);
     reset_timer (&thisAgent->chunking_cpu_time[ii]);
     reset_timer (&thisAgent->match_cpu_time[ii]);
     reset_timer (&thisAgent->gds_cpu_time[ii]);
  }

  thisAgent->epmem_timers->reset();

  // This is an important part of the state of the agent for io purposes
  // (see io.cpp for details)
  thisAgent->prev_top_state = thisAgent->top_state;

}
