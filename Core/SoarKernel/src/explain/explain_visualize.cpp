#include "debug.h"
#include "ebc.h"
#include "explain.h"
#include "agent.h"
#include "condition.h"
#include "instantiation.h"
#include "preference.h"
#include "print.h"
#include "production.h"
#include "rete.h"
#include "rhs.h"
#include "test.h"
#include "working_memory.h"
#include "output_manager.h"

void Explanation_Logger::visualize_last_output()
{
    if (!last_printed_id)
    {
        visualize_chunk_explanation();
    } else {
        visualize_instantiation_explanation_for_id(last_printed_id);
    }

}


bool Explanation_Logger::visualize_instantiation_explanation_for_id(uint64_t pInstID)
{
    std::unordered_map< uint64_t, instantiation_record* >::iterator iter_inst;

    iter_inst = instantiations->find(pInstID);
    if (iter_inst == instantiations->end())
    {
        outputManager->printa_sf(thisAgent, "Could not find an instantiation with ID %u.\n", pInstID);
        return false;
    }
    last_printed_id = pInstID;
    visualize_instantiation_explanation(iter_inst->second);
    return true;
}

void Explanation_Logger::visualize_chunk_explanation()
{

}

void Explanation_Logger::viz_graph_start()
{
	outputManager->printa(thisAgent, "digraph g {\n"
			"   graph [ rankdir = \"LR\" splines = \"spline\"];\n"
			"   node [fontsize = \"16\" shape = \"record\"];\n"
			"   edge [];\n");
}
void Explanation_Logger::viz_graph_end()
{
	outputManager->printa(thisAgent, "}");
}

void Explanation_Logger::viz_rule_start(Symbol* pName, uint64_t node_id)
{
	outputManager->printa_sf(thisAgent, "   rule%u [\n      label = \"<rule-name> %y |\n", node_id, pName);
}

void Explanation_Logger::viz_rule_end()
{
	outputManager->printa(thisAgent, "\"\n];\n");

}

void Explanation_Logger::viz_NCC_start()
{
	outputManager->printa(thisAgent, "              -{ |\n");

}

void Explanation_Logger::viz_NCC_end()
{
	outputManager->printa(thisAgent, "              } |\n");
}

void Explanation_Logger::viz_seperator()
{
	outputManager->printa(thisAgent, "              ----\\> |\n");
}

void Explanation_Logger::viz_text_record(const char* pMsg)
{
	outputManager->printa_sf(thisAgent, "%s |", pMsg);
}

void Explanation_Logger::viz_condition_record(condition_record* pCond)
{
    bool removed_goal_test, removed_impasse_test;

    test id_test_without_goal_test = copy_test_removing_goal_impasse_tests(thisAgent, pCond->condition_tests.id, &removed_goal_test, &removed_impasse_test);
//    { <f1a> S3 | <f1b> superstate | <f1> S1 } |
	outputManager->printa_sf(thisAgent, "              { <c%ua> %t |%s<c%ub> %t | <c%uc> %t } ",
			pCond->conditionID, id_test_without_goal_test, ((pCond->type == NEGATIVE_CONDITION) ? " -" : " "),
			pCond->conditionID, pCond->condition_tests.attr, pCond->conditionID, pCond->condition_tests.value);
    deallocate_test(thisAgent, id_test_without_goal_test);
}

void Explanation_Logger::viz_condition(condition_record* pCondRecord, condition* pCond)
{
    bool removed_goal_test, removed_impasse_test;

    test id_test_without_goal_test = copy_test_removing_goal_impasse_tests(thisAgent, pCond->data.tests.id_test, &removed_goal_test, &removed_impasse_test);
    test id_test_without_goal_test2 = copy_test_removing_goal_impasse_tests(thisAgent, pCondRecord->condition_tests.id, &removed_goal_test, &removed_impasse_test);
//    { <f1a> S3 | <f1b> superstate | <f1> S1 } |
	outputManager->printa_sf(thisAgent, "              { <c%ua> %o %g |%s<c%ub> %o %g| <c%uc> %o %g}",
			pCondRecord->conditionID, id_test_without_goal_test, id_test_without_goal_test2,
			((pCondRecord->type == NEGATIVE_CONDITION) ? " -" : " "),
			pCondRecord->conditionID, pCond->data.tests.attr_test, pCondRecord->condition_tests.attr,
			pCondRecord->conditionID, pCond->data.tests.value_test, pCondRecord->condition_tests.value);

    deallocate_test(thisAgent, id_test_without_goal_test);
    deallocate_test(thisAgent, id_test_without_goal_test2);
}

void Explanation_Logger::viz_preference(preference* pPref, uint64_t pNodeID)
{
	if (!print_explanation_trace)
	{
		outputManager->printa_sf(thisAgent, "              { <a%ua> %y | <a%ub> %y ", pNodeID, pPref->id, pNodeID, pPref->attr);
		if (preference_is_binary(pPref->type))
        {
        	outputManager->printa_sf(thisAgent, "| <a%uc1> %y | <a%uc> %c %y } ", pNodeID, pPref->value, pNodeID, preference_to_char(pPref->type), pPref->referent);
        } else {
        	outputManager->printa_sf(thisAgent, "| <a%uc> %y %c } ", pNodeID, pPref->value, preference_to_char(pPref->type));
        }
	} else {
		outputManager->printa_sf(thisAgent, "              { <a%ua> ", pNodeID);
		if (pPref->o_ids.id)
		{
			outputManager->printa_sf(thisAgent, "%u", pPref->o_ids.id);
		} else {
			outputManager->printa_sf(thisAgent, "%y", pPref->id);
		}
		outputManager->printa_sf(thisAgent, " | <a%ub> ", pNodeID);
		if (pPref->o_ids.attr)
		{
			outputManager->printa_sf(thisAgent, "%u", pPref->o_ids.attr);
		} else {
			outputManager->printa_sf(thisAgent, "%y", pPref->attr);
		}
		if (preference_is_binary(pPref->type))
        {
			if (pPref->o_ids.value)
			{
				outputManager->printa_sf(thisAgent, " | <a%uc1> %u", pNodeID, pPref->o_ids.value);
			} else {
				outputManager->printa_sf(thisAgent, " | <a%uc1> %y", pNodeID, pPref->value);
			}
			if (pPref->o_ids.referent)
			{
				outputManager->printa_sf(thisAgent, " | <a%uc> %u } ", pNodeID, preference_to_char(pPref->type), pPref->referent);
			} else {
				outputManager->printa_sf(thisAgent, " | <a%uc> %c %y } ", pNodeID, preference_to_char(pPref->type), pPref->referent);
			}
        } else {
			if (pPref->o_ids.value)
			{
				outputManager->printa_sf(thisAgent, " | <a%uc> %u } ", pNodeID, pPref->o_ids.value);
			} else {
				outputManager->printa_sf(thisAgent, " | <a%uc> %y } ", pNodeID, pPref->value);
			}
        }
	}
}

void Explanation_Logger::viz_action(action* pAction, uint64_t pNodeID)
{
	std::string destString;
	std::stringstream vizActionString;

    if (pAction->type == FUNCALL_ACTION)
    {
    	vizActionString << "              { <a" << pNodeID << "a> RHS Funcall | " << "<c" << pNodeID << "c>";
        destString = "";
        outputManager->rhs_value_to_string(thisAgent, pAction->value, destString, NULL, NULL);
        vizActionString << destString << " }";
    } else {
        destString = "";
        outputManager->rhs_value_to_string(thisAgent, pAction->id, destString, NULL, NULL);
    	vizActionString << "              { <a" << pNodeID << "a> " << destString << "| " ;
        destString = "";
        outputManager->rhs_value_to_string(thisAgent, pAction->attr, destString, NULL, NULL);
    	vizActionString << "<a" << pNodeID << "b> " << destString << "| " ;
        destString = "";
        outputManager->rhs_value_to_string(thisAgent, pAction->value, destString, NULL, NULL);
        if (pAction->referent)
        {
        	vizActionString << "<a" << pNodeID << "c1> " << destString << "| ";
            destString = "";
            outputManager->rhs_value_to_string(thisAgent, pAction->referent, destString, NULL, NULL);
        	vizActionString << "<a" << pNodeID << "c> " << preference_to_char(pAction->preference_type) << " " << destString ;
        } else {
        	vizActionString << "<a" << pNodeID << "c> " << destString << preference_to_char(pAction->preference_type) << " }";
        }
    }
    outputManager->printa(thisAgent, vizActionString.str().c_str());
    destString.clear();
    vizActionString.clear();
}

void Explanation_Logger::viz_action_record(action_record* pActionRecord, action* pAction)
{
//    { <f1a> S3 | <f1b> superstate | <f1> S1 } |

//    if (!print_explanation_trace)
//    {
//        outputManager->printa_sf(thisAgent, "%d:%-%p\n", lActionCount, lAction->instantiated_pref);
//    	outputManager->printa_sf(thisAgent, "              { <c%ua> %t |%s<c%ub> %t | <c%uc> %t } ",
//    			pActionRecord->actionID, pCond->condition_tests.id,
//    			pActionRecord->actionID, pCond->condition_tests.attr,
//    			pActionRecord->actionID, pCond->condition_tests.value);
//    } else {
//        thisAgent->outputManager->set_print_test_format(true, false);
//        outputManager->printa_sf(thisAgent, "%d:%-%a", lActionCount,  rhs);
//        thisAgent->outputManager->set_print_test_format(false, true);
//        rhs_value rt = rhs->value;
//
//        if (lAction->variablized_action)
//        {
//            outputManager->printa_sf(thisAgent, "%-%a\n", lAction->variablized_action);
//        } else {
//            outputManager->printa_sf(thisAgent, "%-%p\n", lAction->instantiated_pref);
//        }
//    }
}


void Explanation_Logger::visualize_action_list(action_record_list* pActionRecords, production* pOriginalRule, action* pRhs)
{
    if (pActionRecords->empty())
    {
    	viz_text_record("Empty RHS");
    }
    else
    {
        action_record* lAction;
        condition* top = NULL, *bottom = NULL;
        action* rhs;
        int lActionCount = 0;
        thisAgent->outputManager->set_print_indents("");
        thisAgent->outputManager->set_print_test_format(true, false);
        if (print_explanation_trace)
        {
            /* We use pRhs to deallocate actions at end, and rhs to iterate through actions */
            if (pRhs)
            {
                rhs = pRhs;
            } else {
                if (!pOriginalRule || !pOriginalRule->p_node)
                {
                	viz_text_record("No RETE rule");
                	return;
                } else {
                    p_node_to_conditions_and_rhs(thisAgent, pOriginalRule->p_node, NIL, NIL, &top, &bottom, &rhs);
                    pRhs = rhs;
                }
            }
        }
        for (action_record_list::iterator it = pActionRecords->begin(); it != pActionRecords->end(); it++)
        {
            lAction = (*it);
            ++lActionCount;
            if (!print_explanation_trace)
            {
            	viz_preference(lAction->instantiated_pref, lAction->actionID);
//                outputManager->printa_sf(thisAgent, "%d:%-%p\n", lActionCount, lAction->instantiated_pref);
            } else {
            	viz_action(rhs, lAction->actionID);
//            	outputManager->printa_sf(thisAgent, " (from action rhs)\n");
                outputManager->printa(thisAgent, "\n");
//                thisAgent->outputManager->set_print_test_format(true, false);
//                outputManager->printa_sf(thisAgent, "%d:%-%a", lActionCount,  rhs);
//                thisAgent->outputManager->set_print_test_format(false, true);
//                rhs_value rt = rhs->value;

                if (lAction->variablized_action)
                {
                	viz_action(lAction->variablized_action, lAction->actionID);
//                	outputManager->printa_sf(thisAgent, " (from variablized action)\n");
                    outputManager->printa(thisAgent, "\n");
//                    outputManager->printa_sf(thisAgent, "%-%a\n", lAction->variablized_action);
                } else {
                	viz_preference(lAction->instantiated_pref, lAction->actionID);
//                	outputManager->printa_sf(thisAgent, " (from preference)\n");
                    outputManager->printa(thisAgent, "\n");
//                    outputManager->printa_sf(thisAgent, "%-%p\n", lAction->instantiated_pref);
                }
//                if (print_explanation_trace)
//                {
//                    thisAgent->outputManager->set_print_test_format(false, true);
//                    outputManager->printa_sf(thisAgent, "%-%p\n", lAction->instantiated_pref);
//                    outputManager->printa_sf(thisAgent, "%d:%-%a", lActionCount,  rhs);
//                } else {
//                    outputManager->printa(thisAgent, "\n");
//                }
//                outputManager->printa(thisAgent, "\n");
                rhs = rhs->next;
            }
        }
        if (print_explanation_trace)
        {
            /* If top exists, we generated conditions here and must deallocate. */
            if (pRhs) deallocate_action_list(thisAgent, pRhs);
            if (top) deallocate_condition_list(thisAgent, top);
        }
        thisAgent->outputManager->clear_print_test_format();
    }
}
void Explanation_Logger::visualize_instantiation_explanation(instantiation_record* pInstRecord)
{
    if (pInstRecord->conditions->empty())
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

        viz_graph_start();

        if (print_explanation_trace)
        {
            if (!pInstRecord->original_production || !pInstRecord->original_production->p_node)
            {
                if (pInstRecord->excised_production)
                {
                    top = pInstRecord->excised_production->get_lhs();
                    rhs = pInstRecord->excised_production->get_rhs();
                    assert(top);
                    assert(rhs);
                } else {
                    print_explanation_trace = false;
                    visualize_instantiation_explanation(pInstRecord);
                    print_explanation_trace = true;
                    return;
                }
            } else {
                p_node_to_conditions_and_rhs(thisAgent, pInstRecord->original_production->p_node, NIL, NIL, &top, &bottom, &rhs);
            }
            current_cond = top;
            if (current_cond->type == CONJUNCTIVE_NEGATION_CONDITION)
            {
                currentNegativeCond = current_cond->data.ncc.top;
            } else {
                currentNegativeCond = NULL;
            }
            thisAgent->outputManager->set_print_test_format(true, false);
        } else {
            thisAgent->outputManager->set_print_test_format(false, true);
        }
        viz_rule_start(pInstRecord->production_name, pInstRecord->instantiationID);

        for (condition_record_list::iterator it = pInstRecord->conditions->begin(); it != pInstRecord->conditions->end(); it++)
        {
            lCond = (*it);
            ++lConditionCount;
            if (lConditionCount > 1)
            {
                outputManager->printa(thisAgent, "| \n");
            }
            if (lInNegativeConditions)
            {
                if (lCond->type != CONJUNCTIVE_NEGATION_CONDITION)
                {
                	viz_NCC_end();
                    lInNegativeConditions = false;
                }
            } else {
                if (lCond->type == CONJUNCTIVE_NEGATION_CONDITION)
                {
                	viz_NCC_start();
                    lInNegativeConditions = true;
                }
            }

            if (!print_explanation_trace)
            {
                viz_condition_record(lCond);
            } else {
                /* Get the next condition from the explanation trace.  This is tricky because NCCs are condition lists within condition lists */
                if (currentNegativeCond)
                {
                    print_cond = currentNegativeCond;
                } else {
                    print_cond = current_cond;
                }
                viz_condition(lCond, print_cond);
                if (currentNegativeCond)
                {
                    currentNegativeCond = currentNegativeCond->next;
                } else {
                    current_cond = current_cond->next;
                }
                if (current_cond && (current_cond->type == CONJUNCTIVE_NEGATION_CONDITION) && !currentNegativeCond)
                {
                    currentNegativeCond = current_cond->data.ncc.top;
                }
            }
        }
        if (lInNegativeConditions)
        {
        	viz_NCC_end();
        } else {
            outputManager->printa(thisAgent, "\n");
        }
        viz_seperator();
        visualize_action_list(pInstRecord->actions, pInstRecord->original_production, rhs);
        if (print_explanation_trace && pInstRecord->original_production && pInstRecord->original_production->p_node)
        {
            deallocate_condition_list(thisAgent, top);
        }
        viz_rule_end();
        viz_graph_end();
    }
}

