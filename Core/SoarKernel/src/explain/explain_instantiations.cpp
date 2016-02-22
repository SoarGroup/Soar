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
        if ((*it)->get_conditionID() == 24) {
            dprint(DT_EXPLAIN_PATHS, "Comparing against condition %u", (*it)->get_conditionID());
        }
        if (lp)
        {
            dprint_noprefix(DT_EXPLAIN_PATHS, " %p", lp);
        }
        if (lw)
        {
            dprint_noprefix(DT_EXPLAIN_PATHS, " %w", lw);
        }
        dprint_noprefix(DT_EXPLAIN_PATHS, "\n");

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

void instantiation_record::create_identity_paths(const inst_record_list* pInstPath, action_record* pAction, bool pUseId, bool pUseAttr, bool pUseValue)
{
    inst_record_list* lInstPath = new inst_record_list();
    (*lInstPath) = (*pInstPath);
    lInstPath->push_back(this);

    id_set* lIdsFound;
    int lNumIdsFounds = 0;
    if (pAction)
    {
        /* Compile a list of identities in rhs action */
        lIdsFound = pAction->get_identities();
        lNumIdsFounds = lIdsFound->size();
    }

    dprint(DT_EXPLAIN_PATHS, "Traversing conditions of i%u (%y) looking for %d IDs...\n", instantiationID, production_name, lNumIdsFounds);
    /* If no pAction is passed in, this must be from the base instantiation or
     * the CDPS, so we process all conditions.
     *
     * If pAction but !lIdsFound, then the action is not connected to any LHS
     * conditions, so we don't process any conditions
     *
     * If pAction but !lIdsFound, then we look for conditions that contain any
     * of the identities in lIdsFound and call function recursively on those */
    if (!pAction || lNumIdsFounds)
    {
        bool lAffectedCondition = !pAction;
        for (condition_record_list::iterator it = conditions->begin(); it != conditions->end(); it++)
        {
            if (lNumIdsFounds)
            {
                lAffectedCondition = (*it)->contains_identity_from_set(lIdsFound);
            }

            if (lAffectedCondition || ((match_level > 0) && ((*it)->get_level() < match_level)))
            {
                dprint(DT_EXPLAIN_PATHS, "   Found affected condition %u in i%u.  Recording its dependencies...%s\n", (*it)->get_conditionID(), instantiationID,
                    ((match_level > 0) && ((*it)->get_level() < match_level)) ? "Super" : "Local");
                (*it)->create_identity_paths(lInstPath);
            }
            dprint(DT_EXPLAIN_PATHS, "   path_to_base for condition %u in i%u is now: ", (*it)->get_conditionID(), instantiationID);
//            (*it)->print_path_to_base();
            dprint(DT_EXPLAIN_PATHS, "\n");

        }
    }
    if (lInstPath)
    {
        dprint(DT_EXPLAIN_PATHS, "Deleting lInstPath of size %d for instantiation record %u.\n", lInstPath->size(), instantiationID);
        delete lInstPath;
    }
    /* Note:  lIdsFound will be deleted by the action record.  That way they can be
     *        re-used, since they won't change between different chunks explained */
}
