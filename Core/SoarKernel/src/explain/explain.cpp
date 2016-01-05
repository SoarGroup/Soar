#include "explain.h"
#include "agent.h"
#include "condition.h"
#include "debug.h"
#include "instantiation.h"
#include "preference.h"
#include "production.h"
#include "rhs.h"
#include "output_manager.h"
#include "working_memory.h"

Explanation_Logger::Explanation_Logger(agent* myAgent)
{
    /* Cache agent and Output Manager pointer */
    thisAgent = myAgent;
    outputManager = &Output_Manager::Get_OM();

    initialize_counters();

    /* Create data structures used for EBC */
    chunks = new std::unordered_map< Symbol*, chunk_record* >();
    instantiations = new std::unordered_map< uint64_t, instantiation_record* >();
    conditions = new std::unordered_map< uint64_t, condition_record* >();
    actions = new std::unordered_map< uint64_t, action_record* >();
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
        delete it->second;
    }
    chunks->clear();

    dprint(DT_EXPLAIN, "Explanation logger clearing instantiation records...\n");
    for (std::unordered_map< uint64_t, instantiation_record* >::iterator it = (*instantiations).begin(); it != (*instantiations).end(); ++it)
    {
//        thisAgent->memoryManager->free_with_pool(MP_attachments, it->second);
        delete it->second;
    }
    instantiations->clear();

    dprint(DT_EXPLAIN, "Explanation logger clearing condition records...\n");
    for (std::unordered_map< uint64_t, condition_record* >::iterator it = (*conditions).begin(); it != (*conditions).end(); ++it)
    {
//        thisAgent->memoryManager->free_with_pool(MP_attachments, it->second);
        delete it->second;
    }
    conditions->clear();

    dprint(DT_EXPLAIN, "Explanation logger clearing action records...\n");
    for (std::unordered_map< uint64_t, action_record* >::iterator it = (*actions).begin(); it != (*actions).end(); ++it)
    {
//        thisAgent->memoryManager->free_with_pool(MP_attachments, it->second);
        delete it->second;
    }
    actions->clear();
}

Explanation_Logger::~Explanation_Logger()
{
    dprint(DT_EXPLAIN, "Deleting explanation logger.\n");

    current_recording_chunk = NULL;
    clear_explanations();

    delete chunks;
    delete conditions;
    delete actions;
    delete instantiations;

}

void Explanation_Logger::re_init()
{
    dprint(DT_EXPLAIN, "Re-intializing explanation logger.\n");
    clear_explanations();
    initialize_counters();
    current_recording_chunk = NULL;
    dprint(DT_EXPLAIN, "Done re-intializing explanation logger.\n");

}

chunk_record* Explanation_Logger::add_chunk_record(instantiation* pBaseInstantiation)
{
    dprint(DT_EXPLAIN, "Adding chunk record for %y (i%u)\n", (pBaseInstantiation->prod ? pBaseInstantiation->prod->name : thisAgent->fake_instantiation_symbol), pBaseInstantiation->i_id);
    chunk_record* lChunkRecord = new chunk_record(thisAgent, pBaseInstantiation, chunk_id_count++);

    current_recording_chunk = lChunkRecord;

    total_recorded.chunks++;

    return lChunkRecord;
}

void chunk_record::record_chunk_contents(Symbol* pName, condition* lhs, action* rhs, preference* results)
{
    name = pName;
    symbol_add_ref(thisAgent, name);

    conditions         = new condition_record_list;
    actions            = new action_record_list;

    /* Create condition and action records */
    condition_record* new_cond_record;
    for (condition* cond = lhs; cond != NIL; cond = cond->next)
    {
        new_cond_record = thisAgent->explanationLogger->add_condition(cond);
        conditions->push_front(new_cond_record);
    }

    dprint(DT_EXPLAIN, "   -->\n");

    action_record* new_action_record;
    preference* pref;
    action* lAction;
    for (pref = results, lAction= rhs; (pref != NIL) && (lAction != NIL); pref = pref->next_result, lAction = lAction->next)
    {
        new_action_record = thisAgent->explanationLogger->add_result(pref, lAction);
        actions->push_front(new_action_record);
    }
}

void Explanation_Logger::record_chunk_contents(Symbol* pName, condition* lhs, action* rhs, preference* results)
{
    dprint(DT_EXPLAIN, "Recording chunk/justification contents for %s c%u\n", pName, current_recording_chunk->chunkID);

    current_recording_chunk->record_chunk_contents(pName, lhs, rhs, results);
    (*chunks)[pName] = current_recording_chunk;

}

condition_record* Explanation_Logger::add_condition(condition* pCond)
{
    dprint(DT_EXPLAIN, "   Creating condition: %l\n", pCond);
    condition_record* lCondRecord = new condition_record(thisAgent, pCond, condition_id_count++);
    (*conditions)[lCondRecord->conditionID] = lCondRecord;

    total_recorded.conditions++;

    return lCondRecord;
}

instantiation_record* Explanation_Logger::add_instantiation(instantiation* pInst)
{
    instantiation_record* lInstRecord;
    lInstRecord = get_instantiation(pInst);
    if (lInstRecord)
    {
        dprint(DT_EXPLAIN, "Found existing instantiation record for %y (i%u)\n", (pInst->prod ? pInst->prod->name : thisAgent->fake_instantiation_symbol), pInst->i_id);
        total_recorded.instantiations_referenced++;
        return lInstRecord;
    }

    dprint(DT_EXPLAIN, "Adding new instantiation record for %y (i%u)\n", (pInst->prod ? pInst->prod->name : thisAgent->fake_instantiation_symbol), pInst->i_id);
    lInstRecord = new instantiation_record(thisAgent, pInst);
    (*instantiations)[pInst->i_id] = lInstRecord;

    dprint(DT_EXPLAIN, "Returning new explanation instantiation record for %y (i%u)\n", (pInst->prod ? pInst->prod->name : thisAgent->fake_instantiation_symbol), pInst->i_id);
    total_recorded.instantiations++;
    return lInstRecord;
}

action_record* Explanation_Logger::add_result(preference* pPref, action* pAction)
{
    dprint(DT_EXPLAIN, "   Adding result: %p\n", pPref);
    action_record* lActionRecord = new action_record(thisAgent, pPref, pAction, action_id_count++);
    (*actions)[lActionRecord->actionID] = lActionRecord;

    total_recorded.actions++;

    return lActionRecord;
}

instantiation_record* Explanation_Logger::get_instantiation(instantiation* pInst)
{
    assert(pInst);

    /* See if we already have an instantiation record */
    std::unordered_map< uint64_t, instantiation_record* >::iterator iter_inst;

    dprint(DT_EXPLAIN, "...Looking  for instantiation id %u...", pInst->i_id);

    iter_inst = instantiations->find(pInst->i_id);
    if (iter_inst != instantiations->end())
    {
        dprint(DT_EXPLAIN, "...found existing ebc logger record.\n");
        return(iter_inst->second);
    }
    dprint(DT_EXPLAIN, "...not found..\n");
    return NULL;
}
action_record* Explanation_Logger::get_action_for_instantiation(preference* pPref, instantiation* pInst)
{
    assert(pInst); // Just to see if this can happen normally.
    if (!pInst) return NULL;

    /* Instantiations backtrace_number are marked as dependency analysis is performed, so
     * we can use that to determine if this instantiation needs to be added. */
    if (pInst->backtrace_number != backtrace_number)
    {
        total_recorded.instantiations_skipped++;
        return NULL;
    }

    /* Find instantiation record */
    instantiation_record* lInstRecord = add_instantiation(pInst);

    /* Find action record that matches this preference */
    assert(lInstRecord);
    return 0;
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
    }
    dprint(DT_EXPLAIN, "Created action record a%u for pref %p (%r ^%r %r)\naction %a", pActionID, pPref, pPref->rhs_funcs.id, pPref->rhs_funcs.attr, pPref->rhs_funcs.value, pAction);
}

action_record::~action_record()
{
    dprint(DT_EXPLAIN, "Deleting action record a%u for: %p\n", actionID, instantiated_pref);
    deallocate_preference(thisAgent, instantiated_pref);
//    deallocate_action_list(thisAgent, variablized_action);
}

chunk_record::chunk_record(agent* myAgent, instantiation* pBaseInstantiation, uint64_t pChunkID)
{
    thisAgent           = myAgent;
    name                = NULL;
    conditions          = new condition_record_list;
    actions             = new action_record_list;
    chunkID             = pChunkID;
    baseInstantiation   = thisAgent->explanationLogger->add_instantiation(pBaseInstantiation);
    dprint(DT_EXPLAIN, "Created chunk record c%u\n", chunkID);
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

condition_record::condition_record(agent* myAgent, condition* pCond, uint64_t pCondID)
{
    thisAgent = myAgent;
    conditionID = pCondID;
    instantiated_cond = copy_condition(thisAgent, pCond, false, false);
    variablized_cond = copy_condition(thisAgent, pCond, false, false);
    //substitute_explanation_variables(variablized_cond);
    if (pCond->bt.wme_)
    {
        matched_wme = new soar_module::symbol_triple;
        matched_wme->id = pCond->bt.wme_->id;
        matched_wme->attr = pCond->bt.wme_->attr;
        matched_wme->value = pCond->bt.wme_->value;
        symbol_add_ref(thisAgent, matched_wme->id);
        symbol_add_ref(thisAgent, matched_wme->attr);
        symbol_add_ref(thisAgent, matched_wme->value);
    } else {
        matched_wme = NULL;
    }
    if (pCond->bt.trace)
    {
        parent_action = thisAgent->explanationLogger->get_action_for_instantiation(pCond->bt.trace, pCond->bt.trace->inst);
    } else {
        parent_action = NULL;
    }
}

condition_record::~condition_record()
{
    dprint(DT_EXPLAIN, "Deleting condition record c%u for: %l\n", conditionID, variablized_cond);
    deallocate_condition(thisAgent, instantiated_cond);
    deallocate_condition(thisAgent, variablized_cond);
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

    dprint(DT_EXPLAIN, "Created instantiation record c%u\n", pInst->i_id);
    /* Create condition and action records */
    condition_record* new_cond_record;
    for (condition* cond = pInst->top_of_instantiated_conditions; cond != NIL; cond = cond->next)
    {
        new_cond_record = thisAgent->explanationLogger->add_condition(cond);
        conditions->push_front(new_cond_record);
    }

    dprint(DT_EXPLAIN, "   -->\n");

    action_record* new_action_record;
    for (preference* pref = pInst->preferences_generated; pref != NIL; pref = pref->next_result)
    {
        new_action_record = thisAgent->explanationLogger->add_result(pref);
        actions->push_front(new_action_record);
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

action_record* instantiation_record::find_rhs_action(preference* pPref)
{
    action_record_list::iterator iter;

    dprint(DT_EXPLAIN, "...Looking  for preference action %p...", pPref);
    for (iter = actions->begin(); iter != actions->end(); ++iter)
    {
        if ((*iter)->original_pref == pPref)
        {
            dprint(DT_EXPLAIN, "...found action! %u.\n", (*iter)->get_actionID());
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
