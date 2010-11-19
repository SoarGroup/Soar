#include <portability.h>

/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/*************************************************************************
 *
 *  file:  agent.cpp
 *
 * =======================================================================
 *  Initialization for the agent structure.  Also the cleanup routine
 *  when an agent is destroyed.  These routines are usually replaced
 *  by the same-named routines in the Tcl interface file soarAgent.c
 *  The versions in this file are used only when not linking in Tcl.
 *  HOWEVER, this code should be maintained, and the agent structure
 *  must be kept up to date.
 * =======================================================================
 */

#include <stdlib.h>
#include <map>

#include "agent.h"
#include "kernel.h"
#include "mem.h"
#include "lexer.h"
#include "symtab.h"
#include "gdatastructs.h"
#include "rhsfun.h"
#include "instantiations.h"
#include "production.h"
#include "gsysparam.h"
#include "init_soar.h"
#include "decide.h"
#include "print.h"
#include "recmem.h"
#include "backtrace.h"
#include "chunk.h"
#include "explain.h"
#include "rete.h"
#include "trace.h"
#include "callback.h"
#include "io_soar.h"
#include "xml.h"
#include "utilities.h"
#include "exploration.h"
#include "reinforcement_learning.h"
#include "decision_manipulation.h"
#include "wma.h"
#include "episodic_memory.h"
#include "semantic_memory.h"


/* ================================================================== */

//char * soar_version_string;

/* ===================================================================

                           Initialization Function

=================================================================== */

void init_soar_agent(agent* thisAgent) {

  /* JC ADDED: initialize the rhs function linked list */
  thisAgent->rhs_functions = NIL;

   /* --- initialize everything --- */
  init_symbol_tables(thisAgent);
  create_predefined_symbols(thisAgent);
  init_production_utilities(thisAgent);
  init_built_in_rhs_functions (thisAgent);
  init_rete (thisAgent);
  init_lexer (thisAgent);
  init_firer (thisAgent);
  init_decider (thisAgent);
  init_soar_io (thisAgent);
  init_chunker (thisAgent);
  init_tracing (thisAgent);
  init_explain(thisAgent);  /* AGR 564 */
  select_init(thisAgent);
  predict_init(thisAgent);

  init_memory_pool( thisAgent, &( thisAgent->wma_decay_element_pool ), sizeof( wma_decay_element ), "wma_decay" );
  wma_init(thisAgent);

  thisAgent->epmem_params->exclusions->set_value( "epmem" );
  thisAgent->epmem_params->exclusions->set_value( "smem" );

#ifdef REAL_TIME_BEHAVIOR
  /* RMJ */
  init_real_time(thisAgent);
#endif


  /* --- add default object trace formats --- */
  add_trace_format (thisAgent, FALSE, FOR_ANYTHING_TF, NIL,
                    "%id %ifdef[(%v[name])]");
  add_trace_format (thisAgent, FALSE, FOR_STATES_TF, NIL,
                    "%id %ifdef[(%v[attribute] %v[impasse])]");
  { Symbol *evaluate_object_sym;
    evaluate_object_sym = make_sym_constant (thisAgent, "evaluate-object");
    add_trace_format (thisAgent, FALSE, FOR_OPERATORS_TF, evaluate_object_sym,
                      "%id (evaluate-object %o[object])");
    symbol_remove_ref (thisAgent, evaluate_object_sym);
  }
  /* --- add default stack trace formats --- */
  add_trace_format (thisAgent, TRUE, FOR_STATES_TF, NIL,
                    "%right[6,%dc]: %rsd[   ]==>S: %cs");
  add_trace_format (thisAgent, TRUE, FOR_OPERATORS_TF, NIL,
                    "%right[6,%dc]: %rsd[   ]   O: %co");

  reset_statistics (thisAgent);

  /* RDF: For gSKI */
  init_agent_memory(thisAgent);
  /* END */

}

agent * create_soar_agent (char * agent_name) {                                          /* loop index */
  char cur_path[MAXPATHLEN];   /* AGR 536 */

  //agent* newAgent = static_cast<agent *>(malloc(sizeof(agent)));
  agent* newAgent = new agent();

  newAgent->current_tc_number = 0;

  newAgent->name                               = savestring(agent_name);

  /* mvp 5-17-94 */
  newAgent->variables_set                      = NIL;

//#ifdef _WINDOWS
//  newAgent->current_line[0]                    = 0;
//  newAgent->current_line_index                 = 0;
//#endif /* _WINDOWS */

  newAgent->all_wmes_in_rete                   = NIL;
  newAgent->alpha_mem_id_counter               = 0;
  newAgent->alternate_input_string             = NIL;
  newAgent->alternate_input_suffix             = NIL;
  newAgent->alternate_input_exit               = FALSE;/* Soar-Bugs #54 */
  newAgent->backtrace_number                   = 0;
  newAgent->beta_node_id_counter               = 0;
  newAgent->bottom_goal                        = NIL;
  newAgent->changed_slots                      = NIL;
  newAgent->chunk_count                        = 1;
  newAgent->chunk_free_problem_spaces          = NIL;
  newAgent->chunky_problem_spaces              = NIL;  /* AGR MVL1 */
  strcpy(newAgent->chunk_name_prefix,"chunk");	/* ajc (5/14/02) */
  newAgent->context_slots_with_changed_acceptable_preferences = NIL;
  newAgent->current_file                       = NIL;
  newAgent->current_phase                      = INPUT_PHASE;
  newAgent->applyPhase                         = FALSE;
  newAgent->current_symbol_hash_id             = 0;
  newAgent->current_variable_gensym_number     = 0;
  newAgent->current_wme_timetag                = 1;
  newAgent->default_wme_depth                  = 1;  /* AGR 646 */
  newAgent->disconnected_ids                   = NIL;
  newAgent->existing_output_links              = NIL;
  newAgent->output_link_changed                = FALSE;  /* KJC 11/9/98 */
  /* newAgent->explain_flag                       = FALSE; */
  newAgent->go_number                          = 1;
  newAgent->go_type                            = GO_DECISION;
  newAgent->grounds_tc                         = 0;
  newAgent->highest_goal_whose_context_changed = NIL;
  newAgent->ids_with_unknown_level             = NIL;
  newAgent->input_period                       = 0;     /* AGR REW1 */
  newAgent->input_cycle_flag                   = TRUE;  /* AGR REW1 */
  newAgent->justification_count                = 1;
  newAgent->lex_alias                          = NIL;  /* AGR 568 */
  newAgent->link_update_mode                   = UPDATE_LINKS_NORMALLY;
  newAgent->locals_tc                          = 0;
  newAgent->max_chunks_reached                 = FALSE; /* MVP 6-24-94 */
  newAgent->mcs_counter                        = 1;
  newAgent->memory_pools_in_use                = NIL;
  newAgent->ms_assertions                      = NIL;
  newAgent->ms_retractions                     = NIL;
  newAgent->num_existing_wmes                  = 0;
  newAgent->num_wmes_in_rete                   = 0;
  newAgent->potentials_tc                      = 0;
  newAgent->prev_top_state                     = NIL;
  newAgent->print_prompt_flag                  = TRUE;
  newAgent->printer_output_column              = 1;
  newAgent->production_being_fired             = NIL;
  newAgent->productions_being_traced           = NIL;
  newAgent->promoted_ids                       = NIL;
  newAgent->reason_for_stopping                = "Startup";
  newAgent->slots_for_possible_removal         = NIL;
  newAgent->stop_soar                          = TRUE;
  newAgent->system_halted                      = FALSE;
  newAgent->token_additions                    = 0;
  newAgent->top_dir_stack                      = NIL;   /* AGR 568 */
  newAgent->top_goal                           = NIL;
  newAgent->top_state                          = NIL;
  newAgent->wmes_to_add                        = NIL;
  newAgent->wmes_to_remove                     = NIL;
  newAgent->wme_filter_list                    = NIL;   /* Added this to avoid
															    access violation
																-AJC (5/13/02) */
  newAgent->multi_attributes                   = NIL;

  /* REW: begin 09.15.96 */

  newAgent->did_PE                             = FALSE;
  newAgent->soar_verbose_flag                  = FALSE;
  newAgent->FIRING_TYPE                        = IE_PRODS;
  newAgent->ms_o_assertions                    = NIL;
  newAgent->ms_i_assertions                    = NIL;

  /* REW: end   09.15.96 */

  newAgent->postponed_assertions			   = NIL;

  /* REW: begin 08.20.97 */
  newAgent->active_goal                        = NIL;
  newAgent->active_level                       = 0;
  newAgent->previous_active_level              = 0;

  /* Initialize Waterfall-specific lists */
  newAgent->nil_goal_retractions               = NIL;
  /* REW: end   08.20.97 */

  /* REW: begin 10.24.97 */
  newAgent->waitsnc                            = FALSE;
  newAgent->waitsnc_detect                     = FALSE;
  /* REW: end   10.24.97 */

  /* Initializing rete stuff */
  for (int i=0; i < 256; i++) {
     newAgent->actual[i]=0;
     newAgent->if_no_merging[i]=0;
     newAgent->if_no_sharing[i]=0;
  }

  /* Initializing lexeme */
  newAgent->lexeme.type = NULL_LEXEME;
  newAgent->lexeme.string[0] = 0;
  newAgent->lexeme.length = 0;
  newAgent->lexeme.int_val = 0;
  newAgent->lexeme.float_val = 0.0;
  newAgent->lexeme.id_letter = 'A';
  newAgent->lexeme.id_number = 0;

  /* Initializing all the timer structures */
#ifndef NO_TIMING_STUFF
  newAgent->timers_cpu.set_enabled(&(newAgent->sysparams[TIMERS_ENABLED]));
  newAgent->timers_kernel.set_enabled(&(newAgent->sysparams[TIMERS_ENABLED]));
  newAgent->timers_phase.set_enabled(&(newAgent->sysparams[TIMERS_ENABLED]));
#ifdef DETAILED_TIMING_STATS
  newAgent->timers_gds.set_enabled(&(newAgent->sysparams[TIMERS_ENABLED]));
#endif
  reset_timers(newAgent);
#endif

  reset_max_stats(newAgent);

  newAgent->real_time_tracker = 0;
  newAgent->attention_lapse_tracker = 0;

  if(!getcwd(cur_path, MAXPATHLEN))
    print(newAgent, "Unable to set current directory while initializing agent.\n");
  newAgent->top_dir_stack = static_cast<dir_stack_struct *>(malloc(sizeof(dir_stack_struct)));   /* AGR 568 */
  newAgent->top_dir_stack->directory = static_cast<char *>(malloc(MAXPATHLEN*sizeof(char)));   /* AGR 568 */
  newAgent->top_dir_stack->next = NIL;   /* AGR 568 */
  strcpy(newAgent->top_dir_stack->directory, cur_path);   /* AGR 568 */

  /* changed all references of 'i', a var belonging to a previous for loop, to 'productionTypeCounter' to be unique
    stokesd Sept 10 2004*/
  for (int productionTypeCounter=0; productionTypeCounter<NUM_PRODUCTION_TYPES; productionTypeCounter++) {
    newAgent->all_productions_of_type[productionTypeCounter] = NIL;
    newAgent->num_productions_of_type[productionTypeCounter] = 0;
  }

  newAgent->o_support_calculation_type = 4; /* KJC 7/00 */ // changed from 3 to 4 by voigtjr  (/* bugzilla bug 339 */)
//#ifdef NUMERIC_INDIFFERENCE
  newAgent->numeric_indifferent_mode = NUMERIC_INDIFFERENT_MODE_SUM;
//#endif

  /* JC ADDED: Make sure that the RHS functions get initialized correctly */
  newAgent->rhs_functions = NIL;

  // JRV: Allocates data for XML generation
  xml_create( newAgent );

  soar_init_callbacks( newAgent );

  //
  // This call is needed to set up callbacks.
  init_memory_utilities(newAgent);

  //
  // This was moved here so that system parameters could
  // be set before the agent was initialized.
  init_sysparams (newAgent);


  // exploration initialization
  newAgent->exploration_params[ EXPLORATION_PARAM_EPSILON ] = exploration_add_parameter( 0.1, &exploration_validate_epsilon, "epsilon" );
  newAgent->exploration_params[ EXPLORATION_PARAM_TEMPERATURE ] = exploration_add_parameter( 25, &exploration_validate_temperature, "temperature" );

  // rl initialization
  newAgent->rl_params = new rl_param_container( newAgent );
  newAgent->rl_stats = new rl_stat_container( newAgent ); 

  rl_initialize_template_tracking( newAgent );

  newAgent->rl_first_switch = true;


  // select initialization
  newAgent->select = new select_info;
  select_init( newAgent );


  // predict initialization
  newAgent->prediction = new std::string();
  predict_init( newAgent );


  // wma initialization
  newAgent->wma_params = new wma_param_container( newAgent );
  newAgent->wma_stats = new wma_stat_container( newAgent );

  newAgent->wma_forget_pq = new wma_forget_p_queue;
  newAgent->wma_touched_elements = new wma_wme_set;  
  newAgent->wma_initialized = false;
  newAgent->wma_tc_counter = 2;


  // epmem initialization
  newAgent->epmem_params = new epmem_param_container( newAgent );
  newAgent->epmem_stats = new epmem_stat_container( newAgent );  
  newAgent->epmem_timers = new epmem_timer_container( newAgent );

  newAgent->epmem_db = new soar_module::sqlite_database();
  newAgent->epmem_stmts_common = NULL;  
  newAgent->epmem_stmts_graph = NULL;
  
  newAgent->epmem_node_removals = new std::map<epmem_node_id, bool>();
  newAgent->epmem_node_mins = new std::vector<epmem_time_id>();
  newAgent->epmem_node_maxes = new std::vector<bool>();

  newAgent->epmem_edge_removals = new std::map<epmem_node_id, bool>();
  newAgent->epmem_edge_mins = new std::vector<epmem_time_id>();
  newAgent->epmem_edge_maxes = new std::vector<bool>();

  newAgent->epmem_id_repository = new epmem_parent_id_pool();
  newAgent->epmem_id_replacement = new epmem_return_id_pool();
  newAgent->epmem_id_ref_counts = new epmem_id_ref_counter();

  newAgent->epmem_validation = 0;
  newAgent->epmem_first_switch = true;

  // smem initialization
  newAgent->smem_params = new smem_param_container( newAgent );
  newAgent->smem_stats = new smem_stat_container( newAgent );  
  newAgent->smem_timers = new smem_timer_container( newAgent );

  newAgent->smem_db = new soar_module::sqlite_database();

  newAgent->smem_validation = 0;
  newAgent->smem_first_switch = true;

  // statistics initialization
  newAgent->dc_stat_tracking = false;
  newAgent->stats_db = new soar_module::sqlite_database();

  newAgent->substate_break_level = 0;

  return newAgent;
}

/*
===============================

===============================
*/
void destroy_soar_agent (agent * delete_agent)
{
  //print(delete_agent, "\nDestroying agent %s.\n", delete_agent->name);  /* AGR 532 */

//#ifdef USE_X_DISPLAY
//
//  /* Destroy X window associated with agent */
//  destroy_agent_window (delete_agent);
//#endif /* USE_X_DISPLAY */

  /////////////////////////////////////////////////////////
  // Soar Modules - could potentially rely on hash tables
  /////////////////////////////////////////////////////////

  // cleanup exploration
  for ( int i=0; i<EXPLORATION_PARAMS; i++ )
	  delete delete_agent->exploration_params[ i ];

  // cleanup Soar-RL
  delete delete_agent->rl_params;
  delete delete_agent->rl_stats;

  // cleanup select
  select_init( delete_agent );
  delete delete_agent->select;

  // cleanup predict
  delete delete_agent->prediction;

  // cleanup wma
  wma_deinit( delete_agent );
  delete delete_agent->wma_forget_pq;
  delete delete_agent->wma_touched_elements;  
  delete delete_agent->wma_params;
  delete delete_agent->wma_stats;

  // cleanup epmem
  epmem_close( delete_agent );
  delete delete_agent->epmem_params;
  delete delete_agent->epmem_stats;
  delete delete_agent->epmem_timers;

  delete delete_agent->epmem_node_removals;
  delete delete_agent->epmem_node_mins;
  delete delete_agent->epmem_node_maxes;
  delete delete_agent->epmem_edge_removals;
  delete delete_agent->epmem_edge_mins;
  delete delete_agent->epmem_edge_maxes;
  delete delete_agent->epmem_id_repository;
  delete delete_agent->epmem_id_replacement;
  delete delete_agent->epmem_id_ref_counts;

  delete delete_agent->epmem_db;


  // cleanup smem
  smem_close( delete_agent );
  delete delete_agent->smem_params;
  delete delete_agent->smem_stats;
  delete delete_agent->smem_timers;

  delete delete_agent->smem_db;

  // cleanup statistics db
  stats_close( delete_agent );
  delete delete_agent->stats_db;
  delete_agent->stats_db = 0;

  /////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////

  remove_built_in_rhs_functions(delete_agent);

  /* Free structures stored in agent structure */
  free(delete_agent->name);
  free(delete_agent->top_dir_stack->directory);
  free(delete_agent->top_dir_stack);

  /* Freeing the agent's multi attributes structure */
  multi_attribute* lastmattr = 0;
  for ( multi_attribute* curmattr = delete_agent->multi_attributes;
        curmattr != 0;
        curmattr = curmattr->next ) {

     symbol_remove_ref(delete_agent, curmattr->symbol);

     free_memory(delete_agent, lastmattr, MISCELLANEOUS_MEM_USAGE);
     lastmattr = curmattr;
  }
  free_memory(delete_agent, lastmattr, MISCELLANEOUS_MEM_USAGE);

  /* Freeing all the productions owned by this agent */
  excise_all_productions(delete_agent, false);

  /* Releasing all the predefined symbols */
  release_predefined_symbols(delete_agent);

  /* Releasing rete stuff RPM 11/06 */
  free_with_pool(&delete_agent->rete_node_pool, delete_agent->dummy_top_node);
  free_with_pool(&delete_agent->token_pool, delete_agent->dummy_top_token);

  /* Cleaning up the various callbacks
     TODO: Not clear why callbacks need to take the agent pointer essentially twice.
  */
  soar_remove_all_monitorable_callbacks(delete_agent);

  /* RPM 9/06 begin */

  free_memory(delete_agent, delete_agent->left_ht, HASH_TABLE_MEM_USAGE);
  free_memory(delete_agent, delete_agent->right_ht, HASH_TABLE_MEM_USAGE);
  free_memory(delete_agent, delete_agent->rhs_variable_bindings, MISCELLANEOUS_MEM_USAGE);

  /* Releasing memory allocated in inital call to start_lex_from_file from init_lexer */
  free_memory_block_for_string(delete_agent, delete_agent->current_file->filename);
  free_memory (delete_agent, delete_agent->current_file, MISCELLANEOUS_MEM_USAGE);

  /* Releasing trace formats (needs to happen before tracing hashtables are released) */
  remove_trace_format (delete_agent, FALSE, FOR_ANYTHING_TF, NIL);
  remove_trace_format (delete_agent, FALSE, FOR_STATES_TF, NIL);
  Symbol *evaluate_object_sym = find_sym_constant (delete_agent, "evaluate-object");
  remove_trace_format (delete_agent, FALSE, FOR_OPERATORS_TF, evaluate_object_sym);
  remove_trace_format (delete_agent, TRUE, FOR_STATES_TF, NIL);
  remove_trace_format (delete_agent, TRUE, FOR_OPERATORS_TF, NIL);

  /* Releasing hashtables allocated in init_tracing */
  for (int i=0; i<3; i++) {
	free_hash_table(delete_agent, delete_agent->object_tr_ht[i]);
	free_hash_table(delete_agent, delete_agent->stack_tr_ht[i]);
  }

  /* Releasing memory allocated in init_rete */
  for (int i=0; i<16; i++) {
	  free_hash_table(delete_agent, delete_agent->alpha_hash_tables[i]);
  }

  /* Releasing other hashtables */
  free_hash_table(delete_agent, delete_agent->variable_hash_table);
  free_hash_table(delete_agent, delete_agent->identifier_hash_table);
  free_hash_table(delete_agent, delete_agent->sym_constant_hash_table);
  free_hash_table(delete_agent, delete_agent->int_constant_hash_table);
  free_hash_table(delete_agent, delete_agent->float_constant_hash_table);

  /* Releasing memory pools */
  memory_pool* cur_pool = delete_agent->memory_pools_in_use;
  memory_pool* next_pool;
  while(cur_pool != NIL) {
	  next_pool = cur_pool->next;
	  free_memory_pool(delete_agent, cur_pool);
	  cur_pool = next_pool;
  }

  /* RPM 9/06 end */

  // JRV: Frees data used by XML generation
  xml_destroy( delete_agent );

  /* Free soar agent structure */
  delete delete_agent;
}
