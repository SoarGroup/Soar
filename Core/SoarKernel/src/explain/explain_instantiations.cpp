#include "instantiation_record.h"

#include "explain.h"

#include "action_record.h"
#include "condition_record.h"
#include "production_record.h"

#include "agent.h"
#include "condition.h"
#include "debug.h"
#include "instantiation.h"
#include "preference.h"
#include "production.h"
#include "rete.h"
#include "rhs.h"
#include "symbol.h"
#include "test.h"
#include "output_manager.h"
#include "visualize.h"
#include "working_memory.h"

instantiation_record::instantiation_record(agent* myAgent, instantiation* pInst)
{
    thisAgent           = myAgent;

    instantiationID     = pInst->i_id;
    cached_inst         = pInst;
    production_name     = pInst->prod_name;
    symbol_add_ref(thisAgent, production_name);
    original_production = pInst->prod;
    excised_production  = NULL;

    creating_chunk      = 0;

    match_level         = pInst->match_goal_level;
    terminal            = false;
    path_to_base        = NULL;
    lhs_identities      = NULL;

    conditions          = new condition_record_list;
    actions             = new action_record_list;

    if (pInst->prod)
    {
        original_production->save_for_justification_explanation = true;
    }

    action_record* new_action_record;
    for (preference* pref = pInst->preferences_generated; pref != NIL; pref = pref->inst_next)
    {
        new_action_record = thisAgent->explanationLogger->add_result(pref);
        actions->push_front(new_action_record);
    }

}

instantiation_record::~instantiation_record()
{
    dprint(DT_EXPLAIN, "Deleting instantiation record i%u (%y)\n", instantiationID, production_name);
    symbol_remove_ref(thisAgent, production_name);
    delete conditions;
    delete actions;
    dprint(DT_EXPLAIN, "Done deleting instantiation record i%u (%y)\n", instantiationID, production_name);
    if (path_to_base)
    {
        delete path_to_base;
    }
}

void instantiation_record::delete_instantiation()
{
    for (auto it = conditions->begin(); it != conditions->end(); it++)
    {
        thisAgent->explanationLogger->delete_condition((*it)->get_conditionID());
    }
    for (auto it = actions->begin(); it != actions->end(); it++)
    {
        thisAgent->explanationLogger->delete_action((*it)->get_actionID());
    }
}

void instantiation_record::record_instantiation_contents()
{
    dprint(DT_EXPLAIN_ADD_INST, "- Recording instantiation contents for i%u (%y)\n", cached_inst->i_id, production_name);
    /* Create condition and action records */
    for (condition* cond = cached_inst->top_of_instantiated_conditions; cond != NIL; cond = cond->next)
    {
        condition_record* lCondRecord = thisAgent->explanationLogger->add_condition(conditions, cond, this);
        lCondRecord->connect_to_action();
    }
}


void instantiation_record::viz_connect_conditions()
{
    condition_record* lCondRecord;
    for (auto it = conditions->begin(); it != conditions->end(); it++)
    {
        lCondRecord = (*it);
        lCondRecord->viz_connect_to_action();
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
        /* I don't think the connection can ever change without the instantiation retracting.  If we have problems
         * with a trace being connected incorrectly, though, this could be why. */
//        lCondRecord->connect_to_action();
    }
}

action_record* instantiation_record::find_rhs_action(preference* pPref)
{
    action_record_list::iterator iter;

    for (iter = actions->begin(); iter != actions->end(); ++iter)
    {
        if ((*iter)->original_pref == pPref)
        {
            dprint(DT_EXPLAIN_CONDS, "...found RHS action a%u for condition preference %p.\n", (*iter)->get_actionID(), pPref);
        }
        return (*iter);
    }
    dprint(DT_EXPLAIN_CONNECT, "...did not find pref %p among:\n", pPref);
    for (iter = actions->begin(); iter != actions->end(); ++iter)
    {
            dprint(DT_EXPLAIN_CONNECT, "      %p\n", (*iter)->original_pref);
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
//                dprint(DT_EXPLAIN_PATHS, "...%d >= %d...\n", match_level, (*it)->get_level());
//            }
//            path_to_base->pop_back();
        } else {
            dprint(DT_EXPLAIN_PATHS, "...not recursing because no parent or parent != match level\n");
            dprint(DT_EXPLAIN_PATHS, "...%u: %d != %d...\n", lParentInst, (lParentInst ? lParentInst->get_match_level() : 0), match_level);
        }
    }
}

id_set* instantiation_record::get_lhs_identities()
{
    if (lhs_identities) return lhs_identities;
    lhs_identities = new id_set();
    condition* top, *bottom;
    if (!original_production || !original_production->p_node)
    {
        if (excised_production)
        {
            top = excised_production->get_lhs();
            assert(top);
        } else {
            thisAgent->outputManager->printa_sf(thisAgent, "%fError:  Cannot generate identity analysis this instantiation.  Original rule conditions no longer in RETE.\n");
        }
    } else {
        p_node_to_conditions_and_rhs(thisAgent, original_production->p_node, NIL, NIL, &top, &bottom, NULL);
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
        condition* top, *bottom, *currentNegativeCond, *current_cond, *print_cond;
        test id_test_without_goal_test = NULL, id_test_without_goal_test2 = NULL;
        bool removed_goal_test, removed_impasse_test;


        outputManager->set_column_indent(0, 7);
        outputManager->set_column_indent(1, 57);
        outputManager->set_column_indent(2, 72);
        /* Print header */
        outputManager->printa_sf(thisAgent, "Working memory trace of instantiation # %u %-(match of rule %y)\n\n",
        		instantiationID, production_name);
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

            id_test_without_goal_test = copy_test_removing_goal_impasse_tests(thisAgent, lCond->condition_tests.id, &removed_goal_test, &removed_impasse_test);

            outputManager->printa_sf(thisAgent, "(%t%s^%t %t)%s%-",
            		id_test_without_goal_test, ((lCond->type == NEGATIVE_CONDITION) ? " -" : " "),
					lCond->condition_tests.attr, lCond->condition_tests.value, thisAgent->explanationLogger->is_condition_related(lCond) ? "*" : "");
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
        thisAgent->explanationLogger->print_action_list(actions, original_production, rhs);
        if (printFooter) {
        	thisAgent->explanationLogger->print_footer();
            outputManager->printa_sf(thisAgent, "\n- All working memory elements matched at level %d or higher.\n", match_level);
            thisAgent->explanationLogger->print_path_to_base(path_to_base, false, "- This instantiation produced one of the results of the chunk being explained.", "- Shortest path to a result instantiation: ");
        }
        outputManager->printa(thisAgent, "\n");

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
        test id_test_without_goal_test = NULL, id_test_without_goal_test2 = NULL;
        bool removed_goal_test, removed_impasse_test;

        /* If we're printing the explanation trace, we reconstruct the conditions.  We need to do this
         * because we don't want to cache the explanation trace's original symbols every time we create an instantiation.
         * We used to and it's very inefficient.  We also can't use the ebChunker's identity lookup table because that
         * is only for debugging and does not get built for releases. */
        if (!original_production || !original_production->p_node)
        {
        	if (excised_production)
        	{
        		top = excised_production->get_lhs();
        		rhs = excised_production->get_rhs();
        		assert(top);
        		assert(rhs);
        	} else {
        		outputManager->printa_sf(thisAgent, "Explanation trace of instantiation # %u %-(match of rule %y)\n",
        				instantiationID, production_name);
        		outputManager->printa_sf(thisAgent,
        				"\nWarning:  Cannot print explanation trace for this instantiation because no underlying\n"
        				"            rule found in RETE.  Printing working memory trace instead.\n\n");
        		thisAgent->explanationLogger->print_explanation_trace = false;
        		print_for_wme_trace(printFooter);
        		thisAgent->explanationLogger->print_explanation_trace = true;
        		return;
        	}
        } else {
        	p_node_to_conditions_and_rhs(thisAgent, original_production->p_node, NIL, NIL, &top, &bottom, &rhs);
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
        outputManager->printa_sf(thisAgent, "Explanation trace of instantiation # %u %-(match of rule %y)\n\n",
        		instantiationID, production_name);
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
            id_test_without_goal_test = copy_test_removing_goal_impasse_tests(thisAgent, print_cond->data.tests.id_test, &removed_goal_test, &removed_impasse_test);
            id_test_without_goal_test2 = copy_test_removing_goal_impasse_tests(thisAgent, lCond->condition_tests.id, &removed_goal_test, &removed_impasse_test);
            outputManager->printa_sf(thisAgent, "(%o%s^%o %o)%s%-",
            		id_test_without_goal_test, ((lCond->type == NEGATIVE_CONDITION) ? " -" : " "),
					print_cond->data.tests.attr_test, print_cond->data.tests.value_test,
					thisAgent->explanationLogger->is_condition_related(lCond) ? "*" : "");
            outputManager->printa_sf(thisAgent, "(%g%s^%g %g)%-",
            		id_test_without_goal_test2, ((lCond->type == NEGATIVE_CONDITION) ? " -" : " "),
					lCond->condition_tests.attr, lCond->condition_tests.value);
            deallocate_test(thisAgent, id_test_without_goal_test);
            deallocate_test(thisAgent, id_test_without_goal_test2);

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
        thisAgent->explanationLogger->print_action_list(actions, original_production, rhs);
        if (printFooter) {
        	thisAgent->explanationLogger->print_footer();
            outputManager->printa_sf(thisAgent, "\n- All working memory elements matched at level %d or higher.\n", match_level);
            thisAgent->explanationLogger->print_path_to_base(path_to_base, false, "- This instantiation produced one of the results of the chunk being explained.", "- Shortest path to a result instantiation: ");
        }
        outputManager->printa(thisAgent, "\n");

        if (original_production && original_production->p_node)
        {
            deallocate_condition_list(thisAgent, top);
        }
    }
}

void instantiation_record::viz_simple_instantiation()
{
    thisAgent->visualizer->viz_rule_start(production_name, instantiationID, viz_simple_inst);
    thisAgent->visualizer->viz_rule_end();
}


void instantiation_record::viz_wm_instantiation()
{

    Output_Manager* outputManager = thisAgent->outputManager;
    GraphViz_Visualizer* visualizer = thisAgent->visualizer;
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
        test id_test_without_goal_test = NULL, id_test_without_goal_test2 = NULL;
        bool removed_goal_test, removed_impasse_test;

        thisAgent->outputManager->set_print_test_format(false, true);
        thisAgent->visualizer->viz_rule_start(production_name, instantiationID, viz_inst_record);

        for (condition_record_list::iterator it = conditions->begin(); it != conditions->end(); it++)
        {
            lCond = (*it);

            ++lConditionCount;
            if (lConditionCount > 1)
                visualizer->viz_record_line_end();

            if (lInNegativeConditions)
            {
                if (lCond->type != CONJUNCTIVE_NEGATION_CONDITION)
                {
                    thisAgent->visualizer->viz_NCC_end();
                    lInNegativeConditions = false;
                }
            } else {
                if (lCond->type == CONJUNCTIVE_NEGATION_CONDITION)
                {
                    thisAgent->visualizer->viz_NCC_start();
                    lInNegativeConditions = true;
                }
            }

            thisAgent->visualizer->viz_wt_condition(lCond);
        }
        if (lInNegativeConditions)
        {
            thisAgent->visualizer->viz_NCC_end();
        } else {
            thisAgent->visualizer->viz_record_line_end();
        }
        thisAgent->visualizer->viz_seperator();
        thisAgent->explanationLogger->viz_action_list(actions, original_production, rhs);
        thisAgent->visualizer->viz_rule_end();
    }
}

void instantiation_record::viz_et_instantiation()
{
    Output_Manager* outputManager = thisAgent->outputManager;
    GraphViz_Visualizer* visualizer = thisAgent->visualizer;

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

        if (!original_production || !original_production->p_node)
        {
            if (excised_production)
            {
                top = excised_production->get_lhs();
                rhs = excised_production->get_rhs();
                assert(top);
                assert(rhs);
            } else {
                thisAgent->explanationLogger->print_explanation_trace = false;
                viz_wm_instantiation();
                thisAgent->explanationLogger->print_explanation_trace = true;
                return;
            }
        } else {
            p_node_to_conditions_and_rhs(thisAgent, original_production->p_node, NIL, NIL, &top, &bottom, &rhs);
        }
        current_cond = top;
        if (current_cond->type == CONJUNCTIVE_NEGATION_CONDITION)
        {
            currentNegativeCond = current_cond->data.ncc.top;
        } else {
            currentNegativeCond = NULL;
        }
        outputManager->set_print_test_format(true, false);

        thisAgent->visualizer->viz_rule_start(production_name, instantiationID, viz_inst_record);

        for (condition_record_list::iterator it = conditions->begin(); it != conditions->end(); it++)
        {
            lCond = (*it);
            ++lConditionCount;
            if (lConditionCount > 1)
            {
                thisAgent->visualizer->viz_record_line_end();
            }

            if (!lInNegativeConditions && (lCond->type == CONJUNCTIVE_NEGATION_CONDITION))
            {
                thisAgent->visualizer->viz_NCC_start();
                lInNegativeConditions = true;
            }

            /* Get the next condition from the explanation trace.  This is tricky because NCCs are condition lists within condition lists */
            if (currentNegativeCond)
            {
                print_cond = currentNegativeCond;
            } else {
                print_cond = current_cond;
            }
            thisAgent->visualizer->viz_et_condition(lCond, print_cond);
            if (currentNegativeCond)
            {
                currentNegativeCond = currentNegativeCond->next;
                if (!currentNegativeCond)
                {
                    current_cond = current_cond->next;
                    thisAgent->visualizer->viz_NCC_end();
                    lInNegativeConditions = false;
                }
            } else {
                current_cond = current_cond->next;
            }
            if (current_cond && (current_cond->type == CONJUNCTIVE_NEGATION_CONDITION) && !currentNegativeCond)
            {
                currentNegativeCond = current_cond->data.ncc.top;
            }
        }
        if (lInNegativeConditions)
        {
            thisAgent->visualizer->viz_NCC_end();
        } else {
            thisAgent->visualizer->viz_record_line_end();
        }
        thisAgent->visualizer->viz_seperator();
        thisAgent->explanationLogger->viz_action_list(actions, original_production, rhs);
        if (original_production && original_production->p_node)
        {
            deallocate_condition_list(thisAgent, top);
        }
        thisAgent->visualizer->viz_rule_end();
    }
}

