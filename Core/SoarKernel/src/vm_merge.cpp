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
    /* MToDo | Remove.  No dynamically allocated items in merge map any more. */
//    std::map< Symbol *, std::map< Symbol *, condition *> >::iterator iter_id;
//    std::map< Symbol *, condition *>::iterator iter_attr;
//    std::map< Symbol *, condition *> *attr_values;
//
//    for (iter_id = cond_merge_map->begin(); iter_id != cond_merge_map->end(); ++iter_id)
//    {
//        attr_values = &(iter_id->second);
//        for (iter_attr = attr_values->begin(); iter_attr != attr_values->end(); ++iter_attr)
//        {
//            delete iter_attr->second;
//        }
//        attr_values->clear();
//    }
    cond_merge_map->clear();
}

void Variablization_Manager::merge_values_in_conds(condition *pDestCond, condition *pSrcCond)
{
    dprint(DT_MERGE, "...merging conditions in attribute element...\n");
    add_non_identical_tests(thisAgent, &(pDestCond->data.tests.value_test), pSrcCond->data.tests.value_test);
    dprint(DT_MERGE, "...merging conditions in value element...\n");
    add_non_identical_tests(thisAgent, &(pDestCond->data.tests.value_test), pSrcCond->data.tests.value_test);
}

/* MToDo | Remove.  I think we can just set item directly since we don't need to add to a list any more */

void Variablization_Manager::set_cond_for_id_attr_tests(condition *pCond)
{

    //    std::map< Symbol *, std::map< Symbol *, condition *> >::iterator iter_id;
//    std::map< Symbol *, condition *>::iterator iter_attr;
//    condition * new_merge_info = NULL;
//
//    dprint_condition(DT_MERGE, pCond, "Savind cond in merge map: ", true, false, true);
//
//    dprint(DT_MERGE, "...found equality tests (%s ^%s %s)\n", pCond->data.tests.id_test->eq_test->data.referent->to_string(), pCond->data.tests.attr_test->eq_test->data.referent->to_string(), pCond->data.tests.value_test->eq_test->data.referent->to_string());
//    iter_id = cond_merge_map->find(pCond->data.tests.id_test->eq_test->data.referent);
//    if (iter_id == cond_merge_map->end())
//    {
//        dprint(DT_MERGE, "...id test not found.  Creating new entry.\n");
//
//        new_merge_info = new merge_info;
//        new_merge_info->cond = pCond;
//        new_merge_info->value_equality_test = pCond->data.tests.value_test->eq_test->data.referent;
//
//        std::map<Symbol *, condition *> inner;
//        inner.insert(std::make_pair(pCond->data.tests.attr_test->eq_test->data.referent, pCond));
//        cond_merge_map->insert(std::make_pair(pCond->data.tests.id_test->eq_test->data.referent, inner));
//        dprint(DT_CONSTRAINTS, "ADDED (*cond_merge_map)[%s][%s][%s]\n", pCond->data.tests.id_test->eq_test->data.referent->to_string(), pCond->data.tests.attr_test->eq_test->data.referent->to_string(),pCond->data.tests.value_test->eq_test->data.referent->to_string());
//    } else {
//        dprint(DT_MERGE, "...id test found.  Looking for attribute test...\n");
//        iter_attr = iter_id->second.find(pCond->data.tests.attr_test->eq_test->data.referent);
//        if (iter_attr == iter_id->second.end())
//        {
//            dprint(DT_MERGE, "...attr test not found.  Creating new entry.\n");
//            new_merge_info = new merge_info;
//            new_merge_info->cond = pCond;
//            new_merge_info->value_equality_test = pCond->data.tests.value_test->eq_test->data.referent;
//            (*cond_merge_map)[pCond->data.tests.id_test->eq_test->data.referent][pCond->data.tests.attr_test->eq_test->data.referent] = new_merge_info;
//            dprint(DT_CONSTRAINTS, "ADDED (*cond_merge_map)[%s][%s][%s]\n", pCond->data.tests.id_test->eq_test->data.referent->to_string(), pCond->data.tests.attr_test->eq_test->data.referent->to_string(),pCond->data.tests.value_test->eq_test->data.referent->to_string());
//        } else {
//            dprint(DT_MERGE, "...attr test found.  Creating new entry.\n");
//            new_merge_info = (*cond_merge_map)[pCond->data.tests.id_test->eq_test->data.referent][pCond->data.tests.attr_test->eq_test->data.referent];
//            new_merge_info->cond = pCond;
//            new_merge_info->value_equality_test = pCond->data.tests.value_test->eq_test->data.referent;
//            dprint(DT_CONSTRAINTS, "UPDATED (*cond_merge_map)[%s][%s][%s]\n", pCond->data.tests.id_test->eq_test->data.referent->to_string(), pCond->data.tests.attr_test->eq_test->data.referent->to_string(),pCond->data.tests.value_test->eq_test->data.referent->to_string());
//        }
//    }
}

condition *Variablization_Manager::get_previously_seen_cond(condition *pCond)
{
    std::map< Symbol *, std::map< Symbol *, std::map< Symbol *, condition *> > >::iterator iter_id;
    std::map< Symbol *, std::map< Symbol *, condition *> >::iterator iter_attr;
    std::map< Symbol *, condition *>::iterator iter_value;

    //  dprint_condition(DT_MERGE, pCond, "get_previously_seen_cond() called with: ", true, false, true);

    dprint(DT_MERGE, "...looking for id equality test %s\n", pCond->data.tests.id_test->eq_test->data.referent->to_string());
    iter_id = cond_merge_map->find(pCond->data.tests.id_test->eq_test->data.referent);
    if (iter_id != cond_merge_map->end())
    {
        dprint(DT_MERGE, "...Found.  Looking  for attr equality test %s\n", pCond->data.tests.attr_test->eq_test->data.referent->to_string());
        iter_attr = iter_id->second.find(pCond->data.tests.attr_test->eq_test->data.referent);
        if (iter_attr != iter_id->second.end())
        {
            dprint(DT_MERGE, "...Found.  Looking  for value equality test %s\n", pCond->data.tests.value_test->eq_test->data.referent->to_string());

            iter_value = iter_attr->second.find(pCond->data.tests.value_test->eq_test->data.referent);
            if (iter_value != iter_attr->second.end())
            {
                dprint_condition(DT_MERGE, iter_value->second, "...found similar condition: ", true, false, true);
                return iter_value->second;
            }
            else dprint(DT_MERGE, "...no previously seen similar condition with that value element.\n");
        }
        else dprint(DT_MERGE, "...no previously seen similar condition with that attribute element.\n");
    }
    else dprint(DT_MERGE, "...no previously seen similar condition with that ID element.\n");

    dprint(DT_MERGE, "...returning NULL.\n");
    return NULL;
}


/* -- Variablization_Manager::merge_conditions
 *
 *    Requires: Variablized condition list that does not have any conjunctive
 *              tests containing multiple equality tests
 *    Modifies: top_cond list (may delete entries and move non-equality tests
 *              to other conditions)
 *    Effects:  This function merges redundant conditions in a condition list
 *              by combining constraints of conditions that share identical
 *              equality tests for all three elements of the condition.
 *    Notes:    Since we are working with the variablized list, we do not
 *              need to worry about grounding id's.  The variablization
 *              logic should have already utilized that information when
 *              variablizing.  If we have the same equality symbol, we can
 *              assume they have the same grounding (or one that is unified.)
 * -- */

void Variablization_Manager::merge_conditions(condition *top_cond)
{
    dprint(DT_MERGE, "======================\n");
    dprint(DT_MERGE, "= Merging Conditions =\n");
    dprint(DT_MERGE, "======================\n");
    dprint_condition_list(DT_MERGE, top_cond, "", true, false, true);
    dprint(DT_MERGE, "======================\n");

    condition *found_cond, *next_cond, *last_cond=NULL;
    for (condition *cond = top_cond; cond;)
    {
        dprint_condition(DT_MERGE, cond, "Processing condition: ", true, false, true);
        next_cond = cond->next;
        if (cond->type==POSITIVE_CONDITION) {
            /* -- Check if there already exists a condition with the same
             *    equality tests for all three elements of the condition. -- */

            found_cond = get_previously_seen_cond(cond);
            if (found_cond)
            {
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
                    /* -- At the head of the list.  This probably can never
                     *    occur since the head of the list will never be found
                     *    as a previously seen condition.  -- */
                    assert(false);
                    dprint(DT_MERGE, "...deleting head of list.\n");
                    top_cond = cond->next;
                    deallocate_condition(thisAgent, cond);
                    if (top_cond->next)
                        top_cond->next->prev = top_cond;
                    /* -- This will cause last_cond to be set to  NULL, indicating we're
                     *    at the head of the list -- */
                    cond = NULL;
                }
            } else {
                /* -- First condition with these equality tests.  Add to merge map. -- */
                dprint(DT_MERGE, "...did not find condition that matched.  Creating entry in merge map.\n");
                (*cond_merge_map)[cond->data.tests.id_test->eq_test->data.referent][cond->data.tests.attr_test->eq_test->data.referent][cond->data.tests.value_test->eq_test->data.referent] = cond;
//                set_cond_for_id_attr_tests(cond);
            }
        } else {
            // Search previous conditions for identical NC or NCC
        }
        last_cond = cond;
        cond = next_cond;
        dprint(DT_MERGE, "...done merging this constraint.\n");
    }
    dprint(DT_MERGE, "======================\n");
    dprint_condition_list(DT_MERGE, top_cond, "", true, false, true);
    dprint(DT_MERGE, "===========================\n");
    dprint(DT_MERGE, "= Done Merging Conditions =\n");
    dprint(DT_MERGE, "===========================\n");
}


void Variablization_Manager::fix_tests(condition *top_cond)
{

}

void Variablization_Manager::find_redundancies(condition *top_cond)
{
    dprint(DT_MERGE, "========================\n");
    dprint(DT_MERGE, "= Finding redundancies =\n");
    dprint(DT_MERGE, "========================\n");
    dprint_condition_list(DT_MERGE, top_cond, "", true, false, true);
    dprint(DT_MERGE, "========================\n");

    // Right now, this just
    condition *next_cond, *last_cond=NULL;
    for (condition *cond = top_cond; cond;)
    {
        dprint_condition(DT_MERGE, cond, "Processing condition: ", true, false, true);
        next_cond = cond->next;
        if (cond->type != CONJUNCTIVE_NEGATION_CONDITION) {
            cond->data.tests.id_test->eq_test = equality_test_found_in_test(thisAgent, cond->data.tests.id_test);
            cond->data.tests.attr_test->eq_test = equality_test_found_in_test(thisAgent, cond->data.tests.attr_test);
            cond->data.tests.value_test->eq_test = equality_test_found_in_test(thisAgent, cond->data.tests.value_test);
        } else {
            // Do we really need for NCCs?
        }
        last_cond = cond;
        cond = next_cond;
        dprint(DT_MERGE, "...done merging this constraint.\n");
    }
    dprint(DT_MERGE, "======================\n");
    dprint_condition_list(DT_MERGE, top_cond, "", true, false, true);
    dprint(DT_MERGE, "===========================\n");
    dprint(DT_MERGE, "= Done Merging Conditions =\n");
    dprint(DT_MERGE, "===========================\n");
}


