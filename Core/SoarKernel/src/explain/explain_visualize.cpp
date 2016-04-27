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
        visualize_explanation_trace();
    } else {
        visualize_instantiation_explanation_for_id(last_printed_id);
    }
    thisAgent->visualizer->viz_graph_end();
}


void Explanation_Logger::visualize_explanation_trace()
{
    if (thisAgent->visualizer->is_include_chunk_enabled())
    {
        current_discussed_chunk->visualize();
    }
    for (auto it = current_discussed_chunk->backtraced_inst_records->begin(); it != current_discussed_chunk->backtraced_inst_records->end(); it++)
    {
    	viz_instantiation((*it));
    }
    for (auto it = current_discussed_chunk->backtraced_inst_records->begin(); it != current_discussed_chunk->backtraced_inst_records->end(); it++)
    {
        (*it)->viz_connect_conditions();
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
    viz_instantiation(iter_inst->second);
    return true;
}

void Explanation_Logger::viz_instantiation(instantiation_record* pInstRecord)
{
    if (thisAgent->visualizer->is_simple_inst_enabled())
    {
        pInstRecord->viz_simple_instantiation();
    } else {
        if (print_explanation_trace)
        {
            pInstRecord->viz_et_instantiation();
        } else {
            pInstRecord->viz_wm_instantiation();
        }
    }
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
                thisAgent->visualizer->viz_endl();
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




