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
#include "explanation_memory.h"
#include "output_manager.h"

SoarDecider::SoarDecider(agent* myAgent)
{
    /* Cache agent and Output Manager pointer */
    thisAgent = myAgent;
    outputManager = &Output_Manager::Get_OM();

    thisAgent->Decider = this;

    decider_params = new decider_param_container(thisAgent, decider_settings);

}

void SoarDecider::print_soar_status()
{
    std::string tempString;

    outputManager->reset_column_indents();
    outputManager->set_column_indent(0, 55);

    outputManager->printa(thisAgent,    "=======================================================\n");
    outputManager->printa(thisAgent,    "                     Soar Status\n");
    outputManager->printa(thisAgent,    "=======================================================\n");
    outputManager->printa_sf(thisAgent, "%s\n", concatJustified("When Soar will learn rules", thisAgent->explanationBasedChunker->ebc_params->chunk_in_states->get_string(), 55).c_str());
    outputManager->printa_sf(thisAgent, "%s\n", concatJustified("Chunks learned", std::to_string(thisAgent->explanationMemory->get_stat_succeeded()), 55).c_str());
    outputManager->printa_sf(thisAgent, "%s\n", concatJustified("Chunks attempted", std::to_string(thisAgent->explanationMemory->get_stat_chunks_attempted()), 55).c_str());

    outputManager->printa_sf(thisAgent, "\nFor a full list of soar's sub-commands and settings:  soar ?");
}

void SoarDecider::clean_up_for_agent_deletion()
{
    delete decider_params;
}

