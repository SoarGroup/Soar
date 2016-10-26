/*
 * variablization_manager.cpp
 *
 *  Created on: Jul 25, 2013
 *      Author: mazzin
 */

#include "ebc.h"
#include "agent.h"
#include "dprint.h"
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

sym_identity_info* Explanation_Based_Chunker::store_variablization(uint64_t pIdentity, Symbol* variable)
{
    assert(variable && pIdentity);
    sym_identity_info* lVarInfo = new sym_identity_info();
    lVarInfo->variable_sym = variable;
    lVarInfo->identity = this->get_or_create_o_id(variable, m_chunk_new_i_id);
//    lVarInfo->identity = pIdentity;
    thisAgent->symbolManager->symbol_add_ref(variable);
    (*identity_to_var_map)[pIdentity] = lVarInfo;
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

uint64_t Explanation_Based_Chunker::variablize_rhs_symbol(rhs_value pRhs_val, bool pShouldCachedMatchValue)
{
    char prefix[2];
    Symbol* var;
    sym_identity_info* found_variablization = NULL;

    if (rhs_value_is_funcall(pRhs_val))
    {
        cons* fl = rhs_value_to_funcall_list(pRhs_val);
        cons* c;

        for (c = fl->rest; c != NIL; c = c->rest)
        {
            dprint(DT_RHS_VARIABLIZATION, "Variablizing RHS value %r\n", static_cast<char*>(c->first));
            variablize_rhs_symbol(static_cast<char*>(c->first), pShouldCachedMatchValue);
            dprint(DT_RHS_VARIABLIZATION, "Variablized RHS value is now %r\n", static_cast<char*>(c->first));
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

        found_variablization = store_variablization(rs->o_id, var);
    }
    if (found_variablization)
    {
        dprint(DT_RHS_VARIABLIZATION, "... using variablization %y.\n", found_variablization->variable_sym);
        if (pShouldCachedMatchValue)
        {
            add_matched_sym_for_rhs_var(found_variablization->variable_sym, rs->referent);
        }
        thisAgent->symbolManager->symbol_remove_ref(&rs->referent);
        rs->referent = found_variablization->variable_sym;
        rs->o_id = found_variablization->identity;
        thisAgent->symbolManager->symbol_add_ref(found_variablization->variable_sym);
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
    Symbol* lNewVariable;
    Symbol* lOldSym;
    sym_identity_info* var_info;

    dprint(DT_LHS_VARIABLIZATION, "Variablizing equality tests in: %t\n", pTest);
    assert(pTest && pTest->eq_test);
    
    if (pTest->eq_test->identity && !pTest->eq_test->data.referent->is_variable())
    {
        dprint(DT_LHS_VARIABLIZATION, "Variablizing equality test %t [%u] from %t\n", pTest->eq_test, pTest->eq_test->identity, pTest);
//        variablize_lhs_symbol(&(pTest->data.referent), pTest->identity);

        var_info = get_variablization(pTest->eq_test->identity);
        if (var_info)
        {
            thisAgent->symbolManager->symbol_remove_ref(&(pTest->eq_test->data.referent));
            pTest->eq_test->data.referent = var_info->variable_sym;
            thisAgent->symbolManager->symbol_add_ref(var_info->variable_sym);
            pTest->eq_test->identity = var_info->identity;
            pTest->eq_test->counterpart_test->identity = var_info->identity;
            pTest->eq_test->counterpart_test->counterpart_test = NULL;
            pTest->eq_test->counterpart_test = NULL;
            dprint(DT_LHS_VARIABLIZATION, "...with found variablization info %y [%u]\n", var_info->variable_sym, var_info->identity);
        } else {

            /* Create a new variable.  If constant is being variablized just used
             * 'c' instead of first letter of id name.  We now don'pTest use 'o' for
             * non-operators and don't use 's' for non-states.  That makes things
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

            var_info = store_variablization(pTest->eq_test->identity, lNewVariable);

            thisAgent->symbolManager->symbol_remove_ref(&lOldSym);
            pTest->eq_test->data.referent = var_info->variable_sym;
            thisAgent->symbolManager->symbol_add_ref(var_info->variable_sym);
            pTest->eq_test->identity = var_info->identity;
            pTest->eq_test->counterpart_test->identity = var_info->identity;
            pTest->eq_test->counterpart_test->counterpart_test = NULL;
            pTest->eq_test->counterpart_test = NULL;
            dprint(DT_LHS_VARIABLIZATION, "...with newly created variablization info for new variable %y [%u]\n", var_info->variable_sym, var_info->identity);
        }
        dprint(DT_LHS_VARIABLIZATION, "Equality test is now: %t [%u] and test is %t\n", pTest->eq_test, pTest->eq_test->identity, pTest);
    } else {
        /* Literalized identity, so set identity in chunk to 0 */
        pTest->eq_test->identity = NULL_IDENTITY_SET;
    }

//    if (t->type == CONJUNCTIVE_TEST)
//    {
//
//        dprint(DT_LHS_VARIABLIZATION, "Iterating through conjunction list.\n");
//        for (c = t->data.conjunct_list; c != NIL; c = c->rest)
//        {
//            tt = reinterpret_cast<test>(c->first);
//            if (tt->type == EQUALITY_TEST)
//            {
//                dprint(DT_LHS_VARIABLIZATION, "Variablizing equality test: %pTest\n", tt);
//                if (tt->identity && !tt->data.referent->is_variable())
//                {
//                    variablize_lhs_symbol(&(tt->data.referent), tt->identity);
//                }
//                dprint(DT_LHS_VARIABLIZATION, "Setting conjunctive test %pTest's eq_test to: %pTest\n", pTest, tt);
//            }
//        }
//
//        dprint(DT_LHS_VARIABLIZATION, "Done iterating through conjunction list.\n");
//        dprint(DT_LHS_VARIABLIZATION, "---------------------------------------\n");
//    }
//    else
//    {
//        if ((pTest->type == EQUALITY_TEST) &&
//            (t->identity && !pTest->data.referent->is_variable()))
//        {
//            dprint(DT_LHS_VARIABLIZATION, "Variablizing equality test %pTest's eq_test is: %pTest\n", pTest, pTest->eq_test);
//            variablize_lhs_symbol(&(pTest->data.referent), pTest->identity);
//            dprint(DT_LHS_VARIABLIZATION, "Equality test %t's new eq_test is: %pTest\n", pTest, pTest->eq_test);
//        }
//    }
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
        t->counterpart_test->identity = found_variablization->identity;
        t->counterpart_test->counterpart_test = NULL;
        t->counterpart_test = NULL;
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
                else if (tt->identity)
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
        }
    }
    dprint(DT_LHS_VARIABLIZATION, "Pass 2: Variablizing all other LHS tests via lookup only:\n");
    for (condition* cond = top_cond; cond != NIL; cond = cond->next)
    {
        if (cond->type == POSITIVE_CONDITION)
        {
            dprint_header(DT_LHS_VARIABLIZATION, PrintBoth, "Variablizing LHS positive non-equality tests: %l\n", cond);
            if (cond->data.tests.id_test->type == CONJUNCTIVE_TEST)
                variablize_tests_by_lookup(cond->data.tests.id_test, !pInNegativeCondition);
            if (cond->data.tests.attr_test->type == CONJUNCTIVE_TEST)
                variablize_tests_by_lookup(cond->data.tests.attr_test, !pInNegativeCondition);
            if (cond->data.tests.value_test->type == CONJUNCTIVE_TEST)
                variablize_tests_by_lookup(cond->data.tests.value_test, !pInNegativeCondition);
        }
        else if (cond->type == NEGATIVE_CONDITION)
        {
            dprint_header(DT_LHS_VARIABLIZATION, PrintBoth, "Variablizing LHS negative condition: %l\n", cond);
            variablize_tests_by_lookup(cond->data.tests.id_test, false);
            variablize_tests_by_lookup(cond->data.tests.attr_test, false);
            variablize_tests_by_lookup(cond->data.tests.value_test, false);
        }
        else if (cond->type == CONJUNCTIVE_NEGATION_CONDITION)
        {
            dprint_header(DT_NCC_VARIABLIZATION, PrintBoth, "Variablizing LHS negative conjunctive condition:\n");
            dprint_noprefix(DT_NCC_VARIABLIZATION, "%1", cond->data.ncc.top);
            variablize_condition_list(cond->data.ncc.top, false);
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

    variablize_rhs_symbol(rhs->id, true);
    variablize_rhs_symbol(rhs->attr);
    variablize_rhs_symbol(rhs->value);
    variablize_rhs_symbol(rhs->referent);

    dprint(DT_RL_VARIABLIZATION, "Created variablized action: %a\n", rhs);

    return rhs;
}

action* Explanation_Based_Chunker::variablize_result_into_actions(preference* result, bool variablize)
{
    action* a;

    if (!result)
    {
        return NIL;
    }
    std::unordered_map< uint64_t, uint64_t >::iterator iter;
    uint64_t lO_id = 0;

    a = make_action(thisAgent);
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

    if (variablize)
    {

        lO_id = variablize_rhs_symbol(a->id, true);
//        if (!result->rhs_funcs.id) result->clone_identities.id = lO_id;
        if (!result->rhs_funcs.id) result->identities.id = lO_id;

        lO_id = variablize_rhs_symbol(a->attr);
//        if (!result->rhs_funcs.attr) result->clone_identities.attr = lO_id;
        if (!result->rhs_funcs.attr) result->identities.attr = lO_id;

        lO_id = variablize_rhs_symbol(a->value);
//        if (!result->rhs_funcs.value) result->clone_identities.value = lO_id;
        if (!result->rhs_funcs.value) result->identities.value = lO_id;

        if (preference_is_binary(result->type))
        {
            lO_id = variablize_rhs_symbol(a->referent);
            result->clone_identities.referent = lO_id;
            result->identities.referent = lO_id;
        }
    }

    dprint(DT_RHS_VARIABLIZATION, "Variablized result: %a\n", a);

    a->next = variablize_results_into_actions(result->next_result, variablize);
    return a;
}

action* Explanation_Based_Chunker::variablize_results_into_actions(preference* result, bool variablize)
{
    dprint_o_id_substitution_map(DT_RHS_VARIABLIZATION);
    action* returnAction = variablize_result_into_actions(result, variablize);
    return returnAction;
}
