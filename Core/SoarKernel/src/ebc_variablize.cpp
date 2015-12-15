/*
 * variablization_manager.cpp
 *
 *  Created on: Jul 25, 2013
 *      Author: mazzin
 */

#include <ebc.h>
#include "agent.h"
#include "instantiations.h"
#include "prefmem.h"
#include "assert.h"
#include "test.h"
#include "print.h"
#include "debug.h"
#include "rhs.h"
#include "xml.h"

Symbol* Explanation_Based_Chunker::get_variablization_for_identity(uint64_t index_id)
{
    if (index_id == 0)
    {
        return NULL;
    }

    std::unordered_map< uint64_t, Symbol* >::iterator iter = (*o_id_to_var_map).find(index_id);
    if (iter != (*o_id_to_var_map).end())
    {
        dprint(DT_VM_MAPS, "...found o%u in non-STI variablization table: %y\n", index_id, iter->second);
        return iter->second;
    }
    else
    {
        dprint(DT_VM_MAPS, "...did not find o%u in non-STI variablization table.\n", index_id);
        dprint_variablization_tables(DT_LHS_VARIABLIZATION, 2);
        return NULL;
    }
}

Symbol* Explanation_Based_Chunker::get_variablization_for_sti(Symbol* index_sym)
{
    std::unordered_map< Symbol*, Symbol* >::iterator iter = (*sym_to_var_map).find(index_sym);
    if (iter != (*sym_to_var_map).end())
    {
        dprint(DT_VM_MAPS, "...found %y in STI variablization table: %y\n", index_sym, iter->second);
        return iter->second;
    }
    else
    {
        dprint(DT_VM_MAPS, "...did not find %y in STI variablization table.\n", index_sym);
        dprint_variablization_tables(DT_VM_MAPS, 1);
        return NULL;
    }
}

void Explanation_Based_Chunker::store_variablization(Symbol* instantiated_sym,
        Symbol* variable,
        uint64_t pIdentity)
{
    assert(instantiated_sym && variable);
    dprint(DT_LHS_VARIABLIZATION, "Storing variablization for %y(o%u) to %y.\n",
           instantiated_sym, pIdentity, variable);

    if (instantiated_sym->is_sti())
    {
        /* -- STI variablization is looked up using the matched symbol instead
         *    of the identity. -- */

        (*sym_to_var_map)[instantiated_sym] = variable;
        symbol_add_ref(thisAgent, instantiated_sym);
        symbol_add_ref(thisAgent, variable);
        dprint(DT_VM_MAPS, "Created symbol_to_var_map ([%y] and [%y] to new variablization.\n",
                        instantiated_sym, variable);
    }
    else if (pIdentity)
    {

        /* -- A constant symbol is being variablized, so store variablization info
         *    indexed by the constant's o_id. -- */
        (*o_id_to_var_map)[pIdentity] = variable;
        symbol_add_ref(thisAgent, variable);

        dprint(DT_VM_MAPS, "Created o_id_to_var_map for %u to new variablization.\n", pIdentity);
    }
    else
    {
        assert(false);
    }
    //  print_variablization_table();
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
void Explanation_Based_Chunker::variablize_lhs_symbol(Symbol** sym, uint64_t pIdentity)
{
    char prefix[2];
    Symbol* var;
    Symbol* var_info;

    dprint(DT_LHS_VARIABLIZATION, "variablize_lhs_symbol variablizing %y(o%u)...\n", (*sym), pIdentity);

    if ((*sym)->is_sti())
    {
        var_info = get_variablization_for_sti(*sym);
    }
    else
    {
        var_info = get_variablization_for_identity(pIdentity);
    }
    if (var_info)
    {
        symbol_remove_ref(thisAgent, (*sym));
        *sym = var_info;
        symbol_add_ref(thisAgent, var_info);
        dprint(DT_LHS_VARIABLIZATION, "...with found variablization info %y(%y)\n", (*sym), var_info);

        return;

    } else {

        /* --- need to create a new variable.  If constant is being variablized
         *     just used 'c' instead of first letter of id name --- */
        if ((*sym)->is_identifier())
        {
            prefix[0] = static_cast<char>(tolower((*sym)->id->name_letter));
        }
        else
        {
            prefix[0] = 'c';
        }
        prefix[1] = 0;
        var = generate_new_variable(thisAgent, prefix);

        store_variablization((*sym), var, pIdentity);

        symbol_remove_ref(thisAgent, *sym);
        *sym = var;
        dprint(DT_LHS_VARIABLIZATION, "...with newly created variablization info for new variable %y\n", (*sym));
    }
}

void Explanation_Based_Chunker::variablize_rhs_symbol(rhs_value pRhs_val)
{
    char prefix[2];
    Symbol* var;
    Symbol* found_variablization = NULL;

    if (rhs_value_is_funcall(pRhs_val))
    {
        list* fl = rhs_value_to_funcall_list(pRhs_val);
        cons* c;

        for (c = fl->rest; c != NIL; c = c->rest)
        {
            dprint(DT_RHS_VARIABLIZATION, "Variablizing RHS value %r\n", static_cast<char*>(c->first));
            variablize_rhs_symbol(static_cast<char*>(c->first));
            dprint(DT_RHS_VARIABLIZATION, "Variablized RHS value is now %r\n", static_cast<char*>(c->first));
        }
        return;
    }

    rhs_symbol rs = rhs_value_to_rhs_symbol(pRhs_val);

    dprint(DT_RHS_VARIABLIZATION, "variablize_rhs_symbol called for %y(%y o%u).\n",
           rs->referent, get_ovar_for_o_id(rs->o_id), rs->o_id);
    /* -- identifiers and unbound vars (which are instantiated as identifiers) are indexed by their symbol
     *    instead of their original variable. --  */

    if (rs->referent->is_sti())
    {
        dprint(DT_RHS_VARIABLIZATION, "...searching for sti %y in variablization sym table...\n", rs->referent);
        found_variablization = get_variablization_for_sti(rs->referent);
    }
    else
    {
        if (rs->o_id)
        {
            dprint(DT_RHS_VARIABLIZATION, "...searching for variablization for %y...\n", get_ovar_for_o_id(rs->o_id));
                found_variablization = get_variablization_for_identity(rs->o_id);
        }
        else
        {
            dprint(DT_RHS_VARIABLIZATION, "...is a literal constant.  Not variablizing!\n");
            return;
        }
    }


    if (found_variablization)
    {
        /* --- Grounded symbol that has been variablized before--- */
        dprint(DT_RHS_VARIABLIZATION, "... found existing variablization %y.\n", found_variablization);
        symbol_remove_ref(thisAgent, rs->referent);
        rs->referent = found_variablization;
        symbol_add_ref(thisAgent, found_variablization);
        return;
    } else {
        /* -- Either the variablization manager has never seen this symbol or symbol is ungrounded symbol or literal constant.
         *    Both cases return 0. -- */

        if (rs->referent->is_sti())
        {
            /* -- First instance of an unbound rhs var -- */
            dprint(DT_RHS_VARIABLIZATION, "...is unbound variable.\n");
            prefix[0] = static_cast<char>(tolower(rs->referent->id->name_letter));
            prefix[1] = 0;
            var = generate_new_variable(thisAgent, prefix);

            dprint(DT_RHS_VARIABLIZATION, "...created new variable for unbound rhs %y.\n", var);
            store_variablization(rs->referent, var, 0);

            symbol_remove_ref(thisAgent, rs->referent);
            rs->referent = var;
        }
        else
        {
            /* -- RHS constant with an original variable that does not map onto a LHS condition.  Do not variablize. -- */
            dprint(DT_RHS_VARIABLIZATION, "...matched a constant with an ungrounded variable that that did not appear on the LHS.  Not variablizing.\n");
        }
    }
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

void Explanation_Based_Chunker::variablize_equality_tests(test t)
{
    cons* c;
    test tt;
    dprint(DT_LHS_VARIABLIZATION, "Variablizing equality tests in: %t\n", t);
    assert(t);

    if (t->type == CONJUNCTIVE_TEST)
    {

        dprint(DT_LHS_VARIABLIZATION, "Iterating through conjunction list.\n");
        for (c = t->data.conjunct_list; c != NIL; c = c->rest)
        {
            tt = reinterpret_cast<test>(c->first);
            if (tt->type == EQUALITY_TEST)
            {
                dprint(DT_LHS_VARIABLIZATION, "Variablizing equality test: %t\n", tt);
                if ((tt->identity && tt->data.referent->is_variablizable()) || tt->data.referent->is_sti())
                {
                    variablize_lhs_symbol(&(tt->data.referent), tt->identity);
                }
                dprint(DT_LHS_VARIABLIZATION, "Setting conjunctive test %t's eq_test to: %t\n", t, tt);
            }
        }

        dprint(DT_LHS_VARIABLIZATION, "Done iterating through conjunction list.\n");
        dprint(DT_LHS_VARIABLIZATION, "---------------------------------------\n");
    }
    else
    {
        if ((t->type == EQUALITY_TEST) &&
            ((t->identity && t->data.referent->is_variablizable()) || t->data.referent->is_sti()))
        {
            dprint(DT_LHS_VARIABLIZATION, "Variablizing equality test: %t\n", t);
            dprint(DT_LHS_VARIABLIZATION, "Equality test %t's eq_test is: %t\n", t, t->eq_test);
            variablize_lhs_symbol(&(t->data.referent), t->identity);
            dprint(DT_LHS_VARIABLIZATION, "Equality test %t's eq_test is: %t\n", t, t->eq_test);
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
    Symbol* found_variablization = NULL;

    dprint(DT_LHS_VARIABLIZATION, "Variablizing by lookup %t\n", t);

    if (pSkipTopLevelEqualities && (t->type == EQUALITY_TEST))
    {
        /* -- Wrong test type for this variablization pass -- */
        dprint(DT_CONSTRAINTS, "Not variablizing constraint b/c equality test in second variablization pass.\n");
        return true;
    }
    if (t->data.referent->is_sti())
    {
        found_variablization =  get_variablization_for_sti(t->data.referent);
    }
    else
    {
        found_variablization =  get_variablization_for_identity(t->identity);
    }
    if (found_variablization)
    {
        // It has been variablized before, so just variablize
        symbol_remove_ref(thisAgent, t->data.referent);
        t->data.referent = found_variablization;
        symbol_add_ref(thisAgent, found_variablization);
    }
    else
    {
        dprint(DT_LHS_VARIABLIZATION, "%s", t->data.referent->is_sti() ?
            "Ungrounded STI in in chunk.  Will delete during consolidation phase.\n" :
            "Not variablizing constraint b/c referent not grounded in chunk.\n");
        return false;
    }

    dprint(DT_LHS_VARIABLIZATION, "Result: %t\n", t);
    dprint(DT_LHS_VARIABLIZATION, "---------------------------------------\n");

    return true;
}

void Explanation_Based_Chunker::variablize_tests_by_lookup(test t, bool pSkipTopLevelEqualities)
{

    cons* c;
    test tt;
//    bool isGrounded;
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
        if (test_has_referent(t) && (t->identity || t->data.referent->is_sti()))
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

void Explanation_Based_Chunker::variablize_rl_test(test t)
{
    cons* c;
    test ct;

    assert(t);
    dprint(DT_RL_VARIABLIZATION, "%t\n", t);

    if (t->type == CONJUNCTIVE_TEST)
    {
        dprint(DT_RL_VARIABLIZATION, "Iterating through conjunction list.\n");
        ct = t;
        for (c = ct->data.conjunct_list; c != NIL; c = c->rest)
        {
            variablize_rl_test(reinterpret_cast<test>(c->first));
        }

        dprint(DT_RL_VARIABLIZATION, "Done iterating through conjunction list.\n");
        dprint(DT_RL_VARIABLIZATION, "---------------------------------------\n");

    }
    else
    {
        if (test_has_referent(t) && (t->data.referent->is_sti()))
        {
            dprint(DT_RL_VARIABLIZATION, "Variablizing test type %s with referent %y\n",
                   test_type_to_string(t->type), t->data.referent);
            variablize_lhs_symbol(&(t->data.referent), t->identity);
        }
        else
        {
            dprint(DT_RL_VARIABLIZATION, "Not an STI or a non-variablizable test type.\n");

        }
    }

    dprint(DT_RL_VARIABLIZATION, "Resulting in  %t\n", t);
    dprint(DT_RL_VARIABLIZATION, "---------------------------------------\n");
}


// creates an action for a template instantiation
action* Explanation_Based_Chunker::make_variablized_rl_action(Symbol* id_sym, Symbol* attr_sym, Symbol* val_sym, Symbol* ref_sym)
{
    action* rhs;

    rhs = make_action(thisAgent);
    rhs->type = MAKE_ACTION;
    rhs->preference_type = NUMERIC_INDIFFERENT_PREFERENCE_TYPE;

    rhs->id = allocate_rhs_value_for_symbol(thisAgent, id_sym, 0);
    rhs->attr = allocate_rhs_value_for_symbol(thisAgent, attr_sym, 0);
    rhs->value = allocate_rhs_value_for_symbol(thisAgent, val_sym, 0);
    rhs->referent = allocate_rhs_value_for_symbol(thisAgent, ref_sym, 0);

    dprint(DT_RL_VARIABLIZATION, "Variablizing action: %a\n", rhs);
    variablize_rhs_symbol(rhs->id);
    variablize_rhs_symbol(rhs->attr);
    variablize_rhs_symbol(rhs->value);
    variablize_rhs_symbol(rhs->referent);
    dprint(DT_RL_VARIABLIZATION, "Created variablized action: %a\n", rhs);
    return rhs;
}

void Explanation_Based_Chunker::variablize_rl_condition_list(condition* top_cond, bool pInNegativeCondition)
{

    dprint_header(DT_RL_VARIABLIZATION, PrintBoth, "Variablizing LHS condition list for template:\n");

    dprint(DT_LHS_VARIABLIZATION, "Pass 1: Variablizing equality tests in positive conditions...\n");

    if (!pInNegativeCondition)
    {
        for (condition* cond = top_cond; cond != NIL; cond = cond->next)
        {
            if (cond->type == POSITIVE_CONDITION)
            {
                dprint_header(DT_RL_VARIABLIZATION, PrintBoth, "Variablizing LHS positive condition equality tests: %l\n", cond);
                dprint(DT_RL_VARIABLIZATION, "Variablizing RL identifier: ");
                variablize_rl_test(cond->data.tests.id_test);
                dprint(DT_RL_VARIABLIZATION, "Variablizing RL attribute: ");
                variablize_rl_test(cond->data.tests.attr_test);
                dprint(DT_RL_VARIABLIZATION, "Variablizing RL value: ");
                variablize_rl_test(cond->data.tests.value_test);
            }
        }
    }
    dprint(DT_RL_VARIABLIZATION, "Pass 2: Variablizing all other LHS tests via lookup only:\n");
    for (condition* cond = top_cond; cond != NIL; cond = cond->next)
    {
        if (cond->type == POSITIVE_CONDITION)
        {
            dprint_header(DT_RL_VARIABLIZATION, PrintBoth, "Variablizing LHS positive non-equality tests: %l\n", cond);
            variablize_tests_by_lookup(cond->data.tests.id_test, !pInNegativeCondition);
            variablize_tests_by_lookup(cond->data.tests.attr_test, !pInNegativeCondition);
            variablize_tests_by_lookup(cond->data.tests.value_test, !pInNegativeCondition);
        }
        else if (cond->type == NEGATIVE_CONDITION)
        {
            dprint_header(DT_RL_VARIABLIZATION, PrintBoth, "Variablizing LHS negative condition: %l\n", cond);
            variablize_tests_by_lookup(cond->data.tests.id_test, false);
            variablize_tests_by_lookup(cond->data.tests.attr_test, false);
            variablize_tests_by_lookup(cond->data.tests.value_test, false);
        }
        else if (cond->type == CONJUNCTIVE_NEGATION_CONDITION)
        {
            dprint_header(DT_RL_VARIABLIZATION, PrintBefore, "Variablizing RL LHS negative conjunctive condition:\n");
            dprint_noprefix(DT_RL_VARIABLIZATION, "%1", cond->data.ncc.top);
            variablize_rl_condition_list(cond->data.ncc.top, false);
        }
    }

    dprint_header(DT_RL_VARIABLIZATION, PrintAfter, "Done variablizing LHS condition list for template.\n");
}

action* Explanation_Based_Chunker::variablize_results_into_actions(preference* result, bool variablize)
{
    action* a;

    if (!result)
    {
        return NIL;
    }

    a = make_action(thisAgent);
    a->type = MAKE_ACTION;

    if (!result->rhs_funcs.id)
    {
        a->id = allocate_rhs_value_for_symbol(thisAgent, result->id, result->o_ids.id);
    } else {
        dprint(DT_RHS_VARIABLIZATION, "VM found rhs_func:  %r", result->rhs_funcs.id);
        a->id = copy_rhs_value(thisAgent, result->rhs_funcs.id);
    }
    if (!result->rhs_funcs.attr)
    {
        a->attr = allocate_rhs_value_for_symbol(thisAgent, result->attr, result->o_ids.attr);
    } else {
        dprint(DT_RHS_VARIABLIZATION, "VM found rhs_func:  %r", result->rhs_funcs.attr);
        a->attr = copy_rhs_value(thisAgent, result->rhs_funcs.attr);
    }
    if (!result->rhs_funcs.value)
    {
        a->value = allocate_rhs_value_for_symbol(thisAgent, result->value, result->o_ids.value);
    } else {
        dprint(DT_RHS_VARIABLIZATION, "VM found rhs_func:  %r", result->rhs_funcs.value);
        a->value = copy_rhs_value(thisAgent, result->rhs_funcs.value);
    }
    if (preference_is_binary(result->type))
    {
        a->referent = allocate_rhs_value_for_symbol(thisAgent, result->referent, 0);
    }

    if (variablize)
    {
        dprint_set_indents(DT_RHS_VARIABLIZATION, "");
        dprint(DT_RHS_VARIABLIZATION, "Variablizing preference for %p\n", result);
        dprint_clear_indents(DT_RHS_VARIABLIZATION);

        variablize_rhs_symbol(a->id);
        variablize_rhs_symbol(a->attr);
        variablize_rhs_symbol(a->value);
        if (preference_is_binary(result->type))
        {
            variablize_rhs_symbol(a->referent);
        }
        dprint(DT_RHS_VARIABLIZATION, "Variablized result: %a\n", a);
    }

    a->preference_type = result->type;

    a->next = variablize_results_into_actions(result->next_result, variablize);
    return a;
}
