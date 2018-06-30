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

#include "agent.h"

#include "action_record.h"
#include "callback.h"
#include "cmd_settings.h"
#include "condition_record.h"
#include "decide.h"
#include "decider.h"
#include "decision_manipulation.h"
#include "dprint.h"
#include "ebc.h"
#include "ebc_identity.h"
#include "ebc_repair.h"
#include "episodic_memory.h"
#include "explanation_memory.h"
#include "exploration.h"
#include "instantiation.h"
#include "instantiation_record.h"
#include "io_link.h"
#include "lexer.h"
#include "mem.h"
#include "memory_manager.h"
#include "output_manager.h"
#include "parser.h"
#include "print.h"
#include "preference.h"
#include "production.h"
#include "production_record.h"
#include "instantiation.h"
#include "reinforcement_learning.h"
#include "rete.h"
#include "rhs.h"
#include "rhs_functions.h"
#include "run_soar.h"
#include "semantic_memory.h"
#include "smem_settings.h"
#include "smem_stats.h"
#include "smem_structs.h"
#include "soar_instance.h"
#include "soar_module.h"
#include "stats.h"
#include "symbol.h"
#include "trace.h"
#include "working_memory_activation.h"
#include "working_memory.h"
#include "xml.h"
#include "visualize.h"

#ifndef NO_SVS
#include "svs_interface.h"
#endif

#include <stdlib.h>
#include <map>

/* ===================================================================

                           Initialization Function

=================================================================== */
void init_soar_agent(agent* thisAgent)
{

    thisAgent->rhs_functions = NIL;

    /* --- initialize everything --- */
    init_production_utilities(thisAgent);
    init_built_in_rhs_functions(thisAgent);
    init_rete(thisAgent);
    init_instantiation_pool(thisAgent);
    init_decider(thisAgent);
    init_soar_io(thisAgent);
    init_tracing(thisAgent);
    select_init(thisAgent);
    predict_init(thisAgent);

    thisAgent->memoryManager->init_memory_pool(MP_chunk_cond, sizeof(chunk_cond), "chunk_condition");
    thisAgent->memoryManager->init_memory_pool(MP_constraints, sizeof(constraint_struct), "constraints");
    thisAgent->memoryManager->init_memory_pool(MP_sym_triple, sizeof(symbol_triple), "symbol_triple");
    thisAgent->memoryManager->init_memory_pool(MP_identity_mapping, sizeof(identity_mapping), "id_mapping");
    thisAgent->memoryManager->init_memory_pool(MP_chunk_element, sizeof(chunk_element), "chunk_element");
    thisAgent->memoryManager->init_memory_pool(MP_identity_sets, sizeof(Identity), "identities");

    thisAgent->memoryManager->init_memory_pool(MP_action_record, sizeof(action_record), "action_record");
    thisAgent->memoryManager->init_memory_pool(MP_condition_record, sizeof(condition_record), "cond_record");
    thisAgent->memoryManager->init_memory_pool(MP_instantiation_record, sizeof(instantiation_record), "inst_record");
    thisAgent->memoryManager->init_memory_pool(MP_chunk_record, sizeof(chunk_record), "chunk_record");
    thisAgent->memoryManager->init_memory_pool(MP_production_record, sizeof(production_record), "prod_record");
    thisAgent->memoryManager->init_memory_pool(MP_repair_path, sizeof(Repair_Path), "repair_paths");

    thisAgent->memoryManager->init_memory_pool(MP_gds, sizeof(goal_dependency_set), "gds");

    thisAgent->memoryManager->init_memory_pool(MP_rl_info, sizeof(rl_data), "rl_id_data");
    thisAgent->memoryManager->init_memory_pool(MP_rl_et, sizeof(rl_et_map), "rl_et");
    thisAgent->memoryManager->init_memory_pool(MP_rl_rule, sizeof(production_list), "rl_rules");

    thisAgent->memoryManager->init_memory_pool(MP_wma_decay_element, sizeof(wma_decay_element), "wma_decay");
    thisAgent->memoryManager->init_memory_pool(MP_wma_decay_set, sizeof(wma_decay_set), "wma_decay_set");
    thisAgent->memoryManager->init_memory_pool(MP_wma_wme_oset, sizeof(wme_set), "wma_oset");
    thisAgent->memoryManager->init_memory_pool(MP_wma_slot_refs, sizeof(wma_sym_reference_map), "wma_slot_ref");

    thisAgent->memoryManager->init_memory_pool(MP_smem_wmes, sizeof(preference_list), "smem_wmes");
    thisAgent->memoryManager->init_memory_pool(MP_smem_info, sizeof(smem_data), "smem_id_data");

    thisAgent->memoryManager->init_memory_pool(MP_epmem_wmes, sizeof(preference_list), "epmem_wmes");
    thisAgent->memoryManager->init_memory_pool(MP_epmem_info, sizeof(epmem_data), "epmem_id_data");
    thisAgent->memoryManager->init_memory_pool(MP_epmem_literal, sizeof(epmem_literal), "epmem_literals");
    thisAgent->memoryManager->init_memory_pool(MP_epmem_pedge, sizeof(epmem_pedge), "epmem_pedges");
    thisAgent->memoryManager->init_memory_pool(MP_epmem_uedge, sizeof(epmem_uedge), "epmem_uedges");
    thisAgent->memoryManager->init_memory_pool(MP_epmem_interval, sizeof(epmem_interval), "epmem_intervals");

    thisAgent->EpMem->epmem_params->exclusions->set_value("epmem");
    thisAgent->EpMem->epmem_params->exclusions->set_value("smem");

    /* --- add default object trace formats --- */
    add_trace_format(thisAgent, false, FOR_ANYTHING_TF, NIL,
                     "%id %ifdef[(%v[name])]");
    add_trace_format(thisAgent, false, FOR_STATES_TF, NIL,
                     "%id %ifdef[(%v[attribute] %v[impasse])]");
    {
        Symbol* evaluate_object_sym;
        evaluate_object_sym = thisAgent->symbolManager->make_str_constant("evaluate-object");
        add_trace_format(thisAgent, false, FOR_OPERATORS_TF, evaluate_object_sym,
                         "%id (evaluate-object %o[object])");
        thisAgent->symbolManager->symbol_remove_ref(&evaluate_object_sym);
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
    char cur_path[MAXPATHLEN];

    agent* thisAgent = new agent();
    thisAgent->name                                     = savestring(agent_name);
    thisAgent->output_settings                          = new AgentOutput_Info();

    thisAgent->newly_created_instantiations             = NULL;
    thisAgent->current_tc_number       = 0;
    thisAgent->all_wmes_in_rete                         = NIL;
    thisAgent->alpha_mem_id_counter                     = 0;
    thisAgent->beta_node_id_counter                     = 0;
    thisAgent->bottom_goal                              = NIL;
    thisAgent->changed_slots                            = NIL;
    thisAgent->context_slots_with_changed_accept_prefs  = NIL;
    thisAgent->current_phase                            = INPUT_PHASE;
    thisAgent->applyPhase                               = false;
    thisAgent->current_wme_timetag                      = 1;
    thisAgent->disconnected_ids                         = NIL;
    thisAgent->existing_output_links                    = NIL;
    thisAgent->output_link_changed                      = false;
    thisAgent->go_number                                = 1;
    thisAgent->go_type                                  = GO_DECISION;
    thisAgent->init_count                               = 0;
    thisAgent->highest_goal_whose_context_changed       = NIL;
    thisAgent->ids_with_unknown_level                   = NIL;
    thisAgent->input_period                             = 0;
    thisAgent->input_cycle_flag                         = true;
    thisAgent->link_update_mode                         = UPDATE_LINKS_NORMALLY;
    thisAgent->mcs_counter                              = 1;
    thisAgent->ms_assertions                            = NIL;
    thisAgent->ms_retractions                           = NIL;
    thisAgent->num_existing_wmes                        = 0;
    thisAgent->num_wmes_in_rete                         = 0;
    thisAgent->prev_top_state                           = NIL;
    thisAgent->production_being_fired                   = NIL;
    thisAgent->productions_being_traced                 = NIL;
    thisAgent->promoted_ids                             = NIL;
    thisAgent->reason_for_stopping                      = "Startup";
    thisAgent->slots_for_possible_removal               = NIL;
    thisAgent->stop_soar                                = true;
    thisAgent->system_halted                            = false;
    thisAgent->token_additions                          = 0;
    thisAgent->top_goal                                 = NIL;
    thisAgent->top_state                                = NIL;
    thisAgent->wmes_to_add                              = NIL;
    thisAgent->wmes_to_remove                           = NIL;
    thisAgent->wme_filter_list                          = NIL;
    thisAgent->multi_attributes                         = NIL;

    thisAgent->did_PE                                   = false;
    thisAgent->FIRING_TYPE                              = IE_PRODS;
    thisAgent->ms_o_assertions                          = NIL;
    thisAgent->ms_i_assertions                          = NIL;

    thisAgent->postponed_assertions                     = NIL;

    thisAgent->active_goal                              = NIL;
    thisAgent->active_level                             = 0;
    thisAgent->previous_active_level                    = 0;

    thisAgent->nil_goal_retractions                     = NIL;

    /* Initializing rete stuff */
    for (int i = 0; i < 256; i++)
    {
        thisAgent->actual[i] = 0;
        thisAgent->if_no_merging[i] = 0;
        thisAgent->if_no_sharing[i] = 0;
    }

    reset_max_stats(thisAgent);

    if (!getcwd(cur_path, MAXPATHLEN))
    {
		char* error = strerror(errno);
        thisAgent->outputManager->printa_sf(thisAgent, "Unable to set current directory while initializing agent: %s\n", error);
    }

    for (int productionTypeCounter = 0; productionTypeCounter < NUM_PRODUCTION_TYPES; productionTypeCounter++)
    {
        thisAgent->all_productions_of_type[productionTypeCounter] = NIL;
        thisAgent->num_productions_of_type[productionTypeCounter] = 0;
    }

    thisAgent->numeric_indifferent_mode = NUMERIC_INDIFFERENT_MODE_SUM;

    thisAgent->rhs_functions = NIL;

    // Allocate data for XML generation
    xml_create(thisAgent);

    soar_init_callbacks(thisAgent);

    //
    thisAgent->memoryManager = &Memory_Manager::Get_MPM();
    init_memory_utilities(thisAgent);

    //
    // This was moved here so that system parameters could
    // be set before the agent was initialized.
    init_trace_settings(thisAgent);

    /* Initializing all the timer structures.  Must be initialized after sysparams */
    thisAgent->timers_enabled = true;

#ifndef NO_TIMING_STUFF
    thisAgent->timers_cpu.set_enabled(&(thisAgent->timers_enabled));
    thisAgent->timers_kernel.set_enabled(&(thisAgent->timers_enabled));
    thisAgent->timers_phase.set_enabled(&(thisAgent->timers_enabled));
#ifdef DETAILED_TIMING_STATS
    thisAgent->timers_gds.set_enabled(&(thisAgent->timers_enabled));
#endif
    reset_timers(thisAgent);
#endif

    // dynamic counters
    thisAgent->dyn_counters = new std::unordered_map< std::string, uint64_t >();

    thisAgent->outputManager = &Output_Manager::Get_OM();
    thisAgent->command_params = new cli_command_params(thisAgent);
    thisAgent->EpMem = new EpMem_Manager(thisAgent);
    thisAgent->SMem = new SMem_Manager(thisAgent);
    thisAgent->symbolManager = new Symbol_Manager(thisAgent);
    thisAgent->explanationBasedChunker = new Explanation_Based_Chunker(thisAgent);
    thisAgent->explanationMemory = new Explanation_Memory(thisAgent);
    thisAgent->visualizationManager = new GraphViz_Visualizer(thisAgent);
    thisAgent->RL = new RL_Manager(thisAgent);
    thisAgent->WM = new WM_Manager(thisAgent);
    thisAgent->Decider = new SoarDecider(thisAgent);

    /* Something used for one of Alex's unit tests.  Should remove. */
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
    delete delete_agent->visualizationManager;
    delete delete_agent->explanationBasedChunker;
    delete_agent->explanationBasedChunker = NULL;
    delete_agent->visualizationManager = NULL;
    #ifndef NO_SVS
        delete delete_agent->svs;
        delete_agent->svs = NULL;
    #endif

    delete_agent->RL->clean_up_for_agent_deletion();
    delete_agent->WM->clean_up_for_agent_deletion();
    delete_agent->EpMem->clean_up_for_agent_deletion();
    delete_agent->SMem->clean_up_for_agent_deletion();
    delete_agent->Decider->clean_up_for_agent_deletion();

    delete delete_agent->command_params;
    delete_agent->command_params = NULL;

    stats_close(delete_agent);
    delete delete_agent->stats_db;
    delete_agent->stats_db = NULL;

    remove_built_in_rhs_functions(delete_agent);
    getSoarInstance()->Delete_Agent(delete_agent->name);
    free(delete_agent->name);

    multi_attribute* lastmattr = 0;
    for (multi_attribute* curmattr = delete_agent->multi_attributes; curmattr != 0; curmattr = curmattr->next) {
        delete_agent->symbolManager->symbol_remove_ref(&(curmattr->symbol));
        delete_agent->memoryManager->free_memory(lastmattr, MISCELLANEOUS_MEM_USAGE);
        lastmattr = curmattr;
    }
    delete_agent->memoryManager->free_memory(lastmattr, MISCELLANEOUS_MEM_USAGE);

    excise_all_productions(delete_agent, false);

    /* We can't clean up the explanation manager until after production excision */
    delete delete_agent->explanationMemory;
    delete_agent->explanationMemory = NULL;

    delete_agent->symbolManager->release_predefined_symbols();
    delete_agent->symbolManager->release_common_variables_and_numbers();

    delete_agent->memoryManager->free_with_pool(MP_rete_node, delete_agent->dummy_top_node);
    delete_agent->memoryManager->free_with_pool(MP_token, delete_agent->dummy_top_token);

    soar_remove_all_monitorable_callbacks(delete_agent);

    delete_agent->memoryManager->free_memory(delete_agent->left_ht, HASH_TABLE_MEM_USAGE);
    delete_agent->memoryManager->free_memory(delete_agent->right_ht, HASH_TABLE_MEM_USAGE);
    delete_agent->memoryManager->free_memory(delete_agent->rhs_variable_bindings, MISCELLANEOUS_MEM_USAGE);

    /* Releasing trace formats (needs to happen before tracing hashtables are released) */
    remove_trace_format(delete_agent, false, FOR_ANYTHING_TF, NIL);
    remove_trace_format(delete_agent, false, FOR_STATES_TF, NIL);
    Symbol* evaluate_object_sym = delete_agent->symbolManager->find_str_constant("evaluate-object");
    remove_trace_format(delete_agent, false, FOR_OPERATORS_TF, evaluate_object_sym);
    remove_trace_format(delete_agent, true, FOR_STATES_TF, NIL);
    remove_trace_format(delete_agent, true, FOR_OPERATORS_TF, NIL);

    delete delete_agent->output_settings;
    delete_agent->output_settings = NULL;

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

    /* Release module managers */
    delete delete_agent->WM;
    delete delete_agent->Decider;
    delete delete_agent->RL;
    delete delete_agent->EpMem;
    delete delete_agent->SMem;
    delete delete_agent->symbolManager;


    delete delete_agent->dyn_counters;

    /* Release data used by XML generation */
    xml_destroy(delete_agent);

    /* Release agent data structure */
    delete delete_agent;
}

void reinitialize_agent(agent* thisAgent)
{
    /* Clean up explanation-based chunking, episodic and semantic memory data structures */
    epmem_reinit(thisAgent);
    thisAgent->SMem->reinit();
    thisAgent->explanationBasedChunker->reinit();

    /* Turn off WM activation and RL forgetting temporarily while clearing the goal stack */
    bool wma_was_enabled = wma_enabled(thisAgent);
    thisAgent->WM->wma_params->activation->set_value(off);
    rl_param_container::apoptosis_choices rl_apoptosis = thisAgent->RL->rl_params->apoptosis->get_value();
    thisAgent->RL->rl_params->apoptosis->set_value(rl_param_container::apoptosis_none);

    /* Remove all states and report to input/output functions */
    clear_goal_stack(thisAgent);

    /* Re-enable WM activation and RL forgetting */
    if (wma_was_enabled) thisAgent->WM->wma_params->activation->set_value(on);
    thisAgent->RL->rl_params->apoptosis->set_value(rl_apoptosis);

    /* Clear stats for WM, EpMem, SMem and RL */
    thisAgent->RL->rl_stats->reset();
    thisAgent->WM->wma_stats->reset();
    thisAgent->EpMem->epmem_stats->reset();
    thisAgent->SMem->reset_stats();
    thisAgent->dyn_counters->clear();

    /* Signal that everything should be retracted and allow all i-instantiations to retract */
    thisAgent->active_level = 0;
    thisAgent->FIRING_TYPE = IE_PRODS;
    do_preference_phase(thisAgent);

    /* It's now safe to clear out explanation memory */
    thisAgent->explanationMemory->re_init();

    /* Reset Soar identifier hash table and counters for WMEs, SMem and Soar IDs.
     * Note:  reset_hash_table() is where refcount leaks in identifiers are detected. */
    reset_wme_timetags(thisAgent);
    thisAgent->symbolManager->reset_hash_table(MP_identifier);
    thisAgent->symbolManager->reset_id_counters();
    if (thisAgent->SMem->connected()) thisAgent->SMem->reset_id_counters();

    /* Reset basic Soar counters and pending XML trace/commands */
    reset_statistics(thisAgent);
    xml_reset(thisAgent);
}

cli_command_params::cli_command_params(agent* thisAgent)
{
    decide_params = new decide_param_container(thisAgent);
    load_params = new load_param_container(thisAgent);
    production_params = new production_param_container(thisAgent);
    save_params = new save_param_container(thisAgent);
    wm_params = new wm_param_container(thisAgent);
}

cli_command_params::~cli_command_params()
{
    delete decide_params;
    delete load_params;
    delete production_params;
    delete save_params;
    delete wm_params;
}
