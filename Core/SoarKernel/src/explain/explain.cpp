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

int condition_count(condition* pCond)
{
    int cnt = 0;
    while (pCond != NULL)
    {
        ++cnt;
        pCond = pCond->next;
    }
    return cnt;
}

void chunk_record::record_chunk_contents(production* pProduction, condition* lhs, action* rhs, preference* results, id_to_id_map_type* pIdentitySetMappings, instantiation* pBaseInstantiation, tc_number pBacktraceNumber, instantiation* pChunkInstantiation)
{
    if (pProduction)
    {
        name = pProduction->name;
        symbol_add_ref(thisAgent, name);
        original_production = pProduction;
        original_production->save_for_justification_explanation = true;

    } else {
        name = NULL;
        original_production = NULL;
        assert(false);
    }
    conditions         = new condition_record_list;
    actions            = new action_record_list;

    dprint(DT_EXPLAIN, "(1) Recording base instantiation i%u of chunk (%d conditions)...\n", pBaseInstantiation->i_id, condition_count(pBaseInstantiation->top_of_instantiated_conditions));
    baseInstantiation = thisAgent->explanationLogger->add_instantiation(pBaseInstantiation, 1);

    dprint(DT_EXPLAIN, "(2) Recording other result instantiation of chunk...\n", pBaseInstantiation->i_id);
    instantiation_record* lResultInstRecord;
    for (auto it = result_instantiations->begin(); it != result_instantiations->end(); ++it)
    {
//        lResultInstRecord = thisAgent->explanationLogger->get_instantiation((*it));
//        if (!lResultInstRecord)
//        {
//            dprint(DT_EXPLAIN, "...FOUND EXTRA RESULT INSTANTIATION: %u (%y)...\n", (*it)->i_id, ((*it)->prod ? (*it)->prod->name : thisAgent->fake_instantiation_symbol));
            lResultInstRecord = thisAgent->explanationLogger->add_instantiation((*it), 1);
            assert(lResultInstRecord);
            result_inst_records->insert(lResultInstRecord);
//        } else {
//            dprint(DT_EXPLAIN, "...result instantiation already exists: %u (%y)...\n", (*it)->i_id, ((*it)->prod ? (*it)->prod->name : thisAgent->fake_instantiation_symbol));
//        }
    }

    /* Comment not true if we keep here:  We link up all of the dependencies here.  Since the linking may be expensive and
     * we may be watching all chunk formations, we wait until someone attempts to look
     * at the dependency analysis before we perform the linking. */
    dprint(DT_EXPLAIN, "(3) Connecting conditions in trace and generating baths to base instantiations...\n");
    record_dependencies();

    dprint(DT_EXPLAIN, "(4) Recording conditions of chunk...\n");
    thisAgent->explanationLogger->print_involved_instantiations();
    /* Create condition and action records */
    bool has_backtrace_num = false;
    instantiation_record* lchunkInstantiation;
    condition_record* lcondRecord;

    for (condition* cond = lhs; cond != NIL; cond = cond->next)
    {
        dprint(DT_EXPLAIN, "Matching chunk condition %l from instantiation i%u (%y)", cond, cond->bt.inst->i_id,
            (cond->bt.inst->prod ? cond->bt.inst->prod->name : thisAgent->fake_instantiation_symbol));
        has_backtrace_num = (cond->bt.trace && cond->bt.trace->inst && (cond->bt.trace->inst->backtrace_number == pBacktraceNumber));
        lcondRecord = thisAgent->explanationLogger->add_condition(conditions, cond, NULL, has_backtrace_num, 0);
        lchunkInstantiation = thisAgent->explanationLogger->get_instantiation(cond->bt.inst);
//        lchunkInstantiation = thisAgent->explanationLogger->add_instantiation(cond->bt.inst, 1);
        /* The backtrace should have already added all instantiations that contained
         * grounds, so we can just look up the instantiation for each condition */
        assert(lchunkInstantiation);
        lcondRecord->set_instantiation(lchunkInstantiation);
    }
    dprint(DT_EXPLAIN, "...done with (4) adding chunk instantiation conditions!\n");

    dprint(DT_EXPLAIN, "(5) Recording actions...\n");

    action_record* new_action_record;
    preference* pref;
    action* lAction;
    for (pref = results, lAction= rhs; (pref != NIL) && (lAction != NIL); pref = pref->next_result, lAction = lAction->next)
    {
        new_action_record = thisAgent->explanationLogger->add_result(pref, lAction);
        actions->push_back(new_action_record);
    }
    identity_set_mappings = new id_to_id_map_type();
    (*identity_set_mappings) = (*pIdentitySetMappings);

    dprint(DT_EXPLAIN, "DONE recording chunk contents...\n");
}

void instantiation_record::connect_conds_to_actions()
{
    dprint(DT_EXPLAIN_CONNECT, "Connecting instantiation conditions for c%u (%y)\n", instantiationID, production_name);
    /* Now that instantiations in backtrace are guaranteed to be recorded, connect
     * each condition to the appropriate parent instantiation action record */
    for (condition_record_list::iterator it = conditions->begin(); it != conditions->end(); it++)
    {
        (*it)->connect_to_action();
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

void Explanation_Logger::print_involved_instantiations()
{
//    struct cmp_iID
//        {
//            bool operator () (const instantiation_record& a, const instantiation_record& b)
//            {
//                  return (a.instantiationID <= b.instantiationID);
//            }
//        };
//    std::set< instantiation_record*, cmp_iID > sorted_set;
////    { std::begin((*instantiations_for_current_chunk)), std::end((*instantiations_for_current_chunk)) };
//    std::copy(std::begin(instantiations_for_current_chunk), std::end(instantiations_for_current_chunk), std::inserter(sorted_set));

    dprint(DT_EXPLAIN, "Involved instantiations: \n");

    for (auto it = instantiations_for_current_chunk->begin(); it != instantiations_for_current_chunk->end(); it++)
    {
        dprint(DT_EXPLAIN, "%u (%y)\n", (*it)->instantiationID, (*it)->production_name);
    }
}

instantiation_record* Explanation_Logger::add_instantiation(instantiation* pInst, uint64_t bt_depth)
{
    if (++bt_depth > EXPLAIN_MAX_BT_DEPTH) return NULL;

    bool lIsTerminalInstantiation = false;

    dprint(DT_EXPLAIN_ADD_INST, "Adding instantation for i%u (%y).\n",
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
        if ((pInst->backtrace_number != backtrace_number) || (bt_depth < EXPLAIN_MAX_BT_DEPTH))
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

action_record::action_record(agent* myAgent, preference* pPref, action* pAction, uint64_t pActionID)
{
    thisAgent               = myAgent;
    actionID                = pActionID;
    instantiated_pref       = shallow_copy_preference(thisAgent, pPref);
    original_pref           = pPref;
    if (pAction)
    {
        variablized_action = copy_action(thisAgent, pAction);
    } else {
        variablized_action = NULL;
    }
    identities_used = NULL;
    dprint(DT_EXPLAIN_CONDS, "   Created action record a%u for pref %p (%r ^%r %r), [act %a]", pActionID, pPref, pPref->rhs_funcs.id, pPref->rhs_funcs.attr, pPref->rhs_funcs.value, pAction);
}

void action_record::update_action(preference* pPref)
{
    original_pref           = pPref;
//    dprint(DT_EXPLAIN_CONDS, "   Updated action record a%u for pref %p (%r ^%r %r)", actionID, pPref, pPref->rhs_funcs.id, pPref->rhs_funcs.attr, pPref->rhs_funcs.value);
}

action_record::~action_record()
{
    dprint(DT_EXPLAIN_CONDS, "   Deleting action record a%u for: %p\n", actionID, instantiated_pref);
    deallocate_preference(thisAgent, instantiated_pref);
    deallocate_action_list(thisAgent, variablized_action);
    if (identities_used)
    {
        delete identities_used;
    }
}

chunk_record::chunk_record(agent* myAgent, uint64_t pChunkID)
{
    thisAgent           = myAgent;
    name                = NULL;
    conditions          = new condition_record_list;
    actions             = new action_record_list;
    chunkID             = pChunkID;
    baseInstantiation   = NULL;
    chunkInstantiation  = NULL;
    original_production = NULL;
    dependency_paths    = NULL;
//    dependency_paths = new condition_to_ipath_map();

    result_instantiations = new inst_set;
    result_inst_records = new inst_record_set;

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
    symbol_remove_ref(thisAgent, name);
//    production_remove_ref(thisAgent, original_production);
    delete conditions;
    delete actions;
    delete result_instantiations;
    delete result_inst_records;
}

void condition_record::connect_to_action()
{
    if (parent_instantiation && cached_pref)
    {
        assert(cached_pref);
        parent_action = parent_instantiation->find_rhs_action(cached_pref);
        assert(parent_action);
        parent_instantiation->connect_conds_to_actions();
        dprint(DT_EXPLAIN_CONNECT, "   Linked condition %u (%t ^%t %t) to a%u in i%u.\n", conditionID, condition_tests.id, condition_tests.attr, condition_tests.value, parent_action->get_actionID(), parent_instantiation->get_instantiationID());
//    } else {
//        dprint(DT_EXPLAIN, "   Did not link condition %u (%t ^%t %t) because no parent instantiation.\n", conditionID, condition_tests.id, condition_tests.attr, condition_tests.value);
    }
//    cached_pref = NULL;
}

void condition_record::update_condition(condition* pCond, instantiation_record* pInst, bool pStopHere, uint64_t bt_depth)
{
//    dprint(DT_EXPLAIN_UPDATE, "   Updating condition c%u for %l.\n", conditionID, pCond);
    if (!matched_wme)
    {
        set_matched_wme_for_cond(pCond);
    }
    cached_pref = pCond->bt.trace;
    cached_wme = pCond->bt.wme_;
    if (!pStopHere && pCond->bt.trace)
    {
        parent_instantiation = thisAgent->explanationLogger->add_instantiation(pCond->bt.trace->inst, bt_depth);
    } else {
        assert(!parent_instantiation);
    }
    parent_action = NULL;
    if (path_to_base) {
        delete path_to_base;
    }
    path_to_base = NULL;
}

void condition_record::set_matched_wme_for_cond(condition* pCond)
{
    /* bt info wme doesn't seem to always exist (maybe just for terminal nodes), so
     * we use actual tests if we know it's a literal condition because identifier is STI */
    if (condition_tests.id->eq_test->data.referent->is_identifier() &&
        !condition_tests.attr->eq_test->data.referent->is_variable() &&
        !condition_tests.attr->eq_test->data.referent->is_variable())
    {
        matched_wme = new symbol_triple(condition_tests.id->eq_test->data.referent, condition_tests.attr->eq_test->data.referent, condition_tests.value->eq_test->data.referent);
        symbol_add_ref(thisAgent, matched_wme->id);
        symbol_add_ref(thisAgent, matched_wme->attr);
        symbol_add_ref(thisAgent, matched_wme->value);
    } else {
        if (pCond->bt.wme_)
        {
            matched_wme = new symbol_triple(pCond->bt.wme_->id, pCond->bt.wme_->attr, pCond->bt.wme_->value);
            symbol_add_ref(thisAgent, matched_wme->id);
            symbol_add_ref(thisAgent, matched_wme->attr);
            symbol_add_ref(thisAgent, matched_wme->value);
        } else {
            matched_wme = NULL;
        }
    }
}

condition_record::condition_record(agent* myAgent, condition* pCond, uint64_t pCondID, bool pStopHere, uint64_t bt_depth)
{
    thisAgent = myAgent;
    conditionID = pCondID;
    type = pCond->type;
    dprint(DT_EXPLAIN_CONDS, "   Creating condition %u for %l.\n", conditionID, pCond);

    condition_tests.id = copy_test(thisAgent, pCond->data.tests.id_test);
    condition_tests.attr = copy_test(thisAgent, pCond->data.tests.attr_test);
    condition_tests.value = copy_test(thisAgent, pCond->data.tests.value_test);

    set_matched_wme_for_cond(pCond);

    if (pCond->bt.level)
    {
        wme_level_at_firing = pCond->bt.level;
    } else if (condition_tests.id->eq_test->data.referent->is_identifier())
    {
        assert (condition_tests.id->eq_test->data.referent->id->level);
        wme_level_at_firing = condition_tests.id->eq_test->data.referent->id->level;
        dprint(DT_EXPLAIN_CONDS, "   No backtrace level found.  Setting condition level to id's current level.\n", wme_level_at_firing);
    } else {
        wme_level_at_firing = 0;
        dprint(DT_EXPLAIN_CONDS, "   No backtrace level or sti identifier found.  Setting condition level to 0.\n", wme_level_at_firing);
    }

    /* Cache the pref to make it easier to connect this condition to the action that created
     * the preference later. Tricky because NCs and NCCs have neither and architectural
     * may have niether */
    cached_pref = pCond->bt.trace;
    cached_wme = pCond->bt.wme_;
    if (cached_wme) {
        dprint(DT_EXPLAIN_CONDS, "   Make sure all wme's can print here: %w.\n", cached_wme);
    }
    if (conditionID == 471) {
        dprint(DT_EXPLAIN_CONDS, "Comparing against condition %u", conditionID);
    }

    assert(!(cached_pref && !cached_wme));
    if (!cached_pref && pCond->counterpart) {
        dprint(DT_EXPLAIN_CONDS, "   Chunk condition without pref found.\n");
    }
    if (!pStopHere && pCond->bt.trace)
    {
        parent_instantiation = thisAgent->explanationLogger->add_instantiation(pCond->bt.trace->inst, bt_depth);

        // Crude way to print a dependency chart
        //        Output_Manager::Get_OM().set_column_indent(0, (bt_depth * 3));
        //        dprint(DT_EXPLAIN, "%-%u\n", pCond->bt.trace->inst->i_id);
        //        dependency_chart.append(((pDepth + 1) * 3) , ' ');
        //        std::string new_entry;
        //        thisAgent->outputManager->sprinta_sf(thisAgent, new_entry, "%u (%y)\n", pInstRecord->instantiationID, pInstRecord->production_name);
        //        dependency_chart.append(new_entry);
    } else {
        parent_instantiation = NULL;
    }
    parent_action = NULL;
    path_to_base = NULL;
    dprint(DT_EXPLAIN_CONDS, "   Created condition %u DONE.\n", conditionID);
}

condition_record::~condition_record()
{
    dprint(DT_EXPLAIN_CONDS, "   Deleting condition record c%u for: (%t ^%t %t)\n", conditionID, condition_tests.id, condition_tests.attr, condition_tests.value);

    /* MToDo | I think the conditions in the chunk record don't get cleared b/c not connected */
    //    assert(!cached_pref);

    deallocate_test(thisAgent, condition_tests.id);
    deallocate_test(thisAgent, condition_tests.attr);
    deallocate_test(thisAgent, condition_tests.value);
    if (matched_wme)
    {
        dprint(DT_EXPLAIN_CONDS, "   Removing references for matched wme: (%y ^%y %y)\n", matched_wme->id, matched_wme->attr, matched_wme->value);
        symbol_remove_ref(thisAgent, matched_wme->id);
        symbol_remove_ref(thisAgent, matched_wme->attr);
        symbol_remove_ref(thisAgent, matched_wme->value);
        delete matched_wme;
    }
    if (path_to_base)
    {
        delete path_to_base;
    }
}


instantiation_record::instantiation_record(agent* myAgent, instantiation* pInst)
{
    thisAgent           = myAgent;
    instantiationID     = pInst->i_id;
    match_level         = pInst->match_goal_level;
    conditions          = new condition_record_list;
    actions             = new action_record_list;
    original_production = pInst->prod;
    production_name     = (pInst->prod ? pInst->prod->name : thisAgent->fake_instantiation_symbol);

    symbol_add_ref(thisAgent, production_name);
    if (pInst->prod)
    {
        original_production->save_for_justification_explanation = true;
    }
}

instantiation_record::~instantiation_record()
{
    dprint(DT_EXPLAIN, "Deleting instantiation record i%u (%y)\n", instantiationID, production_name);
    symbol_remove_ref(thisAgent, production_name);
    delete conditions;
    delete actions;
}

void instantiation_record::record_instantiation_contents(instantiation* pInst, bool pStopHere, uint64_t bt_depth)
{
    dprint(DT_EXPLAIN_ADD_INST, "- Recording instantiation contents for i%u (%y)\n", pInst->i_id, production_name);
    ++bt_depth;
    /* Create condition and action records */
    for (condition* cond = pInst->top_of_instantiated_conditions; cond != NIL; cond = cond->next)
    {
        thisAgent->explanationLogger->add_condition(conditions, cond, this, pStopHere, bt_depth);
    }

    action_record* new_action_record;
    for (preference* pref = pInst->preferences_generated; pref != NIL; pref = pref->inst_next)
    {
        new_action_record = thisAgent->explanationLogger->add_result(pref);
        actions->push_front(new_action_record);
    }
}

void instantiation_record::update_instantiation_contents(instantiation* pInst, bool pStopHere, uint64_t bt_depth)
{
    dprint(DT_EXPLAIN_UPDATE, "- Updating instantiation contents for i%u (%y)\n", pInst->i_id, production_name);
    ++bt_depth;
    /* Update condition and action records */
    condition_record* lCondRecord;
    condition* cond;

    cond = pInst->top_of_instantiated_conditions;
    for (condition_record_list::iterator it = conditions->begin(); it != conditions->end() && cond != NIL; it++, cond = cond->next)
    {
        lCondRecord = (*it);
        lCondRecord->update_condition(cond, this, pStopHere, bt_depth);
    }

    dprint(DT_EXPLAIN_UPDATE, "   -->\n");

    action_record* lActionRecord;
    preference* pref = pInst->preferences_generated;
    for (action_record_list::iterator it = actions->begin(); pref != NIL && it != actions->end(); it++, pref = pref->inst_next)
    {
        lActionRecord = (*it);
        lActionRecord->update_action(pref);
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
            dprint(DT_EXPLAIN_CONDS, "...found RHS action a%u for condition preference %p.\n", (*iter)->get_actionID(), pPref);
        }
        return (*iter);
    }
    dprint(DT_EXPLAIN_CONNECT, "...did not find pref %p among:\n", pPref);
    for (iter = actions->begin(); iter != actions->end(); ++iter)
    {
            dprint(DT_EXPLAIN_CONNECT, "      %p\n", (*iter)->original_pref);
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
