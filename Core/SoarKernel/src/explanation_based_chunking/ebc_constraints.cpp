/*
 * variablization_manager_constraints.cpp
 *
 *  Created on: Jul 25, 2013
 *      Author: mazzin
 */

#include "ebc.h"

#include "agent.h"
#include "instantiation.h"
#include <assert.h>
#include "test.h"
#include "working_memory.h"
#include "print.h"
#include "condition.h"
#include "dprint.h"
#include "explanation_memory.h"

void Explanation_Based_Chunker::clear_cached_constraints()
{
    for (std::list< constraint* >::iterator it = constraints->begin(); it != constraints->end(); ++it)
    {
        /* We intentionally used the tests in the conditions backtraced through instead of copying
         * them, so we don't need to deallocate the tests in the constraint. We just delete the
         * constraint struct that contains the two pointers.*/
        thisAgent->memoryManager->free_with_pool(MP_constraints, *it);
    }
    constraints->clear();
}

void Explanation_Based_Chunker::cache_constraints_in_test(test t)
{
    /* Only conjunctive tests can have relational tests here */
    if (t->type != CONJUNCTIVE_TEST) return;

    test ctest;
    constraint* new_constraint = NULL;

    for (cons* c = t->data.conjunct_list; c != NIL; c = c->rest)
    {
        ctest = static_cast<test>(c->first);
        if (test_can_be_transitive_constraint(ctest))
        {
            assert(t->eq_test);
            thisAgent->memoryManager->allocate_with_pool(MP_constraints, &new_constraint);
            new_constraint->eq_test = t->eq_test;
            new_constraint->constraint_test = ctest;
            dprint(DT_CONSTRAINTS, "Caching constraints on %t [%g]: %t [%g]\n", new_constraint->eq_test, new_constraint->eq_test, new_constraint->constraint_test, new_constraint->constraint_test);
            constraints->push_back(new_constraint);
            thisAgent->explanationMemory->increment_stat_constraints_collected();
        }
    }
}

void Explanation_Based_Chunker::cache_constraints_in_cond(condition* c)
{
    if (!ebc_settings[SETTING_EBC_LEARNING_ON]) return;
    dprint(DT_CONSTRAINTS, "Caching relational constraints in condition: %l\n", c);
    cache_constraints_in_test(c->data.tests.id_test);
    cache_constraints_in_test(c->data.tests.attr_test);
    cache_constraints_in_test(c->data.tests.value_test);
}


attachment_point* Explanation_Based_Chunker::get_attachment_point(uint64_t pO_id)
{
    std::unordered_map< uint64_t, attachment_point* >::iterator it = (*attachment_points).find(pO_id);
    if (it != (*attachment_points).end())
    {
        return it->second;
    }
    return 0;
}

bool Explanation_Based_Chunker::has_positive_condition(uint64_t pO_id)
{
    std::unordered_map< uint64_t, attachment_point* >::iterator it = (*attachment_points).find(pO_id);
    return (it != (*attachment_points).end());
}

void Explanation_Based_Chunker::set_attachment_point(uint64_t pO_id, condition* pCond, WME_Field pField)
{
    std::unordered_map< uint64_t, attachment_point* >::iterator it = (*attachment_points).find(pO_id);
    if (it != (*attachment_points).end())
    {
        dprint(DT_CONSTRAINTS, "Skipping because existing attachment already exists: %u -> %s of %l\n", it->first, field_to_string(it->second->field), it->second->cond);
        return;
    }

    dprint(DT_CONSTRAINTS, "Recording attachment point: %u -> %s of %l\n", pO_id, field_to_string(pField), pCond);
    attachment_point* new_attachment;
    thisAgent->memoryManager->allocate_with_pool(MP_attachments, &new_attachment);
    new_attachment->cond = pCond;
    new_attachment->field = pField;
    (*attachment_points)[pO_id] = new_attachment;
}

void Explanation_Based_Chunker::find_attachment_points(condition* pCond)
{
    dprint_header(DT_CONSTRAINTS, PrintBefore, "Scanning conditions for constraint attachment points...\n%1", pCond);

    while (pCond)
    {
        if (pCond->type == POSITIVE_CONDITION)
        {
            dprint(DT_CONSTRAINTS, "Adding attachment points for positive condition %l\n", pCond);
            test lTest = pCond->data.tests.value_test->eq_test;
            if (lTest && lTest->identity)
            {
                set_attachment_point(lTest->identity, pCond, VALUE_ELEMENT);
            }
            lTest = pCond->data.tests.attr_test->eq_test;
            if (lTest && lTest->identity)
            {
                set_attachment_point(lTest->identity, pCond, ATTR_ELEMENT);
            }
        }
        pCond = pCond->next;
    }
    dprint_header(DT_CONSTRAINTS, PrintAfter, "Done scanning conditions for attachment points.\n");
}

void Explanation_Based_Chunker::invert_relational_test(test* pEq_test, test* pRelational_test)
{
    assert(test_has_referent(*pEq_test));
    assert(test_has_referent(*pRelational_test));
    assert((*pEq_test)->type == EQUALITY_TEST);
    assert((*pRelational_test)->type != EQUALITY_TEST);

    TestType tt = (*pRelational_test)->type;
    if (tt == NOT_EQUAL_TEST)
    {
        (*pEq_test)->type = NOT_EQUAL_TEST;
    }
    else if (tt == LESS_TEST)
    {
        (*pEq_test)->type = GREATER_TEST;
    }
    else if (tt == GREATER_TEST)
    {
        (*pEq_test)->type = LESS_TEST;
    }
    else if (tt == LESS_OR_EQUAL_TEST)
    {
        (*pEq_test)->type = GREATER_OR_EQUAL_TEST;
    }
    else if (tt == GREATER_OR_EQUAL_TEST)
    {
        (*pEq_test)->type = LESS_OR_EQUAL_TEST;
    }
    else if (tt == SAME_TYPE_TEST)
    {
        (*pEq_test)->type = SAME_TYPE_TEST;
    }
    (*pRelational_test)->type = EQUALITY_TEST;

    test temp = *pEq_test;
    (*pEq_test) = (*pRelational_test);
    (*pRelational_test) = temp;

}

void Explanation_Based_Chunker::attach_relational_test(test pEq_test, test pRelational_test)
{
    dprint(DT_CONSTRAINTS, "Attempting to attach %t(o%u) %t(o%u).\n", pRelational_test, pRelational_test->identity, pEq_test, pEq_test->identity);
    attachment_point* attachment_info = get_attachment_point(pEq_test->identity);
    if (attachment_info)
    {
        if (attachment_info->field == VALUE_ELEMENT)
        {
            add_test(thisAgent, &(attachment_info->cond->data.tests.value_test), pRelational_test, true);
        } else if (attachment_info->field == ATTR_ELEMENT)
        {
            add_test(thisAgent, &(attachment_info->cond->data.tests.attr_test), pRelational_test, true);
        } else
        {
            add_test(thisAgent, &(attachment_info->cond->data.tests.id_test), pRelational_test, true);
        }
        thisAgent->explanationMemory->increment_stat_constraints_attached();
        return;
    }
    dprint(DT_CONSTRAINTS, "Did not find attachment point!\n");
    assert(false);
}

void Explanation_Based_Chunker::prune_redundant_constraints()
{
    dprint(DT_CONSTRAINTS, "Pruning redundant constraints from set of size %u.\n", static_cast<uint64_t>(constraints->size()));
    for (std::list< constraint* >::iterator iter = constraints->begin(); iter != constraints->end();)
    {
        if ((*iter)->constraint_test->tc_num == tc_num_found)
        {
            iter = constraints->erase(iter);
        }
        else
        {
            ++iter;
        }
    }
    dprint(DT_CONSTRAINTS, "Final pruned constraints is a set of size %u.\n", static_cast<uint64_t>(constraints->size()));
}

void Explanation_Based_Chunker::add_additional_constraints()
{
    if (!ebc_settings[SETTING_EBC_LEARNING_ON]) return;

    constraint* lConstraint = NULL;
    test eq_copy = NULL, constraint_test = NULL;

    dprint_header(DT_CONSTRAINTS, PrintBefore, "Adding additional constraints...\n");

    /* Most constraints should already be in a chunk condition.  We marked
     * them with a tc_num as they were copied from the grounds to the
     * chunk condition, so that we can prune them from the list here. */

    prune_redundant_constraints();
    if (constraints->empty())
    {
        dprint_header(DT_CONSTRAINTS, PrintAfter, "All constraints already in chunk conditions.  Done propagating additional constraints.\n");
        return;
    }

    find_attachment_points(m_lhs);
    for (std::list< constraint* >::iterator iter = constraints->begin(); iter != constraints->end(); ++iter)
    {
        lConstraint = *iter;
        constraint_test = copy_test(thisAgent, lConstraint->constraint_test, true);
        eq_copy = copy_test(thisAgent, lConstraint->eq_test, true);

        dprint(DT_CONSTRAINTS, "...unattached test found: %t[%g] %t[%g]\n", eq_copy, eq_copy, constraint_test, constraint_test);

        if (eq_copy->identity && has_positive_condition(eq_copy->identity))
        {
            /* Attach to a positive chunk condition test of eq_test */
            dprint(DT_CONSTRAINTS, "...equality test has an identity, so attaching.\n");
            attach_relational_test(eq_copy, constraint_test);
        } else {
            /* Original identity constraint was attached to was literalized */
            if (constraint_test->identity && has_positive_condition(constraint_test->identity))
            {
                /* Relational tests referent was not literalized, so make complement and
                 * add to a positive chunk condition test for the referent */
                dprint(DT_CONSTRAINTS, "...equality test is a literal but referent has identity, so attaching complement to referent.\n");
                invert_relational_test(&eq_copy, &constraint_test);
                attach_relational_test(eq_copy, constraint_test);

            } else {
                // Both tests are literals.  Delete.
                dprint(DT_CONSTRAINTS, "...both tests are literals.  Oh my god.\n");
                deallocate_test(thisAgent, constraint_test);
            }
        }
        /* eq_test no longer needed so deallocate.  relational test now attached */
        deallocate_test(thisAgent, eq_copy);
    }
    dprint_header(DT_CONSTRAINTS, PrintAfter, "Done propagating additional constraints.\n");
}
