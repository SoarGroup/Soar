#include "repair.h"
#include "ebc.h"

#include "agent.h"
#include "condition.h"
#include "explain.h"
#include "preference.h"
#include "slot.h"
#include "test.h"
#include "working_memory.h"
#include "../debug_code/dprint.h"

wme_list* Explanation_Based_Chunker::find_wmes_to_ground_lti(Symbol* pUnconnected_LTI)
{
    sym_grounding_path_list ids_to_walk;
    sym_grounding_path* lCurrentPath = NULL, *lNewPath = NULL;
    wme_list* final_path = NULL;

    dprint(DT_GROUND_LTI, "Finding path to connect LTI %y to a goal state.\n", pUnconnected_LTI);

    ground_lti_tc = get_new_tc_number(thisAgent);

    Symbol* g = thisAgent->top_goal;
    while (g->id->level < pUnconnected_LTI->id->level)
    {
        g = g->id->lower_goal;
    }

    lNewPath = new sym_grounding_path(g);
    ids_to_walk.push_back(lNewPath);
    g->tc_num = ground_lti_tc;

    while (!ids_to_walk.empty())
    {
        if (lCurrentPath) delete lCurrentPath;

        lCurrentPath = ids_to_walk.back();
        ids_to_walk.pop_back();

        if (!final_path) /* We keep iterating after we find the final path, so that  */
        {                /* we can delete the rest of the sym_grounding_path objects */
//            dprint(DT_GROUND_LTI, "Adding IDs from slots of %y to walk list to find %y...\n", lCurrentPath->get_root(), pUnconnected_LTI);
            for (slot* s = lCurrentPath->get_root()->id->slots; s != NIL; s = s->next)
            {
                for (wme* w = s->wmes; w != NIL; w = w->next)
                {
                    if (w->value->is_identifier() && (w->value->tc_num != ground_lti_tc))
                    {
                        w->value->tc_num = ground_lti_tc;
                        lNewPath = new sym_grounding_path(w->value, lCurrentPath->get_path(), w);
                        if (w->value == pUnconnected_LTI)
                        {
                            dprint(DT_GROUND_LTI, "...found path to LTI.\n");
                            final_path = new wme_list();
                            (*final_path) = *(lNewPath->get_path());
                        } else {
//                            dprint(DT_GROUND_LTI, "      - Adding path through (%y ^%y %y)\n", w->id, w->attr, w->value);
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

condition* Explanation_Based_Chunker::find_lti_that_matched_var(condition* pCondList, Symbol* pUnconnected_LTI)
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

void Explanation_Based_Chunker::generate_conditions_to_ground_lti(symbol_list* pUnconnected_LTIs, uint64_t pInstID)
{

    test ltiEqTest = NULL, ltiMatchEqTest = NULL;
    condition* lCond;
    wme_set lConditionWMEs;
    std::unordered_map< Symbol*, uint64_t > lti_to_identity_map;
    std::unordered_map< Symbol*, uint64_t >::iterator iter_sym;
    uint64_t lIdentity;

    /* Generate connecting wme's for each unconnected LTI and add to a set */
    for (auto it = pUnconnected_LTIs->begin(); it != pUnconnected_LTIs->end(); it++)
    {
        lCond = find_lti_that_matched_var(m_vrblz_top, (*it));
        ltiEqTest = lCond->data.tests.id_test->eq_test;
        ltiMatchEqTest = lCond->counterpart->data.tests.id_test->eq_test;
        wme_list* l_WMEPath = find_wmes_to_ground_lti(ltiMatchEqTest->data.referent);
        for (auto it = l_WMEPath->begin(); it != l_WMEPath->end(); it++)
        {
            lConditionWMEs.insert((*it));
            lti_to_identity_map[ltiMatchEqTest->data.referent] = ltiEqTest->identity;
            dprint(DT_GROUND_LTI, "Adding wme to connecting condition wme set: (%y ^%y %y)\n", (*it)->id, (*it)->attr, (*it)->value);
        }
    }
    #ifdef BUILD_WITH_EXPLAINER
    thisAgent->explanationLogger->increment_stat_grounded(lConditionWMEs.size());
    #endif
    /* Create conditions based on set of wme's compiled */
    condition* new_cond, *new_inst_cond, *prev_cond = m_vrblz_top, *first_cond = m_vrblz_top;
    while (prev_cond->next != NULL)
        prev_cond = prev_cond->next;

//    prev_cond = first_cond = NULL;

    dprint(DT_GROUND_LTI, "Final set: \n");
    for (auto it = lConditionWMEs.begin(); it != lConditionWMEs.end(); it++)
    {
        dprint(DT_GROUND_LTI, "Creating condition for %u: (%y ^%y %y)\n", (*it)->timetag, (*it)->id, (*it)->attr, (*it)->value);
        new_cond = make_condition(thisAgent,
            make_test(thisAgent, (*it)->id, EQUALITY_TEST),
            make_test(thisAgent, (*it)->attr, EQUALITY_TEST),
            make_test(thisAgent, (*it)->value, EQUALITY_TEST));
        new_cond->test_for_acceptable_preference = (*it)->acceptable;
        new_cond->bt.wme_ = (*it);
        new_cond->bt.level = (*it)->id->id->level;
        new_cond->bt.trace = (*it)->preference;
        new_cond->inst = (*it)->preference->inst;

        /* In other functions we only add a reference if the instantiation match goal level is
         * not the top level.  We don't have that value yet, so I'm going to try to use the level
         * of the wme itself. */
#ifndef DO_TOP_LEVEL_REF_CTS
        if (new_cond->bt.level > TOP_GOAL_LEVEL)
#endif
        {
            wme_add_ref((*it));
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

        assert(new_cond->bt.wme_->preference = new_cond->bt.trace);

        /* Copy in any identities for the LTI that was used in the unconnected conditions */
        iter_sym = lti_to_identity_map.find((*it)->id);
        if (iter_sym != lti_to_identity_map.end())
        {
            new_cond->data.tests.id_test->identity = iter_sym->second;
        }
        iter_sym = lti_to_identity_map.find((*it)->value);
        if (iter_sym != lti_to_identity_map.end())
        {
            new_cond->data.tests.value_test->identity = iter_sym->second;
        }

        new_inst_cond = copy_condition(thisAgent, new_cond);
        new_cond->counterpart = new_inst_cond;
        new_inst_cond->counterpart = new_cond;

        /* Variablize and add to condition list */
        variablize_equality_tests(new_cond->data.tests.id_test);
        variablize_equality_tests(new_cond->data.tests.value_test);
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

    dprint(DT_GROUND_LTI, "Final conditions: \n%1\n%1", m_vrblz_top, m_inst_top);
}

