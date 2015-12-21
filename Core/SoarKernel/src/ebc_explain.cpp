#include "ebc_explain.h"

#include "condition.h"
#include "debug.h"
#include "instantiations.h"
#include "prefmem.h"
#include "production.h"
#include "output_manager.h"
#include "wmem.h"

Explanation_Logger::Explanation_Logger(agent* myAgent)
{
    /* Cache agent and Output Manager pointer */
    thisAgent = myAgent;
    outputManager = &Output_Manager::Get_OM();

    /* Initialize instantiation and identity ID counters */
    chunks_attempted_count = 0;
    duplicate_chunks_count = 0;
    merge_count = 0;
    chunk_id_count = 1;
    condition_id_count = 1;
    action_id_count = 1;

    /* Create data structures used for EBC */
    chunks = new std::unordered_map< uint64_t, chunk_record* >();
    instantiations = new std::unordered_map< uint64_t, instantiation_record* >();
    conditions = new std::unordered_map< uint64_t, condition_record* >();
    actions = new std::unordered_map< uint64_t, action_record* >();
}

Explanation_Logger::~Explanation_Logger()
{
    dprint(DT_EXPLAIN, "Cleaning up explanation logger.\n");
}

void Explanation_Logger::re_init()
{
    dprint(DT_EXPLAIN, "Cleaning up explanation logger.\n");
}

uint64_t Explanation_Logger::add_chunk_record(instantiation* pBaseInstantiation)
{
    dprint(DT_EXPLAIN, "Adding chunk record for %y (ri%u)\n", pBaseInstantiation->prod->name, pBaseInstantiation->i_id);
    chunk_record* lChunkRecord = new chunk_record;
    lChunkRecord->baseInstantiation = add_instantiation(pBaseInstantiation);
    lChunkRecord->conditions         = new condition_record_list;
    lChunkRecord->actions            = new action_record_list;
    lChunkRecord->chunkID = chunk_id_count++;
    dprint(DT_EXPLAIN, "Returning chunk record id %u\n", lChunkRecord->chunkID);
    (*chunks)[lChunkRecord->chunkID] = lChunkRecord;

    return lChunkRecord->chunkID;
}

instantiation_record* Explanation_Logger::add_instantiation(instantiation* pInst)
{
    instantiation_record* lInstRecord;
    lInstRecord = get_instantiation(pInst);
    if (lInstRecord)
    {
        dprint(DT_EXPLAIN, "Found existing instantiation record for %y (ri%u)\n", pInst->prod->name, pInst->i_id);
        return lInstRecord;
    }

    dprint(DT_EXPLAIN, "Adding new instantiation record for %y (ri%u)\n", pInst->prod->name, pInst->i_id);
    lInstRecord = new instantiation_record;
    (*instantiations)[pInst->i_id] = lInstRecord;

    lInstRecord->production_name    = pInst->prod->name;
    lInstRecord->conditions         = new condition_record_list;
    lInstRecord->actions            = new action_record_list;

    /* Create condition and action records */
    condition_record* new_cond_record;
    for (condition* cond = pInst->top_of_instantiated_conditions; cond != NIL; cond = cond->next)
    {
        new_cond_record = add_condition(cond);
        lInstRecord->conditions->push_front(new_cond_record);
    }

    dprint(DT_EXPLAIN, "   -->\n");

    action_record* new_action_record;
    for (preference* pref = pInst->preferences_generated; pref != NIL; pref = pref->next_result)
    {
        new_action_record = add_result(pref);
        lInstRecord->actions->push_front(new_action_record);
    }

    dprint(DT_EXPLAIN, "Returning new explanation instantiation record i%u\n", pInst->i_id);
    return lInstRecord;
}

condition_record* Explanation_Logger::add_condition(condition* pCond)
{
    dprint(DT_EXPLAIN, "   Adding condition: %l\n", pCond);
    condition_record* lCondRecord = new condition_record;
    lCondRecord->conditionID = condition_id_count++;
    lCondRecord->instantiated_cond = copy_condition(thisAgent, pCond, false, false);
    lCondRecord->variablized_cond = copy_condition(thisAgent, pCond, false, false);
    //substitute_explanation_variables(lCondRecord->variablized_cond);
    lCondRecord->matched_wme = new soar_module::symbol_triple;
    lCondRecord->matched_wme->id = pCond->bt.wme_->id;
    lCondRecord->matched_wme->attr = pCond->bt.wme_->attr;
    lCondRecord->matched_wme->value = pCond->bt.wme_->value;
    symbol_add_ref(thisAgent, lCondRecord->matched_wme->id);
    symbol_add_ref(thisAgent, lCondRecord->matched_wme->attr);
    symbol_add_ref(thisAgent, lCondRecord->matched_wme->value);
    if (pCond->bt.trace)
    {
        lCondRecord->parent_action = get_action_for_instantiation(pCond->bt.trace, pCond->bt.trace->inst);
    } else {
        lCondRecord->parent_action = NULL;
    }
    return lCondRecord;
}

void Explanation_Logger::delete_condition(condition_record* lCondRecord)
{
    dprint(DT_EXPLAIN, "   deleting condition: %u\n", lCondRecord->conditionID);
    deallocate_condition(thisAgent, lCondRecord->instantiated_cond);
    deallocate_condition(thisAgent, lCondRecord->variablized_cond);
    symbol_remove_ref(thisAgent, lCondRecord->matched_wme->id);
    symbol_remove_ref(thisAgent, lCondRecord->matched_wme->attr);
    symbol_remove_ref(thisAgent, lCondRecord->matched_wme->value);
    delete lCondRecord->matched_wme;
}

action_record* Explanation_Logger::add_result(preference* pPref)
{
    dprint(DT_EXPLAIN, "   Adding result: %p\n", pPref);
    action_record* lActionRecord = new action_record;
    lActionRecord->actionID = action_id_count++;
    lActionRecord->instantiated_pref = copy_preference(thisAgent, pPref);
    lActionRecord->original_pref = pPref;
    lActionRecord->variablized_action = NULL;
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
        return NULL;
    }

    /* Find instantiation record */
    instantiation_record* lInstRecord = add_instantiation(pInst);

    /* Find action record that matches this preference */
    assert(lInstRecord);
    action_record_list::iterator iter;

    dprint(DT_EXPLAIN, "...Looking  for preference action %p...", pPref);
    for (iter = lInstRecord->actions->begin(); iter != lInstRecord->actions->end(); ++iter)
    {
        if ((*iter)->original_pref == pPref)
        {
            dprint(DT_EXPLAIN, "...found action! %u.\n", (*iter)->actionID);
        }
        return (*iter);
    }
    dprint(DT_EXPLAIN, "...did not find pref %p among:\n", pPref);
    for (iter = lInstRecord->actions->begin(); iter != lInstRecord->actions->end(); ++iter)
    {
            dprint(DT_EXPLAIN, "      %p\n", (*iter)->original_pref);
    }

    return 0;
}

uint64_t Explanation_Logger::add_condition_to_chunk_record(condition* pCond, chunk_record* pChunkRecord)
{
    dprint(DT_EXPLAIN, "   Adding condition to chunk c%u: %l", pChunkRecord->chunkID, pCond);

    return 0;
}

uint64_t Explanation_Logger::add_result_to_chunk_record(action* pAction, preference* pPref, chunk_record* pChunkRecord)
{
    dprint(DT_EXPLAIN, "   Adding result to chunk c%u: %a %p", pChunkRecord->chunkID, pAction, pPref);
    return 0;
}
