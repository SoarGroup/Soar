#include "repair.h"
#include "ebc.h"

#include "agent.h"
#include "condition.h"
#include "explain.h"
#include "preference.h"
#include "slot.h"
#include "test.h"
#include "working_memory.h"
#include "dprint.h"


void delete_ungrounded_symbol_list(symbol_with_match_list** unconnected_syms)
{
    symbol_with_match_list* lSyms = *unconnected_syms;
    for (auto it = lSyms->begin(); it != lSyms->end(); it++)
    {
        if ((*it)->sym)
        {
            (*it)->sym->tc_num = 0;
        }
        delete (*it);
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

    dprint(DT_REPAIR, "Finding path to connect LTI %y (level %d) to a goal state.\n", pNonOperationalSym, pNonOperationalSym->id->level);

    ground_lti_tc = get_new_tc_number(thisAgent);

    Symbol* g = thisAgent->top_goal;
    while (g->id->level < pNonOperationalSym->id->level)
    {
        g = g->id->lower_goal;
    }
    dprint(DT_REPAIR, "...goal %y found for level %d.\n", g, pNonOperationalSym->id->level);

    lNewPath = new Path_to_Goal_State(g);
    ids_to_walk.push_back(lNewPath);
    g->tc_num = ground_lti_tc;

    while (!ids_to_walk.empty())
    {
        if (lCurrentPath) delete lCurrentPath;

        lCurrentPath = ids_to_walk.back();
        ids_to_walk.pop_back();

        if (!final_path) /* We keep iterating after we find the final path, so that  */
        {                /* we can delete the rest of the sym_grounding_path objects */
            dprint(DT_REPAIR, "Walk list += IDs from slots of %y to find %y...\n", lCurrentPath->get_root(), pNonOperationalSym);
            for (slot* s = lCurrentPath->get_root()->id->slots; s != NIL; s = s->next)
            {
                for (wme* w = s->wmes; w != NIL; w = w->next)
                {
                    if (w->value->is_identifier() && (w->value->tc_num != ground_lti_tc))
                    {
                        w->value->tc_num = ground_lti_tc;
                        lNewPath = new Path_to_Goal_State(w->value, lCurrentPath->get_path(), w);
                        if (w->value == pNonOperationalSym)
                        {
                            dprint(DT_REPAIR, "...found path to LTI %y: %w.\n", pNonOperationalSym, w);
                            final_path = new wme_list();
                            (*final_path) = *(lNewPath->get_path());
                        } else {
                            dprint(DT_REPAIR, "      - Adding wme (%y ^%y %y)\n", w->id, w->attr, w->value);
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

condition* Repair_Manager::find_cond_for_unconnected_var(condition* pCondList, Symbol* pUnconnected_LTI)
{
    /* The re-orderer does not give us the matched LTI.  It returns a list of
     * variables that aren't connected, so we need to find a condition that
     * contains the variables, so that we can get the LTI and
     * identity information we need to generate connecting conditions */
    for (condition* cond = pCondList; cond != NIL; cond = cond->next)
    {
        if (cond->type != POSITIVE_CONDITION)
        {
            continue;
        }
        if (cond->data.tests.id_test->eq_test->data.referent == pUnconnected_LTI)
        {
            return cond;
            break;
        }
    }
    return NULL;
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
    dprint(DT_REPAIR, "...adding state link WMEs: \n");
    Symbol* g, *last_goal = NULL;
    wme* w;
//    condition* new_cond;

    g = thisAgent->bottom_goal;
    while (g->id->level > pTargetGoal)
    {
        if (g->tc_num != pSeenTC && (g->id->level < m_match_goal_level))
        {
            assert(last_goal);
            for (w = last_goal->id->impasse_wmes; w != NIL; w = w->next)
            {
                if (w->attr == thisAgent->superstate_symbol)
                {
                    m_repair_WMEs.insert(w);
//                    new_cond = make_condition_from_wme(w);
                    dprint_noprefix(DT_REPAIR, "Adding wme for superstate link: %w \n", w);
                }
            }
        }
        last_goal = g;
        g = g->id->higher_goal;
    }
}

void Repair_Manager::add_path_to_goal_WMEs(symbol_with_match* lUngroundedSym)
{
    dprint(DT_REPAIR, "...searching for path to goal for %y [%y/%u]...\n", lUngroundedSym->matched_sym, lUngroundedSym->sym, lUngroundedSym->identity);
    wme_list* l_WMEPath = find_path_to_goal_for_symbol(lUngroundedSym->matched_sym);
    dprint(DT_REPAIR, "...search complete.  Adding %d wme's to set...\n", l_WMEPath->size());
    for (auto it = l_WMEPath->begin(); it != l_WMEPath->end(); it++)
    {
        wme* lWME = (*it);
        m_repair_WMEs.insert(lWME);
        dprint(DT_REPAIR, "......adding to repair wme set: (%y ^%y %y)\n", lWME->id, lWME->attr, lWME->value);
    }

}
Repair_Manager::Repair_Manager(agent* myAgent, goal_stack_level  p_goal_level)
{
    thisAgent = myAgent;
    m_match_goal_level = p_goal_level;
}

Repair_Manager::~Repair_Manager()
{

}

void Repair_Manager::add_var_for_sym(Symbol* pSym, Symbol* pVar)
{
    m_sym_to_var_map[pSym] = pVar;
}

void Repair_Manager::variablize_connecting_sti(Symbol*& pSym)
{
    char* prefix;
    Symbol* var;

    /* Copy in any identities for the LTI that was used in the unconnected conditions */
    std::unordered_map< Symbol*, Symbol* >::iterator iter_sym;
    iter_sym = m_sym_to_var_map.find(pSym);
    if (iter_sym == m_sym_to_var_map.end())
    {
        /* Create a new variable.  If constant is being variablized just used
         * 'c' instead of first letter of id name.  We now don't use 'o' for
         * non-operators and don't use 's' for non-states.  That makes things
         * clearer in chunks because of standard naming conventions. --- */
        if (pSym->is_identifier())
        {
            char prefix_char = static_cast<char>(tolower(pSym->id->name_letter));
            if (((prefix_char == 's') || (prefix_char == 'S')) && !pSym->id->isa_goal)
            {
                prefix[0] = 'c';
            } else if (((prefix_char == 'o') || (prefix_char == 'O')) && !pSym->id->isa_operator) {
                prefix[0] = 'c';
            } else {
                prefix[0] = prefix_char;
            }
        }
        else
        {
            prefix[0] = 'c';
        }
        prefix[1] = 0;
        var = generate_new_variable(thisAgent, prefix);

        m_sym_to_var_map[pSym] = var;
        symbol_remove_ref(thisAgent, pSym);
        pSym = var;

        /* We may want to set up a fake identity here, but I don't think it will be used
         * after this point, so no point. */
    }
    else
    {
        symbol_remove_ref(thisAgent, pSym);
        pSym = iter_sym->second;
        symbol_add_ref(thisAgent, pSym);
    }
}


condition* Repair_Manager::make_condition_from_wme(wme* lWME)
{

    condition* new_cond;

//    dprint(DT_REPAIR, "Creating condition for %u: (%y ^%y %y)\n", lWME->timetag, lWME->id, lWME->attr, lWME->value);
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

void Repair_Manager::mark_states_in_cond_list(condition* pCondList, tc_number tc)
{
    assert(pCondList);
    condition* lCond;
    Symbol* highest_goal = thisAgent->bottom_goal;
    test lID_test, lValue_test, lInst_ID_test, lInst_Value_test, highest_goal_test = NULL;

    for (lCond = pCondList; lCond; lCond = lCond->next)
    {
        if (lCond->type == POSITIVE_CONDITION)
        {
            if (lCond->data.tests.id_test->eq_test->data.referent->is_identifier())
            {
                if (lCond->data.tests.id_test->eq_test->data.referent->id->isa_goal)
                {
                    lCond->data.tests.id_test->eq_test->data.referent->tc_num = tc;
                    if (lCond->counterpart && lCond->counterpart->data.tests.id_test->eq_test->data.referent->is_variable())
                    {
                        dprint(DT_REPAIR, "Adding state variablization found for %y -> %y\n", lCond->data.tests.id_test->eq_test->data.referent, lCond->counterpart->data.tests.id_test->eq_test->data.referent);
                        m_sym_to_var_map[lCond->data.tests.id_test->eq_test->data.referent] = lCond->counterpart->data.tests.id_test->eq_test->data.referent;
                    }
                }
            } else {
                if (lCond->counterpart &&
                    lCond->counterpart->data.tests.id_test->eq_test->data.referent->is_identifier() &&
                    lCond->counterpart->data.tests.id_test->eq_test->data.referent->id->isa_goal)
                {
                    lCond->counterpart->data.tests.id_test->eq_test->data.referent->tc_num = tc;
                    if (lCond->data.tests.id_test->eq_test->data.referent->is_variable())
                    {
                        dprint(DT_REPAIR, "Adding state variablization found for %y -> %y\n", lCond->counterpart->data.tests.id_test->eq_test->data.referent, lCond->data.tests.id_test->eq_test->data.referent);
                        m_sym_to_var_map[lCond->counterpart->data.tests.id_test->eq_test->data.referent] = lCond->data.tests.id_test->eq_test->data.referent;
                    }
                }
            }
            if (lCond->data.tests.value_test->eq_test->data.referent->is_identifier())
            {
                if (lCond->data.tests.value_test->eq_test->data.referent->id->isa_goal)
                {
                    lCond->data.tests.value_test->eq_test->data.referent->tc_num = tc;
                    if (lCond->counterpart && lCond->counterpart->data.tests.value_test->eq_test->data.referent->is_variable())
                    {
                        dprint(DT_REPAIR, "Adding state variablization found for %y -> %y\n", lCond->data.tests.value_test->eq_test->data.referent, lCond->counterpart->data.tests.value_test->eq_test->data.referent);
                        m_sym_to_var_map[lCond->data.tests.value_test->eq_test->data.referent] = lCond->counterpart->data.tests.value_test->eq_test->data.referent;
                    }
                }
            } else {
                if (lCond->counterpart &&
                    lCond->counterpart->data.tests.value_test->eq_test->data.referent->is_identifier() &&
                    lCond->counterpart->data.tests.value_test->eq_test->data.referent->id->isa_goal)
                {
                    lCond->counterpart->data.tests.value_test->eq_test->data.referent->tc_num = tc;
                    if (lCond->data.tests.value_test->eq_test->data.referent->is_variable())
                    {
                        dprint(DT_REPAIR, "Adding state variablization found for %y -> %y\n", lCond->counterpart->data.tests.value_test->eq_test->data.referent, lCond->data.tests.value_test->eq_test->data.referent);
                        m_sym_to_var_map[lCond->counterpart->data.tests.value_test->eq_test->data.referent] = lCond->data.tests.value_test->eq_test->data.referent;
                    }
                }
            }
        }
    }
}

void Repair_Manager::repair_rule(condition*& m_vrblz_top, condition*& m_inst_top, condition*& m_inst_bottom, symbol_with_match_list* p_dangling_syms, uint64_t pInstID)
{
    symbol_with_match* lUngroundedSymInfo;
    wme* lWME;
    goal_stack_level targetLevel;

    #ifdef BUILD_WITH_EXPLAINER
    thisAgent->explanationLogger->increment_stat_grounded(m_repair_WMEs.size());
    #endif

    dprint(DT_REPAIR, "Repair rule started...\n");
    dprint(DT_VARIABLIZATION_MANAGER, "- Variablized cond: \n%1", m_vrblz_top);
    dprint(DT_CONSTRAINTS, "\n- Instantiated conds :\n%1", m_inst_top, NULL);

    /* Generate connecting wme's for each unconnected LTI and add to a set */
    targetLevel = thisAgent->bottom_goal->id->level;
    for (auto it = p_dangling_syms->begin(); it != p_dangling_syms->end(); it++)
    {
        lUngroundedSymInfo = *it;
        if(lUngroundedSymInfo->matched_sym->id->level < targetLevel)
        {
            targetLevel = lUngroundedSymInfo->matched_sym->id->level;
            dprint(DT_REPAIR, "Adding state variablization found for %y -> %y\n", lUngroundedSymInfo->matched_sym, lUngroundedSymInfo->sym);
            m_sym_to_var_map[lUngroundedSymInfo->matched_sym] = lUngroundedSymInfo->sym;
        }
    }

    tc_number tc;
    tc = get_new_tc_number(thisAgent);

    mark_states_in_cond_list(m_vrblz_top, tc);
    reset_variable_generator(thisAgent, m_vrblz_top, NULL);
    add_state_link_WMEs(targetLevel, tc);

    /* Generate connecting wme's for each unconnected LTI and add to a set */
    for (auto it = p_dangling_syms->begin(); it != p_dangling_syms->end(); it++)
    {
        lUngroundedSymInfo = *it;
        add_path_to_goal_WMEs(lUngroundedSymInfo);
    }

    /* Create conditions based on set of wme's compiled */
    condition* new_cond, *new_inst_cond, *prev_cond = m_vrblz_top, *first_cond = m_vrblz_top;
    while (prev_cond->next != NULL)
        prev_cond = prev_cond->next;

//    prev_cond = first_cond = NULL;

    dprint(DT_REPAIR, "Final set of WMEs to connect all dangling identifiers: \n");
    for (auto it = m_repair_WMEs.begin(); it != m_repair_WMEs.end(); it++)
    {
        lWME = (*it);

        new_cond = make_condition_from_wme(lWME);

        new_inst_cond = copy_condition(thisAgent, new_cond);
        new_cond->counterpart = new_inst_cond;
        new_inst_cond->counterpart = new_cond;
        dprint(DT_REPAIR, "Variablizing condition %l.\n", new_cond);
        /* Variablize and add to condition list */
        variablize_connecting_sti(new_cond->data.tests.id_test->data.referent);
        variablize_connecting_sti(new_cond->data.tests.value_test->data.referent);
        dprint(DT_REPAIR, "Variablized condition %l.\n", new_cond);
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

