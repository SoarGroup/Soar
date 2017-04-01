#include "instantiation_record.h"

#include "action_record.h"
#include "agent.h"
#include "condition_record.h"
#include "condition.h"
#include "dprint.h"
#include "explanation_memory.h"
#include "instantiation.h"
#include "output_manager.h"
#include "preference.h"
#include "production_record.h"
#include "production.h"
#include "rete.h"
#include "rhs.h"
#include "symbol_manager.h"
#include "symbol.h"
#include "test.h"
#include "working_memory.h"
#include "visualize.h"

void instantiation_record::init(agent* myAgent, instantiation* pInst)
{
    thisAgent               = myAgent;
    instantiationID         = pInst->i_id;
    cached_inst             = pInst;
    original_productionID   = pInst->prod ? thisAgent->explanationMemory->add_production_id_if_necessary(pInst->prod) : 0;
    excised_production      = NULL;
    creating_chunk          = 0;
    match_level             = pInst->match_goal_level;
    terminal                = false;
    path_to_base            = NULL;
    lhs_identities          = NULL;

    conditions              = new condition_record_list;
    actions                 = new action_record_list;

    production_name         = pInst->prod_name;
    thisAgent->symbolManager->symbol_add_ref(production_name);

    if (pInst->prod)
    {
        pInst->prod->save_for_justification_explanation = true;
    }

    action_record* new_action_record;
    for (preference* pref = pInst->preferences_generated; pref != NIL; pref = pref->inst_next)
    {
        new_action_record = thisAgent->explanationMemory->add_result(pref);
        actions->push_front(new_action_record);
        //actions->push_back(new_action_record);
    }
    for (preference* pref = pInst->preferences_cached; pref != NIL; pref = pref->inst_next)
    {
        new_action_record = thisAgent->explanationMemory->add_result(pref);
        actions->push_front(new_action_record);
        //actions->push_back(new_action_record);
    }

}

void instantiation_record::clean_up()
{
    dprint(DT_EXPLAIN, "Deleting instantiation record i%u (%y)\n", instantiationID, production_name);
    thisAgent->symbolManager->symbol_remove_ref(&production_name);
    delete conditions;
    delete actions;
    dprint(DT_EXPLAIN, "Done deleting instantiation record i%u (%y)\n", instantiationID, production_name);
    production* originalProduction = thisAgent->explanationMemory->get_production(original_productionID);
    if (originalProduction)
    {
        originalProduction->save_for_justification_explanation = false;
    }
    if (path_to_base)
    {
        delete path_to_base;
    }
}

void instantiation_record::delete_instantiation()
{
    for (auto it = conditions->begin(); it != conditions->end(); it++)
    {
        thisAgent->explanationMemory->delete_condition((*it)->get_conditionID());
    }
    for (auto it = actions->begin(); it != actions->end(); it++)
    {
        thisAgent->explanationMemory->delete_action((*it)->get_actionID());
    }
}

void instantiation_record::record_instantiation_contents()
{
    dprint(DT_EXPLAIN_ADD_INST, "- Recording instantiation contents for i%u (%y)\n", cached_inst->i_id, production_name);
    /* Create condition and action records */
    for (condition* cond = cached_inst->top_of_instantiated_conditions; cond != NIL; cond = cond->next)
    {
        condition_record* lCondRecord = thisAgent->explanationMemory->add_condition(conditions, cond, this);
        lCondRecord->connect_to_action();
    }
}


void instantiation_record::viz_connect_conditions()
{
    condition_record* lCondRecord;
    for (auto it = conditions->begin(); it != conditions->end(); it++)
    {
        lCondRecord = (*it);
        lCondRecord->viz_connect_to_action(this->match_level);
    }
}

void instantiation_record::update_instantiation_contents()
{
    dprint(DT_EXPLAIN_UPDATE, "- Updating instantiation contents for i%u (%y)\n", cached_inst->i_id, production_name);
    /* Update condition and action records */
    condition_record* lCondRecord;
    condition* cond;

    cond = cached_inst->top_of_instantiated_conditions;
    for (condition_record_list::iterator it = conditions->begin(); it != conditions->end() && cond != NIL; it++, cond = cond->next)
    {
        lCondRecord = (*it);
        lCondRecord->update_condition(cond, this);
        lCondRecord->connect_to_action();
    }
}

action_record* instantiation_record::find_rhs_action(preference* pPref)
{
    action_record_list::iterator iter;
    action_record* pAct;

    for (iter = actions->begin(); iter != actions->end(); ++iter)
    {
        pAct = *iter;
        if (pAct->original_pref == pPref)
        {
            dprint(DT_EXPLAIN_CONDS, "...found RHS action a%u for condition preference %p.\n", pAct->get_actionID(), pPref);
            return (*iter);
        }
    }
    dprint(DT_EXPLAIN_CONNECT, "...did not find pref %p among:\n", pPref);
    for (iter = actions->begin(); iter != actions->end(); ++iter)
    {
        pAct = *iter;
        dprint(DT_EXPLAIN_CONNECT, "      %p\n", pAct->original_pref);
    }
    return NULL;
}

condition_record* instantiation_record::find_condition_for_chunk(preference* pPref, wme* pWME)
{
    dprint(DT_EXPLAIN_PATHS, "Looking for condition in i%u: ", instantiationID);
    if (pPref)
    {
        dprint_noprefix(DT_EXPLAIN_PATHS, " pref %p", pPref);
    }
    if (pWME)
    {
        dprint_noprefix(DT_EXPLAIN_PATHS, " wme %w", pWME);
    }
    dprint_noprefix(DT_EXPLAIN_PATHS, "\n");
    for (condition_record_list::iterator it = conditions->begin(); it != conditions->end(); it++)
    {
        dprint(DT_EXPLAIN_PATHS, "Comparing against condition %u", (*it)->get_conditionID());
        if (pWME && ((*it)->get_cached_wme() == pWME))
        {
            dprint(DT_EXPLAIN_PATHS, "Found condition %u %w for target wme %w\n", (*it)->get_conditionID(), (*it)->get_cached_wme(), pWME);
            return (*it);
        }
        else if (pPref && ((*it)->get_cached_pref() == pPref))
        {
            /* Don't think we can ever have a wme without a pref */
            assert(false);
            dprint(DT_EXPLAIN_PATHS, "Found condition %u %p for target preference %p\n", (*it)->get_conditionID(), (*it)->get_cached_pref(), pPref);
            return (*it);
        }
    }
    //    assert(false);
    return NULL;
}

void instantiation_record::create_identity_paths(const inst_record_list* pInstPath)
{
    if (!path_to_base)
    {
        path_to_base = new inst_record_list();
    }
    else {
        int size1 = path_to_base->size();
        int size2 = pInstPath->size();
        if (path_to_base->size() <= pInstPath->size())
        {
            return;
        }
    }

    (*path_to_base) = (*pInstPath);
    path_to_base->push_back(this);
    instantiation_record* lParentInst;
    dprint(DT_EXPLAIN_PATHS, "Adding paths recursively for conditions of i%u (%y)...\n", instantiationID, production_name);
    for (condition_record_list::iterator it = conditions->begin(); it != conditions->end(); it++)
    {
        lParentInst = (*it)->get_parent_inst();
        if (lParentInst && (lParentInst->get_match_level() == match_level))
        {
            //            if ((match_level > 0) && ((*it)->get_level() < match_level))
            //            {
            lParentInst->create_identity_paths(path_to_base);
            //            } else {
            //                dprint(DT_EXPLAIN_PATHS, "...not recursing because match level is 0 or condition level < match level\n");
            //                dprint(DT_EXPLAIN_PATHS, "...%d >= %d...\n", static_cast<int64_t>(match_level), static_cast<int64_t>((*it)->get_level()));
            //            }
            //            path_to_base->pop_back();
            dprint(DT_EXPLAIN_PATHS, "...creating identity path for i%u (%y) at level %d\n", lParentInst->instantiationID, lParentInst->production_name, static_cast<int64_t>(match_level));
        } else {
            dprint(DT_EXPLAIN_PATHS, "...not recursing because %s: %d%s%d\n", lParentInst ? "parent inst level != match level for " : "no parent instantiation! ", (lParentInst ? static_cast<int64_t>(lParentInst->get_match_level()) : 0), (lParentInst ? " != " : " "), static_cast<int64_t>(match_level));
            condition_record* lCond = (*it);
            dprint(DT_EXPLAIN_PATHS, "   pref: %p\n", (*it)->cached_pref);
            dprint(DT_EXPLAIN_PATHS, "   tests: (%t ^%t %t)\n", lCond->condition_tests.id, lCond->condition_tests.attr, lCond->condition_tests.value);
        }
    }
}

id_set* instantiation_record::get_lhs_identities()
{
    if (lhs_identities) return lhs_identities;
    lhs_identities = new id_set();
    condition* top, *bottom;
    production* originalProduction = thisAgent->explanationMemory->get_production(original_productionID);
    if (!originalProduction || !originalProduction->p_node)
    {
        if (excised_production)
        {
            top = excised_production->get_lhs();
            assert(top);
        } else {
            thisAgent->outputManager->printa_sf(thisAgent, "%eError:  Cannot generate identity analysis this instantiation.  Original rule conditions no longer in RETE.\n");
        }
    } else {
        p_node_to_conditions_and_rhs(thisAgent, originalProduction->p_node, NIL, NIL, &top, &bottom, NULL);
    }
    return lhs_identities;
}
void instantiation_record::print_for_wme_trace(bool printFooter)
{
    Output_Manager* outputManager = thisAgent->outputManager;

    if (conditions->empty())
    {
        outputManager->printa(thisAgent, "No conditions on left-hand-side\n");
    }
    else
    {
        condition_record* lCond;
        bool lInNegativeConditions = false;
        int lConditionCount = 0;
        action* rhs;
        test id_test_without_goal_test = NULL;

        outputManager->set_column_indent(0, 7);
        outputManager->set_column_indent(1, 57);
        outputManager->set_column_indent(2, 72);
        /* Print header */
        outputManager->printa_sf(thisAgent, "Working memory trace of instantiation # %u %-(match of rule %y at level %d)\n",
            instantiationID, production_name, static_cast<int64_t>(match_level));
        outputManager->printa_sf(thisAgent, "%- %-Operational %-Creator\n\n");
        outputManager->set_print_test_format(false, true);

        for (condition_record_list::iterator it = conditions->begin(); it != conditions->end(); it++)
        {
            lCond = (*it);
            ++lConditionCount;
            if (lInNegativeConditions)
            {
                if (lCond->type != CONJUNCTIVE_NEGATION_CONDITION)
                {
                    outputManager->printa(thisAgent, "     }\n");
                    lInNegativeConditions = false;
                }
            } else {
                if (lCond->type == CONJUNCTIVE_NEGATION_CONDITION)
                {
                    outputManager->printa(thisAgent, "     -{\n");
                    lInNegativeConditions = true;
                }
            }
            outputManager->printa_sf(thisAgent, "%d:%-", lConditionCount);

            id_test_without_goal_test = copy_test(thisAgent, lCond->condition_tests.id, false, false, true);

            outputManager->printa_sf(thisAgent, "(%t%s^%t %t%s)%-",
                id_test_without_goal_test, ((lCond->type == NEGATIVE_CONDITION) ? " -" : " "),
                lCond->condition_tests.attr, lCond->condition_tests.value,
                lCond->test_for_acceptable_preference ? " +" : "");
            deallocate_test(thisAgent, id_test_without_goal_test);

            bool isSuper = (match_level > 0) && (lCond->wme_level_at_firing < match_level);
            outputManager->printa_sf(thisAgent, "%s", (isSuper ? "    Yes" : "    No"));
            if (lCond->parent_instantiation)
            {
                outputManager->printa_sf(thisAgent, "%-i %u (%y)%-",
                    (lCond->parent_instantiation->instantiationID),
                    (lCond->parent_instantiation->production_name));
            } else if (lCond->type == POSITIVE_CONDITION)
            {
                outputManager->printa_sf(thisAgent, isSuper ? "%-Higher-level Problem Space%-" : "%-Soar Architecture%-");
            } else {
                outputManager->printa_sf(thisAgent, "%-N/A%-");
            }
            outputManager->printa(thisAgent, "\n");
        }
        if (lInNegativeConditions)
        {
            outputManager->printa(thisAgent, "     }\n");
        }
        outputManager->printa(thisAgent, "   -->\n");
        thisAgent->explanationMemory->print_instantiation_actions(actions, thisAgent->explanationMemory->get_production(original_productionID), rhs);
        if (printFooter) {
            thisAgent->explanationMemory->print_footer();
        }
    }
}
void instantiation_record::print_for_explanation_trace(bool printFooter)
{
    Output_Manager* outputManager = thisAgent->outputManager;

    if (conditions->empty())
    {
        outputManager->printa(thisAgent, "No conditions on left-hand-side\n");
    }
    else
    {
        condition_record* lCond;
        bool lInNegativeConditions = false;
        int lConditionCount = 0;
        action* rhs;
        condition* top, *bottom, *currentNegativeCond, *current_cond, *print_cond;
        production* originalProduction = thisAgent->explanationMemory->get_production(original_productionID);

        /* If we're printing the explanation trace, we reconstruct the conditions.  We need to do this
         * because we don't want to cache the explanation trace's original symbols every time we create an instantiation.
         * We used to and it's very inefficient.  We also can't use the ebChunker's identity lookup table because that
         * is only for debugging and does not get built for releases. */
        if (!originalProduction || !originalProduction->p_node)
        {
            if (excised_production)
            {
                top = excised_production->get_lhs();
                rhs = excised_production->get_rhs();
                assert(top);
                assert(rhs);
            } else {
                outputManager->printa_sf(thisAgent, "Explanation trace of instantiation # %u %-(match of rule %y at level %d)\n",
                    instantiationID, production_name, match_level);
                outputManager->printa_sf(thisAgent,
                    "\nWarning:  Cannot print explanation trace for this instantiation because no underlying\n"
                    "            rule found in RETE.  Printing working memory trace instead.\n\n");
                thisAgent->explanationMemory->print_explanation_trace = false;
//                print_for_wme_trace(printFooter);
                print_arch_inst_for_explanation_trace(printFooter);
                thisAgent->explanationMemory->print_explanation_trace = true;
                return;
            }
        } else {
            p_node_to_conditions_and_rhs(thisAgent, originalProduction->p_node, NIL, NIL, &top, &bottom, &rhs);
        }
        current_cond = top;
        if (current_cond->type == CONJUNCTIVE_NEGATION_CONDITION)
        {
            currentNegativeCond = current_cond->data.ncc.top;
        } else {
            currentNegativeCond = NULL;
        }
        /* Print header */
        outputManager->set_column_indent(0, 7);
        outputManager->set_column_indent(1, 57);
        outputManager->set_column_indent(2, 100);
        outputManager->set_column_indent(3, 115);
        thisAgent->outputManager->set_print_test_format(true, false);
        outputManager->printa_sf(thisAgent, "Explanation trace of instantiation # %u %-(match of rule %y at level %d)\n",
            instantiationID, production_name, match_level);
        thisAgent->explanationMemory->print_path_to_base(path_to_base, false, " (produced chunk result)", "- Shortest path to a result: ");
        outputManager->printa_sf(thisAgent, "%- %-Identities instead of variables %-Operational %-Creator\n\n");

        for (condition_record_list::iterator it = conditions->begin(); it != conditions->end(); it++)
        {
            lCond = (*it);
            ++lConditionCount;
            if (!lInNegativeConditions && (lCond->type == CONJUNCTIVE_NEGATION_CONDITION))
            {
                outputManager->printa(thisAgent, "     -{\n");
                lInNegativeConditions = true;
            }

            outputManager->printa_sf(thisAgent, "%d:%-", lConditionCount);

            /* Get the next condition from the explanation trace.  This is tricky because NCCs are condition lists within condition lists */
            if (currentNegativeCond)
            {
                print_cond = currentNegativeCond;
            } else {
                print_cond = current_cond;
            }
            outputManager->printa_sf(thisAgent, "(%t%s^%t %t%s)%-",
                print_cond->data.tests.id_test, ((lCond->type == NEGATIVE_CONDITION) ? " -" : " "),
                print_cond->data.tests.attr_test, print_cond->data.tests.value_test,
                print_cond->test_for_acceptable_preference ? " +" : "");
            outputManager->printa_sf(thisAgent, "(%g%s^%g %g%s)%-",
                lCond->condition_tests.id, ((lCond->type == NEGATIVE_CONDITION) ? " -" : " "),
                lCond->condition_tests.attr, lCond->condition_tests.value,
                lCond->test_for_acceptable_preference ? " +" : "");

            bool isSuper = (match_level > 0) && (lCond->wme_level_at_firing < match_level);
            outputManager->printa_sf(thisAgent, "%s", (isSuper ? "    Yes" : "    No"));
            if (lCond->parent_instantiation)
            {
                outputManager->printa_sf(thisAgent, "%-i %u (%y)%-",
                    (lCond->parent_instantiation->instantiationID),
                    (lCond->parent_instantiation->production_name));
            } else if (lCond->type == POSITIVE_CONDITION)
            {
                outputManager->printa_sf(thisAgent, isSuper ? "%-Higher-level Problem Space%-" : "%-Soar Architecture%-");
            } else {
                outputManager->printa_sf(thisAgent, "%-N/A%-");
            }
            if (currentNegativeCond)
            {
                currentNegativeCond = currentNegativeCond->next;
                if (!currentNegativeCond)
                {
                    current_cond = current_cond->next;
                    outputManager->printa(thisAgent, "\n     }");
                    lInNegativeConditions = false;
                }
            } else {
                current_cond = current_cond->next;
            }
            if (current_cond && (current_cond->type == CONJUNCTIVE_NEGATION_CONDITION) && !currentNegativeCond)
            {
                currentNegativeCond = current_cond->data.ncc.top;
            }
            outputManager->printa(thisAgent, "\n");
        }
        if (lInNegativeConditions)
        {
            outputManager->printa(thisAgent, "     }\n");
        }
        outputManager->printa(thisAgent, "   -->\n");
        thisAgent->explanationMemory->print_instantiation_actions(actions, originalProduction, rhs);
        outputManager->printa(thisAgent, "\n");
        thisAgent->explanationMemory->current_discussed_chunk->identity_analysis.print_instantiation_mappings(instantiationID);
        if (printFooter) {
            thisAgent->explanationMemory->print_footer();
        }

        if (original_productionID && originalProduction->p_node)
        {
            deallocate_condition_list(thisAgent, top);
        }
    }
}
void instantiation_record::print_arch_inst_for_explanation_trace(bool printFooter)
{

    Output_Manager* outputManager = thisAgent->outputManager;

    if (conditions->empty())
    {
        outputManager->printa(thisAgent, "No conditions on left-hand-side\n");
    }
    else
    {
        condition_record* lCond;
        bool lInNegativeConditions = false;
        int lConditionCount = 0;
        action* rhs;
        production* originalProduction = thisAgent->explanationMemory->get_production(original_productionID);

        /* Print header */
        outputManager->set_column_indent(0, 7);
        outputManager->set_column_indent(1, 57);
        outputManager->set_column_indent(2, 100);
        outputManager->set_column_indent(3, 115);
        thisAgent->outputManager->set_print_test_format(true, false);
        outputManager->printa_sf(thisAgent, "Explanation trace of instantiation # %u %-(match of rule %y at level %d)\n",
            instantiationID, production_name, match_level);
        thisAgent->explanationMemory->print_path_to_base(path_to_base, false, " (produced chunk result)", "- Shortest path to a result: ");
        outputManager->printa_sf(thisAgent, "%- %-Identities instead of variables %-Operational %-Creator\n\n");

        for (condition_record_list::iterator it = conditions->begin(); it != conditions->end(); it++)
        {
            lCond = (*it);
            ++lConditionCount;
            if (!lInNegativeConditions && (lCond->type == CONJUNCTIVE_NEGATION_CONDITION))
            {
                outputManager->printa(thisAgent, "     -{\n");
                lInNegativeConditions = true;
            } else if (lInNegativeConditions && (lCond->type != CONJUNCTIVE_NEGATION_CONDITION))
            {
                outputManager->printa(thisAgent, "\n     }");
                lInNegativeConditions = false;
            }

            outputManager->printa_sf(thisAgent, "%d:%-", lConditionCount);

            outputManager->printa_sf(thisAgent, "(%t%s^%t %t%s)%-",
                lCond->condition_tests.id, ((lCond->type == NEGATIVE_CONDITION) ? " -" : " "),
                lCond->condition_tests.attr, lCond->condition_tests.value,
                lCond->test_for_acceptable_preference ? " +" : "");
            outputManager->printa_sf(thisAgent, "(%g%s^%g %g%s)%-",
                lCond->condition_tests.id, ((lCond->type == NEGATIVE_CONDITION) ? " -" : " "),
                lCond->condition_tests.attr, lCond->condition_tests.value,
                lCond->test_for_acceptable_preference ? " +" : "");

            bool isSuper = (match_level > 0) && (lCond->wme_level_at_firing < match_level);
            outputManager->printa_sf(thisAgent, "%s", (isSuper ? "    Yes" : "    No"));
            if (lCond->parent_instantiation)
            {
                outputManager->printa_sf(thisAgent, "%-i %u (%y)%-",
                    (lCond->parent_instantiation->instantiationID),
                    (lCond->parent_instantiation->production_name));
            } else if (lCond->type == POSITIVE_CONDITION)
            {
                outputManager->printa_sf(thisAgent, isSuper ? "%-Higher-level Problem Space%-" : "%-Soar Architecture%-");
            } else {
                outputManager->printa_sf(thisAgent, "%-N/A%-");
            }

            outputManager->printa(thisAgent, "\n");
        }
        if (lInNegativeConditions)
        {
            outputManager->printa(thisAgent, "     }\n");
        }
        outputManager->printa(thisAgent, "   -->\n");
        thisAgent->explanationMemory->print_instantiation_actions(actions, originalProduction, NULL);
        outputManager->printa(thisAgent, "\n");
        thisAgent->explanationMemory->current_discussed_chunk->identity_analysis.print_instantiation_mappings(instantiationID);
        if (printFooter) {
            thisAgent->explanationMemory->print_footer();
        }
    }
}

void instantiation_record::viz_simple_instantiation()
{
    thisAgent->visualizationManager->viz_object_start(production_name, instantiationID, viz_simple_inst);
    thisAgent->visualizationManager->viz_object_end(viz_simple_inst);
}


void instantiation_record::viz_wm_instantiation()
{

    Output_Manager* outputManager = thisAgent->outputManager;
    GraphViz_Visualizer* visualizer = thisAgent->visualizationManager;
    condition_record* lCond;

    if (conditions->empty())
    {
        outputManager->printa(thisAgent, "No conditions on left-hand-side\n");
        assert(false);
    }
    else
    {
        condition_record* lCond;
        bool lInNegativeConditions = false;
        int lConditionCount = 0;
        action* rhs;

        thisAgent->outputManager->set_print_test_format(false, true);
        thisAgent->visualizationManager->viz_object_start(production_name, instantiationID, viz_inst_record);

        for (condition_record_list::iterator it = conditions->begin(); it != conditions->end(); it++)
        {
            lCond = (*it);

            ++lConditionCount;
            if (lConditionCount > 1)
                visualizer->viz_endl();

            if (lInNegativeConditions)
            {
                if (lCond->type != CONJUNCTIVE_NEGATION_CONDITION)
                {
                    thisAgent->visualizationManager->viz_NCC_end();
                    lInNegativeConditions = false;
                }
            } else {
                if (lCond->type == CONJUNCTIVE_NEGATION_CONDITION)
                {
                    thisAgent->visualizationManager->viz_NCC_start();
                    lInNegativeConditions = true;
                }
            }
            lCond->visualize_for_wm_trace(match_level);
        }
        if (lInNegativeConditions)
        {
            thisAgent->visualizationManager->viz_NCC_end();
        } else {
            thisAgent->visualizationManager->viz_endl();
        }
        thisAgent->visualizationManager->viz_seperator();
        action_record::viz_action_list(thisAgent, actions, thisAgent->explanationMemory->get_production(original_productionID), rhs, excised_production);
        thisAgent->visualizationManager->viz_object_end(viz_inst_record);
    }
}

void instantiation_record::viz_et_instantiation()
{
    Output_Manager* outputManager = thisAgent->outputManager;
    GraphViz_Visualizer* visualizer = thisAgent->visualizationManager;

    if (conditions->empty())
    {
        outputManager->printa(thisAgent, "No conditions on left-hand-side\n");
        assert(false);
    }
    else
    {
        condition_record* lCond;
        bool lInNegativeConditions = false;
        int lConditionCount = 0;
        action* rhs;
        condition* top, *bottom, *currentNegativeCond, *current_cond, *print_cond;
        test id_test_without_goal_test = NULL, id_test_without_goal_test2 = NULL;
        bool removed_goal_test, removed_impasse_test;
        production* originalProduction = thisAgent->explanationMemory->get_production(original_productionID);

        if (!originalProduction || !originalProduction->p_node)
        {
            if (excised_production)
            {
                top = excised_production->get_lhs();
                rhs = excised_production->get_rhs();
                assert(top);
                assert(rhs);
            } else {
                thisAgent->explanationMemory->print_explanation_trace = false;
                viz_wm_instantiation();
                thisAgent->explanationMemory->print_explanation_trace = true;
                return;
            }
        } else {
            p_node_to_conditions_and_rhs(thisAgent, originalProduction->p_node, NIL, NIL, &top, &bottom, &rhs);
        }
        current_cond = top;
        if (current_cond->type == CONJUNCTIVE_NEGATION_CONDITION)
        {
            currentNegativeCond = current_cond->data.ncc.top;
        } else {
            currentNegativeCond = NULL;
        }
        outputManager->set_print_test_format(true, false);

        thisAgent->visualizationManager->viz_object_start(production_name, instantiationID, viz_inst_record);

        for (condition_record_list::iterator it = conditions->begin(); it != conditions->end(); it++)
        {
            lCond = (*it);
            ++lConditionCount;
            if (lConditionCount > 1)
            {
                thisAgent->visualizationManager->viz_endl();
            }

            if (!lInNegativeConditions && (lCond->type == CONJUNCTIVE_NEGATION_CONDITION))
            {
                thisAgent->visualizationManager->viz_NCC_start();
                lInNegativeConditions = true;
            }

            /* Get the next condition from the explanation trace.  This is tricky because NCCs are condition lists within condition lists */
            if (currentNegativeCond)
            {
                print_cond = currentNegativeCond;
            } else {
                print_cond = current_cond;
            }

            /* Visualize it */
            if (print_cond)
            {
                lCond->visualize_for_explanation_trace(print_cond, match_level);
            } else {
                /* For deep copy */
                lCond->visualize_for_chunk();
            }
            if (currentNegativeCond)
            {
                currentNegativeCond = currentNegativeCond->next;
                if (!currentNegativeCond)
                {
                    current_cond = current_cond->next;
                    thisAgent->visualizationManager->viz_NCC_end();
                    lInNegativeConditions = false;
                }
            } else {
                if (current_cond) current_cond = current_cond->next;
            }
            if (current_cond && (current_cond->type == CONJUNCTIVE_NEGATION_CONDITION) && !currentNegativeCond)
            {
                currentNegativeCond = current_cond->data.ncc.top;
            }
        }
        if (lInNegativeConditions)
        {
            thisAgent->visualizationManager->viz_NCC_end();
        } else {
            thisAgent->visualizationManager->viz_endl();
        }
        thisAgent->visualizationManager->viz_seperator();

        action_record::viz_action_list(thisAgent, actions, originalProduction, rhs, excised_production);

        if (originalProduction && originalProduction->p_node)
        {
            deallocate_condition_list(thisAgent, top);
        }
        thisAgent->visualizationManager->viz_object_end(viz_inst_record);
    }
}

void instantiation_record::visualize()
{
    if (thisAgent->visualizationManager->settings->rule_format->get_value() == viz_name)
    {
        viz_simple_instantiation();
    } else {
        if (thisAgent->explanationMemory->print_explanation_trace)
        {
            viz_et_instantiation();
        } else {
            viz_wm_instantiation();
        }
    }
}
