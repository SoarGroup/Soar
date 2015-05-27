/*
 * variablization_manager_constraints.cpp
 *
 *  Created on: Jul 25, 2013
 *      Author: mazzin
 */

#include "variablization_manager.h"
#include "agent.h"
#include "instantiations.h"
#include "assert.h"
#include "test.h"
#include "wmem.h"
#include "print.h"
#include "debug.h"

void Variablization_Manager::clear_cached_constraints()
{
    for (std::list< constraint* >::iterator it = constraints->begin(); it != constraints->end(); ++it)
    {
        /* We intentionally used the tests in the conditions backtraced through instead of copying
         * them, so we don't need to deallocate the tests in the constraint. We just delete the
         * constraint struct that contains the two pointers.*/
        delete *it;
    }
    constraints->clear();
}

void Variablization_Manager::cache_constraints_in_test(test t)
{
    /* -- Only conjunctive tests can have relational tests here.  Otherwise,
     *    should be an equality test. -- */
    if (t->type != CONJUNCTIVE_TEST)
    {
        assert(t->type == EQUALITY_TEST);
        return;
    }

    test equality_test = NULL, referent_test, ctest;
    cons* c;
    for (c = t->data.conjunct_list; c != NIL; c = c->rest)
    {
        if (static_cast<test>(c->first)->type == EQUALITY_TEST)
        {
            equality_test = static_cast<test>(c->first);
            break;
        }
    }
    assert(equality_test);
    constraint* new_constraint = NULL;
    for (c = t->data.conjunct_list; c != NIL; c = c->rest)
    {
        ctest = static_cast<test>(c->first);
        switch (ctest->type)
        {
            case GREATER_TEST:
            case GREATER_OR_EQUAL_TEST:
            case LESS_TEST:
            case LESS_OR_EQUAL_TEST:
            case NOT_EQUAL_TEST:
            case SAME_TYPE_TEST:
            case DISJUNCTION_TEST:
                new_constraint = new constraint(equality_test, ctest);
                constraints->push_back(new_constraint);
                break;
            default:
                break;
        }
    }
}

void Variablization_Manager::cache_constraints_in_cond(condition* c)
{
    /* MToDo| Verify we don't need to do id element.  It should always be an equality test */
    //  assert(!c->data.tests.id_test || (c->data.tests.id_test->type == EQUALITY_TEST));
    dprint(DT_CONSTRAINTS, "Caching relational constraints in condition: %l\n", c);
    /* MToDo| Re-enable attribute constraint caching here.  Disabled just to simplify debugging for now */
    cache_constraints_in_test(c->data.tests.attr_test);
    cache_constraints_in_test(c->data.tests.value_test);
}


attachment_point* Variablization_Manager::get_attachment_point(uint64_t pO_id)
{
    std::map< uint64_t, attachment_point* >::iterator it = (*attachment_points).find(pO_id);
    if (it != (*attachment_points).end())
    {
        dprint(DT_CONSTRAINTS, "...found attachment point: %y(o%u) -> %s of %l\n",
            get_ovar_for_o_id(it->first), it->first, field_to_string(it->second->field), it->second->cond);

        return it->second;
    } else {
        dprint(DT_CONSTRAINTS, "...did not find attachment point for %y(o%u)!\n", get_ovar_for_o_id(pO_id), pO_id);
        print_attachment_points(DT_CONSTRAINTS);
    }
    return 0;
}

bool Variablization_Manager::has_positive_condition(uint64_t pO_id)
{
    std::map< uint64_t, attachment_point* >::iterator it = (*attachment_points).find(pO_id);
    if (it != (*attachment_points).end())
    {
        dprint(DT_CONSTRAINTS, "...found positive condition, returning true: %y(o%u) -> %s of %l\n",
            get_ovar_for_o_id(it->first), it->first, field_to_string(it->second->field), it->second->cond);

        return true;
    } else {
        dprint(DT_CONSTRAINTS, "...did not find positive condition, returning false for %y(o%u)!\n", get_ovar_for_o_id(pO_id), pO_id);
//        print_attachment_points(DT_CONSTRAINTS);
//        print_o_id_update_map(DT_CONSTRAINTS);
    }
    return false;
}

void Variablization_Manager::set_attachment_point(uint64_t pO_id, condition* pCond, WME_Field pField)
{
    std::map< uint64_t, attachment_point* >::iterator it = (*attachment_points).find(pO_id);
    if (it != (*attachment_points).end())
    {
        dprint(DT_CONSTRAINTS, "Skipping because existing attachment already exists: %y(o%u) -> %s of %l\n",
            get_ovar_for_o_id(it->first), it->first, field_to_string(it->second->field), it->second->cond);
        return;
    }

    dprint(DT_CONSTRAINTS, "Recording attachment point: %y(o%u) -> %s of %l\n",
        get_ovar_for_o_id(pO_id), pO_id, field_to_string(pField), pCond);
    (*attachment_points)[pO_id] = new attachment_point(pCond, pField);;
}

void Variablization_Manager::find_attachment_points(condition* pCond)
{
    dprint_header(DT_CONSTRAINTS, PrintBefore, "Scanning conditions for constraint attachment points...\n%1", pCond);

    while (pCond)
    {
        if (pCond->type == POSITIVE_CONDITION)
        {
            dprint(DT_CONSTRAINTS, "Adding attachment points for positive condition %l\n", pCond);
            test lTest = equality_test_found_in_test(pCond->data.tests.value_test);
            if (lTest && lTest->identity)
            {
                set_attachment_point(lTest->identity, pCond, VALUE_ELEMENT);
            }
            lTest = equality_test_found_in_test(pCond->data.tests.attr_test);
            if (lTest && lTest->identity)
            {
                set_attachment_point(lTest->identity, pCond, ATTR_ELEMENT);
            }
        }
        else
        {
            dprint(DT_CONSTRAINTS, (pCond->type == NEGATIVE_CONDITION) ?
                "Skipping for negative condition %l\n" :
                "Skipping for negative conjunctive condition:\n%l", pCond);
        }
        pCond = pCond->next;
    }
    dprint_header(DT_CONSTRAINTS, PrintAfter, "Done scanning conditions for attachment points.\n");
}

void Variablization_Manager::invert_relational_test(test* pEq_test, test* pRelational_test)
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

void Variablization_Manager::attach_relational_test(test pEq_test, test pRelational_test)
{
    dprint(DT_CONSTRAINTS, "Attempting to attach %t(o%u) %t(o%u).\n", pRelational_test, pRelational_test->identity, pEq_test, pEq_test->identity);
    attachment_point* attachment_info = get_attachment_point(pEq_test->identity);
    if (attachment_info)
    {
        dprint(DT_CONSTRAINTS, "Found attachment point in condition %l.\n", attachment_info->cond);
        assert(attachment_info->cond);
        if (attachment_info->field == VALUE_ELEMENT)
        {
            add_test(thisAgent, &(attachment_info->cond->data.tests.value_test), pRelational_test);
        } else if (attachment_info->field == ATTR_ELEMENT)
        {
            add_test(thisAgent, &(attachment_info->cond->data.tests.attr_test), pRelational_test);
        } else
        {
            assert(false);
            add_test(thisAgent, &(attachment_info->cond->data.tests.id_test), pRelational_test);
        }
        return;
    }
    dprint(DT_CONSTRAINTS, "Did not find attachment point!\n");
    assert(false);
}

void Variablization_Manager::prune_redundant_constraints()
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

void Variablization_Manager::add_additional_constraints(condition* cond)
{
    constraint* lConstraint = NULL;
    test eq_copy = NULL, constraint_test = NULL;

    dprint_header(DT_CONSTRAINTS, PrintBefore, "Propagating additional constraints...\n");

    /* Most constraints should already be in a chunk condition.  We marked
     * them with a tc_num as they were copied from the grounds to the
     * chunk condition, so that we can prune them from the list here. */

    prune_redundant_constraints();
    if (constraints->empty())
    {
        dprint_header(DT_CONSTRAINTS, PrintAfter, "All constraints already in chunk conditions.  Done propagating additional constraints.\n");
        return;
    }

    find_attachment_points(cond);
    print_attachment_points(DT_CONSTRAINTS);

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

void Variablization_Manager::remove_ungrounded_sti_from_test_and_cache_eq_test(test* t)
{
    assert(t);

    if ((*t)->type == EQUALITY_TEST)
    {
        /* Cache main equality test.  Clearly redundant for an equality test, but simplifies code. */
        (*t)->eq_test = (*t);
        return;
    }
    else if ((*t)->type == CONJUNCTIVE_TEST)
    {
        test tt, found_eq_test;
        ::list* c = (*t)->data.conjunct_list;
        while (c)
        {
            tt = static_cast<test>(c->first);

            // For all tests, check if referent is STI.  If so, it's ungrounded.  Delete.
            if (test_has_referent(tt) && (tt->data.referent->is_sti()))
            {
                dprint(DT_FIX_CONDITIONS, "Ungrounded STI found: %y\n", tt->data.referent);
                c = delete_test_from_conjunct(thisAgent, t, c);
                dprint(DT_FIX_CONDITIONS, "          ...after deletion: %t [%g]\n", (*t), (*t));
            }
            else if (tt->type == EQUALITY_TEST)
            {
                found_eq_test = tt;
                c = c->rest;
            }
            else
            {
                c = c->rest;
            }
        }

        /* -- We also cache the main equality test for each element in each condition
         *    since it's easy to do here. This allows us to avoid searching conjunctive
         *    tests repeatedly during merging. -- */
        (*t)->eq_test = found_eq_test;
        found_eq_test->eq_test = found_eq_test;
    }
}

void Variablization_Manager::remove_ungrounded_sti_constraints_and_cache_eq_tests(condition* top_cond)
{
    dprint_header(DT_FIX_CONDITIONS, PrintBoth, "= Removing constraints with ungrounded STIs as referents and caching equality tests for merging =\n%1", top_cond);

    condition* next_cond, *last_cond = NULL;
    for (condition* cond = top_cond; cond;)
    {
        dprint(DT_FIX_CONDITIONS, "Processing condition: %l\n", cond);
        next_cond = cond->next;
        if (cond->type != CONJUNCTIVE_NEGATION_CONDITION)
        {
            remove_ungrounded_sti_from_test_and_cache_eq_test(&(cond->data.tests.id_test));
            remove_ungrounded_sti_from_test_and_cache_eq_test(&(cond->data.tests.attr_test));
            remove_ungrounded_sti_from_test_and_cache_eq_test(&(cond->data.tests.value_test));
        }
        else
        {
            /* MToDo | Check if we need for NCCs.  It could be possible to get ungroundeds in NCCs */
        }
        last_cond = cond;
        cond = next_cond;
    }

    dprint_header(DT_FIX_CONDITIONS, PrintBoth, "= Done removing constraints with ungrounded STIs as referents and caching equality tests for merging =\n%1", top_cond);
}
