/*
 * variablization_manager.cpp
 *
 *  Created on: Jul 25, 2013
 *      Author: mazzin
 */

#include "ebc.h"
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
#include "xml.h"

#include <assert.h>

sym_identity_info* Explanation_Based_Chunker::get_variablization(uint64_t index_id)
{
    if (index_id == 0) return NULL;
    auto iter = (*identity_to_var_map).find(index_id);
    if (iter != (*identity_to_var_map).end()) return iter->second; else return NULL;
}

sym_identity_info* Explanation_Based_Chunker::store_variablization(uint64_t pIdentity, Symbol* variable, Symbol* pMatched_sym)
{
    assert(pIdentity);
    sym_identity_info* lVarInfo;
    thisAgent->memoryManager->allocate_with_pool(MP_sym_identity, &lVarInfo);
    lVarInfo->variable_sym = variable;
    variable->var->instantiated_sym = pMatched_sym;
    lVarInfo->identity = this->get_or_create_o_id(variable, m_chunk_new_i_id);
    thisAgent->symbolManager->symbol_add_ref(variable);
    (*identity_to_var_map)[pIdentity] = lVarInfo;
    #ifdef BUILD_WITH_EXPLAINER
    thisAgent->explanationMemory->add_identity_set_mapping(m_chunk_new_i_id, IDS_base_instantiation, pIdentity, lVarInfo->identity, get_ovar_for_o_id(pIdentity), lVarInfo->variable_sym);
    #endif
    return lVarInfo;
}

/* ============================================================================
 *            Variablization_Manager::variablize_lhs_symbol
 *
 * Requires: Test must not be a conjunctive test.
 * Modifies: sym, variablization maps
 * Effect:   Replaces symbol with a variable.  Creates new variable if
 *           necessary.
 * Note:     Caller is responsible for determining whether this symbol should
 *           be variablized.  For example, we check the original symbol
 *           in the production to determine whether it is a literal and should
 *           not be variablized.
 *
 *           For RL rules, identity may be NULL
 *
 * ========================================================================= */

uint64_t Explanation_Based_Chunker::variablize_rhs_symbol(rhs_value &pRhs_val, tc_number lti_link_tc)
{
    char prefix[2];
    Symbol* var;
    sym_identity_info* found_variablization = NULL;

    if (rhs_value_is_funcall(pRhs_val))
    {
        cons* fl = rhs_value_to_funcall_list(pRhs_val);
        cons* c;
        rhs_value lRhsValue, *lc;

        dprint(DT_RHS_FUN_VARIABLIZATION, "Variablizing RHS funcall %r\n", pRhs_val);
        dprint_unification_map(DT_RHS_FUN_VARIABLIZATION);
        for (c = fl->rest; c != NIL; c = c->rest)
        {
            lRhsValue = static_cast<rhs_value>(c->first);
            dprint(DT_RHS_FUN_VARIABLIZATION, "Variablizing RHS funcall argument %r\n", lRhsValue);
            variablize_rhs_symbol(lRhsValue);
            assert(c->first == lRhsValue);
            dprint(DT_RHS_FUN_VARIABLIZATION, "... RHS funcall argument is now   %r\n", static_cast<char*>(c->first));
        }
        /* Overall function does not have an identity */
        return NULL_IDENTITY_SET;
    }

    rhs_symbol rs = rhs_value_to_rhs_symbol(pRhs_val);

    dprint(DT_RHS_VARIABLIZATION, "variablize_rhs_symbol called for %y(%y o%u).\n", rs->referent, get_ovar_for_o_id(rs->o_id), rs->o_id);

    if (rs->o_id)
    {
        dprint(DT_RHS_VARIABLIZATION, "...searching for variablization for %u (%y)...\n", rs->o_id, get_ovar_for_o_id(rs->o_id));
        found_variablization = get_variablization(rs->o_id);
    }
    else
    {
        if (rs->referent->is_sti())
        {
            /* I think this can only occur now when trying to variablize a locally promoted STI.*/
            dprint(DT_RHS_VARIABLIZATION, "...sti with no identity.  Must be architectural or locally promoted.\n");
            return NULL_IDENTITY_SET;
        }
    }
    if (!found_variablization && rs->referent->is_sti())
    {
        /* -- First time we've encountered an unbound rhs var. -- */
        dprint(DT_RHS_VARIABLIZATION, "...is new unbound variable.\n");
        prefix[0] = static_cast<char>(tolower(rs->referent->id->name_letter));
        prefix[1] = 0;
        var = thisAgent->symbolManager->generate_new_variable(prefix);
        dprint(DT_RHS_VARIABLIZATION, "...created new variable for unbound var %y = %y [%u].\n", rs->referent, var, rs->o_id);

        found_variablization = store_variablization(rs->o_id, var, rs->referent);
    }
    if (found_variablization)
    {
        rhs_value lMatchedSym_with_LTI_Link = NULL;

        dprint(DT_RHS_VARIABLIZATION, "... using variablization %y.\n", found_variablization->variable_sym);
        /* MToDo | Add test that symbol is local to the substate analyzed */
        if (rs->referent->is_lti() && lti_link_tc && (rs->referent->tc_num != lti_link_tc))
        {
            dprint(DT_RHS_LTI_LINKING, "Found RHS symbol with LTI link during variablization: %y and LTI %u \n", rs->referent, rs->referent->id->LTI_ID);
            lMatchedSym_with_LTI_Link = pRhs_val;
            rs->referent->tc_num = lti_link_tc;
        }

        thisAgent->symbolManager->symbol_remove_ref(&rs->referent);
        thisAgent->symbolManager->symbol_add_ref(found_variablization->variable_sym);
        rs->referent = found_variablization->variable_sym;
        rs->o_id = found_variablization->identity;

        /* If matched symbol had an LTI link, add the symbol to list of variables that we will later create news LTM-linking actions for */
        if (lMatchedSym_with_LTI_Link)
        {
            dprint(DT_RHS_LTI_LINKING, "Adding %r to local_linked_STIs\n", lMatchedSym_with_LTI_Link);
            local_linked_STIs->push_back(lMatchedSym_with_LTI_Link);
        }
        return rs->o_id;
    }
    else
    {
        assert(!rs->referent->is_sti());
        dprint(DT_RHS_VARIABLIZATION, "...literal RHS symbol, maps to null identity set or has an identity not found on LHS.  Not variablizing.\n");
        dprint_variablization_table(DT_RHS_VARIABLIZATION);
    }
    return NULL_IDENTITY_SET;
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
    sym_identity_info* var_info;

    dprint(DT_LHS_VARIABLIZATION, "Variablizing equality tests in: %t\n", pTest);
    assert(pTest && pTest->eq_test);
    
    if (!pTest->eq_test->data.referent->is_variable())
    {
        if (pTest->eq_test->identity)
    {
        dprint(DT_LHS_VARIABLIZATION, "Variablizing equality test %t [%u] from %t\n", pTest->eq_test, pTest->eq_test->identity, pTest);

        var_info = get_variablization(pTest->eq_test->identity);
        if (var_info)
        {
            thisAgent->symbolManager->symbol_remove_ref(&(pTest->eq_test->data.referent));
            pTest->eq_test->data.referent = var_info->variable_sym;
            thisAgent->symbolManager->symbol_add_ref(var_info->variable_sym);

            pTest->eq_test->identity = var_info->identity;
            dprint(DT_LHS_VARIABLIZATION, "...with found variablization info %y [%u]\n", var_info->variable_sym, var_info->identity);
        } else {
            /* Create a new variable.  If constant is being variablized just used
             * 'c' instead of first letter of id name.  We now avoid using 'o' for
             * non-operators and 's' for non-states.  That makes things
             * clearer in chunks because of standard naming conventions. --- */
            lOldSym = pTest->eq_test->data.referent;
            if (lOldSym->is_sti())
            {
                char prefix_char = static_cast<char>(tolower(lOldSym->id->name_letter));
                if (((prefix_char == 's') || (prefix_char == 'S')) && !lOldSym->id->isa_goal)
                {
                    prefix[0] = 'c';
                } else if (((prefix_char == 'o') || (prefix_char == 'O')) && !lOldSym->id->isa_operator) {
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

            var_info = store_variablization(pTest->eq_test->identity, lNewVariable, pTest->eq_test->data.referent);

            thisAgent->symbolManager->symbol_remove_ref(&lOldSym);
            pTest->eq_test->data.referent = var_info->variable_sym;
            thisAgent->symbolManager->symbol_add_ref(var_info->variable_sym);

            pTest->eq_test->identity = var_info->identity;
            dprint(DT_LHS_VARIABLIZATION, "...with newly created variablization info for new variable %y [%u]\n", var_info->variable_sym, var_info->identity);
        }
        dprint(DT_LHS_VARIABLIZATION, "Equality test is now: %t [%u] and test is %t\n", pTest->eq_test, pTest->eq_test->identity, pTest);
    } else {
        /* Literalized identity, so set identity in chunk to 0 */
        pTest->eq_test->identity = NULL_IDENTITY_SET;
    }
    }
}

/* ============================================================================
 *            Variablization_Manager::variablize_test_by_lookup
 *
 * Requires: Nothing
 * Modifies: t
 * Effect:   Variablizes any symbols in a test that were previously variablized
 *           when variablizing the equality test.
 *
 * ========================================================================= */
bool Explanation_Based_Chunker::variablize_test_by_lookup(test t, bool pSkipTopLevelEqualities)
{
    sym_identity_info* found_variablization = NULL;

    dprint(DT_LHS_VARIABLIZATION, "Variablizing by lookup %t [%u]\n", t, t->identity);

    if (pSkipTopLevelEqualities && (t->type == EQUALITY_TEST))
    {
        /* -- Wrong test type for this variablization pass -- */
        dprint(DT_CONSTRAINTS, "Not variablizing constraint b/c equality test in second variablization pass.\n");
        return true;
    }
    found_variablization =  get_variablization(t->identity);
    if (found_variablization)
    {
        // It has been variablized before, so just variablize
        thisAgent->symbolManager->symbol_remove_ref(&t->data.referent);
        t->data.referent = found_variablization->variable_sym;
        thisAgent->symbolManager->symbol_add_ref(found_variablization->variable_sym);

        t->identity = found_variablization->identity;
        dprint(DT_LHS_VARIABLIZATION, "...with found variablization info %y [%u]\n", found_variablization->variable_sym, found_variablization->identity);
    }
    else
    {
        /* Could be a literalized identity, so set identity in chunk to 0 */
        t->identity = NULL_IDENTITY_SET;
        dprint(DT_LHS_VARIABLIZATION, "%s", t->data.referent->is_sti() ?
            "Ungrounded STI in in relational test.  Will delete during consolidation phase.\n" :
            "Not variablizing constraint b/c referent not grounded in chunk.\n");
        return false;
    }

    dprint(DT_LHS_VARIABLIZATION, "Result: %t [%u]\n", t, t->identity);
    dprint(DT_LHS_VARIABLIZATION, "---------------------------------------\n");

    return true;
}

void Explanation_Based_Chunker::variablize_tests_by_lookup(test t, bool pSkipTopLevelEqualities)
{

    cons* c;
    test tt;
//    dprint(DT_LHS_VARIABLIZATION, "Variablizing by lookup tests in: %t\n", t);

    assert(t);

    if (t->type == CONJUNCTIVE_TEST)
    {
        dprint(DT_LHS_VARIABLIZATION, "Iterating through conjunction list.\n");
        for (c = t->data.conjunct_list; c != NIL; )
        {
            dprint(DT_LHS_VARIABLIZATION, "Variablizing conjunctive test: \n");
            /* -- Note that we ignore what variablize_test_by_lookup returns b/c merge will later delete
             *    any ungrounded tests on STI's.  Any ungrounded tests on non-STIs do not need to be
             *    deleted.  We just leave them as a literal. We only use the return value of
             *    variablize_test_by_lookup when variablizing constraints collected during
             *    backtracing, since we can just avoid adding them to the condition list. -- */
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

        dprint(DT_LHS_VARIABLIZATION, "Done iterating through conjunction list.\n");
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
    dprint_header(DT_LHS_VARIABLIZATION, PrintBoth, "Variablizing LHS condition list:\n");

    dprint(DT_LHS_VARIABLIZATION, "Pass 1: Variablizing equality tests in positive conditions...\n");

    if (!pInNegativeCondition)
    {
        for (condition* cond = top_cond; cond != NIL; cond = cond->next)
        {
            if (cond->type == POSITIVE_CONDITION)
            {
                dprint_header(DT_LHS_VARIABLIZATION, PrintBoth, "Variablizing LHS positive condition equality tests: %l\n", cond);
                variablize_equality_tests(cond->data.tests.id_test);
                variablize_equality_tests(cond->data.tests.attr_test);
                variablize_equality_tests(cond->data.tests.value_test);
            }
            else if (cond->type == NEGATIVE_CONDITION)
            {
                dprint_header(DT_LHS_VARIABLIZATION, PrintBoth, "Variablizing LHS NC equality test for id in: %l\n", cond);
                variablize_equality_tests(cond->data.tests.id_test);
            }
        }
    }
    dprint(DT_LHS_VARIABLIZATION, "Pass 2: Variablizing all other LHS tests via lookup only:\n");
    for (condition* cond = top_cond; cond != NIL; cond = cond->next)
    {
        if (cond->type == POSITIVE_CONDITION)
        {
            dprint_header(DT_LHS_VARIABLIZATION, PrintBoth, "Variablizing LHS positive non-equality tests: %l\n", cond);
            if ((cond->data.tests.id_test->type == CONJUNCTIVE_TEST) || pInNegativeCondition)
                variablize_tests_by_lookup(cond->data.tests.id_test, !pInNegativeCondition);
            if ((cond->data.tests.attr_test->type == CONJUNCTIVE_TEST) || pInNegativeCondition)
                variablize_tests_by_lookup(cond->data.tests.attr_test, !pInNegativeCondition);
            if ((cond->data.tests.value_test->type == CONJUNCTIVE_TEST) || pInNegativeCondition)
                variablize_tests_by_lookup(cond->data.tests.value_test, !pInNegativeCondition);
        }
        else if (cond->type == NEGATIVE_CONDITION)
        {
            dprint_header(DT_LHS_VARIABLIZATION, PrintBoth, "Variablizing LHS negative condition: %l\n", cond);
            variablize_tests_by_lookup(cond->data.tests.id_test, !pInNegativeCondition);
            variablize_tests_by_lookup(cond->data.tests.attr_test, !pInNegativeCondition);
            variablize_tests_by_lookup(cond->data.tests.value_test, !pInNegativeCondition);
        }
        else if (cond->type == CONJUNCTIVE_NEGATION_CONDITION)
        {
            dprint_header(DT_NCC_VARIABLIZATION, PrintBoth, "Variablizing LHS negative conjunctive condition:\n");
            dprint_noprefix(DT_NCC_VARIABLIZATION, "%1", cond->data.ncc.top);
            variablize_condition_list(cond->data.ncc.top, true);
        }
    }
    dprint_header(DT_LHS_VARIABLIZATION, PrintAfter, "Done variablizing LHS condition list.\n");
}

// creates an action for a template instantiation
action* Explanation_Based_Chunker::variablize_rl_action(action* pRLAction, struct token_struct* tok, wme* w, double & initial_value)
{
    action* rhs;
    Symbol* id_sym, *attr_sym, *val_sym, *ref_sym;
    char first_letter;

    // get the preference value
    id_sym = instantiate_rhs_value(thisAgent, pRLAction->id, -1, 's', tok, w);
    attr_sym = instantiate_rhs_value(thisAgent, pRLAction->attr, id_sym->id->level, 'a', tok, w);
    first_letter = first_letter_from_symbol(attr_sym);
    val_sym = instantiate_rhs_value(thisAgent, pRLAction->value, id_sym->id->level, first_letter, tok, w);
    ref_sym = instantiate_rhs_value(thisAgent, pRLAction->referent, id_sym->id->level, first_letter, tok, w);

    rhs = make_action(thisAgent);
    rhs->type = MAKE_ACTION;
    rhs->preference_type = NUMERIC_INDIFFERENT_PREFERENCE_TYPE;

    rhs->id = allocate_rhs_value_for_symbol(thisAgent, id_sym, rhs_value_to_o_id(pRLAction->id));
    rhs->attr = allocate_rhs_value_for_symbol(thisAgent, attr_sym, rhs_value_to_o_id(pRLAction->attr));
    rhs->value = allocate_rhs_value_for_symbol(thisAgent, val_sym, rhs_value_to_o_id(pRLAction->value));
    rhs->referent = allocate_rhs_value_for_symbol(thisAgent, ref_sym, rhs_value_to_o_id(pRLAction->referent));

    /* instantiate and allocate both increased refcount by 1.  Decrease one here.  Variablize may decrease also */
    thisAgent->symbolManager->symbol_remove_ref(&id_sym);
    thisAgent->symbolManager->symbol_remove_ref(&attr_sym);
    thisAgent->symbolManager->symbol_remove_ref(&val_sym);
    thisAgent->symbolManager->symbol_remove_ref(&ref_sym);

    if (ref_sym->symbol_type == INT_CONSTANT_SYMBOL_TYPE)
    {
        initial_value = static_cast< double >(ref_sym->ic->value);
    }
    else if (ref_sym->symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE)
    {
        initial_value = ref_sym->fc->value;
    } else {
        deallocate_action_list(thisAgent, rhs);
        return NULL;
    }

    dprint(DT_RL_VARIABLIZATION, "Variablizing action: %a\n", rhs);

    tc_number lti_link_tc = get_new_tc_number(thisAgent);
    variablize_rhs_symbol(rhs->id, lti_link_tc);
    variablize_rhs_symbol(rhs->attr, lti_link_tc);
    variablize_rhs_symbol(rhs->value, lti_link_tc);
    variablize_rhs_symbol(rhs->referent, lti_link_tc);

    dprint(DT_RL_VARIABLIZATION, "Created variablized action: %a\n", rhs);

    return rhs;
}

action* Explanation_Based_Chunker::variablize_result_into_actions(preference* result, tc_number lti_link_tc)
{

    std::unordered_map< uint64_t, uint64_t >::iterator iter;
    uint64_t lO_id = 0;

    action* a = make_action(thisAgent);
    a->type = MAKE_ACTION;
    a->preference_type = result->type;

    if (!result->rhs_funcs.id)
    {
        iter = (*unification_map).find(result->identities.id);
        if (iter != (*unification_map).end())
        {
            lO_id = iter->second;
        } else {
            lO_id = result->identities.id;
        }
        a->id = allocate_rhs_value_for_symbol(thisAgent, result->id, lO_id);
    } else {
        a->id = copy_rhs_value(thisAgent, result->rhs_funcs.id, true);
    }
    if (!result->rhs_funcs.attr)
    {
        iter = (*unification_map).find(result->identities.attr);
        if (iter != (*unification_map).end())
        {
            lO_id = iter->second;
        } else {
            lO_id = result->identities.attr;
        }
        a->attr = allocate_rhs_value_for_symbol(thisAgent, result->attr, lO_id);
    } else {
        a->attr = copy_rhs_value(thisAgent, result->rhs_funcs.attr, true);
    }
    if (!result->rhs_funcs.value)
    {
        iter = (*unification_map).find(result->identities.value);
        if (iter != (*unification_map).end())
        {
            lO_id = iter->second;
        } else {
            lO_id = result->identities.value;
        }
        a->value = allocate_rhs_value_for_symbol(thisAgent, result->value, lO_id);
    } else {
        a->value = copy_rhs_value(thisAgent, result->rhs_funcs.value, true);
    }
    if (preference_is_binary(result->type))
    {
        iter = (*unification_map).find(result->identities.referent);
        if (iter != (*unification_map).end())
        {
            lO_id = iter->second;
        } else {
            lO_id = result->identities.referent;
        }
        a->referent = allocate_rhs_value_for_symbol(thisAgent, result->referent, lO_id);
    }

    dprint_set_indents(DT_RHS_VARIABLIZATION, "");
    dprint(DT_RHS_VARIABLIZATION, "Variablizing preference for %p\n", result);
    dprint_clear_indents(DT_RHS_VARIABLIZATION);

    lO_id = variablize_rhs_symbol(a->id, lti_link_tc);
    if (!result->rhs_funcs.id)
    {
        result->clone_identities.id = lO_id;
    } else {
        result->clone_identities.id = lO_id;
        result->cloned_rhs_funcs.id = a->id;
//        a->id = copy_rhs_value(thisAgent, result->rhs_funcs.id, true);
        a->id = copy_rhs_value(thisAgent, result->cloned_rhs_funcs.id, true);
    }

    lO_id = variablize_rhs_symbol(a->attr, lti_link_tc);
    if (!result->rhs_funcs.attr)
    {
        result->clone_identities.attr = lO_id;
    } else {
        result->clone_identities.attr = lO_id;
        result->cloned_rhs_funcs.attr = a->attr;
//        a->attr = copy_rhs_value(thisAgent, result->rhs_funcs.attr, true);
        a->attr = copy_rhs_value(thisAgent, result->cloned_rhs_funcs.attr, true);
    }

    lO_id = variablize_rhs_symbol(a->value, lti_link_tc);
    if (!result->rhs_funcs.value)
    {
        result->clone_identities.value = lO_id;
    } else {
        result->clone_identities.value = lO_id;
        result->cloned_rhs_funcs.value = a->value;
//        a->value = copy_rhs_value(thisAgent, result->rhs_funcs.value, true);
        a->value = copy_rhs_value(thisAgent, result->cloned_rhs_funcs.value, true);
    }

    if (preference_is_binary(result->type))
    {
        lO_id = variablize_rhs_symbol(a->referent, lti_link_tc);
        result->clone_identities.referent = lO_id;
    }

    dprint(DT_RHS_VARIABLIZATION, "Variablized result: %a\n", a);

    return a;
}

void Explanation_Based_Chunker::add_LTM_linking_actions(action* pLastAction)
{
    dprint(DT_RHS_LTI_LINKING, "Adding linked local STIs...\n");
    assert(pLastAction);

    rhs_symbol lRSym = NULL;
    rhs_value lRV = NULL, lIntRV = NULL, lNewFuncallList = NULL;
    action* lAction = NULL;

    cons* funcall_list;

    for (auto it = local_linked_STIs->begin(); it != local_linked_STIs->end(); it++)
    {
        lRSym = rhs_value_to_rhs_symbol(*it);
        lIntRV = allocate_rhs_value_for_symbol_no_refcount(thisAgent, thisAgent->symbolManager->make_int_constant(lRSym->referent->var->instantiated_sym->id->LTI_ID), lRSym->o_id);
        lRV = copy_rhs_value(thisAgent, (*it));

        dprint_noprefix(DT_RHS_LTI_LINKING, "Creating action to linking %y to LTI ID %u", lRSym->referent, lRSym->referent->var->instantiated_sym->id->LTI_ID);

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

action* Explanation_Based_Chunker::variablize_results_into_actions()
{
    dprint(DT_VARIABLIZATION_MANAGER, "Result preferences before variablizing: \n%6", NULL, m_results);
    dprint_unification_map(DT_RHS_VARIABLIZATION);

    action* returnAction, *lAction, *lLastAction;
    preference* lPref;

    local_linked_STIs->clear();
    thisAgent->symbolManager->reset_variable_generator(m_vrblz_top, NIL);
    tc_number lti_link_tc = get_new_tc_number(thisAgent);
    returnAction = lAction = lLastAction = NULL;

    for (lPref = m_results; lPref; lPref = lPref->next_result)
    {
        lAction = variablize_result_into_actions(lPref, lti_link_tc);
        if (!returnAction)  returnAction = lAction;
        if (lLastAction) lLastAction->next = lAction;
        lLastAction = lAction;
    }

    dprint(DT_VARIABLIZATION_MANAGER, "Actions after variablizing: \n%2", returnAction);

    if (!local_linked_STIs->empty())
    {
        add_LTM_linking_actions(lLastAction);
        dprint(DT_VARIABLIZATION_MANAGER, "Actions after adding LTM linking actions: \n%2", returnAction);
    }
    return returnAction;
}

void Explanation_Based_Chunker::reinstantiate_test (test pTest)
{
    if (pTest->type == CONJUNCTIVE_TEST)
    {
        for (cons* c = pTest->data.conjunct_list; c != NIL; c = c->rest)
        {
            reinstantiate_test(static_cast<test>(c->first));
        }
    }
    else if (test_has_referent(pTest) && pTest->data.referent->is_variable() && pTest->data.referent->var->instantiated_sym)
    {
    /* We test for pTest->data.referent->var->instantiated_sym because that won't exist for variables in NCCs
     * that don't appear in a positive condition.  Those do not match anything and cannot be reinstantiated */
        Symbol* oldSym = pTest->data.referent;
        pTest->data.referent = pTest->data.referent->var->instantiated_sym;
        thisAgent->symbolManager->symbol_add_ref(pTest->data.referent);
        thisAgent->symbolManager->symbol_remove_ref(&oldSym);
    }
}

void sanity_test_chunk (test pTest)
{
    if (pTest->type == CONJUNCTIVE_TEST)
    {
        for (cons* c = pTest->data.conjunct_list; c != NIL; c = c->rest)
        {
            sanity_test_chunk(static_cast<test>(c->first));
        }
    } else {
        assert(!test_has_referent(pTest) || !pTest->data.referent->is_sti());
    }
}

void sanity_check_conditions(condition* top_cond)
{
    for (condition* cond = top_cond; cond != NIL; cond = cond->next)
    {
        if (cond->type != CONJUNCTIVE_NEGATION_CONDITION)
        {
//            dprint_header(DT_LHS_VARIABLIZATION, PrintBoth, "Variablizing LHS positive non-equality tests: %l\n", cond);
            if (cond->data.tests.id_test->type == CONJUNCTIVE_TEST)
                sanity_test_chunk(cond->data.tests.id_test);
            if (cond->data.tests.attr_test->type == CONJUNCTIVE_TEST)
                sanity_test_chunk(cond->data.tests.attr_test);
            if (cond->data.tests.value_test->type == CONJUNCTIVE_TEST)
                sanity_test_chunk(cond->data.tests.value_test);
        }
        else
        {
//            dprint_header(DT_NCC_VARIABLIZATION, PrintBoth, "Variablizing LHS negative conjunctive condition:\n");
//            dprint_noprefix(DT_NCC_VARIABLIZATION, "%1", cond->data.ncc.top);
            sanity_check_conditions(cond->data.ncc.top);
        }
    }
}

void sanity_test_justification (test pTest)
{
    if (pTest->type == CONJUNCTIVE_TEST)
    {
        for (cons* c = pTest->data.conjunct_list; c != NIL; c = c->rest)
        {
            sanity_test_justification(static_cast<test>(c->first));
        }
    } else {
        assert(!test_has_referent(pTest) || !pTest->data.referent->is_variable());
    }
}

condition* Explanation_Based_Chunker::reinstantiate_condition_list(condition* top_cond)
{
    dprint_header(DT_REINSTANTIATE, PrintBoth, "Reversing variablization of all LHS Condition list:\n");

    condition* last_cond, *lCond, *inst_top;
    last_cond = inst_top = lCond = NULL;

    for (condition* cond = m_vrblz_top; cond != NIL; cond = cond->next)
    {
        dprint(DT_REINSTANTIATE, "Reversing variablization of condition: %l\n", cond);

        if (m_rule_type == ebc_justification)
        {
            if (cond->type != CONJUNCTIVE_NEGATION_CONDITION)
            {
                reinstantiate_test(cond->data.tests.id_test);
                reinstantiate_test(cond->data.tests.attr_test);
                reinstantiate_test(cond->data.tests.value_test);
//                if (cond->type == POSITIVE_CONDITION)
//                {
//                    sanity_test_justification(cond->data.tests.id_test);
//                    sanity_test_justification(cond->data.tests.attr_test);
//                    sanity_test_justification(cond->data.tests.value_test);
//                }
            } else {
                for (condition* ncond = cond->data.ncc.top; ncond != NIL; ncond = ncond->next)
                {
                    reinstantiate_test(ncond->data.tests.id_test);
                    reinstantiate_test(ncond->data.tests.attr_test);
                    reinstantiate_test(ncond->data.tests.value_test);
                }
            }
            lCond = copy_condition(thisAgent, cond, false, false);
            lCond->inst = cond->inst;
            lCond->bt = cond->bt;
        } else {
            lCond = copy_condition(thisAgent, cond, false, false);
            lCond->inst = cond->inst;
            lCond->bt = cond->bt;
            if (cond->type != CONJUNCTIVE_NEGATION_CONDITION)
            {
                reinstantiate_test(lCond->data.tests.id_test);
                reinstantiate_test(lCond->data.tests.attr_test);
                reinstantiate_test(lCond->data.tests.value_test);
//                if (cond->type == POSITIVE_CONDITION)
//                {
//                    sanity_test_justification(lCond->data.tests.id_test);
//                    sanity_test_justification(lCond->data.tests.attr_test);
//                    sanity_test_justification(lCond->data.tests.value_test);
//                }
            } else {
                for (condition* ncond = lCond->data.ncc.top; ncond != NIL; ncond = ncond->next)
                {
                    reinstantiate_test(ncond->data.tests.id_test);
                    reinstantiate_test(ncond->data.tests.attr_test);
                    reinstantiate_test(ncond->data.tests.value_test);
                }
            }
        }

        if (last_cond)
        {
            last_cond->next = lCond;
        } else {
            inst_top = lCond;
        }
            lCond->prev = last_cond;
        last_cond = lCond;
    }
    if (last_cond)
    {
        last_cond->next = NULL;
        }
    else
    {
        inst_top = NULL;
    }

    dprint_header(DT_REINSTANTIATE, PrintAfter, "Done reversing variablization of LHS condition list.\n");
    return inst_top;
}

void Explanation_Based_Chunker::reinstantiate_rhs_symbol(rhs_value pRhs_val)
{

    Symbol* var;

    if (rhs_value_is_funcall(pRhs_val))
    {
        cons* fl = rhs_value_to_funcall_list(pRhs_val);
        cons* c;

        for (c = fl->rest; c != NULL; c = c->rest)
        {
            dprint(DT_RHS_FUN_VARIABLIZATION, "Reversing variablization of funcall RHS value %r\n", static_cast<char*>(c->first));
            reinstantiate_rhs_symbol(static_cast<char*>(c->first));
            dprint(DT_RHS_FUN_VARIABLIZATION, "... RHS value is now %r\n", static_cast<char*>(c->first));
        }
        return;
    }

    rhs_symbol rs = rhs_value_to_rhs_symbol(pRhs_val);

    if (rs->referent->is_variable())
    {
        dprint(DT_REINSTANTIATE, "Reversing variablization for RHS symbol %y (%y/o%u) -> %y.\n", rs->referent, get_ovar_for_o_id(rs->o_id), rs->o_id, rs->referent->var->instantiated_sym);
        Symbol* oldSym = rs->referent;
        rs->referent = rs->referent->var->instantiated_sym;
        thisAgent->symbolManager->symbol_add_ref(rs->referent);
        thisAgent->symbolManager->symbol_remove_ref(&oldSym);
    } else {
        dprint(DT_REINSTANTIATE, "Not a variable.  Ignoring %y [%u]\n", rs->referent, rs->o_id);
    }
    assert(!rs->referent->is_variable());

}

void Explanation_Based_Chunker::reinstantiate_actions(action* pActionList)
{
    for (action* lAction = pActionList; lAction != NULL; lAction = lAction->next)
    {
        if (lAction->type == MAKE_ACTION)
        {
            reinstantiate_rhs_symbol(lAction->id);
            reinstantiate_rhs_symbol(lAction->attr);
            reinstantiate_rhs_symbol(lAction->value);
            if (lAction->referent)
            {
                reinstantiate_rhs_symbol(lAction->referent);
            }
        }
    }
}

condition* Explanation_Based_Chunker::reinstantiate_current_rule()
{
    dprint(DT_REINSTANTIATE, "m_vrblz_top before reinstantiation: \n%1", m_vrblz_top);

    condition* returnConds = reinstantiate_condition_list(m_vrblz_top);

    dprint(DT_REINSTANTIATE, "m_vrblz_top after reinstantiation: \n%1", m_vrblz_top);

    if (m_rule_type == ebc_justification)
    {
        dprint(DT_REINSTANTIATE, "m_rhs before reinstantiation: \n%2", m_rhs);
        reinstantiate_actions(m_rhs);
        dprint(DT_REINSTANTIATE, "m_rhs after reinstantiation: \n%2", m_rhs);
    }

    return returnConds;
}

void Explanation_Based_Chunker::wrap_with_lti_link(rhs_value &pRhs_val, uint64_t pLTI_ID)
{
    assert(rhs_value_is_symbol(pRhs_val));
    cons* funcall_list = NULL;
    dprint(DT_RHS_LTI_LINKING, "Wrapping rhs value into rhs function (@ %r %u)\n", pRhs_val, pLTI_ID);
    push(thisAgent, lti_link_function, funcall_list);
    push(thisAgent, pRhs_val, funcall_list);
    push(thisAgent, thisAgent->symbolManager->make_int_constant(pLTI_ID), funcall_list);
    funcall_list = destructively_reverse_list(funcall_list);
    pRhs_val = funcall_list_to_rhs_value(funcall_list);
    dprint(DT_RHS_LTI_LINKING, "rhs_value is now %r\n", pRhs_val);
}
