#include <ebc_repair.h>
#include "ebc.h"

#include "agent.h"
#include "condition.h"
#include "output_manager.h"
#include "preference.h"
#include "production.h"
#include "slot.h"
#include "symbol.h"
#include "symbol_manager.h"
#include "test.h"
#include "working_memory.h"
#include "dprint.h"

#include "explanation_memory.h"


void delete_ungrounded_symbol_list(agent* thisAgent, matched_symbol_list** unconnected_syms)
{
    matched_symbol_list* lSyms = *unconnected_syms;
    matched_sym* lSym;

    for (auto it = lSyms->begin(); it != lSyms->end(); it++)
    {
        lSym = (*it);
        if (lSym->sym)
        {
            lSym->sym->tc_num = 0;
        }
        delete lSym;
    }
    delete (*unconnected_syms);
    (*unconnected_syms) = NULL;
}

wme_list* Repair_Manager::find_path_to_goal_for_symbol(Symbol* pNonOperationalSym)
{
    sym_grounding_path_list ids_to_walk;
    Path_to_Goal_State*     lCurrentPath = NULL, *lNewPath = NULL;
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

    lNewPath = new Path_to_Goal_State(g);
//    if (g == pNonOperationalSym)
//    {
////        return (new wme_list());
//        return NULL;
//    }

    ids_to_walk.push_back(lNewPath);
    g->tc_num = ground_lti_tc;

    while (!ids_to_walk.empty())
    {
        if (lCurrentPath) delete lCurrentPath;

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
                        lNewPath = new Path_to_Goal_State(w->value, lCurrentPath->get_path(), w);
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

    delete lCurrentPath;

    assert(final_path);
    return final_path;
}

inline void add_cond_to_lists(condition** c, condition** prev, condition** first)
{
    if (*prev)
    {
        (*c)->prev = *prev;
        (*prev)->next = *c;
        (*c)->counterpart->prev = (*prev)->counterpart;
        (*prev)->counterpart->next = (*c)->counterpart;
    }
    else
    {
        *first = *c;
        *prev = NIL;
        (*c)->prev = NIL;
        (*first)->counterpart = (*c)->counterpart;
        (*c)->counterpart->prev = NIL;
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
            assert(last_goal);
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

void Repair_Manager::add_path_to_goal_WMEs(matched_sym* pTargetSym, tc_number cond_tc)
{
    dprint(DT_REPAIR, "Searching for path to goal for %y [%y/%u]...\n", pTargetSym->matched_sym, pTargetSym->sym, pTargetSym->identity);
    wme_list* l_WMEPath = find_path_to_goal_for_symbol(pTargetSym->matched_sym);
    dprint(DT_REPAIR, "...search complete.  Adding %d WMEs to repair wme path...\n", l_WMEPath->size());
    for (auto it = l_WMEPath->begin(); it != l_WMEPath->end(); it++)
    {
        wme* lWME = (*it);
        dprint(DT_REPAIR, "......adding to repair wme set: (%y ^%y %y)\n", lWME->id, lWME->attr, lWME->value);
        if ((lWME->tc == cond_tc) && (lWME->value != pTargetSym->matched_sym))
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

}

void Repair_Manager::variablize_connecting_sti(test pTest)
{
    assert(pTest && pTest->type == EQUALITY_TEST);

    char prefix[2];
    Symbol* lNewVar = NULL, *lMatchedSym = pTest->data.referent;
    uint64_t lMatchedIdentity = 0;

    assert(lMatchedSym->is_sti());

    /* Copy in any identities for the unconnected identifier that was used in the unconnected conditions */
    std::unordered_map< Symbol*, Symbol* >::iterator iter_sym;
    std::unordered_map< Symbol*, uint64_t >::iterator iter_id;

    iter_sym = m_sym_to_var_map.find(lMatchedSym);
    if (iter_sym == m_sym_to_var_map.end())
    {
        /* Create a new variable.  If constant is being variablized just used
         * 'c' instead of first letter of id name.  We now don't use 'o' for
         * non-operators and don't use 's' for non-states.  That makes things
         * clearer in chunks because of standard naming conventions. --- */
        char prefix_char = static_cast<char>(tolower(lMatchedSym->id->name_letter));
        if (((prefix_char == 's') || (prefix_char == 'S')) && !lMatchedSym->id->isa_goal)
        {
            prefix[0] = 'c';
        } else if (((prefix_char == 'o') || (prefix_char == 'O')) && !lMatchedSym->id->isa_operator) {
            prefix[0] = 'c';
        } else {
            prefix[0] = prefix_char;
        }
        prefix[1] = 0;
        lNewVar = thisAgent->symbolManager->generate_new_variable(prefix);
        lMatchedIdentity = thisAgent->explanationBasedChunker->get_or_create_o_id(lNewVar, m_chunk_ID);
    }
    else
    {
        lNewVar = iter_sym->second;
        thisAgent->symbolManager->symbol_add_ref(lNewVar);
        iter_id = m_sym_to_id_map.find(lMatchedSym);
        /* Always added in pairs.  Should use a single map */
        assert(iter_id != m_sym_to_id_map.end());
        lMatchedIdentity = iter_id->second;
    }

    add_variablization(lMatchedSym, lNewVar, lMatchedIdentity, "new condition");
    pTest->data.referent = lNewVar;
    pTest->identity = lMatchedIdentity;
    thisAgent->symbolManager->symbol_remove_ref(&lMatchedSym);
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
    new_cond->bt.level = lWME->id->id->level;
    new_cond->bt.trace = lWME->preference;
    new_cond->inst = lWME->preference ? lWME->preference->inst : NULL;

    /* In other functions we only add a reference if the instantiation match goal level is
     * not the top level.  We don't have that value yet, so I'm going to try to use the level
     * of the wme itself. */
#ifndef DO_TOP_LEVEL_REF_CTS
    if (new_cond->bt.level > TOP_GOAL_LEVEL)
#endif
    {
        wme_add_ref(lWME);
    }

    if (new_cond->bt.trace)
    {
#ifndef DO_TOP_LEVEL_REF_CTS
        if (new_cond->bt.level > TOP_GOAL_LEVEL)
#endif
        {
            preference_add_ref(new_cond->bt.trace);
        }
    }

    assert(new_cond->bt.wme_->preference == new_cond->bt.trace);

    return new_cond;
}

void Repair_Manager::add_variablization(Symbol* pSym, Symbol* pVar, uint64_t pIdentity, const char* pTypeStr)
{
    dprint(DT_REPAIR, "Adding %s variablization found for %y -> %y [%u]\n", pTypeStr, pSym, pVar, pIdentity);
    m_sym_to_var_map[pSym] = pVar;
    m_sym_to_id_map[pSym] = pIdentity;
    assert(!pVar->is_variable() || pIdentity);
}

void Repair_Manager::mark_states_WMEs_and_store_variablizations(condition* pCondList, tc_number tc)
{
    assert(pCondList);
    condition* lCond;
    Symbol* highest_goal = thisAgent->bottom_goal;
    test lID_test, lValue_test, lInst_ID_test, lInst_Value_test, highest_goal_test = NULL;

    for (lCond = pCondList; lCond; lCond = lCond->next)
    {
        if (lCond->type == POSITIVE_CONDITION)
        {
            assert(lCond->bt.wme_);
            /* Mark the wme so that we don't add a duplicate */
            lCond->bt.wme_->tc = tc;
            if (lCond->data.tests.id_test->eq_test->data.referent->is_sti())
            {
                if (lCond->data.tests.id_test->eq_test->data.referent->id->isa_goal)
                {
                    dprint(DT_REPAIR, "Marking state found %y in id element with tc_num %u\n", lCond->data.tests.id_test->eq_test->data.referent, tc);
                    lCond->data.tests.id_test->eq_test->data.referent->tc_num = tc;
                }
            } else {
                if (lCond->counterpart &&
                    lCond->counterpart->data.tests.id_test->eq_test->data.referent->is_sti() &&
                    lCond->counterpart->data.tests.id_test->eq_test->data.referent->id->isa_goal)
                {
                    dprint(DT_REPAIR, "Marking state found %y in id element counterpart with tc_num %u\n", lCond->counterpart->data.tests.id_test->eq_test->data.referent, tc);
                    lCond->counterpart->data.tests.id_test->eq_test->data.referent->tc_num = tc;
                }
            }
            if (lCond->counterpart && lCond->counterpart->data.tests.id_test->eq_test->data.referent->is_variable())
            {
                add_variablization(lCond->data.tests.id_test->eq_test->data.referent, lCond->counterpart->data.tests.id_test->eq_test->data.referent, lCond->data.tests.id_test->eq_test->identity);
            }
            if (lCond->data.tests.id_test->eq_test->data.referent->is_variable())
            {
                add_variablization(lCond->counterpart->data.tests.id_test->eq_test->data.referent, lCond->data.tests.id_test->eq_test->data.referent, lCond->data.tests.id_test->eq_test->identity);
            }
            if (lCond->data.tests.value_test->eq_test->data.referent->is_sti())
            {
                if (lCond->data.tests.value_test->eq_test->data.referent->id->isa_goal)
                {
                    dprint(DT_REPAIR, "Marking state found %y in value element with tc_num %u\n", lCond->data.tests.value_test->eq_test->data.referent, tc);
                    lCond->data.tests.value_test->eq_test->data.referent->tc_num = tc;
                }
            } else {
                if (lCond->counterpart &&
                    lCond->counterpart->data.tests.value_test->eq_test->data.referent->is_sti() &&
                    lCond->counterpart->data.tests.value_test->eq_test->data.referent->id->isa_goal)
                {
                    dprint(DT_REPAIR, "Marking state found %y in value element counterpart with tc_num %u\n", lCond->counterpart->data.tests.value_test->eq_test->data.referent, tc);
                    lCond->counterpart->data.tests.value_test->eq_test->data.referent->tc_num = tc;
                }
            }
            if (lCond->counterpart && lCond->counterpart->data.tests.value_test->eq_test->data.referent->is_variable())
            {
                add_variablization(lCond->data.tests.value_test->eq_test->data.referent, lCond->counterpart->data.tests.value_test->eq_test->data.referent, lCond->data.tests.value_test->eq_test->identity);
            }
            if (lCond->data.tests.value_test->eq_test->data.referent->is_variable())
            {
                add_variablization(lCond->counterpart->data.tests.value_test->eq_test->data.referent, lCond->data.tests.value_test->eq_test->data.referent, lCond->data.tests.value_test->eq_test->identity);
            }
        }
    }
}

void Repair_Manager::repair_rule(condition*& m_vrblz_top, condition*& m_inst_top, condition*& m_inst_bottom, matched_symbol_list* p_dangling_syms)
{
    matched_sym* lDanglingSymInfo;
    wme* lWME;
    goal_stack_level targetLevel;


    dprint(DT_REPAIR, "Repair rule started...\n");
    dprint(DT_REPAIR, "- Variablized cond: \n%1", m_vrblz_top);
    dprint(DT_REPAIR, "\n- Instantiated conds :\n%1", m_inst_top, NULL);

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
        dprint(DT_REPAIR, "Processing dangling sym %y/%y [%u] at level %i...\n", lDanglingSymInfo->matched_sym, lDanglingSymInfo->sym,
            lDanglingSymInfo->identity, static_cast<int64_t>(lDanglingSymInfo->matched_sym->id->level));
        if(lDanglingSymInfo->matched_sym->id->level < targetLevel)
        {
            targetLevel = lDanglingSymInfo->matched_sym->id->level;
        } else {
            dprint(DT_REPAIR, "...symbol is at Lower level %i than current target level of %i...\n",
                static_cast<int64_t>(lDanglingSymInfo->matched_sym->id->level), static_cast<int64_t>(targetLevel));
        }
        add_variablization(lDanglingSymInfo->matched_sym, lDanglingSymInfo->sym, lDanglingSymInfo->identity, "dangling symbol");
    }

    tc_number tc;
    tc = get_new_tc_number(thisAgent);

    dprint(DT_REPAIR, "Step 2: Marking states currently in conditions: \n");
    mark_states_WMEs_and_store_variablizations(m_vrblz_top, tc);
    thisAgent->symbolManager->reset_variable_generator(m_vrblz_top, NULL);
    dprint(DT_REPAIR, "Step 3: Iterating through goal stack to find linking ^superstate augmentations for marked states: \n");
    add_state_link_WMEs(targetLevel, tc);

    /* Generate connecting wme's for each unconnected identifier and add to a set */
    dprint(DT_REPAIR, "Step 3: Adding WMEs to connect each dangling symbol...\n");
    for (auto it = p_dangling_syms->begin(); it != p_dangling_syms->end(); it++)
    {
        lDanglingSymInfo = *it;
        /* If dangling symbol is a state, then we will have picked it up when we
         * added the state links above */
        if (!lDanglingSymInfo->matched_sym->is_state())
        {
            add_path_to_goal_WMEs(lDanglingSymInfo, tc);
        }
    }

    #ifdef BUILD_WITH_EXPLAINER
    thisAgent->explanationMemory->increment_stat_grounding_conds_added(m_repair_WMEs.size());
    #endif

    /* Create conditions based on set of wme's compiled */
    dprint(DT_REPAIR, "Step 4:  Creating repair condition based on connecting set of WMEs: \n");
    condition* new_cond, *new_inst_cond, *prev_cond = m_vrblz_top, *first_cond = m_vrblz_top;
    while (prev_cond->next != NULL)
        prev_cond = prev_cond->next;

//    prev_cond = first_cond = NULL;

    for (auto it = m_repair_WMEs.begin(); it != m_repair_WMEs.end(); it++)
    {
        lWME = (*it);
        new_cond = make_condition_from_wme(lWME);
        new_inst_cond = copy_condition(thisAgent, new_cond);
        new_cond->counterpart = new_inst_cond;
        new_inst_cond->counterpart = new_cond;
        dprint(DT_REPAIR, "   Variablizing %l\n", new_cond);
        /* Variablize and add to condition list */
        variablize_connecting_sti(new_cond->data.tests.id_test);
        variablize_connecting_sti(new_cond->data.tests.value_test);
        new_cond->counterpart->data.tests.id_test->identity = new_cond->data.tests.id_test->identity;
        new_cond->counterpart->data.tests.value_test->identity = new_cond->data.tests.value_test->identity;
        dprint(DT_REPAIR, "   --> %l\n", new_cond);
        add_cond_to_lists(&new_cond, &prev_cond, &first_cond);

    }
    if (prev_cond)
    {
        prev_cond->next = NIL;
    }
    else if (first_cond)
    {
        first_cond->next = NIL;
    }

    m_vrblz_top = first_cond;
    m_inst_top = first_cond->counterpart;
    prev_cond = m_inst_top;
    while (prev_cond->next != NULL) prev_cond = prev_cond->next;
    m_inst_bottom = prev_cond;

    dprint(DT_REPAIR, "Final variablized conditions: \n%1Final instantiated counterparts:\n%1", m_vrblz_top, m_inst_top);
}

bool Explanation_Based_Chunker::reorder_and_validate_chunk()
{
    /* This is called for justifications even though it does nothing because in the future
     * we might want to fix a justification that has conditions unconnected to
     * a state.  Chunks that have such variablized conditions seem to be able to
     * corrupt the rete, but we don't know if justifications can as well.  While we
     * could ground those conditions like we do with chunks to be safe, we're not doing
     * that right now because it will introduce a high computational cost that may
     * not be necessary.*/

    if (m_prod_type != JUSTIFICATION_PRODUCTION_TYPE)
    {
        matched_symbol_list* unconnected_syms = new matched_symbol_list();

        reorder_and_validate_lhs_and_rhs(thisAgent, &m_vrblz_top, &m_rhs, false,
            unconnected_syms, ebc_settings[SETTING_EBC_REPAIR_LHS], ebc_settings[SETTING_EBC_REPAIR_LHS]);

        /* Fix m_inst's bottom and top pointers to match re-ordered variablized conditions*/
        m_inst_top = m_vrblz_top->counterpart;
        condition* c = m_vrblz_top;
        while (c->next) c = c->next;
        m_inst_bottom = c->counterpart;

        if (m_failure_type != ebc_success)
        {
            if (((m_failure_type == ebc_failed_unconnected_conditions) && ebc_settings[SETTING_EBC_REPAIR_LHS]) ||
                ((m_failure_type == ebc_failed_reordering_rhs) && ebc_settings[SETTING_EBC_REPAIR_RHS]))
            {
                thisAgent->outputManager->display_soar_feedback(thisAgent, ebc_progress_repairing);
                Repair_Manager* lRepairManager = new Repair_Manager(thisAgent, m_results_match_goal_level, m_chunk_new_i_id);
                lRepairManager->repair_rule(m_vrblz_top, m_inst_top, m_inst_bottom, unconnected_syms);
                delete_ungrounded_symbol_list(thisAgent, &unconnected_syms);
                unconnected_syms = new matched_symbol_list();
                thisAgent->outputManager->display_soar_feedback(thisAgent, ebc_progress_validating);
                if (reorder_and_validate_lhs_and_rhs(thisAgent, &m_vrblz_top, &m_rhs, false, unconnected_syms))
                {
                    delete_ungrounded_symbol_list(thisAgent, &unconnected_syms);
                    thisAgent->outputManager->display_soar_feedback(thisAgent, ebc_progress_repaired);
                    print_current_built_rule("Repaired rule:");
                    #ifdef BUILD_WITH_EXPLAINER
                    thisAgent->explanationMemory->increment_stat_chunks_repaired();
                    #endif
                    return true;
                }
                #ifdef BUILD_WITH_EXPLAINER
                thisAgent->explanationMemory->increment_stat_could_not_repair();
                #endif
            }
            thisAgent->outputManager->display_soar_feedback(thisAgent, ebc_error_invalid_chunk);
            delete_ungrounded_symbol_list(thisAgent, &unconnected_syms);
            return false;
        }
        delete_ungrounded_symbol_list(thisAgent, &unconnected_syms);
    }
    else if (ebc_settings[SETTING_EBC_REPAIR_JUSTIFICATIONS] || ebc_settings[SETTING_EBC_DONT_ADD_BAD_JUSTIFICATIONS])
    {
        matched_symbol_list* unconnected_syms = new matched_symbol_list();

        reorder_and_validate_lhs_and_rhs(thisAgent, &m_vrblz_top, &m_rhs, false,
            unconnected_syms, ebc_settings[SETTING_EBC_REPAIR_JUSTIFICATIONS], ebc_settings[SETTING_EBC_REPAIR_JUSTIFICATIONS]);

        if (m_failure_type != ebc_success)
        {
            #ifdef BUILD_WITH_EXPLAINER
            thisAgent->explanationMemory->increment_stat_ungrounded_justifications();
            #endif
            if (!ebc_settings[SETTING_EBC_DONT_ADD_BAD_JUSTIFICATIONS] &&
                (((m_failure_type == ebc_failed_unconnected_conditions) && ebc_settings[SETTING_EBC_REPAIR_JUSTIFICATIONS]) ||
                ((m_failure_type == ebc_failed_reordering_rhs) && ebc_settings[SETTING_EBC_REPAIR_JUSTIFICATIONS])))
            {
                thisAgent->outputManager->display_soar_feedback(thisAgent, ebc_progress_repairing);
                Repair_Manager* lRepairManager = new Repair_Manager(thisAgent, m_results_match_goal_level, m_chunk_new_i_id);
                lRepairManager->repair_rule(m_vrblz_top, m_inst_top, m_inst_bottom, unconnected_syms);
                delete_ungrounded_symbol_list(thisAgent, &unconnected_syms);
                unconnected_syms = new matched_symbol_list();
                thisAgent->outputManager->display_soar_feedback(thisAgent, ebc_progress_validating);
                if (reorder_and_validate_lhs_and_rhs(thisAgent, &m_vrblz_top, &m_rhs, false, unconnected_syms))
                {
                    delete_ungrounded_symbol_list(thisAgent, &unconnected_syms);
                    thisAgent->outputManager->display_soar_feedback(thisAgent, ebc_progress_repaired);
                    print_current_built_rule("Repaired rule:");
                    #ifdef BUILD_WITH_EXPLAINER
                    thisAgent->explanationMemory->increment_stat_justifications_repaired();
                    #endif
                    return true;
                }
                #ifdef BUILD_WITH_EXPLAINER
                thisAgent->explanationMemory->increment_stat_could_not_repair();
                #endif
            }
            thisAgent->outputManager->display_soar_feedback(thisAgent, ebc_error_invalid_chunk);
            delete_ungrounded_symbol_list(thisAgent, &unconnected_syms);
            return false;
        }
        delete_ungrounded_symbol_list(thisAgent, &unconnected_syms);
    }
    return true;
}

