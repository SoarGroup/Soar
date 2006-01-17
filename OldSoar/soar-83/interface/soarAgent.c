/*
 * =======================================================================
 *  File:  soarAgent.c
 *
 * This file includes the routines for creating, initializing and destroying
 * Soar agents.  It also includes the code for the "Tcl" rhs function
 * which allows Soar agents to call Tcl functions on the rhs of productions.
 *
 * =======================================================================
 *
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

#include "soar.h"
#include "scheduler.h"
#include "soarCommands.h"
#include "rhsfun_examples.h"

#ifdef __hpux
#include <sys/syscall.h>
#include <unistd.h>
#define getrusage(a, b) syscall(SYS_GETRUSAGE, a, b)
#define getwd(arg) getcwd(arg, (size_t) 9999)
#endif /* __hpux */

#if defined(WIN32)
#include <direct.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define getwd(arg) getcwd((arg), (size_t) 9999)
/* macro taken from linux time.h file since not in VC++ !!! */
#define timersub(a, b, result)                                                \
  do {                                                                        \
    (result)->tv_sec = (a)->tv_sec - (b)->tv_sec;                             \
    (result)->tv_usec = (a)->tv_usec - (b)->tv_usec;                          \
    if ((result)->tv_usec < 0) {                                              \
      --(result)->tv_sec;                                                     \
      (result)->tv_usec += 1000000;                                           \
    }                                                                         \
  } while (0)
#endif /* WIN32 */

#if defined(MACINTOSH)
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define getwd(arg) getcwd((arg), (size_t) 9999)
#endif /* MACINTOSH */


agent *soar_agent;

list *all_soar_agents = NIL;

int agent_counter = -1;
int agent_count   = -1;

char * soar_version_string;

/* --------------------------------------------------------------------
                                Tcl 

   Sends a string to the Tcl interpreter
-------------------------------------------------------------------- */

Symbol *tcl_rhs_function_code (list *args) {
  Symbol *arg;
  growable_string script_to_run;
  int result;

  if (!args) {
    print ("Error: 'tcl' function called with no arguments.\n");
    return NIL;
  }

  script_to_run = make_blank_growable_string();

  for ( ; args != NIL; args = args->rest) 
    {
      arg = args->first;
    /* --- Note use of FALSE here--print the symbol itself, not a rereadable
       version of it --- */
      add_to_growable_string(&script_to_run,
                             symbol_to_string (arg, FALSE, NIL));
    }

  result = Tcl_GlobalEval(current_agent(interpreter), 
			  text_of_growable_string(script_to_run));

  if (result != TCL_OK)
    {
      print("Error: Failed RHS Tcl evaluation of \"%s\"\n", 
	    text_of_growable_string(script_to_run));
      print("Reason: %s\n", ((Tcl_Interp*) current_agent(interpreter))->result);
      control_c_handler(0);
      free_growable_string(script_to_run);
      return NIL;
    }

  free_growable_string(script_to_run);
  
  return make_sym_constant(((Tcl_Interp*) current_agent(interpreter))->result);
}

#ifdef ATTENTION_LAPSE
/* RMJ;
   When doing attentional lapsing, we need a function that determines
   when (and for how long) attentional lapses should occur.  This
   will normally be provided as a user-defined TCL procedure.
*/

#ifdef USE_TCL
long init_lapse_duration(struct timeval *tv) {
   int ret;
   long time_since_last_lapse;
   char buf[128];

   start_timer (current_real_time);
   timersub(current_real_time, tv, current_real_time);
   time_since_last_lapse = 1000 * current_real_time->tv_sec +
                           current_real_time->tv_usec / 1000;
   sprintf(buf, "InitLapseDuration %d", time_since_last_lapse);
   if ((Tcl_Eval(current_agent(interpreter), buf) == TCL_OK) &&
       (Tcl_GetInt(current_agent(interpreter),
                   ((Tcl_Interp*)current_agent(interpreter))->result,
                   &ret) == TCL_OK)) {
      return (long)ret;
   }
   return 0;
}
#endif


#endif

void init_soar_agent(void) {

  /* --- initialize everything --- */
  init_memory_utilities();
  init_symbol_tables();
  create_predefined_symbols();
  init_production_utilities();
  init_built_in_rhs_functions ();
  init_rete ();
  init_lexer ();
  init_firer ();
  init_decider ();
  init_soar_io ();
  init_chunker ();
  init_sysparams ();
  init_tracing ();
  init_explain();  /* AGR 564 */
#ifdef REAL_TIME_BEHAVIOR
  /* RMJ */
  init_real_time();
#endif
#ifdef ATTENTION_LAPSE
  /* RMJ */
  init_attention_lapse();
#endif


  /* --- add default object trace formats --- */
  add_trace_format (FALSE, FOR_ANYTHING_TF, NIL,
                    "%id %ifdef[(%v[name])]");
  add_trace_format (FALSE, FOR_STATES_TF, NIL,
                    "%id %ifdef[(%v[attribute] %v[impasse])]");
  { Symbol *evaluate_object_sym;
    evaluate_object_sym = make_sym_constant ("evaluate-object");
    add_trace_format (FALSE, FOR_OPERATORS_TF, evaluate_object_sym,
                      "%id (evaluate-object %o[object])");
    symbol_remove_ref (evaluate_object_sym);
  }
  /* --- add default stack trace formats --- */
  add_trace_format (TRUE, FOR_STATES_TF, NIL,
                    "%right[6,%dc]: %rsd[   ]==>S: %cs");
  add_trace_format (TRUE, FOR_OPERATORS_TF, NIL,
                    "%right[6,%dc]: %rsd[   ]   O: %co");
  reset_statistics ();

}

agent * create_soar_agent (char * agent_name) {
  int i;                                          /* loop index */
  char cur_path[MAXPATHLEN];   /* AGR 536 */

  agent * curr_agent;
  agent * this_agent;

  this_agent = (agent *) malloc(sizeof(agent));  
  memset(this_agent, 0, sizeof(*this_agent));

  curr_agent = soar_agent;
  soar_agent = this_agent;

  agent_counter++;
  agent_count++;

  current_agent(name)                               = savestring(agent_name);

  /* mvp 5-17-94 */
  current_agent(variables_set)                      = NIL;

#ifdef _WINDOWS
  current_agent(current_line)[0]		    = 0;
  current_agent(current_line_index)		    = 0;
#endif /* _WINDOWS */
  /* String redirection */
  current_agent(using_output_string)		    = FALSE;
  current_agent(using_input_string)		    = FALSE;
  current_agent(output_string)			    = NIL;
  current_agent(input_string)			    = NIL;

  current_agent(alias_list)                         = NIL;  /* AGR 568 */
  current_agent(all_wmes_in_rete)                   = NIL;
  current_agent(alpha_mem_id_counter)               = 0;
  current_agent(alternate_input_string)             = NIL;
  current_agent(alternate_input_suffix)             = NIL;
  current_agent(alternate_input_exit)               = FALSE;/* Soar-Bugs #54 */
  current_agent(backtrace_number)                   = 0;
  current_agent(beta_node_id_counter)               = 0;
  current_agent(bottom_goal)                        = NIL;
  current_agent(changed_slots)                      = NIL;
  current_agent(chunk_count)                        = 1;
  current_agent(chunk_free_problem_spaces)          = NIL;
  current_agent(chunky_problem_spaces)              = NIL;  /* AGR MVL1 */
  strcpy(current_agent(chunk_name_prefix),"chunk");	/* kjh (B14) */
  current_agent(context_slots_with_changed_acceptable_preferences) = NIL;
  current_agent(current_file)                       = NIL;
  current_agent(current_phase)                      = INPUT_PHASE;
  current_agent(current_symbol_hash_id)             = 0;
  current_agent(current_variable_gensym_number)     = 0;
  current_agent(current_wme_timetag)                = 1;
  current_agent(default_wme_depth)                  = 1;  /* AGR 646 */
  current_agent(disconnected_ids)                   = NIL;
  current_agent(existing_output_links)              = NIL;
  current_agent(output_link_changed)                = FALSE; /* KJC 11/9/98 */
  current_agent(go_number)                          = 1;
  current_agent(go_type)                            = GO_DECISION;
  current_agent(grounds_tc)                         = 0;
  current_agent(highest_goal_whose_context_changed) = NIL;
  current_agent(ids_with_unknown_level)             = NIL;
  current_agent(input_period)                       = 0;     /* AGR REW1 */
  current_agent(input_cycle_flag)                   = TRUE;  /* AGR REW1 */
  current_agent(justification_count)                = 1;
  current_agent(lex_alias)                          = NIL;  /* AGR 568 */
  current_agent(link_update_mode)                   = UPDATE_LINKS_NORMALLY;
  current_agent(locals_tc)                          = 0;
  current_agent(logging_to_file)                    = FALSE;
  current_agent(max_chunks_reached)                 = FALSE; /* MVP 6-24-94 */
  current_agent(mcs_counter)                        = 1;
  current_agent(memory_pools_in_use)                = NIL;
  current_agent(ms_assertions)                      = NIL;
  current_agent(ms_retractions)                     = NIL;
  current_agent(multi_attributes)                   = NIL;
  current_agent(num_existing_wmes)                  = 0;
  current_agent(num_wmes_in_rete)                   = 0;
  current_agent(potentials_tc)                      = 0;
  current_agent(prev_top_state)                     = NIL;
  current_agent(print_prompt_flag)                  = TRUE;
  current_agent(printer_output_column)              = 1;
  current_agent(production_being_fired)             = NIL;
  current_agent(productions_being_traced)           = NIL; 
  current_agent(promoted_ids)                       = NIL;
  current_agent(reason_for_stopping)                = "Startup";
  current_agent(redirecting_to_file)                = FALSE;
/*  current_agent(replaying)                   		= FALSE; /* kjh(CUSP-B10) */
  current_agent(replay_input_data)                  = FALSE;
  current_agent(slots_for_possible_removal)         = NIL;
  current_agent(stop_soar)                          = TRUE;           
  current_agent(system_halted)                      = FALSE;
  current_agent(token_additions)                    = 0;
  current_agent(top_dir_stack)                      = NIL;   /* AGR 568 */
  current_agent(top_goal)                           = NIL;
  current_agent(top_state)                          = NIL;
  current_agent(wmes_to_add)                        = NIL;
  current_agent(wmes_to_remove)                     = NIL;
  current_agent(wme_filter_list)                    = NIL; /* kjh(CUSP-B2) */

  /* RCHONG: begin 10.11 */
  current_agent(did_PE)                             = FALSE;
  /* RCHONG: end 10.11 */

  /* REW: begin 09.15.96 */ 
  current_agent(operand2_mode)                      = TRUE;
  current_agent(soar_verbose_flag)                  = FALSE;
  current_agent(FIRING_TYPE)                        = IE_PRODS;
  current_agent(ms_o_assertions)                    = NIL;
  current_agent(ms_i_assertions)                    = NIL;
  /* REW: end   09.15.96 */

  /* REW: begin 08.20.97 */
  current_agent(active_goal)                        = NIL;
  current_agent(active_level)                       = 0;
  current_agent(previous_active_level)              = 0;

  /* Initialize Waterfal specific lists */
  current_agent(nil_goal_retractions)               = NIL;
  /* REW: end   08.20.97 */

  /* KJC: delineate between Pref/WM(propose) and Pref/WM (apply) 10.05.98 */
  current_agent(applyPhase)                         = FALSE;

  /* REW: begin 10.24.97 */
  current_agent(waitsnc)                            = FALSE;
  current_agent(waitsnc_detect)                     = FALSE;
  /* REW: end   10.24.97 */

  if(!getwd(cur_path))
    print("Unable to set current directory while initializing agent.\n");
  current_agent(top_dir_stack) = (dir_stack_struct *) malloc(sizeof(dir_stack_struct));   /* AGR 568 */
  current_agent(top_dir_stack)->directory = (char *) malloc(MAXPATHLEN*sizeof(char));   /* AGR 568 */
  current_agent(top_dir_stack)->next = NIL;   /* AGR 568 */
  strcpy(current_agent(top_dir_stack)->directory, cur_path);   /* AGR 568 */

  for (i=0; i<NUM_PRODUCTION_TYPES; i++) {  
    current_agent(all_productions_of_type)[i] = NIL;
    current_agent(num_productions_of_type)[i] = 0;
  }

  current_agent(o_support_calculation_type) = 3;  /* KJC 7/31/00 */
  current_agent(attribute_preferences_mode) = 0;  /* RBD 4/17/95 */

  soar_init_callbacks((soar_callback_agent) soar_agent);

  init_soar_agent();

  add_rhs_function (make_sym_constant("tcl"),
                    tcl_rhs_function_code,
                    -1,
                    TRUE,
                    TRUE);

  add_bot_rhs_functions (soar_agent);

  /* This callback was in 7.0.*, but Chris W removed it.
   *  soar_add_callback(soar_agent, PRINT_CALLBACK, 
   *	    (soar_callback_fn) Soar_PrintToFile, 
   *	    (soar_callback_data) stdout,
   *	    (soar_callback_free_fn) NULL,
   *	    "default_print_callback");
   */

/* kjh CUSP B10 begin*/
/*  soar_add_callback(soar_agent, READ_CALLBACK, 
		    (soar_callback_fn) Soar_ReadFromFile, 
		    (soar_callback_data) stdin,
		    (soar_callback_free_fn) NULL,
		    "default_read_callback");
/* kjh CUSP B10 end */


  /* Add agent to global list   */
  /* of all agents.             */
  push(soar_agent, all_soar_agents);

  soar_invoke_callbacks(soar_agent, 
			AFTER_INIT_AGENT_CALLBACK,
			(soar_call_data) NULL);

  soar_agent = curr_agent;

  scheduler_cycle_count = 0;

  return this_agent;

}

void destroy_soar_agent (agent * delete_agent)
{
  cons  * c;
  cons  * prev = NULL;   /* Initialized to placate gcc -Wall */
  agent * the_agent;
 
  remove_rhs_function (make_sym_constant("tcl"));  /* from Koss 8/00 */
  remove_bot_rhs_functions (delete_agent);
  remove_built_in_rhs_functions();

  /* Splice agent structure out of global list of agents. */
  for (c = all_soar_agents; c != NIL; c = c->rest) {  
    the_agent = (agent *) c->first;
    if (the_agent == delete_agent) {
      if (c == all_soar_agents) {
	all_soar_agents = c->rest;
      } else {
	prev->rest = c->rest;
      }
      break;
    }
    prev = c;
  }

  /* Free structures stored in agent structure */
  free(delete_agent->name);

  /* KNOWN MEMORY LEAK! Need to track down and free ALL structures */
  /* pointed to by fields in the agent structure.                  */

  /* Free soar agent structure */
  free((void *) delete_agent);
 
  agent_count--;
}

void init_soar (void)
{
  char buffer[1000];

  if (strcmp(MICRO_VERSION_NUMBER,"")) {
    sprintf(buffer,
	    "%d.%d.%s",
	    MAJOR_VERSION_NUMBER, 
	    MINOR_VERSION_NUMBER, 
	    MICRO_VERSION_NUMBER);
  } else {
    sprintf(buffer,
	    "%d.%d",
	    MAJOR_VERSION_NUMBER, 
	    MINOR_VERSION_NUMBER);
  }

  soar_version_string = savestring(buffer);

  /* These are not in the buffer right now.  They should be added to
   * the startup string, but Tcl uses the version number for package
   * info.  So can't append all this stuff.  Build another string.
   */
/* kjh(CUSP-B8) begin */
#ifdef USE_TCL
  strcat(buffer," Tcl");
#endif
#ifdef USE_TK
  strcat(buffer," Tk");
#endif
#ifdef __MSDOS__
  strcat(buffer," MS-DOS");
#endif
#ifdef __SC__
  strcat(buffer," SC");
#endif
#ifdef _CodeWarrior_
  strcat(buffer," CodeWarrior");
#endif
#ifdef __hpux
  strcat(buffer," hpux");
#endif
#ifdef THINK_C
  strcat(buffer," ThinkC");
#endif
#ifdef DEBUG_MEMORY
  strcat(buffer," DebugMem");
#endif
#ifdef MEMORY_POOL_STATS
  strcat(buffer," MStats");
#endif
#ifdef USE_X_DISPLAY
  strcat(buffer," XDisplay");
#endif
#ifdef __cplusplus
  strcat(buffer," C++");
#endif
#ifdef DETAILED_TIMING_STATS
  strcat(buffer," DetailedTiming");
#endif
#ifdef NO_TIMING_STUFF
  strcat(buffer," NoTiming");
#endif
#ifndef USE_STDARGS
  strcat(buffer," NoStdArgs");
#endif
/* kjh(CUSP-B8) end */

  /* --- set the random number generator seed to a "random" value --- */
  {
#ifndef NO_TIMING_STUFF
#if defined(__hpux) || defined(_WINDOWS) || defined(WIN32) || defined(MACINTOSH)
	srand(time(0));
#else
	struct timeval tv;
	gettimeofday (&tv, NIL);
	srandom(tv.tv_usec);
#endif
#else
#if defined(THINK_C) || defined(_WINDOWS) || defined(WIN32) || defined(MACINTOSH)
	srand(time(0));
#else
	srandom(time(0));
#endif
#endif
   }

  setup_signal_handling();
#if 0
  init_built_in_commands ();
  init_parser ();
  soar_invoke_callbacks(soar_agent, 
			SYSTEM_STARTUP_CALLBACK,
			(soar_call_data) NULL);
#endif

}

