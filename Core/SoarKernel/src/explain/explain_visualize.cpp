#include "explain.h"

#include "action_record.h"
#include "condition_record.h"
#include "identity_record.h"
#include "instantiation_record.h"
#include "production_record.h"

#include "agent.h"
#include "condition.h"
#include "debug.h"
#include "ebc.h"
#include "instantiation.h"
#include "output_manager.h"
#include "preference.h"
#include "print.h"
#include "production.h"
#include "rete.h"
#include "rhs.h"
#include "test.h"
#include "working_memory.h"
#include "visualize.h"

void Explanation_Logger::visualize_last_output()
{
	thisAgent->visualizer->viz_graph_start();
    if (!last_printed_id)
    {
//        visualize_chunk_explanation();
        visualize_explanation_trace();
    } else {
        visualize_instantiation_explanation_for_id(last_printed_id);
    }
    thisAgent->visualizer->viz_graph_end();
}


void Explanation_Logger::visualize_explanation_trace()
{
    for (auto it = current_discussed_chunk->backtraced_inst_records->begin(); it != current_discussed_chunk->backtraced_inst_records->end(); it++)
    {
    	viz_instantiation((*it));
    }
    for (auto it = current_discussed_chunk->backtraced_inst_records->begin(); it != current_discussed_chunk->backtraced_inst_records->end(); it++)
    {
    	(*it)->viz_connect_conditions();
    }
    visualize_chunk_explanation();
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
    viz_instantiation(iter_inst->second);
    return true;
}

void Explanation_Logger::visualize_chunk_explanation()
{
		current_discussed_chunk->visualize();
}

void Explanation_Logger::viz_action_list(action_record_list* pActionRecords, production* pOriginalRule, action* pRhs)
{
    if (pActionRecords->empty())
    {
        thisAgent->visualizer->viz_text_record("Empty RHS");
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
                    thisAgent->visualizer->viz_text_record("No RETE rule");
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
                thisAgent->visualizer->viz_record_line_end();
            }
            if (!print_explanation_trace)
            {
                thisAgent->visualizer->viz_wt_preference(lAction->instantiated_pref, lAction->actionID);
            } else {
                thisAgent->visualizer->viz_et_action(rhs, lAction->variablized_action, lAction->instantiated_pref, lAction->actionID);
                rhs = rhs->next;
            }
        }
        thisAgent->visualizer->graphviz_output += "\n";
        if (print_explanation_trace)
        {
            /* If top exists, we generated conditions here and must deallocate. */
            if (pRhs) deallocate_action_list(thisAgent, pRhs);
            if (top) deallocate_condition_list(thisAgent, top);
        }
        thisAgent->outputManager->clear_print_test_format();
    }
}

void Explanation_Logger::viz_instantiation(instantiation_record* pInstRecord)
{
    if (print_explanation_trace)
	{
    	viz_et_instantiation(pInstRecord);
	} else {
		viz_wm_instantiation(pInstRecord);
	}
}

void Explanation_Logger::viz_wm_instantiation(instantiation_record* pInstRecord)
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

        thisAgent->outputManager->set_print_test_format(false, true);
        thisAgent->visualizer->viz_rule_start(pInstRecord->production_name, pInstRecord->instantiationID, viz_inst_record);

        for (condition_record_list::iterator it = pInstRecord->conditions->begin(); it != pInstRecord->conditions->end(); it++)
        {
            lCond = (*it);
            ++lConditionCount;
            if (lConditionCount > 1)
            {
                thisAgent->visualizer->viz_record_line_end();
            }
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
        viz_action_list(pInstRecord->actions, pInstRecord->original_production, rhs);
        thisAgent->visualizer->viz_rule_end();
    }
}
void Explanation_Logger::viz_et_instantiation(instantiation_record* pInstRecord)
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
        		viz_instantiation(pInstRecord);
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

        thisAgent->visualizer->viz_rule_start(pInstRecord->production_name, pInstRecord->instantiationID, viz_inst_record);

        for (condition_record_list::iterator it = pInstRecord->conditions->begin(); it != pInstRecord->conditions->end(); it++)
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
        viz_action_list(pInstRecord->actions, pInstRecord->original_production, rhs);
        if (pInstRecord->original_production && pInstRecord->original_production->p_node)
        {
            deallocate_condition_list(thisAgent, top);
        }
        thisAgent->visualizer->viz_rule_end();
    }
}

