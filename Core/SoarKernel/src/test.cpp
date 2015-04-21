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

void fill_identity_for_eq_tests(agent* thisAgent, test t, wme* w, WME_Field default_field, uint64_t pI_id);

/* =================================================================

                      Utility Routines for Tests

================================================================= */

void unify_variablization_identity(agent* thisAgent, test t)
{
    uint64_t found_o_id = 0;

    found_o_id = thisAgent->variablizationManager->get_o_id_substitution(t->identity->original_var_id);
    if (found_o_id)
    {
        t->identity->original_var_id = found_o_id;
        symbol_remove_ref(thisAgent, t->identity->original_var);
        /* MToDo | This was originally a debug table to make o_ids more intelligible, so probably should find
         *         a better way to set ovar here. */
        t->identity->original_var = thisAgent->variablizationManager->get_ovar_for_o_id(t->identity->original_var_id);
        symbol_add_ref(thisAgent, t->identity->original_var);
    }
}

/* --- This just copies a consed list of tests and returns
 *     a new copy of it. --- */

list* copy_test_list(agent* thisAgent, cons* c, bool pUnify_variablization_identity, uint64_t pI_id)
{
    cons* new_c;

    if (!c)
    {
        return NIL;
    }
    allocate_cons(thisAgent, &new_c);
    new_c->first = copy_test(thisAgent, static_cast<test>(c->first), pUnify_variablization_identity, pI_id);
    new_c->rest = copy_test_list(thisAgent, c->rest, pUnify_variablization_identity, pI_id);
    return new_c;
}

/* ----------------------------------------------------------------
   Takes a test and returns a new copy of it.
---------------------------------------------------------------- */

test copy_test(agent* thisAgent, test t, bool pUnify_variablization_identity, uint64_t pI_id)
{
    Symbol* referent;
    test new_ct;

    if (test_is_blank(t))
    {
        return make_blank_test();
    }

    switch (t->type)
    {
        case GOAL_ID_TEST:
        case IMPASSE_ID_TEST:
            new_ct = make_test(thisAgent, NIL, t->type);
            break;
        case DISJUNCTION_TEST:
            new_ct = make_test(thisAgent, NIL, t->type);
            new_ct->identity->grounding_id = t->identity->grounding_id;
            new_ct->identity->grounding_field = t->identity->grounding_field;
            new_ct->identity->grounding_wme = t->identity->grounding_wme;
            new_ct->data.disjunction_list = copy_symbol_list_adding_references(thisAgent, t->data.disjunction_list);
            break;
        case CONJUNCTIVE_TEST:
            new_ct = make_test(thisAgent, NIL, t->type);
            new_ct->identity->grounding_id = t->identity->grounding_id;
            new_ct->identity->grounding_field = t->identity->grounding_field;
            new_ct->identity->grounding_wme = t->identity->grounding_wme;
            new_ct->data.conjunct_list = copy_test_list(thisAgent, t->data.conjunct_list, pUnify_variablization_identity, pI_id);
            break;
        default:
            new_ct = make_test(thisAgent, t->data.referent, t->type);
            new_ct->identity->grounding_id = t->identity->grounding_id;
            new_ct->identity->grounding_field = t->identity->grounding_field;
            new_ct->identity->grounding_wme = t->identity->grounding_wme;
            new_ct->identity->original_var = t->identity->original_var;
            new_ct->identity->original_var_id = t->identity->original_var_id;
            if (new_ct->identity->original_var)
            {
                symbol_add_ref(thisAgent, new_ct->identity->original_var);
            }
            if (pUnify_variablization_identity)
            {
                if (new_ct->identity->original_var_id)
                {
                    unify_variablization_identity(thisAgent, new_ct);
                    /* At this point, we can also generate new o_ids for the chunk.  They currently have o_ids that came from the
                     * conditions of the rules backtraced through and any unifications that occurred.  pI_id should only be
                     * 0 in the case of reinforcement rules being created.  (I think they're different b/c rl is creating
                     * rules that do not currently match unlike chunks/justifications) */
                    if (new_ct->identity->original_var_id && pI_id)
                    {
                        dprint(DT_FIX_CONDITIONS, "Creating new o_ids and o_vars for chunk using o%u(%y, g%u) for i%u.\n", new_ct->identity->original_var_id, new_ct->identity->original_var, new_ct->identity->grounding_id, pI_id);
                        //                        old_o_id = new_ct->identity->original_var_id;
                        thisAgent->variablizationManager->update_o_id_for_new_instantiation(&(new_ct->identity->original_var), &(new_ct->identity->original_var_id), &(new_ct->identity->grounding_id), pI_id);
                        dprint(DT_FIX_CONDITIONS, "Test after ovar update is now %t [%g].\n", new_ct, new_ct);
                        thisAgent->variablizationManager->print_o_id_to_gid_map(DT_FIX_CONDITIONS);
                        assert(new_ct->identity->original_var_id != t->identity->original_var_id);
                    }
                }
            }

            break;
    }
    if (t->original_test)
    {
        /* -- MToDo | Probably no need to unify original test.  Check. -- */
        new_ct->original_test = copy_test(thisAgent, t->original_test, pUnify_variablization_identity, pI_id);
    }
    /* Cached eq_test is used by the chunker to avoid repeatedly searching
     * through conjunctions for the main equality test.  Value set during
     * chunking, but we had it here at some point for debugging test
     * and in case we need it to be general in the future. */
//    cache_eq_test(new_ct);
    if (new_ct->identity->grounding_wme)
    {
//        wme_add_ref(new_ct->identity->grounding_wme);
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
            return make_blank_test();
        case IMPASSE_ID_TEST:
            *removed_impasse = true;
            return make_blank_test();
        case CONJUNCTIVE_TEST:
            new_t = make_blank_test();
            for (c = t->data.conjunct_list; c != NIL; c = c->rest)
            {
                temp = copy_test_removing_goal_impasse_tests(thisAgent, static_cast<test>(c->first),
                        removed_goal,
                        removed_impasse);
                if (! test_is_blank(temp))
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
            new_t = make_blank_test();
            for (c = t->data.conjunct_list; c != NIL; c = c->rest)
            {
                temp = copy_test_without_relationals(thisAgent, static_cast<test>(c->first));
                if (! test_is_blank(temp))
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
            return make_blank_test();
    }
}

/* ----------------------------------------------------------------
   Deallocates a test.
---------------------------------------------------------------- */

void deallocate_test(agent* thisAgent, test t)
{
    cons* c, *next_c;

    dprint(DT_DEALLOCATES, "DEALLOCATE test %t\n", t);
    if (test_is_blank(t))
    {
        return;
    }

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
    if (t->original_test)
    {
        dprint(DT_DEALLOCATES, "DEALLOCATE original test %t\n", t->original_test);
        deallocate_test(thisAgent, t->original_test);
    }
    /* -- MToDo | All tests should have identity for now, so we shouldn't need to check this.  Leaving in for now to see
     *            if other unit tests fail.  -- */
    if (t->identity)
    {
        if (t->identity->original_var)
        {
            symbol_remove_ref(thisAgent, t->identity->original_var);
        }
        if (t->identity->grounding_wme)
        {
//            wme_remove_ref(thisAgent, t->identity->grounding_wme);
        }
        delete t->identity;
    }
    /* -- The eq_test was just a cache to prevent repeated searches on conjunctive tests
     *    during chunking.  We did not copy the test or increment the refcount, so we
     *    don't need to decrease the refcount here. -- */
    t->eq_test = NULL;

    free_with_pool(&thisAgent->test_pool, t);
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


//  dprint(DT_ADD_TEST_TO_TEST, "add_test()<-- %s\n", get_stacktrace().c_str());
    dprint(DT_ADD_TEST_TO_TEST, "          %t\n", *dest_test_address);
    dprint(DT_ADD_TEST_TO_TEST, "          %t\n", new_test);

    if (test_is_blank(new_test))
    {
        dprint(DT_ADD_TEST_TO_TEST, "= add test is blank.  Doing nothing.\n");
        return;
    }

    if (test_is_blank(*dest_test_address))
    {
        *dest_test_address = new_test;
        dprint(DT_ADD_TEST_TO_TEST, "        = %t\n", *dest_test_address);
        return;
    }

    destination = *dest_test_address;
    if (destination->type != CONJUNCTIVE_TEST)
    {
        destination = make_test(thisAgent, NIL, CONJUNCTIVE_TEST);
        allocate_cons(thisAgent, &c);
        destination->data.conjunct_list = c;
        c->first = *dest_test_address;
        c->rest = NIL;
        /* -- Conjunctive tests do not have original tests.  Each individual test has its own original -- */
        destination->original_test = NIL;
        *dest_test_address = destination;
    }
    /* --- now add add_test to the conjunct list --- */
    allocate_cons(thisAgent, &c);
    c->first = new_test;
    c->rest = destination->data.conjunct_list;
    destination->data.conjunct_list = c;

    dprint(DT_ADD_TEST_TO_TEST, "        = %t\n", *dest_test_address);
}

/* -- This function is a special purpose function for adding relational tests to another test. It
 *    adds a test to a list but checks if there already exists an equality test for that same symbol.
 *    If one does exist but doesn't have an original test, it replaces the missing original
 *    test with the one from the new test but does not add a new equality test.  This is only used
 *    when reconstructing the original conditions and adding relational tests.
 *
 *    Note:  This was added to handle a yet unexplained rare bug where the main equality test in a
 *           reconstructed test does not get an original test.  Normally, that variable is
 *           retrieved from the rete's varname data structures, but for some cases, the
 *           varname is empty, and it later adds an equality test for that variable that it
 *           finds in the extra_tests portion of the rete node.  This effected two equality
 *           tests for the same symbol, one with and one without the original test,
 *           which caused problems with other aspects of chunking. -- */

void add_relational_test(agent* thisAgent, test* dest_test_address, test new_test, uint64_t pI_id)
{
    // Handle case where relational test is equality test
    if ((*dest_test_address) && new_test && (new_test->type == EQUALITY_TEST))
    {
        test destination = *dest_test_address;
        if (destination->type == EQUALITY_TEST)
        {
            if (destination->data.referent == new_test->data.referent)
            {
                if (!destination->original_test && new_test->original_test)
                {
                    /* This is the special case */
                    destination->original_test = new_test->original_test;
                    if (destination->identity->original_var)
                    {
                        symbol_remove_ref(thisAgent, destination->identity->original_var);
                        // Don't think it's possible
                        assert(false);
                    }
                    if (new_test->original_test->data.referent->is_variable())
                    {
                        destination->identity->original_var =  new_test->original_test->data.referent;
                        if (pI_id)
                        {
                            destination->identity->original_var_id = thisAgent->variablizationManager->get_or_create_o_id(new_test->original_test->data.referent, pI_id);
                        }
                        symbol_add_ref(thisAgent, destination->identity->original_var);
                    }
                    /* MToDo | Should this be deallocated? */
                    new_test->original_test = NIL;
                    dprint(DT_IDENTITY_PROP, "Making original var string for add_relational_test %t: %y\n",
                        destination, destination->identity->original_var);
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
                        if (!check_test->original_test && new_test->original_test)
                        {
                            /* This is the special case */
                            check_test->original_test = new_test->original_test;
                            if (check_test->identity->original_var)
                            {
                                symbol_remove_ref(thisAgent, check_test->identity->original_var);
                                // Don't think it's possible
                                assert(false);
                            }
                            if (new_test->original_test->data.referent->is_variable())
                            {
                                check_test->identity->original_var =  new_test->original_test->data.referent;
                                if (pI_id)
                                {
                                    check_test->identity->original_var_id = thisAgent->variablizationManager->get_or_create_o_id(new_test->original_test->data.referent, pI_id);
                                }
                                symbol_add_ref(thisAgent, check_test->identity->original_var);
                            }
                            new_test->original_test = NIL;
                            dprint(DT_IDENTITY_PROP, "Making original var string for add_relational_test %t: %s\n", check_test, check_test->identity->original_var);
                            deallocate_test(thisAgent, new_test);
                            return;
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
                    if (t1->identity)
                    {
                        if (t2->identity)
                        {
                            /* -- Two grounded constants -- */
                            return (t1->identity->grounding_id == t2->identity->grounding_id);
                        }
                        else
                        {
                            /* -- A literal constant and a grounded one-- */
                            return false;
                        }
                    }
                    else
                    {
                        if (t2->identity)
                        {
                            /* -- A literal constant and a grounded one-- */
                            return false;
                        }
                        else
                        {
                            /* -- Two literal constants -- */
                            return true;
                        }
                    }
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

    if (test_is_blank(t))
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
   Returns true iff the test contains an equality test for the given
   symbol.  If sym==NIL, returns true iff the test contains any
   equality test.
---------------------------------------------------------------- */

bool test_includes_equality_test_for_symbol(test t, Symbol* sym)
{
    cons* c;

    if (test_is_blank(t))
    {
        return false;
    }

    if (t->type == EQUALITY_TEST)
    {
        if (sym)
        {
            return (t->data.referent == sym);
        }
        return true;
    }
    else if (t->type == CONJUNCTIVE_TEST)
    {
        for (c = t->data.conjunct_list; c != NIL; c = c->rest)
            if (test_includes_equality_test_for_symbol(static_cast<test>(c->first), sym))
            {
                return true;
            }
    }
    return false;
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

    if (test_is_blank(t))
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
            if ((!test_is_blank(static_cast<test>(c->first))) &&
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

test equality_var_test_found_in_test(test t)
{
    cons* c;

    assert(t);
    if ((t->type == EQUALITY_TEST) && (t->data.referent->is_variable()))
    {
        return t;
    }
    if (t->type == CONJUNCTIVE_TEST)
    {
        for (c = t->data.conjunct_list; c != NIL; c = c->rest)
            if ((static_cast<test>(c->first)->type == EQUALITY_TEST) && (static_cast<test>(c->first)->data.referent->is_variable()))
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

    if (test_is_blank(t))
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

    if (test_is_blank(t))
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

    if (test_is_blank(t))
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
    //symbol_remove_ref (thisAgent, New);
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
                    if (!test_includes_equality_test_for_symbol(cond->data.tests.id_test, NIL))
                    {
                        add_gensymmed_equality_test(thisAgent, &(cond->data.tests.id_test), 's');
                    }
                }
                else if (rt->data.variable_referent.field_num == 1)
                {
                    if (!test_includes_equality_test_for_symbol(cond->data.tests.attr_test, NIL))
                    {
                        add_gensymmed_equality_test(thisAgent, &(cond->data.tests.attr_test), 'a');
                    }
                }
                else
                {
                    if (!test_includes_equality_test_for_symbol(cond->data.tests.value_test, NIL))
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
    test New = 0;

    temp = (var_bound_in_reconstructed_original_conds(thisAgent, cond, field_num, levels_up))->data.referent;
    dprint(DT_ADD_CONSTRAINTS_ORIG_TESTS, "add_hash_info_to_original_id_test %s.\n", temp->var->name);
    New = make_test(thisAgent, temp, EQUALITY_TEST);
    add_test(thisAgent, &(cond->data.tests.id_test->original_test), New);
}



/* --------------------------------------------------------------------------
                 Get grounding IDs for a WME
 --------------------------------------------------------------------------*/
inline uint64_t get_gid_from_pref_for_field(preference* p, WME_Field f)
{
    switch (f)
    {
        case ID_ELEMENT:
            return p->g_ids.id;
            break;
        case ATTR_ELEMENT:
            return p->g_ids.attr;
            break;
        case VALUE_ELEMENT:
            return p->g_ids.value;
            break;
        default:
            assert(false);
            break;
    }
    return 0;
}

inline uint64_t get_ground_id(agent* thisAgent, wme* w, WME_Field f, goal_stack_level pLevel)
{
    if (!w || (pLevel == TOP_GOAL_LEVEL))
    {
        return NON_GENERALIZABLE;
    }

    dprint(DT_IDENTITY_PROP, "- g_id requested for %s of %w at level %d...\n", field_to_string(f), w, pLevel);

    grounding_info* g = w->ground_id_list;
//    if (!g)
//    {
//        dprint(DT_IDENTITY_PROP, "- no grounding struct at level %d.\n", pLevel);
//    }
    /* -- See if we already have ground IDs for this goal level -- */
    bool create_grounding_info = true;
    for (; g; g = g->next)
    {
        if (g->level == pLevel)
        {
//            dprint(DT_IDENTITY_PROP, "- found grounding struct for level %i: (%u ^%u %u)\n", pLevel, g->grounding_id[ID_ELEMENT], g->grounding_id[ATTR_ELEMENT], g->grounding_id[VALUE_ELEMENT]);
            if (g->grounding_id[f] == 0)
            {
//                dprint(DT_IDENTITY_PROP, "-- will attempt to retrieve via propagation or create a new g_id for %s element.\n", field_to_string(f));
                create_grounding_info = false;
                break;
            }
            else
            {
                dprint(DT_IDENTITY_PROP, "-- returning g_id %u\n", g->grounding_id[f]);
                return g->grounding_id[f];
            }
        }
    }

    /* -- Create new grounding info with unique IDs for this goal level and
     *    add to head of ground_id_list -- */
    if (create_grounding_info)
    {
        g = new grounding_info(pLevel, w->ground_id_list);
        w->ground_id_list = g;
    }
    /* -- When a grounding ID is requested for a WME at the same level as the match level,
     *    we first check if there is a propagated value from the instantiation that created
     *    that wme.  If so, we use that value. -- */
    if (w->preference && (w->id->id->level == pLevel))
    {
        /* MToDo | We can probably eliminate generating id's for level 1 to level 1 matches here */
        g->grounding_id[f] = get_gid_from_pref_for_field(w->preference, f);
        dprint(DT_IDENTITY_PROP, "- g_id is %u in wme preference at the same level...", g->grounding_id[f]);
    }
    else
    {
        if (!w->preference)
        {
            dprint(DT_IDENTITY_PROP, "- not propagating.  No preference found for wme...");
        }
        else
        {
//            dprint(DT_IDENTITY_PROP, "- not propagating.  WME at higher level %d...", w->id->id->level);
        }
    }
    if (g->grounding_id[f] == 0)
    {
        g->grounding_id[f] = thisAgent->variablizationManager->get_new_ground_id();
        dprint_noprefix(DT_IDENTITY_PROP, "generating new g_id ");
    }
    else
    {
        dprint_noprefix(DT_IDENTITY_PROP, "returning existing g_id ");
    }
    dprint_noprefix(DT_IDENTITY_PROP, "%u\n", g->grounding_id[f]);
    return g->grounding_id[f];
}

inline wme* get_wme_for_referent(condition* cond, rete_node_level where_levels_up)
{
    while (where_levels_up)
    {
        where_levels_up--;
        cond = cond->prev;
    }
    return (cond->bt.wme_);
}

inline void add_identity_to_test(agent* thisAgent,
                                 test* t,
                                 WME_Field default_f,
                                 goal_stack_level level)
{
    cons* c;

    assert(t);
    assert((*t));

    switch ((*t)->type)
    {
        case DISJUNCTION_TEST:
        case GOAL_ID_TEST:
        case IMPASSE_ID_TEST:
            break;

        case CONJUNCTIVE_TEST:
            for (c = (*t)->data.conjunct_list; c != NIL; c = c->rest)
            {
                test ct = static_cast<test>(c->first);
                add_identity_to_test(thisAgent, &ct, default_f, level);
            }
            break;

        default:
            if ((*t)->identity->grounding_field == NO_ELEMENT)
            {
                (*t)->identity->grounding_field = default_f;
            }

            /* -- Set the grounding id for all variablizable constants, i.e. non short-term identifiers -- */
            Symbol* sym = get_wme_element((*t)->identity->grounding_wme, (*t)->identity->grounding_field);

            /* -- Do not generate identity for identifier symbols.  This is important in other parts of the
             *    chunking code, since it is used to determine whether a constant or identifier was variablized -- */
            if (sym)
            {
                if (!sym->is_sti())
                {
                    (*t)->identity->grounding_id = get_ground_id(thisAgent, (*t)->identity->grounding_wme, (*t)->identity->grounding_field, level);
                    dprint(DT_IDENTITY_PROP, "- Setting g_id for %y to %i.\n", sym, (*t)->identity->grounding_id);
                    if ((*t)->identity->original_var_id)
                    {
                        if ((*t)->identity->grounding_id != NON_GENERALIZABLE)
                        {
                            dprint(DT_OVAR_MAPPINGS, "Adding original variable mappings entry: o%u(%y) to g%u.\n", (*t)->identity->original_var_id, (*t)->identity->original_var, (*t)->identity->grounding_id);
                            thisAgent->variablizationManager->add_o_id_to_gid_mapping((*t)->identity->original_var_id, (*t)->identity->grounding_id);
                        }
                        else
                        {
//                            dprint(DT_IDENTITY_PROP, "- Not adding ovar to g_id mapping for %y. Marked ungeneralizable (g0).\n", sym);
                        }
                    }
                    else
                    {
//                        dprint(DT_IDENTITY_PROP, "- Not adding ovar to g_id mapping for literal %t. No original variable.\n", (*t));
                    }
                }
                else
                {
//                    dprint(DT_IDENTITY_PROP, "- Skipping %y.  No g_id necessary for STI.\n", sym);
                }
            }
            else
            {
                dprint(DT_IDENTITY_PROP, "- Skipping.  No %s sym retrieved from wme in add_identity_to_test!\n", field_to_string((*t)->identity->grounding_field));
            }
            break;
    }
}

inline void add_identity_to_negative_test(agent* thisAgent,
        test t,
        WME_Field default_f)
{
    assert(t);
    cons* c;


    switch (t->type)
    {
        case DISJUNCTION_TEST:
        case GOAL_ID_TEST:
        case IMPASSE_ID_TEST:
            dprint(DT_IDENTITY_PROP, "Will not propagate g_id for NC b/c test type does not take a referent.\n");
            break;

        case CONJUNCTIVE_TEST:
            dprint(DT_IDENTITY_PROP, "Propagating g_ids to NCC...\n");
            for (c = t->data.conjunct_list; c != NIL; c = c->rest)
            {
                add_identity_to_negative_test(thisAgent, static_cast<test>(c->first), default_f);
            }
            break;

        default:
            if (t->identity->grounding_field == NO_ELEMENT)
            {
                t->identity->grounding_field = default_f;
            }

            /* -- Set the grounding id for all variablizable constants, i.e. non short-term identifiers -- */
            Symbol* sym = t->data.referent;
            Symbol* orig_sym = t->identity->original_var;

            /* -- Do not generate identity for identifier symbols.  This is important in other parts of the
             *    chunking code, since it is used to determine whether a constant or identifier was variablized -- */
            if (sym && orig_sym)
            {
                if (!sym->is_sti() && !sym->is_variable())
                {
                    t->identity->grounding_id = thisAgent->variablizationManager->get_gid_for_o_id(t->identity->original_var_id);
                    dprint(DT_IDENTITY_PROP, "Setting g_id for %y to %i.\n", sym, t->identity->grounding_id);
                }
                else
                {
                    dprint(DT_IDENTITY_PROP, "Could not propagate g_id for NC b/c symbol %y is STI or variable.\n", sym);
                }
            }
            else
            {
                dprint(DT_IDENTITY_PROP, "Will not propagate g_id for NC b/c no referent in add_identity_to_negative_test (or one with no original variable)!\n");
            }
            break;
    }
}

void propagate_identity(agent* thisAgent,
                        condition* cond,
                        goal_stack_level level,
                        bool use_negation_lookup)
{
    condition* c;
    bool has_negative_conds = false;

    dprint_set_indents(DT_IDENTITY_PROP, "          ");
    dprint(DT_IDENTITY_PROP, "Pre-propagation conditions: \n%1", cond);
    dprint_clear_indents(DT_IDENTITY_PROP);
    for (c = cond; c; c = c->next)
    {
        if (c->type == POSITIVE_CONDITION)
        {
            dprint(DT_IDENTITY_PROP, "Propagating identity for positive condition: %l\n", c);

            if (use_negation_lookup)
            {
                /* -- Positive conditions within an NCC.  This was recursive call. -- */
                add_identity_to_negative_test(thisAgent, c->data.tests.id_test, ID_ELEMENT);
                add_identity_to_negative_test(thisAgent, c->data.tests.attr_test, ATTR_ELEMENT);
                add_identity_to_negative_test(thisAgent, c->data.tests.value_test, VALUE_ELEMENT);
            }
            else
            {
                /* -- Either a top-level positive condition or a -- */
                /* -- The last parameter determines whether to cache g_ids for NCCs.  We
                 *    only need to do this when negative conditions exist (has_negative_conds == true)
                 *    and this isn't a recursive call on an NCC list (use_negation_lookup = true) -- */
                add_identity_to_test(thisAgent, &(c->data.tests.id_test), ID_ELEMENT, level);
                add_identity_to_test(thisAgent, &(c->data.tests.attr_test), ATTR_ELEMENT, level);
                add_identity_to_test(thisAgent, &(c->data.tests.value_test), VALUE_ELEMENT, level);
            }
            dprint_set_indents(DT_IDENTITY_PROP, "          ");
            dprint(DT_IDENTITY_PROP, "Condition is now: %l\n", c);
            dprint_clear_indents(DT_IDENTITY_PROP);
        }
        else
        {
            has_negative_conds = true;
        }
    }

    if (has_negative_conds)
    {
        for (c = cond; c; c = c->next)
        {

            if (c->type == CONJUNCTIVE_NEGATION_CONDITION)
            {
                dprint(DT_IDENTITY_PROP, "Propagating identity for NCC.  Calling propagate_identity recursively.\n%c\n", c);

                propagate_identity(thisAgent, c->data.ncc.top, level, true);
            }
            else if (c->type == NEGATIVE_CONDITION)
            {
                dprint(DT_IDENTITY_PROP, "Propagating identity for negative condition: %l", c);
                add_identity_to_negative_test(thisAgent, c->data.tests.id_test, ID_ELEMENT);
                add_identity_to_negative_test(thisAgent, c->data.tests.attr_test, ATTR_ELEMENT);
                add_identity_to_negative_test(thisAgent, c->data.tests.value_test, VALUE_ELEMENT);
            } else {
                continue;
            }

            dprint_set_indents(DT_IDENTITY_PROP, "          ");
            dprint(DT_IDENTITY_PROP, "Condition is now: %l\n", c);
            dprint_clear_indents(DT_IDENTITY_PROP);

        }
    }
    dprint_set_indents(DT_IDENTITY_PROP, "          ");
    dprint(DT_IDENTITY_PROP, "Post-propagation conditions: \n%1", cond);
    dprint_clear_indents(DT_IDENTITY_PROP);
    thisAgent->variablizationManager->print_tables(DT_IDENTITY_PROP);
}



byte get_original_symbol_type(test t)
{
    if (t && t->original_test && t->original_test->data.referent)
    {
        return t->original_test->data.referent->symbol_type;
    }
    return UNDEFINED_SYMBOL_TYPE;
}


/* ----------------------------------------------------------------------
                 add_additional_tests_and_originals

   This function gets passed the instantiated conditions for a production
   being fired.  It adds all the original tests in the given Rete test list
   (from the "other tests" at a Rete node), and adds them to the equality
   test in the instantiation. These tests will then also be variablized later.

   - MMA 2013

---------------------------------------------------------------------- */
void add_additional_tests_and_originals(agent* thisAgent,
                                        rete_node* node,
                                        condition* cond,
                                        wme* w,
                                        node_varnames* nvn,
                                        uint64_t pI_id,
                                        AddAdditionalTestsMode additional_tests)
{
    Symbol* referent = NULL, *original_referent = NULL;
    test chunk_test = NULL;
    TestType test_type;
    soar_module::symbol_triple* orig = NULL;
    wme* relational_wme = NULL;
    rete_test* rt = node->b.posneg.other_tests;

    /* --- Store original referent information.  Note that sometimes the
     *     original referent equality will be stored in the beta nodes extra tests
     *     data structure rather than the alpha memory --- */
    alpha_mem* am;
    am = node->b.posneg.alpha_mem_;

    dprint(DT_ADD_CONSTRAINTS_ORIG_TESTS, "-=-=-=-=-=-\n");
    dprint(DT_ADD_CONSTRAINTS_ORIG_TESTS, "add_additional_tests_and_originals called for %s (mode = %s).\n",
           thisAgent->newly_created_instantiations->prod->name->sc->name,
           ((additional_tests == ALL_ORIGINALS) ? "ALL" : ((additional_tests == JUST_INEQUALITIES) ? "JUST INEQUALITIES" : "NONE")));
    dprint(DT_ADD_CONSTRAINTS_ORIG_TESTS, "%l\n", cond);
    dprint(DT_ADD_CONSTRAINTS_ORIG_TESTS, "AM: (%y ^%y %y)\n", am->id , am->attr, am->value);

    if (additional_tests == ALL_ORIGINALS)
    {

        if (nvn)
        {
            dprint(DT_ADD_CONSTRAINTS_ORIG_TESTS, "adding var names node to original tests:\n");
            dprint_varnames_node(DT_ADD_CONSTRAINTS_ORIG_TESTS, nvn);

            add_varnames_to_test(thisAgent, nvn->data.fields.id_varnames,
                                 &(cond->data.tests.id_test->original_test));
            add_varnames_to_test(thisAgent, nvn->data.fields.attr_varnames,
                                 &(cond->data.tests.attr_test->original_test));
            add_varnames_to_test(thisAgent, nvn->data.fields.value_varnames,
                                 &(cond->data.tests.value_test->original_test));

            dprint(DT_ADD_CONSTRAINTS_ORIG_TESTS, "Done adding var names to original tests resulting in: %l\n", cond);

        }

        /* --- on hashed nodes, add equality test for the hash function --- */
        if ((node->node_type == MP_BNODE) || (node->node_type == NEGATIVE_BNODE))
        {
            dprint(DT_ADD_CONSTRAINTS_ORIG_TESTS, "adding unique hash info to original id test for MP_BNODE or NEGATIVE_BNODE...\n");
            add_hash_info_to_original_id_test(thisAgent, cond,
                                              node->left_hash_loc_field_num,
                                              node->left_hash_loc_levels_up);

            dprint(DT_ADD_CONSTRAINTS_ORIG_TESTS, "...resulting in: %t\n", cond->data.tests.id_test);

        }
        else if (node->node_type == POSITIVE_BNODE)
        {
            dprint(DT_ADD_CONSTRAINTS_ORIG_TESTS, "adding unique hash info to original id test for POSITIVE_BNODE...\n");
            add_hash_info_to_original_id_test(thisAgent, cond,
                                              node->parent->left_hash_loc_field_num,
                                              node->parent->left_hash_loc_levels_up);

            dprint(DT_ADD_CONSTRAINTS_ORIG_TESTS, "...resulting in: %t\n", cond->data.tests.id_test);

        }
    } // endif (additional_tests == ALL_ORIGINALS)

    /* -- Now process any additional relational test -- */
    dprint(DT_ADD_CONSTRAINTS_ORIG_TESTS, "Processing additional tests...\n");
    for (; rt != NIL; rt = rt->next)
    {
        chunk_test = NULL;
        switch (rt->type)
        {
            case ID_IS_GOAL_RETE_TEST:
                if (additional_tests == ALL_ORIGINALS)
                {
                    dprint(DT_ADD_CONSTRAINTS_ORIG_TESTS, "Creating goal test.\n");
                    chunk_test = make_test(thisAgent, NIL, GOAL_ID_TEST);
                    chunk_test->original_test = make_test(thisAgent, NIL, GOAL_ID_TEST);
                }
                break;
            case ID_IS_IMPASSE_RETE_TEST:
                if (additional_tests == ALL_ORIGINALS)
                {
                    dprint(DT_ADD_CONSTRAINTS_ORIG_TESTS, "Creating impasse test.\n");
                    chunk_test =  make_test(thisAgent, NIL, IMPASSE_ID_TEST);;
                    chunk_test->original_test = make_test(thisAgent, NIL, IMPASSE_ID_TEST);
                }
                break;
            case DISJUNCTION_RETE_TEST:
                if (additional_tests == ALL_ORIGINALS)
                {
                    dprint(DT_ADD_CONSTRAINTS_ORIG_TESTS, "Creating disjunction test.\n");
                    chunk_test = make_test(thisAgent, NIL, DISJUNCTION_TEST);
                    chunk_test->original_test = make_test(thisAgent, NIL, DISJUNCTION_TEST);
                    chunk_test->data.disjunction_list = copy_symbol_list_adding_references(thisAgent, rt->data.disjunction_list);
                    /* MToDo | We probably don't need to copy the disjunction list to original_test.  Will not be used. */
                    chunk_test->original_test->data.disjunction_list = copy_symbol_list_adding_references(thisAgent, rt->data.disjunction_list);
                    if (rt->right_field_num == 0)
                    {
                        add_test(thisAgent, &(cond->data.tests.id_test), chunk_test);
                        dprint(DT_ADD_CONSTRAINTS_ORIG_TESTS, "adding relational test to id resulting in: %t\n", cond->data.tests.id_test);

                    }
                    else if (rt->right_field_num == 1)
                    {
                        add_test(thisAgent, &(cond->data.tests.attr_test), chunk_test);
                        dprint(DT_ADD_CONSTRAINTS_ORIG_TESTS, "adding relational test to attr resulting in: %t\n", cond->data.tests.attr_test);

                    }
                    else
                    {
                        add_test(thisAgent, &(cond->data.tests.value_test), chunk_test);
                        dprint(DT_ADD_CONSTRAINTS_ORIG_TESTS, "adding relational test to value resulting in: %t\n", cond->data.tests.value_test);
                    }
                }
                break;
            default:
                if (test_is_constant_relational_test(rt->type))
                {
                    if (additional_tests == ALL_ORIGINALS)
                    {
                        dprint(DT_ADD_CONSTRAINTS_ORIG_TESTS, "Creating constant relational test.\n");
                        test_type = relational_test_type_to_test_type(kind_of_relational_test(rt->type));
                        referent = rt->data.constant_referent;
                        chunk_test = make_test(thisAgent, referent, test_type);
                        chunk_test->original_test = make_test(thisAgent, referent, test_type);
                    }
                }
                else if (test_is_variable_relational_test(rt->type))
                {
                    test_type = relational_test_type_to_test_type(kind_of_relational_test(rt->type));
                    if (!rt->data.variable_referent.levels_up)
                    {
                        dprint(DT_ADD_CONSTRAINTS_ORIG_TESTS, "Creating variable relational test.\n");
                        switch (rt->data.variable_referent.field_num)
                        {
                            case 0:
                                if (!test_includes_equality_test_for_symbol(cond->data.tests.id_test, NIL))
                                {
                                    dprint(DT_ADD_CONSTRAINTS_ORIG_TESTS, "adding gensymmed but non-unique id test...\n");
                                    add_gensymmed_equality_test(thisAgent, &(cond->data.tests.id_test), 's');

                                    dprint(DT_ADD_CONSTRAINTS_ORIG_TESTS, "added gensymmed but non-unique id test resulting in: %t\n", cond->data.tests.id_test);
                                }
                                if (additional_tests == ALL_ORIGINALS)
                                {
                                    if (!test_includes_equality_test_for_symbol(cond->data.tests.id_test->original_test, NIL))
                                    {
                                        dprint(DT_ADD_CONSTRAINTS_ORIG_TESTS, "adding gensymmed but non-unique original id test...\n");
                                        add_gensymmed_equality_test(thisAgent, &(cond->data.tests.id_test->original_test), 's');

                                        dprint(DT_ADD_CONSTRAINTS_ORIG_TESTS, "added gensymmed but non-unique original id test resulting in: %t\n", cond->data.tests.id_test);
                                    }
                                }
                                break;
                            case 1:
                                if (!test_includes_equality_test_for_symbol(cond->data.tests.attr_test, NIL))
                                {
                                    dprint(DT_ADD_CONSTRAINTS_ORIG_TESTS, "adding gensymmed but non-unique attr test...\n");
                                    add_gensymmed_equality_test(thisAgent, &(cond->data.tests.attr_test), 'a');

                                    dprint(DT_ADD_CONSTRAINTS_ORIG_TESTS, "added gensymmed but non-unique attr test resulting in: %t\n", cond->data.tests.attr_test);
                                }
                                if (additional_tests == ALL_ORIGINALS)
                                {
                                    if (!test_includes_equality_test_for_symbol(cond->data.tests.attr_test->original_test, NIL))
                                    {
                                        dprint(DT_ADD_CONSTRAINTS_ORIG_TESTS, "adding gensymmed but non-unique original attr test...\n");
                                        add_gensymmed_equality_test(thisAgent, &(cond->data.tests.attr_test->original_test), 'a');

                                        dprint(DT_ADD_CONSTRAINTS_ORIG_TESTS, "added gensymmed but non-unique original attr test resulting in: %t\n", cond->data.tests.attr_test);
                                    }
                                }
                                break;
                            case 2:
                                if (!test_includes_equality_test_for_symbol(cond->data.tests.value_test, NIL))
                                {
                                    dprint(DT_ADD_CONSTRAINTS_ORIG_TESTS, "adding gensymmed but non-unique value test...\n");
                                    add_gensymmed_equality_test(thisAgent, &(cond->data.tests.value_test),
                                                                first_letter_from_test(cond->data.tests.attr_test));

                                    dprint(DT_ADD_CONSTRAINTS_ORIG_TESTS, "added gensymmed but non-unique value test resulting in: %t\n", cond->data.tests.value_test);
                                }
                                if (additional_tests == ALL_ORIGINALS)
                                {
                                    if (!test_includes_equality_test_for_symbol(cond->data.tests.value_test->original_test, NIL))
                                    {
                                        dprint(DT_ADD_CONSTRAINTS_ORIG_TESTS, "adding gensymmed but non-unique original value test...\n");
                                        add_gensymmed_equality_test(thisAgent, &(cond->data.tests.value_test->original_test),
                                                                    first_letter_from_test(cond->data.tests.attr_test->original_test));

                                        dprint(DT_ADD_CONSTRAINTS_ORIG_TESTS, "added gensymmed but non-unique original value test resulting in: %t\n", cond->data.tests.value_test);
                                    }
                                }
                                break;
                            default:
                                assert(false);
                                break;
                        }
                    }

                    referent = var_bound_in_reconstructed_conds(thisAgent, cond,
                               rt->data.variable_referent.field_num,
                               rt->data.variable_referent.levels_up);
                    if (additional_tests == JUST_INEQUALITIES)
                    {
                        if (((test_type == EQUALITY_TEST) || (test_type == NOT_EQUAL_TEST))
                                && (referent != NIL)
                                && referent->is_identifier())
                        {
                            chunk_test = make_test(thisAgent, referent, test_type);
                        }
                        else
                        {
                            dprint(DT_ADD_CONSTRAINTS_ORIG_TESTS, "not a valid template relational test.  Ignoring.\n");
                        }
                    }
                    else if (additional_tests == ALL_ORIGINALS)
                    {
                        chunk_test = make_test(thisAgent, referent, test_type);
                        original_referent = (var_bound_in_reconstructed_original_conds(thisAgent, cond,
                                            rt->data.variable_referent.field_num,
                                            rt->data.variable_referent.levels_up))->data.referent;

                        dprint(DT_ADD_CONSTRAINTS_ORIG_TESTS, "created relational test with referent %y.\n", original_referent);
                        chunk_test->original_test = make_test(thisAgent, original_referent, test_type);

                        /* -- Set identity information for relational test with variable as original symbol-- */
                        if (original_referent && original_referent->is_variable())
                        {
                            dprint(DT_IDENTITY_PROP, "Adding wme and test/symbol type information for relational test against %y\n", original_referent);
                            chunk_test->identity->grounding_wme = get_wme_for_referent(cond,  rt->data.variable_referent.levels_up);
                            if (chunk_test->identity->grounding_wme)
                            {
//                                wme_add_ref(chunk_test->identity->grounding_wme);
                            }
                            chunk_test->identity->grounding_field = static_cast<WME_Field>(rt->data.variable_referent.field_num);
                            if (original_referent->is_variable())
                            {
                                chunk_test->identity->original_var =  original_referent;
                                symbol_add_ref(thisAgent, original_referent);
                                if (pI_id)
                                {
                                    chunk_test->identity->original_var_id = thisAgent->variablizationManager->get_or_create_o_id(original_referent, pI_id);
                                }
                            }
                        }
                    }
                }

                if (chunk_test)
                {
                    if (rt->right_field_num == 0)
                    {
                        add_relational_test(thisAgent, &(cond->data.tests.id_test), chunk_test, pI_id);

                        dprint(DT_ADD_CONSTRAINTS_ORIG_TESTS, "adding relational test to id resulting in: %t\n", cond->data.tests.id_test);
                    }
                    else if (rt->right_field_num == 1)
                    {
                        add_relational_test(thisAgent, &(cond->data.tests.attr_test), chunk_test, pI_id);

                        dprint(DT_ADD_CONSTRAINTS_ORIG_TESTS, "adding relational test to attr resulting in: %t\n", cond->data.tests.attr_test);
                    }
                    else
                    {
                        add_relational_test(thisAgent, &(cond->data.tests.value_test), chunk_test, pI_id);

                        dprint(DT_ADD_CONSTRAINTS_ORIG_TESTS, "adding relational test to value resulting in: %t\n", cond->data.tests.value_test);
                    }
                }
                break;
        }
    }

    if (additional_tests == ALL_ORIGINALS)
    {
        if (!nvn)
        {
            if (! test_includes_equality_test_for_symbol
                    (cond->data.tests.id_test->original_test, NIL))
            {
                add_gensymmed_equality_test(thisAgent, &(cond->data.tests.id_test->original_test), 's');

                dprint(DT_ADD_CONSTRAINTS_ORIG_TESTS, "added gensymmed original id test resulting in: %t\n", cond->data.tests.id_test);
            }
            if (! test_includes_equality_test_for_symbol
                    (cond->data.tests.attr_test->original_test, NIL))
            {
                add_gensymmed_equality_test(thisAgent, &(cond->data.tests.attr_test->original_test), 'a');

                dprint(DT_ADD_CONSTRAINTS_ORIG_TESTS, "added gensymmed original attr test resulting in: %t\n", cond->data.tests.attr_test);
            }
            if (! test_includes_equality_test_for_symbol
                    (cond->data.tests.value_test->original_test, NIL))
            {
                add_gensymmed_equality_test(thisAgent, &(cond->data.tests.value_test->original_test),
                                            first_letter_from_test(cond->data.tests.attr_test->original_test));

                dprint(DT_ADD_CONSTRAINTS_ORIG_TESTS, "added gensymmed original value test resulting in: %t\n", cond->data.tests.value_test);
            }
        }

    }

    dprint(DT_ADD_CONSTRAINTS_ORIG_TESTS, "Final test (without identity): %l\n", cond);

    fill_identity_for_eq_tests(thisAgent, cond->data.tests.id_test, w, ID_ELEMENT, pI_id);
    fill_identity_for_eq_tests(thisAgent, cond->data.tests.attr_test, w, ATTR_ELEMENT, pI_id);
    fill_identity_for_eq_tests(thisAgent, cond->data.tests.value_test, w, VALUE_ELEMENT, pI_id);

    dprint(DT_ADD_CONSTRAINTS_ORIG_TESTS, "add_additional_tests_and_originals finished for %s.\n",
           thisAgent->newly_created_instantiations->prod->name->sc->name);
    dprint(DT_ADD_CONSTRAINTS_ORIG_TESTS, "Final test: %l\n", cond);
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

    allocate_with_pool(thisAgent, &thisAgent->test_pool, &new_ct);

    new_ct->type = test_type;
    new_ct->data.referent = sym;
    new_ct->original_test = NULL;
    new_ct->eq_test = NULL;
    /* MToDo| Should limit creation of identity to only tests that need them.
     *        For example, STIs and tests read during initial parse don't
     *        need identity. */
    new_ct->identity = new identity_info;

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

/* -- find_original_equality_in_conjunctive_test
 *
 *    This function will find the first equality test in the original
 *    tests of a conjunctive test, preferring equality tests on variables
 *    over equality tests on literal constants.
 *
 *    Note: This function will only return an equality test on a literal
 *    constant only after it does a complete scan of the conjunction and
 *    determines that there doesn't exist an equality test on a variable
 *    symbol. -- */

test find_original_equality_test_preferring_vars(test t, bool useOriginals)
{

    cons* c;
    test ct, found_literal = NULL, foundTest = NULL;

    if (t)
    {
        switch (t->type)
        {

            case EQUALITY_TEST:
                if (useOriginals)
                {
                    return find_original_equality_test_preferring_vars(t->original_test, false);
                }
                else
                {
                    assert(t->data.referent);
                    return t;
                }
                break;

            case CONJUNCTIVE_TEST:
                for (c = t->data.conjunct_list; c != NIL; c = c->rest)
                {
                    ct = static_cast<test>(c->first);
                    assert(ct);
                    if (useOriginals)
                    {
                        foundTest = find_original_equality_test_preferring_vars(ct->original_test, false);
                        if (foundTest)
                        {
                            assert(foundTest->data.referent);
                            if (foundTest->data.referent->is_variable())
                            {
                                return foundTest;
                            }
                            else
                            {
                                found_literal = foundTest;
                            }
                        }
                    }
                    else
                    {
                        if (ct->type == EQUALITY_TEST)
                        {
                            assert(ct->data.referent);
                            if (ct->data.referent->is_variable())
                            {
                                return ct;
                            }
                            else
                            {
                                found_literal = ct;
                            }
                        }
                    }
                }
                /* -- At this point, we have not found an equality test on a variable.  If
                 *    we have found one on a literal, we return it -- */
                return found_literal;
                break;
            default:
                break;
        }
    }
    return NULL;
}

/* No longer used, but could be again in the future */
void cache_eq_test(test t)
{
    if (t->type == CONJUNCTIVE_TEST)
    {
        t->eq_test = equality_test_found_in_test(t);
        t->eq_test->eq_test = t->eq_test;
    }
    else if (t->type == EQUALITY_TEST)
    {
        t->eq_test = t;
    }
    else
    {
        t->eq_test = NULL;
    }
}

void fill_identity_for_eq_tests(agent* thisAgent, test t, wme* w, WME_Field default_field, uint64_t pI_id)
{
    cons* c;
    test orig_test;

    if (test_is_blank(t))
    {
        return;
    }

    if (t->type == EQUALITY_TEST)
    {
        if (t->original_test && t->original_test->data.referent->symbol_type == VARIABLE_SYMBOL_TYPE)
        {
            orig_test = find_original_equality_test_preferring_vars(t, true);
            if (orig_test && orig_test->data.referent->is_variable())
            {
//                dprint(DT_IDENTITY_PROP, "Caching original symbol and wme in identity for \"%y\": %y + %s\n",
//                       t->data.referent, orig_test->data.referent,
//                       (w ? "WME" : "No WME"));
                t->identity->original_var = orig_test->data.referent;
                symbol_add_ref(thisAgent, t->identity->original_var);
                if (pI_id)
                {
                    t->identity->original_var_id = thisAgent->variablizationManager->get_or_create_o_id(orig_test->data.referent, pI_id);
                }
            }
        }
        else
        {
//            dprint(DT_IDENTITY_PROP, "No original test for \"%y\".  Cannot set identity's original var!\n", t->data.referent);
        }
        if (!t->identity->grounding_wme)
        {
            t->identity->grounding_wme = w;
            if (w)
            {
//                wme_add_ref(w);
            }
        }
        if (t->identity->grounding_field == NO_ELEMENT)
        {
            t->identity->grounding_field = default_field;
        }
    }
    else if (t->type == CONJUNCTIVE_TEST)
    {
        for (c = t->data.conjunct_list; c != NIL; c = c->rest)
        {
            fill_identity_for_eq_tests(thisAgent, static_cast<test>(c->first), w, default_field, pI_id);
        }
    }
}

