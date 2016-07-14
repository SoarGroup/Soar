
/*************************************************************************
 *
 *  file:  condition.cpp
 *
 * ====================================================================
 *                    Condition Utilities
 *
 * This file contains various utility routines for manipulating
 * Init_production_utilities() should be called before anything else here.
 * =======================================================================
 */

#include "run_soar.h"
#include "condition.h"

#include "agent.h"
#include "test.h"
#include "memory_manager.h"
#include "dprint.h"
#include "explanation_memory.h"
#include "identity_record.h"

/* ----------------------------------------------------------------
   Deallocates a condition list (including any NCC's and tests in it).
---------------------------------------------------------------- */

void deallocate_condition(agent* thisAgent, condition*& cond)
{
    dprint(DT_DEALLOCATES, "Deallocating condition %l\n", cond);
    if (cond->type == CONJUNCTIVE_NEGATION_CONDITION)
    {
        deallocate_condition_list(thisAgent, cond->data.ncc.top);
    }
    else     /* positive and negative conditions */
    {
        deallocate_test(thisAgent, cond->data.tests.id_test);
        deallocate_test(thisAgent, cond->data.tests.attr_test);
        deallocate_test(thisAgent, cond->data.tests.value_test);
    }
    thisAgent->memoryManager->free_with_pool(MP_condition, cond);
    cond = NULL;
}

void deallocate_condition_list(agent* thisAgent,
                               condition*& cond_list)
{
    condition* c;

    while (cond_list)
    {
        c = cond_list;
        cond_list = cond_list->next;
        if (c->type == CONJUNCTIVE_NEGATION_CONDITION)
        {
            deallocate_condition_list(thisAgent, c->data.ncc.top);
        }
        else     /* positive and negative conditions */
        {
            dprint(DT_DEALLOCATES, "Deallocating condition: %l\n", c);
            deallocate_test(thisAgent, c->data.tests.id_test);
            deallocate_test(thisAgent, c->data.tests.attr_test);
            deallocate_test(thisAgent, c->data.tests.value_test);
        }
        thisAgent->memoryManager->free_with_pool(MP_condition, c);
    }
    cond_list = NULL;
}

condition* make_condition(agent* thisAgent, test pId, test pAttr, test pValue)
{
    condition* cond;
    thisAgent->memoryManager->allocate_with_pool(MP_condition,  &cond);
    cond->data.tests.id_test = pId;
    cond->data.tests.attr_test = pAttr;
    cond->data.tests.value_test = pValue;
    cond->type = POSITIVE_CONDITION;
    cond->already_in_tc = false;
    cond->test_for_acceptable_preference = false;
    cond->next = cond->prev = cond->counterpart = NULL;
    cond->inst = NULL;
    cond->bt.wme_ = NULL;
    cond->bt.level = 0;
    cond->bt.trace = NULL;
    cond->bt.CDPS = NULL;
    /* Other data initialized to 0 in struct initializers */
    return cond;
}

condition* copy_condition_without_relational_constraints(agent* thisAgent,
        condition* cond)
{
    condition* New;

    if (!cond)
    {
        return NIL;
    }
    New = make_condition(thisAgent);
    New->type = cond->type;

    switch (cond->type)
    {
        case POSITIVE_CONDITION:
            New->bt = cond->bt;
            New->data.tests.id_test = copy_test_without_relationals(thisAgent, cond->data.tests.id_test);
            New->data.tests.attr_test = copy_test_without_relationals(thisAgent, cond->data.tests.attr_test);
            New->data.tests.value_test = copy_test_without_relationals(thisAgent, cond->data.tests.value_test);
            New->test_for_acceptable_preference = cond->test_for_acceptable_preference;
            break;
        case NEGATIVE_CONDITION:
            New->data.tests.id_test = copy_test(thisAgent, cond->data.tests.id_test);
            New->data.tests.attr_test = copy_test(thisAgent, cond->data.tests.attr_test);
            New->data.tests.value_test = copy_test(thisAgent, cond->data.tests.value_test);
            New->test_for_acceptable_preference = cond->test_for_acceptable_preference;
            break;
        case CONJUNCTIVE_NEGATION_CONDITION:
            copy_condition_list(thisAgent, cond->data.ncc.top, &(New->data.ncc.top),
                                &(New->data.ncc.bottom));
            break;
    }
    return New;
}

/* ----------------------------------------------------------------
   Returns a new copy of the given condition.
---------------------------------------------------------------- */

condition* copy_condition(agent* thisAgent, condition* cond, bool pUnify_variablization_identity, bool pStripLiteralConjuncts)
{
    condition* New;

    if (!cond)
    {
        return NIL;
    }
    New = make_condition(thisAgent);
    New->type = cond->type;
    New->counterpart = cond->counterpart;

    switch (cond->type)
    {
        case POSITIVE_CONDITION:
            New->bt = cond->bt;
            New->data.tests.id_test = copy_test(thisAgent, cond->data.tests.id_test, pUnify_variablization_identity, pStripLiteralConjuncts);
            New->data.tests.attr_test = copy_test(thisAgent, cond->data.tests.attr_test, pUnify_variablization_identity, pStripLiteralConjuncts);
            New->data.tests.value_test = copy_test(thisAgent, cond->data.tests.value_test, pUnify_variablization_identity, pStripLiteralConjuncts);
            New->test_for_acceptable_preference = cond->test_for_acceptable_preference;
            break;
        case NEGATIVE_CONDITION:
            New->data.tests.id_test = copy_test(thisAgent, cond->data.tests.id_test, pUnify_variablization_identity, pStripLiteralConjuncts);
            New->data.tests.attr_test = copy_test(thisAgent, cond->data.tests.attr_test, pUnify_variablization_identity, pStripLiteralConjuncts);
            New->data.tests.value_test = copy_test(thisAgent, cond->data.tests.value_test, pUnify_variablization_identity, pStripLiteralConjuncts);
            New->test_for_acceptable_preference = cond->test_for_acceptable_preference;
            break;
        case CONJUNCTIVE_NEGATION_CONDITION:
            copy_condition_list(thisAgent, cond->data.ncc.top, &(New->data.ncc.top),
                &(New->data.ncc.bottom), pUnify_variablization_identity, pStripLiteralConjuncts, false);
            break;
    }
    return New;
}

/* ----------------------------------------------------------------
   Copies the given condition list, returning pointers to the
   top-most and bottom-most conditions in the new copy.
---------------------------------------------------------------- */

void copy_condition_list(agent* thisAgent,
                         condition* top_cond,
                         condition** dest_top,
                         condition** dest_bottom,
                         bool pUnify_variablization_identity,
                         bool pStripLiteralConjuncts,
                         bool pCopyInstantiation)
{
    condition* New, *prev;

    prev = NIL;
    while (top_cond)
    {
        New = copy_condition(thisAgent, top_cond, pUnify_variablization_identity, pStripLiteralConjuncts);
        if (pCopyInstantiation)
        {
            New->inst = top_cond->inst;
        }
        if (prev)
        {
            prev->next = New;
        }
        else
        {
            *dest_top = New;
        }
        New->prev = prev;
        prev = New;
        top_cond = top_cond->next;
    }
    if (prev)
    {
        prev->next = NIL;
    }
    else
    {
        *dest_top = NIL;
    }
    *dest_bottom = prev;
}

/* ----------------------------------------------------------------
   Returns true iff the two conditions are identical.
---------------------------------------------------------------- */

bool conditions_are_equal(condition* c1, condition* c2)
{
    if (c1->type != c2->type)
    {
        return false;
    }
    bool neg = true;
    switch (c1->type)
    {
        case POSITIVE_CONDITION:
            neg = false;
        case NEGATIVE_CONDITION:
            if (! tests_are_equal(c1->data.tests.id_test,
                                  c2->data.tests.id_test, neg))
            {
                return false;
            }
            if (! tests_are_equal(c1->data.tests.attr_test,
                                  c2->data.tests.attr_test, neg))
            {
                return false;
            }
            if (! tests_are_equal(c1->data.tests.value_test,
                                  c2->data.tests.value_test, neg))
            {
                return false;
            }
            if (c1->test_for_acceptable_preference !=
                    c2->test_for_acceptable_preference)
            {
                return false;
            }
            return true;

        case CONJUNCTIVE_NEGATION_CONDITION:
            for (c1 = c1->data.ncc.top, c2 = c2->data.ncc.top;
                    ((c1 != NIL) && (c2 != NIL));
                    c1 = c1->next, c2 = c2->next)
                if (! conditions_are_equal(c1, c2))
                {
                    return false;
                }
            if (c1 == c2)
            {
                return true;    /* make sure they both hit end-of-list */
            }
            return false;
    }
    return false; /* unreachable, but without it, gcc -Wall warns here */
}

/* ----------------------------------------------------------------
   Returns a hash value for the given condition.
---------------------------------------------------------------- */

uint32_t hash_condition(agent* thisAgent,
                        condition* cond)
{
    uint32_t result;
    condition* c;

    switch (cond->type)
    {
        case POSITIVE_CONDITION:
            result = hash_test(thisAgent, cond->data.tests.id_test);
            result = (result << 24) | (result >>  8);
            result ^= hash_test(thisAgent, cond->data.tests.attr_test);
            result = (result << 24) | (result >>  8);
            result ^= hash_test(thisAgent, cond->data.tests.value_test);
            if (cond->test_for_acceptable_preference)
            {
                result++;
            }
            break;
        case NEGATIVE_CONDITION:
            result = 1267818;
            result ^= hash_test(thisAgent, cond->data.tests.id_test);
            result = (result << 24) | (result >>  8);
            result ^= hash_test(thisAgent, cond->data.tests.attr_test);
            result = (result << 24) | (result >>  8);
            result ^= hash_test(thisAgent, cond->data.tests.value_test);
            if (cond->test_for_acceptable_preference)
            {
                result++;
            }
            break;
        case CONJUNCTIVE_NEGATION_CONDITION:
            result = 82348149;
            for (c = cond->data.ncc.top; c != NIL; c = c->next)
            {
                result ^= hash_condition(thisAgent, c);
                result = (result << 24) | (result >>  8);
            }
            break;
        default:
        {
            char msg[BUFFER_MSG_SIZE];
            strncpy(msg, "Internal error: bad cond type in hash_condition\n", BUFFER_MSG_SIZE);
            msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
            abort_with_fatal_error(thisAgent, msg);
        }
            result = 0; /* unreachable, but gcc -Wall warns without it */
        break;
    }
    return result;
}

int condition_count(condition* pCond)
{
    int cnt = 0;
    while (pCond != NULL)
    {
        ++cnt;
        pCond = pCond->next;
    }
    return cnt;
}

void add_identities_in_test(agent* thisAgent, test pTest, test pInstantiatedTest, id_set* pID_Set, id_to_idset_map_type* pID_Set_Map)
{
    if (pTest->type == CONJUNCTIVE_TEST)
    {
            pTest = pTest->eq_test;
    }
    if (pInstantiatedTest->type == CONJUNCTIVE_TEST)
    {
        pInstantiatedTest = pInstantiatedTest->eq_test;
    }
    if (test_has_referent(pTest)) {
        if (pTest->identity)
        {
            if (pID_Set->find(pTest->identity) == pID_Set->end())
            {
                pID_Set->insert(pTest->identity);
                if (pID_Set_Map)
                {
                    identity_set_info* lNewIDSet = new identity_set_info();
                    if (pTest->data.referent->is_variable())
                    {
                        lNewIDSet->identity_set_ID = thisAgent->explanationMemory->get_identity_set_counter();
                        lNewIDSet->rule_variable = pTest->data.referent;
                        symbol_add_ref(thisAgent, lNewIDSet->rule_variable);
                    } else {
                        lNewIDSet->identity_set_ID = NULL_IDENTITY_SET;
                        lNewIDSet->rule_variable = NULL;
                    }
                    pID_Set_Map->insert({pTest->identity, lNewIDSet});
                }
            }
        }
    }
}

void add_identities_in_condition_list(agent* thisAgent, condition* lhs, id_set* pID_Set, id_to_idset_map_type* pID_Set_Map)
{
    for (condition* lCond = lhs; lCond != NULL; lCond = lCond->next)
    {
        if (lCond->type == CONJUNCTIVE_NEGATION_CONDITION)
        {
            add_identities_in_condition_list(thisAgent, lCond->data.ncc.top, pID_Set, pID_Set_Map);
        } else {
            thisAgent->outputManager->set_dprint_test_format(DT_EXPLAIN_IDENTITIES, true, true);
            test id_test_without_goal_test = NULL;
            bool removed_goal_test, removed_impasse_test;
            id_test_without_goal_test = copy_test_removing_goal_impasse_tests(thisAgent, lCond->data.tests.id_test, &removed_goal_test, &removed_impasse_test);
            add_identities_in_test(thisAgent, id_test_without_goal_test, lCond->counterpart->data.tests.id_test, pID_Set, pID_Set_Map);
            add_identities_in_test(thisAgent, lCond->data.tests.attr_test, lCond->counterpart->data.tests.attr_test, pID_Set, pID_Set_Map);
            add_identities_in_test(thisAgent, lCond->data.tests.value_test, lCond->counterpart->data.tests.value_test, pID_Set, pID_Set_Map);
            deallocate_test(thisAgent, id_test_without_goal_test);
        }
    }
}
