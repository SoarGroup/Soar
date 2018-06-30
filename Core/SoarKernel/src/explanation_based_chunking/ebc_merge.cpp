#include "ebc.h"

#include "agent.h"
#include "condition.h"
#include "instantiation.h"
#include "print.h"
#include "test.h"
#include "explanation_memory.h"

void Explanation_Based_Chunker::clear_merge_map()
{
    cond_merge_map->clear();
}

void Explanation_Based_Chunker::merge_values_in_conds(condition* pDestCond, condition* pSrcCond)
{
    copy_non_identical_tests(thisAgent, &(pDestCond->data.tests.attr_test), pSrcCond->data.tests.attr_test);
    copy_non_identical_tests(thisAgent, &(pDestCond->data.tests.value_test), pSrcCond->data.tests.value_test);
    thisAgent->explanationMemory->increment_stat_merged_conditions();
}

condition* Explanation_Based_Chunker::get_previously_seen_cond(condition* pCond)
{
    triple_merge_map::iterator          iter_id;
    sym_to_sym_to_cond_map::iterator    iter_attr;
    sym_to_cond_map::iterator           iter_value;

    iter_id = cond_merge_map->find(pCond->data.tests.id_test->eq_test->data.referent);
    if (iter_id != cond_merge_map->end())
    {
        iter_attr = iter_id->second.find(pCond->data.tests.attr_test->eq_test->data.referent);
        if (iter_attr != iter_id->second.end())
        {
            iter_value = iter_attr->second.find(pCond->data.tests.value_test->eq_test->data.referent);
            if (iter_value != iter_attr->second.end())
            {
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
    int64_t current_cond = 1, cond_diff, new_num_conds, old_num_conds = count_conditions(m_lhs);

    condition* found_cond, *next_cond, *last_cond = NULL;
    for (condition* cond = m_lhs; cond; ++current_cond)
    {
        next_cond = cond->next;
        if (cond->type == POSITIVE_CONDITION)
        {
            found_cond = get_previously_seen_cond(cond);
            if (found_cond)
            {
                merge_values_in_conds(found_cond, cond);

                /* -- Delete the redundant condition -- */
                if (last_cond)
                {
                    /* -- Not at the head of the list -- */
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
    }
    new_num_conds = count_conditions(m_lhs);
    cond_diff = old_num_conds - new_num_conds;

    clear_merge_map();
}
