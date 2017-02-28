/*
 * variablization_manager_map.cpp
 *
 *  Created on: Jul 25, 2013
 *      Author: mazzin
 */

#include "ebc.h"
#include "ebc_timers.h"

#include "debug_inventories.h"
#include "dprint.h"
#include "explanation_memory.h"
#include "rhs.h"
#include "rhs_functions.h"
#include "working_memory.h"

identity_set* Explanation_Based_Chunker::make_identity_set(uint64_t pIdentity)
{
    identity_set* new_join_set;
    thisAgent->memoryManager->allocate_with_pool(MP_identity_sets, &new_join_set);
    new_join_set->identity = pIdentity;
    new_join_set->dirty = false;
    new_join_set->super_join = new_join_set;
    new_join_set->identity_sets = NULL;
    new_join_set->new_var = NULL;
    new_join_set->clone_identity = NULL_IDENTITY_SET;
    new_join_set->literalized = false;
    new_join_set->operational_cond = NULL;
    new_join_set->operational_field = NO_ELEMENT;
    ISI_add(thisAgent, new_join_set);

    dprint(DT_DEALLOCATE_ID_SETS, "Created identity set %u\n", new_join_set->identity);
    return new_join_set;
}


void Explanation_Based_Chunker::update_identity_set_clone_id(identity_set* pIdentitySet)
{
    assert(pIdentitySet);
    assert(!pIdentitySet->super_join->clone_identity);

    increment_counter(variablization_identity_counter);
    pIdentitySet->super_join->clone_identity = variablization_identity_counter;
    touch_identity_set(pIdentitySet);

}

void Explanation_Based_Chunker::deallocate_identity_set(identity_set* &pIDSet)
{
    dprint(DT_DEALLOCATE_ID_SETS, "Deallocating identity set %us%u.\n", pIDSet->identity, pIDSet->super_join->identity);
    if (pIDSet->super_join != pIDSet)
    {
        dprint(DT_DEALLOCATE_ID_SETS, "Removing identity set %u from super-join %u's identity_sets\n", pIDSet->identity, pIDSet->super_join->identity);
        pIDSet->super_join->identity_sets->remove(pIDSet);
    }
    if (pIDSet->identity_sets)
    {
        identity_set* lPreviouslyJoinedIdentity;
        for (auto it = pIDSet->identity_sets->begin(); it != pIDSet->identity_sets->end(); it++)
        {
            lPreviouslyJoinedIdentity = *it;
            dprint(DT_DEALLOCATE_ID_SETS, "Resetting previous join set mapping of %u to %u\n", lPreviouslyJoinedIdentity->identity, pIDSet->identity);
            lPreviouslyJoinedIdentity->super_join = lPreviouslyJoinedIdentity;
        }
    }
    clean_up_identity_set_transient(pIDSet);
    ISI_remove(thisAgent, pIDSet);
    thisAgent->memoryManager->free_with_pool(MP_identity_sets, pIDSet);
    pIDSet = NULL;
}

void Explanation_Based_Chunker::clean_up_identity_set_transient(identity_set* pIDSet)
{
    if (pIDSet->new_var) thisAgent->symbolManager->symbol_remove_ref(&pIDSet->new_var);
    if (pIDSet->identity_sets) delete pIDSet->identity_sets;
    pIDSet->dirty = false;
    pIDSet->super_join = pIDSet;
    pIDSet->identity_sets = NULL;
    pIDSet->new_var = NULL;
    pIDSet->clone_identity = NULL_IDENTITY_SET;
    pIDSet->literalized = false;
    pIDSet->operational_cond = NULL;
    pIDSet->operational_field = NO_ELEMENT;
}

void Explanation_Based_Chunker::clean_up_identity_sets()
{
    for (auto it = identity_sets_to_clean_up.begin(); it != identity_sets_to_clean_up.end(); it++)
    {
        identity_set* lJoin_set = *it;
        clean_up_identity_set_transient(lJoin_set);
    }
    identity_sets_to_clean_up.clear();
}

void Explanation_Based_Chunker::join_identity_sets(identity_set* lFromJoinSet, identity_set* lToJoinSet)
{
    assert(ebc_settings[SETTING_EBC_LEARNING_ON]);
    assert(lFromJoinSet && lToJoinSet && (lFromJoinSet->super_join != lToJoinSet->super_join));

    lFromJoinSet = lFromJoinSet->super_join;
    lToJoinSet = lToJoinSet->super_join;

    ebc_timers->variablization_rhs->start();
    ebc_timers->variablization_rhs->stop();
    ebc_timers->identity_unification->start();

    touch_identity_set(lFromJoinSet);
    touch_identity_set(lToJoinSet);

    dprint(DT_UNIFY_IDENTITY_SETS, "Combining two join sets for %u and %u...\n", lFromJoinSet->super_join->identity, lToJoinSet->super_join->identity);

    /* Check size and swap if necessary to favor growing the bigger join set */
    uint64_t lFromSize = lFromJoinSet->identity_sets ? lFromJoinSet->identity_sets->size() : 0;
    uint64_t lToSize = lToJoinSet->identity_sets ? lToJoinSet->identity_sets->size() : 0;
    if (lFromSize > lToSize)
    {
        dprint(DT_UNIFY_IDENTITY_SETS, "Swapping join sets so that %u is target and not %u\n", lFromJoinSet->super_join->identity, lToJoinSet->super_join->identity);
        identity_set* tempJoin = lFromJoinSet;
        lFromJoinSet = lToJoinSet;
        lToJoinSet = tempJoin;
    }

    if (!lToJoinSet->identity_sets)
    {
        lToJoinSet->identity_sets = new identity_set_list();
    }

    /* Iterate through identity sets in lFromJoinSet and set their super join set point to lToJoinSet */
    identity_set* lPreviouslyJoinedIdentity;
    if (lFromJoinSet->identity_sets)
    {
        for (auto it = lFromJoinSet->identity_sets->begin(); it != lFromJoinSet->identity_sets->end(); it++)
        {
            lPreviouslyJoinedIdentity = *it;
            dprint(DT_UNIFY_IDENTITY_SETS, "Changing previous join set mapping of %u to %u\n", lPreviouslyJoinedIdentity->identity, lFromJoinSet->super_join->identity);
            lPreviouslyJoinedIdentity->super_join = lToJoinSet;
            if (lPreviouslyJoinedIdentity->literalized) lToJoinSet->literalized = true;
        }
        lToJoinSet->identity_sets->splice(lToJoinSet->identity_sets->begin(), (*lFromJoinSet->identity_sets));
        delete lFromJoinSet->identity_sets;
        lFromJoinSet->identity_sets = NULL;
    }
    /* The identity set being joined is not on its child identity_sets list, so we add it to other identity set here*/
    dprint(DT_UNIFY_IDENTITY_SETS, "Changing join set mapping of %u -> %u to %u -> %u\n", lFromJoinSet->identity, lFromJoinSet->super_join->identity, lFromJoinSet->identity, lToJoinSet->identity);
    lToJoinSet->identity_sets->push_back(lFromJoinSet);

    /* Propagate literalization */
    if (lFromJoinSet->literalized) lToJoinSet->literalized = true;

    /* Point super_join to joined identity set */
    lFromJoinSet->super_join = lToJoinSet;

    ebc_timers->identity_unification->stop();

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
                    literalize_identity_set(rs->identity_set->super_join);
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

    dprint(DT_UNIFY_IDENTITY_SETS, "Unifying backtraced condition %l with rhs identities (%u ^%u %u)\n", parent_cond,
        o_ids_to_replace.id ? o_ids_to_replace.id->identity : 0, o_ids_to_replace.attr ? o_ids_to_replace.attr->identity : 0, o_ids_to_replace.value ? o_ids_to_replace.value->identity : 0);

    if (o_ids_to_replace.id)
    {
        if (lId->identity_set)
        {
            if (o_ids_to_replace.id->super_join != lId->identity_set->super_join)
            {
                dprint(DT_UNIFY_IDENTITY_SETS, "Unifying identity sets of identifier element: %u/%us%u -> %us%u\n", lId->identity, lId->identity_set->identity, lId->identity_set->super_join->identity, o_ids_to_replace.id->identity, o_ids_to_replace.id->super_join->identity);
                join_identity_sets(o_ids_to_replace.id, lId->identity_set);
            } else {
                dprint(DT_UNIFY_IDENTITY_SETS, "Both identities are already in the same identity set: %u/%us%u -> %us%u\n", lId->identity, lId->identity_set->identity, lId->identity_set->super_join->identity, o_ids_to_replace.id->identity, o_ids_to_replace.id->super_join->identity);
            }
        } else {
            dprint(DT_UNIFY_IDENTITY_SETS, "Literalizing identity set of identifier element: %us%u -> %t\n", o_ids_to_replace.id->identity, o_ids_to_replace.id->super_join->identity, lId);
            literalize_identity_set(o_ids_to_replace.id->super_join);
        }
    }
    else if (rhs_value_is_literalizing_function(rhs_funcs.id))
    {
        dprint(DT_UNIFY_IDENTITY_SETS, "Literalizing arguments of RHS function in identifier element %r\n", rhs_funcs.id);
        literalize_RHS_function_args(rhs_funcs.id, parent_cond->inst->i_id);
        if (lId->identity_set) literalize_identity_set(lId->identity_set->super_join);
    }
    if (o_ids_to_replace.attr)
    {
        if (lAttr->identity_set)
        {
            if (o_ids_to_replace.attr->super_join != lAttr->identity_set->super_join)
            {
                dprint(DT_UNIFY_IDENTITY_SETS, "Unifying identity sets of identifier element: %u/%us%u -> %us%u\n", lAttr->identity, lAttr->identity_set->identity, lAttr->identity_set->super_join->identity, o_ids_to_replace.attr->identity, o_ids_to_replace.attr->super_join->identity);
                join_identity_sets(o_ids_to_replace.attr, lAttr->identity_set);
            } else {
                dprint(DT_UNIFY_IDENTITY_SETS, "Both identities are already in the same identity set: %u/%us%u -> %us%u\n", lAttr->identity, lAttr->identity_set->identity, lAttr->identity_set->super_join->identity, o_ids_to_replace.attr->identity, o_ids_to_replace.attr->super_join->identity);
            }
        } else {
            dprint(DT_UNIFY_IDENTITY_SETS, "Literalizing identity set of identifier element: %us%u -> %t\n", o_ids_to_replace.attr->identity, o_ids_to_replace.attr->super_join->identity, lAttr);
            literalize_identity_set(o_ids_to_replace.attr->super_join);
        }
    }
    else if (rhs_value_is_literalizing_function(rhs_funcs.attr))
    {
        dprint(DT_UNIFY_IDENTITY_SETS, "Literalizing arguments of RHS function in attribute element %r\n", rhs_funcs.attr);
        literalize_RHS_function_args(rhs_funcs.attr, parent_cond->inst->i_id);
        if (lAttr->identity_set) literalize_identity_set(lAttr->identity_set->super_join);
    }
    if (o_ids_to_replace.value)
    {
        if (lValue->identity_set)
        {
            if (o_ids_to_replace.value->super_join != lValue->identity_set->super_join)
            {
                dprint(DT_UNIFY_IDENTITY_SETS, "Unifying identity sets of identifier element: %u/%us%u -> %us%u\n", lValue->identity, lValue->identity_set->identity, lValue->identity_set->super_join->identity, o_ids_to_replace.value->identity, o_ids_to_replace.value->super_join->identity);
                join_identity_sets(o_ids_to_replace.value, lValue->identity_set);
            } else {
                dprint(DT_UNIFY_IDENTITY_SETS, "Both identities are already in the same identity set: %u/%us%u -> %us%u\n", lValue->identity, lValue->identity_set->identity, lValue->identity_set->super_join->identity, o_ids_to_replace.value->identity, o_ids_to_replace.value->super_join->identity);
            }
        } else {
            dprint(DT_UNIFY_IDENTITY_SETS, "Literalizing identity set of identifier element: %us%u -> %t\n", o_ids_to_replace.value->identity, o_ids_to_replace.value->super_join->identity, lValue);
            literalize_identity_set(o_ids_to_replace.value->super_join);
        }
    }
    else if (rhs_value_is_literalizing_function(rhs_funcs.value))
    {
        dprint(DT_UNIFY_IDENTITY_SETS, "Literalizing arguments of RHS function in value element %r\n", rhs_funcs.value);
        literalize_RHS_function_args(rhs_funcs.value, parent_cond->inst->i_id);
        if (lValue->identity_set) literalize_identity_set(lValue->identity_set->super_join);
    }
    assert(!o_ids_to_replace.referent);
    if (rhs_value_is_literalizing_function(rhs_funcs.referent))
    {
        dprint(DT_UNIFY_IDENTITY_SETS, "Literalizing arguments of RHS function in referent element %r\n", rhs_funcs.referent);
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
            identity_set* pCondIDSet = pCond->data.tests.value_test->eq_test->identity_set ? pCond->data.tests.value_test->eq_test->identity_set->super_join : NULL;
            identity_set* pLCondIDSet = last_cond->data.tests.value_test->eq_test->identity_set ? last_cond->data.tests.value_test->eq_test->identity_set->super_join : NULL;
            if (pCondIDSet != pLCondIDSet)
            {
                ebc_timers->dependency_analysis->stop();
                thisAgent->explanationMemory->add_identity_set_mapping(pCond->inst->i_id, IDS_unified_with_singleton, pCond->data.tests.value_test->eq_test->identity_set, last_cond->data.tests.value_test->eq_test->identity_set);
                join_identity_sets(pCond->data.tests.value_test->eq_test->identity_set, last_cond->data.tests.value_test->eq_test->identity_set);
                ebc_timers->dependency_analysis->start();
            }
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
            identity_set* pCondIDSet = pCond->data.tests.value_test->eq_test->identity_set ? pCond->data.tests.value_test->eq_test->identity_set->super_join : NULL;
            identity_set* pLCondIDSet = last_cond->data.tests.value_test->eq_test->identity_set ? last_cond->data.tests.value_test->eq_test->identity_set->super_join : NULL;
            if (pCondIDSet != pLCondIDSet)
            {
                ebc_timers->dependency_analysis->stop();
                thisAgent->explanationMemory->add_identity_set_mapping(pCond->inst->i_id, IDS_unified_with_singleton, pCond->data.tests.value_test->eq_test->identity_set, last_cond->data.tests.value_test->eq_test->identity_set);
                join_identity_sets(pCond->data.tests.value_test->eq_test->identity_set, last_cond->data.tests.value_test->eq_test->identity_set);
                ebc_timers->dependency_analysis->start();
            }
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
