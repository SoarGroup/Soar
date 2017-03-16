#include "ebc.h"
#include "ebc_repair.h"
#include "ebc_timers.h"

#include "agent.h"
#include "condition.h"
#include "dprint.h"
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

    dprint(DT_REPAIR, "Finding path to connect %y (level %d) to a goal state.\n", pNonOperationalSym, static_cast<int64_t>(pNonOperationalSym->id->level));

    ground_lti_tc = get_new_tc_number(thisAgent);

    Symbol* g = thisAgent->top_goal;
    while (g->id->level < pNonOperationalSym->id->level)
    {
        g = g->id->lower_goal;
    }
    dprint(DT_REPAIR, "...%y's goal found: %y at level %d.\n", pNonOperationalSym, g, static_cast<int64_t>(pNonOperationalSym->id->level));

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
//            dprint(DT_REPAIR, "Walk list += IDs from slots of %y to find %y...\n", lCurrentPath->get_root(), pNonOperationalSym);
            for (slot* s = lCurrentPath->get_root()->id->slots; s != NIL; s = s->next)
            {
                for (wme* w = s->wmes; w != NIL; w = w->next)
                {
                    dprint(DT_REPAIR, "   ...considering WME: (%y ^%y %y) %s\n", w->id, w->attr, w->value, w->preference ? "Preference" : "NO Preference");

                    if (w->preference && w->value->is_sti() && (w->value->tc_num != ground_lti_tc))
                    {
                        w->value->tc_num = ground_lti_tc;
                        thisAgent->memoryManager->allocate_with_pool(MP_repair_path, &lNewPath);
                        lNewPath->init(w->value, lCurrentPath->get_path(), w);
                        if (w->value == pNonOperationalSym)
                        {
                            dprint(DT_REPAIR, "...found path to %y: %w.\n", pNonOperationalSym, w);
                            final_path = new wme_list();
                            (*final_path) = *(lNewPath->get_path());
                        } else {
                            dprint(DT_REPAIR, "      - Adding WME (%y ^%y %y) %s\n", w->id, w->attr, w->value, w->preference ? "Preference" : "NO Preference");
                            ids_to_walk.push_back(lNewPath);
                        }
                    }
                }
            }
        }
    }

    lCurrentPath->clean_up();
    thisAgent->memoryManager->free_with_pool(MP_repair_path, lCurrentPath);

    //assert(final_path);
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
            dprint(DT_REPAIR, "Found marked state %y.  Looking for superstate wme in subgoal %y...", g, last_goal);
            for (w = last_goal->id->impasse_wmes; w != NIL; w = w->next)
            {
                if (w->attr == thisAgent->symbolManager->soarSymbols.superstate_symbol)
                {
                    m_repair_WMEs.insert(w);
                    dprint_noprefix(DT_REPAIR, "adding wme for superstate link: %w \n", w);
                }
            }
        } else {
            dprint(DT_REPAIR, "State %y not marked (%u != %u) or level is below match goal (%d < %d).\n", g, g->tc_num, pSeenTC, static_cast<int64_t>(g->id->level), static_cast<int64_t>(m_match_goal_level));
        }
        last_goal = g;
        g = g->id->higher_goal;
    }
}

void Repair_Manager::add_path_to_goal_WMEs(chunk_element* pTargetSym, tc_number cond_tc)
{
    dprint(DT_REPAIR, "Searching for path to goal for %y [%y/%u]...\n", pTargetSym->instantiated_sym, pTargetSym->variable_sym, pTargetSym->identity);
    wme_list* l_WMEPath = find_path_to_goal_for_symbol(pTargetSym->instantiated_sym);
    dprint(DT_REPAIR, "...search complete.  Adding %d WMEs to repair wme path...\n", l_WMEPath->size());
    for (auto it = l_WMEPath->begin(); it != l_WMEPath->end(); it++)
    {
        wme* lWME = (*it);
        dprint(DT_REPAIR, "......adding to repair wme set: (%y ^%y %y)\n", lWME->id, lWME->attr, lWME->value);
        if ((lWME->tc == cond_tc) && (lWME->value != pTargetSym->instantiated_sym))
        {
            dprint(DT_REPAIR, "   ...WME exists in conditions already.  Skipping.\n");
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

    dprint(DT_REPAIR, "Creating condition for %u: (%y ^%y %y)\n", lWME->timetag, lWME->id, lWME->attr, lWME->value);
//    dprint(DT_REPAIR, "   identities of associated pref: (%u ^%u %u)\n", lWME->preference ? lWME->preference->o_ids.id : 0, lWME->preference ? lWME->preference->o_ids.attr : 0, lWME->preference ? lWME->preference->o_ids.value : 0);
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
                dprint(DT_REPAIR, "Marking state found %y in id element with tc_num %u\n", lCond->data.tests.id_test->eq_test->data.referent, tc);
                lSym->tc_num = tc;
            }
            else if (lMatchedSym && lMatchedSym->is_sti() && lMatchedSym->id->isa_goal)
            {
                dprint(DT_REPAIR, "Marking state found %y in id element's instantiated sym with tc_num %u\n", lMatchedSym, tc);
                lMatchedSym->tc_num = tc;
            }
            if (lMatchedSym)
            {
                thisAgent->explanationBasedChunker->add_sti_variablization(lMatchedSym, lSym, lCond->data.tests.id_test->eq_test->identity);
            }

            /* Check if the value element is a state */
            lSym = lCond->data.tests.value_test->eq_test->data.referent;
            lMatchedSym = lSym->is_variable() ? lSym->var->instantiated_sym : NULL;

            if (lSym->is_sti() && lSym->id->isa_goal)
            {
                dprint(DT_REPAIR, "Marking state found %y in value element with tc_num %u\n", lCond->data.tests.id_test->eq_test->data.referent, tc);
                lSym->tc_num = tc;
            }
            else if (lMatchedSym && lMatchedSym->is_sti() && lMatchedSym->id->isa_goal)
            {
                dprint(DT_REPAIR, "Marking state found %y in value element's instantiated sym with tc_num %u\n", lMatchedSym, tc);
                lMatchedSym->tc_num = tc;
            }
            if (lMatchedSym && lMatchedSym->is_sti())
            {
                thisAgent->explanationBasedChunker->add_sti_variablization(lMatchedSym, lSym, lCond->data.tests.value_test->eq_test->identity);
            }
        }
    }
}

void Repair_Manager::repair_rule(condition*& p_lhs_top, matched_symbol_list* p_dangling_syms)
{
    chunk_element* lDanglingSymInfo;
    wme* lWME;
    goal_stack_level targetLevel;


    dprint(DT_REPAIR, "Repair rule started for:\n%1", p_lhs_top);

    /* Determine the highest level of a dangling sym.  We need to add conditions
     * for all (state ^superstate state) wme's between that level and the match
     * level that are not already in the rule. We also add the identity-based
     * variablizations for these dangling symbols so that the repair conditions
     * connect to the real ones correctly */
    dprint(DT_REPAIR, "Step 1: Iterating through dangling syms to determine lowest level that we need to build links to (also adding initial variablizations)...\n");
    targetLevel = thisAgent->bottom_goal->id->level;
    for (auto it = p_dangling_syms->begin(); it != p_dangling_syms->end(); it++)
    {
        lDanglingSymInfo = *it;
        dprint(DT_REPAIR, "Processing dangling sym %y/%y [%u] at level %d...\n", lDanglingSymInfo->instantiated_sym, lDanglingSymInfo->variable_sym,
            lDanglingSymInfo->identity, static_cast<int64_t>(lDanglingSymInfo->instantiated_sym->id->level));
        if(lDanglingSymInfo->instantiated_sym->id->level < targetLevel)
        {
            targetLevel = lDanglingSymInfo->instantiated_sym->id->level;
        } else {
            dprint(DT_REPAIR, "...symbol is at Lower level %d than current target level of %d...\n",
                static_cast<int64_t>(lDanglingSymInfo->instantiated_sym->id->level), static_cast<int64_t>(targetLevel));
        }
        thisAgent->explanationBasedChunker->add_sti_variablization(lDanglingSymInfo->instantiated_sym, lDanglingSymInfo->variable_sym, lDanglingSymInfo->identity);
    }

    tc_number tc;
    tc = get_new_tc_number(thisAgent);

    dprint(DT_REPAIR, "Step 2: Marking states currently in conditions: \n");
    mark_states_WMEs_and_store_variablizations(p_lhs_top, tc);
    thisAgent->symbolManager->reset_variable_generator(p_lhs_top, NULL);
    dprint(DT_REPAIR, "Step 3: Iterating through goal stack to find linking ^superstate augmentations for marked states: \n");
    add_state_link_WMEs(targetLevel, tc);

    dprint(DT_REPAIR, "Step 3: Adding WMEs to connect each dangling symbol...\n");
    for (auto it = p_dangling_syms->begin(); it != p_dangling_syms->end(); it++)
    {
        lDanglingSymInfo = *it;
        /* If dangling symbol is a state, then we will have picked it up when we added the state links above */
        if (!lDanglingSymInfo->instantiated_sym->is_state())
        {
            add_path_to_goal_WMEs(lDanglingSymInfo, tc);
        }
    }

    thisAgent->explanationMemory->increment_stat_grounding_conds_added(m_repair_WMEs.size());

    dprint(DT_REPAIR, "Step 4:  Creating repair condition based on connecting set of WMEs: \n");
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

    dprint(DT_REPAIR, "Final variablized conditions: \n%1", p_lhs_top);
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

            ebc_timers->repair->start();
            Repair_Manager* lRepairManager = new Repair_Manager(thisAgent, m_results_match_goal_level, m_chunk_inst->i_id);
            lRepairManager->repair_rule(m_lhs, unconnected_syms);
            ebc_timers->repair->stop();

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
        thisAgent->explanationMemory->increment_stat_could_not_repair();
        return false;
    }
    delete_ungrounded_symbol_list(thisAgent, &unconnected_syms);

    return true;
}

