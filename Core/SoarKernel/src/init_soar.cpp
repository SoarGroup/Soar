#include "portability.h"
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


#include "xml.h"
#include "stats.h"

#include <assert.h>
#include <time.h>

#include "reinforcement_learning.h"
#include "wma.h"
#include "episodic_memory.h"
#include "semantic_memory.h"
#include "variablization_manager.h"
#ifndef NO_SVS
#include "svs_interface.h"
#endif
#include "output_manager.h"

/* REW: begin 08.20.97   these defined in consistency.c  */
extern void determine_highest_active_production_level_in_stack_propose(agent* thisAgent);
extern void determine_highest_active_production_level_in_stack_apply(agent* thisAgent);
/* REW: end   08.20.97 */

#if (defined(REAL_TIME_BEHAVIOR) || defined(ATTENTION_LAPSE))
/* RMJ; just a temporary variable, but we don't want to
      reallocate it every time we process a phase, so we make it global
      and allocate memory for it in init_soar() (init agent.c) */
struct timeval* current_real_time;
#endif

#ifdef ATTENTION_LAPSE
/* RMJ; just a temporary variable, but we don't want to
      reallocate it every time we process a phase, so we make it global */
int64_t lapse_duration;
#endif

/* ===================================================================

                            Exiting Soar

   Abort_with_fatal_error(msg) terminates Soar, closing
   the log file before exiting.  It also prints
   an error message and tries to write a file before exiting.
=================================================================== */

/* MToDo | May also want to close debug and other databases here -- */

void abort_with_fatal_error(agent* thisAgent, const char* msg)
{
    FILE* f;
    const char* warning = "Soar cannot recover from this error. \nYou will have to restart Soar to run an agent.\nData is still available for inspection, but may be corrupt.\nIf a log was open, it has been closed for safety.";

    Output_Manager::Get_OM().printa(thisAgent, msg);
    Output_Manager::Get_OM().printa(thisAgent, warning);

    xml_generate_error(thisAgent, msg);
    xml_generate_error(thisAgent, warning);

    f = fopen("soarerror", "w");
    fprintf(f, "%s", msg);
    fprintf(f, "%s", warning);
    fclose(f);

    assert(false);

}

/* -- A version for use when the current agent variable is not available == */

void abort_with_fatal_error_noagent(const char* msg)
{
    FILE* f;
    const char* warning = "Soar cannot recover from this error. \nYou will have to restart Soar to run an agent.\nData is still available for inspection, but may be corrupt.\nIf a log was open, it has been closed for safety.";

    Output_Manager::Get_OM().print(msg);
    Output_Manager::Get_OM().print(warning);

    f = fopen("soarerror", "w");
    fprintf(f, "%s", msg);
    fprintf(f, "%s", warning);
    fclose(f);

    assert(false);
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
//    the_agent->stop_soar = true;
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


void set_sysparam(agent* thisAgent, int param_number, int64_t new_value)
{
    if ((param_number < 0) || (param_number > HIGHEST_SYSPARAM_NUMBER))
    {
        print(thisAgent,  "Internal error: tried to set bad sysparam #: %d\n", param_number);
        return;
    }
    thisAgent->sysparams[param_number] = new_value;

    soar_invoke_callbacks(thisAgent,
                          SYSTEM_PARAMETER_CHANGED_CALLBACK,
                          reinterpret_cast<soar_call_data>(param_number));
}

void init_sysparams(agent* thisAgent)
{
    int i;

    for (i = 0; i < HIGHEST_SYSPARAM_NUMBER + 1; i++)
    {
        thisAgent->sysparams[i] = 0;
    }

    /* --- set all params to zero, except the following: --- */
    thisAgent->sysparams[TRACE_CONTEXT_DECISIONS_SYSPARAM] = true;
    thisAgent->sysparams[TRACE_FIRINGS_OF_CHUNKS_SYSPARAM] = false;
    thisAgent->sysparams[TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM] = NONE_WME_TRACE; /* RPM 6/05 Changed from timetag to none */
    thisAgent->sysparams[TRACE_CHUNK_NAMES_SYSPARAM] = false;
    thisAgent->sysparams[TRACE_JUSTIFICATION_NAMES_SYSPARAM] = false;
    thisAgent->sysparams[TRACE_LOADING_SYSPARAM] = true; /* KJC 8/96 */
    thisAgent->sysparams[MAX_ELABORATIONS_SYSPARAM] = 100;
    thisAgent->sysparams[MAX_CHUNKS_SYSPARAM] = 50;
    thisAgent->sysparams[MAX_NIL_OUTPUT_CYCLES_SYSPARAM] = 15;
    thisAgent->sysparams[MAX_GOAL_DEPTH] = 100;  /* generate an interrupt so users can recover before exceed program stack*/
    thisAgent->sysparams[MAX_MEMORY_USAGE_SYSPARAM] = 100000000; /* default to 100MB.  Event generated when exceeded*/

    thisAgent->sysparams[RESPOND_TO_LOAD_ERRORS_SYSPARAM] = true;

#ifdef ATTENTION_LAPSE
    /* RMJ */
    thisAgent->sysparams[ATTENTION_LAPSE_ON_SYSPARAM] = false;
#endif /* ATTENTION_LAPSE */

    // voigtjr:  turning learning off by default
    thisAgent->sysparams[LEARNING_ON_SYSPARAM] = false;

    thisAgent->sysparams[LEARNING_ONLY_SYSPARAM] = false;  /* AGR MVL1 */
    thisAgent->sysparams[LEARNING_EXCEPT_SYSPARAM] = false;  /* KJC 8/96 */
    thisAgent->sysparams[LEARNING_ALL_GOALS_SYSPARAM] = true;
    thisAgent->sysparams[USER_SELECT_MODE_SYSPARAM] = USER_SELECT_SOFTMAX;
    thisAgent->sysparams[USER_SELECT_REDUCE_SYSPARAM] = false;
    thisAgent->sysparams[PRINT_WARNINGS_SYSPARAM] = true;
    thisAgent->sysparams[PRINT_ALIAS_SYSPARAM] = true;  /* AGR 627 */
    thisAgent->sysparams[EXPLAIN_SYSPARAM] = false; /* KJC 7/96 */
    thisAgent->sysparams[TRACE_OPERAND2_REMOVALS_SYSPARAM] = false;
    thisAgent->sysparams[TIMERS_ENABLED] = true;

    // JRV: Chunk through local negations by default
    thisAgent->sysparams[CHUNK_THROUGH_LOCAL_NEGATIONS_SYSPARAM] = true;

    // MMA: Chunk through evaluation rules off by default
    thisAgent->sysparams[CHUNK_THROUGH_EVALUATION_RULES_SYSPARAM] = false;

    thisAgent->sysparams[DECISION_CYCLE_MAX_USEC_INTERRUPT] = 0;
}

/* ===================================================================

                     Adding and Removing Pwatchs

   Productions_being_traced is a (consed) list of all productions
   on which a pwatch has been set.  Pwatchs are added/removed via
   calls to add_pwatch() and remove_pwatch().
=================================================================== */
/* list of production structures */


void add_pwatch(agent* thisAgent, production* prod)
{
    if (prod->trace_firings)
    {
        return;
    }
    prod->trace_firings = true;
    push(thisAgent, prod, thisAgent->productions_being_traced);
}

bool remove_pwatch_test_fn(agent* /*thisAgent*/, cons* c,
                           void* prod_to_remove_pwatch_of)
{
    return (c->first == static_cast<production*>(prod_to_remove_pwatch_of));
}

void remove_pwatch(agent* thisAgent, production* prod)
{
    if (! prod->trace_firings)
    {
        return;
    }
    prod->trace_firings = false;
    free_list(thisAgent,
              extract_list_elements(thisAgent,
                                    &thisAgent->productions_being_traced,
                                    remove_pwatch_test_fn, prod));
}

/* ===================================================================

                         Reinitializing Soar

   Reset_statistics() resets all the statistics (except the firing counts
   on each individual production).  Reinitialize_soar() does all the
   work for an init-soar.
=================================================================== */

void reset_production_firing_counts(agent* thisAgent)
{
    int t;
    production* p;

    for (t = 0; t < NUM_PRODUCTION_TYPES; t++)
    {
        for (p = thisAgent->all_productions_of_type[t];
                p != NIL;
                p = p->next)
        {
            p->firing_count = 0;
        }
    }
}

void reset_statistics(agent* thisAgent)
{

    thisAgent->d_cycle_count = 0;
    thisAgent->decision_phases_count = 0;
    thisAgent->e_cycle_count = 0;
    thisAgent->e_cycles_this_d_cycle = 0;
    thisAgent->chunks_this_d_cycle = 0;
    thisAgent->production_firing_count = 0;
    thisAgent->start_dc_production_firing_count = 0;
    thisAgent->wme_addition_count = 0;
    thisAgent->wme_removal_count = 0;
    thisAgent->max_wm_size = 0;

    thisAgent->start_dc_wme_addition_count = 0;
    thisAgent->start_dc_wme_removal_count = 0;

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

    reset_timers(thisAgent);
    reset_max_stats(thisAgent);

    thisAgent->wma_timers->reset();
    thisAgent->epmem_timers->reset();
    thisAgent->smem_timers->reset();

    thisAgent->wma_d_cycle_count = 0;
}

void reset_timers(agent* thisAgent)
{
#ifndef NO_TIMING_STUFF
    thisAgent->timers_cpu.reset();
    thisAgent->timers_kernel.reset();
    thisAgent->timers_phase.reset();
#ifdef DETAILED_TIMER_STATS
    thisAgent->timers_gds.set_enabled(&(thisAgent->sysparams[TIMERS_ENABLED]));
    thisAgent->timers_gds.reset();
#endif

    thisAgent->timers_total_cpu_time.reset();
    thisAgent->timers_total_kernel_time.reset();
    thisAgent->timers_input_function_cpu_time.reset();
    thisAgent->timers_output_function_cpu_time.reset();

    for (int i = 0; i < NUM_PHASE_TYPES; i++)
    {
        thisAgent->timers_decision_cycle_phase[i].reset();
        thisAgent->timers_monitors_cpu_time[i].reset();
#ifdef DETAILED_TIMER_STATS
        thisAgent->timers_ownership_cpu_time[i].reset();
        thisAgent->timers_chunking_cpu_time[i].reset();
        thisAgent->timers_match_cpu_time[i].reset();
        thisAgent->timers_gds_cpu_time[i].reset();
#endif
    }

    thisAgent->last_derived_kernel_time_usec = 0;
#endif // NO_TIMING_STUFF
}

void reset_max_stats(agent* thisAgent)
{
    thisAgent->max_dc_production_firing_count_cycle = 0;
    thisAgent->max_dc_production_firing_count_value = 0;
    thisAgent->max_dc_wm_changes_value = 0;
    thisAgent->max_dc_wm_changes_cycle = 0;
#ifndef NO_TIMING_STUFF
    thisAgent->max_dc_time_cycle = 0;
    thisAgent->max_dc_time_usec = 0;

    thisAgent->max_dc_epmem_time_sec = 0;
    thisAgent->total_dc_epmem_time_sec = -1;
    thisAgent->max_dc_epmem_time_cycle = 0;

    thisAgent->max_dc_smem_time_sec = 0;
    thisAgent->total_dc_smem_time_sec = -1;
    thisAgent->max_dc_smem_time_cycle = 0;
#endif // NO_TIMING_STUFF
}

bool reinitialize_soar(agent* thisAgent)
{
    ++thisAgent->init_count;
    ++thisAgent->rl_init_count;

    int64_t cur_TRACE_CONTEXT_DECISIONS_SYSPARAM;
    int64_t cur_TRACE_PHASES_SYSPARAM;
    int64_t cur_TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM;
    int64_t cur_TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM;
    int64_t cur_TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM;
    int64_t cur_TRACE_FIRINGS_PREFERENCES_SYSPARAM;
    int64_t cur_TRACE_WM_CHANGES_SYSPARAM;
    int64_t cur_TRACE_GDS_SYSPARAM;

    thisAgent->did_PE = false;    /* RCHONG:  10.11 */

    soar_invoke_callbacks(thisAgent,
                          BEFORE_INIT_SOAR_CALLBACK,
                          0);

    /* Stash trace state: */
    cur_TRACE_CONTEXT_DECISIONS_SYSPARAM        = thisAgent->sysparams[TRACE_CONTEXT_DECISIONS_SYSPARAM];
    cur_TRACE_PHASES_SYSPARAM                   = thisAgent->sysparams[TRACE_PHASES_SYSPARAM];
    cur_TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM = thisAgent->sysparams[TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM];
    cur_TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM    = thisAgent->sysparams[TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM];
    cur_TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM   = thisAgent->sysparams[TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM];
    cur_TRACE_FIRINGS_PREFERENCES_SYSPARAM      = thisAgent->sysparams[TRACE_FIRINGS_PREFERENCES_SYSPARAM];
    cur_TRACE_WM_CHANGES_SYSPARAM               = thisAgent->sysparams[TRACE_WM_CHANGES_SYSPARAM];
    cur_TRACE_GDS_SYSPARAM                      = thisAgent->sysparams[TRACE_GDS_SYSPARAM];

    /* Temporarily disable tracing: */
    set_sysparam(thisAgent, TRACE_CONTEXT_DECISIONS_SYSPARAM,        false);
    set_sysparam(thisAgent, TRACE_PHASES_SYSPARAM,                   false);
    set_sysparam(thisAgent, TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM, false);
    set_sysparam(thisAgent, TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM,    false);
    set_sysparam(thisAgent, TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM,   NONE_WME_TRACE);
    set_sysparam(thisAgent, TRACE_FIRINGS_PREFERENCES_SYSPARAM,      false);
    set_sysparam(thisAgent, TRACE_WM_CHANGES_SYSPARAM,               false);
    set_sysparam(thisAgent, TRACE_GDS_SYSPARAM,                      false);

    /* Re-init episodic and semantic memory databases */
    epmem_reinit(thisAgent);
    smem_reinit(thisAgent);

    bool wma_was_enabled = wma_enabled(thisAgent);
    thisAgent->wma_params->activation->set_value(off);

    rl_param_container::apoptosis_choices rl_apoptosis = thisAgent->rl_params->apoptosis->get_value();
    thisAgent->rl_params->apoptosis->set_value(rl_param_container::apoptosis_none);

    clear_goal_stack(thisAgent);

    if (wma_was_enabled)
    {
        thisAgent->wma_params->activation->set_value(on);
    }

    thisAgent->rl_params->apoptosis->set_value(rl_apoptosis);

    thisAgent->rl_stats->reset();
    thisAgent->wma_stats->reset();
    thisAgent->epmem_stats->reset();
    thisAgent->smem_stats->reset();
    thisAgent->dyn_counters->clear();
    thisAgent->variablizationManager->reinit();

    thisAgent->active_level = 0; /* Signal that everything should be retracted */
    thisAgent->FIRING_TYPE = IE_PRODS;
    do_preference_phase(thisAgent);    /* allow all i-instantiations to retract */

    reset_explain(thisAgent);
    bool ok = reset_id_counters(thisAgent);
    reset_wme_timetags(thisAgent);
    reset_statistics(thisAgent);

    // JRV: For XML generation
    xml_reset(thisAgent);


    /* RDF 01282003: Reinitializing the various halt and stop flags */
    thisAgent->system_halted = false;
    thisAgent->stop_soar = false;           // voigtjr:  this line doesn't exist in other kernel
    thisAgent->reason_for_stopping = 0;
    thisAgent->substate_break_level = 0;

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
    set_sysparam(thisAgent, TRACE_GDS_SYSPARAM,                      cur_TRACE_GDS_SYSPARAM);
    /* kjh (CUSP-B4) end */

    soar_invoke_callbacks(thisAgent,
                          AFTER_INIT_SOAR_CALLBACK,
                          0);

    thisAgent->input_cycle_flag = true;  /* reinitialize flag  AGR REW1 */
    thisAgent->current_phase = INPUT_PHASE;  /* moved here June 05 from loop below.  KJC */

    /* REW: begin 09.15.96 */
    thisAgent->FIRING_TYPE = IE_PRODS;  /* KJC 10.05.98 was PE */
    thisAgent->did_PE = false;
    /* REW: end 09.15.96 */

    // reset old stats information
    stats_close(thisAgent);
    delete thisAgent->stats_db;
    thisAgent->stats_db = new soar_module::sqlite_database();

    // voigtjr: WARN_IF_TIMERS_REPORT_ZERO block goes here in other kernel
    return ok ;
}

/* ===================================================================

                            Running Soar

   Do_one_top_level_phase() runs Soar one top-level phase.  Note that
   this does not start/stop the total_cpu_time timer--the caller must
   do this.

   Each of the following routines runs Soar for a certain duration,
   or until stop_soar gets set to true.
     - Run_forever() runs Soar forever.
     - Run_for_n_phases() runs Soar for a given number (n) of top-level
       phases.  (If n==-1, it runs forever.)
     - Run_for_n_elaboration_cycles() runs Soar for a given number (n)
       of elaboration cycles.  (Here, decision phase is counted as
       an elaboration cycle.)  (If n==-1, it runs forever.)
     - Run_for_n_decision_cycles() runs Soar for a given number (n) of
       decision cycles.  (If n==-1, it runs forever.)
     - Run_for_n_selections_of_slot (int64_t n, Symbol *attr_of_slot): this
       runs Soar until the nth time a selection is made for a given
       type of slot.  Attr_of_slot should be either state_symbol or
       operator_symbol.
     - Run_for_n_selections_of_slot_at_level (int64_t n, Symbol *attr_of_slot,
       goal_stack_level level):  this runs Soar for n selections of the
       given slot at the given level, or until the goal stack is popped
       so that level no longer exists.
=================================================================== */

void do_one_top_level_phase(agent* thisAgent)
{
    //  Symbol *iterate_goal_sym;  kjc commented /* RCHONG: end 10.11 */

    if (thisAgent->system_halted)
    {
        print(thisAgent,
              "\nSystem halted.  Use (init-soar) before running Soar again.");
        xml_generate_error(thisAgent, "System halted.  Use (init-soar) before running Soar again.");
        thisAgent->stop_soar = true;
        thisAgent->reason_for_stopping = "System halted.";
        return;
    }

    smem_attach(thisAgent);

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
     *    thisAgent->d_cycle_count++;
     * }
     */

    switch (thisAgent->current_phase)
    {

        case INPUT_PHASE:

            if (thisAgent->sysparams[TRACE_PHASES_SYSPARAM])
            {
                print_phase(thisAgent, "\n--- Input Phase --- \n", 0);
            }

            /* for Operand2 mode using the new decision cycle ordering,
             * we need to do some initialization in the INPUT PHASE, which
             * now comes first.  e_cycles are also zeroed before the APPLY Phase.
             */
            thisAgent->chunks_this_d_cycle = 0;
            thisAgent->e_cycles_this_d_cycle = 0;
#ifndef NO_TIMING_STUFF   /* REW:  28.07.96 */
            thisAgent->timers_phase.start();
#endif

            /* we check e_cycle_count because Soar 7 runs multiple input cycles per decision */
            /* always true for Soar 8 */
            if (thisAgent->e_cycles_this_d_cycle == 0)
            {
                soar_invoke_callbacks(thisAgent,
                                      BEFORE_DECISION_CYCLE_CALLBACK,
                                      reinterpret_cast<soar_call_data>(INPUT_PHASE));
            }  /* end if e_cycles_this_d_cycle == 0 */

#ifdef REAL_TIME_BEHAVIOR  /* RM Jones */
            test_for_input_delay(thisAgent);
#endif
#ifdef ATTENTION_LAPSE  /* RM Jones */
            determine_lapsing(thisAgent);
#endif

            if (thisAgent->input_cycle_flag == true)   /* Soar 7 flag, but always true for Soar8 */
            {
                soar_invoke_callbacks(thisAgent,
                                      BEFORE_INPUT_PHASE_CALLBACK,
                                      reinterpret_cast<soar_call_data>(INPUT_PHASE));

#ifndef NO_SVS
                if (thisAgent->svs->is_enabled())
                {
                    thisAgent->svs->input_callback();
                }
#endif
                do_input_cycle(thisAgent);

                thisAgent->run_phase_count++ ;
                thisAgent->run_elaboration_count++ ;  // All phases count as a run elaboration
                soar_invoke_callbacks(thisAgent,
                                      AFTER_INPUT_PHASE_CALLBACK,
                                      reinterpret_cast<soar_call_data>(INPUT_PHASE));

                if (thisAgent->input_period)
                {
                    thisAgent->input_cycle_flag = false;
                }
            }  /* END if (input_cycle_flag==true) AGR REW1 this line and 1 previous line */

            if (thisAgent->sysparams[TRACE_PHASES_SYSPARAM])
            {
                print_phase(thisAgent, "\n--- END Input Phase --- \n", 1);
            }

#ifndef NO_TIMING_STUFF  /* REW:  28.07.96 */
            thisAgent->timers_phase.stop();
            thisAgent->timers_decision_cycle_phase[INPUT_PHASE].update(thisAgent->timers_phase);
#endif

            thisAgent->current_phase = PROPOSE_PHASE;

            break;  /* END of INPUT PHASE */

        /////////////////////////////////////////////////////////////////////////////////

        case PROPOSE_PHASE:   /* added in 8.6 to clarify Soar8 decision cycle */

#ifndef NO_TIMING_STUFF
            thisAgent->timers_phase.start();
#endif

            /* e_cycles_this_d_cycle will always be zero UNLESS we are
             * running by ELABORATIONS.
             * We only want to do the following if we've just finished INPUT and are
             * starting PROPOSE.  If this is the second elaboration for PROPOSE, then
             * just do the while loop below.   KJC  June 05
             */
            if (thisAgent->e_cycles_this_d_cycle < 1)
            {
                if (thisAgent->sysparams[TRACE_PHASES_SYSPARAM])
                {
                    print_phase(thisAgent, "\n--- Proposal Phase ---\n", 0);
                }

                soar_invoke_callbacks(thisAgent,
                                      BEFORE_PROPOSE_PHASE_CALLBACK,
                                      reinterpret_cast<soar_call_data>(PROPOSE_PHASE));

                // We need to generate this event here in case no elaborations fire...
                // FIXME return the correct enum top_level_phase constant in soar_call_data?
                /*(soar_call_data)((thisAgent->applyPhase == true)? gSKI_K_APPLY_PHASE: gSKI_K_PROPOSAL_PHASE)*/
                soar_invoke_callbacks(thisAgent, BEFORE_ELABORATION_CALLBACK, NULL) ;

                /* 'Prime the decision for a new round of production firings at the end of
                * REW:   05.05.97   */  /*  KJC 04.05 moved here from INPUT_PHASE for 8.6.0 */
                initialize_consistency_calculations_for_new_decision(thisAgent);

                thisAgent->FIRING_TYPE = IE_PRODS;
                thisAgent->applyPhase = false;   /* KJC 04/05: do we still need this line?  gSKI does*/
                determine_highest_active_production_level_in_stack_propose(thisAgent);

                if (thisAgent->current_phase == DECISION_PHASE)
                {
                    // no elaborations will fire this phase
                    thisAgent->run_elaboration_count++ ;    // All phases count as a run elaboration
                    // FIXME return the correct enum top_level_phase constant in soar_call_data?
                    /*(soar_call_data)((thisAgent->applyPhase == true)? gSKI_K_APPLY_PHASE: gSKI_K_PROPOSAL_PHASE)*/
                    soar_invoke_callbacks(thisAgent, AFTER_ELABORATION_CALLBACK, NULL) ;
                }
            }

            /* max-elaborations are checked in determine_highest_active... and if they
            * are reached, the current phase is set to DECISION.  phase is also set
            * to DECISION when PROPOSE is done.
            */

            while (thisAgent->current_phase != DECISION_PHASE)
            {
                if (thisAgent->e_cycles_this_d_cycle)
                {
                    // only for 2nd cycle or higher.  1st cycle fired above
                    // FIXME return the correct enum top_level_phase constant in soar_call_data? /*(soar_call_data)((thisAgent->applyPhase == true)? gSKI_K_APPLY_PHASE: gSKI_K_PROPOSAL_PHASE)*/
                    soar_invoke_callbacks(thisAgent, BEFORE_ELABORATION_CALLBACK, NULL) ;
                }

                do_preference_phase(thisAgent);
                do_working_memory_phase(thisAgent);

                if (smem_enabled(thisAgent))
                {
                    smem_go(thisAgent, true);
                }

                // allow epmem searches in proposal phase
                // FIXME should turn this into a command line option
                if (epmem_enabled(thisAgent))
                {
                    // epmem_go( thisAgent, false );
                }

                /* Update accounting.  Moved here by KJC 04/05/05 */
                thisAgent->e_cycle_count++;
                thisAgent->e_cycles_this_d_cycle++;
                thisAgent->run_elaboration_count++ ;
                determine_highest_active_production_level_in_stack_propose(thisAgent);
                // FIXME return the correct enum top_level_phase constant in soar_call_data? /*(soar_call_data)((thisAgent->applyPhase == true)? gSKI_K_APPLY_PHASE: gSKI_K_PROPOSAL_PHASE)*/
                soar_invoke_callbacks(thisAgent, AFTER_ELABORATION_CALLBACK, NULL) ;

                if (thisAgent->system_halted)
                {
                    break;
                }
                if (thisAgent->go_type == GO_ELABORATION)
                {
                    break;
                }
            }

            /*  If we've finished PROPOSE, then current_phase will be equal to DECISION
             *  otherwise, we're only stopping because we're running by ELABORATIONS, so
             *  don't do the end-of-phase updating in that case.
             */
            if (thisAgent->current_phase == DECISION_PHASE)
            {
                /* This is a HACK for Soar 8.6.0 beta release... KCoulter April 05
                * We got here, because we should move to DECISION, so PROPOSE is done
                * Set phase back to PROPOSE, do print_phase, callbacks, and then
                * reset phase to DECISION
                */
                thisAgent->current_phase = PROPOSE_PHASE;
                if (thisAgent->sysparams[TRACE_PHASES_SYSPARAM])
                {
                    print_phase(thisAgent, "\n--- END Proposal Phase ---\n", 1);
                }

                thisAgent->run_phase_count++ ;
                soar_invoke_callbacks(thisAgent,
                                      AFTER_PROPOSE_PHASE_CALLBACK,
                                      reinterpret_cast<soar_call_data>(PROPOSE_PHASE));
                thisAgent->current_phase = DECISION_PHASE;
            }

#ifndef NO_TIMING_STUFF
            thisAgent->timers_phase.stop();
            thisAgent->timers_decision_cycle_phase[PROPOSE_PHASE].update(thisAgent->timers_phase);
#endif

            break;  /* END of Soar8 PROPOSE PHASE */

        /////////////////////////////////////////////////////////////////////////////////
        case PREFERENCE_PHASE:
            /* starting with 8.6.0, PREFERENCE_PHASE is only Soar 7 mode -- applyPhase not valid here */
            /* needs to be updated for gSKI interface, and gSKI needs to accommodate Soar 7 */

            /* JC ADDED: Tell gski about elaboration phase beginning */
            // FIXME return the correct enum top_level_phase constant in soar_call_data? /*(soar_call_data)((thisAgent->applyPhase == true)? gSKI_K_APPLY_PHASE: gSKI_K_PROPOSAL_PHASE)*/
            soar_invoke_callbacks(thisAgent, BEFORE_ELABORATION_CALLBACK, NULL) ;

#ifndef NO_TIMING_STUFF       /* REW: 28.07.96 */
            thisAgent->timers_phase.start();
#endif

            soar_invoke_callbacks(thisAgent,
                                  BEFORE_PREFERENCE_PHASE_CALLBACK,
                                  reinterpret_cast<soar_call_data>(PREFERENCE_PHASE));

            do_preference_phase(thisAgent);

            thisAgent->run_phase_count++ ;
            thisAgent->run_elaboration_count++ ;  // All phases count as a run elaboration
            soar_invoke_callbacks(thisAgent,
                                  AFTER_PREFERENCE_PHASE_CALLBACK,
                                  reinterpret_cast<soar_call_data>(PREFERENCE_PHASE));
            thisAgent->current_phase = WM_PHASE;

#ifndef NO_TIMING_STUFF       /* REW:  28.07.96 */
            thisAgent->timers_phase.stop();
            thisAgent->timers_decision_cycle_phase[PREFERENCE_PHASE].update(thisAgent->timers_phase);
#endif

            /* tell gSKI PREF_PHASE ending...
            */
            break;      /* END of Soar7 PREFERENCE PHASE */

        /////////////////////////////////////////////////////////////////////////////////
        case WM_PHASE:
            /* starting with 8.6.0, WM_PHASE is only Soar 7 mode; see PROPOSE and APPLY */
            /* needs to be updated for gSKI interface, and gSKI needs to accommodate Soar 7 */

            /*  we need to tell gSKI WM Phase beginning... */

#ifndef NO_TIMING_STUFF     /* REW: begin 28.07.96 */
            thisAgent->timers_phase.start();
#endif

            soar_invoke_callbacks(thisAgent,
                                  BEFORE_WM_PHASE_CALLBACK,
                                  reinterpret_cast<soar_call_data>(WM_PHASE));

            do_working_memory_phase(thisAgent);

            thisAgent->run_phase_count++ ;
            thisAgent->run_elaboration_count++ ;  // All phases count as a run elaboration
            soar_invoke_callbacks(thisAgent,
                                  AFTER_WM_PHASE_CALLBACK,
                                  reinterpret_cast<soar_call_data>(WM_PHASE));

            thisAgent->current_phase = OUTPUT_PHASE;

#ifndef NO_TIMING_STUFF      /* REW:  28.07.96 */
            thisAgent->timers_phase.stop();
            thisAgent->timers_decision_cycle_phase[WM_PHASE].update(thisAgent->timers_phase);
#endif

            // FIXME return the correct enum top_level_phase constant in soar_call_data? /*(soar_call_data)((thisAgent->applyPhase == true)? gSKI_K_APPLY_PHASE: gSKI_K_PROPOSAL_PHASE)*/
            soar_invoke_callbacks(thisAgent, AFTER_ELABORATION_CALLBACK, NULL) ;

            break;     /* END of Soar7 WM PHASE */

        /////////////////////////////////////////////////////////////////////////////////
        case APPLY_PHASE:   /* added in 8.6 to clarify Soar8 decision cycle */

#ifndef NO_TIMING_STUFF
            thisAgent->timers_phase.start();
#endif

            /* e_cycle_count will always be zero UNLESS we are running by ELABORATIONS.
             * We only want to do the following if we've just finished DECISION and are
             * starting APPLY.  If this is the second elaboration for APPLY, then
             * just do the while loop below.   KJC  June 05
             */
            if (thisAgent->e_cycles_this_d_cycle < 1)
            {

                if (thisAgent->sysparams[TRACE_PHASES_SYSPARAM])
                {
                    print_phase(thisAgent, "\n--- Application Phase ---\n", 0);
                }

                soar_invoke_callbacks(thisAgent,
                                      BEFORE_APPLY_PHASE_CALLBACK,
                                      reinterpret_cast<soar_call_data>(APPLY_PHASE));

                // We need to generate this event here in case no elaborations fire...
                // FIXME return the correct enum top_level_phase constant in soar_call_data? /*(soar_call_data)((thisAgent->applyPhase == true)? gSKI_K_APPLY_PHASE: gSKI_K_PROPOSAL_PHASE)*/
                soar_invoke_callbacks(thisAgent, BEFORE_ELABORATION_CALLBACK, NULL) ;

                /* 'prime' the cycle for a new round of production firings
                * in the APPLY (pref/wm) phase *//* KJC 04.05 moved here from end of DECISION */
                initialize_consistency_calculations_for_new_decision(thisAgent);

                thisAgent->FIRING_TYPE = PE_PRODS;  /* might get reset in det_high_active_prod_level... */
                thisAgent->applyPhase = true;       /* KJC 04/05: do we still need this line?  gSKI does*/
                determine_highest_active_production_level_in_stack_apply(thisAgent);
                if (thisAgent->current_phase == OUTPUT_PHASE)
                {
                    // no elaborations will fire this phase
                    thisAgent->run_elaboration_count++ ;    // All phases count as a run elaboration
                    // FIXME return the correct enum top_level_phase constant in soar_call_data? /*(soar_call_data)((thisAgent->applyPhase == true)? gSKI_K_APPLY_PHASE: gSKI_K_PROPOSAL_PHASE)*/
                    soar_invoke_callbacks(thisAgent, AFTER_ELABORATION_CALLBACK, NULL) ;
                }
            }
            /* max-elaborations are checked in determine_highest_active... and if they
             * are reached, the current phase is set to OUTPUT.  phase is also set
             * to OUTPUT when APPLY is done.
             */

            while (thisAgent->current_phase != OUTPUT_PHASE)
            {
                /* JC ADDED: Tell gski about elaboration phase beginning */
                if (thisAgent->e_cycles_this_d_cycle)
                {
                    // only for 2nd cycle or higher.  1st cycle fired above
                    // FIXME return the correct enum top_level_phase constant in soar_call_data? /*(soar_call_data)((thisAgent->applyPhase == true)? gSKI_K_APPLY_PHASE: gSKI_K_PROPOSAL_PHASE)*/
                    soar_invoke_callbacks(thisAgent, BEFORE_ELABORATION_CALLBACK, NULL) ;
                }

                do_preference_phase(thisAgent);
                do_working_memory_phase(thisAgent);

                if (smem_enabled(thisAgent))
                {
                    smem_go(thisAgent, true);
                }

                /* Update accounting.  Moved here by KJC 04/05/05 */
                thisAgent->e_cycle_count++;
                thisAgent->e_cycles_this_d_cycle++;
                thisAgent->run_elaboration_count++ ;

                if (thisAgent->FIRING_TYPE == PE_PRODS)
                {
                    thisAgent->pe_cycle_count++;
                    thisAgent->pe_cycles_this_d_cycle++;
                }
                determine_highest_active_production_level_in_stack_apply(thisAgent);
                // FIXME return the correct enum top_level_phase constant in soar_call_data? /*(soar_call_data)((thisAgent->applyPhase == true)? gSKI_K_APPLY_PHASE: gSKI_K_PROPOSAL_PHASE)*/
                soar_invoke_callbacks(thisAgent, AFTER_ELABORATION_CALLBACK, NULL) ;

                if (thisAgent->system_halted)
                {
                    break;
                }
                if (thisAgent->go_type == GO_ELABORATION)
                {
                    break;
                }
            }

            /*  If we've finished APPLY, then current_phase will be equal to OUTPUT
             *  otherwise, we're only stopping because we're running by ELABORATIONS, so
             *  don't do the end-of-phase updating in that case.
             */
            if (thisAgent->current_phase == OUTPUT_PHASE)
            {
                /* This is a HACK for Soar 8.6.0 beta release... KCoulter April 05
                * We got here, because we should move to OUTPUT, so APPLY is done
                * Set phase back to APPLY, do print_phase, callbacks and reset phase to OUTPUT
                */
                thisAgent->current_phase = APPLY_PHASE;
                if (thisAgent->sysparams[TRACE_PHASES_SYSPARAM])
                {
                    print_phase(thisAgent, "\n--- END Application Phase ---\n", 1);
                }
                thisAgent->run_phase_count++ ;
                soar_invoke_callbacks(thisAgent,
                                      AFTER_APPLY_PHASE_CALLBACK,
                                      reinterpret_cast<soar_call_data>(APPLY_PHASE));

                thisAgent->current_phase = OUTPUT_PHASE;
            }

#ifndef NO_TIMING_STUFF
            thisAgent->timers_phase.stop();
            thisAgent->timers_decision_cycle_phase[APPLY_PHASE].update(thisAgent->timers_phase);
#endif

            break;  /* END of Soar8 APPLY PHASE */

        /////////////////////////////////////////////////////////////////////////////////
        case OUTPUT_PHASE:

            if (thisAgent->sysparams[TRACE_PHASES_SYSPARAM])
            {
                print_phase(thisAgent, "\n--- Output Phase ---\n", 0);
            }

#ifndef NO_TIMING_STUFF      /* REW:  28.07.96 */
            thisAgent->timers_phase.start();
#endif

            soar_invoke_callbacks(thisAgent,
                                  BEFORE_OUTPUT_PHASE_CALLBACK,
                                  reinterpret_cast<soar_call_data>(OUTPUT_PHASE));

#ifndef NO_SVS
            if (thisAgent->svs->is_enabled())
            {
                thisAgent->svs->output_callback();
            }
#endif
            /** KJC June 05:  moved output function timers into do_output_cycle ***/

            do_output_cycle(thisAgent);

            if (smem_enabled(thisAgent))
            {
                smem_go(thisAgent, false);
            }

            ///////////////////////////////////////////////////////////////////
            assert(thisAgent->wma_d_cycle_count == thisAgent->d_cycle_count);
            ///////////////////////////////////////////////////////////////////

            // update histories only first, allows:
            // - epmem retrieval cues to be biased by activation
            // - epmem encoding to capture wmes that may be forgotten shortly
            if (wma_enabled(thisAgent))
            {
                wma_go(thisAgent, wma_histories);
            }

            if (epmem_enabled(thisAgent) && (thisAgent->epmem_params->phase->get_value() == epmem_param_container::phase_output))
            {
                // since we consolidated wma histories from this decision,
                // we need to pretend it's the next time step in case
                // an epmem retrieval wants to know current activation value
                thisAgent->wma_d_cycle_count++;
                {
                    epmem_go(thisAgent);
                }
                thisAgent->wma_d_cycle_count--;
            }

            // now both update histories and forget, allows
            // - epmem retrieval to affect history
            // - epmem encoding to capture wmes that may be forgotten shortly
            if (wma_enabled(thisAgent))
            {
                wma_go(thisAgent, wma_histories);
                wma_go(thisAgent, wma_forgetting);
            }

            ///////////////////////////////////////////////////////////////////
            assert(thisAgent->wma_d_cycle_count == thisAgent->d_cycle_count);
            ///////////////////////////////////////////////////////////////////

            // RL apoptosis
            {
                rl_param_container::apoptosis_choices rl_apoptosis = thisAgent->rl_params->apoptosis->get_value();
                if (rl_apoptosis != rl_param_container::apoptosis_none)
                {
                    thisAgent->rl_prods->process_buffered_references();
                    thisAgent->rl_prods->forget();
                    thisAgent->rl_prods->time_forward();

                    for (rl_production_memory::object_set::iterator p = thisAgent->rl_prods->forgotten_begin(); p != thisAgent->rl_prods->forgotten_end(); p++)
                    {
                        // conditions:
                        // - no matched instantiations AND
                        // - if RL...
                        //   - no update count
                        //   - not in some state's prev_op_rl_rules list
                        if (((*p)->instantiations == NIL) && (!(*p)->rl_rule || ((static_cast<int64_t>((*p)->rl_update_count) == 0) && ((*p)->rl_ref_count == 0))))
                        {
                            excise_production(thisAgent, const_cast< production* >(*p), false);
                        }
                    }
                }
            }

            // Count the outputs the agent generates (or times reaching max-nil-outputs without sending output)
            if (thisAgent->output_link_changed || ((++(thisAgent->run_last_output_count)) >= static_cast<uint64_t>(thisAgent->sysparams[MAX_NIL_OUTPUT_CYCLES_SYSPARAM])))
            {
                thisAgent->run_last_output_count = 0 ;
                thisAgent->run_generated_output_count++ ;
            }

            thisAgent->run_phase_count++ ;
            thisAgent->run_elaboration_count++ ;  // All phases count as a run elaboration
            soar_invoke_callbacks(thisAgent,
                                  AFTER_OUTPUT_PHASE_CALLBACK,
                                  reinterpret_cast<soar_call_data>(OUTPUT_PHASE));

            /* REW: begin 09.15.96 */
            // JRV: Get rid of the cached XML after every decision but before the after-decision-phase callback
            xml_invoke_callback(thisAgent);   // invokes XML_GENERATION_CALLBACK, clears XML state

            /* KJC June 05:  moved here from DECISION Phase */
            soar_invoke_callbacks(thisAgent,
                                  AFTER_DECISION_CYCLE_CALLBACK,
                                  reinterpret_cast<soar_call_data>(OUTPUT_PHASE));
#ifndef NO_TIMING_STUFF    /* timers stopped KJC 10-04-98 */
            thisAgent->timers_phase.stop();
            thisAgent->timers_decision_cycle_phase[OUTPUT_PHASE].update(thisAgent->timers_phase);
#endif

            // Update per-cycle statistics
            {
                uint64_t dc_time_usec = 0;
#ifndef NO_TIMING_STUFF
                uint64_t derived_kernel_time_usec = get_derived_kernel_time_usec(thisAgent);
                dc_time_usec = derived_kernel_time_usec - thisAgent->last_derived_kernel_time_usec;
                if (thisAgent->max_dc_time_usec < dc_time_usec)
                {
                    thisAgent->max_dc_time_usec = dc_time_usec;
                    thisAgent->max_dc_time_cycle = thisAgent->d_cycle_count;
                }
                if (thisAgent->sysparams[DECISION_CYCLE_MAX_USEC_INTERRUPT] > 0)
                {
                    if (dc_time_usec >= static_cast<uint64_t>(thisAgent->sysparams[DECISION_CYCLE_MAX_USEC_INTERRUPT]))
                    {
                        thisAgent->stop_soar = true;
                        thisAgent->reason_for_stopping = "decision cycle time greater than interrupt threshold";
                    }
                }
                thisAgent->last_derived_kernel_time_usec = derived_kernel_time_usec;

                double total_epmem_time = thisAgent->epmem_timers->total->value();
                if (thisAgent->total_dc_epmem_time_sec >= 0)
                {
                    double delta_epmem_time = total_epmem_time - thisAgent->total_dc_epmem_time_sec;
                    if (thisAgent->max_dc_epmem_time_sec < delta_epmem_time)
                    {
                        thisAgent->max_dc_epmem_time_sec = delta_epmem_time;
                        thisAgent->max_dc_epmem_time_cycle = thisAgent->d_cycle_count;
                    }
                }
                thisAgent->total_dc_epmem_time_sec = total_epmem_time;

                double total_smem_time = thisAgent->smem_timers->total->value();
                if (thisAgent->total_dc_smem_time_sec >= 0)
                {
                    double delta_smem_time = total_smem_time - thisAgent->total_dc_smem_time_sec;
                    if (thisAgent->max_dc_smem_time_sec < delta_smem_time)
                    {
                        thisAgent->max_dc_smem_time_sec = delta_smem_time;
                        thisAgent->max_dc_smem_time_cycle = thisAgent->d_cycle_count;
                    }
                }
                thisAgent->total_dc_smem_time_sec = total_smem_time;

#endif // NO_TIMING_STUFF

                uint64_t dc_wm_changes = thisAgent->wme_addition_count - thisAgent->start_dc_wme_addition_count;
                dc_wm_changes += thisAgent->wme_removal_count - thisAgent->start_dc_wme_removal_count;
                if (thisAgent->max_dc_wm_changes_value < dc_wm_changes)
                {
                    thisAgent->max_dc_wm_changes_value = dc_wm_changes;
                    thisAgent->max_dc_wm_changes_cycle = thisAgent->d_cycle_count;
                }
                thisAgent->start_dc_wme_addition_count = thisAgent->wme_addition_count;
                thisAgent->start_dc_wme_removal_count = thisAgent->wme_removal_count;

                uint64_t dc_firing_counts = thisAgent->production_firing_count - thisAgent->start_dc_production_firing_count;
                if (thisAgent->max_dc_production_firing_count_value < dc_firing_counts)
                {
                    thisAgent->max_dc_production_firing_count_value = dc_firing_counts;
                    thisAgent->max_dc_production_firing_count_cycle = thisAgent->d_cycle_count;
                }
                thisAgent->start_dc_production_firing_count = thisAgent->production_firing_count;

                // Commit per-cycle stats to db
                if (thisAgent->dc_stat_tracking)
                {
                    stats_db_store(thisAgent, dc_time_usec, dc_wm_changes, dc_firing_counts);
                }
            }

            if (thisAgent->sysparams[TRACE_PHASES_SYSPARAM])
            {
                print_phase(thisAgent, "\n--- END Output Phase ---\n", 1);
            }
            thisAgent->current_phase = INPUT_PHASE;
            thisAgent->d_cycle_count++;
            thisAgent->wma_d_cycle_count++;
            /* REW: end 09.15.96 */
            break;

        /////////////////////////////////////////////////////////////////////////////////
        case DECISION_PHASE:
            /* not yet cleaned up for 8.6.0 release */

            if (thisAgent->sysparams[TRACE_PHASES_SYSPARAM])
            {
                print_phase(thisAgent, "\n--- Decision Phase ---\n", 0);
            }

#ifndef NO_TIMING_STUFF   /* REW:  28.07.96 */
            thisAgent->timers_phase.start();
#endif

            thisAgent->decision_phases_count++;  /* counts decisions, not cycles, for more accurate stats */

            /* AGR REW1 begin */
            if (!thisAgent->input_period)
            {
                thisAgent->input_cycle_flag = true;
            }
            else if ((thisAgent->d_cycle_count % thisAgent->input_period) == 0)
            {
                thisAgent->input_cycle_flag = true;
            }
            /* AGR REW1 end */

            soar_invoke_callbacks(thisAgent,
                                  BEFORE_DECISION_PHASE_CALLBACK,
                                  reinterpret_cast<soar_call_data>(DECISION_PHASE));

            do_decision_phase(thisAgent);

            thisAgent->run_phase_count++ ;
            thisAgent->run_elaboration_count++ ;  // All phases count as a run elaboration

            soar_invoke_callbacks(thisAgent,
                                  AFTER_DECISION_PHASE_CALLBACK,
                                  reinterpret_cast<soar_call_data>(DECISION_PHASE));

            if (thisAgent->sysparams[TRACE_CONTEXT_DECISIONS_SYSPARAM])
            {
                print_string(thisAgent, "\n");
                print_lowest_slot_in_context_stack(thisAgent);
            }

            /* reset elaboration counter */
            thisAgent->e_cycles_this_d_cycle = 0;
            thisAgent->pe_cycles_this_d_cycle = 0;

            if (epmem_enabled(thisAgent) && (thisAgent->epmem_params->phase->get_value() == epmem_param_container::phase_selection))
            {
                epmem_go(thisAgent);
            }

            {
                if (thisAgent->sysparams[TRACE_PHASES_SYSPARAM])
                {
                    print_phase(thisAgent, "\n--- END Decision Phase ---\n", 1);
                }

                /* printf("\nSetting next phase to APPLY following a decision...."); */
                thisAgent->applyPhase = true;
                thisAgent->FIRING_TYPE = PE_PRODS;
                thisAgent->current_phase = APPLY_PHASE;
            }

            /* REW: begin 28.07.96 */
#ifndef NO_TIMING_STUFF
            thisAgent->timers_phase.stop();
            thisAgent->timers_decision_cycle_phase[DECISION_PHASE].update(thisAgent->timers_phase);
#endif
            /* REW: end 28.07.96 */

            break;  /* end DECISION phase */

        /////////////////////////////////////////////////////////////////////////////////

        default: // 2/24/05: added default case to quell gcc compile warning
            assert(false && "Invalid phase enumeration value!");
            break;

    }  /* end switch stmt for current_phase */

    /* --- update WM size statistics --- */
    if (thisAgent->num_wmes_in_rete > thisAgent->max_wm_size)
    {
        thisAgent->max_wm_size = thisAgent->num_wmes_in_rete;
    }
    thisAgent->cumulative_wm_size += thisAgent->num_wmes_in_rete;
    thisAgent->num_wm_sizes_accumulated++;

    if (thisAgent->system_halted)
    {
        thisAgent->stop_soar = true;
        thisAgent->reason_for_stopping = "System halted.";
        soar_invoke_callbacks(thisAgent,
                              AFTER_HALT_SOAR_CALLBACK,
                              reinterpret_cast<soar_call_data>(thisAgent->current_phase));

        // To model episodic task, after halt, perform RL update with next-state value 0
        if (rl_enabled(thisAgent))
        {
            for (Symbol* g = thisAgent->bottom_goal; g; g = g->id->higher_goal)
            {
                rl_tabulate_reward_value_for_goal(thisAgent, g);
                rl_perform_update(thisAgent, 0, true, g);
            }
        }
    }

    if (thisAgent->stop_soar)
    {
        if (thisAgent->reason_for_stopping)
        {
            print(thisAgent,  "\n%s\n", thisAgent->reason_for_stopping);
        }
    }
}

void run_forever(agent* thisAgent)
{
#ifndef NO_TIMING_STUFF
    thisAgent->timers_cpu.start();
    thisAgent->timers_kernel.start();
#endif

    thisAgent->stop_soar = false;
    thisAgent->reason_for_stopping = 0;
    while (! thisAgent->stop_soar)
    {
        do_one_top_level_phase(thisAgent);
    }

#ifndef NO_TIMING_STUFF
    thisAgent->timers_kernel.stop();
    thisAgent->timers_cpu.stop();
    thisAgent->timers_total_kernel_time.update(thisAgent->timers_kernel);
    thisAgent->timers_total_cpu_time.update(thisAgent->timers_cpu);
#endif
}

void run_for_n_phases(agent* thisAgent, int64_t n)
{
    if (n == -1)
    {
        run_forever(thisAgent);
        return;
    }
    if (n < -1)
    {
        return;
    }
#ifndef NO_TIMING_STUFF
    thisAgent->timers_cpu.start();
    thisAgent->timers_kernel.start();
#endif
    thisAgent->stop_soar = false;
    thisAgent->reason_for_stopping = "";
    while (!thisAgent->stop_soar && n)
    {
        do_one_top_level_phase(thisAgent);
        n--;
    }
#ifndef NO_TIMING_STUFF
    thisAgent->timers_kernel.stop();
    thisAgent->timers_cpu.stop();
    thisAgent->timers_total_kernel_time.update(thisAgent->timers_kernel);
    thisAgent->timers_total_cpu_time.update(thisAgent->timers_cpu);
#endif
}

void run_for_n_elaboration_cycles(agent* thisAgent, int64_t n)
{
    int64_t e_cycles_at_start, d_cycles_at_start, elapsed_cycles = 0;
    go_type_enum save_go_type = GO_PHASE;

    if (n == -1)
    {
        run_forever(thisAgent);
        return;
    }
    if (n < -1)
    {
        return;
    }
#ifndef NO_TIMING_STUFF
    thisAgent->timers_cpu.start();
    thisAgent->timers_kernel.start();
#endif
    thisAgent->stop_soar = false;
    thisAgent->reason_for_stopping = 0;
    e_cycles_at_start = thisAgent->e_cycle_count;
    d_cycles_at_start = thisAgent->d_cycle_count;
    elapsed_cycles = -1;
    save_go_type = thisAgent->go_type;
    thisAgent->go_type = GO_ELABORATION;
    /* need next line or runs only the input phase for "d 1" after init-soar */
    if (d_cycles_at_start == 0)
    {
        d_cycles_at_start++;
    }
    while (!thisAgent->stop_soar)
    {
        elapsed_cycles++;
        if (n == elapsed_cycles)
        {
            break;
        }
        do_one_top_level_phase(thisAgent);
    }
    thisAgent->go_type = save_go_type;

#ifndef NO_TIMING_STUFF
    thisAgent->timers_kernel.stop();
    thisAgent->timers_cpu.stop();
    thisAgent->timers_total_kernel_time.update(thisAgent->timers_kernel);
    thisAgent->timers_total_cpu_time.update(thisAgent->timers_cpu);
#endif
}

void run_for_n_modifications_of_output(agent* thisAgent, int64_t n)
{
    bool was_output_phase;
    int64_t count = 0;

    if (n == -1)
    {
        run_forever(thisAgent);
        return;
    }
    if (n < -1)
    {
        return;
    }
#ifndef NO_TIMING_STUFF
    thisAgent->timers_cpu.start();
    thisAgent->timers_kernel.start();
#endif
    thisAgent->stop_soar = false;
    thisAgent->reason_for_stopping = 0;
    while (!thisAgent->stop_soar && n)
    {
        was_output_phase = (thisAgent->current_phase == OUTPUT_PHASE);
        do_one_top_level_phase(thisAgent);
        if (was_output_phase)
        {
            if (thisAgent->output_link_changed)
            {
                n--;
            }
            else
            {
                count++;
            }
        }
        if (count >= thisAgent->sysparams[MAX_NIL_OUTPUT_CYCLES_SYSPARAM])
        {
            break;
            //thisAgent->stop_soar = true;
            //thisAgent->reason_for_stopping = "exceeded max_nil_output_cycles with no output";
        }
    }
#ifndef NO_TIMING_STUFF
    thisAgent->timers_kernel.stop();
    thisAgent->timers_cpu.stop();
    thisAgent->timers_total_kernel_time.update(thisAgent->timers_kernel);
    thisAgent->timers_total_cpu_time.update(thisAgent->timers_cpu);
#endif
}

void run_for_n_decision_cycles(agent* thisAgent, int64_t n)
{
    int64_t d_cycles_at_start;

    if (n == -1)
    {
        run_forever(thisAgent);
        return;
    }
    if (n < -1)
    {
        return;
    }
#ifndef NO_TIMING_STUFF
    thisAgent->timers_cpu.start();
    thisAgent->timers_kernel.start();
#endif
    thisAgent->stop_soar = false;
    thisAgent->reason_for_stopping = 0;
    d_cycles_at_start = thisAgent->d_cycle_count;
    /* need next line or runs only the input phase for "d 1" after init-soar */
    if (d_cycles_at_start == 0)
    {
        d_cycles_at_start++;
    }
    while (!thisAgent->stop_soar)
    {
        if (n == static_cast<int64_t>(thisAgent->d_cycle_count - d_cycles_at_start))
        {
            break;
        }
        do_one_top_level_phase(thisAgent);
    }
#ifndef NO_TIMING_STUFF
    thisAgent->timers_kernel.stop();
    thisAgent->timers_cpu.stop();
    thisAgent->timers_total_kernel_time.update(thisAgent->timers_kernel);
    thisAgent->timers_total_cpu_time.update(thisAgent->timers_cpu);
#endif
}

Symbol* attr_of_slot_just_decided(agent* thisAgent)
{
    if (thisAgent->bottom_goal->id->operator_slot->wmes)
    {
        return thisAgent->operator_symbol;
    }
    return thisAgent->state_symbol;
}

void run_for_n_selections_of_slot(agent* thisAgent, int64_t n, Symbol* attr_of_slot)
{
    int64_t count;
    bool was_decision_phase;

    if (n == -1)
    {
        run_forever(thisAgent);
        return;
    }
    if (n < -1)
    {
        return;
    }
#ifndef NO_TIMING_STUFF
    thisAgent->timers_cpu.start();
    thisAgent->timers_kernel.start();
#endif
    thisAgent->stop_soar = false;
    thisAgent->reason_for_stopping = 0;
    count = 0;
    while (!thisAgent->stop_soar && (count < n))
    {
        was_decision_phase = (thisAgent->current_phase == DECISION_PHASE);
        do_one_top_level_phase(thisAgent);
        if (was_decision_phase)
            if (attr_of_slot_just_decided(thisAgent) == attr_of_slot)
            {
                count++;
            }
    }
#ifndef NO_TIMING_STUFF
    thisAgent->timers_kernel.stop();
    thisAgent->timers_cpu.stop();
    thisAgent->timers_total_kernel_time.update(thisAgent->timers_kernel);
    thisAgent->timers_total_cpu_time.update(thisAgent->timers_cpu);
#endif
}

void run_for_n_selections_of_slot_at_level(agent* thisAgent, int64_t n,
        Symbol* attr_of_slot,
        goal_stack_level level)
{
    int64_t count;
    bool was_decision_phase;

    if (n == -1)
    {
        run_forever(thisAgent);
        return;
    }
    if (n < -1)
    {
        return;
    }
#ifndef NO_TIMING_STUFF
    thisAgent->timers_cpu.start();
    thisAgent->timers_kernel.start();
#endif
    thisAgent->stop_soar = false;
    thisAgent->reason_for_stopping = 0;
    count = 0;
    while (!thisAgent->stop_soar && (count < n))
    {
        was_decision_phase = (thisAgent->current_phase == DECISION_PHASE);
        do_one_top_level_phase(thisAgent);
        if (was_decision_phase)
        {
            if (thisAgent->bottom_goal->id->level < level)
            {
                break;
            }
            if (thisAgent->bottom_goal->id->level == level)
            {
                if (attr_of_slot_just_decided(thisAgent) == attr_of_slot)
                {
                    count++;
                }
            }
        }
    }
#ifndef NO_TIMING_STUFF
    thisAgent->timers_kernel.stop();
    thisAgent->timers_cpu.stop();
    thisAgent->timers_total_kernel_time.update(thisAgent->timers_kernel);
    thisAgent->timers_total_cpu_time.update(thisAgent->timers_cpu);
#endif
}

/* ===================================================================

             Loading the Initialization File ".init.soar"

   This routine looks for a file ".init.soar" in either the current
   directory or $HOME, and if found, loads it.
=================================================================== */

extern char* getenv();

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
    assert(thisAgent->top_goal == 0 &&
           "There should be no top goal when init_agent_memory is called!");
    if (thisAgent->top_goal)
    {
        return;
    }

    thisAgent->io_header = get_new_io_identifier(thisAgent, 'I');
    thisAgent->io_header_input = get_new_io_identifier(thisAgent, 'I');
    thisAgent->io_header_output = get_new_io_identifier(thisAgent, 'I');

    create_top_goal(thisAgent);
    if (thisAgent->sysparams[TRACE_CONTEXT_DECISIONS_SYSPARAM])
    {
        print_string(thisAgent, "\n");
        print_lowest_slot_in_context_stack(thisAgent);
    }
    thisAgent->current_phase = INPUT_PHASE;
    thisAgent->d_cycle_count++;
    thisAgent->wma_d_cycle_count++;

    /* The following code was taken from the do_input_cycle function of io.cpp */
    // Creating the io_header and adding the top state io header wme
    thisAgent->io_header_link = add_input_wme(thisAgent,
                                thisAgent->top_state,
                                thisAgent->io_symbol,
                                thisAgent->io_header);
    // Creating the input and output header symbols and wmes
    // RPM 9/06 changed to use thisAgent->input/output_link_symbol
    // Note we don't have to save these wmes for later release since their parent
    //  is already being saved (above), and when we release it they will automatically be released
    add_input_wme(thisAgent, thisAgent->io_header,
                  thisAgent->input_link_symbol,
                  thisAgent->io_header_input);
    add_input_wme(thisAgent, thisAgent->io_header,
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
    reset_timers(thisAgent);
    reset_max_stats(thisAgent);

    thisAgent->wma_timers->reset();
    thisAgent->epmem_timers->reset();
    thisAgent->smem_timers->reset();

    // This is an important part of the state of the agent for io purposes
    // (see io.cpp for details)
    thisAgent->prev_top_state = thisAgent->top_state;

}
