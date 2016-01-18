#include "explain.h"
#include "agent.h"
#include "condition.h"
#include "debug.h"
#include "instantiation.h"
#include "preference.h"
#include "production.h"
#include "rhs.h"
#include "Symbol.h"
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
}

void Explanation_Logger::initialize_counters()
{

    current_discussed_chunk = NULL;

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
    total_recorded.chunks++;
}

void Explanation_Logger::end_chunk_record()
{
    current_recording_chunk = NULL;
}

void chunk_record::record_chunk_contents(Symbol* pName, condition* lhs, action* rhs, preference* results, id_to_id_map_type* pIdentitySetMappings, instantiation* pBaseInstantiation, tc_number pBacktraceNumber)
{
    name = pName;
    symbol_add_ref(thisAgent, name);

    conditions         = new condition_record_list;
    actions            = new action_record_list;

    dprint(DT_EXPLAIN, "Recording conditions...\n");
    /* Create condition and action records */
    bool has_backtrace_num = false;
    for (condition* cond = lhs; cond != NIL; cond = cond->next)
    {
        has_backtrace_num = (cond->bt.trace && cond->bt.trace->inst && (cond->bt.trace->inst->backtrace_number == pBacktraceNumber));
        thisAgent->explanationLogger->add_condition(conditions, cond, has_backtrace_num, 0);
    }

    dprint(DT_EXPLAIN, "Recording actions...\n");

    action_record* new_action_record;
    preference* pref;
    action* lAction;
    for (pref = results, lAction= rhs; (pref != NIL) && (lAction != NIL); pref = pref->next_result, lAction = lAction->next)
    {
        new_action_record = thisAgent->explanationLogger->add_result(pref, lAction);
        actions->push_back(new_action_record);
    }

    dprint(DT_EXPLAIN, "Recording base instantiation...\n");
    /* Might be needed in the case when none of the base instantiation conditions are in the chunk */
    baseInstantiation = thisAgent->explanationLogger->add_instantiation(pBaseInstantiation, 1);

    dprint(DT_EXPLAIN, "Connecting conditions...\n");
    /* Now that instantiations in backtrace are guaranteed to be recorded, connect
     * each condition to the appropriate parent instantiation action record */
    for (condition_record_list::iterator it = conditions->begin(); it != conditions->end(); it++)
    {
        (*it)->connect_to_action();
    }

    identity_set_mappings = new id_to_id_map_type();
    (*identity_set_mappings) = (*pIdentitySetMappings);

}

void Explanation_Logger::record_chunk_contents(Symbol* pName, condition* lhs, action* rhs, preference* results, id_to_id_map_type* pIdentitySetMappings, instantiation* pBaseInstantiation)
{
    if (current_recording_chunk)
    {
        dprint(DT_EXPLAIN, "Recording chunk contents for %y (c%u).  Backtrace number: %d\n", pName, current_recording_chunk->chunkID, backtrace_number);
        current_recording_chunk->record_chunk_contents(pName, lhs, rhs, results, pIdentitySetMappings, pBaseInstantiation, backtrace_number);
        chunks->insert({pName, current_recording_chunk});
        chunks_by_ID->insert({current_recording_chunk->chunkID, current_recording_chunk});

        symbol_add_ref(thisAgent, pName);
    } else {
        dprint(DT_EXPLAIN, "Not recording chunk contents for %y because it is not being watched.\n", pName);
    }
}

void Explanation_Logger::add_condition(condition_record_list* pCondList, condition* pCond, bool pStopHere, uint64_t bt_depth, bool pMakeNegative)
{
    dprint(DT_EXPLAIN, "   Creating %s condition: %l\n", (!pStopHere ? "new" : "new terminal"), pCond);
    condition_record* lCondRecord;

    if (pCond->type != CONJUNCTIVE_NEGATION_CONDITION)
    {
        lCondRecord = new condition_record(thisAgent, pCond, condition_id_count++, pStopHere, bt_depth);
        if (pMakeNegative)
        {
            lCondRecord->type = CONJUNCTIVE_NEGATION_CONDITION;
        }
        all_conditions->insert({lCondRecord->conditionID, lCondRecord});
        total_recorded.conditions++;
        pCondList->push_back(lCondRecord);
    }
    else
    {
        dprint(DT_EXPLAIN, "Recording new conditions for NCC...\n");

        /* Create condition and action records */
        condition_record* new_cond_record;
        for (condition* cond = pCond->data.ncc.top; cond != NIL; cond = cond->next)
        {
            add_condition(pCondList, cond, pStopHere, bt_depth, true);
        }
    }
}

instantiation_record* Explanation_Logger::add_instantiation(instantiation* pInst, uint64_t bt_depth)
{
    ++bt_depth;
    if (bt_depth > EXPLAIN_MAX_BT_DEPTH)
    {
        return NULL;
    }

    if (pInst->explain_status == explain_unrecorded)
    {
        bool lIsTerminalInstantiation = false;
        if ((pInst->backtrace_number == backtrace_number) && (bt_depth < EXPLAIN_MAX_BT_DEPTH))
        {
            /* Should not already be recorded */
//            assert(!get_instantiation(pInst));
        } else {
            /* Instantiations have their backtrace_number marked as the dependency analysis
             * is performed, so we can use that to determine if this instantiation needs to
             * be added.
             * If bt_depth == max, we also set as terminal instantiation*/
            dprint(DT_EXPLAIN, "Backtrace number does not match (%d != %d).  Creating terminal instantiation record for %y (i%u).\n",
                pInst->backtrace_number, backtrace_number, (pInst->prod ? pInst->prod->name : thisAgent->fake_instantiation_symbol), pInst->i_id);
            lIsTerminalInstantiation = true;
        }

        /* Set status flag to recording to handle recursive addition */
        pInst->explain_status = explain_recording;
        instantiation_record* lInstRecord = new instantiation_record(thisAgent, pInst);
        instantiations->insert({pInst->i_id, lInstRecord});
        lInstRecord->record_instantiation_contents(pInst, lIsTerminalInstantiation, bt_depth);
        total_recorded.instantiations++;
        pInst->explain_status = explain_recorded;
        dprint(DT_EXPLAIN, "Returning new explanation instantiation record for %y (i%u)\n", (pInst->prod ? pInst->prod->name : thisAgent->fake_instantiation_symbol), pInst->i_id);
        return lInstRecord;
    } else if (pInst->explain_status == explain_recording) {
        dprint(DT_EXPLAIN, "Currently recording instantiation record for %y (i%u) in a parent call.  Did not create new record.\n", (pInst->prod ? pInst->prod->name : thisAgent->fake_instantiation_symbol), pInst->i_id);
    } else if (pInst->explain_status == explain_recorded) {
        dprint(DT_EXPLAIN, "Already recorded instantiation record for %y (i%u).  Did not create new record.\n", (pInst->prod ? pInst->prod->name : thisAgent->fake_instantiation_symbol), pInst->i_id);
    } else if (pInst->explain_status == explain_connected) {
        dprint(DT_EXPLAIN, "Already recorded and connected instantiation record for %y (i%u).  Did not create new record.\n", (pInst->prod ? pInst->prod->name : thisAgent->fake_instantiation_symbol), pInst->i_id);
    }
    total_recorded.instantiations_skipped++;
    return get_instantiation(pInst);
}

action_record* Explanation_Logger::add_result(preference* pPref, action* pAction)
{
    dprint(DT_EXPLAIN, "   Adding result: %p\n", pPref);
    action_record* lActionRecord = new action_record(thisAgent, pPref, pAction, action_id_count++);
    all_actions->insert({lActionRecord->actionID, lActionRecord});

    total_recorded.actions++;

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

action_record::action_record(agent* myAgent, preference* pPref, action* pAction, uint64_t pActionID)
{
    thisAgent = myAgent;
    actionID = pActionID;
    instantiated_pref = shallow_copy_preference(thisAgent, pPref);
    original_pref = pPref;
    if (pAction)
    {
        variablized_action = copy_action(thisAgent, pAction);
    } else {
        variablized_action = NULL;
    }
    dprint(DT_EXPLAIN, "Created action record a%u for pref %p (%r ^%r %r)\naction %a", pActionID, pPref, pPref->rhs_funcs.id, pPref->rhs_funcs.attr, pPref->rhs_funcs.value, pAction);
}

action_record::~action_record()
{
    dprint(DT_EXPLAIN, "Deleting action record a%u for: %p\n", actionID, instantiated_pref);
    deallocate_preference(thisAgent, instantiated_pref);
    deallocate_action_list(thisAgent, variablized_action);
}

chunk_record::chunk_record(agent* myAgent, uint64_t pChunkID)
{
    thisAgent           = myAgent;
    name                = NULL;
    conditions          = new condition_record_list;
    actions             = new action_record_list;
    chunkID             = pChunkID;
    baseInstantiation   = NULL;
    stats.constraints_attached = 0;
    stats.constraints_collected = 0;
    stats.duplicates = 0;
    stats.instantations_backtraced = 0;
    stats.merged_conditions = 0;
    stats.seen_instantations_backtraced = 0;
    stats.tested_local_negation = false;

    dprint(DT_EXPLAIN, "Created new empty chunk record c%u\n", chunkID);
}

chunk_record::~chunk_record()
{
    dprint(DT_EXPLAIN, "Deleting chunk record c%u\n", chunkID);
    if (name)
    {
        symbol_remove_ref(thisAgent, name);
    }
    delete conditions;
    delete actions;
}

void condition_record::connect_to_action()
{
    if (parent_instantiation)
    {
        assert(cached_pref);
        parent_action = parent_instantiation->find_rhs_action(cached_pref);
        assert(parent_action);
        cached_pref = NULL;
        dprint(DT_EXPLAIN, "Linked condition (%t ^%t %t).\n", instantiated_tests.id, instantiated_tests.attr, instantiated_tests.value);
    } else {
        dprint(DT_EXPLAIN, "Did not link condition (%t ^%t %t).\n", instantiated_tests.id, instantiated_tests.attr, instantiated_tests.value);
    }
}

condition_record::condition_record(agent* myAgent, condition* pCond, uint64_t pCondID, bool pStopHere, uint64_t bt_depth)
{
    thisAgent = myAgent;
    conditionID = pCondID;
    type = pCond->type;

    condition_tests.id = copy_test(thisAgent, pCond->data.tests.id_test);
    condition_tests.attr = copy_test(thisAgent, pCond->data.tests.attr_test);
    condition_tests.value = copy_test(thisAgent, pCond->data.tests.value_test);

    if (pCond->bt.wme_)
    {
        matched_wme = new soar_module::symbol_triple(pCond->bt.wme_->id, pCond->bt.wme_->attr, pCond->bt.wme_->value);
        symbol_add_ref(thisAgent, matched_wme->id);
        symbol_add_ref(thisAgent, matched_wme->attr);
        symbol_add_ref(thisAgent, matched_wme->value);
    } else {
        matched_wme = NULL;
    }
    if (!pStopHere && pCond->bt.trace)
    {
        parent_instantiation = thisAgent->explanationLogger->add_instantiation(pCond->bt.trace->inst, bt_depth);
        /* Cache the pref to make it easier to connect this condition to the action that created
         * the preference later. */
        cached_pref = parent_instantiation ? pCond->bt.trace : NULL;
    } else {
        parent_instantiation = NULL;
        cached_pref = NULL;
    }
    parent_action = NULL;
}

condition_record::~condition_record()
{
    dprint(DT_EXPLAIN, "Deleting condition record c%u for: (%t ^%t %t)\n", conditionID, instantiated_tests.id, instantiated_tests.attr, instantiated_tests.value);
    deallocate_test(thisAgent, condition_tests.id);
    deallocate_test(thisAgent, condition_tests.attr);
    deallocate_test(thisAgent, condition_tests.value);
    if (matched_wme)
    {
        dprint(DT_EXPLAIN, "Removing references for matched wme: (%y ^%y %y)\n", matched_wme->id, matched_wme->attr, matched_wme->value);
        symbol_remove_ref(thisAgent, matched_wme->id);
        symbol_remove_ref(thisAgent, matched_wme->attr);
        symbol_remove_ref(thisAgent, matched_wme->value);
        delete matched_wme;
    }
}


instantiation_record::instantiation_record(agent* myAgent, instantiation* pInst)
{
    thisAgent           = myAgent;
    instantiationID     = pInst->i_id;
    conditions          = new condition_record_list;
    actions             = new action_record_list;
    production_name     = (pInst->prod ? pInst->prod->name : thisAgent->fake_instantiation_symbol);

    if (pInst->prod)
    {
        symbol_add_ref(thisAgent, pInst->prod->name);
    }

}

instantiation_record::~instantiation_record()
{
    dprint(DT_EXPLAIN, "Deleting instantiation record %y (i%u)\n", production_name, instantiationID);
    if (production_name)
    {
        symbol_remove_ref(thisAgent, production_name);
    }
    delete conditions;
    delete actions;
}

void instantiation_record::record_instantiation_contents(instantiation* pInst, bool pStopHere, uint64_t bt_depth)
{
    dprint(DT_EXPLAIN, "Recording instantiation contents for c%u (%y)\n", pInst->i_id, production_name);
    ++bt_depth;
    /* Create condition and action records */
    for (condition* cond = pInst->top_of_instantiated_conditions; cond != NIL; cond = cond->next)
    {
        thisAgent->explanationLogger->add_condition(conditions, cond, pStopHere, bt_depth);
    }

    dprint(DT_EXPLAIN, "   -->\n");

    action_record* new_action_record;
    for (preference* pref = pInst->preferences_generated; pref != NIL; pref = pref->next_result)
    {
        new_action_record = thisAgent->explanationLogger->add_result(pref);
        actions->push_front(new_action_record);
    }
}
action_record* instantiation_record::find_rhs_action(preference* pPref)
{
    action_record_list::iterator iter;

//    dprint(DT_EXPLAIN, "...Looking  for preference action %p...", pPref);
    for (iter = actions->begin(); iter != actions->end(); ++iter)
    {
        if ((*iter)->original_pref == pPref)
        {
            dprint(DT_EXPLAIN, "...found RHS action a%u for condition preference %p.\n", (*iter)->get_actionID(), pPref);
        }
        return (*iter);
    }
    dprint(DT_EXPLAIN, "...did not find pref %p among:\n", pPref);
    for (iter = actions->begin(); iter != actions->end(); ++iter)
    {
            dprint(DT_EXPLAIN, "      %p\n", (*iter)->original_pref);
    }
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
    stats.duplicates++;
    dprint(DT_EXPLAIN, "Incrementing stats for duplicate chunk of rule %y.\n", duplicate_rule->name);
    chunk_record* lChunkRecord = get_chunk_record(duplicate_rule->name);
    if (lChunkRecord)
    {
        lChunkRecord->stats.duplicates++;
    }
};
