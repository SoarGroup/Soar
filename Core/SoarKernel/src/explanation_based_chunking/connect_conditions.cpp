
#include "ebc.h"

#include "agent.h"
#include "condition.h"
#include "connect_conditions.h"
#include "debug.h"
#include "slot.h"
#include "test.h"
#include "working_memory.h"

wme_list* Explanation_Based_Chunker::walk_and_find_lti(Symbol* root, Symbol* targetLTI)
{
    sym_grounding_path_list ids_to_walk;
    sym_grounding_path* lCurrentPath = NULL, *lNewPath = NULL;
    wme_list* final_path = NULL;

    lNewPath = new sym_grounding_path(root);
    ids_to_walk.push_back(lNewPath);
    root->tc_num = ground_lti_tc;

    while (!ids_to_walk.empty())
    {
        if (lCurrentPath) delete lCurrentPath;

        lCurrentPath = ids_to_walk.back();
        ids_to_walk.pop_back();

        if (!final_path) /* We keep iterating after we find the final path, so that  */
        {                /* we can delete the rest of the sym_grounding_path objects */
//            dprint(DT_GROUND_LTI, "Adding IDs from slots of %y to walk list to find %y...\n", lCurrentPath->get_root(), targetLTI);
            for (slot* s = lCurrentPath->get_root()->id->slots; s != NIL; s = s->next)
            {
                for (wme* w = s->wmes; w != NIL; w = w->next)
                {
                    if (w->value->is_identifier() && (w->value->tc_num != ground_lti_tc))
                    {
                        w->value->tc_num = ground_lti_tc;
                        lNewPath = new sym_grounding_path(w->value, lCurrentPath->get_path(), w);
                        if (w->value == targetLTI)
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

void Explanation_Based_Chunker::generate_conditions_to_ground_lti(condition** pCondList, Symbol* pUnconnected_LTI)
{
    dprint(DT_GROUND_LTI, "Finding path to connect LTI %y to a goal state.\n", pUnconnected_LTI);

    /* The re-orderer returns a list of variables that aren't connected, so we
     * need to find a condition that uses it, so that we can get the LTI and
     * identity information we need to generate connecting conditions */
    test ltiEqTest = NULL, ltiMatchEqTest = NULL;
    for (condition* cond = (*pCondList); cond != NIL; cond = cond->next)
    {
        if (cond->type != POSITIVE_CONDITION)
        {
            continue;
        }
        if (cond->data.tests.id_test->eq_test->data.referent == pUnconnected_LTI)
        {
            ltiEqTest = cond->data.tests.id_test->eq_test;
            ltiMatchEqTest = cond->counterpart->data.tests.id_test->eq_test;
            break;
        }
    }
    assert(ltiEqTest);

    ground_lti_tc = get_new_tc_number(thisAgent);

    Symbol* g = thisAgent->top_goal;
    while (g->id->level < ltiMatchEqTest->data.referent->id->level)
    {
        g = g->id->lower_goal;
    }

    wme_list* l_WMEPath = walk_and_find_lti(g, ltiMatchEqTest->data.referent);

    for (auto it = l_WMEPath->rbegin(); it != l_WMEPath->rend(); it++) {
        dprint(DT_GROUND_LTI, "      (%y ^%y %y)\n", (*it)->id, (*it)->attr, (*it)->value);
    }

    /* Create conditions based on wme_list returned */

//    condition* return_conditions;
//
//    return_conditions = NULL;
//
//    for (auto it = l_WMEPath->rbegin(); it != l_WMEPath->rend(); it++) {
//        thisAgent->memoryManager->allocate_with_pool(MP_condition, &cond);
//        init_condition(cond);
//        cond->data.tests.id_test = make_test(thisAgent, ap_wme->id, EQUALITY_TEST);
//        cond->data.tests.id_test->identity = thisAgent->ebChunker->get_or_create_o_id(thisAgent->ss_context_variable, inst->i_id);
//        cond->data.tests.attr_test = make_test(thisAgent, ap_wme->attr, EQUALITY_TEST);
//        cond->data.tests.value_test = make_test(thisAgent, ap_wme->value, EQUALITY_TEST);
//        cond->data.tests.value_test->identity = thisAgent->ebChunker->get_or_create_o_id(thisAgent->o_context_variable, inst->i_id);
//        dprint(DT_GROUND_LTI, "      (%y ^%y %y)\n", (*it)->id, (*it)->attr, (*it)->value);
//    }
}

