/*
 * variablization_manager.cpp
 *
 *  Created on: Jul 25, 2013
 *      Author: mazzin
 */

#include "variablization_manager.h"
#include "agent.h"
#include "instantiations.h"
#include "prefmem.h"
#include "assert.h"
#include "test.h"
#include "print.h"
#include "debug.h"
#include "rhs.h"

Variablization_Manager::Variablization_Manager(agent* myAgent)
{
    thisAgent = myAgent;
    sym_to_var_map = new std::map< Symbol*, variablization* >();
    g_id_to_var_map = new std::map< uint64_t, variablization* >();
    orig_var_to_g_id_map = new std::map< Symbol*, uint64_t >();
    sti_constraints = new std::map< Symbol*, ::list* >();
    constant_constraints = new std::map< uint64_t , ::list* >();

    cond_merge_map = new std::map< Symbol*, std::map< Symbol*, std::map< Symbol*, condition*> > >();
    substitution_map = new std::map< Symbol*, test >();

    dnvl_set = new std::set< Symbol* >;

    ground_id_counter = 0;
}

Variablization_Manager::~Variablization_Manager()
{
    clear_data();
    delete sym_to_var_map;
    delete g_id_to_var_map;
    delete orig_var_to_g_id_map;
    delete sti_constraints;
    delete constant_constraints;

    delete cond_merge_map;
    delete substitution_map;
    delete dnvl_set;
}

void Variablization_Manager::reinit()
{
    dprint(DT_VARIABLIZATION_MANAGER, "Original_Variable_Manager reinitializing...\n");
    clear_data();
    ground_id_counter = 0;
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
 * ========================================================================= */
void Variablization_Manager::variablize_lhs_symbol(Symbol** sym, identity_info* identity)
{
    char prefix[2];
    Symbol* var;
    variablization* var_info;

    dprint(DT_LHS_VARIABLIZATION, "Variablizing %y(g%u)...\n",
           (*sym),
           (identity ? identity->grounding_id : 0));

    if (!((*sym)->is_sti()))
    {
        /* MToDo | Identity currently exists for all tests.  This isn't necessary until we change that */
        assert(identity);
//        if (identity->grounding_id == NON_GENERALIZABLE)
//        {
//            /* -- This symbol has been marked as non-generalizable, for
//             *    example because it is an LTI retrieved in a substate -- */
//            dprint(DT_LHS_VARIABLIZATION, "...not variablizing because test marked as non-generalizable.\n");
//            return;
//        }
        var_info = get_variablization(identity->grounding_id);
    }
    else
    {
        var_info = get_variablization(*sym);
    }
    if (var_info)
    {
        /* -- Symbol being passed in is being replaced, so decrease -- */
        /* -- and increase refcount for new variable symbol being returned -- */
        symbol_remove_ref(thisAgent, (*sym));
        *sym = var_info->variablized_symbol;
        symbol_add_ref(thisAgent, var_info->variablized_symbol);
        return;
    }

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

    store_variablization((*sym), var, identity);

    dprint(DT_LHS_VARIABLIZATION, "...created new variablization %y.\n", var);

    /* MToDoRefCnt | This remove ref was removed before, but it seems like we should have it, no? */
    symbol_remove_ref(thisAgent, *sym);
    *sym = var;
}
/* ======================================================================================================
 *
 *                                          variablize_rhs_symbol
 *
 *      The logic for variablizing the rhs is slightly different than the lhs since constants on the
 *      rhs do not yet have grounding id's.  We match variables bound to constants to lhs variables by
 *      looking up the grounding id using the original variable names instead of using the grounding id.
 *
 * ====================================================================================================== */

void Variablization_Manager::variablize_rhs_symbol(rhs_value pRhs_val, Symbol* original_var)
{
    char prefix[2];
    Symbol* var;
    variablization* found_variablization = NULL;
    uint64_t g_id;

    rhs_symbol rs = rhs_value_to_rhs_symbol(pRhs_val);

    dprint(DT_RHS_VARIABLIZATION, "variablize_rhs_symbol called for %y(%y).\n",
           rs->referent, original_var);

    /* -- identifiers and unbound vars (which are instantiated as identifiers) are indexed by their symbol
     *    instead of their original variable. --  */

    if (rs->referent->is_sti())
    {
        dprint(DT_RHS_VARIABLIZATION, "...searching for sti %y in variablization sym table...\n", rs->referent);
        found_variablization = get_variablization(rs->referent);
    }
    else
    {
        if (original_var)
        {
            dprint(DT_RHS_VARIABLIZATION, "...searching for original var %y in variablization orig var table...\n", original_var);
            g_id = get_gid_for_orig_var(original_var);
            if (g_id != NON_GENERALIZABLE)
            {
                found_variablization = get_variablization(g_id);
            }
            else
            {
                /* Normally this should not occur.  All ovars on rhs for constants should have
                 * entries in table since we scanned original variables of starting conditions
                 * of instantiation.
                 *
                 * But I think there is one exception.  If we have a preference that is added to
                 * the result because it is a rhs identifier that previously was linked to the
                 * resulting state.  The identifier will seem ungrounded because its original
                 * variable came from another production.  It should be treated like an unbound
                 * variable, so we'll fall through to code at end of function.
                 * */
                dprint(DT_RHS_VARIABLIZATION, "...%y has original_var %y that does not map to any variablized symbol.  Must be linked from top state.  Will treat as unbound variable.\n", rs->referent, original_var);
            }
        }
        else
        {
            dprint(DT_RHS_VARIABLIZATION, "...is a literal constant.  Not variablizing!\n");
            rs->g_id = NON_GENERALIZABLE;
            return;
        }
    }


    if (found_variablization)
    {
        /* --- Grounded symbol that has been variablized before--- */

        dprint(DT_RHS_VARIABLIZATION, "... found existing grounded variablization %y.\n", found_variablization->variablized_symbol);

        symbol_remove_ref(thisAgent, rs->referent);
        rs->referent = found_variablization->variablized_symbol;
        symbol_add_ref(thisAgent, found_variablization->variablized_symbol);
        rs->g_id = found_variablization->grounding_id;
        return;
    }

    /* -- Either the variablization manager has never seen this symbol or symbol is ungrounded symbol or literal constant.
     *    Both cases return 0.  Grounding id will be generated if requested by another match. -- */

    if (rs->referent->is_sti())
    {
        /* -- First instance of an unbound rhs var -- */
        dprint(DT_RHS_VARIABLIZATION, "...is unbound variable.\n");
        prefix[0] = static_cast<char>(tolower(rs->referent->id->name_letter));
        prefix[1] = 0;
        var = generate_new_variable(thisAgent, prefix);

        dprint(DT_RHS_VARIABLIZATION, "...created new variable for unbound rhs %y.\n", var);
        store_variablization(rs->referent, var, NULL);

        symbol_remove_ref(thisAgent, rs->referent);
        rs->referent = var;
    }
    else
    {
        /* -- RHS constant with an original variable that does not map onto a LHS condition.  Do not variablize. -- */
        /* MToDo | Remove.  Is this even possible?  Won't this be caught by not having an original var above? */
        dprint(DT_RHS_VARIABLIZATION, "...is a variable that did not appear in the LHS.  Not variablizing!\n");
    }
    rs->g_id = NON_GENERALIZABLE;
}

/* ============================================================================
 *            Variablization_Manager::variablize_test
 *
 * Requires: Test from positive condition.
 *           Test must not be a conjunctive test.
 * Modifies: t
 * Effect:   If referent is variablizable, replaces referent symbol with a
 *           variable by calling variablize_lhs_symbol
 *
 * ========================================================================= */
void Variablization_Manager::variablize_test(test* t, Symbol* original_referent)
{
    Symbol* instantiated_referent;

    assert(t && (*t));

    dprint(DT_LHS_VARIABLIZATION, "Variablizing test %t\n", *t);

    instantiated_referent = (*t)->data.referent;
    assert(instantiated_referent && original_referent);

    bool is_variablizable = false;
    if (instantiated_referent->symbol_type == IDENTIFIER_SYMBOL_TYPE)
    {
        if (instantiated_referent->id->smem_lti == NIL)
        {
            is_variablizable = true;
        } else {
            is_variablizable = instantiated_referent->is_variablizable_constant(original_referent) && !is_in_dnvl(instantiated_referent);
        }
    } else {
        is_variablizable = instantiated_referent->is_variablizable_constant(original_referent);
    }
    if (is_variablizable)
    {
        dprint(DT_LHS_VARIABLIZATION, "...variablizing test type %s with referent %y\n", test_type_to_string((*t)->type), instantiated_referent);
        variablize_lhs_symbol(&((*t)->data.referent), (*t)->identity);
    }
    else
    {
        dprint(DT_LHS_VARIABLIZATION, "...non-variablizable referent %y or in DNVL.  Original: %y.\n", instantiated_referent, original_referent);
    }

    dprint(DT_LHS_VARIABLIZATION, "Result: %t\n", *t);
    dprint(DT_LHS_VARIABLIZATION, "---------------------------------------\n");
}

/* ============================================================================
 *            Variablization_Manager::variablize_equality_test
 *
 * Requires: Test from positive condition.
 *           Test must not be a conjunctive test.
 * Modifies: t
 * Effect:   Variablizes all equality tests in a test.  Does not require all
 *           tests to be equality tests.  While it does not require that one
 *           test is an equality test, we have a bug somewhere else if this
 *           funtion gets passed a test that contains at least one equality
 *           test.
 *
 * ========================================================================= */
void Variablization_Manager::variablize_equality_test(test* t)
{
    test original_test, original_eq_test;
    Symbol* original_referent;

    assert(t && (*t));
    original_test = (*t)->original_test;

    dprint(DT_LHS_VARIABLIZATION, "Variablizing equality test %t\n", *t);

    if (original_test)
    {
        // Sanity check on originals
        assert(original_test->type && (original_test->type < NUM_TEST_TYPES));

        if (original_test->type == EQUALITY_TEST)
        {
            original_referent = original_test->data.referent;
        }
        else if (original_test->type == CONJUNCTIVE_TEST)
        {
            /* --  A non-conjunctive test with a conjunctive original test must be an equality tests,
                   since it is the only type that can sensically have multiple originals and be a conjunction.  -- */

            /* MToDo | It may be legal, but are multiple original equality tests really sensical?  Does it
             *         ever make sense to put that in a rule?  Can they ever become conjunctive at runtime. */

            dprint(DT_LHS_VARIABLIZATION, "...this is an eq test with conj original test: %t\n", original_test);
            assert((*t)->type == EQUALITY_TEST);
            original_eq_test = find_original_equality_test_preferring_vars(original_test, false);
            if (!original_eq_test)
            {
                return;
            }
            original_referent = original_eq_test->data.referent;
        }
        else /* -- Not an equality test -- */
        {
            return;
        }
        variablize_test(t, original_referent);
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

void Variablization_Manager::variablize_equality_tests(test* t)
{
    cons* c;
    dprint(DT_LHS_VARIABLIZATION, "Variablizing equality tests in: %t\n", *t);
    assert(*t);

    if ((*t)->type == CONJUNCTIVE_TEST)
    {
        // Original test should always be null for a conjunctive tests.
        // MToDo | Previous logic wouldn't assert false but print an error out instead.  Remove.

        assert((*t)->original_test == NULL);
        dprint(DT_LHS_VARIABLIZATION, "Iterating through conjunction list.\n");
        for (c = (*t)->data.conjunct_list; c != NIL; c = c->rest)
        {
            dprint(DT_LHS_VARIABLIZATION, "Variablizing conjunctive test: ");
            variablize_equality_test(reinterpret_cast<test*>(&(c->first)));
        }

        dprint(DT_LHS_VARIABLIZATION, "Done iterating through conjunction list.\n");
        dprint(DT_LHS_VARIABLIZATION, "---------------------------------------\n");
    }
    else
    {
        variablize_equality_test(t);
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
 *           Returns true if test should be kept in condition.  Only returns
 *           false for ungrounded STI that may have been collected during
 *           backtracing.
 *
 * ========================================================================= */
bool Variablization_Manager::variablize_test_by_lookup(test* t, bool pSkipTopLevelEqualities)
{
    variablization* found_variablization = NULL;

    if (!test_has_referent((*t)))
    {
        return true;
    }

    dprint(DT_LHS_VARIABLIZATION, "Variablizing by lookup %t\n", *t);

    if (pSkipTopLevelEqualities && ((*t)->type == EQUALITY_TEST))
    {
        /* -- Wrong test type for this variablization pass -- */
        dprint(DT_CONSTRAINTS, "Not variablizing constraint b/c equality test in second variablization pass.\n");
        return true;
    }
    found_variablization = get_variablization(*t);
    if (found_variablization)
    {
        // It has been variablized before, so just variablize
        symbol_remove_ref(thisAgent, (*t)->data.referent);
        (*t)->data.referent = found_variablization->variablized_symbol;
        symbol_add_ref(thisAgent, found_variablization->variablized_symbol);
    }
    else
    {
        if ((*t)->data.referent->is_sti())
        {
            /* -- STI identifier that is ungrounded.  Error.  -- */
            dprint(DT_DEBUG, "Ungrounded STI in in chunk.  Will delete during consolidation phase.\n");
            return false;
        }
        else
        {
            /* -- Constant referent that is ungrounded.  Ignore. -- */
            dprint(DT_CONSTRAINTS, "Not variablizing constraint b/c referent not grounded in chunk.\n");
        }
    }

    dprint(DT_LHS_VARIABLIZATION, "Result: %t\n", *t);
    dprint(DT_LHS_VARIABLIZATION, "---------------------------------------\n");
    return true;

}

void Variablization_Manager::variablize_tests_by_lookup(test* t, bool pSkipTopLevelEqualities)
{

    cons* c;
    bool isGrounded;
    dprint(DT_LHS_VARIABLIZATION, "Variablizing by lookup tests in: %t\n", *t);

    assert(*t);

    if ((*t)->type == CONJUNCTIVE_TEST)
    {
        // previous logic wouldn't assert false but print an error out instead.`
        assert((*t)->original_test == NULL);
        dprint(DT_LHS_VARIABLIZATION, "Iterating through conjunction list.\n");
        for (c = (*t)->data.conjunct_list; c != NIL; c = c->rest)
        {
            dprint(DT_LHS_VARIABLIZATION, "Variablizing conjunctive test: \n");
            /* -- Note that we ignore what variablize_test_by_lookup returns b/c merge will later delete
             *    any ungrounded tests on STI's.  Any ungrounded tests on non-STIs do not need to be
             *    deleted.  We just leave them as a literal. We only use the return value of
             *    variablize_test_by_lookup when variablizing constraints collected during
             *    backtracing, since we can just avoid adding them to the condition list. -- */
            variablize_test_by_lookup(reinterpret_cast<test*>(&(c->first)), pSkipTopLevelEqualities);
        }

        dprint(DT_LHS_VARIABLIZATION, "Done iterating through conjunction list.\n");
        dprint(DT_LHS_VARIABLIZATION, "---------------------------------------\n");
    }
    else
    {
        variablize_test_by_lookup(t, pSkipTopLevelEqualities);
    }
}

/* MToDo | Check what was meant by...  This gets passed in a copy of the chunk instantiation's condition lists, which
 * will get thrown away */

void Variablization_Manager::variablize_condition_list(condition* top_cond, bool pInNegativeCondition)
{
    dprint(DT_LHS_VARIABLIZATION, "==========================================\n");
    dprint(DT_LHS_VARIABLIZATION, "Variablizing LHS condition list:\n");
    dprint(DT_LHS_VARIABLIZATION, "==========================================\n");

    dprint(DT_LHS_VARIABLIZATION, "Pass 1: Variablizing equality tests in positive conditions...\n");

    if (!pInNegativeCondition)
    {
        for (condition* cond = top_cond; cond != NIL; cond = cond->next)
        {
            if (cond->type == POSITIVE_CONDITION)
            {
                dprint(DT_LHS_VARIABLIZATION, "----------------------------------------------------------------------\n");
                dprint(DT_LHS_VARIABLIZATION, "Variablizing LHS positive condition equality tests: %l\n", cond);
                dprint(DT_LHS_VARIABLIZATION, "----------------------------------------------------------------------\n");
                dprint(DT_LHS_VARIABLIZATION, "Variablizing identifier: ");
                variablize_equality_tests(&(cond->data.tests.id_test));
                dprint(DT_LHS_VARIABLIZATION, "Variablizing attribute: ");
                variablize_equality_tests(&(cond->data.tests.attr_test));
                dprint(DT_LHS_VARIABLIZATION, "Variablizing value: ");
                variablize_equality_tests(&(cond->data.tests.value_test));
            }
        }
    }
    dprint(DT_LHS_VARIABLIZATION, "Pass 2: Variablizing all other LHS tests via lookup only:\n");
    for (condition* cond = top_cond; cond != NIL; cond = cond->next)
    {
        if (cond->type == POSITIVE_CONDITION)
        {
            dprint(DT_LHS_VARIABLIZATION, "----------------------------------------------------------------------\n");
            dprint(DT_LHS_VARIABLIZATION, "Variablizing LHS positive non-equality tests: %l\n", cond);
            dprint(DT_LHS_VARIABLIZATION, "----------------------------------------------------------------------\n");
            dprint(DT_LHS_VARIABLIZATION, "Variablizing identifier: ");
            variablize_tests_by_lookup(&(cond->data.tests.id_test), !pInNegativeCondition);
            dprint(DT_LHS_VARIABLIZATION, "Variablizing attribute: ");
            variablize_tests_by_lookup(&(cond->data.tests.attr_test), !pInNegativeCondition);
            dprint(DT_LHS_VARIABLIZATION, "Variablizing value: ");
            variablize_tests_by_lookup(&(cond->data.tests.value_test), !pInNegativeCondition);
        }
        else if (cond->type == NEGATIVE_CONDITION)
        {
            dprint(DT_LHS_VARIABLIZATION, "----------------------------------------------------------------------\n");
            dprint(DT_LHS_VARIABLIZATION, "Variablizing LHS negative condition: %l\n", cond);
            dprint(DT_LHS_VARIABLIZATION, "----------------------------------------------------------------------\n");
            dprint(DT_LHS_VARIABLIZATION, "Variablizing identifier: ");
            variablize_tests_by_lookup(&(cond->data.tests.id_test), false);
            dprint(DT_LHS_VARIABLIZATION, "Variablizing attribute: ");
            variablize_tests_by_lookup(&(cond->data.tests.attr_test), false);
            dprint(DT_LHS_VARIABLIZATION, "Variablizing value: ");
            variablize_tests_by_lookup(&(cond->data.tests.value_test), false);
        }
        else if (cond->type == CONJUNCTIVE_NEGATION_CONDITION)
        {
            dprint(DT_NCC_VARIABLIZATION, "-------------======-----------\n");
            dprint(DT_NCC_VARIABLIZATION, "Variablizing LHS negative conjunctive condition:\n");
            dprint_noprefix(DT_NCC_VARIABLIZATION, "%1", cond->data.ncc.top);
            variablize_condition_list(cond->data.ncc.top, false);
        }
    }
    dprint(DT_LHS_VARIABLIZATION, "Done variablizing LHS condition list.\n");
    dprint(DT_LHS_VARIABLIZATION, "==========================================\n");
}


/* =====================================================================

          variablize_rl_symbol and variablize_rl_condition_list

   Variablizing of conditions is done by walking over a condition list
   and destructively modifying it, replacing tests of identifiers with
   tests of tests of variables.

   These functions are analogous to the ones in chunk.cpp but without
   all of the generalized variablization logic introduced in Soar 9.4
===================================================================== */

void Variablization_Manager::variablize_rl_test(test* t)
{
    cons* c;
    test ct;

    assert(*t);
    dprint(DT_RL_VARIABLIZATION, "%t\n", *t);

    if ((*t)->type == CONJUNCTIVE_TEST)
    {
        dprint(DT_RL_VARIABLIZATION, "Iterating through conjunction list.\n");
        ct = *t;
        for (c = ct->data.conjunct_list; c != NIL; c = c->rest)
        {
            variablize_rl_test(reinterpret_cast<test*>(&(c->first)));
        }

        dprint(DT_RL_VARIABLIZATION, "Done iterating through conjunction list.\n");
        dprint(DT_RL_VARIABLIZATION, "---------------------------------------\n");

    }
    else
    {
        if (test_has_referent((*t)) && ((*t)->data.referent->is_sti()))
        {
            dprint(DT_RL_VARIABLIZATION, "Variablizing test type %s with referent %y\n",
                   test_type_to_string((*t)->type), (*t)->data.referent);
            thisAgent->variablizationManager->variablize_lhs_symbol(&((*t)->data.referent), NULL);
        }
        else
        {
            dprint(DT_RL_VARIABLIZATION, "Not an STI or a non-variablizable test type.\n");

        }
    }

    dprint(DT_RL_VARIABLIZATION, "Resulting in  %t\n", (*t));
    dprint(DT_RL_VARIABLIZATION, "---------------------------------------\n");
}


// creates an action for a template instantiation
action* Variablization_Manager::make_variablized_rl_action(Symbol* id_sym, Symbol* attr_sym, Symbol* val_sym, Symbol* ref_sym)
{
    action* rhs;

    rhs = make_action(thisAgent);
    rhs->type = MAKE_ACTION;
    rhs->preference_type = NUMERIC_INDIFFERENT_PREFERENCE_TYPE;

    rhs->id = allocate_rhs_value_for_symbol(thisAgent, id_sym);
    rhs->attr = allocate_rhs_value_for_symbol(thisAgent, attr_sym);
    rhs->value = allocate_rhs_value_for_symbol(thisAgent, val_sym);
    rhs->referent = allocate_rhs_value_for_symbol(thisAgent, ref_sym);

    dprint(DT_RL_VARIABLIZATION, "Variablizing action: %a\n", rhs);
    variablize_rhs_symbol(rhs->id, NULL);
    variablize_rhs_symbol(rhs->attr, NULL);
    variablize_rhs_symbol(rhs->value, NULL);
    variablize_rhs_symbol(rhs->referent, NULL);
    dprint(DT_RL_VARIABLIZATION, "Created variablized action: %a\n", rhs);
    return rhs;
}

void Variablization_Manager::variablize_rl_condition_list(condition* top_cond, bool pInNegativeCondition)
{

    dprint(DT_RL_VARIABLIZATION, "=============================================\n");
    dprint(DT_RL_VARIABLIZATION, "Variablizing LHS condition list for template:\n");
    dprint(DT_RL_VARIABLIZATION, "=============================================\n");

    dprint(DT_LHS_VARIABLIZATION, "Pass 1: Variablizing equality tests in positive conditions...\n");

    if (!pInNegativeCondition)
    {
        for (condition* cond = top_cond; cond != NIL; cond = cond->next)
        {
            if (cond->type == POSITIVE_CONDITION)
            {
                dprint(DT_RL_VARIABLIZATION, "----------------------------------------------------------------------\n");
                dprint(DT_RL_VARIABLIZATION, "Variablizing LHS positive condition equality tests: %l\n", cond);
                dprint(DT_RL_VARIABLIZATION, "----------------------------------------------------------------------\n");
                dprint(DT_RL_VARIABLIZATION, "Variablizing RL identifier: ");
                variablize_rl_test(&(cond->data.tests.id_test));
                dprint(DT_RL_VARIABLIZATION, "Variablizing RL attribute: ");
                variablize_rl_test(&(cond->data.tests.attr_test));
                dprint(DT_RL_VARIABLIZATION, "Variablizing RL value: ");
                variablize_rl_test(&(cond->data.tests.value_test));
            }
        }
    }
    dprint(DT_RL_VARIABLIZATION, "Pass 2: Variablizing all other LHS tests via lookup only:\n");
    for (condition* cond = top_cond; cond != NIL; cond = cond->next)
    {
        if (cond->type == POSITIVE_CONDITION)
        {
            dprint(DT_RL_VARIABLIZATION, "----------------------------------------------------------------------\n");
            dprint(DT_RL_VARIABLIZATION, "Variablizing LHS positive non-equality tests: %l\n", cond);
            dprint(DT_RL_VARIABLIZATION, "----------------------------------------------------------------------\n");
            variablize_tests_by_lookup(&(cond->data.tests.id_test), !pInNegativeCondition);
            variablize_tests_by_lookup(&(cond->data.tests.attr_test), !pInNegativeCondition);
            variablize_tests_by_lookup(&(cond->data.tests.value_test), !pInNegativeCondition);
        }
        else if (cond->type == NEGATIVE_CONDITION)
        {
            dprint(DT_RL_VARIABLIZATION, "----------------------------------------------------------------------\n");
            dprint(DT_RL_VARIABLIZATION, "Variablizing LHS negative condition: %l\n", cond);
            dprint(DT_RL_VARIABLIZATION, "----------------------------------------------------------------------\n");
            variablize_tests_by_lookup(&(cond->data.tests.id_test), false);
            variablize_tests_by_lookup(&(cond->data.tests.attr_test), false);
            variablize_tests_by_lookup(&(cond->data.tests.value_test), false);
        }
        else if (cond->type == CONJUNCTIVE_NEGATION_CONDITION)
        {
            dprint(DT_RL_VARIABLIZATION, "-------------======-----------\n");
            dprint(DT_RL_VARIABLIZATION, "Variablizing RL LHS negative conjunctive condition:\n");
            dprint_noprefix(DT_RL_VARIABLIZATION, "%1", cond->data.ncc.top);
            variablize_rl_condition_list(cond->data.ncc.top, false);
        }
    }

    dprint(DT_RL_VARIABLIZATION, "Done variablizing LHS condition list for template.\n");
    dprint(DT_RL_VARIABLIZATION, "==================================================\n");
}

action* Variablization_Manager::variablize_results(preference* result, bool variablize)
{
    action* a;

    if (!result)
    {
        return NIL;
    }

    a = make_action(thisAgent);
    a->type = MAKE_ACTION;

    a->id = allocate_rhs_value_for_symbol(thisAgent,  result->id, result->original_symbols.id, 0);
    a->attr = allocate_rhs_value_for_symbol(thisAgent,  result->attr, result->original_symbols.attr, 0);
    a->value = allocate_rhs_value_for_symbol(thisAgent, result->value, result->original_symbols.value, 0);
    if (preference_is_binary(result->type))
    {
        a->referent = allocate_rhs_value_for_symbol(thisAgent, result->referent);
    }

    if (variablize)
    {
        dprint_set_params(DT_RHS_VARIABLIZATION, "");
        dprint(DT_RHS_VARIABLIZATION, "Variablizing preference for %p\n", result);
        dprint(DT_IDENTITY_PROP, "\nSetting g_ids for action and variablizing results...\n");

        thisAgent->variablizationManager->variablize_rhs_symbol(a->id, result->original_symbols.id);
        thisAgent->variablizationManager->variablize_rhs_symbol(a->attr, result->original_symbols.attr);
        thisAgent->variablizationManager->variablize_rhs_symbol(a->value, result->original_symbols.value);
        if (preference_is_binary(result->type))
        {
            thisAgent->variablizationManager->variablize_rhs_symbol(a->referent, NULL);
        }
        dprint(DT_RHS_VARIABLIZATION, "Variablized result: %a\n", a);
    }
    else
    {
        /* MToDo | We might need to set these g_ids properly.  For example, do
         *         we need g_ids when we have chunk-less states that are being
         *         chunked through from a chunky state. So justifications need
         *         them? -- */
    }


    a->preference_type = result->type;

    a->next = variablize_results(result->next_result, variablize);
    return a;
}

void Variablization_Manager::add_ltis_to_dnvl_for_test(test t)
{
    cons* c;
    dprint(DT_IDENTITY_PROP, "Adding LTIs for test: %t\n", t);

    if (t->type == CONJUNCTIVE_TEST)
    {
        dprint(DT_IDENTITY_PROP, "Adding LTIs for conjunctive test: ");
        for (c = t->data.conjunct_list; c != NIL; c = c->rest)
        {
            add_ltis_to_dnvl_for_test(reinterpret_cast<test>(c->first));
        }
    }
    else if (test_has_referent(t) && t->data.referent->is_lti())
    {
        dprint(DT_IDENTITY_PROP, "Adding LTI %y to DNVL.", t->data.referent);
        add_dnvl(t->data.referent);
    }
}

void Variablization_Manager::add_ltis_to_dnvl_for_conditions(condition* top_cond)
{
    dprint(DT_IDENTITY_PROP, "Adding LTIs to DNVL: ");
    for (condition* cond = top_cond; cond != NIL; cond = cond->next)
    {
        if (cond->type != CONJUNCTIVE_NEGATION_CONDITION)
        {
            dprint(DT_IDENTITY_PROP, "Adding for condition: %l\n", cond);
            add_ltis_to_dnvl_for_test(cond->data.tests.id_test);
            add_ltis_to_dnvl_for_test(cond->data.tests.attr_test);
            add_ltis_to_dnvl_for_test(cond->data.tests.value_test);
        }
        else
        {
            dprint(DT_NCC_VARIABLIZATION, "Adding for negative conjunctive condition:\n");
            dprint_noprefix(DT_NCC_VARIABLIZATION, "%1", cond->data.ncc.top);
            add_ltis_to_dnvl_for_conditions(cond->data.ncc.top);
        }
    }
    dprint_noprefix(DT_IDENTITY_PROP, "\n");
    dprint(DT_IDENTITY_PROP, "Done adding LTIs to DNVL.\n");
    dprint(DT_IDENTITY_PROP, "==========================================\n");
    print_dnvl_set(DT_IDENTITY_PROP);

}

void Variablization_Manager::add_ltis_to_dnvl_for_prefs(preference* prefs)
{
    if (!prefs)
    {
        dprint(DT_IDENTITY_PROP, "Done adding LTIs to DNVL.\n");
        dprint(DT_IDENTITY_PROP, "==========================================\n");
        print_dnvl_set(DT_IDENTITY_PROP);
        return;
    }

    dprint(DT_IDENTITY_PROP, "Adding LTIs to for preference: %p\n",
        prefs);

    if (prefs->id && (prefs->id->is_lti())) { add_dnvl(prefs->id); }
    if (prefs->attr && (prefs->attr->is_lti())) { add_dnvl(prefs->attr); }
    if (prefs->value && (prefs->value->is_lti())) { add_dnvl(prefs->value); }
    if (preference_is_binary(prefs->type))
    {
        if (prefs->referent && (prefs->referent->is_lti())) { add_dnvl(prefs->referent); }
    }
    add_ltis_to_dnvl_for_prefs(prefs->next_result);

}
