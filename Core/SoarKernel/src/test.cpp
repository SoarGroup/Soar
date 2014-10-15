/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/* =================================================================

                      Utility Routines for Tests

================================================================= */

#include <assert.h>
#include "kernel.h"
#include "test.h"
#include "debug.h"
#include "symtab.h"
#include "agent.h"
#include "print.h"
#include "rete.h"
#include "instantiations.h"
#include "output_manager.h"
#include "wmem.h"
#include "prefmem.h"

/* --- This just copies a consed list of tests. --- */
list* copy_test_list(agent* thisAgent, cons* c)
{
    cons* new_c;

    if (!c)
    {
        return NIL;
    }
    allocate_cons(thisAgent, &new_c);
    new_c->first = copy_test(thisAgent, static_cast<char*>(c->first));
    new_c->rest = copy_test_list(thisAgent, c->rest);
    return new_c;
}

/* ----------------------------------------------------------------
   Takes a test and returns a new copy of it.
---------------------------------------------------------------- */

test copy_test(agent* thisAgent, test t)
{
    Symbol* referent;
    complex_test* ct, *new_ct;

    if (test_is_blank_test(t))
    {
        return make_blank_test();
    }

    if (test_is_blank_or_equality_test(t))
    {
        referent = referent_of_equality_test(t);
        return make_equality_test(referent);
    }

    ct = complex_test_from_test(t);

    allocate_with_pool(thisAgent, &thisAgent->complex_test_pool, &new_ct);
    new_ct->type = ct->type;
    switch (ct->type)
    {
        case GOAL_ID_TEST:
        case IMPASSE_ID_TEST:
            break;
        case DISJUNCTION_TEST:
            new_ct->data.disjunction_list =
                copy_symbol_list_adding_references(thisAgent, ct->data.disjunction_list);
            break;
        case CONJUNCTIVE_TEST:
            new_ct->data.conjunct_list = copy_test_list(thisAgent, ct->data.conjunct_list);
            break;
        default:  /* relational tests other than equality */
            new_ct->data.referent = ct->data.referent;
            symbol_add_ref(thisAgent, ct->data.referent);
            break;
    }
    return make_test_from_complex_test(new_ct);
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
    complex_test* ct, *new_ct;
    cons* c;
    test new_t, temp;

    if (test_is_blank_or_equality_test(t))
    {
        return copy_test(thisAgent, t);
    }

    ct = complex_test_from_test(t);

    switch (ct->type)
    {
        case GOAL_ID_TEST:
            *removed_goal = true;
            return make_blank_test();
        case IMPASSE_ID_TEST:
            *removed_impasse = true;
            return make_blank_test();

        case CONJUNCTIVE_TEST:
            new_t = make_blank_test();
            for (c = ct->data.conjunct_list; c != NIL; c = c->rest)
            {
                temp = copy_test_removing_goal_impasse_tests(thisAgent, static_cast<char*>(c->first),
                        removed_goal,
                        removed_impasse);
                if (! test_is_blank_test(temp))
                {
                    add_new_test_to_test(thisAgent, &new_t, temp);
                }
            }
            if (test_is_complex_test(new_t))
            {
                new_ct = complex_test_from_test(new_t);
                if (new_ct->type == CONJUNCTIVE_TEST)
                    new_ct->data.conjunct_list =
                        destructively_reverse_list(new_ct->data.conjunct_list);
            }
            return new_t;

        default:  /* relational tests other than equality */
            return copy_test(thisAgent, t);
    }
}

/* ----------------------------------------------------------------
   Deallocates a test.
---------------------------------------------------------------- */

void deallocate_test(agent* thisAgent, test t)
{
    cons* c, *next_c;
    complex_test* ct;

    if (test_is_blank_test(t))
    {
        return;
    }
    if (test_is_blank_or_equality_test(t))
    {
        symbol_remove_ref(thisAgent, referent_of_equality_test(t));
        return;
    }

    ct = complex_test_from_test(t);

    switch (ct->type)
    {
        case GOAL_ID_TEST:
        case IMPASSE_ID_TEST:
            break;
        case DISJUNCTION_TEST:
            deallocate_symbol_list_removing_references(thisAgent, ct->data.disjunction_list);
            break;
        case CONJUNCTIVE_TEST:
            c = ct->data.conjunct_list;
            while (c)
            {
                next_c = c->rest;
                deallocate_test(thisAgent, static_cast<char*>(c->first));
                free_cons(thisAgent, c);
                c = next_c;
            }
            break;
        default: /* relational tests other than equality */
            symbol_remove_ref(thisAgent, ct->data.referent);
            break;
    }
    free_with_pool(&thisAgent->complex_test_pool, ct);
}

/* ----------------------------------------------------------------
   Destructively modifies the first test (t) by adding the second
   one (add_me) to it (usually as a new conjunct).  The first test
   need not be a conjunctive test.
---------------------------------------------------------------- */

void add_new_test_to_test(agent* thisAgent,
                          test* t, test add_me)
{
    complex_test* ct = 0;
    cons* c;
    bool already_a_conjunctive_test;

    if (test_is_blank_test(add_me))
    {
        return;
    }

    if (test_is_blank_test(*t))
    {
        *t = add_me;
        return;
    }

    /* --- if *t isn't already a conjunctive test, make it into one --- */
    already_a_conjunctive_test = false;
    if (test_is_complex_test(*t))
    {
        ct = complex_test_from_test(*t);
        if (ct->type == CONJUNCTIVE_TEST)
        {
            already_a_conjunctive_test = true;
        }
    }

    if (! already_a_conjunctive_test)
    {
        allocate_with_pool(thisAgent, &thisAgent->complex_test_pool, &ct);
        ct->type = CONJUNCTIVE_TEST;
        allocate_cons(thisAgent, &c);
        ct->data.conjunct_list = c;
        c->first = *t;
        c->rest = NIL;
        *t = make_test_from_complex_test(ct);
    }
    /* --- at this point, ct points to the complex test structure for *t --- */

    /* --- now add add_me to the conjunct list --- */
    allocate_cons(thisAgent, &c);
    c->first = add_me;
    c->rest = ct->data.conjunct_list;
    ct->data.conjunct_list = c;
}

/* ----------------------------------------------------------------
   Same as add_new_test_to_test(), only has no effect if the second
   test is already included in the first one.
---------------------------------------------------------------- */

void add_new_test_to_test_if_not_already_there(agent* thisAgent, test* t, test add_me, bool neg)
{
    complex_test* ct;
    cons* c;

    if (tests_are_equal(*t, add_me, neg))
    {
        deallocate_test(thisAgent, add_me);
        return;
    }

    if (test_is_complex_test(*t))
    {
        ct = complex_test_from_test(*t);
        if (ct->type == CONJUNCTIVE_TEST)
            for (c = ct->data.conjunct_list; c != NIL; c = c->rest)
                if (tests_are_equal(static_cast<char*>(c->first), add_me, neg))
                {
                    deallocate_test(thisAgent, add_me);
                    return;
                }
    }

    add_new_test_to_test(thisAgent, t, add_me);
}

/* ----------------------------------------------------------------
   Returns true iff the two tests are identical.
   If neg is true, ignores order of members in conjunctive tests
   and assumes variables are all equal.
---------------------------------------------------------------- */

bool tests_are_equal(test t1, test t2, bool neg)
{
    cons* c1, *c2;
    complex_test* ct1, *ct2;

    if (test_is_blank_or_equality_test(t1))
    {
        if (!test_is_blank_or_equality_test(t2))
        {
            return false;
        }

        if (t1 == t2) /* Warning: this relies on the representation of tests */
        {
            return true;
        }

        if (!neg)
        {
            return false;
        }

        // ignore variables in negation tests
        Symbol* s1 = referent_of_equality_test(t1);
        Symbol* s2 = referent_of_equality_test(t2);

        if ((s1->symbol_type == VARIABLE_SYMBOL_TYPE) && (s2->symbol_type == VARIABLE_SYMBOL_TYPE))
        {
            return true;
        }
        return false;
    }

    ct1 = complex_test_from_test(t1);
    ct2 = complex_test_from_test(t2);

    if (ct1->type != ct2->type)
    {
        return false;
    }

    switch (ct1->type)
    {
        case GOAL_ID_TEST:
            return true;

        case IMPASSE_ID_TEST:
            return true;

        case DISJUNCTION_TEST:
            for (c1 = ct1->data.disjunction_list, c2 = ct2->data.disjunction_list; (c1 != NIL) && (c2 != NIL); c1 = c1->rest, c2 = c2->rest)
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
            for (c2 = ct2->data.conjunct_list; c2 != NIL; c2 = c2->rest)
            {
                copy2.push_back(static_cast<test>(c2->first));
            }

            std::list<test>::iterator iter;
            for (c1 = ct1->data.conjunct_list; c1 != NIL; c1 = c1->rest)
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
            if (ct1->data.referent == ct2->data.referent)
            {
                return true;
            }
            return false;
    }
}

/* ----------------------------------------------------------------
   Returns a hash value for the given test.
---------------------------------------------------------------- */

uint32_t hash_test(agent* thisAgent, test t)
{
    complex_test* ct;
    cons* c;
    uint32_t result;

    if (test_is_blank_test(t))
    {
        return 0;
    }

    if (test_is_blank_or_equality_test(t))
    {
        return referent_of_equality_test(t)->hash_id;
    }

    ct = complex_test_from_test(t);

    switch (ct->type)
    {
        case GOAL_ID_TEST:
            return 34894895;  /* just use some unusual number */
        case IMPASSE_ID_TEST:
            return 2089521;
        case DISJUNCTION_TEST:
            result = 7245;
            for (c = ct->data.conjunct_list; c != NIL; c = c->rest)
            {
                result = result + static_cast<Symbol*>(c->first)->hash_id;
            }
            return result;
        case CONJUNCTIVE_TEST:
            result = 100276;
            // bug 510: conjunctive tests' order needs to be ignored
            //for (c=ct->data.disjunction_list; c!=NIL; c=c->rest)
            //  result = result + hash_test (thisAgent, static_cast<char *>(c->first));
            return result;
        case NOT_EQUAL_TEST:
        case LESS_TEST:
        case GREATER_TEST:
        case LESS_OR_EQUAL_TEST:
        case GREATER_OR_EQUAL_TEST:
        case SAME_TYPE_TEST:
            return (ct->type << 24) + ct->data.referent->hash_id;
        default:
        {
            char msg[BUFFER_MSG_SIZE];
            strncpy(msg, "production.c: Error: bad test type in hash_test\n", BUFFER_MSG_SIZE);
            msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
            abort_with_fatal_error(thisAgent, msg);
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
    complex_test* ct;

    if (test_is_blank_test(t))
    {
        return false;
    }

    if (test_is_blank_or_equality_test(t))
    {
        if (sym)
        {
            return (referent_of_equality_test(t) == sym);
        }
        return true;
    }

    ct = complex_test_from_test(t);

    if (ct->type == CONJUNCTIVE_TEST)
    {
        for (c = ct->data.conjunct_list; c != NIL; c = c->rest)
            if (test_includes_equality_test_for_symbol(static_cast<char*>(c->first), sym))
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
    complex_test* ct;
    cons* c;

    if (test_is_blank_or_equality_test(t))
    {
        return false;
    }
    ct = complex_test_from_test(t);
    if (look_for_goal && (ct->type == GOAL_ID_TEST))
    {
        return true;
    }
    if (look_for_impasse && (ct->type == IMPASSE_ID_TEST))
    {
        return true;
    }
    if (ct->type == CONJUNCTIVE_TEST)
    {
        for (c = ct->data.conjunct_list; c != NIL; c = c->rest)
            if (test_includes_goal_or_impasse_id_test(static_cast<char*>(c->first),
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
    complex_test* ct;
    cons* c;
    char msg[BUFFER_MSG_SIZE];

    if (test_is_blank_test(t))
    {
        strncpy(msg, "Internal error: can't find equality test in test\n", BUFFER_MSG_SIZE);
        msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
        abort_with_fatal_error(thisAgent, msg);
    }
    if (test_is_blank_or_equality_test(t))
    {
        return copy_test(thisAgent, t);
    }
    ct = complex_test_from_test(t);
    if (ct->type == CONJUNCTIVE_TEST)
    {
        for (c = ct->data.conjunct_list; c != NIL; c = c->rest)
            if ((! test_is_blank_test(static_cast<test>(c->first))) &&
                    (test_is_blank_or_equality_test(static_cast<test>(c->first))))
            {
                return copy_test(thisAgent, static_cast<char*>(c->first));
            }
    }
    strncpy(msg, "Internal error: can't find equality test in test\n", BUFFER_MSG_SIZE);
    abort_with_fatal_error(thisAgent, msg);
    return 0; /* unreachable, but without it, gcc -Wall warns here */
}

void add_all_variables_in_test(agent* thisAgent, test t,
                               tc_number tc, list** var_list)
{
    cons* c;
    Symbol* referent;
    complex_test* ct;

    if (test_is_blank_test(t))
    {
        return;
    }

    if (test_is_blank_or_equality_test(t))
    {
        referent = referent_of_equality_test(t);
        if (referent->symbol_type == VARIABLE_SYMBOL_TYPE)
        {
            mark_variable_if_unmarked(thisAgent, referent, tc, var_list);
        }
        return;
    }

    ct = complex_test_from_test(t);

    switch (ct->type)
    {
        case GOAL_ID_TEST:
        case IMPASSE_ID_TEST:
        case DISJUNCTION_TEST:
            break;

        case CONJUNCTIVE_TEST:
            for (c = ct->data.conjunct_list; c != NIL; c = c->rest)
            {
                add_all_variables_in_test(thisAgent, static_cast<char*>(c->first), tc, var_list);
            }
            break;

        default:
            /* --- relational tests other than equality --- */
            referent = ct->data.referent;
            if (referent->symbol_type == VARIABLE_SYMBOL_TYPE)
            {
                mark_variable_if_unmarked(thisAgent, referent, tc, var_list);
            }
            break;
    }
}

void add_bound_variables_in_test(agent* thisAgent, test t,
                                 tc_number tc, list** var_list)
{
    cons* c;
    Symbol* referent;
    complex_test* ct;

    if (test_is_blank_test(t))
    {
        return;
    }

    if (test_is_blank_or_equality_test(t))
    {
        referent = referent_of_equality_test(t);
        if (referent->symbol_type == VARIABLE_SYMBOL_TYPE)
        {
            mark_variable_if_unmarked(thisAgent, referent, tc, var_list);
        }
        return;
    }

    ct = complex_test_from_test(t);
    if (ct->type == CONJUNCTIVE_TEST)
    {
        for (c = ct->data.conjunct_list; c != NIL; c = c->rest)
        {
            add_bound_variables_in_test(thisAgent, static_cast<char*>(c->first), tc, var_list);
        }
    }
}

/* -----------------------------------------------------------------
   Find first letter of test, or '*' if nothing appropriate.
   (See comments on first_letter_from_symbol for more explanation.)
----------------------------------------------------------------- */

char first_letter_from_test(test t)
{
    complex_test* ct;
    cons* c;
    char ch;

    if (test_is_blank_test(t))
    {
        return '*';
    }
    if (test_is_blank_or_equality_test(t))
    {
        return first_letter_from_symbol(referent_of_equality_test(t));
    }

    ct = complex_test_from_test(t);
    switch (ct->type)
    {
        case GOAL_ID_TEST:
            return 's';
        case IMPASSE_ID_TEST:
            return 'i';
        case CONJUNCTIVE_TEST:
            for (c = ct->data.conjunct_list; c != NIL; c = c->rest)
            {
                ch = first_letter_from_test(static_cast<char*>(c->first));
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
    test eq_test;
    char prefix[2];

    prefix[0] = first_letter;
    prefix[1] = 0;
    New = generate_new_variable(thisAgent, prefix);
    eq_test = make_equality_test(New);
    symbol_remove_ref(thisAgent, New);
    add_new_test_to_test(thisAgent, t, eq_test);
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
    test New;
    complex_test* new_ct;
    byte test_type;

    for (; rt != NIL; rt = rt->next)
    {

        if (rt->type == ID_IS_GOAL_RETE_TEST)
        {
            allocate_with_pool(thisAgent, &thisAgent->complex_test_pool, &new_ct);
            New = make_test_from_complex_test(new_ct);
            new_ct->type = GOAL_ID_TEST;
        }
        else if (rt->type == ID_IS_IMPASSE_RETE_TEST)
        {
            allocate_with_pool(thisAgent, &thisAgent->complex_test_pool, &new_ct);
            New = make_test_from_complex_test(new_ct);
            new_ct->type = IMPASSE_ID_TEST;
        }
        else if (rt->type == DISJUNCTION_RETE_TEST)
        {
            allocate_with_pool(thisAgent, &thisAgent->complex_test_pool, &new_ct);
            New = make_test_from_complex_test(new_ct);
            new_ct->type = DISJUNCTION_TEST;
            new_ct->data.disjunction_list =
                copy_symbol_list_adding_references(thisAgent, rt->data.disjunction_list);
        }
        else if (test_is_constant_relational_test(rt->type))
        {
            test_type =
                relational_test_type_to_test_type[kind_of_relational_test(rt->type)];
            referent = rt->data.constant_referent;
            symbol_add_ref(thisAgent, referent);
            if (test_type == EQUAL_TEST_TYPE)
            {
                New = make_equality_test_without_adding_reference(referent);
            }
            else
            {
                allocate_with_pool(thisAgent, &thisAgent->complex_test_pool, &new_ct);
                New = make_test_from_complex_test(new_ct);
                new_ct->type = test_type;
                new_ct->data.referent = referent;
            }
        }
        else if (test_is_variable_relational_test(rt->type))
        {
            test_type =
                relational_test_type_to_test_type[kind_of_relational_test(rt->type)];
            if (! rt->data.variable_referent.levels_up)
            {
                /* --- before calling var_bound_in_reconstructed_conds, make sure
                   there's an equality test in the referent location (add one if
                   there isn't one already there), otherwise there'd be no variable
                   there to test against --- */
                if (rt->data.variable_referent.field_num == 0)
                {
                    if (! test_includes_equality_test_for_symbol
                            (cond->data.tests.id_test, NIL))
                    {
                        add_gensymmed_equality_test(thisAgent, &(cond->data.tests.id_test), 's');
                    }
                }
                else if (rt->data.variable_referent.field_num == 1)
                {
                    if (! test_includes_equality_test_for_symbol
                            (cond->data.tests.attr_test, NIL))
                    {
                        add_gensymmed_equality_test(thisAgent, &(cond->data.tests.attr_test), 'a');
                    }
                }
                else
                {
                    if (! test_includes_equality_test_for_symbol
                            (cond->data.tests.value_test, NIL))
                        add_gensymmed_equality_test(thisAgent, &(cond->data.tests.value_test),
                                                    first_letter_from_test(cond->data.tests.attr_test));
                }
            }
            referent = var_bound_in_reconstructed_conds(thisAgent, cond,
                       rt->data.variable_referent.field_num,
                       rt->data.variable_referent.levels_up);
            symbol_add_ref(thisAgent, referent);
            if (test_type == EQUAL_TEST_TYPE)
            {
                New = make_equality_test_without_adding_reference(referent);
            }
            else
            {
                allocate_with_pool(thisAgent, &thisAgent->complex_test_pool, &new_ct);
                New = make_test_from_complex_test(new_ct);
                new_ct->type = test_type;
                new_ct->data.referent = referent;
            }
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
            add_new_test_to_test(thisAgent, &(cond->data.tests.id_test), New);
        }
        else if (rt->right_field_num == 2)
        {
            add_new_test_to_test(thisAgent, &(cond->data.tests.value_test), New);
        }
        else
        {
            add_new_test_to_test(thisAgent, &(cond->data.tests.attr_test), New);
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
    test New;

    temp = var_bound_in_reconstructed_conds(thisAgent, cond, field_num, levels_up);
    New = make_equality_test(temp);
    add_new_test_to_test(thisAgent, &(cond->data.tests.id_test), New);
}

