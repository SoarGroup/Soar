#include "test.h"

#include "agent.h"
#include "condition.h"
#include "ebc.h"
#include "ebc_identity.h"
#include "ebc_repair.h"
#include "explanation_memory.h"
#include "output_manager.h"
#include "instantiation.h"
#include "preference.h"
#include "print.h"
#include "rete.h"
#include "run_soar.h"
#include "symbol.h"
#include "symbol_manager.h"
#include "working_memory.h"

cons* copy_test_list(agent* thisAgent, cons* c, test* pEq_test, bool pUnify_variablization_identity, bool pStripLiteralConjuncts)
{
    cons* new_c;

    if (!c)
    {
        return NIL;
    }
    allocate_cons(thisAgent, &new_c);
    new_c->first = copy_test(thisAgent, static_cast<test>(c->first), pUnify_variablization_identity, pStripLiteralConjuncts);
    if (static_cast<test>(new_c->first)->type == EQUALITY_TEST)
    {
        *pEq_test = static_cast<test>(new_c->first);
    }
    new_c->rest = copy_test_list(thisAgent, c->rest, pEq_test, pUnify_variablization_identity, pStripLiteralConjuncts);
    return new_c;
}

inline bool in_null_identity_set(test t) { if (t->identity) return t->identity->literalized(); return true; };

test copy_test(agent* thisAgent, test t, bool pUseUnifiedIdentitySet, bool pStripLiteralConjuncts, bool remove_state_impasse, bool* removed_goal, bool* removed_impasse)
{
    test new_ct;

    if (!t) return NULL;

    switch (t->type)
    {
        case GOAL_ID_TEST:
            if (remove_state_impasse)
            {
                if (removed_goal) *removed_goal = true;
                return NULL;
            }
        case IMPASSE_ID_TEST:
            if (remove_state_impasse)
            {
                if (removed_impasse) *removed_impasse = true;
                return NULL;
            }
        case SMEM_LINK_UNARY_TEST:
        case SMEM_LINK_UNARY_NOT_TEST:
            new_ct = make_test(thisAgent, NIL, t->type);
            break;
        case DISJUNCTION_TEST:
            new_ct = make_test(thisAgent, NIL, t->type);
            new_ct->data.disjunction_list = thisAgent->symbolManager->copy_symbol_list_adding_references(t->data.disjunction_list);
            break;
        case CONJUNCTIVE_TEST:
            if (pStripLiteralConjuncts && thisAgent->explanationBasedChunker->ebc_settings[SETTING_EBC_LEARNING_ON] && in_null_identity_set(t->eq_test))
            {
                new_ct = make_test(thisAgent, t->eq_test->data.referent, t->eq_test->type);
                if (pUseUnifiedIdentitySet)
                {
                    if (t->eq_test->identity)
                    {
                        new_ct->inst_identity     = t->eq_test->identity->get_identity();
                        set_test_identity(thisAgent, new_ct, t->eq_test->identity->joined_identity);
                        new_ct->chunk_inst_identity = t->eq_test->identity->get_clone_identity();
                    } else {
                        new_ct->inst_identity = t->eq_test->inst_identity;
                        set_test_identity(thisAgent, new_ct, t->eq_test->identity);
                        new_ct->chunk_inst_identity = t->eq_test->chunk_inst_identity;
                    }
                } else {
                    new_ct->inst_identity = t->eq_test->inst_identity;
                    set_test_identity(thisAgent, new_ct, t->eq_test->identity);
                    new_ct->chunk_inst_identity = t->eq_test->chunk_inst_identity;
                }
            }
            else if (remove_state_impasse)
            {
                new_ct = NULL;
                test temp;
                cons* c;
                 for (c = t->data.conjunct_list; c != NIL; c = c->rest)
                 {
                     temp = copy_test(thisAgent, static_cast<test>(c->first), pUseUnifiedIdentitySet, pStripLiteralConjuncts,
                                      remove_state_impasse, removed_goal, removed_impasse);
                     if (temp)
                     {
                         add_test(thisAgent, &new_ct, temp);
                     }
                 }
                 if (new_ct->type == CONJUNCTIVE_TEST)
                 {
                     new_ct->data.conjunct_list = destructively_reverse_list(new_ct->data.conjunct_list);
                 }
            } else {
                new_ct = make_test(thisAgent, NIL, t->type);
                new_ct->data.conjunct_list = copy_test_list(thisAgent, t->data.conjunct_list, &(new_ct->eq_test), pUseUnifiedIdentitySet, pStripLiteralConjuncts);
            }
            break;
        default:
            new_ct = make_test(thisAgent, t->data.referent, t->type);
            new_ct->inst_identity = t->inst_identity;
            new_ct->chunk_inst_identity = t->chunk_inst_identity;
            if (t->type == EQUALITY_TEST)
            {
                new_ct->eq_test = new_ct;
            }
            if (pUseUnifiedIdentitySet && thisAgent->explanationBasedChunker->ebc_settings[SETTING_EBC_LEARNING_ON] && new_ct->identity)
            {
                new_ct->inst_identity        = get_joined_identity_id(new_ct->identity);
                new_ct->chunk_inst_identity  = get_joined_identity_chunk_inst_id(new_ct->identity);
                set_test_identity(thisAgent, new_ct, get_joined_identity(new_ct->identity));
            } else {
                set_test_identity(thisAgent, new_ct, t->identity);
            }

            break;
    }
    return new_ct;
}

/* ----------------------------------------------------------------
   Deallocates a test.
---------------------------------------------------------------- */

void deallocate_test(agent* thisAgent, test t)
{
    cons* c, *next_c;

    if (!t) return;

    switch (t->type)
    {
        case GOAL_ID_TEST:
        case IMPASSE_ID_TEST:
        case SMEM_LINK_UNARY_TEST:
        case SMEM_LINK_UNARY_NOT_TEST:
            break;
        case DISJUNCTION_TEST:
            thisAgent->symbolManager->deallocate_symbol_list_removing_references(t->data.disjunction_list);
            break;
        case CONJUNCTIVE_TEST:
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
            t->data.conjunct_list = NULL;
            break;
        default: /* tests with a referent */
            thisAgent->symbolManager->symbol_remove_ref(&t->data.referent);
            break;
    }

    if (t->identity) IdentitySet_remove_ref(thisAgent, t->identity);

    thisAgent->memoryManager->free_with_pool(MP_test, t);
}

void merge_disjunction_tests(agent* thisAgent, test destination, test new_test)
{
    cons* c_new, *c_next, *c_last = NULL, *c_first = NULL;
    Symbol* lSym;
    tc_number lTC = get_new_tc_number(thisAgent);
    tc_number lTC2 = get_new_tc_number(thisAgent);

    int dest_count, new_count, final_count;
    final_count = dest_count = new_count = 0;

    for (c_new = destination->data.disjunction_list; c_new != NULL; c_new = c_new->rest)
    {
        static_cast<Symbol*>(c_new->first)->tc_num = lTC;
        ++dest_count;
    }

    for (c_new = new_test->data.disjunction_list; c_new != NULL; c_new = c_next)
    {
        lSym = static_cast<Symbol*>(c_new->first);
        ++new_count;
        c_next = c_new->rest;
        if (lSym->tc_num == lTC)
        {
            lSym->tc_num = lTC2;
        }
        thisAgent->symbolManager->symbol_remove_ref(&lSym);
        free_cons(thisAgent, c_new);
    }
    new_test->data.disjunction_list = NULL;
    deallocate_test(thisAgent, new_test);

    for (c_new = destination->data.disjunction_list; c_new != NULL; c_new = c_next)
    {
        lSym = static_cast<Symbol*>(c_new->first);
        c_next = c_new->rest;
        if (lSym->tc_num != lTC2)
        {
            if (c_last)
            {
                c_last->rest = c_next;
            }
            thisAgent->symbolManager->symbol_remove_ref(&lSym);
            free_cons(thisAgent, c_new);
            continue;
        } else if (!c_first) {
            c_first = c_new;
        }
        c_last = c_new;
        ++final_count;
    }
    destination->data.disjunction_list = c_first;
    thisAgent->explanationMemory->increment_stat_merged_disjunctions();
}

bool add_test_merge_disjunctions(agent* thisAgent, test* dest_test_address, test new_test)
{
    test destination = 0;
    cons* c;

    destination = *dest_test_address;

    if (destination->type != CONJUNCTIVE_TEST)
    {
        if (destination->type == DISJUNCTION_TEST)
        {
            merge_disjunction_tests(thisAgent, destination, new_test);
            return true;
        }

        destination = make_test(thisAgent, NIL, CONJUNCTIVE_TEST);
        allocate_cons(thisAgent, &c);
        destination->data.conjunct_list = c;
        destination->eq_test = (*dest_test_address)->eq_test;
        c->first = *dest_test_address;
        c->rest = NIL;
        *dest_test_address = destination;
    }

    for (c = destination->data.conjunct_list; c != NIL; c = c->rest)
    {
        if (static_cast<test>(c->first)->type == DISJUNCTION_TEST)
        {
            merge_disjunction_tests(thisAgent, static_cast<test>(c->first), new_test);
            return true;
        }
    }
    /* --- now add add_test to the conjunct list --- */
    allocate_cons(thisAgent, &c);
    c->first = new_test;
    c->rest = destination->data.conjunct_list;
    destination->data.conjunct_list = c;

    return true;
}


/* ----------------------------------------------------------------
   Destructively modifies the first test (t) by adding the second
   one (add_me) to it (usually as a new conjunct).  The first test
   need not be a conjunctive test nor even exist.
---------------------------------------------------------------- */
bool add_test(agent* thisAgent, test* dest_test_address, test new_test, bool merge_disjunctions)
{

    test destination = 0;//, original = 0;
    cons* c;//, *c_orig;

    if (!new_test)
    {
        return false;
    }

    if (!(*dest_test_address))
    {
        *dest_test_address = new_test;
        return true;
    }

    destination = *dest_test_address;

    /* Since this function is called frequently but merges infrequently, we call a special
     * version of this function instead */
    if (merge_disjunctions && (new_test->type == DISJUNCTION_TEST))
    {
        return add_test_merge_disjunctions(thisAgent, dest_test_address, new_test);
    }

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
    }

    /* --- now add add_test to the conjunct list --- */
    allocate_cons(thisAgent, &c);
    c->first = new_test;
    c->rest = destination->data.conjunct_list;
    destination->data.conjunct_list = c;

    return true;
}


/* ----------------------------------------------------------------
   Same as add_test(), only has no effect if the second
   test is already included in the first one.
---------------------------------------------------------------- */

void add_test_if_not_already_there(agent* thisAgent, test* t, test add_me, bool neg, bool merge_disjunctions)
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

    if (thisAgent->explanationBasedChunker->is_learning_chunk()) thisAgent->explanationMemory->increment_stat_operational_constraints();

    add_test(thisAgent, t, add_me, merge_disjunctions);
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
        case IMPASSE_ID_TEST:
        case SMEM_LINK_UNARY_TEST:
        case SMEM_LINK_UNARY_NOT_TEST:
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
//    test test1, test2;

    if (t1->type != t2->type)
    {
        return false;
    }

    switch (t1->type)
    {
        case GOAL_ID_TEST:
        case IMPASSE_ID_TEST:
        case SMEM_LINK_UNARY_TEST:
        case SMEM_LINK_UNARY_NOT_TEST:
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
            return false;
        default:  /* relational tests */
        {
            if (t1->data.referent != t2->data.referent)
            {
                return false;
            }
            if (considerIdentity)
            {
                return (t1->identity->joined_identity == t2->identity->joined_identity);
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
        case SMEM_LINK_UNARY_TEST:
            return 42201412;
        case SMEM_LINK_UNARY_NOT_TEST:
            return 1455212;
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
        case SMEM_LINK_TEST:
        case SMEM_LINK_NOT_TEST:
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

bool test_includes_goal_or_impasse_id_test(test t, bool look_for_goal, bool look_for_impasse)
{
    cons* c;

    if (t->type == EQUALITY_TEST) return false;
    if (look_for_goal && (t->type == GOAL_ID_TEST)) return true;
    if (look_for_impasse && (t->type == IMPASSE_ID_TEST)) return true;
    if (t->type == CONJUNCTIVE_TEST)
    {
        for (c = t->data.conjunct_list; c != NIL; c = c->rest)
            if (test_includes_goal_or_impasse_id_test(static_cast<test>(c->first), look_for_goal, look_for_impasse))
            {
                return true;
            }
        return false;
    }
    return false;
}

test find_eq_test(test t)
{
    cons* c;

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
                               tc_number tc, cons** var_list)
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
        case SMEM_LINK_UNARY_TEST:
        case SMEM_LINK_UNARY_NOT_TEST:
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

/* The add_LTI parameter is available so that when Soar is marking symbols for
 * action ordering based on whether the levels of the symbols would be known,
 * it also consider whether the LTIs level can be determined by being linked
 * to a LHS element or a RHS action that has already been executed */

void add_bound_variables_in_test(agent* thisAgent, test t, tc_number tc, cons** var_list)
{
    cons* c;
    Symbol* referent = NULL;

    if (!t) return;

    switch (t->type)
    {
        case GOAL_ID_TEST:
        case IMPASSE_ID_TEST:
        case DISJUNCTION_TEST:
        case SMEM_LINK_UNARY_TEST:
        case SMEM_LINK_UNARY_NOT_TEST:
            break;
        case CONJUNCTIVE_TEST:
            for (c = t->data.conjunct_list; c != NIL; c = c->rest)
            {
                add_bound_variables_in_test(thisAgent, static_cast<test>(c->first), tc, var_list);
            }
            break;
            /* If you re-enable the next section, variables bound to lti-id's will be legal on the rhs.
             * Note that they will not work as expected and will point to instance variable */
            //        case SMEM_LINK_TEST:
            //            referent = t->data.referent;
            //            break;
        case EQUALITY_TEST:
            referent = t->data.referent;
            break;
        default:
            break;
    }

    if (referent && referent->is_variable())
    {
        referent->mark_if_unmarked(thisAgent, tc, var_list);
    }
    return;
}

void add_bound_variable_with_identity(agent* thisAgent, Symbol* pSym, Symbol* pMatchedSym, uint64_t pInstIdentity,  tc_number tc, matched_symbol_list* var_list)
{
    Symbol* referent;

    if (pSym->is_variable())
    {
        if (pSym->tc_num != tc)
        {
            pSym->tc_num = tc;
            if (var_list)
            {
                chunk_element* lNewUngroundedSym;
                thisAgent->memoryManager->allocate_with_pool(MP_chunk_element, &lNewUngroundedSym);
                lNewUngroundedSym->variable_sym = pSym;
                lNewUngroundedSym->inst_identity = pInstIdentity;
                lNewUngroundedSym->instantiated_sym = pMatchedSym ? pMatchedSym : pSym;
                var_list->push_back(lNewUngroundedSym);
            }
        }
    }
    return;
}

/* -----------------------------------------------------------------
   Find first letter of test, or '*' if nothing appropriate.
   (See comments on first_letter_from_symbol for more explanation.)
----------------------------------------------------------------- */

char first_letter_from_test(test t)
{
    cons* c;
    char ch;

    if (!t) return '*';

    switch (t->type)
    {
        case EQUALITY_TEST:
            return first_letter_from_symbol(t->data.referent);
        case CONJUNCTIVE_TEST:
            return first_letter_from_symbol(t->eq_test->data.referent);
        case GOAL_ID_TEST:
            return 's';
        case IMPASSE_ID_TEST:
            return 'i';
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
    New = thisAgent->symbolManager->generate_new_variable(prefix);
    eq_test = make_test(thisAgent, New, EQUALITY_TEST);
    thisAgent->symbolManager->symbol_remove_ref (&New);
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
        else if (rt->type == UNARY_SMEM_LINK_RETE_TEST)
        {
            New = make_test(thisAgent, NIL, SMEM_LINK_UNARY_TEST);
        }
        else if (rt->type == UNARY_SMEM_LINK_NOT_RETE_TEST)
        {
            New = make_test(thisAgent, NIL, SMEM_LINK_UNARY_NOT_TEST);
        }
        else if (rt->type == DISJUNCTION_RETE_TEST)
        {
            New = make_test(thisAgent, NIL, DISJUNCTION_TEST);
            New->data.disjunction_list = thisAgent->symbolManager->copy_symbol_list_adding_references(rt->data.disjunction_list);
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
                    if (!cond->data.tests.id_test || !cond->data.tests.id_test->eq_test)
                    {
                        add_gensymmed_equality_test(thisAgent, &(cond->data.tests.id_test), 's');
                    }
                }
                else if (rt->data.variable_referent.field_num == 1)
                {
                    if (!cond->data.tests.attr_test || !cond->data.tests.attr_test->eq_test)
                    {
                        add_gensymmed_equality_test(thisAgent, &(cond->data.tests.attr_test), 'a');
                    }
                }
                else
                {
                    if (!cond->data.tests.value_test || !cond->data.tests.value_test->eq_test)
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


test make_test(agent* thisAgent, Symbol* sym, TestType test_type)
{
    test new_ct;

    thisAgent->memoryManager->allocate_with_pool(MP_test, &new_ct);
    new_ct->type = test_type;
    new_ct->data.referent = sym;
    new_ct->inst_identity = new_ct->chunk_inst_identity = LITERAL_VALUE;
    new_ct->identity = NULL_IDENTITY_SET;
    new_ct->eq_test = (test_type == EQUALITY_TEST) ? new_ct : NULL;
    if (sym) thisAgent->symbolManager->symbol_add_ref(sym);
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
cons* delete_test_from_conjunct(agent* thisAgent, test* t, cons* pDeleteItem)
{
    cons* prev, *next;
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
        while (prev->rest != pDeleteItem) prev = prev->rest;
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
        (*t)->eq_test = find_eq_test(*t);
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
    if (add_me->type != EQUALITY_TEST)
    {
        if (target_test->type != CONJUNCTIVE_TEST)
        {
            if (tests_identical(target_test, add_me)) return;
        }
        else
        {
            for (c = target_test->data.conjunct_list; c != NIL; c = c->rest)
                if (tests_identical(static_cast<test>(c->first), add_me)) return;
        }
        add_test(thisAgent, t, copy_test(thisAgent, add_me), true);
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


