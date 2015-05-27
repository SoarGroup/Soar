/*
 * variablization_manager_merge.cpp
 *
 *  Created on: Jul 25, 2013
 *      Author: mazzin
 */

#include "variablization_manager.h"
#include "agent.h"
#include "instantiations.h"
#include "prefmem.h"
#include "assert.h"
#include "test.h"
#include "print.h"
#include "debug.h"

void Variablization_Manager::remove_ungrounded_sti_tests(test* t, bool ignore_ungroundeds)
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
            if (!ignore_ungroundeds && test_has_referent(tt) && (tt->data.referent->is_sti()))
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

void Variablization_Manager::fix_conditions(condition* top_cond, uint64_t pI_id, bool ignore_ungroundeds)
{
    dprint_header(DT_FIX_CONDITIONS, PrintBoth, "= Fixing conditions =\n");
    dprint_set_indents(DT_FIX_CONDITIONS, "          ");
    dprint_noprefix(DT_FIX_CONDITIONS, "%1", top_cond);
    dprint_clear_indents(DT_FIX_CONDITIONS);
    dprint_header(DT_FIX_CONDITIONS, PrintAfter, "");

    condition* next_cond, *last_cond = NULL;
    for (condition* cond = top_cond; cond;)
    {
        dprint(DT_FIX_CONDITIONS, "Finding redundancies in condition: %l\n", cond);
        next_cond = cond->next;
        if (cond->type != CONJUNCTIVE_NEGATION_CONDITION)
        {
            remove_ungrounded_sti_tests(&(cond->data.tests.id_test), ignore_ungroundeds);
            remove_ungrounded_sti_tests(&(cond->data.tests.attr_test), ignore_ungroundeds);
            remove_ungrounded_sti_tests(&(cond->data.tests.value_test), ignore_ungroundeds);
        }
        else
        {
            /* MToDo | Check if we need for NCCs.  It could be possible to get ungroundeds in NCCs */
        }
        last_cond = cond;
        cond = next_cond;
    }

    dprint_header(DT_FIX_CONDITIONS, PrintBefore, "");
    dprint_set_indents(DT_FIX_CONDITIONS, "          ");
    dprint_noprefix(DT_FIX_CONDITIONS, "%1", top_cond);
    dprint_clear_indents(DT_FIX_CONDITIONS);
    dprint_header(DT_FIX_CONDITIONS, PrintBoth, "= Done fixing conditions =\n");
}

void Variablization_Manager::fix_results(preference* result, uint64_t pI_id)
{

    if (!result) return;

    dprint(DT_FIX_CONDITIONS, "Fixing result %p\n", result);
//    print_o_id_substitution_map(DT_FIX_CONDITIONS);
//    print_o_id_update_map(DT_FIX_CONDITIONS);
    print_o_id_tables(DT_FIX_CONDITIONS);

    if (pI_id)
    {
        if (result->o_ids.id)
        {
            unify_identity_for_result_element(thisAgent, result, ID_ELEMENT);
        }
        if (result->o_ids.attr)
        {
            unify_identity_for_result_element(thisAgent, result, ATTR_ELEMENT);
        }
        if (result->o_ids.value)
        {
            unify_identity_for_result_element(thisAgent, result, VALUE_ELEMENT);
        }
    }
    fix_results(result->next_result, pI_id);
    /* MToDo | Do we need to fix o_ids in clones too? */
}
