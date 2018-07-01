/*
 * decider.cpp
 *
 *  Created on: Sep 11, 2016
 *      Author: mazzin
 */

#include "decider.h"

#include "decide.h"
#include "decider_settings.h"
#include "ebc.h"
#include "ebc_settings.h"
#include "episodic_memory.h"
#include "explanation_memory.h"
#include "output_manager.h"
#include "reinforcement_learning.h"
#include "semantic_memory.h"
#ifndef NO_SVS
#include "svs_interface.h"
#endif
#include "working_memory_activation.h"

#include "sml_Names.h"

SoarDecider::SoarDecider(agent* myAgent)
{
    /* Cache agent and Output Manager pointer */
    thisAgent = myAgent;
    outputManager = myAgent->outputManager;

    thisAgent->Decider = this;
    last_dc = 0;
    last_fc = 0;
    last_cl = thisAgent->num_productions_of_type[CHUNK_PRODUCTION_TYPE];

    params = new decider_param_container(thisAgent, settings);

}

void SoarDecider::get_enabled_module_strings(std::string &enabledStr, std::string &disabledStr)
{
    bool ebcEnabled = (thisAgent->explanationBasedChunker->ebc_params->chunk_in_states->get_value() != ebc_never);
    bool smemEnabled = thisAgent->SMem->enabled();
    bool epmemEnabled = epmem_enabled(thisAgent);
    bool svsEnabled = false;
    #ifndef NO_SVS
        svsEnabled = thisAgent->svs->is_enabled();
    #endif
    bool rlEnabled = rl_enabled(thisAgent);
    bool wmaEnabled = wma_enabled(thisAgent);
    bool spreadingEnabled = false;
    enabledStr = "Core";
    bool notFirstEnabledItem = true, notFirstDisabledItem = false;
    if (ebcEnabled)
    {
        if (notFirstEnabledItem) enabledStr += ", "; else notFirstEnabledItem = true;
        enabledStr += "EBC";
    } else {
        if (notFirstDisabledItem) disabledStr += ", "; else notFirstDisabledItem = true;
        disabledStr += "EBC";
    }

    if (smemEnabled)
    {
        if (notFirstEnabledItem) enabledStr += ", "; else notFirstEnabledItem = true;
        enabledStr += "SMem";
    } else {
        if (notFirstDisabledItem) disabledStr += ", "; else notFirstDisabledItem = true;
        disabledStr += "SMem";
    }
    if (epmemEnabled)
    {
        if (notFirstEnabledItem) { enabledStr += ", "; notFirstEnabledItem = true; }
        enabledStr += "EpMem";
    } else {
        if (notFirstDisabledItem) disabledStr += ", "; else notFirstDisabledItem = true;
        disabledStr += "EpMem";
    }
    if (svsEnabled)
    {
        if (notFirstEnabledItem) { enabledStr += ", "; notFirstEnabledItem = true; }
        enabledStr += "SVS";
    } else {
        if (notFirstDisabledItem) disabledStr += ", "; else notFirstDisabledItem = true;
        disabledStr += "SVS";
    }
    if (rlEnabled)
    {
        if (notFirstEnabledItem) { enabledStr += ", "; notFirstEnabledItem = true; }
        enabledStr += "RL";
    } else {
        if (notFirstDisabledItem) disabledStr += ", "; else notFirstDisabledItem = true;
        disabledStr += "RL";
    }
    if (wmaEnabled)
    {
        if (notFirstEnabledItem) { enabledStr += ", "; notFirstEnabledItem = true; }
        enabledStr += "WMA";
    } else {
        if (notFirstDisabledItem) disabledStr += ", "; else notFirstDisabledItem = true;
        disabledStr += "WMA";
    }
    if (spreadingEnabled)
    {
        if (notFirstEnabledItem) { enabledStr += ", "; notFirstEnabledItem = true; }
        enabledStr += "SSA";
    } else {
        if (notFirstDisabledItem) disabledStr += ", "; else notFirstDisabledItem = true;
        disabledStr += "SSA";
    }
}

int SoarDecider::get_state_stack_string(std::string &stateStackStr)
{
    int soarStackDepth = 1;
    Symbol* lState = thisAgent->top_goal;
    bool notFirstItem;

    while (lState->id->lower_goal)
    {
        soarStackDepth++;
        lState = lState->id->lower_goal;
    }
    if (soarStackDepth < 4)
    {
        lState = thisAgent->top_goal;
        notFirstItem = false;
        while (lState)
        {
            if (notFirstItem) stateStackStr += ", "; else notFirstItem = true;
            stateStackStr.append(lState->to_string());
            lState = lState->id->lower_goal;
        }
    } else {
        stateStackStr.append(thisAgent->top_goal->to_string());
        stateStackStr.append(", ");
        stateStackStr.append(thisAgent->top_goal->id->lower_goal->to_string());
        if (soarStackDepth > 4)
            stateStackStr.append(" ... ");
        else
            stateStackStr.append(", ");
        stateStackStr.append(thisAgent->bottom_goal->id->higher_goal->to_string());
        stateStackStr.append(", ");
        stateStackStr.append(thisAgent->bottom_goal->to_string());
    }
    return soarStackDepth;
}

void SoarDecider::clean_up_for_agent_deletion()
{
    delete params;
}

void SoarDecider::before_run()
{
    if (thisAgent->d_cycle_count > 1) last_dc = thisAgent->d_cycle_count; else last_dc = 0;
    last_fc = thisAgent->production_firing_count;
    last_cl = thisAgent->num_productions_of_type[CHUNK_PRODUCTION_TYPE];
}

void SoarDecider::get_run_result_string(std::string &runResultStr)
{
    uint64_t now_dc, now_fc, now_cl;
    uint64_t dc_delta, fc_delta, cl_delta;
    bool result_started, result_finished;
    result_started = result_finished = false;
    now_dc = thisAgent->d_cycle_count;
    now_fc = thisAgent->production_firing_count;
    now_cl = thisAgent->num_productions_of_type[CHUNK_PRODUCTION_TYPE];
    dc_delta = now_dc - last_dc;
    fc_delta = now_fc - last_fc;
    cl_delta = now_cl - last_cl;

	runResultStr += "\n--> ";
    runResultStr.append(std::to_string(dc_delta));

    runResultStr += (dc_delta > 1 ? " decision cycles executed. " : " decision cycle executed. ");
    if (fc_delta)
    {
    	runResultStr.append(std::to_string(fc_delta));
    	runResultStr += ( fc_delta > 1 ? " rules fired. " : " rule fired. ");
    } else runResultStr += "No rules fired. ";
    if (cl_delta)
    {
    	runResultStr.append(std::to_string(cl_delta));
    	runResultStr += ( cl_delta > 1 ? " new rules learned." : " new rule learned.");
    }
    last_dc = now_dc;
    last_fc = now_fc;
    last_cl = now_cl;
}
