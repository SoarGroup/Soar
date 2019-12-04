#include "ebc.h"
#include "ebc_identity.h"
#include "ebc_timers.h"

#include "condition.h"
#include "explanation_memory.h"
#include "instantiation.h"
#include "output_manager.h"
#include "rhs.h"
#include "rhs_functions.h"
#include "test.h"
#include "working_memory.h"

void Explanation_Based_Chunker::unify_lhs_rhs_connection(condition* lhs_cond, identity_set_quadruple &rhs_id_sets, const rhs_quadruple rhs_funcs)
{
    test lId = 0, lAttr = 0, lValue = 0;
    lId = lhs_cond->data.tests.id_test->eq_test;
    lAttr = lhs_cond->data.tests.attr_test->eq_test;
    lValue = lhs_cond->data.tests.value_test->eq_test;

    if (rhs_id_sets.id)
    {
        if (lId->identity)
        {
            if (rhs_id_sets.id->joined_identity != lId->identity->joined_identity)
            {
                thisAgent->explanationMemory->add_identity_set_mapping(lhs_cond->inst->i_id, IDS_join, rhs_id_sets.id, lId->identity);
                join_identities(rhs_id_sets.id, lId->identity);
            }
        } else {
            thisAgent->explanationMemory->add_identity_set_mapping(lhs_cond->inst->i_id, IDS_literalized_LHS_literal, rhs_id_sets.id, NULL);
            rhs_id_sets.id->literalize();
        }
    }
    else
    {
        if (rhs_funcs.id)
        {
            if (rhs_value_is_literalizing_function(rhs_funcs.id))
            {
                literalize_RHS_function_args(rhs_funcs.id, lhs_cond->inst->i_id);
            }
            if (lId->identity)
            {
                thisAgent->explanationMemory->add_identity_set_mapping(lhs_cond->inst->i_id, IDS_literalized_RHS_function_compare, lId->identity, NULL);
                lId->identity->literalize();
            }
        }
        else if (lId->identity)
        {
            thisAgent->explanationMemory->add_identity_set_mapping(lhs_cond->inst->i_id, IDS_literalized_RHS_literal, lId->identity, NULL);
            lId->identity->literalize();
        }
    }
    if (rhs_id_sets.attr)
    {
        if (lAttr->identity)
        {
            if (rhs_id_sets.attr->joined_identity != lAttr->identity->joined_identity)
            {
                thisAgent->explanationMemory->add_identity_set_mapping(lhs_cond->inst->i_id, IDS_join, rhs_id_sets.attr, lAttr->identity);
                join_identities(rhs_id_sets.attr, lAttr->identity);
            }
        } else {
            thisAgent->explanationMemory->add_identity_set_mapping(lhs_cond->inst->i_id, IDS_literalized_LHS_literal, rhs_id_sets.attr, NULL);
            rhs_id_sets.attr->literalize();
        }
    }
    else
    {
        if (rhs_funcs.attr)
        {
            if (rhs_value_is_literalizing_function(rhs_funcs.attr))
            {
                literalize_RHS_function_args(rhs_funcs.attr, lhs_cond->inst->i_id);
            }
            if (lAttr->identity)
            {
                thisAgent->explanationMemory->add_identity_set_mapping(lhs_cond->inst->i_id, IDS_literalized_RHS_function_compare, lAttr->identity, NULL);
                lAttr->identity->literalize();
            }
        }
        else if (lAttr->identity)
        {
            thisAgent->explanationMemory->add_identity_set_mapping(lhs_cond->inst->i_id, IDS_literalized_RHS_literal, lAttr->identity, NULL);
            lAttr->identity->literalize();
        }
    }

    if (rhs_id_sets.value)
    {
        if (lValue->identity)
        {
            if (rhs_id_sets.value->joined_identity != lValue->identity->joined_identity)
            {
                thisAgent->explanationMemory->add_identity_set_mapping(lhs_cond->inst->i_id, IDS_join, rhs_id_sets.value, lValue->identity);
                join_identities(rhs_id_sets.value, lValue->identity);
            }
        } else {
            thisAgent->explanationMemory->add_identity_set_mapping(lhs_cond->inst->i_id, IDS_literalized_LHS_literal, rhs_id_sets.value, NULL);
            rhs_id_sets.value->literalize();
        }
    }
    else
    {
        if (rhs_funcs.value)
        {
            if (rhs_value_is_literalizing_function(rhs_funcs.value))
            {
                literalize_RHS_function_args(rhs_funcs.value, lhs_cond->inst->i_id);
            }
            if (lValue->identity)
            {
                thisAgent->explanationMemory->add_identity_set_mapping(lhs_cond->inst->i_id, IDS_literalized_RHS_function_compare, lValue->identity, NULL);
                lValue->identity->literalize();
            }
        }
        else if (lValue->identity)
        {
            thisAgent->explanationMemory->add_identity_set_mapping(lhs_cond->inst->i_id, IDS_literalized_RHS_literal, lValue->identity, NULL);
            lValue->identity->literalize();
        }
    }

    if (rhs_funcs.referent && rhs_value_is_literalizing_function(rhs_funcs.referent))
    {
        literalize_RHS_function_args(rhs_funcs.referent, lhs_cond->inst->i_id);
    }
}

/* Requires: pCond is being added to grounds and is the second condition being added to grounds
 *           that matched a given wme, which guarantees chunker_bt_last_ground_cond points to the
 *           first condition that matched. */
void Explanation_Based_Chunker::check_for_singleton_unification(condition* pCond)
{
    if (wme_is_a_singleton(pCond->bt.wme_))
    {
        condition* last_cond = pCond->bt.wme_->chunker_bt_last_ground_cond;
        if (pCond->data.tests.value_test->eq_test->identity || last_cond->data.tests.value_test->eq_test->identity)
        {
            if (!pCond->data.tests.value_test->eq_test->identity)
            {
                thisAgent->explanationMemory->add_identity_set_mapping(pCond->inst->i_id, IDS_unified_with_singleton, last_cond->data.tests.value_test->eq_test->identity, NULL);
                last_cond->data.tests.value_test->eq_test->identity->literalize();
            } else if (!last_cond->data.tests.value_test->eq_test->identity)
            {
                thisAgent->explanationMemory->add_identity_set_mapping(pCond->inst->i_id, IDS_unified_with_singleton, pCond->data.tests.value_test->eq_test->identity, NULL);
                pCond->data.tests.value_test->eq_test->identity->literalize();
            } else
            {
                thisAgent->explanationMemory->add_identity_set_mapping(pCond->inst->i_id, IDS_unified_with_singleton, pCond->data.tests.value_test->eq_test->identity, last_cond->data.tests.value_test->eq_test->identity);
                join_identities(pCond->data.tests.value_test->eq_test->identity, last_cond->data.tests.value_test->eq_test->identity);
            }
        }
    }
    /* The code that sets isa_operator checks if an id is a goal, so don't need to check here */
    else if ((pCond->bt.wme_->attr == thisAgent->symbolManager->soarSymbols.operator_symbol) &&
        (pCond->bt.wme_->value->is_sti() &&  pCond->bt.wme_->value->id->isa_operator) &&
        (!pCond->test_for_acceptable_preference))
    {
        condition* last_cond = pCond->bt.wme_->chunker_bt_last_ground_cond;
        if (pCond->data.tests.value_test->eq_test->identity || last_cond->data.tests.value_test->eq_test->identity)
        {
            Identity* pCondIDSet = get_joined_identity(pCond->data.tests.value_test->eq_test->identity);
            Identity* pLCondIDSet = get_joined_identity(last_cond->data.tests.value_test->eq_test->identity);
            if (pCondIDSet != pLCondIDSet)
            {
                thisAgent->explanationMemory->add_identity_set_mapping(pCond->inst->i_id, IDS_unified_with_singleton, pCond->data.tests.value_test->eq_test->identity, last_cond->data.tests.value_test->eq_test->identity);
                join_identities(pCond->data.tests.value_test->eq_test->identity, last_cond->data.tests.value_test->eq_test->identity);
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
                    literalize_RHS_function_args(static_cast<char*>(c->first), inst_id);
                }
            } else {
                rhs_symbol rs = rhs_value_to_rhs_symbol(static_cast<char*>(c->first));
                Identity* l_identity = rs->identity;
                if (l_identity && !rs->referent->is_sti())
                {
                    thisAgent->explanationMemory->add_identity_set_mapping(inst_id, IDS_literalized_RHS_function_arg, l_identity, NULL);
                    l_identity->literalize();
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
    if (!pWME->attr->is_string() || !pWME->attr->sc->singleton.possible) return false;

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
