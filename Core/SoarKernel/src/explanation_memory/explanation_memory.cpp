#include "explanation_memory.h"

#include "action_record.h"
#include "agent.h"
#include "condition_record.h"
#include "condition.h"
#include "ebc.h"
#include "ebc_identity.h"
#include "instantiation_record.h"
#include "instantiation.h"
#include "memory_manager.h"
#include "output_manager.h"
#include "preference.h"
#include "production_record.h"
#include "production.h"
#include "rhs.h"
#include "symbol.h"
#include "symbol_manager.h"
#include "test.h"
#include "working_memory.h"
#include "visualize.h"

/* This crashes in count-and-die if depth is around 1000 (Macbook Pro 2012, 8MB) */
#define EXPLAIN_MAX_BT_DEPTH 900

Explanation_Memory::Explanation_Memory(agent* myAgent)
{
    thisAgent = myAgent;
    outputManager = &Output_Manager::Get_OM();

    settings = new Explainer_Parameters(thisAgent);

    current_recording_chunk = NULL;
    current_discussed_chunk = NULL;
    initialize_counters();
    m_all_enabled = false;
    m_justifications_enabled = false;
    num_rules_watched = 0;

    print_explanation_trace = true;
    last_printed_id = 0;

    /* Create data structures used for EBC */
    all_actions = new action_record_map();
    all_conditions = new condition_record_map();
    all_identities_in_goal = new sym_to_identity_set_map();
    cached_production = new production_record_set();
    chunks = new chunk_record_symbol_map();
    chunks_by_ID = new chunk_record_id_map();
    instantiations = new instantiation_record_map();
    production_id_map = new production_map();
}

Explanation_Memory::~Explanation_Memory()
{
    current_recording_chunk = NULL;
    current_discussed_chunk = NULL;
    clear_explanations();
    clear_identity_sets();

    delete all_actions;
    delete all_conditions;
    delete all_identities_in_goal;
    delete cached_production;
    delete chunks;
    delete chunks_by_ID;
    delete instantiations;
    delete production_id_map;
    delete settings;
}

void Explanation_Memory::initialize_counters()
{
    chunk_id_count = 1;
    condition_id_count = 0;
    action_id_count = 0;

    stats.duplicates = 0;
    stats.no_grounds = 0;
    stats.max_chunks = 0;
    stats.max_dupes = 0;
    stats.tested_local_negation = 0;
    stats.tested_quiescence = 0;
    stats.tested_ltm_recall = 0;
    stats.tested_local_negation_just = 0;
    stats.tested_ltm_recall_just = 0;
    stats.merged_conditions = 0;
    stats.merged_disjunctions = 0;
    stats.chunks_attempted = 0;
    stats.chunks_succeeded = 0;
    stats.justifications_succeeded = 0;
    stats.instantations_backtraced = 0;
    stats.constraints_attached = 0;
    stats.constraints_collected = 0;
    stats.chunks_repaired = 0;
    stats.identities_created                = 0;
    stats.identities_joined                 = 0;
    stats.identities_literalized            = 0;
    stats.identities_participated           = 0;
    stats.identity_propagations             = 0;
    stats.identity_propagations_blocked     = 0;
    stats.operational_constraints           = 0;
}

void Explanation_Memory::clear_explanations()
{
    Symbol* lSym;
    for (auto it = (*chunks).begin(); it != (*chunks).end(); ++it)
    {
        lSym = it->first;
        thisAgent->symbolManager->symbol_remove_ref(&lSym);
        it->second->clean_up();
        thisAgent->memoryManager->free_with_pool(MP_chunk_record, it->second);
    }
    chunks->clear();
    chunks_by_ID->clear();

    for (auto it = (*instantiations).begin(); it != (*instantiations).end(); ++it)
    {
        it->second->clean_up();
        thisAgent->memoryManager->free_with_pool(MP_instantiation_record, it->second);
    }
    instantiations->clear();

    for (auto it = (*all_conditions).begin(); it != (*all_conditions).end(); ++it)
    {
        it->second->clean_up();
        thisAgent->memoryManager->free_with_pool(MP_condition_record, it->second);
    }
    all_conditions->clear();

    for (auto it = (*all_actions).begin(); it != (*all_actions).end(); ++it)
    {
        it->second->clean_up();
        thisAgent->memoryManager->free_with_pool(MP_action_record, static_cast<action_record *>(it->second));
    }
    all_actions->clear();

    for (auto it = (*cached_production).begin(); it != (*cached_production).end(); ++it)
    {
        (*it)->clean_up();
        thisAgent->memoryManager->free_with_pool(MP_production_record, (*it));
    }

    cached_production->clear();
    production_id_map->clear();
}

void Explanation_Memory::re_init()
{
    clear_explanations();
    clear_identity_sets();
    initialize_counters();
    current_recording_chunk = NULL;
    current_discussed_chunk = NULL;
}

void Explanation_Memory::add_chunk_record(instantiation* pBaseInstantiation)
{
    bool lShouldRecord = false;
    if ((!m_all_enabled) && (!pBaseInstantiation->prod || !pBaseInstantiation->prod->explain_its_chunks))
    {
        current_recording_chunk = NULL;
        return;
    }

    thisAgent->memoryManager->allocate_with_pool(MP_chunk_record, &current_recording_chunk);
    current_recording_chunk->init(thisAgent, chunk_id_count++);
}

void Explanation_Memory::end_chunk_record()
{
    if (current_recording_chunk)
    {
        current_recording_chunk->end_chunk_record();
        current_recording_chunk = NULL;
    }
}

void Explanation_Memory::cancel_chunk_record()
{
    if (current_recording_chunk)
    {
        current_recording_chunk->clean_up();
        thisAgent->memoryManager->free_with_pool(MP_chunk_record, current_recording_chunk);
        current_recording_chunk = NULL;
    }
}
void Explanation_Memory::delete_condition(uint64_t pCondID)
{
    all_conditions->erase(pCondID);
}

void Explanation_Memory::delete_action(uint64_t pActionID)
{
    all_actions->erase(pActionID);
}

void Explanation_Memory::delete_instantiation(uint64_t pInstID)
{
    instantiations->erase(pInstID);
}

void Explanation_Memory::add_result_instantiations(instantiation* pBaseInst, preference* pResults)
{
    if (current_recording_chunk)
    {
        for (preference* lResult = pResults; lResult != NIL; lResult = lResult->next_result)
        {
            if (lResult->inst != pBaseInst)
            {
                current_recording_chunk->result_instantiations->insert(lResult->inst);
            }
        }
    }
}

void Explanation_Memory::record_chunk_contents(production* pProduction, condition* lhs, action* rhs, preference* results, id_to_join_map* pIdentitySetMappings, instantiation* pBaseInstantiation, instantiation* pChunkInstantiation, ProductionType prodType)
{
    if (current_recording_chunk)
    {
        current_recording_chunk->record_chunk_contents(pProduction, lhs, rhs, results, pIdentitySetMappings, pBaseInstantiation, backtrace_number, pChunkInstantiation, prodType);
        chunks->insert({pProduction->name, current_recording_chunk});
        chunks_by_ID->insert({current_recording_chunk->chunkID, current_recording_chunk});
        thisAgent->symbolManager->symbol_add_ref(pProduction->name);
    }
}

condition_record* Explanation_Memory::add_condition(condition_record_list* pCondList, condition* pCond, instantiation_record* pInst, bool pMakeNegative, bool isChunkInstantiation)
{
    condition_record* lCondRecord;

    if (pCond->type != CONJUNCTIVE_NEGATION_CONDITION)
    {
        thisAgent->memoryManager->allocate_with_pool(MP_condition_record, &lCondRecord);
        increment_counter(condition_id_count);
        lCondRecord->init(thisAgent, pCond, condition_id_count, pInst, isChunkInstantiation);
        if (pMakeNegative)
        {
            lCondRecord->type = CONJUNCTIVE_NEGATION_CONDITION;
        }
        all_conditions->insert({lCondRecord->conditionID, lCondRecord});
        pCondList->push_back(lCondRecord);
        return lCondRecord;
    }
    else
    {
        /* Create condition and action records */
        condition_record* new_cond_record;
        for (condition* cond = pCond->data.ncc.top; cond != NIL; cond = cond->next)
        {
            new_cond_record = add_condition(pCondList, cond, pInst, true, isChunkInstantiation);
        }
        return new_cond_record;
    }
}

instantiation_record* Explanation_Memory::add_instantiation(instantiation* pInst, uint64_t pChunkID, bool isChunkInstantiation)
{
    if (pInst->explain_depth > EXPLAIN_MAX_BT_DEPTH) return NULL;

    bool lIsTerminalInstantiation = false;

    if (pInst->explain_status == explain_unrecorded)
    {

        /* Instantiations have their backtrace_number marked as the dependency analysis
         * is performed, so we can use that to determine if this instantiation needs to
         * be added.
         *
         * If it was not in the backtrace (created superstate item that was in bt) or
         * bt_depth == max, we add this instantiation but set it as a terminal
         * instantiation.  This puts a cap on the memory used, which can easily cause
         * issues for example in the count-and-die test agent that creates thousands
         * of instantiations */
        if ((pInst->backtrace_number != backtrace_number) || (pInst->explain_depth == EXPLAIN_MAX_BT_DEPTH))
        {
            lIsTerminalInstantiation = true;
        }

        /* Set status flag to recording to handle recursive addition */
        pInst->explain_status = explain_recording;
        pInst->explain_tc_num = backtrace_number;

        instantiation_record* lInstRecord;
        thisAgent->memoryManager->allocate_with_pool(MP_instantiation_record, &lInstRecord);
        lInstRecord->init(thisAgent, pInst, isChunkInstantiation);
        instantiations->insert({pInst->i_id, lInstRecord});
        lInstRecord->creating_chunk = pChunkID;
        return lInstRecord;
    } else if ((pInst->explain_status == explain_recorded) && (pInst->explain_tc_num != backtrace_number))
    {
        /* Update instantiation*/
        if ((pInst->backtrace_number != backtrace_number) || (pInst->explain_depth == EXPLAIN_MAX_BT_DEPTH))
        {
            lIsTerminalInstantiation = true;
        }
        /* Set status flag to recording to handle recursive addition */
        pInst->explain_status = explain_recording_update;
        pInst->explain_tc_num = backtrace_number;

        instantiation_record* lInstRecord = get_instantiation(pInst);
        return lInstRecord;
    }
    return get_instantiation(pInst);
}

action_record* Explanation_Memory::add_result(preference* pPref, action* pAction, bool isChunkInstantiation)
{
    increment_counter(action_id_count);
    action_record* lActionRecord;
    thisAgent->memoryManager->allocate_with_pool(MP_action_record, &lActionRecord);
    lActionRecord->init(thisAgent, pPref, pAction, action_id_count, isChunkInstantiation);

    all_actions->insert({lActionRecord->actionID, lActionRecord});
    return lActionRecord;
}

chunk_record* Explanation_Memory::get_chunk_record(Symbol* pChunkName)
{
    assert(pChunkName);
    auto iter_chunk = chunks->find(pChunkName);
    if (iter_chunk != chunks->end())
    {
        return(iter_chunk->second);
    }
    return NULL;
}

instantiation_record* Explanation_Memory::get_instantiation(instantiation* pInst)
{
    assert(pInst);
    auto iter_inst = instantiations->find(pInst->i_id);
    if (iter_inst != instantiations->end())
    {
        return(iter_inst->second);
    }
    return NULL;
}

void Explanation_Memory::excise_production_id(uint64_t pId)
{
    assert(pId);
    auto iter = production_id_map->find(pId);
    if (iter != production_id_map->end())
    {
        (*production_id_map)[pId] = NULL;
    }
}

production* Explanation_Memory::get_production(uint64_t pId)
{
    if (!pId) return NULL;
    auto iter = production_id_map->find(pId);
    if (iter != production_id_map->end())
    {
        return iter->second;
    }
    return NULL;
}

uint64_t Explanation_Memory::add_production_id_if_necessary(production* pProd)
{
    assert(pProd);

    auto iter = production_id_map->find(pProd->p_id);
    if (iter == production_id_map->end())
    {
        production_id_map->insert({pProd->p_id, pProd});
    }
    return pProd->p_id;
}

bool Explanation_Memory::toggle_production_watch(production* pProduction)
{
    if (pProduction->explain_its_chunks)
    {
        pProduction->explain_its_chunks = false;
        --num_rules_watched;
        outputManager->printa_sf(thisAgent, "No longer watching any chunks formed by rule '%y'\n", pProduction->name);
    } else {
        pProduction->explain_its_chunks = true;
        ++num_rules_watched;
        outputManager->printa_sf(thisAgent, "%eNow watching any chunks formed by rule '%y'\n", pProduction->name);
    }
    return true;
}

bool Explanation_Memory::watch_rule(const std::string* pStringParameter)
{
    Symbol* sym;

    sym = thisAgent->symbolManager->find_str_constant(pStringParameter->c_str());
    if (sym && (sym->sc->production))
    {
        toggle_production_watch(sym->sc->production);
        return true;
    }

    outputManager->printa_sf(thisAgent, "Could not find a rule named %s to watch.\nType 'print' to see a list of all rules.\n", pStringParameter->c_str());
    return false;
}

bool Explanation_Memory::explain_chunk(const std::string* pStringParameter)
{
    Symbol* sym;
    uint64_t lObjectID = 0;

    if (from_string(lObjectID, pStringParameter->c_str()))
    {
        if (!print_chunk_explanation_for_id(lObjectID))
        {
            outputManager->printa_sf(thisAgent, "Could not find a rule name or id %s.\nType 'explain list-chunks' or 'explain list-justifications' to see a list of rule formations Soar has recorded.\n", pStringParameter->c_str());
        } else {
            outputManager->printa_sf(thisAgent, "Now explaining %y.\n", current_discussed_chunk->name);
            print_chunk_explanation();
            return true;
        }
    } else {

        sym = thisAgent->symbolManager->find_str_constant(pStringParameter->c_str());
        if (sym && sym->sc->production)
        {
            /* Print chunk record if we can find it */
            chunk_record* lFoundChunk = get_chunk_record(sym);
            if (lFoundChunk)
            {
                discuss_chunk(lFoundChunk);
                outputManager->printa_sf(thisAgent, "Now explaining %y.\n\n", lFoundChunk->name);
                print_chunk_explanation();
                return true;
            }

            outputManager->printa_sf(thisAgent, "Soar has not recorded an explanation for %s.\nType 'explain list-chunks' or 'explain list-justifications' to see a list of rule formations Soar has recorded.\n", pStringParameter->c_str());
            return false;
        }
    }
    return false;
}

void Explanation_Memory::discuss_chunk(chunk_record* pChunkRecord)
{
    if (current_discussed_chunk != pChunkRecord)
    {
        if (current_discussed_chunk)
        {
            clear_chunk_from_instantiations();
            thisAgent->visualizationManager->reset_colors_for_id();
        }
        current_discussed_chunk = pChunkRecord;
        current_discussed_chunk->generate_dependency_paths();
    }
    last_printed_id = 0;

}

void Explanation_Memory::save_excised_production(production* pProd)
{
    production_record* lProductionRecord;
    thisAgent->memoryManager->allocate_with_pool(MP_production_record, &lProductionRecord);
    lProductionRecord->init(thisAgent, pProd);
    if (lProductionRecord->was_generated())
    {
        cached_production->insert(lProductionRecord);
    } else {
        thisAgent->memoryManager->free_with_pool(MP_production_record, lProductionRecord);
    }
}

bool Explanation_Memory::print_chunk_explanation_for_id(uint64_t pChunkID)
{
    auto iter_chunk = chunks_by_ID->find(pChunkID);
    if (iter_chunk == chunks_by_ID->end()) return false;
    discuss_chunk(iter_chunk->second);
    return true;
}

bool Explanation_Memory::print_instantiation_explanation_for_id(uint64_t pInstID)
{
    auto iter_inst = instantiations->find(pInstID);
    if (iter_inst == instantiations->end())
    {
        outputManager->printa_sf(thisAgent, "Could not find an instantiation with ID %u.\n", pInstID);
        return false;
    }
    last_printed_id = pInstID;
    print_instantiation_explanation(iter_inst->second);
    return true;
}

bool Explanation_Memory::explain_instantiation(const std::string* pObjectIDString)
{
    bool lSuccess = false;
    uint64_t lObjectID = 0;
    if (!from_string(lObjectID, pObjectIDString->c_str()))
    {
        outputManager->printa_sf(thisAgent, "The instantiation ID must be a number.\n");
    }
    lSuccess = print_instantiation_explanation_for_id(lObjectID);
    return lSuccess;
}


void Explanation_Memory::add_identity(Identity* pNewIdentity, Symbol* pGoal)
{
    assert(pNewIdentity && pGoal);
    identity_set* lIdentities;

    auto iter = all_identities_in_goal->find(pGoal);
    if (iter == all_identities_in_goal->end())
    {
        lIdentities = new identity_set();
        (*all_identities_in_goal)[pGoal] = lIdentities;
        thisAgent->symbolManager->symbol_add_ref(pGoal);
    } else {
        lIdentities = iter->second;
    }
    lIdentities->insert(pNewIdentity);
    pNewIdentity->add_ref();
}

void Explanation_Memory::clear_identities_in_set(identity_set* lIdenty_set)
{
    Identity*        l_inst_identity;

    for (auto it = lIdenty_set->begin(); it != lIdenty_set->end(); ++it)
    {
        l_inst_identity = (*it);
        IdentitySet_remove_ref(thisAgent, l_inst_identity);
    }
    delete lIdenty_set;
}

void Explanation_Memory::clear_identity_sets()
{
    Symbol*             lSym;

    assert(all_identities_in_goal->size() == 0);
    for (auto it1 = all_identities_in_goal->begin(); it1 != all_identities_in_goal->end(); ++it1)
    {
        lSym = it1->first;
        clear_identities_in_set(it1->second);
        thisAgent->symbolManager->symbol_remove_ref(&lSym);
    }
    all_identities_in_goal->clear();
}

void Explanation_Memory::clear_identity_sets_for_goal(Symbol* pGoal)
{
    Symbol*             lSym;

    auto iter = all_identities_in_goal->find(pGoal);
    if (iter != all_identities_in_goal->end())
    {
        lSym = iter->first;
        clear_identities_in_set(iter->second);
        thisAgent->symbolManager->symbol_remove_ref(&lSym);
        all_identities_in_goal->erase(iter);
    }
}

void Explanation_Memory::add_identity_set_mapping(uint64_t pI_ID, IDSet_Mapping_Type pType, Identity* pFromJoinSet, Identity* pToJoinSet)
{
    if (current_recording_chunk)
        current_recording_chunk->identity_analysis.add_identity_mapping(pI_ID, pType, pFromJoinSet, pToJoinSet);
}

bool Explanation_Memory::current_discussed_chunk_exists()
{
    return current_discussed_chunk;
}

void Explanation_Memory::increment_stat_duplicates(production* duplicate_rule)
{
    assert(duplicate_rule);
    increment_counter(stats.duplicates);
    if (current_recording_chunk)
    {
        chunk_record* lChunkRecord = get_chunk_record(duplicate_rule->name);
        if (lChunkRecord)
        {
            increment_counter(lChunkRecord->stats.duplicates);
        }
    }
};

void Explanation_Memory::clear_chunk_from_instantiations()
{
    instantiation_record* lNewInstRecord;
    for (auto it = current_discussed_chunk->backtraced_inst_records->begin(); it != current_discussed_chunk->backtraced_inst_records->end(); it++)
    {
        lNewInstRecord = (*it);
        if (lNewInstRecord->path_to_base)
        {
            lNewInstRecord->path_to_base->clear();
        }
    }
}

void Explanation_Memory::visualize_last_output()
{
    thisAgent->visualizationManager->viz_graph_start();
    if (!last_printed_id)
    {
        current_discussed_chunk->visualize();
    } else {
        visualize_instantiation_explanation_for_id(last_printed_id);
    }
    thisAgent->visualizationManager->viz_graph_end();
}

void Explanation_Memory::visualize_instantiation_graph()
{
    thisAgent->visualizationManager->viz_graph_start();
    for (auto it = current_discussed_chunk->backtraced_inst_records->begin(); it != current_discussed_chunk->backtraced_inst_records->end(); it++)
    {
        (*it)->visualize();
    }
    for (auto it = current_discussed_chunk->backtraced_inst_records->begin(); it != current_discussed_chunk->backtraced_inst_records->end(); it++)
    {
        (*it)->viz_connect_conditions();
    }
    thisAgent->visualizationManager->viz_graph_end();
}

void Explanation_Memory::visualize_contributors()
{
    thisAgent->visualizationManager->viz_graph_start();
    for (auto it = current_discussed_chunk->backtraced_inst_records->begin(); it != current_discussed_chunk->backtraced_inst_records->end(); it++)
    {
        (*it)->visualize();
    }
    for (auto it = current_discussed_chunk->backtraced_inst_records->begin(); it != current_discussed_chunk->backtraced_inst_records->end(); it++)
    {
        (*it)->viz_connect_conditions();
    }
    current_discussed_chunk->visualize();
    thisAgent->visualizationManager->viz_graph_end();
}

void Explanation_Memory::visualize_identity_graph()
{
    GraphViz_Visualizer* vm = thisAgent->visualizationManager;
    vm->viz_graph_start();
    current_discussed_chunk->identity_analysis.visualize();
    vm->viz_graph_end();
}

void Explanation_Memory::visualize_identity_graph_for_goal(Symbol* pGoal)
{
    GraphViz_Visualizer* vm = thisAgent->visualizationManager;
    vm->viz_graph_start();
    vm->viz_graph_end();
}

bool Explanation_Memory::visualize_instantiation_explanation_for_id(uint64_t pInstID)
{
    auto iter_inst = instantiations->find(pInstID);
    if (iter_inst == instantiations->end())
    {
        outputManager->printa_sf(thisAgent, "Could not find an instantiation with ID %u.\n", pInstID);
        return false;
    }
    last_printed_id = pInstID;
    (iter_inst->second)->visualize();
    return true;
}



