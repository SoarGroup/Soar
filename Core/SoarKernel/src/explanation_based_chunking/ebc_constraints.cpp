#include "ebc.h"
#include "ebc_identity.h"

#include "agent.h"
#include "condition.h"
#include "explanation_memory.h"
#include "instantiation.h"
#include "print.h"
#include "test.h"
#include "working_memory.h"

void Explanation_Based_Chunker::clear_cached_constraints()
{
    for (constraint_list::iterator it = constraints->begin(); it != constraints->end(); ++it)
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

            thisAgent->memoryManager->allocate_with_pool(MP_constraints, &new_constraint);
            new_constraint->eq_test = t->eq_test;
            new_constraint->constraint_test = ctest;
            constraints->push_back(new_constraint);
            thisAgent->explanationMemory->increment_stat_constraints_collected();
        }
    }
}

void Explanation_Based_Chunker::cache_constraints_in_cond(condition* c)
{
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

void Explanation_Based_Chunker::add_additional_constraints()
{
    constraint* lConstraint = NULL;
    test eq_copy = NULL, constraint_test = NULL;

    for (constraint_list::iterator iter = constraints->begin(); iter != constraints->end();)
    {
        lConstraint = *iter;
        condition* lOperationalCond = lConstraint->eq_test->identity ? lConstraint->eq_test->identity->get_operational_cond() : NULL;
        condition* lOperationalConstraintCond = lConstraint->constraint_test->identity ? lConstraint->constraint_test->identity->get_operational_cond() : NULL;

        if (lOperationalCond && !lConstraint->eq_test->identity->literalized())
        {
            constraint_test = copy_test(thisAgent, lConstraint->constraint_test, true);
            attach_relational_test(constraint_test, lOperationalCond, lConstraint->eq_test->identity->get_operational_field());
        }
        else if (lOperationalConstraintCond && !lConstraint->constraint_test->identity->literalized())
        {
            eq_copy = copy_test(thisAgent, lConstraint->eq_test, true);
            constraint_test = copy_test(thisAgent, lConstraint->constraint_test, true);
            invert_relational_test(&eq_copy, &constraint_test);
            attach_relational_test(constraint_test, lOperationalConstraintCond, lConstraint->constraint_test->identity->get_operational_field());
            deallocate_test(thisAgent, eq_copy);
        }
        ++iter;
    }
    clear_cached_constraints();
}
