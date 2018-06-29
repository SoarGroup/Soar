#include "ebc.h"
#include "ebc_repair.h"
#include "ebc_timers.h"

#include "agent.h"
#include "condition.h"
#include "explanation_memory.h"
#include "instantiation.h"
#include "output_manager.h"
#include "preference.h"
#include "production.h"
#include "slot.h"
#include "symbol.h"
#include "symbol_manager.h"
#include "test.h"
#include "working_memory.h"

void delete_ungrounded_symbol_list(agent* thisAgent, matched_symbol_list** unconnected_syms)
{
    matched_symbol_list* lSyms = *unconnected_syms;
    chunk_element* lSym;
   
    //for(matched_symbol_list::iterator it = lSyms->begin(), end = lSyms->end(); it != end; ++it)
    for (auto it = lSyms->begin(); it != lSyms->end(); it++)
    {
        lSym = (*it);
        if (lSym->variable_sym)
        {
            lSym->variable_sym->tc_num = 0;
        }
        thisAgent->memoryManager->free_with_pool(MP_chunk_element, lSym);
    }
    delete (*unconnected_syms);
    (*unconnected_syms) = NULL;
}

void Repair_Path::init(Symbol* new_root, wme_list* new_path, wme* new_wme)
{
    topSym = new_root;
    wme_path = new wme_list();
    if (new_path) {
        (*wme_path) = (*new_path);
        if (new_wme) wme_path->push_front(new_wme);
        /* For use when we try using a vector with a new memory allocator that can handle variable size allocations */
        //if (new_wme) wme_path->push_back(new_wme);
    }
}

wme_list* Repair_Manager::find_path_to_goal_for_symbol(Symbol* pNonOperationalSym)
{
    repair_path_list ids_to_walk;
    Repair_Path*            lCurrentPath = NULL, *lNewPath = NULL;
    wme_list*               final_path = NULL;
    tc_number               ground_lti_tc;


    ground_lti_tc = get_new_tc_number(thisAgent);

    Symbol* g = thisAgent->top_goal;
    while (g->id->level < pNonOperationalSym->id->level)
    {
        g = g->id->lower_goal;
    }

    thisAgent->memoryManager->allocate_with_pool(MP_repair_path, &lNewPath);
    lNewPath->init(g);
    ids_to_walk.push_back(lNewPath);
    g->tc_num = ground_lti_tc;

    while (!ids_to_walk.empty())
    {
        if (lCurrentPath)
        {
            lCurrentPath->clean_up();
            thisAgent->memoryManager->free_with_pool(MP_repair_path, lCurrentPath);
        }

        lCurrentPath = ids_to_walk.back();
        ids_to_walk.pop_back();

        if (!final_path) /* We keep iterating after we find the final path, so that  */
        {                /* we can delete the rest of the sym_grounding_path objects */
            for (slot* s = lCurrentPath->get_root()->id->slots; s != NIL; s = s->next)
            {
                for (wme* w = s->wmes; w != NIL; w = w->next)
                {
                    if (w->preference && w->value->is_sti() && (w->value->tc_num != ground_lti_tc))
                    {
                        w->value->tc_num = ground_lti_tc;
                        thisAgent->memoryManager->allocate_with_pool(MP_repair_path, &lNewPath);
                        lNewPath->init(w->value, lCurrentPath->get_path(), w);
                        if (w->value == pNonOperationalSym)
                        {
                            final_path = new wme_list();
                            (*final_path) = *(lNewPath->get_path());
                        } else {
                            ids_to_walk.push_back(lNewPath);
                        }
                    }
                }
            }
        }
    }

    lCurrentPath->clean_up();
    thisAgent->memoryManager->free_with_pool(MP_repair_path, lCurrentPath);

    return final_path;
}

inline void add_cond_to_lists(condition** c, condition** prev, condition** first)
{
    if (*prev)
    {
        (*c)->prev = *prev;
        (*prev)->next = *c;
    }
    else
    {
        *first = *c;
        *prev = NIL;
        (*c)->prev = NIL;
    }
    *prev = *c;
}

void Repair_Manager::add_state_link_WMEs(goal_stack_level pTargetGoal, tc_number pSeenTC)
{
    Symbol* g, *last_goal = NULL;
    wme* w;

    g = thisAgent->bottom_goal;
    while (g->id->level > pTargetGoal)
    {
        if (g->tc_num != pSeenTC && (g->id->level < m_match_goal_level))
        {
            for (w = last_goal->id->impasse_wmes; w != NIL; w = w->next)
            {
                if (w->attr == thisAgent->symbolManager->soarSymbols.superstate_symbol)
                {
                    m_repair_WMEs.insert(w);
                }
            }
        }
        last_goal = g;
        g = g->id->higher_goal;
    }
}

void Repair_Manager::add_path_to_goal_WMEs(chunk_element* pTargetSym, tc_number cond_tc)
{
    wme_list* l_WMEPath = find_path_to_goal_for_symbol(pTargetSym->instantiated_sym);
    for (auto it = l_WMEPath->begin(); it != l_WMEPath->end(); it++)
    {
        wme* lWME = (*it);
        if ((lWME->tc == cond_tc) && (lWME->value != pTargetSym->instantiated_sym))
        {
            continue;
        }
        m_repair_WMEs.insert(lWME);
    }
}

Repair_Manager::Repair_Manager(agent* myAgent, goal_stack_level  p_goal_level, uint64_t p_chunk_ID)
{
    thisAgent = myAgent;
    m_match_goal_level = p_goal_level;
    m_chunk_ID = p_chunk_ID;
}

Repair_Manager::~Repair_Manager()
{
    thisAgent->explanationBasedChunker->clear_sti_variablization_map();
}

condition* Repair_Manager::make_condition_from_wme(wme* lWME)
{
    condition* new_cond;

    new_cond = make_condition(thisAgent,
        make_test(thisAgent, lWME->id, EQUALITY_TEST),
        make_test(thisAgent, lWME->attr, EQUALITY_TEST),
        make_test(thisAgent, lWME->value, EQUALITY_TEST));
    new_cond->test_for_acceptable_preference = lWME->acceptable;
    new_cond->bt.wme_ = lWME;
    new_cond->bt.trace = lWME->preference;
    new_cond->bt.level = lWME->id->id->level;
    new_cond->inst = lWME->preference ? lWME->preference->inst : NULL;

    return new_cond;
}

void Repair_Manager::mark_states_WMEs_and_store_variablizations(condition* pCondList, tc_number tc)
{
    condition* lCond;
    Symbol* highest_goal = thisAgent->bottom_goal, *lSym, *lMatchedSym;
    test lID_test, lValue_test, lInst_ID_test, lInst_Value_test, highest_goal_test = NULL;

    for (lCond = pCondList; lCond; lCond = lCond->next)
    {
        if (lCond->type == POSITIVE_CONDITION)
        {
            /* Mark the wme so that we don't add a duplicate */
            lCond->bt.wme_->tc = tc;

            /* Check if the id element is a state */
            lSym = lCond->data.tests.id_test->eq_test->data.referent;
            lMatchedSym = lSym->is_variable() ? lSym->var->instantiated_sym : NULL;

            if (lSym->is_sti() && lSym->id->isa_goal)
            {
                lSym->tc_num = tc;
            }
            else if (lMatchedSym && lMatchedSym->is_sti() && lMatchedSym->id->isa_goal)
            {
                lMatchedSym->tc_num = tc;
            }
            if (lMatchedSym)
            {
                thisAgent->explanationBasedChunker->add_sti_variablization(lMatchedSym, lSym, lCond->data.tests.id_test->eq_test->inst_identity);
            }

            /* Check if the value element is a state */
            lSym = lCond->data.tests.value_test->eq_test->data.referent;
            lMatchedSym = lSym->is_variable() ? lSym->var->instantiated_sym : NULL;

            if (lSym->is_sti() && lSym->id->isa_goal)
            {
                lSym->tc_num = tc;
            }
            else if (lMatchedSym && lMatchedSym->is_sti() && lMatchedSym->id->isa_goal)
            {
                lMatchedSym->tc_num = tc;
            }
            if (lMatchedSym && lMatchedSym->is_sti())
            {
                thisAgent->explanationBasedChunker->add_sti_variablization(lMatchedSym, lSym, lCond->data.tests.value_test->eq_test->inst_identity);
            }
        }
    }
}

void Repair_Manager::repair_rule(condition*& p_lhs_top, matched_symbol_list* p_dangling_syms)
{
    chunk_element* lDanglingSymInfo;
    wme* lWME;
    goal_stack_level targetLevel;

    /* Determine the highest level of a dangling sym.  We need to add conditions
     * for all (state ^superstate state) wme's between that level and the match
     * level that are not already in the rule.*/
    targetLevel = thisAgent->bottom_goal->id->level;
    for (auto it = p_dangling_syms->begin(); it != p_dangling_syms->end(); it++)
    {
        lDanglingSymInfo = *it;
        if(lDanglingSymInfo->instantiated_sym->id->level < targetLevel)
        {
            targetLevel = lDanglingSymInfo->instantiated_sym->id->level;
        }
    }

    tc_number tc;
    tc = get_new_tc_number(thisAgent);

    mark_states_WMEs_and_store_variablizations(p_lhs_top, tc);

    for (auto it = p_dangling_syms->begin(); it != p_dangling_syms->end(); it++)
    {
        lDanglingSymInfo = *it;
        thisAgent->explanationBasedChunker->add_sti_variablization(lDanglingSymInfo->instantiated_sym, lDanglingSymInfo->variable_sym, lDanglingSymInfo->inst_identity);
    }

    thisAgent->symbolManager->reset_variable_generator(p_lhs_top, NULL);
    add_state_link_WMEs(targetLevel, tc);

    for (auto it = p_dangling_syms->begin(); it != p_dangling_syms->end(); it++)
    {
        lDanglingSymInfo = *it;
        /* If dangling symbol is a state, then we will have picked it up when we added the state links above */
        if (!lDanglingSymInfo->instantiated_sym->is_state())
        {
            add_path_to_goal_WMEs(lDanglingSymInfo, tc);
        }
    }

    condition* new_cond, *prev_cond = p_lhs_top, *first_cond = p_lhs_top;

    while (prev_cond->next != NULL) prev_cond = prev_cond->next;

    for (auto it = m_repair_WMEs.begin(); it != m_repair_WMEs.end(); it++)
    {
        lWME = (*it);
        new_cond = make_condition_from_wme(lWME);
        thisAgent->explanationBasedChunker->sti_variablize_test(new_cond->data.tests.id_test);
        thisAgent->explanationBasedChunker->sti_variablize_test(new_cond->data.tests.value_test);
        add_cond_to_lists(&new_cond, &prev_cond, &first_cond);
    }

    if (prev_cond) prev_cond->next = NIL;
    else if (first_cond) first_cond->next = NIL;

    p_lhs_top = first_cond;
}

bool Explanation_Based_Chunker::reorder_and_validate_chunk()
{
    matched_symbol_list* unconnected_syms = new matched_symbol_list();

    reorder_and_validate_lhs_and_rhs(thisAgent, &m_lhs, &m_rhs, false, unconnected_syms, ebc_settings[SETTING_EBC_REPAIR_LHS], ebc_settings[SETTING_EBC_REPAIR_RHS]);

    if (m_failure_type != ebc_success)
    {
        if (((m_failure_type == ebc_failed_unconnected_conditions) && ebc_settings[SETTING_EBC_REPAIR_LHS]) ||
            ((m_failure_type == ebc_failed_reordering_rhs) && ebc_settings[SETTING_EBC_REPAIR_RHS]))
        {
            thisAgent->outputManager->display_soar_feedback(thisAgent, ebc_progress_repairing, thisAgent->trace_settings[TRACE_CHUNKS_WARNINGS_SYSPARAM]);

            Repair_Manager* lRepairManager = new Repair_Manager(thisAgent, m_results_match_goal_level, m_chunk_inst->i_id);
            lRepairManager->repair_rule(m_lhs, unconnected_syms);

            delete_ungrounded_symbol_list(thisAgent, &unconnected_syms);
            unconnected_syms = new matched_symbol_list();
            thisAgent->outputManager->display_soar_feedback(thisAgent, ebc_progress_validating, thisAgent->trace_settings[TRACE_CHUNKS_WARNINGS_SYSPARAM]);
            if (reorder_and_validate_lhs_and_rhs(thisAgent, &m_lhs, &m_rhs, false, unconnected_syms, false, false))
            {
                delete_ungrounded_symbol_list(thisAgent, &unconnected_syms);
                if (thisAgent->trace_settings[TRACE_CHUNKS_WARNINGS_SYSPARAM])
                {
                    thisAgent->outputManager->display_soar_feedback(thisAgent, ebc_progress_repaired);
                    print_current_built_rule("Repaired rule:");
                }
                thisAgent->explanationMemory->increment_stat_chunks_repaired();
                return true;
            }
        }
        thisAgent->outputManager->display_soar_feedback(thisAgent, ebc_error_invalid_chunk, thisAgent->trace_settings[TRACE_CHUNKS_WARNINGS_SYSPARAM]);
        delete_ungrounded_symbol_list(thisAgent, &unconnected_syms);
        return false;
    }
    delete_ungrounded_symbol_list(thisAgent, &unconnected_syms);

    return true;
}

