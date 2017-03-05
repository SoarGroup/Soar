/*
 * variablization_manager_map.cpp
 *
 *  Created on: Jul 25, 2013
 *      Author: mazzin
 */

#include "ebc.h"
#include "ebc_timers.h"

#include "condition.h"
#include "debug_inventories.h"
#include "dprint.h"
#include "explanation_memory.h"
#include "instantiation.h"
#include "output_manager.h"
#include "rhs.h"
#include "rhs_functions.h"
#include "test.h"
#include "working_memory.h"

void Explanation_Based_Chunker::unify_backtraced_conditions(condition* parent_cond, identity_set_quadruple &o_ids_to_replace, const rhs_quadruple rhs_funcs)
{
    test lId = 0, lAttr = 0, lValue = 0;
    lId = parent_cond->data.tests.id_test->eq_test;
    lAttr = parent_cond->data.tests.attr_test->eq_test;
    lValue = parent_cond->data.tests.value_test->eq_test;

    dprint(DT_UNIFY_IDENTITY_SETS, "Unifying backtraced condition %l with rhs identities (%u ^%u %u)\n", parent_cond, o_ids_to_replace.id ? o_ids_to_replace.id->idset_id : 0, o_ids_to_replace.attr ? o_ids_to_replace.attr->idset_id : 0, o_ids_to_replace.value ? o_ids_to_replace.value->idset_id : 0);

    if (o_ids_to_replace.id)
    {
        if (lId->identity_set)
        {
            if (o_ids_to_replace.id->super_join != lId->identity_set->super_join)
            {
                dprint(DT_UNIFY_IDENTITY_SETS, "Unifying identity sets of identifier element: %u/%us%u -> %us%u\n", lId->identity, lId->identity_set->idset_id, lId->identity_set->get_identity(), o_ids_to_replace.id->idset_id, o_ids_to_replace.id->get_identity());
//                thisAgent->explanationMemory->add_identity_set_mapping(parent_cond->inst->i_id, IDS_join, o_ids_to_replace.id->super_join, lId->identity_set->super_join);
                join_identity_sets(o_ids_to_replace.id, lId->identity_set);
            } else {
                dprint(DT_UNIFY_IDENTITY_SETS, "Both identities are already in the same identity set: %u/%us%u -> %us%u\n", lId->identity, lId->identity_set->idset_id, lId->identity_set->get_identity(), o_ids_to_replace.id->idset_id, o_ids_to_replace.id->get_identity());
            }
        } else {
            dprint(DT_UNIFY_IDENTITY_SETS, "Literalizing identity set of identifier element: %us%u -> %t\n", o_ids_to_replace.id->idset_id, o_ids_to_replace.id->get_identity(), lId);
//            thisAgent->explanationMemory->add_identity_set_mapping(parent_cond->inst->i_id, IDS_literalized_RHS_function_arg, o_ids_to_replace.id->super_join, NULL);
            o_ids_to_replace.id->literalize();

        }
    }
    else if (rhs_value_is_literalizing_function(rhs_funcs.id))
    {
        dprint(DT_UNIFY_IDENTITY_SETS, "Literalizing arguments of RHS function in identifier element %r\n", rhs_funcs.id);
        literalize_RHS_function_args(rhs_funcs.id, parent_cond->inst->i_id);
        if (lId->identity_set)
        {
//            thisAgent->explanationMemory->add_identity_set_mapping(parent_cond->inst->i_id, IDS_literalized_RHS_function_arg, lId->identity_set->super_join, NULL);
            lId->identity_set->literalize();
        }
    }
    if (o_ids_to_replace.attr)
    {
        if (lAttr->identity_set)
        {
            if (o_ids_to_replace.attr->super_join != lAttr->identity_set->super_join)
            {
                dprint(DT_UNIFY_IDENTITY_SETS, "Unifying identity sets of identifier element: %u/%us%u -> %us%u\n", lAttr->identity, lAttr->identity_set->idset_id, lAttr->identity_set->get_identity(), o_ids_to_replace.attr->idset_id, o_ids_to_replace.attr->get_identity());
//                thisAgent->explanationMemory->add_identity_set_mapping(parent_cond->inst->i_id, IDS_join, o_ids_to_replace.attr->super_join, lAttr->identity_set->super_join);
                join_identity_sets(o_ids_to_replace.attr, lAttr->identity_set);
            } else {
                dprint(DT_UNIFY_IDENTITY_SETS, "Both identities are already in the same identity set: %u/%us%u -> %us%u\n", lAttr->identity, lAttr->identity_set->idset_id, lAttr->identity_set->get_identity(), o_ids_to_replace.attr->idset_id, o_ids_to_replace.attr->get_identity());
            }
        } else {
            dprint(DT_UNIFY_IDENTITY_SETS, "Literalizing identity set of identifier element: %us%u -> %t\n", o_ids_to_replace.attr->idset_id, o_ids_to_replace.attr->get_identity(), lAttr);
//            thisAgent->explanationMemory->add_identity_set_mapping(parent_cond->inst->i_id, IDS_literalized_RHS_function_arg, o_ids_to_replace.attr->super_join, NULL);
            o_ids_to_replace.attr->literalize();

        }
    }
    else if (rhs_value_is_literalizing_function(rhs_funcs.attr))
    {
        dprint(DT_UNIFY_IDENTITY_SETS, "Literalizing arguments of RHS function in attribute element %r\n", rhs_funcs.attr);
        literalize_RHS_function_args(rhs_funcs.attr, parent_cond->inst->i_id);
        if (lAttr->identity_set)
        {
//            thisAgent->explanationMemory->add_identity_set_mapping(parent_cond->inst->i_id, IDS_literalized_RHS_function_arg, lAttr->identity_set->super_join, NULL);
            lAttr->identity_set->literalize();

        }
    }
    if (o_ids_to_replace.value)
    {
        if (lValue->identity_set)
        {
            if (o_ids_to_replace.value->super_join != lValue->identity_set->super_join)
            {
                dprint(DT_UNIFY_IDENTITY_SETS, "Unifying identity sets of identifier element: %u/%us%u -> %us%u\n", lValue->identity, lValue->identity_set->idset_id, lValue->identity_set->get_identity(), o_ids_to_replace.value->idset_id, o_ids_to_replace.value->get_identity());
//                thisAgent->explanationMemory->add_identity_set_mapping(parent_cond->inst->i_id, IDS_join, o_ids_to_replace.value->get_identity(), lValue->identity_set->get_identity());
                join_identity_sets(o_ids_to_replace.value, lValue->identity_set);
            } else {
                dprint(DT_UNIFY_IDENTITY_SETS, "Both identities are already in the same identity set: %u/%us%u -> %us%u\n", lValue->identity, lValue->identity_set->idset_id, lValue->identity_set->get_identity(), o_ids_to_replace.value->idset_id, o_ids_to_replace.value->get_identity());
            }
        } else {
            dprint(DT_UNIFY_IDENTITY_SETS, "Literalizing identity set of identifier element: %us%u -> %t\n", o_ids_to_replace.value->idset_id, o_ids_to_replace.value->get_identity(), lValue);
//            thisAgent->explanationMemory->add_identity_set_mapping(parent_cond->inst->i_id, IDS_literalized_RHS_function_arg, o_ids_to_replace.value->get_identity(), NULL);
            o_ids_to_replace.value->literalize();
        }
    }
    else if (rhs_value_is_literalizing_function(rhs_funcs.value))
    {
        dprint(DT_UNIFY_IDENTITY_SETS, "Literalizing arguments of RHS function in value element %r\n", rhs_funcs.value);
        literalize_RHS_function_args(rhs_funcs.value, parent_cond->inst->i_id);
        if (lValue->identity_set)
        {
//            thisAgent->explanationMemory->add_identity_set_mapping(parent_cond->inst->i_id, IDS_literalized_RHS_function_arg, lValue->identity_set->get_identity(), NULL);
            lValue->identity_set->literalize();
        }
    }

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
    if (wme_is_a_singleton(pCond->bt.wme_) || ebc_settings[SETTING_EBC_UNIFY_ALL])
    {
        condition* last_cond = pCond->bt.wme_->chunker_bt_last_ground_cond;
        dprint(DT_UNIFY_SINGLETONS, "Unifying value element of second condition that matched singleton wme: %l\n", pCond);
        dprint(DT_UNIFY_SINGLETONS, "-- Original condition seen: %l\n", pCond->bt.wme_->chunker_bt_last_ground_cond);
        if (pCond->data.tests.value_test->eq_test->identity_set || last_cond->data.tests.value_test->eq_test->identity_set)
        {
            IdentitySetSharedPtr pCondIDSet = pCond->data.tests.value_test->eq_test->identity_set ? pCond->data.tests.value_test->eq_test->identity_set->super_join : NULL;
            IdentitySetSharedPtr pLCondIDSet = last_cond->data.tests.value_test->eq_test->identity_set ? last_cond->data.tests.value_test->eq_test->identity_set->super_join : NULL;
            if (pCondIDSet != pLCondIDSet)
            {
                ebc_timers->dependency_analysis->stop();
//                thisAgent->explanationMemory->add_identity_set_mapping(pCond->inst->i_id, IDS_unified_with_singleton, pCond->data.tests.value_test->eq_test->identity_set->super_join, last_cond->data.tests.value_test->eq_test->identity_set->super_join);
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
        if (pCond->data.tests.value_test->eq_test->identity_set || last_cond->data.tests.value_test->eq_test->identity_set)
        {
            IdentitySetSharedPtr pCondIDSet = pCond->data.tests.value_test->eq_test->identity_set ? pCond->data.tests.value_test->eq_test->identity_set->super_join : NULL;
            IdentitySetSharedPtr pLCondIDSet = last_cond->data.tests.value_test->eq_test->identity_set ? last_cond->data.tests.value_test->eq_test->identity_set->super_join : NULL;
            if (pCondIDSet != pLCondIDSet)
            {
                ebc_timers->dependency_analysis->stop();
//                thisAgent->explanationMemory->add_identity_set_mapping(pCond->inst->i_id, IDS_unified_with_singleton, pCond->data.tests.value_test->eq_test->identity_set, last_cond->data.tests.value_test->eq_test->identity_set);
                join_identity_sets(pCond->data.tests.value_test->eq_test->identity_set, last_cond->data.tests.value_test->eq_test->identity_set);
                ebc_timers->dependency_analysis->start();
            }
        }
    }
}

void Explanation_Based_Chunker::literalize_RHS_function_args(const rhs_value rv, uint64_t inst_id)
{
    /* Assign identities of all arguments in rhs fun call to null identity set*/
    cons* fl = rhs_value_to_funcall_list(rv);
    rhs_function_struct* rf = static_cast<rhs_function_struct*>(fl->first);
    cons* c;

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
                rhs_symbol rs = rhs_value_to_rhs_symbol(static_cast<char*>(c->first));
                dprint(DT_RHS_FUN_VARIABLIZATION, "Literalizing RHS function argument %r\n", static_cast<char*>(c->first));
                IdentitySetSharedPtr lIDSet = rs->identity_set_wp.lock();
                if (lIDSet && !rs->referent->is_sti())
                {
//                    thisAgent->explanationMemory->add_identity_set_mapping(inst_id, IDS_literalized_RHS_function_arg, lIDSet, 0);
                    lIDSet->literalize();
                    thisAgent->explanationMemory->increment_stat_rhs_arguments_literalized(m_rule_type);
                }
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
    return pWME->is_singleton;
}
