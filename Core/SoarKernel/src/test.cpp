/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/*************************************************************************
 *
 *  file:  rhs.cpp
 *
 * =======================================================================
 *                    Test Utilities
 * This file contains various utility routines for tests.
 *
 * =======================================================================
 */
#include <assert.h>
#include "test.h"
#include "debug.h"
#include "kernel.h"
#include "symtab.h"
#include "agent.h"
#include "print.h"
#include "rete.h"
#include "instantiations.h"
#include "variablization_manager.h"
#include "output_manager.h"
#include "wmem.h"
#include "prefmem.h"

/* =================================================================

                      Utility Routines for Tests

================================================================= */

/* --- This just copies a consed list of tests and returns
 *     a new copy of it. --- */

list* copy_test_list(agent* thisAgent, cons* c, test* pEq_test, bool pUnify_variablization_identity)
{
    cons* new_c;

    if (!c)
    {
        return NIL;
    }
    allocate_cons(thisAgent, &new_c);
    new_c->first = copy_test(thisAgent, static_cast<test>(c->first), pUnify_variablization_identity);
    if (static_cast<test>(new_c->first)->type == EQUALITY_TEST)
    {
        *pEq_test = static_cast<test>(new_c->first);
    }
    new_c->rest = copy_test_list(thisAgent, c->rest, pEq_test, pUnify_variablization_identity);
    return new_c;
}

/* ----------------------------------------------------------------
   Takes a test and returns a new copy of it.
---------------------------------------------------------------- */

test copy_test(agent* thisAgent, test t, bool pUnify_variablization_identity)
{
    Symbol* referent;
    test new_ct;

    if (!t)
    {
        return NULL;
    }

    switch (t->type)
    {
        case GOAL_ID_TEST:
        case IMPASSE_ID_TEST:
            new_ct = make_test(thisAgent, NIL, t->type);
            break;
        case DISJUNCTION_TEST:
            new_ct = make_test(thisAgent, NIL, t->type);
            new_ct->data.disjunction_list = copy_symbol_list_adding_references(thisAgent, t->data.disjunction_list);
            break;
        case CONJUNCTIVE_TEST:
            new_ct = make_test(thisAgent, NIL, t->type);
            new_ct->data.conjunct_list = copy_test_list(thisAgent, t->data.conjunct_list, &(new_ct->eq_test), pUnify_variablization_identity);

            break;
        default:
            new_ct = make_test(thisAgent, t->data.referent, t->type);
            new_ct->identity = t->identity;
            if (t->type == EQUALITY_TEST)
            {
                new_ct->eq_test = new_ct;
            }
            if (pUnify_variablization_identity)
            {
                /* Mark this test as seen.  The tests in the constraint lists are copies of
                 * the pointers in grounds, so we use this tc_num later to later check if
                 * an entry in the constraint propagation list is a duplicate of a test
                 * already in a condition, which most should be. */
                if (t->type != EQUALITY_TEST)
                {
                    t->tc_num = thisAgent->variablizationManager->get_constraint_found_tc_num();
                }
                if (new_ct->identity)
                {
                    thisAgent->variablizationManager->unify_identity(new_ct);
                }
            }

            break;
    }
    return new_ct;
}

/* ----------------------------------------------------------------
   Same as copy_test(), only it doesn't include goal or impasse tests
   in the new copy.  The caller should initialize the two flags to false
   before calling this routine; it sets them to true if it finds a goal
   or impasse test.
---------------------------------------------------------------- */
test copy_test_removing_goal_impasse_tests(agent* thisAgent, test t,
        bool* removed_goal,
        bool* removed_impasse)
{
    cons* c;
    test new_t, temp;

    switch (t->type)
    {
        case EQUALITY_TEST:
            return copy_test(thisAgent, t);
            break;
        case GOAL_ID_TEST:
            *removed_goal = true;
            return NULL;
        case IMPASSE_ID_TEST:
            *removed_impasse = true;
            return NULL;
        case CONJUNCTIVE_TEST:
            new_t = NULL;
            for (c = t->data.conjunct_list; c != NIL; c = c->rest)
            {
                temp = copy_test_removing_goal_impasse_tests(thisAgent, static_cast<test>(c->first),
                        removed_goal,
                        removed_impasse);
                if (temp)
                {
                    add_test(thisAgent, &new_t, temp);
                }
            }
            if (new_t->type == CONJUNCTIVE_TEST)
            {
                new_t->data.conjunct_list =
                    destructively_reverse_list(new_t->data.conjunct_list);
            }
            return new_t;

        default:  /* relational tests other than equality */
            return copy_test(thisAgent, t);
    }
}

test copy_test_without_relationals(agent* thisAgent, test t)
{
    cons* c;
    test new_t, temp;

    switch (t->type)
    {
        case GOAL_ID_TEST:
        case IMPASSE_ID_TEST:
        case EQUALITY_TEST:
            return copy_test(thisAgent, t);
            break;
        case CONJUNCTIVE_TEST:
            new_t = NULL;
            for (c = t->data.conjunct_list; c != NIL; c = c->rest)
            {
                temp = copy_test_without_relationals(thisAgent, static_cast<test>(c->first));
                if (temp)
                {
                    add_test(thisAgent, &new_t, temp);
                }
            }
            if (new_t->type == CONJUNCTIVE_TEST)
            {
                new_t->data.conjunct_list =
                    destructively_reverse_list(new_t->data.conjunct_list);
            }
            return new_t;

        default:  /* relational tests other than equality */
            return NULL;
    }
}

/* ----------------------------------------------------------------
   Deallocates a test.
---------------------------------------------------------------- */

void deallocate_test(agent* thisAgent, test t)
{
    cons* c, *next_c;

    dprint(DT_DEALLOCATES, "DEALLOCATE test %t\n", t);
//    if (!t)
//    {
//        return;
//    }

    switch (t->type)
    {
        case GOAL_ID_TEST:
        case IMPASSE_ID_TEST:
            break;
        case DISJUNCTION_TEST:
            deallocate_symbol_list_removing_references(thisAgent, t->data.disjunction_list);
            break;
        case CONJUNCTIVE_TEST:
            dprint(DT_DEALLOCATES, "DEALLOCATE conjunctive test\n");
            c = t->data.conjunct_list;
            while (c)
            {
                next_c = c->rest;
                test tt;
                tt = static_cast<test>(c->first);
                deallocate_test(thisAgent, static_cast<test>(c->first));
                free_cons(thisAgent, c);
                c = next_c;
            }
            break;
        default: /* relational tests other than equality */
#ifdef DEBUG_TRACE_REFCOUNT_INVENTORY
            symbol_remove_ref(thisAgent, t->data.referent);
#else
            symbol_remove_ref(thisAgent, t->data.referent);
#endif
            break;
    }
    /* -- The eq_test was just a cache to prevent repeated searches on conjunctive tests
     *    during chunking.  We did not copy the test or increment the refcount, so we
     *    don't need to decrease the refcount here. -- */
    t->eq_test = NULL;

    thisAgent->memoryManager->free_with_pool(MP_test, t);
    dprint(DT_DEALLOCATES, "DEALLOCATE test done.\n");
}

/* ----------------------------------------------------------------
   Destructively modifies the first test (t) by adding the second
   one (add_me) to it (usually as a new conjunct).  The first test
   need not be a conjunctive test nor even exist.
---------------------------------------------------------------- */

void add_test(agent* thisAgent, test* dest_test_address, test new_test)
{

    test destination = 0, original = 0;
    cons* c, *c_orig;

    if (!new_test)
    {
        return;
    }

    if (!(*dest_test_address))
    {
        *dest_test_address = new_test;
        return;
    }

    destination = *dest_test_address;
    if (destination->type != CONJUNCTIVE_TEST)
    {
        destination = make_test(thisAgent, NIL, CONJUNCTIVE_TEST);
        allocate_cons(thisAgent, &c);
        destination->data.conjunct_list = c;
        destination->eq_test = (*dest_test_address)->eq_test;
        c->first = *dest_test_address;
        c->rest = NIL;
        *dest_test_address = destination;
    }
    if (!destination->eq_test)
    {
        destination->eq_test = new_test->eq_test;
    } else {
        assert(new_test->type != EQUALITY_TEST);
    }

    /* --- now add add_test to the conjunct list --- */
    allocate_cons(thisAgent, &c);
    c->first = new_test;
    c->rest = destination->data.conjunct_list;
    destination->data.conjunct_list = c;

}

/* -- This function is a special purpose function for adding relational tests to another test. It
 *    adds a test to a list but checks if there already exists an equality test for that same symbol.
 *    If one does exist but doesn't have an original test, it replaces the missing original
 *    test with the one from the new test but does not add a new equality test.  This is only used
 *    when reconstructing the original conditions and adding relational tests.
 *
 *    Note:  This was added to handle a yet unexplained situation in which the main equality test in a
 *           reconstructed test does not get an original test.  Normally, that variable is
 *           retrieved from the rete's varname data structures, but for some cases, the
 *           varname is empty, and it later adds an equality test for that variable that it
 *           finds in the extra_tests portion of the rete node, where it normally gets relational
 *           tests.  This produced two equality tests for the same symbol, one with and one without
 *           the original test, which caused problems with other aspects of chunking. -- */

void add_relational_test(agent* thisAgent, test* dest_test_address, test new_test, uint64_t pI_id, bool has_referent = true)
{
    if (has_referent)
    {
        // Handle case where relational test is equality test
        if ((*dest_test_address) && new_test && (new_test->type == EQUALITY_TEST))
        {
            test destination = *dest_test_address;
            if (destination->type == EQUALITY_TEST)
            {
                if (destination->data.referent == new_test->data.referent)
                {
                    if (!destination->identity && new_test->identity)
                    {
                        /* This is the special case */
                        destination->identity = new_test->identity;
                        dprint(DT_IDENTITY_PROP, "Copying identity to equality test for add_relational_test special case %t: %y\n",
                            destination, thisAgent->variablizationManager->get_ovar_for_o_id(destination->identity));
                        deallocate_test(thisAgent, new_test);
                        return;
                    }
                    else
                    {
                        /* Identical referents and possibly identical originals.  Ignore. */
                        return;
                    }
                } // else different referents and should be added as new test
            }
            else if (destination->type == CONJUNCTIVE_TEST)
            {
                cons* c;
                test check_test;
                for (c = destination->data.conjunct_list; c != NIL; c = c->rest)
                {
                    check_test = static_cast<test>(c->first);
                    if (check_test->type == EQUALITY_TEST)
                    {
                        if (check_test->data.referent == new_test->data.referent)
                        {
                            if (!check_test->identity && new_test->identity)
                            {
                                /* This is the special case */
                                check_test->identity = new_test->identity;
                                dprint(DT_IDENTITY_PROP, "Copying identity to equality test for add_relational_test special case %t: %s\n", check_test,
                                    thisAgent->variablizationManager->get_ovar_for_o_id(check_test->identity));
                                deallocate_test(thisAgent, new_test);
                                return;
                            }
                        }
                    }
                }
            }
        }
    }
    add_test(thisAgent, dest_test_address, new_test);
}

/* ----------------------------------------------------------------
   Same as add_test(), only has no effect if the second
   test is already included in the first one.
---------------------------------------------------------------- */

void add_test_if_not_already_there(agent* thisAgent, test* t, test add_me, bool neg)
{
    test ct;
    cons* c;

    if (tests_are_equal(*t, add_me, neg))
    {
        deallocate_test(thisAgent, add_me);
        return;
    }

    ct = *t;
    if (ct->type == CONJUNCTIVE_TEST)
        for (c = ct->data.conjunct_list; c != NIL; c = c->rest)
            if (tests_are_equal(static_cast<test>(c->first), add_me, neg))
            {
                deallocate_test(thisAgent, add_me);
                return;
            }

    add_test(thisAgent, t, add_me);
}

/* ----------------------------------------------------------------
   Returns true iff the two tests are identical.
   If neg is true, ignores order of members in conjunctive tests
   and assumes variables are all equal.
---------------------------------------------------------------- */

bool tests_are_equal(test t1, test t2, bool neg)
{
    cons* c1, *c2;

    if (t1->type == EQUALITY_TEST)
    {
        if (t2->type != EQUALITY_TEST)
        {
            return false;
        }

        if (t1->data.referent == t2->data.referent)
        {
            return true;
        }

        if (!neg)
        {
            return false;
        }

        // ignore variables in negation tests
        Symbol* s1 = t1->data.referent;
        Symbol* s2 = t2->data.referent;

        if ((s1->is_variable()) && (s2->is_variable()))
        {
            return true;
        }
        return false;
    }

    if (t1->type != t2->type)
    {
        return false;
    }

    switch (t1->type)
    {
        case GOAL_ID_TEST:
            return true;

        case IMPASSE_ID_TEST:
            return true;

        case DISJUNCTION_TEST:
            for (c1 = t1->data.disjunction_list, c2 = t2->data.disjunction_list; (c1 != NIL) && (c2 != NIL); c1 = c1->rest, c2 = c2->rest)
            {
                if (c1->first != c2->first)
                {
                    return false;
                }
            }
            if (c1 == c2)
            {
                return true;    /* make sure they both hit end-of-list */
            }
            return false;

        case CONJUNCTIVE_TEST:
            // bug 510 fix: ignore order of test members in conjunctions
        {
            std::list<test> copy2;
            for (c2 = t2->data.conjunct_list; c2 != NIL; c2 = c2->rest)
            {
                copy2.push_back(static_cast<test>(c2->first));
            }

            std::list<test>::iterator iter;
            for (c1 = t1->data.conjunct_list; c1 != NIL; c1 = c1->rest)
            {
                // check against copy
                for (iter = copy2.begin(); iter != copy2.end(); ++iter)
                {
                    if (tests_are_equal(static_cast<test>(c1->first), *iter, neg))
                    {
                        break;
                    }
                }

                // iter will be end if no match
                if (iter == copy2.end())
                {
                    return false;
                }

                // there was a match, remove it from unmatched
                copy2.erase(iter);
            }

            // make sure no unmatched remain
            if (copy2.empty())
            {
                return true;
            }
        }
        return false;

        default:  /* relational tests other than equality */
            if (t1->data.referent == t2->data.referent)
            {
                return true;
            }
            return false;
    }
}

/* ----------------------------------------------------------------
 * tests_identical
 *
 * Requires: Two non-conjunctive, non-blank tests
 * Modifies: Nothing
 * Effects:  Returns true iff both tests point to the same symbol or symbols
 *           or have the same type for tests without referents
 * Notes:    Unlike tests_are_equal, this function doesn't do anything
 *       special for negations or variables.
  ---------------------------------------------------------------- */

bool tests_identical(test t1, test t2, bool considerIdentity)
{
    cons* c1, *c2;
    test test1, test2;

    if (t1->type != t2->type)
    {
        return false;
    }

    switch (t1->type)
    {
        case GOAL_ID_TEST:
        case IMPASSE_ID_TEST:
            return true;
        case DISJUNCTION_TEST:
        {
            for (c1 = t1->data.disjunction_list, c2 = t2->data.disjunction_list; (c1 != NIL) && (c2 != NIL); c1 = c1->rest, c2 = c2->rest)
                if (c1->first != c2->first)
                {
                    return false;
                }
            if (c1 == c2)
            {
                return true;    /* make sure they both hit end-of-list */
            }
            return false;
        }
        case CONJUNCTIVE_TEST:
            assert(false);
            return false;
        default:  /* relational tests */
        {
            if (t1->data.referent != t2->data.referent)
            {
                return false;
            }
            if (considerIdentity)
            {
                if (t1->data.referent->is_sti())
                {
                    if (!t2->data.referent->is_sti())
                    {
                        /* -- An identifier and something else -- */
                        return false;
                    }
                    else
                    {
                        /* -- Two identifiers -- */
                        return true;
                    }
                }
                else
                {
                    return (t1->identity == t2->identity);
                }
            }
            return true;
        }
    }
}

/* ----------------------------------------------------------------
   Returns a hash value for the given test.
---------------------------------------------------------------- */

uint32_t hash_test(agent* thisAgent, test t)
{
    cons* c;
    uint32_t result;

    if (!t)
    {
        return 0;
    }

    switch (t->type)
    {
        case EQUALITY_TEST:
            return t->data.referent->hash_id;
        case GOAL_ID_TEST:
            return 34894895;  /* just use some unusual number */
        case IMPASSE_ID_TEST:
            return 2089521;
        case DISJUNCTION_TEST:
            result = 7245;
            for (c = t->data.disjunction_list; c != NIL; c = c->rest)
            {
                result = result + static_cast<Symbol*>(c->first)->hash_id;
            }
            return result;
        case CONJUNCTIVE_TEST:
            result = 100276;
            for (c = t->data.conjunct_list; c != NIL; c = c->rest)
            {
                result = result + hash_test(thisAgent, static_cast<test>(c->first));
            }
            // bug 510: conjunctive tests' order needs to be ignored
            //for (c=ct->data.disjunction_list; c!=NIL; c=c->rest)
            //  result = result + hash_test (thisAgent, static_cast<constraint>(c->first));
            return result;
        case NOT_EQUAL_TEST:
        case LESS_TEST:
        case GREATER_TEST:
        case LESS_OR_EQUAL_TEST:
        case GREATER_OR_EQUAL_TEST:
        case SAME_TYPE_TEST:
            return (t->type << 24) + t->data.referent->hash_id;
        default:
        {
            char msg[BUFFER_MSG_SIZE];
            strncpy(msg, "production.c: Error: bad test type in hash_test\n", BUFFER_MSG_SIZE);
            msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
            abort_with_fatal_error(thisAgent, msg);
            break;
        }
    }
    return 0; /* unreachable, but without it, gcc -Wall warns here */
}

/* ----------------------------------------------------------------
   Looks for goal or impasse tests (as directed by the two flag
   parameters) in the given test, and returns true if one is found.
---------------------------------------------------------------- */

bool test_includes_goal_or_impasse_id_test(test t,
        bool look_for_goal,
        bool look_for_impasse)
{
    cons* c;

    if (t->type == EQUALITY_TEST)
    {
        return false;
    }
    if (look_for_goal && (t->type == GOAL_ID_TEST))
    {
        return true;
    }
    if (look_for_impasse && (t->type == IMPASSE_ID_TEST))
    {
        return true;
    }
    if (t->type == CONJUNCTIVE_TEST)
    {
        for (c = t->data.conjunct_list; c != NIL; c = c->rest)
            if (test_includes_goal_or_impasse_id_test(static_cast<test>(c->first),
                    look_for_goal,
                    look_for_impasse))
            {
                return true;
            }
        return false;
    }
    return false;
}

/* ----------------------------------------------------------------
   Looks through a test, and returns a new copy of the first equality
   test it finds.  Signals an error if there is no equality test in
   the given test.
---------------------------------------------------------------- */

test copy_of_equality_test_found_in_test(agent* thisAgent, test t)
{
    cons* c;
    char msg[BUFFER_MSG_SIZE];

    if (!t)
    {
        strncpy(msg, "Internal error: can't find equality constraint in constraint\n", BUFFER_MSG_SIZE);
        msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
        abort_with_fatal_error(thisAgent, msg);
    }
    if (t->type == EQUALITY_TEST)
    {
        return copy_test(thisAgent, t);
    }
    if (t->type == CONJUNCTIVE_TEST)
    {
        for (c = t->data.conjunct_list; c != NIL; c = c->rest)
            if (static_cast<test>(c->first) &&
                    (static_cast<test>(c->first)->type == EQUALITY_TEST))
            {
                return copy_test(thisAgent, static_cast<test>(c->first));
            }
    }
    strncpy(msg, "Internal error: can't find equality constraint in constraint\n", BUFFER_MSG_SIZE);
    abort_with_fatal_error(thisAgent, msg);
    return 0; /* unreachable, but without it, gcc -Wall warns here */
}

test equality_test_found_in_test(test t)
{
    cons* c;

    assert(t);
    if (t->type == EQUALITY_TEST)
    {
        return t;
    }
    if (t->type == CONJUNCTIVE_TEST)
    {
        for (c = t->data.conjunct_list; c != NIL; c = c->rest)
            if (static_cast<test>(c->first)->type == EQUALITY_TEST)
            {
                return (static_cast<test>(c->first));
            }
    }

    return NULL;
}

/* =====================================================================

   Finding all variables from tests, conditions, and condition lists

   These routines collect all the variables in tests, etc.  Their
   "var_list" arguments should either be NIL or else should point to
   the header of the list of marked variables being constructed.
===================================================================== */

void add_all_variables_in_test(agent* thisAgent, test t,
                               tc_number tc, list** var_list)
{
    cons* c;
    Symbol* referent;

    if (!t)
    {
        return;
    }

    switch (t->type)
    {
        case GOAL_ID_TEST:
        case IMPASSE_ID_TEST:
        case DISJUNCTION_TEST:
            break;
        case CONJUNCTIVE_TEST:
            for (c = t->data.conjunct_list; c != NIL; c = c->rest)
            {
                add_all_variables_in_test(thisAgent, static_cast<test>(c->first), tc, var_list);
            }
            break;

        default:
            referent = t->data.referent;
            if (referent->symbol_type == VARIABLE_SYMBOL_TYPE)
            {
                referent->mark_if_unmarked(thisAgent, tc, var_list);
            }
            break;
    }
}

void add_bound_variables_in_test(agent* thisAgent, test t,
                                 tc_number tc, list** var_list)
{
    cons* c;
    Symbol* referent;

    if (!t)
    {
        return;
    }

    if (t->type == EQUALITY_TEST)
    {
        referent = t->data.referent;
        if (referent->symbol_type == VARIABLE_SYMBOL_TYPE)
        {
            referent->mark_if_unmarked(thisAgent, tc, var_list);
        }
        return;
    }
    else if (t->type == CONJUNCTIVE_TEST)
    {
        for (c = t->data.conjunct_list; c != NIL; c = c->rest)
        {
            add_bound_variables_in_test(thisAgent, static_cast<test>(c->first), tc, var_list);
        }
    }
}

/* -----------------------------------------------------------------
   Find first letter of test, or '*' if nothing appropriate.
   (See comments on first_letter_from_symbol for more explanation.)
----------------------------------------------------------------- */

char first_letter_from_test(test t)
{
    cons* c;
    char ch;

    if (!t)
    {
        return '*';
    }

    switch (t->type)
    {
        case EQUALITY_TEST:
            return first_letter_from_symbol(t->data.referent);
        case GOAL_ID_TEST:
            return 's';
        case IMPASSE_ID_TEST:
            return 'i';
        case CONJUNCTIVE_TEST:
            for (c = t->data.conjunct_list; c != NIL; c = c->rest)
            {
                ch = first_letter_from_test(static_cast<test>(c->first));
                if (ch != '*')
                {
                    return ch;
                }
            }
            return '*';
        default:  /* disjunction tests, and relational tests other than equality */
            return '*';
    }
}

/* ----------------------------------------------------------------------
                      Add Gensymmed Equality Test

   This routine destructively modifies a given test, adding to it a test
   for equality with a new gensym variable.
---------------------------------------------------------------------- */

void add_gensymmed_equality_test(agent* thisAgent, test* t, char first_letter)
{
    Symbol* New;
    test eq_test = 0;
    char prefix[2];

    prefix[0] = first_letter;
    prefix[1] = 0;
    New = generate_new_variable(thisAgent, prefix);
    eq_test = make_test(thisAgent, New, EQUALITY_TEST);
    symbol_remove_ref (thisAgent, New);
    add_test(thisAgent, t, eq_test);
}

/* ----------------------------------------------------------------------
                      Add Rete Test List to Tests

   Given the additional Rete tests (besides the hashed equality test) at
   a certain node, we need to convert them into the equivalent tests in
   the conditions being reconstructed.  This procedure does this -- it
   destructively modifies the given currently-being-reconstructed-cond
   by adding any necessary extra tests to its three field tests.
---------------------------------------------------------------------- */

void add_rete_test_list_to_tests(agent* thisAgent,
                                 condition* cond, /* current cond */
                                 rete_test* rt)
{
    Symbol* referent;
    test New = 0;
    TestType test_type;

    // Initialize table
    for (; rt != NIL; rt = rt->next)
    {

        if (rt->type == ID_IS_GOAL_RETE_TEST)
        {
            New = make_test(thisAgent, NIL, GOAL_ID_TEST);
        }
        else if (rt->type == ID_IS_IMPASSE_RETE_TEST)
        {
            New = make_test(thisAgent, NIL, IMPASSE_ID_TEST);
        }
        else if (rt->type == DISJUNCTION_RETE_TEST)
        {
            New = make_test(thisAgent, NIL, DISJUNCTION_TEST);
            New->data.disjunction_list = copy_symbol_list_adding_references(thisAgent, rt->data.disjunction_list);
        }
        else if (test_is_constant_relational_test(rt->type))
        {
            test_type = relational_test_type_to_test_type(kind_of_relational_test(rt->type));
            referent = rt->data.constant_referent;
            New = make_test(thisAgent, referent, test_type);
        }
        else if (test_is_variable_relational_test(rt->type))
        {
            test_type = relational_test_type_to_test_type(kind_of_relational_test(rt->type));
            if (! rt->data.variable_referent.levels_up)
            {
                /* --- before calling var_bound_in_reconstructed_conds, make sure
                   there's an equality test in the referent location (add one if
                   there isn't one already there), otherwise there'd be no variable
                   there to test against --- */
                if (rt->data.variable_referent.field_num == 0)
                {
                    if (!cond->data.tests.id_test->eq_test)
                    {
                        add_gensymmed_equality_test(thisAgent, &(cond->data.tests.id_test), 's');
                    }
                }
                else if (rt->data.variable_referent.field_num == 1)
                {
                    if (!cond->data.tests.attr_test->eq_test)
                    {
                        add_gensymmed_equality_test(thisAgent, &(cond->data.tests.attr_test), 'a');
                    }
                }
                else
                {
                    if (!cond->data.tests.value_test->eq_test)
                    {
                        add_gensymmed_equality_test(thisAgent, &(cond->data.tests.value_test), first_letter_from_test(cond->data.tests.attr_test));
                    }
                }
            }
            referent = var_bound_in_reconstructed_conds(thisAgent, cond,
                       rt->data.variable_referent.field_num,
                       rt->data.variable_referent.levels_up);
            New = make_test(thisAgent, referent, test_type);
        }
        else
        {
            char msg[BUFFER_MSG_SIZE];
            strncpy(msg, "Error: bad test_type in add_rete_test_to_test\n", BUFFER_MSG_SIZE);
            msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
            abort_with_fatal_error(thisAgent, msg);
            New = NIL; /* unreachable, but without it gcc -Wall warns here */
        }

        if (rt->right_field_num == 0)
        {
            add_test(thisAgent, &(cond->data.tests.id_test), New);
        }
        else if (rt->right_field_num == 2)
        {
            add_test(thisAgent, &(cond->data.tests.value_test), New);
        }
        else
        {
            add_test(thisAgent, &(cond->data.tests.attr_test), New);
        }
    }
}

/* ----------------------------------------------------------------------
                      Add Hash Info to ID Test

   This routine adds an equality test to the id field test in a given
   condition, destructively modifying that id test.  The equality test
   is the one appropriate for the given hash location (field_num/levels_up).
---------------------------------------------------------------------- */

void add_hash_info_to_id_test(agent* thisAgent,
                              condition* cond,
                              byte field_num,
                              rete_node_level levels_up)
{
    Symbol* temp;
    test New = 0;

    temp = var_bound_in_reconstructed_conds(thisAgent, cond, field_num, levels_up);
    New = make_test(thisAgent, temp, EQUALITY_TEST);
    add_test(thisAgent, &(cond->data.tests.id_test), New);
}

/* ----------------------------------------------------------------------
                      Add Hash Info to ID Test

   This routine adds an equality test to the id field test in a given
   condition, destructively modifying that id test.  The equality test
   is the one appropriate for the given hash location (field_num/levels_up).
---------------------------------------------------------------------- */

void add_hash_info_to_original_id_test(agent* thisAgent,
                                       condition* cond,
                                       byte field_num,
                                       rete_node_level levels_up)
{
    Symbol* temp;
    test t = 0, New = 0;

    t = var_test_bound_in_reconstructed_conds(thisAgent, cond, field_num, levels_up);
    cond->data.tests.id_test->identity = t->identity;
    dprint(DT_ADD_ADDITIONALS, "add_hash_info_to_original_id_test added o_id o%u(%y) from %t.\n",
        cond->data.tests.id_test->identity,
        thisAgent->variablizationManager->get_ovar_for_o_id(cond->data.tests.id_test->identity),
        t);
}

/* ----------------------------------------------------------------------
                 add_additional_tests_and_originals

   This function gets passed the instantiated conditions for a production
   being fired.  It adds all the original tests in the given Rete test list
   (from the "other tests" at a Rete node), and adds them to the equality
   test in the instantiation. These tests will then also be variablized later.

   - MMA 2013

---------------------------------------------------------------------- */
void add_inequalities(agent* thisAgent,
    rete_node* node,
    condition* cond,
    wme* w,
    node_varnames* nvn,
    uint64_t pI_id,
    AddAdditionalTestsMode additional_tests)
{

    Symbol* referent = NULL;
    test chunk_test = NULL;
    TestType test_type;
    rete_test* rt = node->b.posneg.other_tests;

    /* --- Store original referent information.  Note that sometimes the
     *     original referent equality will be stored in the beta nodes extra tests
     *     data structure rather than the alpha memory --- */
    dprint(DT_ADD_ADDITIONALS, "-=-=-=-=-=-\n");
    dprint(DT_ADD_ADDITIONALS, "add_inequalities called for rl instantiation %s.\n",
           thisAgent->newly_created_instantiations->prod->name->sc->name);
    dprint(DT_ADD_ADDITIONALS, "%l\n", cond);

    /* -- Now process any additional relational test -- */
    dprint(DT_ADD_ADDITIONALS, "Processing additional tests to add to condition %l...\n", cond);
    for (; rt != NIL; rt = rt->next)
    {
        dprint(DT_ADD_ADDITIONALS, "Processing additional test...\n");
        chunk_test = NULL;
        if (test_is_variable_relational_test(rt->type))
            {
                test_type = relational_test_type_to_test_type(kind_of_relational_test(rt->type));
                /* We may need to add equality tests if we start variablizing STIs like
                 * constants, because we'd need the identity from the equality test processed
                 * as a relational here because of weird rete case. Then again, we would need to
                 * generate identities in this function if we had to do that.  Right now, identities
                 * are not needed for RL because we only use them to variablize constants and RL
                 * only variablizes STis.*/
//                if ((test_type == EQUALITY_TEST) || (test_type == NOT_EQUAL_TEST))
                    if (test_type == NOT_EQUAL_TEST)
                {
                    dprint(DT_ADD_ADDITIONALS, "Creating variable relational rl test.\n");

                    test ref_test = var_test_bound_in_reconstructed_conds(thisAgent, cond,
                        rt->data.variable_referent.field_num,
                        rt->data.variable_referent.levels_up);
                    referent = ref_test->data.referent;
                    if(referent->is_identifier())
                    {
                        chunk_test = make_test(thisAgent, referent, test_type);
                        dprint(DT_RL_VARIABLIZATION, "Creating valid relational test for template %t [%g].\n", chunk_test, chunk_test);
                    }
                    else
                    {
                        dprint(DT_RL_VARIABLIZATION, "Relational test referent is not an STI.  Ignoring.\n");
                    }
                }
                else
                {
                    dprint(DT_RL_VARIABLIZATION, "Relational test type is not a valid template relational test.  Ignoring.\n");
                }
            }
            if (chunk_test)
            {
                if (rt->right_field_num == 0)
                {
                    add_relational_test(thisAgent, &(cond->data.tests.id_test), chunk_test, pI_id);

                    dprint(DT_ADD_ADDITIONALS, "Added relational test to id element resulting in: %t [%g]\n", cond->data.tests.id_test, cond->data.tests.id_test);
                }
                else if (rt->right_field_num == 1)
                {
                    add_relational_test(thisAgent, &(cond->data.tests.attr_test), chunk_test, pI_id);

                    dprint(DT_ADD_ADDITIONALS, "Added relational test to attribute element resulting in: %t [%g]\n", cond->data.tests.attr_test, cond->data.tests.attr_test);
                }
                else
                {
                    add_relational_test(thisAgent, &(cond->data.tests.value_test), chunk_test, pI_id);

                    dprint(DT_ADD_ADDITIONALS, "Added relational test to value element resulting in: %t [%g]\n", cond->data.tests.value_test, cond->data.tests.value_test);
                }
            }

    }

    dprint(DT_ADD_ADDITIONALS, "add_additional_tests_and_originals finished for %s.\n",
           thisAgent->newly_created_instantiations->prod->name->sc->name);
    dprint(DT_ADD_ADDITIONALS, "Final test after add_additional_tests and creating identity: %l\n", cond);
}

void add_constraints_and_identities(agent* thisAgent,
                                        rete_node* node,
                                        condition* cond,
                                        wme* w,
                                        node_varnames* nvn,
                                        uint64_t pI_id,
                                        AddAdditionalTestsMode additional_tests)
{
    rete_test* rt = node->b.posneg.other_tests;

    if (additional_tests == JUST_INEQUALITIES)
    {
        add_inequalities(thisAgent, node, cond, w, nvn, pI_id, additional_tests);
        return;
    }
    /* --- Store original referent information.  Note that sometimes the
     *     original referent equality will be stored in the beta nodes extra tests
     *     data structure rather than the alpha memory --- */
    alpha_mem* am;
    am = node->b.posneg.alpha_mem_;

    dprint(DT_ADD_ADDITIONALS, "-=-=-=-=-=-\n");
    dprint(DT_ADD_ADDITIONALS, "add_constraints_and_identities called for %s.\n",
           thisAgent->newly_created_instantiations->prod->name->sc->name);
    dprint(DT_ADD_ADDITIONALS, "%l\n", cond);
    dprint(DT_ADD_ADDITIONALS, "AM: (%y ^%y %y)\n", am->id , am->attr, am->value);

    if (nvn)
    {
        dprint(DT_ADD_ADDITIONALS, "adding var names node to original tests:\n");
        dprint_varnames_node(DT_ADD_ADDITIONALS, nvn);
        add_varname_identity_to_test(thisAgent, nvn->data.fields.id_varnames, cond->data.tests.id_test, pI_id);
        add_varname_identity_to_test(thisAgent, nvn->data.fields.attr_varnames, cond->data.tests.attr_test, pI_id);
        add_varname_identity_to_test(thisAgent, nvn->data.fields.value_varnames, cond->data.tests.value_test, pI_id);
        dprint(DT_ADD_ADDITIONALS, "Done adding var names to original tests resulting in: %l\n", cond);
    }

    /* --- on hashed nodes, add equality test for the hash function --- */
    if ((node->node_type == MP_BNODE) || (node->node_type == NEGATIVE_BNODE))
    {
        dprint(DT_ADD_ADDITIONALS, "adding unique hash info to original id test for MP_BNODE or NEGATIVE_BNODE...\n");
        add_hash_info_to_original_id_test(thisAgent, cond,
            node->left_hash_loc_field_num,
            node->left_hash_loc_levels_up);
        dprint(DT_ADD_ADDITIONALS, "...resulting in: %t [%g]\n", cond->data.tests.id_test, cond->data.tests.id_test);

    }
    else if (node->node_type == POSITIVE_BNODE)
    {
        dprint(DT_ADD_ADDITIONALS, "adding unique hash info to original id test for POSITIVE_BNODE...\n");
        add_hash_info_to_original_id_test(thisAgent, cond,
            node->parent->left_hash_loc_field_num,
            node->parent->left_hash_loc_levels_up);
        dprint(DT_ADD_ADDITIONALS, "...resulting in: %t [%g]\n", cond->data.tests.id_test, cond->data.tests.id_test);

    }

    /* -- Now process any additional relational test -- */
    dprint(DT_ADD_ADDITIONALS, "Processing additional tests to add to condition %l...\n", cond);

    test chunk_test = NULL;
    TestType test_type;
    bool has_referent;
    for (; rt != NIL; rt = rt->next)
    {
        chunk_test = NULL;
        has_referent = true;
        if (rt->type ==DISJUNCTION_RETE_TEST)
        {
            dprint(DT_ADD_ADDITIONALS, "Creating disjunction test.\n");
            chunk_test = make_test(thisAgent, NIL, DISJUNCTION_TEST);
            chunk_test->data.disjunction_list = copy_symbol_list_adding_references(thisAgent, rt->data.disjunction_list);
            has_referent = false;
        } else {
            if (test_is_constant_relational_test(rt->type))
            {
                dprint(DT_ADD_ADDITIONALS, "Creating constant relational test.\n");
                test_type = relational_test_type_to_test_type(kind_of_relational_test(rt->type));
                chunk_test = make_test(thisAgent, rt->data.constant_referent, test_type);
            }
            else if (test_is_variable_relational_test(rt->type))
            {
                test_type = relational_test_type_to_test_type(kind_of_relational_test(rt->type));
                dprint(DT_ADD_ADDITIONALS, "Creating variable relational test.\n");

                test ref_test = var_test_bound_in_reconstructed_conds(thisAgent, cond,
                    rt->data.variable_referent.field_num,
                    rt->data.variable_referent.levels_up);
                chunk_test = make_test(thisAgent, ref_test->data.referent, test_type);
                chunk_test->identity = ref_test->identity;
                dprint(DT_ADD_ADDITIONALS, "Created relational test for chunk: %t [%g].\n", chunk_test, chunk_test);
            }
        }
        if (chunk_test)
        {
            if (rt->right_field_num == 0)
            {
                add_relational_test(thisAgent, &(cond->data.tests.id_test), chunk_test, pI_id, has_referent);
                dprint(DT_ADD_ADDITIONALS, "Added relational test to id element resulting in: %t [%g]\n", cond->data.tests.id_test, cond->data.tests.id_test);
            }
            else if (rt->right_field_num == 1)
            {
                add_relational_test(thisAgent, &(cond->data.tests.attr_test), chunk_test, pI_id, has_referent);
                dprint(DT_ADD_ADDITIONALS, "Added relational test to attribute element resulting in: %t [%g]\n", cond->data.tests.attr_test, cond->data.tests.attr_test);
            }
            else
            {
                add_relational_test(thisAgent, &(cond->data.tests.value_test), chunk_test, pI_id, has_referent);
                dprint(DT_ADD_ADDITIONALS, "Added relational test to value element resulting in: %t [%g]\n", cond->data.tests.value_test, cond->data.tests.value_test);
            }
        }
    }

    dprint(DT_ADD_ADDITIONALS, "Final test after add_constraints_and_identities: %l\n", cond);
}

/* UITODO| Make these methods of test */

/* ----------------------------------------------------------------
   Returns true iff the test contains a test for a variable
   symbol.  Assumes test is not a conjunctive one and does not
   try to search them.
---------------------------------------------------------------- */
bool test_is_variable(agent* thisAgent, test t)
{
    if (!t || !test_has_referent(t))
    {
        return false;
    }
    return (t->data.referent->is_variable());
}

/* UITODO| Make this method of Test. */
const char* test_type_to_string(byte test_type)
{
    switch (test_type)
    {
        case NOT_EQUAL_TEST:
            return "NOT_EQUAL_TEST";
            break;
        case LESS_TEST:
            return "LESS_TEST";
            break;
        case GREATER_TEST:
            return "GREATER_TEST";
            break;
        case LESS_OR_EQUAL_TEST:
            return "LESS_OR_EQUAL_TEST";
            break;
        case GREATER_OR_EQUAL_TEST:
            return "GREATER_OR_EQUAL_TEST";
            break;
        case SAME_TYPE_TEST:
            return "SAME_TYPE_TEST";
            break;
        case DISJUNCTION_TEST:
            return "DISJUNCTION_TEST";
            break;
        case CONJUNCTIVE_TEST:
            return "CONJUNCTIVE_TEST";
            break;
        case GOAL_ID_TEST:
            return "GOAL_ID_TEST";
            break;
        case IMPASSE_ID_TEST:
            return "IMPASSE_ID_TEST";
            break;
        case EQUALITY_TEST:
            return "EQUALITY_TEST";
            break;
    }
    return "UNDEFINED TEST TYPE";
}

inline bool is_test_type_with_no_referent(TestType test_type)
{
    return ((test_type == DISJUNCTION_TEST) ||
            (test_type == CONJUNCTIVE_TEST) ||
            (test_type == GOAL_ID_TEST) ||
            (test_type == IMPASSE_ID_TEST));
}

test make_test(agent* thisAgent, Symbol* sym, TestType test_type)
{
    test new_ct;

    thisAgent->memoryManager->allocate_with_pool(MP_test, &new_ct);

    new_ct->type = test_type;
    new_ct->data.referent = sym;
    new_ct->identity = 0;
    if (test_type == EQUALITY_TEST)
    {
        new_ct->eq_test = new_ct;
    } else {
        new_ct->eq_test = NULL;
    }

    if (sym)
    {
        symbol_add_ref(thisAgent, sym);
    }

    return new_ct;
}

/* -- delete_test_from_conjunct
 *
 * Requires: A valid conjunctive test t (i.e. has at least two tests in it)
 *           a cons item pDeleteItem that is a constituent test of t
 * Modifies: t
 * Effects:  Deallocates the cons pDeleteItem and the test within it
 *           If only one test remains after deletion, it will deallocate
 *           conjunctive test t and replace with the remaining test.
 *
 *           Returns the next item in the conjunct list.  Null if it
 *           was the last one.
 */
::list* delete_test_from_conjunct(agent* thisAgent, test* t, ::list* pDeleteItem)
{
    ::list* prev, *next;
    next = pDeleteItem->rest;

    /* -- Fix links in conjunct list -- */
    if ((*t)->data.conjunct_list == pDeleteItem)
    {
        // Change head of conjunct list to point to rest
        (*t)->data.conjunct_list = pDeleteItem->rest;
    }
    else
    {
        // Iterate from head of list to find the previous item and fix its link
        prev = (*t)->data.conjunct_list;
        while (prev->rest != pDeleteItem)
        {
            prev = prev->rest;
        }
        prev->rest = pDeleteItem->rest;
    }

    // Delete the item
    deallocate_test(thisAgent, static_cast<test>(pDeleteItem->first));
    free_cons(thisAgent, pDeleteItem);

    /* If there were no more tests to process (next == null) and there is only
     * one remaining test left in cons list, then change from a conjunctive
     * test to a single test */
    if (!next && ((*t)->data.conjunct_list->rest == NULL))
    {
        test old_conjunct = (*t);
        (*t) = static_cast<test>((*t)->data.conjunct_list->first);
        free_cons(thisAgent, old_conjunct->data.conjunct_list);
        old_conjunct->data.conjunct_list = NULL;
        deallocate_test(thisAgent, old_conjunct);
        /* -- There are no remaining tests in conjunct list, so return NULL --*/
        return NULL;
    } else {
        (*t)->eq_test = equality_test_found_in_test(*t);
    }

    return next;
}

/* -- copy_non_identical_test
 *
 * Requires:  add_me is a non-conjunctive list.
 * Modifies:  t
 * Effect:    This function iterates through the target's tests and compares
 *            the non-conjunctive test to it.  If it never finds a match, it
 *            adds the test to the target's test
 */
void copy_non_identical_test(agent* thisAgent, test* t, test add_me, bool considerIdentity = false)
{
    test target_test;
    cons* c;

    target_test = *t;
    if (add_me->type == EQUALITY_TEST)
    {
        dprint(DT_MERGE, "          ...test is an equality test.  Skipping: %t\n", add_me);
    }
    else
    {
        if (target_test->type != CONJUNCTIVE_TEST)
        {
            if (tests_identical(target_test, add_me))
            {
                dprint(DT_MERGE, "          ...test already exists.  Skipping: %t\n", add_me);
                return;
            }
        }
        else
        {
            for (c = target_test->data.conjunct_list; c != NIL; c = c->rest)
                if (tests_identical(static_cast<test>(c->first), add_me))
                {
                    dprint(DT_MERGE, "          ...test already exists.  Skipping: %t\n", add_me);
                    return;
                }
        }
        dprint(DT_MERGE, "          ...found test to copy: %t\n", add_me);
        add_test(thisAgent, t, copy_test(thisAgent, add_me));
    }
}

/* -- copy_non_identical_tests
 *
 * Requires:  two lists
 * Modifies:  t
 * Effect:    This function copies any tests from add_me that aren't already in t
 *
 *    Note: Unlike add_test_if_not_already_there, this
 *          function does not deallocate the original test and also
 *          considers two constant tests that have different identities
 *          as non-identical.
 */
void copy_non_identical_tests(agent* thisAgent, test* t, test add_me, bool considerIdentity)
{
    cons* c;

    if (add_me->type != CONJUNCTIVE_TEST)
    {
        copy_non_identical_test(thisAgent, t, add_me);
    }
    else
    {
        for (c = add_me->data.conjunct_list; c != NIL; c = c->rest)
        {
            copy_non_identical_test(thisAgent, t, static_cast<test>(c->first));
        }
    }
}


