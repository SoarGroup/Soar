/*************************************************************************
 *
 *  file:  agent.c
 *
 * =======================================================================
 *  Initialization for the agent structure.  Also the cleanup routine
 *  when an agent is destroyed.  These routines are usually replaced
 *  by the same-named routines in the Tcl interface file soarAgent.c
 *  The versions in this file are used only when not linking in Tcl.
 *  HOWEVER, this code should be maintained, and the agent structure
 *  must be kept up to date.
 * =======================================================================
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
/* ===================================================================
                     Agent-related functions
   =================================================================== */

#include "soarkernel.h"
#include "scheduler.h"

#if defined(WIN32)
#include <direct.h>
#define getwd(arg) getcwd(arg, 9999)
#endif /* WIN32 */

#if defined(MACINTOSH)
#define getwd(arg) getcwd(arg, 9999)
#endif /* MACINTOSH */


#ifdef __hpux
#include <sys/syscall.h>
#include <unistd.h>
#define getrusage(a, b) syscall(SYS_GETRUSAGE, a, b)
#define getwd(arg) getcwd(arg, (size_t) 9999)
#endif /* __hpux */

agent *soar_agent;

list *all_soar_agents = NIL;

int agent_counter = -1;
int agent_count = -1;

char * soar_version_string;

/* ===================================================================
   
                           Initialization Function

=================================================================== */

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
  current_agent(context_slots_with_changed_acceptable_preferences) = NIL;
  current_agent(current_file)                       = NIL;
  current_agent(current_phase)                      = INPUT_PHASE;
  current_agent(current_symbol_hash_id)             = 0;
  current_agent(current_variable_gensym_number)     = 0;
  current_agent(current_wme_timetag)                = 1;
  current_agent(default_wme_depth)                  = 1;  /* AGR 646 */
  current_agent(disconnected_ids)                   = NIL;
  current_agent(existing_output_links)              = NIL;
  current_agent(output_link_changed)                = FALSE;  /* KJC 11/9/98 */
  /* current_agent(explain_flag)                       = FALSE; */
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
  current_agent(multi_attributes)                   = NIL;


  /* REW: begin 09.15.96 */

  current_agent(did_PE)                             = FALSE;
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

  /* Initialize Waterfall-specific lists */
  current_agent(nil_goal_retractions)               = NIL;
  /* REW: end   08.20.97 */

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

  current_agent(o_support_calculation_type) = 3; /* KJC 7/00 */
  current_agent(attribute_preferences_mode) = 0; /* RBD 4/17/95 */

  soar_init_callbacks((soar_callback_agent) soar_agent);

  init_soar_agent();
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
  agent * prev_agent;
 
  if (soar_agent == delete_agent)
    {
      print("\nError: Attempt to delete current agent -- ignored.\n");
      return;      
    }

  prev_agent = soar_agent;
  soar_agent = delete_agent;

  print("\nDestroying agent %s.\n", delete_agent->name);  /* AGR 532 */

#ifdef USE_X_DISPLAY

  /* Destroy X window associated with agent */
  destroy_agent_window (delete_agent);
#endif /* USE_X_DISPLAY */

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
  /* pointed to be fields in the agent structure.                  */

  /* Free soar agent structure */
  free((void *) delete_agent);
 
  agent_count--;

  soar_agent = prev_agent;

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

  /*  RCHONG and REW added following to soar_version_string.
   *  KJC leaving it out for now, since Tcl Pkg mechanism
   *  relies on soar_version_string to load right package.
   *  Will add it in interface level with other options.
   */

  /* 
   *   if (current_agent(operand2_mode) == TRUE)
   *      sprintf(buffer,"%s-%s", OPERAND_MODE_NAME);
   */
/* REW: end   09.15.96 */


#ifdef REAL_TIME_BEHAVIOR
  /* RMJ */
  current_real_time = (struct timeval *) malloc(sizeof(struct timeval));
#endif


  /* --- set the random number generator seed to a "random" value --- */
  {
#ifndef NO_TIMING_STUFF
    struct timeval tv;
    start_timer (&tv);
#if defined(__hpux) || defined(_WINDOWS) || defined(WIN32) || defined(MACINTOSH)
    srand(tv.tv_usec);
#else
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

  soar_agent = create_soar_agent("soar");
  init_built_in_commands ();
  init_parser (); /* This should be in interface.c? */

  soar_invoke_callbacks(soar_agent, 
			SYSTEM_STARTUP_CALLBACK,
			(soar_call_data) NULL);

  printf(soar_version_string);
  printf(soar_news_string);
}

