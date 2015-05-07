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
        dprint(DT_CONSTRAINTS, "...did not find attachment point for %y(o%u)!\n",
            get_ovar_for_o_id(it->first), it->first);
        print_o_id_update_map(DT_VM_MAPS);
    }
    return 0;
}

void Variablization_Manager::set_attachment_point(uint64_t pO_id, condition* pCond, WME_Field pField)
{
    std::map< uint64_t, attachment_point* >::iterator it = (*attachment_points).find(pO_id);
    if (it == (*attachment_points).end())
    {
        dprint(DT_CONSTRAINTS, "Skipping because existing attachment already exists: %y(o%u) -> %s of %l\n",
            get_ovar_for_o_id(it->first), it->first, field_to_string(it->second->field), it->second->cond);
        return;
    }

    dprint(DT_CONSTRAINTS, "Recording attachment point: %y(o%u) -> %s of %l\n",
        get_ovar_for_o_id(pO_id), pO_id, field_to_string(pField), pCond);
    (*attachment_points)[pO_id] = new attachment_point(pCond, pField);;
}

void Variablization_Manager::find_attachment_point_for_test(test pTest, condition* pCond, WME_Field pField)
{
    test lTest = equality_test_found_in_test(pTest);
    if (lTest && lTest->identity->o_id)
    {
        set_attachment_point(lTest->identity->o_id, pCond, pField);
    }
}

void Variablization_Manager::find_attachment_points(condition* pCond)
{
    dprint_header(DT_CONSTRAINTS, PrintBefore, "Scanning conditions for constraint attachment points...\n%1", pCond);

    while (pCond)
    {
        if (pCond->type == POSITIVE_CONDITION)
        {
            dprint(DT_CONSTRAINTS, "Adding attachment points for positive condition %l\n", pCond);
            find_attachment_point_for_test(pCond->data.tests.attr_test, pCond, ATTR_ELEMENT);
            find_attachment_point_for_test(pCond->data.tests.value_test, pCond, VALUE_ELEMENT);
        }
        else
        {
            dprint(DT_CONSTRAINTS, (pCond->type == NEGATIVE_CONDITION) ?
                "Skipping for negative condition %l\n" :
                "Skipping for negative conjunctive condition:\n%l", pCond);
        }
        pCond = pCond->next;
    }
    dprint_header(DT_CONSTRAINTS, PrintAfter, "Done scanning conditions for attachment points.");
}

void Variablization_Manager::invert_relational_test(test* pEq_test, test* pRelational_test)
{
    assert(test_has_referent(*pEq_test));
    assert(test_has_referent(*pRelational_test));
    assert((*pEq_test)->type != EQUALITY_TEST);
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
    pEq_test = pRelational_test;
    (*pRelational_test) = temp;

}

void Variablization_Manager::attach_relational_test(test pEq_test, test pRelational_test, uint64_t pI_id)
{
    o_id_update_info* new_o_id_info = get_updated_o_id_info(pEq_test->identity->o_id);
    if (new_o_id_info)
    {
        assert(new_o_id_info->positive_cond);
        if (new_o_id_info->field == VALUE_ELEMENT)
        {
            add_test(thisAgent, &(new_o_id_info->positive_cond->data.tests.value_test), pRelational_test);
        } else if (new_o_id_info->field == ATTR_ELEMENT)
        {
            add_test(thisAgent, &(new_o_id_info->positive_cond->data.tests.attr_test), pRelational_test);
        } else
        {
            assert(false);
            add_test(thisAgent, &(new_o_id_info->positive_cond->data.tests.id_test), pRelational_test);
        }
//        dprint(DT_CONSTRAINTS, "...found test to attach to %t[%g]\n", attach_test, attach_test);
//        new_o_id_info = get_updated_o_id_info(pRelational_test->identity->o_id);
//        if (new_o_id_info)
//        {
//        }
//        thisAgent->variablizationManager->update_o_id_for_new_instantiation(&(new_ct->identity->rule_symbol), &(new_ct->identity->o_id), pI_id, t);
        return;
    }
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

void Variablization_Manager::add_additional_constraints(condition* cond, uint64_t pI_id)
{
    constraint* lConstraint = NULL;
    test eq_copy = NULL, constraint_test = NULL;

    dprint_header(DT_CONSTRAINTS, PrintBefore, "Propagating additional constraints...\n");

    prune_redundant_constraints();
    if (constraints->empty())
    {
        dprint_header(DT_CONSTRAINTS, PrintAfter, "All constraints already in chunk conditions.  Done propagating additional constraints.\n");
        return;
    }

    return;
    find_attachment_points(cond);

    for (std::list< constraint* >::iterator iter = constraints->begin(); iter != constraints->end(); ++iter)
    {
        lConstraint = *iter;
        if (lConstraint->constraint_test->tc_num != tc_num_found)
        {
            constraint_test = copy_test(thisAgent, lConstraint->constraint_test);
            eq_copy = copy_test(thisAgent, lConstraint->eq_test);
            if (eq_copy->identity->o_id)
            {
                thisAgent->variablizationManager->unify_identity(thisAgent, eq_copy);
            }
            if (eq_copy->identity->o_id)
            {
                thisAgent->variablizationManager->unify_identity(thisAgent, eq_copy);
            }

//            new_test = copy_test(thisAgent, lConstraint->constraint_test, true, pI_id);
//            eq_copy = copy_test(thisAgent, lConstraint->eq_test, true, pI_id);
//            update_o_id_for_new_instantiation(&(constraint_test->identity->rule_symbol), &(constraint_test->identity->o_id), pI_id, NULL, true);
//            update_o_id_for_new_instantiation(&(eq_copy->identity->rule_symbol), &(eq_copy->identity->o_id), pI_id, NULL, true);
            dprint(DT_CONSTRAINTS, "...unattached test found: %t[%g] %t[%g]\n", eq_copy, eq_copy, constraint_test, constraint_test);

            if (eq_copy->identity->o_id)
            {
                /* Attach to a positive chunk condition test of eq_test */
                dprint(DT_CONSTRAINTS, "...equality test has an identity, so attaching.\n");
                attach_relational_test(eq_copy, constraint_test, pI_id);
            } else {
                if (constraint_test->identity->o_id)
                {
                    /* Original identity it was attached to was literalized, but the relational
                     * referent was not, so make complement and add to a positive chunk
                     * condition test for the referent */
                    dprint(DT_CONSTRAINTS, "...equality test is a literal but referent has identity, so attaching complement to referent.\n");
                    invert_relational_test(&eq_copy, &constraint_test);
                    attach_relational_test(eq_copy, constraint_test, pI_id);

                } else {
                    // Both tests are literals.  Delete.
                    dprint(DT_CONSTRAINTS, "...both tests are literals.  Oh my god.\n");
                    deallocate_test(thisAgent, constraint_test);
                    // Is this possible?
                    assert(false);
                }
            }
            /* eq_test no longer needed so deallocate.  relational test now attached */
            deallocate_test(thisAgent, eq_copy);
        } else {
            // Skipping test because marked as already in a chunk condition
            dprint(DT_CONSTRAINTS, "Skipping test b/c marked as already in chunk %g %g...\n", lConstraint->eq_test, lConstraint->constraint_test);
        }
    }
    dprint_header(DT_CONSTRAINTS, PrintAfter, "Done propagating additional constraints.\n");
}
