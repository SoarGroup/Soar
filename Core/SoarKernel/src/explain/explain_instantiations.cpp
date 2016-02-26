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

instantiation_record::instantiation_record(agent* myAgent, instantiation* pInst)
{
    thisAgent           = myAgent;
    instantiationID     = pInst->i_id;
    match_level         = pInst->match_goal_level;
    conditions          = new condition_record_list;
    actions             = new action_record_list;
    original_production = pInst->prod;
    terminal            = false;
    production_name     = (pInst->prod ? pInst->prod->name : thisAgent->fake_instantiation_symbol);
    cached_inst         = pInst;
    path_to_base        = NULL;
    symbol_add_ref(thisAgent, production_name);

    if (pInst->prod)
    {
        original_production->save_for_justification_explanation = true;
    }

    action_record* new_action_record;
    for (preference* pref = pInst->preferences_generated; pref != NIL; pref = pref->inst_next)
    {
        new_action_record = thisAgent->explanationLogger->add_result(pref);
        actions->push_front(new_action_record);
    }

}

instantiation_record::~instantiation_record()
{
    dprint(DT_EXPLAIN, "Deleting instantiation record i%u (%y)\n", instantiationID, production_name);
    symbol_remove_ref(thisAgent, production_name);
    delete conditions;
    delete actions;
    if (path_to_base)
    {
        delete path_to_base;
    }
}

void instantiation_record::delete_instantiation()
{
    for (auto it = conditions->begin(); it != conditions->end(); it++)
    {
        thisAgent->explanationLogger->delete_condition((*it)->get_conditionID());
    }
    for (auto it = actions->begin(); it != actions->end(); it++)
    {
        thisAgent->explanationLogger->delete_action((*it)->get_actionID());
    }
}

void instantiation_record::record_instantiation_contents()
{
    if (cached_inst->i_id == 589)
    {
        dprint(DT_EXPLAIN, "Found. %d\n", condition_count(cached_inst->top_of_instantiated_conditions));
    }
    dprint(DT_EXPLAIN_ADD_INST, "- Recording instantiation contents for i%u (%y)\n", cached_inst->i_id, production_name);
    /* Create condition and action records */
    for (condition* cond = cached_inst->top_of_instantiated_conditions; cond != NIL; cond = cond->next)
    {
        condition_record* lCondRecord = thisAgent->explanationLogger->add_condition(conditions, cond, this);
        lCondRecord->connect_to_action();
    }
    if (cached_inst->i_id == 589)
    {
        dprint(DT_EXPLAIN, "Found. %d\n", conditions->size());
    }
}

void instantiation_record::update_instantiation_contents()
{
    dprint(DT_EXPLAIN_UPDATE, "- Updating instantiation contents for i%u (%y)\n", cached_inst->i_id, production_name);
    /* Update condition and action records */
    condition_record* lCondRecord;
    condition* cond;

    cond = cached_inst->top_of_instantiated_conditions;
    for (condition_record_list::iterator it = conditions->begin(); it != conditions->end() && cond != NIL; it++, cond = cond->next)
    {
        lCondRecord = (*it);
        lCondRecord->update_condition(cond, this);
        /* MToDo | Can the connections ever really change?  I think a new instantiation would be created if that were the case.
         *          Test this out.*/
        lCondRecord->connect_to_action();
    }
}

action_record* instantiation_record::find_rhs_action(preference* pPref)
{
    action_record_list::iterator iter;

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

condition_record* instantiation_record::find_condition_for_chunk(preference* pPref, wme* pWME)
{
    dprint(DT_EXPLAIN_PATHS, "Looking for condition in i%u: ", instantiationID);
    if (pPref)
    {
        dprint_noprefix(DT_EXPLAIN_PATHS, " pref %p", pPref);
    }
    if (pWME)
    {
        dprint_noprefix(DT_EXPLAIN_PATHS, " wme %w", pWME);
    }
    dprint_noprefix(DT_EXPLAIN_PATHS, "\n");
    for (condition_record_list::iterator it = conditions->begin(); it != conditions->end(); it++)
    {
        dprint(DT_EXPLAIN_PATHS, "Comparing against condition %u", (*it)->get_conditionID());
        condition_record* lit = (*it);
        preference* lp = lit->get_cached_pref();
        wme* lw = lit->get_cached_wme();
        uint64_t li = (*it)->get_conditionID();
        if (pPref && ((*it)->get_cached_pref() == pPref))
        {
            dprint(DT_EXPLAIN_PATHS, "Found condition %u %p for target preference %p\n", (*it)->get_conditionID(), (*it)->get_cached_pref(), pPref);
            return (*it);
        } else if (pWME && ((*it)->get_cached_wme() == pWME))
        {
            dprint(DT_EXPLAIN_PATHS, "Found condition %u %w for target wme %w\n", (*it)->get_conditionID(), (*it)->get_cached_wme(), pWME);
            return (*it);
        }
    }
//    assert(false);
    return NULL;
}

void instantiation_record::create_identity_paths(const inst_record_list* pInstPath)
{
    if (!path_to_base)
    {
        path_to_base = new inst_record_list();
    }
    else {
        int size1 = path_to_base->size();
        int size2 = pInstPath->size();
        if (path_to_base->size() <= pInstPath->size())
        {
            return;
        }
    }

    (*path_to_base) = (*pInstPath);
    path_to_base->push_back(this);
    instantiation_record* lParentInst;
    dprint(DT_EXPLAIN_PATHS, "Adding paths recursively for conditions of i%u (%y)...\n", instantiationID, production_name);
    for (condition_record_list::iterator it = conditions->begin(); it != conditions->end(); it++)
    {
        lParentInst = (*it)->get_parent_inst();
        if (lParentInst && (lParentInst->get_match_level() == match_level))
        {
//            if ((match_level > 0) && ((*it)->get_level() < match_level))
//            {
                lParentInst->create_identity_paths(path_to_base);
//            } else {
//                dprint(DT_EXPLAIN_PATHS, "...not recursing because match level is 0 or condition level < match level\n");
//                dprint(DT_EXPLAIN_PATHS, "...%d >= %d...\n", match_level, (*it)->get_level());
//            }
//            path_to_base->pop_back();
        } else {
            dprint(DT_EXPLAIN_PATHS, "...not recursing because no parent or parent != match level\n");
            dprint(DT_EXPLAIN_PATHS, "...%u: %d != %d...\n", lParentInst, (lParentInst ? lParentInst->get_match_level() : 0), match_level);
        }
    }
}
