/*
 * variablization_manager_merge.cpp
 *
 *  Created on: Jul 25, 2013
 *      Author: mazzin
 */

#include "ebc.h"

#include "agent.h"
#include "condition.h"
#include "instantiation.h"
#include "print.h"
#include "test.h"
#include "dprint.h"
#include "explanation_memory.h"

void Explanation_Based_Chunker::clear_merge_map()
{
    cond_merge_map->clear();
}

void Explanation_Based_Chunker::merge_values_in_conds(condition* pDestCond, condition* pSrcCond)
{
    dprint(DT_MERGE, "...merging conditions in attribute element...\n");
    copy_non_identical_tests(thisAgent, &(pDestCond->data.tests.attr_test), pSrcCond->data.tests.attr_test);
    dprint(DT_MERGE, "...merging conditions in value element...\n");
    copy_non_identical_tests(thisAgent, &(pDestCond->data.tests.value_test), pSrcCond->data.tests.value_test);
    #ifdef EBC_DETAILED_STATISTICS
        thisAgent->explanationMemory->increment_stat_merged_conditions();
    #endif
}

condition* Explanation_Based_Chunker::get_previously_seen_cond(condition* pCond)
{
    triple_merge_map::iterator          iter_id;
    sym_to_sym_to_cond_map::iterator    iter_attr;
    sym_to_cond_map::iterator           iter_value;

    dprint(DT_MERGE, "...looking for id equality test %y\n", pCond->data.tests.id_test->eq_test->data.referent);
    iter_id = cond_merge_map->find(pCond->data.tests.id_test->eq_test->data.referent);
    if (iter_id != cond_merge_map->end())
    {
        iter_attr = iter_id->second.find(pCond->data.tests.attr_test->eq_test->data.referent);
        if (iter_attr != iter_id->second.end())
        {
            iter_value = iter_attr->second.find(pCond->data.tests.value_test->eq_test->data.referent);
            if (iter_value != iter_attr->second.end())
            {
                dprint(DT_MERGE, "          ...found similar condition: %l\n", iter_value->second);
                return iter_value->second;
            }
        }
    }

    return NULL;
}


/* -- Variablization_Manager::merge_conditions
 *
 *    Requires: Variablized condition list that does not have any conjunctive
 *              tests containing multiple equality tests
 *    Modifies: m_lhs list (may delete entries and move non-equality tests
 *              to other conditions)
 *    Effects:  This function merges redundant conditions in a condition list
 *              by combining constraints of conditions that share identical
 *              equality tests for all three elements of the condition.  At this
 *              point the conditions are variablized, so it is merging based
 *              on the unified variables, not the wmes matched.
 * -- */

void Explanation_Based_Chunker::merge_conditions()
{
    if (!ebc_settings[SETTING_EBC_MERGE]) return;

    dprint_header(DT_MERGE, PrintBoth, "= Merging Conditions =\n%1", m_lhs);
    int64_t current_cond = 1, cond_diff, new_num_conds, old_num_conds = count_conditions(m_lhs);
    dprint_header(DT_MERGE, PrintAfter, "# of conditions = %d\n", old_num_conds);

    condition* found_cond, *next_cond, *last_cond = NULL;
    for (condition* cond = m_lhs; cond; ++current_cond)
    {
        dprint(DT_MERGE, "Processing condition %d: %l\n", current_cond, cond);
        next_cond = cond->next;
        if (cond->type == POSITIVE_CONDITION)
        {
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
                    {
                        last_cond->next->prev = last_cond;
                    }
                    cond = last_cond;
                }
                else
                {
                    /* -- At the head of the list.  This probably can never
                     *    occur since the head of the list will never be found
                     *    as a previously seen condition.  -- */
                    dprint(DT_MERGE, "...deleting head of list.\n");
                    m_lhs = cond->next;
                    deallocate_condition(thisAgent, cond);
                    if (m_lhs->next)
                    {
                        m_lhs->next->prev = m_lhs;
                    }
                    /* -- This will cause last_cond to be set to  NULL, indicating we're
                     *    at the head of the list -- */
                    cond = NULL;
                }
            }
            else
            {
                /* -- First condition with these equality tests.  Add to merge map. -- */
                dprint(DT_MERGE, "...did not find condition that matched.  Creating entry in merge map.\n");
                (*cond_merge_map)[cond->data.tests.id_test->eq_test->data.referent][cond->data.tests.attr_test->eq_test->data.referent][cond->data.tests.value_test->eq_test->data.referent] = cond;
            }
        }
        else
        {
            /* Search previous conditions for identical NC or NCC.
             *
             * I think identical NCCs might be impossible since collecting the grounds uses a
             * hash table to avoid adding the same NCC twice.
             *
             * We might be able to handle NCs with positive conditions if somehow store a flag
             * that indicates the type of the condition in the merge map.  Would need to use a
             * more complex data structure.
             */

        }
        last_cond = cond;
        cond = next_cond;
        dprint(DT_MERGE, "...done merging this constraint.\n");
    }
    dprint_header(DT_MERGE, PrintBefore, "Final merged conditions:\n");
    dprint_noprefix(DT_MERGE, "%1", m_lhs);
    new_num_conds = count_conditions(m_lhs);
    cond_diff = old_num_conds - new_num_conds;
    dprint(DT_MERGE, "# of conditions = %d\n", new_num_conds);
    dprint(DT_MERGE, ((cond_diff > 0) ? "Conditions decreased by %d conditions! (%d - %d)\n" : "No decrease in number of conditions. [%d = (%d - %d)]\n"), cond_diff, old_num_conds, new_num_conds);

    clear_merge_map();

    dprint(DT_VARIABLIZATION_MANAGER, "Conditions after merging: \n%1", m_lhs);

}
