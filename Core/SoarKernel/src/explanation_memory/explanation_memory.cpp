#include "explanation_memory.h"

#include "action_record.h"
#include "agent.h"
#include "condition_record.h"
#include "condition.h"
#include "dprint.h"
#include "ebc.h"
#include "ebc_identity_set.h"
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
    /* Cache agent and Output Manager pointer */
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
    all_actions = new std::unordered_map< uint64_t, action_record* >();
    all_conditions = new std::unordered_map< uint64_t, condition_record* >();
    all_identities_in_goal = new sym_to_identity_set_map();
    cached_production = new production_record_set();
    chunks = new std::unordered_map< Symbol*, chunk_record* >();
    chunks_by_ID = new std::unordered_map< uint64_t, chunk_record* >();
    instantiations = new std::unordered_map< uint64_t, instantiation_record* >();
    production_id_map = new std::unordered_map< uint64_t, production* >();
}

Explanation_Memory::~Explanation_Memory()
{
    dprint(DT_EXPLAIN, "Deleting explanation logger.\n");

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

    dprint(DT_EXPLAIN, "Done deleting explanation logger.\n");
}

void Explanation_Memory::initialize_counters()
{
    chunk_id_count = 1;
    condition_id_count = 0;
    action_id_count = 0;

    stats.duplicates = 0;
    stats.justification_did_not_match = 0;
    stats.chunk_did_not_match = 0;
    stats.no_grounds = 0;
    stats.max_chunks = 0;
    stats.max_dupes = 0;
    stats.tested_local_negation = 0;
    stats.tested_deep_copy = 0;
    stats.tested_quiescence = 0;
    stats.tested_ltm_recall = 0;
    stats.tested_local_negation_just = 0;
    stats.tested_deep_copy_just = 0;
    stats.tested_ltm_recall_just = 0;
    stats.merged_conditions = 0;
    stats.merged_disjunctions = 0;
    stats.merged_disjunction_values = 0;
    stats.eliminated_disjunction_values = 0;
    stats.chunks_attempted = 0;
    stats.chunks_succeeded = 0;
    stats.justifications_attempted = 0;
    stats.justifications_succeeded = 0;
    stats.instantations_backtraced = 0;
    stats.seen_instantations_backtraced = 0;
    stats.constraints_attached = 0;
    stats.constraints_collected = 0;
    stats.grounding_conditions_added = 0;
    stats.lhs_unconnected = 0;
    stats.rhs_unconnected = 0;
    stats.repair_failed = 0;
    stats.chunks_repaired = 0;
    stats.chunks_reverted = 0;
    stats.identities_created                = 0;
    stats.identities_joined                 = 0;
    stats.identities_joined_variable        = 0;
    stats.identities_joined_local_singleton = 0;
    stats.identities_joined_singleton       = 0;
    stats.identities_joined_child_results   = 0;
    stats.identities_literalized_rhs_literal    = 0;
    stats.identities_participated           = 0;
    stats.identity_propagations             = 0;
    stats.identity_propagations_blocked     = 0;
    stats.operational_constraints           = 0;
    stats.OSK_instantiations                = 0;
    stats.identities_literalized_rhs_func_arg       = 0;
    stats.identities_literalized_rhs_func_compare   = 0;
}

void Explanation_Memory::clear_explanations()
{
    dprint(DT_EXPLAIN_CACHE, "Explanation logger clearing %d chunk records...\n", chunks->size());
    Symbol* lSym;
    for (std::unordered_map< Symbol*, chunk_record* >::iterator it = (*chunks).begin(); it != (*chunks).end(); ++it)
    {
        lSym = it->first;
        thisAgent->symbolManager->symbol_remove_ref(&lSym);
        it->second->clean_up();
        thisAgent->memoryManager->free_with_pool(MP_chunk_record, it->second);
    }
    chunks->clear();
    chunks_by_ID->clear();

    dprint(DT_EXPLAIN_CACHE, "Explanation logger clearing %d instantiation records...\n", instantiations->size());
    for (std::unordered_map< uint64_t, instantiation_record* >::iterator it = (*instantiations).begin(); it != (*instantiations).end(); ++it)
    {
        it->second->clean_up();
        thisAgent->memoryManager->free_with_pool(MP_instantiation_record, it->second);
    }
    instantiations->clear();

    dprint(DT_EXPLAIN_CACHE, "Explanation logger clearing %d condition records...\n", all_conditions->size());
    for (std::unordered_map< uint64_t, condition_record* >::iterator it = (*all_conditions).begin(); it != (*all_conditions).end(); ++it)
    {
        it->second->clean_up();
        thisAgent->memoryManager->free_with_pool(MP_condition_record, it->second);
    }
    all_conditions->clear();

    dprint(DT_EXPLAIN_CACHE, "Explanation logger clearing %d action records...\n", all_actions->size());
    for (std::unordered_map< uint64_t, action_record* >::iterator it = (*all_actions).begin(); it != (*all_actions).end(); ++it)
    {
        it->second->clean_up();
        thisAgent->memoryManager->free_with_pool(MP_action_record, static_cast<action_record *>(it->second));
    }
    all_actions->clear();

    dprint(DT_EXPLAIN_CACHE, "Explanation logger clearing %d cached productions...\n", cached_production->size());
    for (std::set< production_record* >::iterator it = (*cached_production).begin(); it != (*cached_production).end(); ++it)
    {
        (*it)->clean_up();
        thisAgent->memoryManager->free_with_pool(MP_production_record, (*it));
    }

    cached_production->clear();
    production_id_map->clear();

    dprint(DT_EXPLAIN, "Explanation logger done clearing explanation records...\n");
}

void Explanation_Memory::re_init()
{
    dprint(DT_EXPLAIN_CACHE, "Re-initializing explanation logger.\n");
    clear_explanations();
    /* MToDo | Might not need to do.  They might get popped with stack */
    clear_identity_sets();
    initialize_counters();
    current_recording_chunk = NULL;
    current_discussed_chunk = NULL;
    dprint(DT_EXPLAIN_CACHE, "Done re-initializing explanation logger: %d %d %d %d %d %d\n", chunks->size(), chunks_by_ID->size(), all_conditions->size(), all_actions->size(), instantiations->size(), production_id_map->size());
}

void Explanation_Memory::add_chunk_record(instantiation* pBaseInstantiation)
{
    bool lShouldRecord = false;
    if ((!m_all_enabled) && (!pBaseInstantiation->prod || !pBaseInstantiation->prod->explain_its_chunks))
    {
        dprint(DT_EXPLAIN, "Explainer ignoring this chunk because it is not being watched.\n");
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

void Explanation_Memory::record_chunk_contents(production* pProduction, condition* lhs, action* rhs, preference* results, id_to_join_map* pIdentitySetMappings, instantiation* pBaseInstantiation, instantiation* pChunkInstantiation)
{
    if (current_recording_chunk)
    {
        dprint(DT_EXPLAIN, "Recording chunk contents for %y (c%u).  Backtrace number: %d\n", pProduction->name, current_recording_chunk->chunkID, backtrace_number);
        current_recording_chunk->record_chunk_contents(pProduction, lhs, rhs, results, pIdentitySetMappings, pBaseInstantiation, backtrace_number, pChunkInstantiation);
        chunks->insert({pProduction->name, current_recording_chunk});
        chunks_by_ID->insert({current_recording_chunk->chunkID, current_recording_chunk});
        thisAgent->symbolManager->symbol_add_ref(pProduction->name);
        dprint(DT_EXPLAIN, "Explanation logger done record_chunk_contents...\n");
    } else {
        dprint(DT_EXPLAIN, "Not recording chunk contents for %y because it is not being watched.\n", pProduction->name);
    }
}

condition_record* Explanation_Memory::add_condition(condition_record_list* pCondList, condition* pCond, instantiation_record* pInst , bool pMakeNegative)
{
    dprint(DT_EXPLAIN_CONDS, "   Creating condition: %l\n", pCond);
    condition_record* lCondRecord;

    if (pCond->type != CONJUNCTIVE_NEGATION_CONDITION)
    {
        thisAgent->memoryManager->allocate_with_pool(MP_condition_record, &lCondRecord);
        increment_counter(condition_id_count);
        lCondRecord->init(thisAgent, pCond, condition_id_count);
        lCondRecord->set_instantiation(pInst);
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
        dprint(DT_EXPLAIN_CONDS, "   Recording new conditions for NCC...\n");

        /* Create condition and action records */
        condition_record* new_cond_record;
        for (condition* cond = pCond->data.ncc.top; cond != NIL; cond = cond->next)
        {
            new_cond_record = add_condition(pCondList, cond, pInst, true);
        }
        return new_cond_record;
    }
}

instantiation_record* Explanation_Memory::add_instantiation(instantiation* pInst, uint64_t pChunkID)
{
    if (pInst->explain_depth > EXPLAIN_MAX_BT_DEPTH) return NULL;

    bool lIsTerminalInstantiation = false;
    dprint(DT_EXPLAIN_ADD_INST, "Adding instantiation for i%u (%y).\n",
        pInst->i_id, pInst->prod_name);

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
            dprint(DT_EXPLAIN_ADD_INST, "- Backtrace number does not match (%d != %d).  Creating terminal instantiation record...\n",
                pInst->backtrace_number, backtrace_number);
            lIsTerminalInstantiation = true;
        }

        /* Set status flag to recording to handle recursive addition */
        pInst->explain_status = explain_recording;
        pInst->explain_tc_num = backtrace_number;

        instantiation_record* lInstRecord;
        thisAgent->memoryManager->allocate_with_pool(MP_instantiation_record, &lInstRecord);
        lInstRecord->init(thisAgent, pInst);
        instantiations->insert({pInst->i_id, lInstRecord});
        lInstRecord->terminal = lIsTerminalInstantiation;
        lInstRecord->creating_chunk = pChunkID;
        dprint(DT_EXPLAIN_ADD_INST, "- Returning new instantiation record for i%u (%y).\n",
            pInst->i_id, pInst->prod_name);
        return lInstRecord;
    } else if ((pInst->explain_status == explain_recorded) && (pInst->explain_tc_num != backtrace_number))
    {
        /* Update instantiation*/
        dprint(DT_EXPLAIN_ADD_INST, "- Updating instantiation record for i%u (%y) that was created explaining a previous chunk.\n", pInst->i_id, pInst->prod_name);
        if ((pInst->backtrace_number != backtrace_number) || (pInst->explain_depth == EXPLAIN_MAX_BT_DEPTH))
        {
            dprint(DT_EXPLAIN_ADD_INST, "- Backtrace number does not match (%d != %d).  Creating terminal instantiation record for i%u (%y).\n",
                pInst->backtrace_number, backtrace_number, pInst->i_id, pInst->prod_name);
            lIsTerminalInstantiation = true;
        }
        /* Set status flag to recording to handle recursive addition */
        pInst->explain_status = explain_recording_update;
        pInst->explain_tc_num = backtrace_number;

        instantiation_record* lInstRecord = get_instantiation(pInst);
        lInstRecord->terminal = lIsTerminalInstantiation;
        dprint(DT_EXPLAIN_ADD_INST, "- Updated instantiation record for i%u (%y).\n", pInst->i_id, pInst->prod_name);
        return lInstRecord;
    } else if (pInst->explain_status == explain_recording) {
        dprint(DT_EXPLAIN_ADD_INST, "- Currently recording instantiation record for i%u (%y) in a parent call.  Did not create new record.\n", pInst->i_id, pInst->prod_name);
    } else {
        dprint(DT_EXPLAIN_ADD_INST, "- Already recorded instantiation record for i%u (%y).  Did not create new record.\n", pInst->i_id, pInst->prod_name);
        for (std::unordered_map< uint64_t, instantiation_record* >::iterator it = (*instantiations).begin(); it != (*instantiations).end(); ++it)
        {
            dprint(DT_EXPLAIN_ADD_INST, "i%u (%y)\n", it->second->instantiationID, it->second->production_name);
        }
    }
    return get_instantiation(pInst);
}

action_record* Explanation_Memory::add_result(preference* pPref, action* pAction)
{
    increment_counter(action_id_count);
    dprint(DT_EXPLAIN_CONDS, "   Adding action record %u for pref: %p\n", action_id_count, pPref);
    action_record* lActionRecord;
    thisAgent->memoryManager->allocate_with_pool(MP_action_record, &lActionRecord);
    lActionRecord->init(thisAgent, pPref, pAction, action_id_count);

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
    dprint(DT_EXPLAIN_CACHE, "Saving excised production: %y\n", pProd->name);
    production_record* lProductionRecord;
    thisAgent->memoryManager->allocate_with_pool(MP_production_record, &lProductionRecord);
    lProductionRecord->init(thisAgent, pProd);
    if (lProductionRecord->was_generated())
    {
        cached_production->insert(lProductionRecord);
    } else {
        thisAgent->memoryManager->free_with_pool(MP_production_record, lProductionRecord);
    }
    dprint(DT_EXPLAIN_CACHE, "Explanation logger done adding production record for excised production: %y\n", pProd->name);
}

bool Explanation_Memory::print_chunk_explanation_for_id(uint64_t pChunkID)
{
    std::unordered_map< uint64_t, chunk_record* >::iterator iter_chunk;

    iter_chunk = chunks_by_ID->find(pChunkID);
    if (iter_chunk == chunks_by_ID->end()) return false;
    discuss_chunk(iter_chunk->second);
    return true;
}

bool Explanation_Memory::print_instantiation_explanation_for_id(uint64_t pInstID)
{
    std::unordered_map< uint64_t, instantiation_record* >::iterator iter_inst;

    iter_inst = instantiations->find(pInstID);
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


void Explanation_Memory::add_identity(IdentitySet* pNewIdentity, Symbol* pGoal)
{
    assert(pNewIdentity && pGoal);
    identity_set_set* lIdentities;

    auto iter = all_identities_in_goal->find(pGoal);
    if (iter == all_identities_in_goal->end())
    {
        dprint(DT_EXPLAIN_IDENTITIES, "Creating new identities set and increasing refcount on goal %y\n", pGoal);
        lIdentities = new identity_set_set();
        (*all_identities_in_goal)[pGoal] = lIdentities;
        thisAgent->symbolManager->symbol_add_ref(pGoal);
    } else {
        lIdentities = iter->second;
    }
    dprint(DT_EXPLAIN_IDENTITIES, "Increasing refcount of identity %u and adding to identities set of goal %y\n", pNewIdentity->idset_id, pGoal);
    lIdentities->insert(pNewIdentity);
    pNewIdentity->add_ref();
}

void Explanation_Memory::clear_identities_in_set(identity_set_set* lIdenty_set)
{
    IdentitySet*        lIdentity;

    for (auto it = lIdenty_set->begin(); it != lIdenty_set->end(); ++it)
    {
        lIdentity = (*it);
        dprint(DT_EXPLAIN_IDENTITIES, "Removing refcount %u \n", lIdentity->idset_id);
        lIdentity->remove_ref();
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
        dprint(DT_EXPLAIN_IDENTITIES, "Clearing identity refcounts for goal %y\n", it1->first);
        clear_identities_in_set(it1->second);
        dprint(DT_EXPLAIN_IDENTITIES, "... decreasing refcount for goal %y\n", it1->first);
        thisAgent->symbolManager->symbol_remove_ref(&lSym);
    }
    all_identities_in_goal->clear();
}

void Explanation_Memory::clear_identity_sets_for_goal(Symbol* pGoal)
{
    Symbol*             lSym;

    dprint(DT_EXPLAIN_IDENTITIES, "Clearing identity refcounts for goal %y\n", pGoal);
    auto iter = all_identities_in_goal->find(pGoal);
    if (iter != all_identities_in_goal->end())
    {
        lSym = iter->first;
        clear_identities_in_set(iter->second);
        dprint(DT_EXPLAIN_IDENTITIES, "... decreasing refcount for goal %y\n", pGoal);
        thisAgent->symbolManager->symbol_remove_ref(&lSym);
        all_identities_in_goal->erase(iter);
    }
}

void Explanation_Memory::add_identity_set_mapping(uint64_t pI_ID, IDSet_Mapping_Type pType, IdentitySet* pFromJoinSet, IdentitySet* pToJoinSet)
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
            dprint(DT_EXPLAIN, "Incrementing stats for duplicate chunk of rule %y.\n", duplicate_rule->name);
            increment_counter(lChunkRecord->stats.duplicates);
        }
    }
};

void Explanation_Memory::increment_stat_grounding_conds_added(int pNumConds)
{
    increment_counter(stats.grounding_conditions_added);
    if (current_recording_chunk)
    {
        dprint(DT_EXPLAIN, "Incrementing stats for %d grounding conditions in rule %y.\n", pNumConds, current_recording_chunk->name);
        add_to_counter(current_recording_chunk->stats.num_grounding_conditions_added, pNumConds);
    }
};

void Explanation_Memory::increment_stat_chunks_reverted()
{
    increment_counter(stats.chunks_reverted);
    if (current_recording_chunk)
    {
        dprint(DT_EXPLAIN, "Incrementing stats for reverted chunk in rule %y.\n", current_recording_chunk->name);
        current_recording_chunk->stats.reverted = true;
    }
    stats.justifications_attempted--;
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
    current_discussed_chunk->visualize();
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

void Explanation_Memory::visualize_identity_graph()
{
    thisAgent->visualizationManager->viz_graph_start();
    current_discussed_chunk->identity_analysis.visualize();
    thisAgent->visualizationManager->viz_graph_end();
}

bool Explanation_Memory::visualize_instantiation_explanation_for_id(uint64_t pInstID)
{
    std::unordered_map< uint64_t, instantiation_record* >::iterator iter_inst;

    iter_inst = instantiations->find(pInstID);
    if (iter_inst == instantiations->end())
    {
        outputManager->printa_sf(thisAgent, "Could not find an instantiation with ID %u.\n", pInstID);
        return false;
    }
    last_printed_id = pInstID;
    (iter_inst->second)->visualize();
    return true;
}



