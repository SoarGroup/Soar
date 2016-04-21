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
#include <regex>

void Explanation_Logger::visualize_last_output()
{
	graphviz_output.clear();
    if (!last_printed_id)
    {
        visualize_chunk_explanation();
    } else {
        visualize_instantiation_explanation_for_id(last_printed_id);
    }
    /* Note that we temporarily use ≤ and ≥ for graphviz < > that don't need to be escaped */
    /* MToDo | Should be combined into one regex command.  */
    graphviz_output = std::regex_replace(graphviz_output, std::regex("<"), "\\<");
    graphviz_output = std::regex_replace(graphviz_output, std::regex(">"), "\\>");
    graphviz_output = std::regex_replace(graphviz_output, std::regex("@#"), "<");
    graphviz_output = std::regex_replace(graphviz_output, std::regex("#@"), ">");
    outputManager->printa(thisAgent, graphviz_output.c_str());
	graphviz_output.clear();
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
	graphviz_output +=  "digraph g {\n"
			"   graph [ rankdir = \"LR\" splines = \"spline\"];\n"
			"   node [fontsize = \"16\" shape = \"record\"];\n"
			"   edge [];\n";
}
void Explanation_Logger::viz_graph_end()
{
	graphviz_output += "}\n";
}

void Explanation_Logger::viz_rule_start(Symbol* pName, uint64_t node_id)
{
	outputManager->sprinta_sf(thisAgent, graphviz_output, "   rule%u [\n      label = \"@#rule-name#@ %y |\n", node_id, pName);
}

void Explanation_Logger::viz_rule_end()
{
	graphviz_output += "    \"];\n";

}

void Explanation_Logger::viz_NCC_start()
{
	graphviz_output +=  "              -{ |\n";

}

void Explanation_Logger::viz_NCC_end()
{
	graphviz_output += "              } |\n";
}

void Explanation_Logger::viz_seperator()
{
	graphviz_output += "              ---->";
}

void Explanation_Logger::viz_record_seperator(bool pLeftJustify)
{
	if (pLeftJustify)
		graphviz_output +=  " \\l| ";
	else
		graphviz_output +=  " | ";
}

void Explanation_Logger::viz_record_start()
{
	graphviz_output += "              { ";
}

void Explanation_Logger::viz_record_end(bool pLeftJustify)
{
	if (pLeftJustify)
		graphviz_output +=  " \\l} ";
	else
		graphviz_output +=  " } ";
}

void Explanation_Logger::viz_text_record(const char* pMsg)
{
	graphviz_output += pMsg;
	graphviz_output += " |";

}

void Explanation_Logger::viz_add_port(char pTypeChar, uint64_t pNodeID, bool pIsLeftPort)
{
	graphviz_output += "@#";
	graphviz_output += pTypeChar;
	graphviz_output += '_';
	graphviz_output += std::to_string(pNodeID);
	if (pIsLeftPort)
	graphviz_output += "_l#@ ";
	else
		graphviz_output += "_r#@ ";
}
void Explanation_Logger::viz_et_test(test pTest, test pTestIdentity, uint64_t pNode_id, bool printInitialPort, bool printFinalPort)
{
	cons* c, *c2;

    if (pTest->type == CONJUNCTIVE_TEST)
    {
    	graphviz_output += "{ ";
    	if (printInitialPort)
    	{
    		viz_add_port('c', pNode_id, true);
    	}
    	for (c = pTest->data.conjunct_list, c2 = pTestIdentity->data.conjunct_list; c != NIL; c = c->rest, c2 = c2->rest)
        {
        	assert(c2);
        	if (printFinalPort && !c->rest)
        	{
        		viz_add_port('c', pNode_id, false);
        	}
        	viz_et_test(static_cast<test>(c->first), static_cast<test>(c2->first), pNode_id, false, false);
        	if (c->rest)
        	{
        		viz_record_seperator();
        	} else {
        		graphviz_output +=  " \\l} ";
        	}
        }
    } else {
    	if (printInitialPort)
    	{
    		viz_add_port('c', pNode_id, true);
    	} else if (printFinalPort)
    	{
    		viz_add_port('c', pNode_id, false);
    	}
    	/* In original explain function this was %o, but I think that doesn't make sense because it requires the debug
    	 * database for original variable look-up.
    	 * MToDo | Change so that we use %o if debug database is available.  Original var names much more readable
    	 *         for examples and debugging.*/
    	if (pTestIdentity->identity)
    	{
    		outputManager->sprinta_sf(thisAgent, graphviz_output, "%t (%u) ", pTest, pTestIdentity->identity);
    	} else {
    		outputManager->sprinta_sf(thisAgent, graphviz_output, "%t ", pTest);
    	}
    }
}

/* This is only called for instantiated conditions in the wme trace.
 *
 * Note:  This may cause a bad vizgraph if attribute of NC is a conjunct.  The minus
 *        sign would be outside the brackets of the nested records for the conjunct.
 *        */
void Explanation_Logger::viz_wt_condition(condition_record* pCond)
{
    bool removed_goal_test, removed_impasse_test;

    test id_test_without_goal_test = copy_test_removing_goal_impasse_tests(thisAgent, pCond->condition_tests.id, &removed_goal_test, &removed_impasse_test);
    viz_record_start();
    viz_add_port('c', pCond->conditionID, true);
	outputManager->sprinta_sf(thisAgent, graphviz_output, " %t", id_test_without_goal_test);
	viz_record_seperator();
	if (pCond->type == NEGATIVE_CONDITION) graphviz_output += '-';
	outputManager->sprinta_sf(thisAgent, graphviz_output, " %t", pCond->condition_tests.attr);
	viz_record_seperator();
    viz_add_port('c', pCond->conditionID, false);
	outputManager->sprinta_sf(thisAgent, graphviz_output, " %t", pCond->condition_tests.value);
	viz_record_end();
    deallocate_test(thisAgent, id_test_without_goal_test);
}

/* This is only called for conditions in the explanation trace */
void Explanation_Logger::viz_et_condition(condition_record* pCondRecord, condition* pCond)
{
	bool removed_goal_test, removed_impasse_test;

	test id_test_without_goal_test = copy_test_removing_goal_impasse_tests(thisAgent, pCond->data.tests.id_test, &removed_goal_test, &removed_impasse_test);
	test id_test_without_goal_test2 = copy_test_removing_goal_impasse_tests(thisAgent, pCondRecord->condition_tests.id, &removed_goal_test, &removed_impasse_test);

	graphviz_output += "              { ";
	viz_et_test(id_test_without_goal_test, id_test_without_goal_test2, pCondRecord->conditionID, true, false);
	if (id_test_without_goal_test2->type != CONJUNCTIVE_TEST)
	{
		viz_record_seperator();
	} else {
		graphviz_output += "| ";
	}
	if (pCondRecord->type == NEGATIVE_CONDITION)
	{
		graphviz_output += "- ^";
	} else {
		graphviz_output += "^";
	}
	viz_et_test(pCond->data.tests.attr_test, pCondRecord->condition_tests.attr, pCondRecord->conditionID, false, false);
	if (pCond->data.tests.attr_test->type != CONJUNCTIVE_TEST)
	{
		viz_record_seperator();
	} else {
		graphviz_output += "| ";
	}
	viz_et_test(pCond->data.tests.value_test, pCondRecord->condition_tests.value, pCondRecord->conditionID, false, true);
	if (pCond->data.tests.value_test->type != CONJUNCTIVE_TEST)
	{
		viz_record_end();
	} else {
		graphviz_output += "}";
	}

	deallocate_test(thisAgent, id_test_without_goal_test);
	deallocate_test(thisAgent, id_test_without_goal_test2);
}

void Explanation_Logger::viz_wt_preference(preference* pPref, uint64_t pNodeID)
{
	/* First case may no longer be needed since only called for wm trace */
	if (print_explanation_trace)
	 {
		viz_record_start();
		viz_add_port('a', pNodeID, true);

		if (pPref->o_ids.id)
		{
			graphviz_output += std::to_string(pPref->o_ids.id);
		} else {
			outputManager->sprinta_sf(thisAgent, graphviz_output, "%y", pPref->id);
		}
		viz_record_seperator();
		if (pPref->o_ids.attr)
		{
			graphviz_output += std::to_string(pPref->o_ids.attr);
		} else {
			outputManager->sprinta_sf(thisAgent, graphviz_output, "%y", pPref->attr);
		}
		if (preference_is_binary(pPref->type))
        {
			if (pPref->o_ids.value)
			{
				viz_record_seperator();
				graphviz_output += std::to_string(pPref->o_ids.value);
			} else {
				viz_record_seperator();
				outputManager->sprinta_sf(thisAgent, graphviz_output, "%y", pPref->value);
			}
			if (pPref->o_ids.referent)
			{
				viz_record_seperator();
				viz_add_port('a', pNodeID, false);
				graphviz_output += preference_to_char(pPref->type);
				graphviz_output += std::to_string(pPref->o_ids.value);
				viz_record_end();
			} else {
				viz_record_seperator();
				viz_add_port('a', pNodeID, false);
				graphviz_output += preference_to_char(pPref->type);
				outputManager->sprinta_sf(thisAgent, graphviz_output, "%y", pPref->referent);
				viz_record_end();
			}
        } else {
			if (pPref->o_ids.value)
			{
				viz_record_seperator();
				viz_add_port('a', pNodeID, false);
				graphviz_output += preference_to_char(pPref->type);
				graphviz_output += std::to_string(pPref->o_ids.value);
				viz_record_end();
			} else {
				viz_record_seperator();
				viz_add_port('a', pNodeID, false);
				graphviz_output += preference_to_char(pPref->type);
				outputManager->sprinta_sf(thisAgent, graphviz_output, "%y", pPref->value);
				viz_record_end();
			}
        }
	} else {
		viz_record_start();
		viz_add_port('a', pNodeID, true);
		outputManager->sprinta_sf(thisAgent, graphviz_output, "%y", pPref->id);
		viz_record_seperator();
		outputManager->sprinta_sf(thisAgent, graphviz_output, "%y", pPref->attr);

		if (preference_is_binary(pPref->type))
        {
			viz_record_seperator();
        	outputManager->sprinta_sf(thisAgent, graphviz_output, "%y", pPref->value);
    		viz_record_seperator();
			viz_add_port('a', pNodeID, false);
        	outputManager->sprinta_sf(thisAgent, graphviz_output, " %c %y", preference_to_char(pPref->type), pPref->referent);
    		viz_record_end();
        } else {
    		viz_record_seperator();
			viz_add_port('a', pNodeID, false);
        	outputManager->sprinta_sf(thisAgent, graphviz_output, " %y %c", pPref->value, preference_to_char(pPref->type));
    		viz_record_end();
        }
	}
}

void Explanation_Logger::viz_rhs_value(const rhs_value pRHS_value, const rhs_value pRHS_variablized_value, uint64_t pID)
{
	std::string tempString;
	tempString = "";
	outputManager->set_print_test_format(true, false);
	outputManager->rhs_value_to_string(thisAgent, pRHS_value, tempString, NULL, NULL);
	graphviz_output += tempString;
	if (pRHS_variablized_value)
	{
		tempString = "";
		outputManager->set_print_test_format(false, true);
		outputManager->rhs_value_to_string(thisAgent, pRHS_variablized_value, tempString, NULL, NULL);
		if (!tempString.empty()) graphviz_output += tempString;
	} else if (pID) {
		graphviz_output += " (";
		graphviz_output += std::to_string(pID);
		graphviz_output += ')';
	}
}

void Explanation_Logger::viz_et_action(action* pAction, action* pVariablizedAction, preference* pPref, uint64_t pNodeID)
{
	std::string tempString;

	if (pAction->type == FUNCALL_ACTION)
	{
		viz_record_start();
		viz_add_port('a', pNodeID, true);
		graphviz_output += "RHS Funcall";
		viz_record_seperator();
		viz_add_port('a', pNodeID, false);
		tempString = "";
		outputManager->rhs_value_to_string(thisAgent, pAction->value, tempString, NULL, NULL);
		graphviz_output += tempString;
		viz_record_end();
	} else {
		viz_record_start();
		viz_add_port('a', pNodeID, true);
		viz_rhs_value(pAction->id, (pVariablizedAction ? pVariablizedAction->id : NULL), pPref->o_ids.id);
		viz_record_seperator();
		viz_rhs_value(pAction->attr, (pVariablizedAction ? pVariablizedAction->attr : NULL), pPref->o_ids.attr);
		viz_record_seperator();
		if (pAction->referent)
		{
			viz_rhs_value(pAction->value, (pVariablizedAction ? pVariablizedAction->value : NULL), pPref->o_ids.value);
			viz_record_seperator();
			viz_add_port('a', pNodeID, false);
			graphviz_output += preference_to_char(pAction->preference_type);
			viz_rhs_value(pAction->referent, (pVariablizedAction ? pVariablizedAction->referent : NULL), pPref->o_ids.referent);
		} else {
			viz_add_port('a', pNodeID, false);
			viz_rhs_value(pAction->value, (pVariablizedAction ? pVariablizedAction->value : NULL), pPref->o_ids.value);
			graphviz_output += ' ';
			graphviz_output += preference_to_char(pAction->preference_type);
		}
		viz_record_end();
	}
	tempString.clear();
}

void Explanation_Logger::viz_action_list(action_record_list* pActionRecords, production* pOriginalRule, action* pRhs)
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
        size_t lNumRecords = pActionRecords->size();
        for (action_record_list::iterator it = pActionRecords->begin(); it != pActionRecords->end(); it++)
        {
            lAction = (*it);
            ++lActionCount;
            if (lActionCount <= lNumRecords)
            {
                graphviz_output += " | \n";
            }
            if (!print_explanation_trace)
            {
            	viz_wt_preference(lAction->instantiated_pref, lAction->actionID);
            } else {
            	viz_et_action(rhs, lAction->variablized_action, lAction->instantiated_pref, lAction->actionID);
                rhs = rhs->next;
            }
        }
        graphviz_output += "\n";
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
                outputManager->sprinta_sf(thisAgent, graphviz_output, " | \n");
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
                viz_wt_condition(lCond);
            } else {
                /* Get the next condition from the explanation trace.  This is tricky because NCCs are condition lists within condition lists */
                if (currentNegativeCond)
                {
                    print_cond = currentNegativeCond;
                } else {
                    print_cond = current_cond;
                }
                viz_et_condition(lCond, print_cond);
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
            outputManager->sprinta_sf(thisAgent, graphviz_output, " |\n");
        }
        viz_seperator();
        viz_action_list(pInstRecord->actions, pInstRecord->original_production, rhs);
        if (print_explanation_trace && pInstRecord->original_production && pInstRecord->original_production->p_node)
        {
            deallocate_condition_list(thisAgent, top);
        }
        viz_rule_end();
        viz_graph_end();
    }
}

