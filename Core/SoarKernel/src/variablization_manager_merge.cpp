/*
 * variablization_manager_merge.cpp
 *
 *  Created on: Jul 25, 2013
 *      Author: mazzin
 */

#include "variablization_manager.h"
#include "agent.h"
#include "instantiations.h"
#include "assert.h"
#include "test.h"
#include "print.h"
#include "debug.h"

void Variablization_Manager::clear_merge_map()
{
    std::map< Symbol *, std::map< Symbol *, ::list *> >::iterator iter_id;
    std::map< Symbol *, ::list *>::iterator iter_attr;
    std::map< Symbol *, ::list *> *attr_values;

    for (iter_id = cond_merge_map->begin(); iter_id != cond_merge_map->end(); ++iter_id)
    {
        attr_values = &(iter_id->second);
        for (iter_attr = attr_values->begin(); iter_attr != attr_values->end(); ++iter_attr)
        {
            free_list (thisAgent, iter_attr->second);
        }
        attr_values->clear();
    }
    cond_merge_map->clear();
}

void Variablization_Manager::merge_values_in_conds(condition *pDestCond, condition *pSrcCond)
{
    add_non_identical_tests(thisAgent, &(pDestCond->data.tests.value_test), pSrcCond->data.tests.value_test);
}

void Variablization_Manager::set_cond_for_id_attr_tests(condition *pCond)
{
    std::map< Symbol *, std::map< Symbol *, ::list *> >::iterator iter_id;
    std::map< Symbol *, ::list *>::iterator iter_attr;
    ::list * new_list=NULL;

    dprint_condition(DT_MERGE, pCond, "Savind cond in merge map: ", true, false, true);
    test id_test = equality_test_found_in_test(thisAgent, pCond->data.tests.id_test);
    test attr_test = equality_test_found_in_test(thisAgent, pCond->data.tests.attr_test);
    test val_test = equality_test_found_in_test(thisAgent, pCond->data.tests.value_test);
    dprint(DT_MERGE, "...found equality tests (%s ^%s %s)\n", id_test->data.referent->to_string(), attr_test->data.referent->to_string(), val_test->data.referent->to_string());
    iter_id = cond_merge_map->find(id_test->data.referent);
    if (iter_id == cond_merge_map->end())
    {
        dprint(DT_MERGE, "...id test not found.  Creating new entry.\n");
        /* Add new attr->value-cons-list map */
        push(thisAgent, pCond, new_list);
        std::map<Symbol *, ::list *> inner;
        inner.insert(std::make_pair(attr_test->data.referent, new_list));
        cond_merge_map->insert(std::make_pair(id_test->data.referent, inner));
        dprint(DT_CONSTRAINTS, "ADDED (*cond_merge_map)[%s][%s] -> new_list (1 entry)\n", id_test->data.referent->to_string(), attr_test->data.referent->to_string());
    } else {
        dprint(DT_MERGE, "...id test found.  Looking for attribute test...\n");
        iter_attr = iter_id->second.find(attr_test->data.referent);
        if (iter_attr == iter_id->second.end())
        {
            dprint(DT_MERGE, "...attr test not found.  Creating new entry.\n");
            push(thisAgent, pCond, new_list);
            (*cond_merge_map)[id_test->data.referent][attr_test->data.referent] = new_list;
            dprint(DT_CONSTRAINTS, "ADDED (*cond_merge_map)[%s][%s] -> new_list (1 entry)\n", id_test->data.referent->to_string(), attr_test->data.referent->to_string());
        } else {
            dprint(DT_MERGE, "...attr test found.  Creating new entry.\n");
            new_list = (*cond_merge_map)[id_test->data.referent][attr_test->data.referent];
            push(thisAgent, pCond, new_list);
            (*cond_merge_map)[id_test->data.referent][attr_test->data.referent] = new_list;
            dprint(DT_CONSTRAINTS, "ADDED (*cond_merge_map)[%s][%s] -> new_list (+ new entry)\n", id_test->data.referent->to_string(), attr_test->data.referent->to_string());
        }
    }
}

condition *Variablization_Manager::get_previously_seen_cond(condition *pCond)
{
    std::map< Symbol *, std::map< Symbol *, ::list *> >::iterator iter_id;
    std::map< Symbol *, ::list *>::iterator iter_attr;

    //  dprint_condition(DT_MERGE, pCond, "get_previously_seen_cond() called with: ", true, false, true);
    test id_test = equality_test_found_in_test(thisAgent, pCond->data.tests.id_test);
    test attr_test = equality_test_found_in_test(thisAgent, pCond->data.tests.attr_test);
    test val_test = equality_test_found_in_test(thisAgent, pCond->data.tests.value_test);

    dprint(DT_MERGE, "...looking for id equality test %s\n", test_to_string(id_test));
    iter_id = cond_merge_map->find(id_test->data.referent);
    if (iter_id != cond_merge_map->end())
    {
        dprint(DT_MERGE, "...Found entry for %s.  Looking for attr equality test %s\n", static_cast<Symbol *>(iter_id->first)->to_string(), test_to_string(attr_test));
        iter_attr = iter_id->second.find(attr_test->data.referent);
        if (iter_attr != iter_id->second.end())
        {
            dprint(DT_MERGE, "...Found.  Looking in cons list for value equality test %s\n", test_to_string(val_test));
            /* Iterate through cons list and look for matching equality value with the same identity or identifier */
            cons *c;
            condition *lCond;
            test lEqTest;
            c = iter_attr->second;
            while (c)
            {
                lCond = static_cast<condition *>(c->first);
                lEqTest = equality_test_found_in_test(thisAgent, lCond->data.tests.value_test);
                if (lEqTest->data.referent->is_sti())
                {
                    dprint(DT_MERGE, "...comparing with sti %s\n", lEqTest->data.referent);
                    if (lEqTest->data.referent == val_test->data.referent)
                    {
                        dprint_condition(DT_MERGE, lCond, "...returning TRUE with condition: ", true, false, true);
                        return lCond;
                    }
                } else if (lEqTest->identity->grounding_id > 0) {
                    dprint(DT_MERGE, "...comparing with constant %s\n", lEqTest->data.referent->to_string());
                    /* MToDo | Only equality tests on non-literals should be here.  Need to add something to make sure that's true! */
                    if (lEqTest->identity->grounding_id == val_test->identity->grounding_id)
                    {
                        if (lEqTest->identity->original_var == val_test->identity->original_var)
                        {
                            dprint_condition(DT_MERGE, lCond, "...orig vars and g_id match.  returning TRUE with condition: ", true, false, true);
                            return lCond;
                        } else {
                            dprint(DT_MERGE, "...Not merging.  Different original vars: %s != %s\n", val_test->identity->original_var->to_string(), lEqTest->identity->original_var->to_string());
                        }
                    } else {
                        dprint(DT_MERGE, "...Not merging.  Different g_ids: %llu != %llu\n", val_test->identity->grounding_id, lEqTest->identity->grounding_id);
                    }
                } else {
                    dprint(DT_MERGE, "...no grounding id for constant %s!  Should not happen.\n", lEqTest->data.referent);
                }
                c = c->rest;
            }
        }
    }

    dprint(DT_MERGE, "...returning FALSE\n");
    return NULL;
}

void Variablization_Manager::merge_conditions(condition **top_cond)
{
    /* -- This function merges redundant conditions in a condition list by
     *    combining constraints of conditions that share identical equality tests
     *    for all three elements of the condition.
     *
     *    - Iterate through conditions
     *        - Check if value exists in map
     *          - If so,
     *            - add test to original cond value if it doesn't exist (have asserts about extra info not being thrown away)
     *            - delete condition
     *          - If not,
     *            - add cond to map
     * -- */

    /* MToDo | Will probably need to do this for attributes with the same value.  Would double cost but hardly be used I would
     *         think. */

    condition *cond, *found_cond, *last_cond, *next_cond;
    cond = (*top_cond);
    last_cond = NULL;

    dprint(DT_MERGE, "======================\n");
    dprint(DT_MERGE, "= Merging Conditions =\n");
    dprint(DT_MERGE, "======================\n");
    dprint_condition_list(DT_MERGE, *top_cond, "", true, false, true);
    dprint(DT_MERGE, "======================\n");
    while (cond)
    {
        dprint_condition(DT_MERGE, cond, "Merging condition: ", true, false, true);
        next_cond = cond->next;
        if (cond->type==POSITIVE_CONDITION) {
            /* -- Check if there already exists a condition with the same id and
             *    attribute equality tests -- */
            dprint(DT_MERGE, "...looking for previously seen similar condition...\n");
            found_cond = get_previously_seen_cond(cond);

            if (found_cond)
            {
                dprint(DT_MERGE, "...found condition to merge into.  Merging conditions...\n");
                /* -- Add tests in this condition to the already seen condition -- */
                merge_values_in_conds(found_cond, cond);

                /* -- Delete the redundant condition -- */
                if (last_cond)
                {
                    /* -- Not at the head of the list -- */
                    dprint(DT_MERGE, "...deleting non-head item.\n");
                    last_cond->next = cond->next;
                    deallocate_condition(thisAgent, cond);
                    if (last_cond->next)
                        last_cond->next->prev = last_cond;
                    cond = last_cond;
                } else {
                    /* -- At the head of the list -- */
                    dprint(DT_MERGE, "...deleting head of list.\n");
                    (*top_cond) = cond->next;
                    deallocate_condition(thisAgent, cond);
                    if ((*top_cond)->next)
                        (*top_cond)->next->prev = (*top_cond);
                    /* -- This will cause last_cond to be set to  NULL, indicating we're
                     *    at the head of the list -- */
                    cond = NULL;
                }
            } else {
                /* -- First condition seen with given id/attr tests.  So just add to
                 *    map so that future similar conditions can add to it. -- */
                dprint(DT_MERGE, "...did not find condition that matched.  Creating entry in merge map.\n");
                set_cond_for_id_attr_tests(cond);
            }
        }
        last_cond = cond;
        cond = next_cond;
        dprint(DT_MERGE, "...done merging this constraint.\n");
    }
    dprint(DT_MERGE, "======================\n");
    dprint_condition_list(DT_MERGE, *top_cond, "", true, false, true);
    dprint(DT_MERGE, "===========================\n");
    dprint(DT_MERGE, "= Done Merging Conditions =\n");
    dprint(DT_MERGE, "===========================\n");
}

bool is_duplicate_cond (condition *c1, condition *c2) {
  if (c1->type != c2->type) return false;
  bool neg = true;
  switch (c1->type) {
  case POSITIVE_CONDITION:
      neg = false;
  case NEGATIVE_CONDITION:
    if (! tests_are_equal (c1->data.tests.id_test,
                           c2->data.tests.id_test, neg))
      return false;
    if (! tests_are_equal (c1->data.tests.attr_test,
                           c2->data.tests.attr_test, neg))
      return false;
    if (! tests_are_equal (c1->data.tests.value_test,
                           c2->data.tests.value_test, neg))
      return false;
    if (c1->test_for_acceptable_preference !=
        c2->test_for_acceptable_preference)
      return false;
    return true;

  case CONJUNCTIVE_NEGATION_CONDITION:
    for (c1=c1->data.ncc.top, c2=c2->data.ncc.top;
         ((c1!=NIL)&&(c2!=NIL));
         c1=c1->next, c2=c2->next)
      if (! conditions_are_equal (c1,c2)) return false;
    if (c1==c2) return true;  /* make sure they both hit end-of-list */
    return false;
  }
  return false; /* unreachable, but without it, gcc -Wall warns here */
}

/* --
 * Requires:  A condition in a valid doubly-linked condition list
 * Modifies:  Nothing
 * Effect:    Returns whether the condition passed in has a duplicate
 *            previously in the list.  takes a condition and iterates through -- */

 bool Variablization_Manager::condition_is_duplicate(condition *new_condition)
{
    for (condition *check_condition = new_condition->prev; check_condition; check_condition = check_condition->prev)
    {
        if (is_duplicate_cond(check_condition, new_condition))
        {
            return true;
        }
    }
    return false;
}

void Variablization_Manager::remove_dupe_conditions(condition **top_cond)
{
    condition *cond, *last_cond, *next_cond;

    dprint(DT_MERGE, "=================================\n");
    dprint(DT_MERGE, "= Removing duplicate conditions =\n");
    dprint(DT_MERGE, "=================================\n");
    dprint_condition_list(DT_MERGE, *top_cond, "", true, false, true);
    dprint(DT_MERGE, "======================\n");

    last_cond = NULL;
    for (cond = (*top_cond); cond ;)
    {
        dprint_condition(DT_MERGE, cond, "Attempting to add condition: ", true, false, true);
        next_cond = cond->next;
        if (cond->type!=CONJUNCTIVE_NEGATION_CONDITION)
        {
            if (condition_is_duplicate(cond))
            {
                dprint(DT_MERGE, "...condition is duplicate. Deleting.\n");
                if (last_cond)
                {
                    /* -- Not at the head of the list -- */
                    dprint(DT_MERGE, "...deleting non-head item.\n");
                    last_cond->next = cond->next;
                    deallocate_condition(thisAgent, cond);
                    if (last_cond->next)
                        last_cond->next->prev = last_cond;
                    cond = last_cond;
                } else {
                    /* -- At the head of the list -- */
                    dprint(DT_MERGE, "...deleting head of list.\n");
                    (*top_cond) = cond->next;
                    deallocate_condition(thisAgent, cond);
                    if ((*top_cond)->next)
                        (*top_cond)->next->prev = (*top_cond);
                    /* -- Following will cause last_cond to be set to  NULL, indicating we're
                     *    at the head of the list -- */
                    cond = NULL;
                }
            }
        } else {
            // Might need to do merges on NCCs.  Check if really need it.
        }
        last_cond = cond;
        cond = next_cond;
    }

    dprint(DT_MERGE, "======================\n");
    dprint_condition_list(DT_MERGE, *top_cond, "", true, false, true);
    dprint(DT_MERGE, "======================================\n");
    dprint(DT_MERGE, "= Done removing duplicate conditions =\n");
    dprint(DT_MERGE, "======================================\n");
}
