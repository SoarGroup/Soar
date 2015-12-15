#include <ebc_chunk.h>
#include <ebc_variablize.h>
#include "memory_manager.h"
#include "portability.h"

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
#include "debug.h"
#include "kernel.h"
#include "mem.h"
#include "lexer.h"
#include "symtab.h"
#include "rhs.h"
#include "rhs_functions.h"
#include "instantiations.h"
#include "production.h"
#include "gsysparam.h"
#include "init_soar.h"
#include "decide.h"
#include "print.h"
#include "recmem.h"
#include "backtrace.h"
#include "explain.h"
#include "rete.h"
#include "trace.h"
#include "callback.h"
#include "io_soar.h"
#include "xml.h"
#include "stats.h"
#include "soar_module.h"
#include "exploration.h"
#include "reinforcement_learning.h"
#include "decision_manipulation.h"
#include "wma.h"
#include "episodic_memory.h"
#include "semantic_memory.h"
#include "soar_instance.h"
#include "output_manager.h"
#ifndef NO_SVS
#include "svs_interface.h"
#endif

/* ===================================================================

                           Initialization Function

=================================================================== */

void init_soar_agent(agent* thisAgent)
{

    /* JC ADDED: initialize the rhs function linked list */
    thisAgent->rhs_functions = NIL;

    /* --- initialize everything --- */
    init_symbol_tables(thisAgent);
    create_predefined_symbols(thisAgent);
    init_production_utilities(thisAgent);
    init_built_in_rhs_functions(thisAgent);
    init_rete(thisAgent);
    init_firer(thisAgent);
    init_decider(thisAgent);
    init_soar_io(thisAgent);
    init_chunker(thisAgent);
    init_tracing(thisAgent);
    init_explain(thisAgent);  /* AGR 564 */
    select_init(thisAgent);
    predict_init(thisAgent);

    thisAgent->memoryManager->init_memory_pool(MP_gds, sizeof(goal_dependency_set), "gds");

    thisAgent->memoryManager->init_memory_pool(MP_rl_info, sizeof(rl_data), "rl_id_data");
    thisAgent->memoryManager->init_memory_pool(MP_rl_et, sizeof(rl_et_map), "rl_et");
    thisAgent->memoryManager->init_memory_pool(MP_rl_rule, sizeof(rl_rule_list), "rl_rules");

    thisAgent->memoryManager->init_memory_pool(MP_wma_decay_element, sizeof(wma_decay_element), "wma_decay");
    thisAgent->memoryManager->init_memory_pool(MP_wma_decay_set, sizeof(wma_decay_set), "wma_decay_set");
    thisAgent->memoryManager->init_memory_pool(MP_wma_wme_oset, sizeof(wma_pooled_wme_set), "wma_oset");
    thisAgent->memoryManager->init_memory_pool(MP_wma_slot_refs, sizeof(wma_sym_reference_map), "wma_slot_ref");

    thisAgent->memoryManager->init_memory_pool(MP_epmem_wmes, sizeof(epmem_wme_stack), "epmem_wmes");
    thisAgent->memoryManager->init_memory_pool(MP_epmem_info, sizeof(epmem_data), "epmem_id_data");
    thisAgent->memoryManager->init_memory_pool(MP_smem_wmes, sizeof(smem_wme_stack), "smem_wmes");
    thisAgent->memoryManager->init_memory_pool(MP_smem_info, sizeof(smem_data), "smem_id_data");

    thisAgent->memoryManager->init_memory_pool(MP_epmem_literal, sizeof(epmem_literal), "epmem_literals");
    thisAgent->memoryManager->init_memory_pool(MP_epmem_pedge, sizeof(epmem_pedge), "epmem_pedges");
    thisAgent->memoryManager->init_memory_pool(MP_epmem_uedge, sizeof(epmem_uedge), "epmem_uedges");
    thisAgent->memoryManager->init_memory_pool(MP_epmem_interval, sizeof(epmem_interval), "epmem_intervals");

    thisAgent->epmem_params->exclusions->set_value("epmem");
    thisAgent->epmem_params->exclusions->set_value("smem");

    thisAgent->smem_params->base_incremental_threshes->set_string("10");

#ifdef REAL_TIME_BEHAVIOR
    /* RMJ */
    init_real_time(thisAgent);
#endif


    /* --- add default object trace formats --- */
    add_trace_format(thisAgent, false, FOR_ANYTHING_TF, NIL,
                     "%id %ifdef[(%v[name])]");
    add_trace_format(thisAgent, false, FOR_STATES_TF, NIL,
                     "%id %ifdef[(%v[attribute] %v[impasse])]");
    {
        Symbol* evaluate_object_sym;
        evaluate_object_sym = make_str_constant(thisAgent, "evaluate-object");
        add_trace_format(thisAgent, false, FOR_OPERATORS_TF, evaluate_object_sym,
                         "%id (evaluate-object %o[object])");
        symbol_remove_ref(thisAgent, evaluate_object_sym);
    }
    /* --- add default stack trace formats --- */
    add_trace_format(thisAgent, true, FOR_STATES_TF, NIL,
                     "%right[6,%dc]: %rsd[   ]==>S: %cs");
    add_trace_format(thisAgent, true, FOR_OPERATORS_TF, NIL,
                     "%right[6,%dc]: %rsd[   ]   O: %co");

    reset_statistics(thisAgent);

#ifndef NO_SVS
    thisAgent->svs = make_svs(thisAgent);
#endif

    /* RDF: For gSKI */
    init_agent_memory(thisAgent);
    /* END */

}

agent* create_soar_agent(char* agent_name)                                               /* loop index */
{
    char cur_path[MAXPATHLEN];   /* AGR 536 */

    //agent* thisAgent = static_cast<agent *>(malloc(sizeof(agent)));
    agent* thisAgent = new agent();
    thisAgent->output_settings = new AgentOutput_Info();

    thisAgent->current_tc_number = 0;

    thisAgent->name                               = savestring(agent_name);

    /* mvp 5-17-94 */
    thisAgent->variables_set                      = NIL;

    //#ifdef _WINDOWS
    //  thisAgent->current_line[0]                    = 0;
    //  thisAgent->current_line_index                 = 0;
    //#endif /* _WINDOWS */

    thisAgent->all_wmes_in_rete                   = NIL;
    thisAgent->alpha_mem_id_counter               = 0;
    thisAgent->backtrace_number                   = 0;
    thisAgent->beta_node_id_counter               = 0;
    thisAgent->bottom_goal                        = NIL;
    thisAgent->changed_slots                      = NIL;
    thisAgent->chunk_count                        = 1;
    thisAgent->chunk_free_problem_spaces          = NIL;
    thisAgent->chunky_problem_spaces              = NIL;  /* AGR MVL1 */
    strcpy(thisAgent->chunk_name_prefix, "chunk"); /* ajc (5/14/02) */
    thisAgent->context_slots_with_changed_acceptable_preferences = NIL;
    thisAgent->current_phase                      = INPUT_PHASE;
    thisAgent->applyPhase                         = false;
    thisAgent->current_symbol_hash_id             = 0;
    thisAgent->current_variable_gensym_number     = 0;
    thisAgent->current_wme_timetag                = 1;
    thisAgent->default_wme_depth                  = 1;  /* AGR 646 */
    thisAgent->disconnected_ids                   = NIL;
    thisAgent->existing_output_links              = NIL;
    thisAgent->output_link_changed                = false;  /* KJC 11/9/98 */
    /* thisAgent->explain_flag                       = false; */
    thisAgent->go_number                          = 1;
    thisAgent->go_type                            = GO_DECISION;
    thisAgent->init_count                         = 0;
    thisAgent->rl_init_count                      = 0;
    thisAgent->grounds_tc                         = 0;
    thisAgent->highest_goal_whose_context_changed = NIL;
    thisAgent->ids_with_unknown_level             = NIL;
    thisAgent->input_period                       = 0;     /* AGR REW1 */
    thisAgent->input_cycle_flag                   = true;  /* AGR REW1 */
    thisAgent->justification_count                = 1;
    thisAgent->link_update_mode                   = UPDATE_LINKS_NORMALLY;
    thisAgent->locals_tc                          = 0;
    thisAgent->max_chunks_reached                 = false; /* MVP 6-24-94 */
    thisAgent->mcs_counter                        = 1;
    thisAgent->ms_assertions                      = NIL;
    thisAgent->ms_retractions                     = NIL;
    thisAgent->num_existing_wmes                  = 0;
    thisAgent->num_wmes_in_rete                   = 0;
    thisAgent->potentials_tc                      = 0;
    thisAgent->prev_top_state                     = NIL;
    thisAgent->production_being_fired             = NIL;
    thisAgent->productions_being_traced           = NIL;
    thisAgent->promoted_ids                       = NIL;
    thisAgent->reason_for_stopping                = "Startup";
    thisAgent->slots_for_possible_removal         = NIL;
    thisAgent->stop_soar                          = true;
    thisAgent->system_halted                      = false;
    thisAgent->token_additions                    = 0;
    thisAgent->top_goal                           = NIL;
    thisAgent->top_state                          = NIL;
    thisAgent->wmes_to_add                        = NIL;
    thisAgent->wmes_to_remove                     = NIL;
    thisAgent->wme_filter_list                    = NIL;   /* Added this to avoid
                                                                access violation
                                                                -AJC (5/13/02) */
    thisAgent->multi_attributes                   = NIL;

    /* REW: begin 09.15.96 */

    thisAgent->did_PE                             = false;
    thisAgent->soar_verbose_flag                  = false;
    thisAgent->FIRING_TYPE                        = IE_PRODS;
    thisAgent->ms_o_assertions                    = NIL;
    thisAgent->ms_i_assertions                    = NIL;

    /* REW: end   09.15.96 */

    thisAgent->postponed_assertions              = NIL;

    /* REW: begin 08.20.97 */
    thisAgent->active_goal                        = NIL;
    thisAgent->active_level                       = 0;
    thisAgent->previous_active_level              = 0;

    /* Initialize Waterfall-specific lists */
    thisAgent->nil_goal_retractions               = NIL;
    /* REW: end   08.20.97 */

    /* REW: begin 10.24.97 */
    thisAgent->waitsnc                            = false;
    thisAgent->waitsnc_detect                     = false;
    /* REW: end   10.24.97 */

    /* Initializing rete stuff */
    for (int i = 0; i < 256; i++)
    {
        thisAgent->actual[i] = 0;
        thisAgent->if_no_merging[i] = 0;
        thisAgent->if_no_sharing[i] = 0;
    }

    reset_max_stats(thisAgent);

    thisAgent->real_time_tracker = 0;
    thisAgent->attention_lapse_tracker = 0;

    if (!getcwd(cur_path, MAXPATHLEN))
    {
		char* error = strerror(errno);
        print(thisAgent, "Unable to set current directory while initializing agent: %s\n", error);
    }

    for (int productionTypeCounter = 0; productionTypeCounter < NUM_PRODUCTION_TYPES; productionTypeCounter++)
    {
        thisAgent->all_productions_of_type[productionTypeCounter] = NIL;
        thisAgent->num_productions_of_type[productionTypeCounter] = 0;
    }

    thisAgent->o_support_calculation_type = 4; /* KJC 7/00 */ // changed from 3 to 4 by voigtjr  (/* bugzilla bug 339 */)
    thisAgent->numeric_indifferent_mode = NUMERIC_INDIFFERENT_MODE_SUM;

    thisAgent->rhs_functions = NIL;

    // JRV: Allocates data for XML generation
    xml_create(thisAgent);

    soar_init_callbacks(thisAgent);

    //
    thisAgent->memoryManager = &Memory_Manager::Get_MPM();
    init_memory_utilities(thisAgent);

    //
    // This was moved here so that system parameters could
    // be set before the agent was initialized.
    init_sysparams(thisAgent);
    thisAgent->parser_syms = NIL;
    thisAgent->ebcManager = new EBC_Manager(thisAgent);
    thisAgent->outputManager = &Output_Manager::Get_OM();

    /* Initializing all the timer structures */
    // Timers must be initialized after sysparams
#ifndef NO_TIMING_STUFF
    thisAgent->timers_cpu.set_enabled(&(thisAgent->sysparams[TIMERS_ENABLED]));
    thisAgent->timers_kernel.set_enabled(&(thisAgent->sysparams[TIMERS_ENABLED]));
    thisAgent->timers_phase.set_enabled(&(thisAgent->sysparams[TIMERS_ENABLED]));
#ifdef DETAILED_TIMING_STATS
    thisAgent->timers_gds.set_enabled(&(thisAgent->sysparams[TIMERS_ENABLED]));
#endif
    reset_timers(thisAgent);
#endif

    // dynamic counters
    thisAgent->dyn_counters = new std::unordered_map< std::string, uint64_t >();

    // exploration initialization
    thisAgent->exploration_params[ EXPLORATION_PARAM_EPSILON ] = exploration_add_parameter(0.1, &exploration_validate_epsilon, "epsilon");
    thisAgent->exploration_params[ EXPLORATION_PARAM_TEMPERATURE ] = exploration_add_parameter(25, &exploration_validate_temperature, "temperature");

    // rl initialization
    thisAgent->rl_params = new rl_param_container(thisAgent);
    thisAgent->rl_stats = new rl_stat_container(thisAgent);
    thisAgent->rl_prods = new rl_production_memory();

    rl_initialize_template_tracking(thisAgent);

    // select initialization
    thisAgent->select = new select_info;
    select_init(thisAgent);


    // predict initialization
    thisAgent->prediction = new std::string();
    predict_init(thisAgent);


    // wma initialization
    thisAgent->wma_params = new wma_param_container(thisAgent);
    thisAgent->wma_stats = new wma_stat_container(thisAgent);
    thisAgent->wma_timers = new wma_timer_container(thisAgent);

#ifdef USE_MEM_POOL_ALLOCATORS
    thisAgent->wma_forget_pq = new wma_forget_p_queue(std::less< wma_d_cycle >(), soar_module::soar_memory_pool_allocator< std::pair< wma_d_cycle, wma_decay_set* > >());
    thisAgent->wma_touched_elements = new wma_pooled_wme_set(std::less< wme* >(), soar_module::soar_memory_pool_allocator< wme* >(thisAgent));
    thisAgent->wma_touched_sets = new wma_decay_cycle_set(std::less< wma_d_cycle >(), soar_module::soar_memory_pool_allocator< wma_d_cycle >(thisAgent));
#else
    thisAgent->wma_forget_pq = new wma_forget_p_queue();
    thisAgent->wma_touched_elements = new wma_pooled_wme_set();
    thisAgent->wma_touched_sets = new wma_decay_cycle_set();
#endif
    thisAgent->wma_initialized = false;
    thisAgent->wma_tc_counter = 2;


    // epmem initialization
    thisAgent->epmem_params = new epmem_param_container(thisAgent);
    thisAgent->epmem_stats = new epmem_stat_container(thisAgent);
    thisAgent->epmem_timers = new epmem_timer_container(thisAgent);

    thisAgent->epmem_db = new soar_module::sqlite_database();
    thisAgent->epmem_stmts_common = NULL;
    thisAgent->epmem_stmts_graph = NULL;

    thisAgent->epmem_node_mins = new std::vector<epmem_time_id>();
    thisAgent->epmem_node_maxes = new std::vector<bool>();

    thisAgent->epmem_edge_mins = new std::vector<epmem_time_id>();
    thisAgent->epmem_edge_maxes = new std::vector<bool>();
    thisAgent->epmem_id_repository = new epmem_parent_id_pool();
    thisAgent->epmem_id_replacement = new epmem_return_id_pool();
    thisAgent->epmem_id_ref_counts = new epmem_id_ref_counter();

    // debug module parameters
    thisAgent->debug_params = new debug_param_container(thisAgent);

#ifdef USE_MEM_POOL_ALLOCATORS
    thisAgent->epmem_node_removals = new epmem_id_removal_map(std::less< epmem_node_id >(), soar_module::soar_memory_pool_allocator< std::pair< epmem_node_id, bool > >(thisAgent));
    thisAgent->epmem_edge_removals = new epmem_id_removal_map(std::less< epmem_node_id >(), soar_module::soar_memory_pool_allocator< std::pair< epmem_node_id, bool > >(thisAgent));

    thisAgent->epmem_wme_adds = new epmem_symbol_set(std::less< Symbol* >(), soar_module::soar_memory_pool_allocator< Symbol* >(thisAgent));
    thisAgent->epmem_promotions = new epmem_symbol_set(std::less< Symbol* >(), soar_module::soar_memory_pool_allocator< Symbol* >(thisAgent));

    thisAgent->epmem_id_removes = new epmem_symbol_stack(soar_module::soar_memory_pool_allocator< Symbol* >(thisAgent));
#else
    thisAgent->epmem_node_removals = new epmem_id_removal_map();
    thisAgent->epmem_edge_removals = new epmem_id_removal_map();

    thisAgent->epmem_wme_adds = new epmem_symbol_set();
    thisAgent->epmem_promotions = new epmem_symbol_set();

    thisAgent->epmem_id_removes = new epmem_symbol_stack();
#endif

    thisAgent->epmem_validation = 0;

    // smem initialization
    thisAgent->smem_params = new smem_param_container(thisAgent);
    thisAgent->smem_stats = new smem_stat_container(thisAgent);
    thisAgent->smem_timers = new smem_timer_container(thisAgent);

    thisAgent->smem_db = new soar_module::sqlite_database();

    thisAgent->smem_validation = 0;

#ifdef USE_MEM_POOL_ALLOCATORS
    thisAgent->smem_changed_ids = new smem_pooled_symbol_set(std::less< Symbol* >(), soar_module::soar_memory_pool_allocator< Symbol* >(thisAgent));
#else
    thisAgent->smem_changed_ids = new smem_pooled_symbol_set();
#endif
    thisAgent->smem_ignore_changes = false;
    thisAgent->lastCue = NULL;

    // statistics initialization
    thisAgent->dc_stat_tracking = false;
    thisAgent->stats_db = new soar_module::sqlite_database();

    thisAgent->substate_break_level = 0;

    return thisAgent;
}

/*
===============================

===============================
*/
void destroy_soar_agent(agent* delete_agent)
{
    // cleanup exploration
    for (int i = 0; i < EXPLORATION_PARAMS; i++)
    {
        delete delete_agent->exploration_params[ i ];
    }

    // cleanup Soar-RL
    delete_agent->rl_params->apoptosis->set_value(rl_param_container::apoptosis_none);
    delete delete_agent->rl_prods;
    delete delete_agent->rl_params;
    delete delete_agent->rl_stats;
    delete_agent->rl_params = NULL; // apoptosis needs to know this for excise_all_productions below

    // cleanup select
    select_init(delete_agent);
    delete delete_agent->select;

    // cleanup predict
    delete delete_agent->prediction;

    // cleanup wma
    delete_agent->wma_params->activation->set_value(off);
    delete delete_agent->wma_forget_pq;
    delete delete_agent->wma_touched_elements;
    delete delete_agent->wma_touched_sets;
    delete delete_agent->wma_params;
    delete delete_agent->wma_stats;
    delete delete_agent->wma_timers;

    // cleanup epmem
    epmem_close(delete_agent);
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
    delete delete_agent->epmem_id_removes;

    delete delete_agent->epmem_wme_adds;
    delete delete_agent->epmem_promotions;

    delete delete_agent->epmem_db;


    // cleanup smem
    smem_close(delete_agent);
    delete delete_agent->smem_changed_ids;
    delete delete_agent->smem_params;
    delete delete_agent->smem_stats;
    delete delete_agent->smem_timers;

    delete delete_agent->smem_db;

#ifndef NO_SVS
    delete delete_agent->svs;
#endif

    // cleanup statistics db
    stats_close(delete_agent);
    delete delete_agent->stats_db;
    delete_agent->stats_db = 0;

    delete delete_agent->debug_params;
    delete delete_agent->output_settings;

    /////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////

    remove_built_in_rhs_functions(delete_agent);

    getSoarInstance()->Delete_Agent(delete_agent->name);

    /* Free structures stored in agent structure */
    free(delete_agent->name);

    /* Freeing the agent's multi attributes structure */
    multi_attribute* lastmattr = 0;
    for (multi_attribute* curmattr = delete_agent->multi_attributes;
            curmattr != 0;
            curmattr = curmattr->next)
    {

        symbol_remove_ref(delete_agent, curmattr->symbol);

        delete_agent->memoryManager->free_memory(lastmattr, MISCELLANEOUS_MEM_USAGE);
        lastmattr = curmattr;
    }
    delete_agent->memoryManager->free_memory(lastmattr, MISCELLANEOUS_MEM_USAGE);

    /* Freeing all the productions owned by this agent */
    excise_all_productions(delete_agent, false);

    /* Releasing all the predefined symbols */
    release_predefined_symbols(delete_agent);
    //deallocate_symbol_list_removing_references(delete_agent, delete_agent->parser_syms);

    /* Releasing rete stuff RPM 11/06 */
    delete_agent->memoryManager->free_with_pool(MP_rete_node, delete_agent->dummy_top_node);
    delete_agent->memoryManager->free_with_pool(MP_token, delete_agent->dummy_top_token);

    /* Cleaning up the various callbacks
       TODO: Not clear why callbacks need to take the agent pointer essentially twice.
    */
    soar_remove_all_monitorable_callbacks(delete_agent);

    /* RPM 9/06 begin */

    delete_agent->memoryManager->free_memory(delete_agent->left_ht, HASH_TABLE_MEM_USAGE);
    delete_agent->memoryManager->free_memory(delete_agent->right_ht, HASH_TABLE_MEM_USAGE);
    delete_agent->memoryManager->free_memory(delete_agent->rhs_variable_bindings, MISCELLANEOUS_MEM_USAGE);

    /* Releasing trace formats (needs to happen before tracing hashtables are released) */
    remove_trace_format(delete_agent, false, FOR_ANYTHING_TF, NIL);
    remove_trace_format(delete_agent, false, FOR_STATES_TF, NIL);
    Symbol* evaluate_object_sym = find_str_constant(delete_agent, "evaluate-object");
    remove_trace_format(delete_agent, false, FOR_OPERATORS_TF, evaluate_object_sym);
    remove_trace_format(delete_agent, true, FOR_STATES_TF, NIL);
    remove_trace_format(delete_agent, true, FOR_OPERATORS_TF, NIL);

    dprint_identifiers(DT_ID_LEAKING);

    // delete unique varname lookup table
    delete delete_agent->ebcManager;

    /* Releasing hashtables allocated in init_tracing */
    for (int i = 0; i < 3; i++)
    {
        free_hash_table(delete_agent, delete_agent->object_tr_ht[i]);
        free_hash_table(delete_agent, delete_agent->stack_tr_ht[i]);
    }

    /* Releasing memory allocated in init_rete */
    for (int i = 0; i < 16; i++)
    {
        free_hash_table(delete_agent, delete_agent->alpha_hash_tables[i]);
    }

    /* Releasing other hashtables */
    free_hash_table(delete_agent, delete_agent->variable_hash_table);
    free_hash_table(delete_agent, delete_agent->identifier_hash_table);
    free_hash_table(delete_agent, delete_agent->str_constant_hash_table);
    free_hash_table(delete_agent, delete_agent->int_constant_hash_table);
    free_hash_table(delete_agent, delete_agent->float_constant_hash_table);

    delete delete_agent->dyn_counters;

    // JRV: Frees data used by XML generation
    xml_destroy(delete_agent);

    /* Free soar agent structure */
    delete delete_agent;
}
