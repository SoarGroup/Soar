#include "explain.h"
#include "agent.h"
#include "condition.h"
#include "debug.h"
#include "instantiation.h"
#include "preference.h"
#include "production.h"
#include "rhs.h"
#include "symbol.h"
#include "test.h"
#include "output_manager.h"
#include "working_memory.h"

/* This crashes in count-and-die if depth is around 1000 (Macbook Pro 2012, 8MB) */
#define EXPLAIN_MAX_BT_DEPTH 900

Explanation_Logger::Explanation_Logger(agent* myAgent)
{
    /* Cache agent and Output Manager pointer */
    thisAgent = myAgent;
    outputManager = &Output_Manager::Get_OM();

    initialize_counters();
    enabled = false;
    num_rules_watched = 0;

    print_explanation_trace = true;
    last_printed_id = 0;

    /* Create data structures used for EBC */
    chunks = new std::unordered_map< Symbol*, chunk_record* >();
    chunks_by_ID = new std::unordered_map< uint64_t, chunk_record* >();
    instantiations = new std::unordered_map< uint64_t, instantiation_record* >();
    all_conditions = new std::unordered_map< uint64_t, condition_record* >();
    all_actions = new std::unordered_map< uint64_t, action_record* >();
    all_excised_productions = new std::set< production_record* >();

}

void Explanation_Logger::initialize_counters()
{

    current_discussed_chunk = NULL;

    chunk_id_count = 1;
    condition_id_count = 1;
    action_id_count = 1;
    id_set_counter = 0;

    stats.duplicates = 0;
    stats.unorderable = 0;
    stats.justification_did_not_match = 0;
    stats.chunk_did_not_match = 0;
    stats.no_grounds = 0;
    stats.max_chunks = 0;
    stats.tested_local_negation = 0;
    stats.merged_conditions = 0;
    stats.chunks_attempted = 0;
    stats.chunks_succeeded = 0;
    stats.chunks_reverted = 0;
    stats.justifications_attempted = 0;
    stats.justifications_succeeded = 0;
    stats.instantations_backtraced = 0;
    stats.seen_instantations_backtraced = 0;
    stats.constraints_attached = 0;
    stats.constraints_collected = 0;
    stats.grounding_conditions_added = 0;

}
void Explanation_Logger::clear_explanations()
{
    dprint(DT_EXPLAIN, "Explanation logger clearing chunk records...\n");
    for (std::unordered_map< Symbol*, chunk_record* >::iterator it = (*chunks).begin(); it != (*chunks).end(); ++it)
    {
        if (it->second->original_production)
        {
            it->second->original_production->save_for_justification_explanation = false;
        }
        symbol_remove_ref(thisAgent, it->first);
        delete it->second;
    }
    chunks->clear();
    chunks_by_ID->clear();

    dprint(DT_EXPLAIN, "Explanation logger clearing instantiation records...\n");
    for (std::unordered_map< uint64_t, instantiation_record* >::iterator it = (*instantiations).begin(); it != (*instantiations).end(); ++it)
    {
        if (it->second->original_production)
        {
            it->second->original_production->save_for_justification_explanation = false;
        }
        delete it->second;
    }
    instantiations->clear();

    dprint(DT_EXPLAIN, "Explanation logger clearing condition records...\n");
    for (std::unordered_map< uint64_t, condition_record* >::iterator it = (*all_conditions).begin(); it != (*all_conditions).end(); ++it)
    {
        delete it->second;
    }
    all_conditions->clear();

    dprint(DT_EXPLAIN, "Explanation logger clearing action records...\n");
    for (std::unordered_map< uint64_t, action_record* >::iterator it = (*all_actions).begin(); it != (*all_actions).end(); ++it)
    {
        delete it->second;
    }
    all_actions->clear();

    dprint(DT_EXPLAIN, "Explanation logger clearing cached productions...\n");
    for (std::set< production_record* >::iterator it = (*all_excised_productions).begin(); it != (*all_excised_productions).end(); ++it)
    {
        delete (*it);
    }
    all_excised_productions->clear();
}

Explanation_Logger::~Explanation_Logger()
{
    dprint(DT_EXPLAIN, "Deleting explanation logger.\n");

    current_recording_chunk = NULL;
    current_discussed_chunk = NULL;
    clear_explanations();

    delete chunks;
    delete chunks_by_ID;
    delete all_conditions;
    delete all_actions;
    delete instantiations;
}

void Explanation_Logger::re_init()
{
    dprint(DT_EXPLAIN, "Re-intializing explanation logger.\n");
    clear_explanations();
    initialize_counters();
    current_recording_chunk = NULL;
    current_discussed_chunk = NULL;
    dprint(DT_EXPLAIN, "Done re-intializing explanation logger.\n");

}

void Explanation_Logger::add_chunk_record(instantiation* pBaseInstantiation)
{
    bool lShouldRecord = false;
    if ((!enabled) && (!pBaseInstantiation->prod || !pBaseInstantiation->prod->explain_its_chunks))
    {
        dprint(DT_EXPLAIN, "Explainer ignoring this chunk because it is not being watched.\n");
        current_recording_chunk = NULL;
        return;
    }

    current_recording_chunk = new chunk_record(thisAgent, chunk_id_count++);
}

void Explanation_Logger::end_chunk_record()
{
    if (current_recording_chunk)
    {
        current_recording_chunk->end_chunk_record();
        current_recording_chunk = NULL;
    }
}

void Explanation_Logger::cancel_chunk_record()
{
    if (current_recording_chunk)
    {
        delete current_recording_chunk;
        current_recording_chunk = NULL;
    }
}
void Explanation_Logger::delete_condition(uint64_t pCondID)
{
    all_conditions->erase(pCondID);
}

void Explanation_Logger::delete_action(uint64_t pActionID)
{
    all_actions->erase(pActionID);
}

void Explanation_Logger::delete_instantiation(uint64_t pInstID)
{
    instantiations->erase(pInstID);
}

void Explanation_Logger::add_result_instantiations(instantiation* pBaseInst, preference* pResults)
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

void Explanation_Logger::record_chunk_contents(production* pProduction, condition* lhs, action* rhs, preference* results, id_to_id_map_type* pIdentitySetMappings, instantiation* pBaseInstantiation, instantiation* pChunkInstantiation)
{
    if (current_recording_chunk)
    {
        dprint(DT_EXPLAIN, "Recording chunk contents for %y (c%u).  Backtrace number: %d\n", pProduction->name, current_recording_chunk->chunkID, backtrace_number);
        current_recording_chunk->record_chunk_contents(pProduction, lhs, rhs, results, pIdentitySetMappings, pBaseInstantiation, backtrace_number, pChunkInstantiation);
        chunks->insert({pProduction->name, current_recording_chunk});
        chunks_by_ID->insert({current_recording_chunk->chunkID, current_recording_chunk});
        symbol_add_ref(thisAgent, pProduction->name);
        dprint(DT_EXPLAIN, "Explanation logger done record_chunk_contents...\n");
    } else {
        dprint(DT_EXPLAIN, "Not recording chunk contents for %y because it is not being watched.\n", pProduction->name);
    }
}

condition_record* Explanation_Logger::add_condition(condition_record_list* pCondList, condition* pCond, instantiation_record* pInst , bool pMakeNegative)
{
    dprint(DT_EXPLAIN_CONDS, "   Creating condition: %l\n", pCond);
    condition_record* lCondRecord;

    if (pCond->type != CONJUNCTIVE_NEGATION_CONDITION)
    {
        increment_counter(condition_id_count);
        lCondRecord = new condition_record(thisAgent, pCond, condition_id_count);
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

instantiation_record* Explanation_Logger::add_instantiation(instantiation* pInst, uint64_t pChunkID)
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

        instantiation_record* lInstRecord = new instantiation_record(thisAgent, pInst);
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

action_record* Explanation_Logger::add_result(preference* pPref, action* pAction)
{
    increment_counter(action_id_count);
    dprint(DT_EXPLAIN_CONDS, "   Adding action record %u for pref: %p\n", action_id_count, pPref);
    action_record* lActionRecord = new action_record(thisAgent, pPref, pAction, action_id_count);
    all_actions->insert({lActionRecord->actionID, lActionRecord});

    return lActionRecord;
}

chunk_record* Explanation_Logger::get_chunk_record(Symbol* pChunkName)
{
    assert(pChunkName);

    std::unordered_map< Symbol *, chunk_record* >::iterator iter_chunk;

//    dprint(DT_EXPLAIN, "...Looking  for chunk %y...", pChunkName);
    iter_chunk = chunks->find(pChunkName);
    if (iter_chunk != chunks->end())
    {
//        dprint(DT_EXPLAIN, "...found chunk record.\n");
        return(iter_chunk->second);
    }
//    dprint(DT_EXPLAIN, "...not found..\n");
    return NULL;
}

instantiation_record* Explanation_Logger::get_instantiation(instantiation* pInst)
{
    assert(pInst);

    /* See if we already have an instantiation record */
    std::unordered_map< uint64_t, instantiation_record* >::iterator iter_inst;

//    dprint(DT_EXPLAIN, "...Looking  for instantiation id %u...", pInst->i_id);
    iter_inst = instantiations->find(pInst->i_id);
    if (iter_inst != instantiations->end())
    {
//        dprint(DT_EXPLAIN, "...found existing ebc logger record.\n");
        return(iter_inst->second);
    }
//    dprint(DT_EXPLAIN, "...not found..\n");
    return NULL;
}


bool Explanation_Logger::toggle_production_watch(production* pProduction)
{
    if (pProduction->explain_its_chunks)
    {
        pProduction->explain_its_chunks = false;
        --num_rules_watched;
        outputManager->printa_sf(thisAgent, "No longer watching any chunks formed by rule '%y'\n", pProduction->name);
    } else {
        pProduction->explain_its_chunks = true;
        ++num_rules_watched;
        outputManager->printa_sf(thisAgent, "%fNow watching any chunks formed by rule '%y'\n", pProduction->name);
    }
    return true;
}

bool Explanation_Logger::watch_rule(const std::string* pStringParameter)
{
    Symbol* sym;

    sym = find_str_constant(thisAgent, pStringParameter->c_str());
    if (sym && (sym->sc->production))
    {
        toggle_production_watch(sym->sc->production);
        return true;
    }

    outputManager->printa_sf(thisAgent, "Could not find a rule named %s to watch.\nType 'print' to see a list of all rules.\n", pStringParameter->c_str());
    return false;
}

bool Explanation_Logger::explain_chunk(const std::string* pStringParameter)
{
    Symbol* sym;

    sym = find_str_constant(thisAgent, pStringParameter->c_str());
    if (sym && sym->sc->production)
    {
        /* Print chunk record if we can find it */
        chunk_record* lFoundChunk = get_chunk_record(sym);
        if (lFoundChunk)
        {
                    discuss_chunk(lFoundChunk);
                    return true;
        }

        outputManager->printa_sf(thisAgent, "Soar has not recorded an explanation for %s.\nType 'explain -l' to see a list of all chunk formations Soar has recorded.\n", pStringParameter->c_str());
        return false;
    }

    /* String has never been seen by Soar or is not a rule name */
    outputManager->printa_sf(thisAgent, "Could not find a chunk named %s.\nType 'explain -l' to see a list of all chunk formations Soar has recorded.\n", pStringParameter->c_str());
    return false;

}

void Explanation_Logger::discuss_chunk(chunk_record* pChunkRecord)
{
    if (current_discussed_chunk != pChunkRecord)
    {
        outputManager->printa_sf(thisAgent, "Now explaining %y.\n  - Note that future explain commands are now relative to the problem-solving that led to that chunk.\n\n", pChunkRecord->name);
        if (current_discussed_chunk)
        {
            clear_chunk_from_instantiations();
        }
        current_discussed_chunk = pChunkRecord;
        last_printed_id = 0;
        current_discussed_chunk->generate_dependency_paths();
    }
    print_chunk_explanation();

}

void Explanation_Logger::save_excised_production(production* pProd)
{
    dprint(DT_EXPLAIN, "Explanation logger adding production record for excised production: %y\n", pProd->name);
    production_record* lProductionRecord = new production_record(thisAgent, pProd);
    all_excised_productions->insert(lProductionRecord);

    dprint(DT_EXPLAIN, "...searching %d chunks for excised rule %y...\n", chunks->size(), pProd->name);
    for (auto it = (*chunks).begin(); it != (*chunks).end(); ++it)
    {
        chunk_record* pChunk = it->second;
        if (it->second->original_production == pProd)
        {
            dprint(DT_EXPLAIN, "Found chunk %u (%y)...\n", it->second->chunkID, it->second->name);
            it->second->original_production = NULL;
            it->second->excised_production = lProductionRecord;
        }
    }

    dprint(DT_EXPLAIN, "...searching %d instantiations for excised rule %y...\n", instantiations->size(), pProd->name);
    for (auto it = (*instantiations).begin(); it != (*instantiations).end(); ++it)
    {
        instantiation_record* pInst = it->second;
        if (it->second->original_production == pProd)
        {
            dprint(DT_EXPLAIN, "Found instantiations i%u...\n", it->second->instantiationID);
            it->second->original_production = NULL;
            it->second->excised_production = lProductionRecord;
        }
    }
    dprint(DT_EXPLAIN, "Explanation logger done adding production record for excised production: %y\n", pProd->name);
}

bool Explanation_Logger::print_chunk_explanation_for_id(uint64_t pChunkID)
{
    std::unordered_map< uint64_t, chunk_record* >::iterator iter_chunk;

    iter_chunk = chunks_by_ID->find(pChunkID);
    if (iter_chunk == chunks_by_ID->end())
    {
        outputManager->printa_sf(thisAgent, "Could not find a chunk with ID %u.\n", pChunkID);
        return false;
    }

    discuss_chunk(iter_chunk->second);
    return true;
}

bool Explanation_Logger::print_instantiation_explanation_for_id(uint64_t pInstID)
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

bool Explanation_Logger::print_condition_explanation_for_id(uint64_t pConditionID)
{
    std::unordered_map< uint64_t, condition_record* >::iterator iter_inst;
    identity_triple lWatchIdentities;

    iter_inst = all_conditions->find(pConditionID);
    if (iter_inst == all_conditions->end())
    {
        outputManager->printa_sf(thisAgent, "Could not find a condition with ID %u.\n", pConditionID);
        return false;
    } else
    {
        if ((iter_inst->second->condition_tests.id->eq_test->identity == current_explained_ids.id) &&
            (iter_inst->second->condition_tests.attr->eq_test->identity == current_explained_ids.attr) &&
            (iter_inst->second->condition_tests.value->eq_test->identity == current_explained_ids.value))
        {
            current_explained_ids.id = 0;
            current_explained_ids.attr = 0;
            current_explained_ids.value = 0;
            outputManager->printa_sf(thisAgent, "No longer highlighting conditions related to condition %u: (%t ^%t %t).\n", pConditionID,
                iter_inst->second->condition_tests.id, iter_inst->second->condition_tests.attr, iter_inst->second->condition_tests.value);
        } else
        {
            current_explained_ids.id = iter_inst->second->condition_tests.id->eq_test->identity;
            current_explained_ids.attr = iter_inst->second->condition_tests.attr->eq_test->identity;
            current_explained_ids.value = iter_inst->second->condition_tests.value->eq_test->identity;
            outputManager->printa_sf(thisAgent, "Highlighting conditions related to condition %u: (%t ^%t %t).\n", pConditionID,
                iter_inst->second->condition_tests.id, iter_inst->second->condition_tests.attr, iter_inst->second->condition_tests.value);
        }
    }
    return true;
}

bool Explanation_Logger::explain_item(const std::string* pObjectTypeString, const std::string* pObjectIDString)
{
    /* First argument must be an object type.  Current valid types are 'chunk',
     * and 'instantiation' */
    bool lSuccess = false;
    uint64_t lObjectID = 0;
    char lFirstChar = pObjectTypeString->at(0);
    if (lFirstChar == 'c')
    {
        if (!from_string(lObjectID, pObjectIDString->c_str()))
        {
            outputManager->printa_sf(thisAgent, "The chunk ID must be a number.  Use 'explain [chunk-name] to explain by name.'\n");
        }
            lSuccess = print_chunk_explanation_for_id(lObjectID);
        } else if (lFirstChar == 'i')
    {
        if (!from_string(lObjectID, pObjectIDString->c_str()))
        {
            outputManager->printa_sf(thisAgent, "The instantiation ID must be a number.\n");
        }
            lSuccess = print_instantiation_explanation_for_id(lObjectID);
        } else if (lFirstChar == 'l')
    {
        if (!from_string(lObjectID, pObjectIDString->c_str()))
        {
            outputManager->printa_sf(thisAgent, "The condition ID must be a number.\n");
        }
        lSuccess = print_condition_explanation_for_id(lObjectID);
    } else
    {
        outputManager->printa_sf(thisAgent, "'%s' is not a type of item Soar can explain.\n", pObjectTypeString->c_str());
        return false;
    }

    return lSuccess;
}


bool Explanation_Logger::current_discussed_chunk_exists()
{
    return current_discussed_chunk;
}

void Explanation_Logger::increment_stat_duplicates(production* duplicate_rule)
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

void Explanation_Logger::increment_stat_grounded(int pNumConds)
{
    increment_counter(stats.grounding_conditions_added);
    if (current_recording_chunk)
    {
        dprint(DT_EXPLAIN, "Incrementing stats for %d grounding conditions in rule %y.\n", pNumConds, current_recording_chunk->name);
        add_to_counter(current_recording_chunk->stats.num_grounding_conditions_added, pNumConds);
    }
};

void Explanation_Logger::increment_stat_reverted()
{
    increment_counter(stats.chunks_reverted);
    if (current_recording_chunk)
    {
        dprint(DT_EXPLAIN, "Incrementing stats for reverted chunk in rule %y.\n", current_recording_chunk->name);
        current_recording_chunk->stats.reverted = true;
    }
    stats.justifications_attempted--;
};

void Explanation_Logger::clear_chunk_from_instantiations()
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
