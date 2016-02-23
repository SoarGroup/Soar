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

    /* Create data structures used for EBC */
    chunks = new std::unordered_map< Symbol*, chunk_record* >();
    chunks_by_ID = new std::unordered_map< uint64_t, chunk_record* >();
    instantiations = new std::unordered_map< uint64_t, instantiation_record* >();
    all_conditions = new std::unordered_map< uint64_t, condition_record* >();
    all_actions = new std::unordered_map< uint64_t, action_record* >();

    instantiations_for_current_chunk = new inst_record_set();
    backtraced_instantiations = new inst_set();
}

void Explanation_Logger::initialize_counters()
{

    current_discussed_chunk = NULL;
    dependency_chart = "";

    chunk_id_count = 1;
    condition_id_count = 1;
    action_id_count = 1;

    total_recorded.chunks = 0;
    total_recorded.instantiations = 0;
    total_recorded.instantiations_referenced = 0;
    total_recorded.instantiations_skipped = 0;
    total_recorded.conditions = 0;
    total_recorded.actions = 0;

    stats.duplicates = 0;
    stats.unorderable = 0;
    stats.justification_did_not_match = 0;
    stats.chunk_did_not_match = 0;
    stats.no_grounds = 0;
    stats.max_chunks = 0;
    stats.succeeded = 0;
    stats.tested_local_negation = 0;
    stats.merged_conditions = 0;
    stats.chunks_attempted = 0;
    stats.justifications_attempted = 0;
    stats.justifications = 0;
    stats.instantations_backtraced = 0;
    stats.seen_instantations_backtraced = 0;
    stats.constraints_attached = 0;
    stats.constraints_collected = 0;

}
void Explanation_Logger::clear_explanations()
{
    dprint(DT_EXPLAIN, "Explanation logger clearing chunk records...\n");
    for (std::unordered_map< Symbol*, chunk_record* >::iterator it = (*chunks).begin(); it != (*chunks).end(); ++it)
    {
//        thisAgent->memoryManager->free_with_pool(MP_attachments, it->second);
        symbol_remove_ref(thisAgent, it->first);
        delete it->second;
    }
    chunks->clear();
    chunks_by_ID->clear();

    dprint(DT_EXPLAIN, "Explanation logger clearing instantiation records...\n");
    for (std::unordered_map< uint64_t, instantiation_record* >::iterator it = (*instantiations).begin(); it != (*instantiations).end(); ++it)
    {
//        thisAgent->memoryManager->free_with_pool(MP_attachments, it->second);
        delete it->second;
    }
    instantiations->clear();

    dprint(DT_EXPLAIN, "Explanation logger clearing condition records...\n");
    for (std::unordered_map< uint64_t, condition_record* >::iterator it = (*all_conditions).begin(); it != (*all_conditions).end(); ++it)
    {
//        thisAgent->memoryManager->free_with_pool(MP_attachments, it->second);
        delete it->second;
    }
    all_conditions->clear();

    dprint(DT_EXPLAIN, "Explanation logger clearing action records...\n");
    for (std::unordered_map< uint64_t, action_record* >::iterator it = (*all_actions).begin(); it != (*all_actions).end(); ++it)
    {
//        thisAgent->memoryManager->free_with_pool(MP_attachments, it->second);
        delete it->second;
    }
    all_actions->clear();
    instantiations_for_current_chunk->clear();
    backtraced_instantiations->clear();
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
    delete instantiations_for_current_chunk;
    delete backtraced_instantiations;

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
    total_recorded.chunks++;
}

void Explanation_Logger::end_chunk_record()
{
    current_recording_chunk = NULL;
    instantiations_for_current_chunk->clear();
    backtraced_instantiations->clear();

}

void Explanation_Logger::add_result_instantiations(preference* pResults)
{
    if (current_recording_chunk)
    {
        for (preference* lResult = pResults; lResult != NIL; lResult = lResult->next_result)
        {
            current_recording_chunk->result_instantiations->insert(lResult->inst);
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
    } else {
        dprint(DT_EXPLAIN, "Not recording chunk contents for %y because it is not being watched.\n", pProduction->name);
    }
}

condition_record* Explanation_Logger::add_condition(condition_record_list* pCondList, condition* pCond, instantiation_record* pInst, bool pStopHere, uint64_t bt_depth, bool pMakeNegative)
{
    dprint(DT_EXPLAIN_CONDS, "   Creating %s condition: %l\n", (!pStopHere ? "new" : "new terminal"), pCond);
    condition_record* lCondRecord;

    if (pCond->type != CONJUNCTIVE_NEGATION_CONDITION)
    {
        increment_counter(condition_id_count);
        lCondRecord = new condition_record(thisAgent, pCond, condition_id_count, pStopHere, bt_depth);
        lCondRecord->set_instantiation(pInst);
        if (pMakeNegative)
        {
            lCondRecord->type = CONJUNCTIVE_NEGATION_CONDITION;
        }
        all_conditions->insert({lCondRecord->conditionID, lCondRecord});
        increment_counter(total_recorded.conditions);
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
            new_cond_record = add_condition(pCondList, cond, pInst, pStopHere, bt_depth, true);
        }
        return new_cond_record;
    }
}

instantiation_record* Explanation_Logger::add_instantiation(instantiation* pInst, uint64_t bt_depth)
{
    if (++bt_depth > EXPLAIN_MAX_BT_DEPTH) return NULL;

    bool lIsTerminalInstantiation = false;

    dprint(DT_EXPLAIN_ADD_INST, "Adding instantiation for i%u (%y).\n",
        pInst->i_id, (pInst->prod ? pInst->prod->name : thisAgent->fake_instantiation_symbol));

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
        if ((pInst->backtrace_number != backtrace_number) || (bt_depth > EXPLAIN_MAX_BT_DEPTH))
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
        lInstRecord->record_instantiation_contents(pInst, lIsTerminalInstantiation, bt_depth);
        lInstRecord->terminal = lIsTerminalInstantiation;
        instantiations_for_current_chunk->insert(lInstRecord);

        increment_counter(total_recorded.instantiations);
        pInst->explain_status = explain_recorded;
        dprint(DT_EXPLAIN_ADD_INST, "- Returning new instantiation record for i%u (%y).  %d inst involved in current chunk\n",
            pInst->i_id, (pInst->prod ? pInst->prod->name : thisAgent->fake_instantiation_symbol), instantiations_for_current_chunk->size());
        return lInstRecord;
    } else if (pInst->explain_status == explain_recording) {
        dprint(DT_EXPLAIN_ADD_INST, "- Currently recording instantiation record for i%u (%y) in a parent call.  Did not create new record.\n", pInst->i_id, (pInst->prod ? pInst->prod->name : thisAgent->fake_instantiation_symbol));
    } else if ((pInst->explain_status == explain_recorded) && (pInst->explain_tc_num != backtrace_number))
    {
        /* Update instantiation*/
        dprint(DT_EXPLAIN_ADD_INST, "- Updating instantiation record for i%u (%y) that was created explaining a previous chunk.\n", pInst->i_id, (pInst->prod ? pInst->prod->name : thisAgent->fake_instantiation_symbol));
        if ((pInst->backtrace_number != backtrace_number) || (bt_depth > EXPLAIN_MAX_BT_DEPTH))
        {
            dprint(DT_EXPLAIN_ADD_INST, "- Backtrace number does not match (%d != %d).  Creating terminal instantiation record for i%u (%y).\n",
                pInst->backtrace_number, backtrace_number, pInst->i_id, (pInst->prod ? pInst->prod->name : thisAgent->fake_instantiation_symbol));
            lIsTerminalInstantiation = true;
        }
        /* Set status flag to recording to handle recursive addition */
        pInst->explain_status = explain_recording;
        pInst->explain_tc_num = backtrace_number;

        instantiation_record* lInstRecord = get_instantiation(pInst);
        assert(lInstRecord);
        lInstRecord->update_instantiation_contents(pInst, lIsTerminalInstantiation, bt_depth);
        instantiations_for_current_chunk->insert(lInstRecord);

        increment_counter(total_recorded.instantiations);
        pInst->explain_status = explain_recorded;
        dprint(DT_EXPLAIN_ADD_INST, "- Updated instantiation record for i%u (%y).  %d inst involved in current chunk\n",
            pInst->i_id, (pInst->prod ? pInst->prod->name : thisAgent->fake_instantiation_symbol), instantiations_for_current_chunk->size());
    } else {
        dprint(DT_EXPLAIN_ADD_INST, "- Already recorded instantiation record for i%u (%y).  Did not create new record.\n", pInst->i_id, (pInst->prod ? pInst->prod->name : thisAgent->fake_instantiation_symbol));
    }
    increment_counter(total_recorded.instantiations_skipped);
    return get_instantiation(pInst);
}

action_record* Explanation_Logger::add_result(preference* pPref, action* pAction)
{
    increment_counter(action_id_count);
    dprint(DT_EXPLAIN_CONDS, "   Adding action record %u for pref: %p\n", action_id_count, pPref);
    action_record* lActionRecord = new action_record(thisAgent, pPref, pAction, action_id_count);
    all_actions->insert({lActionRecord->actionID, lActionRecord});

    increment_counter(total_recorded.actions);

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


void Explanation_Logger::record_dependencies()
{

    assert(current_discussed_chunk);

    current_discussed_chunk->record_dependencies();

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
            current_discussed_chunk = lFoundChunk;
            print_chunk_explanation();
            return true;
        }

        outputManager->printa_sf(thisAgent, "Soar has not recorded an explanation for %s.\nType 'explain -l' to see a list of all chunk formations Soar has recorded.\n", pStringParameter->c_str());
        return false;
    }

    /* String has never been seen by Soar or is not a rule name */
    outputManager->printa_sf(thisAgent, "Could not find a chunk named %s.\nType 'explain -l' to see a list of all chunk formations Soar has recorded.\n", pStringParameter->c_str());
    return false;

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

    current_discussed_chunk = iter_chunk->second;
    print_chunk_explanation();
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
    dprint(DT_EXPLAIN, "Incrementing stats for duplicate chunk of rule %y.\n", duplicate_rule->name);
    chunk_record* lChunkRecord = get_chunk_record(duplicate_rule->name);
    if (lChunkRecord)
    {
        increment_counter(lChunkRecord->stats.duplicates);
    }
};
