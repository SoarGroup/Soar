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

/****************************/
/* ----------------------------------------------------------------
 This returns a boolean that indicates that one condition is
 greater than another in some ordering of the conditions. The ordering
 is dependent upon the hash-value of each of the tests in the
 condition.
------------------------------------------------------------------*/

#define NON_EQUAL_TEST_RETURN_VAL 0  /* some unusual number */

uint32_t canonical_test(test t)
{
    Symbol* sym;
    
    if (test_is_blank_test(t))
    {
        return NON_EQUAL_TEST_RETURN_VAL;
    }
    
    if (test_is_blank_or_equality_test(t))
    {
        sym = referent_of_equality_test(t);
        if (sym->symbol_type == STR_CONSTANT_SYMBOL_TYPE ||
                sym->symbol_type == INT_CONSTANT_SYMBOL_TYPE ||
                sym->symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE)
        {
            return sym->hash_id;
        }
        else
        {
            return NON_EQUAL_TEST_RETURN_VAL;
        }
    }
    return NON_EQUAL_TEST_RETURN_VAL;
}

#define CANONICAL_TEST_ORDER canonical_test

/*
#define CANONICAL_TEST_ORDER hash_test
*/

bool canonical_cond_greater(condition* c1, condition* c2)
/*

 Original:  676,362 total rete nodes (1 dummy + 560045 positive + 4
            unhashed positive + 2374 negative + 113938 p_nodes)

The following notation describes the order of tests and the relation
of the hash_test that was used. IAV< means test the (I)d field first
then the (A)ttribute field, then the (V)alue field. and use less than
as the ordering constraint. The actual ordering constraint should not
make any difference.

 IAV<:  737,605 total rete nodes (1 dummy + 617394 positive + 3
            unhashed positive + 6269 negative + 113938 p_nodes)

Realized that the identifier will always be a variable and thus
shouldn't be part of the ordering.

Changed to put all negative tests in front of cost 1 tests list.
   That is always break ties of cost 1 with a negative test if
   it exists.

Changed so that canonical_test_order returns a negative -1 when
comparing anything but constants. Also fixed a bug.

Consistency checks:

 Original:  676,362 total rete nodes (1 dummy + 560045 positive + 4
            unhashed positive + 2374 negative + 113938 p_nodes)
    Still holds with 1 optimization in and always returning False

 Remove 1:  720,126 total rete nodes (1 dummy + 605760 positive + 4
            unhashed positive +  423 negative + 113938 p_nodes)
    Always returning False causes the first item in the tie list to
       be picked.

 Surprise:  637,482 total rete nodes (1 dummy + 523251 positive + 3
            unhashed positive +  289 negative + 113938 p_nodes)
    Without 1 optimization and always returning True. Causes the
      last item in 1-tie list to be picked.

 In the following tests ht means hash test provided the canonical
 value. ct means that the routine constant test provided the canonical
 value. ct provides a value for non constant equality tests. I tried
 both 0 and a big number (B)  with no difference noted.

  ht_AV<:   714,427 total rete nodes (1 dummy + 600197 positive + 2
            unhashed positive +  289 negative + 113938 p_nodes)

  ht_AV>:   709,637 total rete nodes (1 dummy + 595305 positive + 3
            unhashed positive +  390 negative + 113938 p_nodes)

  ct0_AV>:  709,960 total rete nodes (1 dummy + 595628 positive + 3
            unhashed positive +  390 negative + 113938 p_nodes)

  ct0_AV<:  714,162 total rete nodes (1 dummy + 599932 positive + 2
            unhashed positive +  289 negative + 113938 p_nodes)

  ctB_AV>:  709,960 total rete nodes (1 dummy + 595628 positive + 3
            unhashed positive +  390 negative + 113938 p_nodes)

  ctB_AV<:  714,162 total rete nodes (1 dummy + 599932 positive + 2
            unhashed positive +  289 negative + 113938 p_nodes)

  ctB_VA>:  691,193 total rete nodes (1 dummy + 576861 positive + 3
            unhashed positive +  390 negative + 113938 p_nodes)

  ctB_VA<:  704,539 total rete nodes (1 dummy + 590309 positive + 2
            unhashed positive +  289 negative + 113938 p_nodes)

  ct0_VA<:  744,604 total rete nodes (1 dummy + 630374 positive + 2
            unhashed positive +  289 negative + 113938 p_nodes)

  ct0_VA>:  672,367 total rete nodes (1 dummy + 558035 positive + 3
            unhashed positive +  390 negative + 113938 p_nodes)

   ht_VA>:  727,742 total rete nodes (1 dummy + 613517 positive + 3
            unhashed positive +  283 negative + 113938 p_nodes)

   ht_VA<:  582,559 total rete nodes (1 dummy + 468328 positive + 3
            unhashed positive +  289 negative + 113938 p_nodes)

Changed  < to > 10/5/92*/
{
    uint32_t test_order_1, test_order_2;
    
    if ((test_order_1 = CANONICAL_TEST_ORDER(c1->data.tests.attr_test)) <
            (test_order_2 = CANONICAL_TEST_ORDER(c2->data.tests.attr_test)))
    {
        return true;
    }
    else if (test_order_1 == test_order_2 &&
             CANONICAL_TEST_ORDER(c1->data.tests.value_test) <
             CANONICAL_TEST_ORDER(c2->data.tests.value_test))
    {
        return true;
    }
    return false;
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
