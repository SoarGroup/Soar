/*
 * variablization_manager_map.cpp
 *
 *  Created on: Jul 25, 2013
 *      Author: mazzin
 */

#include "ebc.h"
#include "ebc_timers.h"

#include "agent.h"
#include "condition.h"
#include "dprint.h"
#include "explanation_memory.h"
#include "instantiation.h"
#include "output_manager.h"
#include "preference.h"
#include "print.h"
#include "rhs.h"
#include "rhs_functions.h"
#include "symbol.h"
#include "symbol_manager.h"
#include "test.h"
#include "working_memory.h"

#include <assert.h>

identity_join* Explanation_Based_Chunker::make_join_set(uint64_t pIdentity)
{
    identity_join* new_join_set = new identity_join();
//    increment_counter(ovar_id_counter);
//    new_join_set->identity = ovar_id_counter;
    new_join_set->identity = pIdentity;
    new_join_set->clone_identity = NULL_IDENTITY_SET;
    new_join_set->new_var = NULL;
    new_join_set->super_join = new_join_set;
    new_join_set->refcount = 1;
    new_join_set->literalization_checked = false;
    new_join_set->literalized = false;
    identity_sets_to_clean_up.push_back(new_join_set);
    return new_join_set;
}

void Explanation_Based_Chunker::clean_up_identity_sets()
{
    for (auto it = identity_sets_to_clean_up.begin(); it != identity_sets_to_clean_up.end(); it++)
    {
        identity_join* lJoin_set = *it;
        if (lJoin_set->super_join != lJoin_set)
        {
            join_set_remove_ref(lJoin_set->super_join);
            lJoin_set->super_join = lJoin_set;
        }
        lJoin_set->identity_sets.clear();
        if (lJoin_set->new_var)
        {
            thisAgent->symbolManager->symbol_remove_ref(&lJoin_set->new_var);
        }
        lJoin_set->refcount = 1;
        lJoin_set->literalization_checked = false;
        lJoin_set->literalized = false;
        if (lJoin_set->constraints) delete lJoin_set->constraints;
    }
    identity_sets_to_clean_up.clear();
}

void Explanation_Based_Chunker::join_identity_sets(identity_join* lFromJoinSet, identity_join* lToJoinSet)
{
    assert(ebc_settings[SETTING_EBC_LEARNING_ON]);
    assert(lFromJoinSet && (lFromJoinSet != lToJoinSet));

    ebc_timers->variablization_rhs->start();
    ebc_timers->variablization_rhs->stop();
    ebc_timers->identity_unification->start();

    /* MToDo | If we always choose to map from the smaller number to the highest number, maybe we can avoid this check.  It would be
     *         impossible to get an inverse mapping, and we will just re-assign an identical mapping which costs the same as checking.
     *         We'd have to resolve the transitive mappings later. */
    if (lToJoinSet == NULL)
    {
        dprint(DT_ADD_IDENTITY_SET_MAPPING, "Literalizing identity set %u n", lFromJoinSet->super_join->identity);
        literalized_identity_sets.insert(lFromJoinSet->super_join);
        /* MToDo | If it has a join set, we may want to store that it has been literalized.  Can avoid lookup later */
        return;
    }

    dprint(DT_ADD_IDENTITY_SET_MAPPING, "Combining two join sets for %u and %u...\n", lFromJoinSet->super_join->identity, lToJoinSet->super_join->identity);

    /* Swapping to consistently favor keeping the join set if bigger joins and higher identities set value
     * otherwise.  Not sure if this, especially the latter case, will really help efficiency
     * especially with forward propagation determining identity set values */
    uint64_t lTargetID = lToJoinSet->super_join->identity;
    if (lFromJoinSet->identity_sets.size() > lToJoinSet->identity_sets.size())
    {
        dprint(DT_ADD_IDENTITY_SET_MAPPING, "Swapping join sets so that %u is target and not %u\n", lFromJoinSet->super_join->identity, lToJoinSet->super_join->identity);
        lTargetID = lFromJoinSet->super_join->identity;
        identity_join* tempJoin = lFromJoinSet;
        lFromJoinSet = lToJoinSet;
        lToJoinSet = tempJoin;
    }

    // Iterate through identity sets in lFromJoinSet and set their super join set point to lToJoinSet
    cons* last_cons;
    for (auto it = lFromJoinSet->identity_sets.begin(); it != lFromJoinSet->identity_sets.end(); it++)
    {
        dprint(DT_ADD_IDENTITY_SET_MAPPING, "Changing additional join set mapping of %u to %u\n", (*it)->identity, lFromJoinSet->super_join->identity);
        (*it)->super_join = lToJoinSet;
        join_set_remove_ref(lFromJoinSet);
        lToJoinSet->identity_sets.push_back(*it);
        join_set_add_ref(lToJoinSet);
    }
    lFromJoinSet->identity_sets.clear();

    ebc_timers->identity_unification->stop();

//    dprint(DT_ADD_IDENTITY_SET_MAPPING, "New identity propagation map:\n");
//    dprint_identity_to_id_set_map(DT_ADD_IDENTITY_SET_MAPPING);
}

void Explanation_Based_Chunker::literalize_RHS_function_args(const rhs_value rv, uint64_t inst_id)
{
    /* Assign identities of all arguments in rhs fun call to null identity set*/
    cons* fl = rhs_value_to_funcall_list(rv);
    rhs_function_struct* rf = static_cast<rhs_function_struct*>(fl->first);
    cons* c;

    assert(ebc_settings[SETTING_EBC_LEARNING_ON]);

    if (rf->can_be_rhs_value)
    {
        for (c = fl->rest; c != NIL; c = c->rest)
        {
            if (rhs_value_is_funcall(static_cast<char*>(c->first)))
            {
                if (rhs_value_is_literalizing_function(static_cast<char*>(c->first)))
                {
                    dprint(DT_RHS_FUN_VARIABLIZATION, "Recursive call to literalize RHS function argument %r\n", static_cast<char*>(c->first));
                    literalize_RHS_function_args(static_cast<char*>(c->first), inst_id);
                }
            } else {
                dprint(DT_RHS_FUN_VARIABLIZATION, "Literalizing RHS function argument %r ", static_cast<char*>(c->first));
                assert(rhs_value_is_symbol(static_cast<char*>(c->first)));
                rhs_symbol rs = rhs_value_to_rhs_symbol(static_cast<char*>(c->first));
                dprint_noprefix(DT_RHS_FUN_VARIABLIZATION, "[%y %u %u]\n", rs->referent, rs->identity, rs->identity_set);
                if (rs->identity_set && !rs->referent->is_sti())
                {
                    thisAgent->explanationMemory->add_identity_set_mapping(inst_id, IDS_literalized_RHS_function_arg, rs->identity_set, 0);
                    join_identity_sets(rs->identity_set, 0);
                    thisAgent->explanationMemory->increment_stat_rhs_arguments_literalized(m_rule_type);
                }
            }
        }
    }
}

void Explanation_Based_Chunker::unify_backtraced_conditions(condition* parent_cond,
                                                         const identity_set_quadruple o_ids_to_replace,
                                                         const rhs_quadruple rhs_funcs)
{
    test lId = 0, lAttr = 0, lValue = 0;
    lId = parent_cond->data.tests.id_test->eq_test;
    lAttr = parent_cond->data.tests.attr_test->eq_test;
    lValue = parent_cond->data.tests.value_test->eq_test;

    assert(ebc_settings[SETTING_EBC_LEARNING_ON]);

    dprint(DT_ADD_IDENTITY_SET_MAPPING, "Unifying backtraced condition %l with rhs identities (%u ^%u %u)\n", parent_cond,
        o_ids_to_replace.id ? o_ids_to_replace.id->identity : 0, o_ids_to_replace.attr ? o_ids_to_replace.attr->identity : 0, o_ids_to_replace.value ? o_ids_to_replace.value->identity : 0);
//    dprint(DT_ADD_IDENTITY_SET_MAPPING, "Unifying backtraced condition (%y ^%y %y):  (%u ^%u %u)  --> (%u ^%u %u)\n",
//        parent_cond->data.tests.id_test->eq_test->data.referent, parent_cond->data.tests.attr_test->eq_test->data.referent, parent_cond->data.tests.value_test->eq_test->data.referent,
//        parent_cond->data.tests.id_test->eq_test->identity_set->identity, parent_cond->data.tests.attr_test->eq_test->identity_set->identity, parent_cond->data.tests.value_test->eq_test->identity_set->identity,
//        o_ids_to_replace.id->identity, o_ids_to_replace.attr->identity, o_ids_to_replace.value->identity);

    if (o_ids_to_replace.id)
    {
        if (lId->identity_set)
        {
            if (o_ids_to_replace.id->super_join != lId->identity_set->super_join)
            {
                dprint(DT_ADD_IDENTITY_SET_MAPPING, "Unifying identity sets of identifier element: %u/%us%u -> %us%u\n", lId->identity, lId->identity_set->identity, lId->identity_set->super_join->identity, o_ids_to_replace.id->identity, o_ids_to_replace.id->super_join->identity);
                join_identity_sets(o_ids_to_replace.id, lId->identity_set);
            } else {
                dprint(DT_ADD_IDENTITY_SET_MAPPING, "Both identities are already in the same identity set: %u/%us%u -> %us%u\n", lId->identity, lId->identity_set->identity, lId->identity_set->super_join->identity, o_ids_to_replace.id->identity, o_ids_to_replace.id->super_join->identity);
            }
        } else {
            dprint(DT_ADD_IDENTITY_SET_MAPPING, "Literalizing identity set of identifier element: %us%u -> %t\n", o_ids_to_replace.id->identity, o_ids_to_replace.id->super_join->identity, lId);
            join_identity_sets(o_ids_to_replace.id, NULL_IDENTITY_SET);
        }
    }
    else if (rhs_value_is_literalizing_function(rhs_funcs.id))
    {
        dprint(DT_ADD_IDENTITY_SET_MAPPING, "Literalizing arguments of RHS function in identifier element %r\n", rhs_funcs.id);
        literalize_RHS_function_args(rhs_funcs.id, parent_cond->inst->i_id);
        if (lId->identity_set) join_identity_sets(lId->identity_set, NULL_IDENTITY_SET);
    }
//    else
//    {
//        dprint(DT_ADD_IDENTITY_SET_MAPPING, "Did not unify because %s%s\n", lId->data.referent->is_sti() ? "is identifier " : "", !o_ids_to_replace.id ? "RHS pref is literal " : "");
//    }
    if (o_ids_to_replace.attr)
    {
        if (lAttr->identity_set)
        {
            if (o_ids_to_replace.attr != lAttr->identity_set)
            {
                dprint(DT_ADD_IDENTITY_SET_MAPPING, "Unifying identity sets of identifier element: %u/%us%u -> %us%u\n", lAttr->identity, lAttr->identity_set->identity, lAttr->identity_set->super_join->identity, o_ids_to_replace.attr->identity, o_ids_to_replace.attr->super_join->identity);
                join_identity_sets(o_ids_to_replace.attr, lAttr->identity_set);
            } else {
                dprint(DT_ADD_IDENTITY_SET_MAPPING, "Both identities are already in the same identity set: %u/%us%u -> %us%u\n", lAttr->identity, lAttr->identity_set->identity, lAttr->identity_set->super_join->identity, o_ids_to_replace.attr->identity, o_ids_to_replace.attr->super_join->identity);
            }
        } else {
            dprint(DT_ADD_IDENTITY_SET_MAPPING, "Literalizing identity set of identifier element: %us%u -> %t\n", o_ids_to_replace.attr->identity, o_ids_to_replace.attr->super_join->identity, lAttr);
            join_identity_sets(o_ids_to_replace.attr, NULL_IDENTITY_SET);
        }
    }
    else if (rhs_value_is_literalizing_function(rhs_funcs.attr))
    {
        dprint(DT_ADD_IDENTITY_SET_MAPPING, "Literalizing arguments of RHS function in attribute element %r\n", rhs_funcs.attr);
        literalize_RHS_function_args(rhs_funcs.attr, parent_cond->inst->i_id);
        if (lAttr->identity_set) join_identity_sets(lAttr->identity_set, NULL_IDENTITY_SET);
    }
//    else
//    {
//        dprint(DT_ADD_IDENTITY_SET_MAPPING, "Did not unify because %s%s\n", lAttr->data.referent->is_sti() ? "is STI " : "", !o_ids_to_replace.attr ? "RHS pref is literal " : "");
//    }
    if (o_ids_to_replace.value)
    {
        if (lValue->identity_set)
        {
            if (o_ids_to_replace.value != lValue->identity_set)
            {
                dprint(DT_ADD_IDENTITY_SET_MAPPING, "Unifying identity sets of identifier element: %u/%us%u -> %us%u\n", lValue->identity, lValue->identity_set->identity, lValue->identity_set->super_join->identity, o_ids_to_replace.value->identity, o_ids_to_replace.value->super_join->identity);
                join_identity_sets(o_ids_to_replace.value, lValue->identity_set);
            } else {
                dprint(DT_ADD_IDENTITY_SET_MAPPING, "Both identities are already in the same identity set: %u/%us%u -> %us%u\n", lValue->identity, lValue->identity_set->identity, lValue->identity_set->super_join->identity, o_ids_to_replace.value->identity, o_ids_to_replace.value->super_join->identity);
            }
        } else {
            dprint(DT_ADD_IDENTITY_SET_MAPPING, "Literalizing identity set of identifier element: %us%u -> %t\n", o_ids_to_replace.value->identity, o_ids_to_replace.value->super_join->identity, lValue);
            join_identity_sets(o_ids_to_replace.value, NULL_IDENTITY_SET);
        }
    }
    else if (rhs_value_is_literalizing_function(rhs_funcs.value))
    {
        dprint(DT_ADD_IDENTITY_SET_MAPPING, "Literalizing arguments of RHS function in value element %r\n", rhs_funcs.value);
        literalize_RHS_function_args(rhs_funcs.value, parent_cond->inst->i_id);
        if (lValue->identity_set) join_identity_sets(lValue->identity_set, NULL_IDENTITY_SET);
    }
//    else
//    {
//        dprint(DT_ADD_IDENTITY_SET_MAPPING, "Did not unify because %s%s\n", lValue->data.referent->is_sti() ? "is STI " : "", !o_ids_to_replace.value ? "RHS pref is literal " : "");
//    }
    assert(!o_ids_to_replace.referent);
    if (rhs_value_is_literalizing_function(rhs_funcs.referent))
    {
        dprint(DT_ADD_IDENTITY_SET_MAPPING, "Literalizing arguments of RHS function in referent element %r\n", rhs_funcs.referent);
        literalize_RHS_function_args(rhs_funcs.referent, parent_cond->inst->i_id);
    }
}

/* Requires: pCond is being added to grounds and is the second condition being added to grounds
 *           that matched a given wme, which guarantees chunker_bt_last_ground_cond points to the
 *           first condition that matched. */
void Explanation_Based_Chunker::add_singleton_unification_if_needed(condition* pCond)
{
//    if (!ebc_settings[SETTING_EBC_LEARNING_ON]) return; // May need this if we have to add both conds when learning is off
    assert(ebc_settings[SETTING_EBC_LEARNING_ON]);

    if (wme_is_a_singleton(pCond->bt.wme_) || ebc_settings[SETTING_EBC_UNIFY_ALL])
    {
        condition* last_cond = pCond->bt.wme_->chunker_bt_last_ground_cond;
        assert(last_cond);
        dprint(DT_UNIFY_SINGLETONS, "Unifying value element of second condition that matched singleton wme: %l\n", pCond);
        dprint(DT_UNIFY_SINGLETONS, "-- Original condition seen: %l\n", pCond->bt.wme_->chunker_bt_last_ground_cond);
        if (pCond->data.tests.value_test->eq_test->identity_set || last_cond->data.tests.value_test->eq_test->identity_set)
        {
            ebc_timers->dependency_analysis->stop();
            thisAgent->explanationMemory->add_identity_set_mapping(pCond->inst->i_id, IDS_unified_with_singleton, pCond->data.tests.value_test->eq_test->identity_set, last_cond->data.tests.value_test->eq_test->identity_set);
            join_identity_sets(pCond->data.tests.value_test->eq_test->identity_set, last_cond->data.tests.value_test->eq_test->identity_set);
            ebc_timers->dependency_analysis->start();
        }
    }
    /* The code that sets isa_operator checks if an id is a goal, so don't need to check here */
    else if ((pCond->bt.wme_->attr == thisAgent->symbolManager->soarSymbols.operator_symbol) &&
        (pCond->bt.wme_->value->is_sti() &&  pCond->bt.wme_->value->id->isa_operator) &&
        (!pCond->test_for_acceptable_preference))
    {
        condition* last_cond = pCond->bt.wme_->chunker_bt_last_ground_cond;
        assert(last_cond);
        if (pCond->data.tests.value_test->eq_test->identity_set || last_cond->data.tests.value_test->eq_test->identity_set)
        {
            ebc_timers->dependency_analysis->stop();
            thisAgent->explanationMemory->add_identity_set_mapping(pCond->inst->i_id, IDS_unified_with_singleton, pCond->data.tests.value_test->eq_test->identity_set, last_cond->data.tests.value_test->eq_test->identity_set);
            join_identity_sets(pCond->data.tests.value_test->eq_test->identity_set, last_cond->data.tests.value_test->eq_test->identity_set);
            ebc_timers->dependency_analysis->start();
        }
    }
}

const std::string Explanation_Based_Chunker::add_new_singleton(singleton_element_type id_type, Symbol* attrSym, singleton_element_type value_type)
{
    std::string returnVal;

    if ((attrSym == thisAgent->symbolManager->soarSymbols.operator_symbol) ||
        (attrSym == thisAgent->symbolManager->soarSymbols.superstate_symbol) ||
        (attrSym == thisAgent->symbolManager->soarSymbols.smem_sym) ||
        (attrSym == thisAgent->symbolManager->soarSymbols.type_symbol) ||
        (attrSym == thisAgent->symbolManager->soarSymbols.impasse_symbol) ||
        (attrSym == thisAgent->symbolManager->soarSymbols.epmem_sym))
    {
        thisAgent->outputManager->sprinta_sf(thisAgent, returnVal, "Soar cannot override the architectural singleton for %y.  Ignoring.", attrSym);
        return returnVal;
    }

    if (attrSym->sc->singleton.possible)
    {
        thisAgent->outputManager->sprinta_sf(thisAgent, returnVal, "Clearing previous singleton for %y.\n", attrSym);
    }
    thisAgent->outputManager->sprinta_sf(thisAgent, returnVal, "Will unify conditions in super-states that match a WME that fits the pattern:  (%s ^%y %s)", singletonTypeToString(id_type), attrSym, singletonTypeToString(value_type));
    singletons->insert(attrSym);
    thisAgent->symbolManager->symbol_add_ref(attrSym);
    attrSym->sc->singleton.possible = true;
    attrSym->sc->singleton.id_type = id_type;
    attrSym->sc->singleton.value_type = value_type;

    return returnVal;
}

const std::string Explanation_Based_Chunker::remove_singleton(singleton_element_type id_type, Symbol* attrSym, singleton_element_type value_type)
{
    std::string returnVal;

    if ((attrSym == thisAgent->symbolManager->soarSymbols.operator_symbol) ||
        (attrSym == thisAgent->symbolManager->soarSymbols.superstate_symbol) ||
        (attrSym == thisAgent->symbolManager->soarSymbols.smem_sym) ||
        (attrSym == thisAgent->symbolManager->soarSymbols.type_symbol) ||
        (attrSym == thisAgent->symbolManager->soarSymbols.impasse_symbol) ||
        (attrSym == thisAgent->symbolManager->soarSymbols.epmem_sym))
    {
        thisAgent->outputManager->sprinta_sf(thisAgent, returnVal, "Soar cannot remove the architectural singleton for %y.  Ignoring.", attrSym);
        return returnVal;
    }
    auto it = singletons->find(attrSym);
    if (it == singletons->end())
    {
        thisAgent->outputManager->sprinta_sf(thisAgent, returnVal, "Could not find pattern (%s ^%y %s).  Did not remove.", singletonTypeToString(id_type), attrSym, singletonTypeToString(value_type));
    } else {
        thisAgent->outputManager->sprinta_sf(thisAgent, returnVal, "Removed. Will no longer unify conditions in super-states that match a WME\n"
                                                                   "         that fits the pattern:  (%s ^%y %s)\n", singletonTypeToString(id_type), attrSym, singletonTypeToString(value_type));
        singletons->erase(attrSym);
        attrSym->sc->singleton.possible = false;
        thisAgent->symbolManager->symbol_remove_ref(&attrSym);
    }

    return returnVal;
}
void Explanation_Based_Chunker::clear_singletons()
{
    Symbol* lSym;
    for (auto it = singletons->begin(); it != singletons->end(); ++it)
    {
        lSym = (*it);
        lSym->sc->singleton.possible = false;
        thisAgent->symbolManager->symbol_remove_ref(&lSym);
    }
    singletons->clear();
}

void Explanation_Based_Chunker::add_to_singletons(wme* pWME)
{
    pWME->singleton_status_checked = true;
    pWME->is_singleton = true;
}

const char* TorF(bool isTrue) { if (isTrue) return "true"; else return "false"; }
const char* PassorFail(bool isTrue) { if (isTrue) return "Pass"; else return "Fail"; }

bool Explanation_Based_Chunker::wme_is_a_singleton(wme* pWME)
{
    if (pWME->singleton_status_checked) return pWME->is_singleton;
    if (!pWME->attr->is_string() || !pWME->attr->sc->singleton.possible || !ebc_settings[SETTING_EBC_USER_SINGLETONS]) return false;

    /* This WME has a valid singleton attribute but has never had it's identifier and
     * value elements checked, so we see if it matches the pattern defined in the attribute. */
    bool lIDPassed = false;
    bool lValuePassed = false;
    singleton_element_type id_type = pWME->attr->sc->singleton.id_type;
    singleton_element_type value_type = pWME->attr->sc->singleton.value_type;

//    dprint(DT_DEBUG, "(%y ^%y %y) vs [%s ^%y %s] :", pWME->id, pWME->attr, pWME->value, singletonTypeToString(id_type), pWME->attr, singletonTypeToString(value_type));
    lIDPassed =     ((id_type == ebc_any) ||
                    ((id_type == ebc_identifier) && !pWME->id->is_state() && !pWME->id->is_operator()) ||
                    ((id_type == ebc_state)      && pWME->id->is_state()) ||
                    ((id_type == ebc_operator)   && pWME->id->is_operator()));
    lValuePassed =  ((value_type == ebc_any) ||
                    ((value_type == ebc_state)      && pWME->value->is_state()) ||
                    ((value_type == ebc_identifier) && pWME->value->is_sti() && !pWME->value->is_state() && !pWME->value->is_operator()) ||
                    ((value_type == ebc_constant)   && pWME->value->is_constant()) ||
                    ((value_type == ebc_operator)   && pWME->value->is_operator()));

    pWME->is_singleton = lIDPassed && lValuePassed;
    pWME->singleton_status_checked = true;
//    dprint_noprefix(DT_DEBUG, "%s! = %s + %s\n", PassorFail(pWME->is_singleton), TorF(lIDPassed), TorF(lValuePassed));
    return pWME->is_singleton;
}
