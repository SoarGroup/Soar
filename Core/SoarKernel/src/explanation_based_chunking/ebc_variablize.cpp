/*
 * variablization_manager.cpp
 *
 *  Created on: Jul 25, 2013
 *      Author: mazzin
 */

#include "ebc.h"
#include "ebc_identity.h"

#include "agent.h"
#include "dprint.h"
#include "explanation_memory.h"
#include "instantiation.h"
#include "condition.h"
#include "preference.h"
#include "symbol.h"
#include "symbol_manager.h"
#include "test.h"
#include "print.h"
#include "rhs.h"

uint64_t Explanation_Based_Chunker::variablize_rhs_value(rhs_value &pRhs_val, tc_number lti_link_tc, bool pStoreCloneIDForRHSFunction)
{
    char prefix[2];
    Symbol* var;
    bool has_variablization = false;

    if (rhs_value_is_funcall(pRhs_val))
    {
        cons* fl = rhs_value_to_funcall_list(pRhs_val);
        cons* c;
        rhs_value lRhsValue, *lc;

        dprint(DT_RHS_FUN_VARIABLIZATION, "Variablizing RHS funcall %r\n", pRhs_val);
        for (c = fl->rest; c != NIL; c = c->rest)
        {
            lRhsValue = static_cast<rhs_value>(c->first);
            dprint(DT_RHS_FUN_VARIABLIZATION, "Variablizing RHS funcall argument %r\n", lRhsValue);
            variablize_rhs_value(lRhsValue);
            dprint(DT_RHS_FUN_VARIABLIZATION, "... RHS funcall argument is now   %r\n", static_cast<char*>(c->first));
        }
        /* Overall function does not have an identity */
        return LITERAL_VALUE;
    }

    rhs_symbol rs = rhs_value_to_rhs_symbol(pRhs_val);
    Identity* l_identity = rs->identity;

    dprint(DT_RHS_VARIABLIZATION, "variablize_rhs_symbol called for %y [%u].\n", rs->referent, l_identity ? l_identity->get_identity() : 0);

    if (l_identity)
    {
        dprint(DT_RHS_VARIABLIZATION, "...searching for variablization of identity %u...\n", l_identity->get_identity());
        has_variablization = (l_identity->get_var() != NULL);
    }
    else
    {
        if (rs->referent->is_sti())
        {
            /* Can this still occur now.  I thought it could when trying to variablize a locally promoted STI, but assert hasn't fired in anything.*/
            assert(false);
            dprint(DT_RHS_VARIABLIZATION, "...sti with no identity.  Must be architectural or locally promoted.\n");
            return LITERAL_VALUE;
        }
    }
    if (!has_variablization && rs->referent->is_sti())
    {
        /* -- First time we've encountered an unbound rhs var. -- */
        prefix[0] = static_cast<char>(tolower(rs->referent->id->name_letter));
        prefix[1] = 0;
        var = thisAgent->symbolManager->generate_new_variable(prefix);
        dprint(DT_RHS_VARIABLIZATION, "...created new variable for unbound var %y = %y [%us%u].\n", rs->referent, var, rs->inst_identity, l_identity ? l_identity->get_identity() : 0);

        l_identity->store_variablization(var, rs->referent);
        has_variablization = true;
    }
    if (has_variablization)
    {
        rhs_value lMatchedSym_with_LTI_Link = NULL;
        Symbol* new_var = l_identity->get_var();

        dprint(DT_RHS_VARIABLIZATION, "... using variablization %y with identity %u.\n", new_var, l_identity->get_identity());
        if (rs->referent->is_lti() && lti_link_tc && (rs->referent->id->level == m_inst->match_goal_level) && (rs->referent->tc_num != lti_link_tc))
        {
            dprint(DT_RHS_LTI_LINKING, "Found RHS symbol with LTI link during variablization: %y and LTI %u \n", rs->referent, rs->referent->id->LTI_ID);
            lMatchedSym_with_LTI_Link = pRhs_val;
            rs->referent->tc_num = lti_link_tc;
        }

        thisAgent->symbolManager->symbol_remove_ref(&rs->referent);
        thisAgent->symbolManager->symbol_add_ref(new_var);
        rs->referent = new_var;
        rs->inst_identity = l_identity->get_identity();
        uint64_t returnID = l_identity->get_clone_identity();
        if (pStoreCloneIDForRHSFunction)
        	rs->inst_identity = returnID;
        rs->identity = NULL;

        /* If matched symbol had an LTI link, add the symbol to list of variables that we will later create news LTM-linking actions for */
        if (lMatchedSym_with_LTI_Link)
        {
            dprint(DT_RHS_LTI_LINKING, "Adding %r to local_linked_STIs\n", lMatchedSym_with_LTI_Link);
            local_linked_STIs->push_back(lMatchedSym_with_LTI_Link);
        }
        return returnID;
    }
    rs->identity = NULL;
    rs->inst_identity = LITERAL_VALUE;
    return LITERAL_VALUE;
}

/* ============================================================================
 *            Variablization_Manager::variablize_equality_tests
 *
 * Requires: Test from positive condition.
 * Modifies: t
 * Effect:   Variablizes all equality tests in t, even if t is a conjunctive
 *           test.
 *
 * ========================================================================= */

void Explanation_Based_Chunker::variablize_equality_tests(test pTest)
{
    cons* c;
    test tt;
    char prefix[2];
    Symbol* lNewVariable = NULL;
    Symbol* lOldSym;

    if (!pTest->eq_test->data.referent->is_variable())
    {
        if (pTest->eq_test->identity && !pTest->eq_test->identity->literalized())
        {
            dprint(DT_LHS_VARIABLIZATION, "Variablizing equality test %t %g from %t %g\n", pTest->eq_test, pTest->eq_test, pTest, pTest);
            Symbol* new_var = pTest->eq_test->identity->get_var();
            if (new_var)
            {
                thisAgent->symbolManager->symbol_remove_ref(&(pTest->eq_test->data.referent));
                pTest->eq_test->data.referent = new_var;
                thisAgent->symbolManager->symbol_add_ref(new_var);
                pTest->eq_test->inst_identity = pTest->eq_test->identity->get_identity();
                pTest->eq_test->chunk_inst_identity = pTest->eq_test->identity->get_clone_identity();
                dprint(DT_LHS_VARIABLIZATION, "...with found variablization info %y [%u/%u]\n", new_var, pTest->eq_test->inst_identity, pTest->eq_test->chunk_inst_identity);
                clear_test_identity(thisAgent, pTest->eq_test);
            } else {
                lOldSym = pTest->eq_test->data.referent;
                if (lOldSym->is_sti())
                {
                    char prefix_char = static_cast<char>(tolower(lOldSym->id->name_letter));
                    if ((((prefix_char == 's') || (prefix_char == 'S')) && !lOldSym->id->isa_goal) ||
                        (((prefix_char == 'o') || (prefix_char == 'O')) && !lOldSym->id->isa_operator))
                    {
                        prefix[0] = 'c';
                    } else {
                        prefix[0] = prefix_char;
                    }
                }
                else
                {
                    prefix[0] = 'c';
                }
                prefix[1] = 0;
                lNewVariable = thisAgent->symbolManager->generate_new_variable(prefix);

                pTest->eq_test->identity->store_variablization(lNewVariable, pTest->eq_test->data.referent);

                thisAgent->symbolManager->symbol_remove_ref(&lOldSym);
                pTest->eq_test->data.referent = lNewVariable;
                thisAgent->symbolManager->symbol_add_ref(lNewVariable);

                pTest->eq_test->inst_identity = pTest->eq_test->identity->get_identity();
                pTest->eq_test->chunk_inst_identity = pTest->eq_test->identity->get_clone_identity();
                dprint(DT_LHS_VARIABLIZATION, "...with newly created variablization info for new variable %y [%u/%u]\n", lNewVariable, pTest->eq_test->inst_identity, pTest->eq_test->chunk_inst_identity);

                clear_test_identity(thisAgent, pTest->eq_test);

            }
        } else {
            pTest->eq_test->inst_identity = LITERAL_VALUE;
            pTest->eq_test->chunk_inst_identity = LITERAL_VALUE;
            clear_test_identity(thisAgent, pTest->eq_test);
        }
    }
}

bool Explanation_Based_Chunker::variablize_test_by_lookup(test t, bool pSkipTopLevelEqualities)
{

    if (pSkipTopLevelEqualities && (t->type == EQUALITY_TEST)) return true;
    Symbol* new_var = t->identity ? t->identity->get_var() : NULL;

    if (new_var)
    {
        dprint(DT_LHS_VARIABLIZATION, "Variablizing by lookup %t %g...with found variablization info %y [%u]\n", t, t, new_var, t->identity->get_clone_identity());
        thisAgent->symbolManager->symbol_remove_ref(&t->data.referent);
        t->data.referent = new_var;
        thisAgent->symbolManager->symbol_add_ref(new_var);
        t->inst_identity = t->identity->get_identity();
        t->chunk_inst_identity = t->identity->get_clone_identity();
        clear_test_identity(thisAgent, t);
        dprint(DT_LHS_VARIABLIZATION, "--> t: %t %g\n", t, t);
    }
    else
    {
        t->inst_identity = LITERAL_VALUE;
        t->chunk_inst_identity = LITERAL_VALUE;
        clear_test_identity(thisAgent, t);
        dprint(DT_LHS_VARIABLIZATION, "%s", t->data.referent->is_sti() ?
            "Ungrounded STI in in relational test.  Will delete.\n" :
            "Not variablizing constraint b/c referent not grounded in chunk.\n");
        return false;
    }
    return true;
}

void Explanation_Based_Chunker::variablize_tests_by_lookup(test t, bool pSkipTopLevelEqualities)
{

    cons* c;
    test tt;

    if (t->type == CONJUNCTIVE_TEST)
    {
        for (c = t->data.conjunct_list; c != NIL; )
        {
            /* Any ungrounded tests on non-STIs do not need to be deleted.  We just leave them as a literal. */
            tt = reinterpret_cast<test>(c->first);
            if (test_has_referent(tt))
            {
                if (tt->data.referent->is_sti())
                {
                    if (!variablize_test_by_lookup(tt, pSkipTopLevelEqualities))
                    {
                        c = delete_test_from_conjunct(thisAgent, &t, c);
                        continue;
                    }
                }
                else if (tt->identity && !tt->data.referent->is_variable())
                {
                    variablize_test_by_lookup(tt, pSkipTopLevelEqualities);
                }
            }
            c = c->rest;
        }
        dprint(DT_LHS_VARIABLIZATION, "---------------------------------------\n");
    }
    else
    {
        if (test_has_referent(t) && t->identity)
        {
            variablize_test_by_lookup(t, pSkipTopLevelEqualities);
        }
    }
}

void Explanation_Based_Chunker::variablize_condition_list(condition* top_cond, bool pInNegativeCondition)
{
    dprint_header(DT_LHS_VARIABLIZATION, PrintBefore, "Variablizing LHS condition list:\n");

    dprint(DT_LHS_VARIABLIZATION, "Pass 1: Variablizing equality tests in positive conditions...\n");

    if (!pInNegativeCondition)
    {
        for (condition* cond = top_cond; cond != NIL; cond = cond->next)
        {
            if (cond->type != CONJUNCTIVE_NEGATION_CONDITION)
            {
                dprint_header(DT_LHS_VARIABLIZATION, PrintBefore, "Variablizing equality test in LHS positive condition: %l\n", cond);
                variablize_equality_tests(cond->data.tests.id_test);
                variablize_equality_tests(cond->data.tests.attr_test);
                variablize_equality_tests(cond->data.tests.value_test);
                dprint(DT_LHS_VARIABLIZATION, "-->variablized equalities in condition: %l\n", cond);
            }
        }
    }
    dprint(DT_LHS_VARIABLIZATION, "Pass 2: Variablizing all other LHS tests via lookup only:\n");
    for (condition* cond = top_cond; cond != NIL; cond = cond->next)
    {
        if (cond->type == POSITIVE_CONDITION)
        {
            dprint_header(DT_LHS_VARIABLIZATION, PrintBefore, "Variablizing LHS positive non-equality tests in: %l\n", cond);
            if ((cond->data.tests.id_test->type == CONJUNCTIVE_TEST) || pInNegativeCondition)
                variablize_tests_by_lookup(cond->data.tests.id_test, !pInNegativeCondition);
            if ((cond->data.tests.attr_test->type == CONJUNCTIVE_TEST) || pInNegativeCondition)
                variablize_tests_by_lookup(cond->data.tests.attr_test, !pInNegativeCondition);
            if ((cond->data.tests.value_test->type == CONJUNCTIVE_TEST) || pInNegativeCondition)
                variablize_tests_by_lookup(cond->data.tests.value_test, !pInNegativeCondition);
            dprint(DT_LHS_VARIABLIZATION, "-->variablized condition: %l\n", cond);
        }
        else if (cond->type == NEGATIVE_CONDITION)
        {
            dprint_header(DT_LHS_VARIABLIZATION, PrintBefore, "Variablizing LHS negative condition: %l\n", cond);
            variablize_tests_by_lookup(cond->data.tests.id_test, !pInNegativeCondition);
            variablize_tests_by_lookup(cond->data.tests.attr_test, !pInNegativeCondition);
            variablize_tests_by_lookup(cond->data.tests.value_test, !pInNegativeCondition);
            dprint(DT_LHS_VARIABLIZATION, "-->variablized negatve condition: %l\n", cond);
        }
        else if (cond->type == CONJUNCTIVE_NEGATION_CONDITION)
        {
            dprint_header(DT_NCC_VARIABLIZATION, PrintBefore, "Variablizing LHS negative conjunctive condition:\n");
            dprint_noprefix(DT_NCC_VARIABLIZATION, "%1", cond->data.ncc.top);
            variablize_condition_list(cond->data.ncc.top, true);
            dprint(DT_LHS_VARIABLIZATION, "-->variablized NCC: %l\n", cond);
        }
    }
    dprint_header(DT_LHS_VARIABLIZATION, PrintAfter, "Done variablizing LHS condition list.\n");
}

action* Explanation_Based_Chunker::variablize_result_into_action(preference* result, tc_number lti_link_tc)
{
    uint64_t l_inst_identity = 0;

    action* a = make_action(thisAgent);
    a->type = MAKE_ACTION;
    a->preference_type = result->type;

    if (!result->rhs_func_inst_identities.id)
    {
        a->id = allocate_rhs_value_for_symbol(thisAgent, result->id, result->inst_identities.id, result->identities.id, result->was_unbound_vars.id);
    } else {
        a->id = copy_rhs_value(thisAgent, result->rhs_func_inst_identities.id);
    }
    if (!result->rhs_func_inst_identities.attr)
    {
        a->attr = allocate_rhs_value_for_symbol(thisAgent, result->attr, result->inst_identities.attr, result->identities.attr, result->was_unbound_vars.attr);
    } else {
        a->attr = copy_rhs_value(thisAgent, result->rhs_func_inst_identities.attr);
    }
    if (!result->rhs_func_inst_identities.value)
    {
        a->value = allocate_rhs_value_for_symbol(thisAgent, result->value, result->inst_identities.value, result->identities.value, result->was_unbound_vars.value);
    } else {
        a->value = copy_rhs_value(thisAgent, result->rhs_func_inst_identities.value);
    }
    if (preference_is_binary(result->type))
    {
        if (!result->rhs_func_inst_identities.referent)
        {
            a->referent = allocate_rhs_value_for_symbol(thisAgent, result->referent, result->inst_identities.referent, result->identities.referent, result->was_unbound_vars.referent);
        } else {
            a->referent = copy_rhs_value(thisAgent, result->rhs_func_inst_identities.referent);
        }
    }

    dprint(DT_RHS_VARIABLIZATION, "Variablizing preference for %p\n", result);

    l_inst_identity = variablize_rhs_value(a->id, lti_link_tc);
    if (!result->rhs_func_inst_identities.id)
    {
        result->chunk_inst_identities.id = l_inst_identity;
    } else {
        result->chunk_inst_identities.id = LITERAL_VALUE;
        result->rhs_func_chunk_inst_identities.id = a->id;
        a->id = copy_rhs_value(thisAgent, result->rhs_func_chunk_inst_identities.id, false, true);
        reinstantiate_rhs_symbol(result->rhs_func_chunk_inst_identities.id);
    }

    l_inst_identity = variablize_rhs_value(a->attr, lti_link_tc);
    if (!result->rhs_func_inst_identities.attr)
    {
        result->chunk_inst_identities.attr = l_inst_identity;
    } else {
        result->chunk_inst_identities.attr = LITERAL_VALUE;
        result->rhs_func_chunk_inst_identities.attr = a->attr;
        a->attr = copy_rhs_value(thisAgent, result->rhs_func_chunk_inst_identities.attr, false, true);
        reinstantiate_rhs_symbol(result->rhs_func_chunk_inst_identities.attr);
    }

    l_inst_identity = variablize_rhs_value(a->value, lti_link_tc);
    if (!result->rhs_func_inst_identities.value)
    {
        result->chunk_inst_identities.value = l_inst_identity;
    } else {
        result->chunk_inst_identities.value = LITERAL_VALUE;
        result->rhs_func_chunk_inst_identities.value = a->value;
        a->value = copy_rhs_value(thisAgent, result->rhs_func_chunk_inst_identities.value, false, true);
        reinstantiate_rhs_symbol(result->rhs_func_chunk_inst_identities.value);
    }

    if (preference_is_binary(result->type))
    {
        l_inst_identity = variablize_rhs_value(a->referent, lti_link_tc);
        if (!result->rhs_func_inst_identities.referent)
        {
            result->chunk_inst_identities.referent = l_inst_identity;
        } else {
            result->chunk_inst_identities.referent = LITERAL_VALUE;
            result->rhs_func_chunk_inst_identities.referent = a->referent;
            a->referent = copy_rhs_value(thisAgent, result->rhs_func_chunk_inst_identities.referent, false, true);
            reinstantiate_rhs_symbol(result->rhs_func_chunk_inst_identities.referent);
        }
    }

    dprint(DT_RHS_VARIABLIZATION, "Variablized result: %a\n", a);

    return a;
}

action* Explanation_Based_Chunker::variablize_results_into_actions()
{
    dprint(DT_VARIABLIZATION_MANAGER, "Result preferences before variablizing: \n%6", NULL, m_results);
    action* returnAction, *lAction, *lLastAction;
    preference* lPref;

    local_linked_STIs->clear();
    thisAgent->symbolManager->reset_variable_generator(m_lhs, NIL);
    tc_number lti_link_tc = get_new_tc_number(thisAgent);
    returnAction = lAction = lLastAction = NULL;

    for (lPref = m_results; lPref; lPref = lPref->next_result)
    {
        lAction = variablize_result_into_action(lPref, lti_link_tc);
        if (!returnAction)  returnAction = lAction;
        if (lLastAction) lLastAction->next = lAction;
        lLastAction = lAction;
    }

    if (!local_linked_STIs->empty() && ebc_settings[SETTING_EBC_ADD_LTM_LINKS])
    {
        add_LTM_linking_actions(lLastAction);
    }

    dprint(DT_VARIABLIZATION_MANAGER, "Actions after variablizing: \n%2", returnAction);

    return returnAction;
}

void Explanation_Based_Chunker::add_LTM_linking_actions(action* pLastAction)
{
    dprint(DT_RHS_LTI_LINKING, "Adding linked local STIs...\n");

    rhs_symbol lRSym = NULL;
    rhs_value lRV = NULL, lIntRV = NULL, lNewFuncallList = NULL;
    action* lAction = NULL;

    cons* funcall_list;

    for (auto it = local_linked_STIs->begin(); it != local_linked_STIs->end(); it++)
    {
        lRSym = rhs_value_to_rhs_symbol(*it);

        lIntRV = allocate_rhs_value_for_symbol_no_refcount(thisAgent, thisAgent->symbolManager->make_int_constant(lRSym->referent->var->instantiated_sym->id->LTI_ID), lRSym->inst_identity, lRSym->identity, false);
        lRV = copy_rhs_value(thisAgent, (*it));

        dprint_noprefix(DT_RHS_LTI_LINKING, "Creating action to linking %y (%u) to LTI ID %u", lRSym->referent, lRSym->identity ? lRSym->identity->get_identity() : 0, lRSym->referent->var->instantiated_sym->id->LTI_ID);

        funcall_list = NULL;
        push(thisAgent, lti_link_function, funcall_list);
        push(thisAgent, lRV, funcall_list);
        push(thisAgent, lIntRV, funcall_list);
        funcall_list = destructively_reverse_list(funcall_list);
        lNewFuncallList = funcall_list_to_rhs_value(funcall_list);

        lAction = make_action(thisAgent);
        lAction->type = FUNCALL_ACTION;
        lAction->value = lNewFuncallList;
        pLastAction->next = lAction;
        pLastAction = lAction;
        dprint(DT_RHS_LTI_LINKING, "Added new action %a\n", lAction);
    }
    dprint_noprefix(DT_RHS_LTI_LINKING, "\n");

}
