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
    outputManager = &Output_Manager::Get_OM();

    thisAgent->Decider = this;

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

void SoarDecider::print_soar_status(sml::KernelSML* pKernelSML)
{
    std::string stateStackStr, enabledStr, disabledStr;
    int soarStackDepth;

    soarStackDepth = get_state_stack_string(stateStackStr);
    get_enabled_module_strings(enabledStr, disabledStr);
    uint64_t totalProductions =
        thisAgent->num_productions_of_type[DEFAULT_PRODUCTION_TYPE] +
        thisAgent->num_productions_of_type[USER_PRODUCTION_TYPE] +
        thisAgent->num_productions_of_type[CHUNK_PRODUCTION_TYPE];



    outputManager->reset_column_indents();
    outputManager->set_column_indent(0, 55);

    outputManager->printa(thisAgent,    "=======================================================\n");
    outputManager->printa_sf(thisAgent,    "                     Soar %s Summary\n", sml::sml_Names::kSoarVersionValue);
    outputManager->printa(thisAgent,    "=======================================================\n");
    outputManager->printa_sf(thisAgent, "%s\n", concatJustified("Enabled:", enabledStr.c_str(), 55).c_str());
    outputManager->printa_sf(thisAgent, "%s\n", concatJustified("Disabled:", disabledStr.c_str(), 55).c_str());
    outputManager->printa(thisAgent,    "-------------------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "%s\n", concatJustified("Number of rules:", std::to_string(totalProductions).c_str(), 55).c_str());
    outputManager->printa_sf(thisAgent, "%s\n", concatJustified("Decisions", std::to_string(thisAgent->d_cycle_count).c_str(), 55).c_str());
    outputManager->printa_sf(thisAgent, "%s\n", concatJustified("Inferences", std::to_string(thisAgent->e_cycle_count).c_str(), 55).c_str());
    outputManager->printa(thisAgent,    "-------------------------------------------------------\n");
    outputManager->printa_sf(thisAgent, "%s\n", concatJustified("State stack", stateStackStr.c_str(), 55).c_str());
    outputManager->printa_sf(thisAgent, "%s\n", concatJustified("Current number of states", std::to_string(soarStackDepth).c_str(), 55).c_str());
    outputManager->printa_sf(thisAgent, "%s\n", concatJustified("Next phase", thisAgent->outputManager->phase_to_string(thisAgent->current_phase), 55).c_str());

    outputManager->printa_sf(thisAgent, "\nFor a full list of sub-commands and settings:  soar ?");
}

void SoarDecider::clean_up_for_agent_deletion()
{
    delete params;
}

