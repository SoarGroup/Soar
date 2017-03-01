/*
 * variablization_manager_constraints.cpp
 *
 *  Created on: Jul 25, 2013
 *      Author: mazzin
 */

#include "ebc.h"

#include "agent.h"
#include "condition.h"
#include "dprint.h"
#include "explanation_memory.h"
#include "instantiation.h"
#include "print.h"
#include "test.h"
#include "working_memory.h"

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
    test ctest;
    constraint* new_constraint = NULL;

    for (cons* c = t->data.conjunct_list; c != NIL; c = c->rest)
    {
        ctest = static_cast<test>(c->first);
        if (test_can_be_transitive_constraint(ctest))
        {
            assert(t->eq_test->identity_set);
//            push(thisAgent, ctest, t->eq_test->identity_set->super_join->constraints);

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
    dprint(DT_CONSTRAINTS, "Caching relational constraints in condition: %l\n", c);
    if (c->data.tests.id_test->type == CONJUNCTIVE_TEST) cache_constraints_in_test(c->data.tests.id_test);
    if (c->data.tests.attr_test->type == CONJUNCTIVE_TEST) cache_constraints_in_test(c->data.tests.attr_test);
    if (c->data.tests.value_test->type == CONJUNCTIVE_TEST) cache_constraints_in_test(c->data.tests.value_test);
}

void Explanation_Based_Chunker::invert_relational_test(test* pEq_test, test* pRelational_test)
{
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

void Explanation_Based_Chunker::attach_relational_test(test pRelational_test, condition* pCond, WME_Field pField)
{
    dprint(DT_CONSTRAINTS, "Attaching transitive constraint %t %g to field %d of %l.\n", pRelational_test, pRelational_test, static_cast<int64_t>(pField), pCond);
    if (pField == VALUE_ELEMENT)
    {
        add_test(thisAgent, &(pCond->data.tests.value_test), pRelational_test, true);
    } else if (pField == ATTR_ELEMENT)
    {
        add_test(thisAgent, &(pCond->data.tests.attr_test), pRelational_test, true);
    } else
    {
        add_test(thisAgent, &(pCond->data.tests.id_test), pRelational_test, true);
    }
    thisAgent->explanationMemory->increment_stat_constraints_attached();
}

test get_test_in_field_of_cond (condition* pCond, WME_Field pField)
{
    if (pField == VALUE_ELEMENT) return pCond->data.tests.value_test;
    else if (pField == ATTR_ELEMENT) return pCond->data.tests.attr_test;
    else return pCond->data.tests.id_test;
}

/* pTest must be the overall conjunctive test */
void Explanation_Based_Chunker::add_constraint_to_test(test pConstraint, identity_set* pDestinationID_Set)
{
    test eq_copy = NULL, constraint_test = NULL;

    dprint(DT_CONSTRAINTS, "Attempting to add constraint %t %g to identity set %u: ", pConstraint, pConstraint, pDestinationID_Set->identity);
    if (!pDestinationID_Set->super_join->literalized)
    {
        assert(pDestinationID_Set->super_join->operational_cond);
        constraint_test = copy_test(thisAgent, pConstraint, true);

        attach_relational_test(pConstraint, pDestinationID_Set->super_join->operational_cond, pDestinationID_Set->super_join->operational_field);
    }
    else if (pConstraint->identity_set && !pConstraint->identity_set->super_join->literalized)
    {
        eq_copy = copy_test(thisAgent,get_test_in_field_of_cond(pDestinationID_Set->super_join->operational_cond, pDestinationID_Set->super_join->operational_field), true);
        constraint_test = copy_test(thisAgent, pConstraint, true);
        invert_relational_test(&eq_copy, &constraint_test);
        if (eq_copy->identity_set->super_join->operational_cond)
        {
            attach_relational_test(constraint_test, eq_copy->identity_set->super_join->operational_cond, eq_copy->identity_set->super_join->operational_field);
            thisAgent->explanationMemory->increment_stat_constraints_attached();
        }
        else
        {
            push(thisAgent, constraint_test, eq_copy->identity_set->super_join->inverted_constraints);
        }
//        deallocate_test(thisAgent, eq_copy);
        dprint(DT_CONSTRAINTS, "...complement of constraint added.  Condition is now %l\n", pConstraint->identity_set->super_join->operational_cond);
    } else {
        dprint(DT_CONSTRAINTS, "...did not add constraint:\n    identity set %u, literalized = %s\n    reltest: %t %g, literalized = %s\n",
            pDestinationID_Set->identity, (pDestinationID_Set->super_join->literalized) ? "true" : "false",
                pConstraint, pConstraint, (pConstraint->identity_set && pConstraint->identity_set->super_join->literalized) ? "true" : "false");
    }
}

void Explanation_Based_Chunker::add_additional_constraints()
{
    constraint* lConstraint = NULL;
    test eq_copy = NULL, constraint_test = NULL;

    dprint_header(DT_CONSTRAINTS, PrintBefore, "Adding %u transitive constraints from non-operational conditions...\n", static_cast<uint64_t>(constraints->size()));

    for (std::list< constraint* >::iterator iter = constraints->begin(); iter != constraints->end();)
    {
        lConstraint = *iter;
        dprint(DT_CONSTRAINTS, "Attempting to add constraint %t %g to %t %g: ", lConstraint->constraint_test, lConstraint->constraint_test, lConstraint->eq_test, lConstraint->eq_test);
        if (lConstraint->eq_test->identity_set && !lConstraint->eq_test->identity_set->super_join->literalized && lConstraint->eq_test->identity_set->super_join->operational_cond)
        {
            constraint_test = copy_test(thisAgent, lConstraint->constraint_test, true);
            attach_relational_test(constraint_test, lConstraint->eq_test->identity_set->super_join->operational_cond, lConstraint->eq_test->identity_set->super_join->operational_field);
            dprint(DT_CONSTRAINTS, "...constraint added.  Condition is now %l\n", lConstraint->eq_test->identity_set->super_join->operational_cond);
            thisAgent->explanationMemory->increment_stat_constraints_attached();
        }
        else if (lConstraint->constraint_test->identity_set && !lConstraint->constraint_test->identity_set->super_join->literalized && lConstraint->constraint_test->identity_set->super_join->operational_cond)
        {
            eq_copy = copy_test(thisAgent, lConstraint->eq_test, true);
            constraint_test = copy_test(thisAgent, lConstraint->constraint_test, true);
            invert_relational_test(&eq_copy, &constraint_test);
            attach_relational_test(constraint_test, lConstraint->constraint_test->identity_set->super_join->operational_cond, lConstraint->constraint_test->identity_set->super_join->operational_field);
            deallocate_test(thisAgent, eq_copy);
            dprint(DT_CONSTRAINTS, "...complement of constraint added.  Condition is now %l\n", lConstraint->constraint_test->identity_set->super_join->operational_cond);
            thisAgent->explanationMemory->increment_stat_constraints_attached();
        } else {
            dprint(DT_CONSTRAINTS, "...did not add constraint:\n    eq_test: %t %g, literalized = %s\n    reltest: %t %g, literalized = %s\n",
                lConstraint->eq_test, lConstraint->eq_test, (lConstraint->eq_test->identity_set && lConstraint->eq_test->identity_set->super_join->literalized) ? "true" : "false",
                    lConstraint->constraint_test, lConstraint->constraint_test, (lConstraint->constraint_test->identity_set && lConstraint->constraint_test->identity_set->super_join->literalized) ? "true" : "false");
        }
        ++iter;
    }
    clear_cached_constraints();

    dprint_header(DT_CONSTRAINTS, PrintAfter, "Done adding transitive constraints.\n");
}
