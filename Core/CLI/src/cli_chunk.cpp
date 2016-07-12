/////////////////////////////////////////////////////////////////
// learn command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include "portability.h"

#include "sml_Utils.h"
#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"

#include "sml_Names.h"
#include "sml_AgentSML.h"

#include "sml_KernelSML.h"
#include "gsysparam.h"
#include "agent.h"
#include "print.h"
#include "ebc.h"
#include "settings.h"
#include "output_manager.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoChunk(const char pOp, const std::string* pAttr, const std::string* pVal)
{
    // No options means print current settings
    agent* thisAgent = m_pAgentSML->GetSoarAgent();
    std::ostringstream tempStringStream;
    std::string tempString;

    if (!pOp)
    {
        PrintCLIMessage_Header("Chunking Settings", 61);
        PrintCLIMessage_Item("Sub-states agent will learn rules from", thisAgent->ebChunker->ebc_params->enabled, 61);
        PrintCLIMessage_Item("Learn only from lowest sub-state", thisAgent->ebChunker->ebc_params->bottom_level_only, 61);
        PrintCLIMessage_Item("Interrupt after learning rule", thisAgent->ebChunker->ebc_params->interrupt_on_chunk, 61);
        PrintCLIMessage_Item("Ignore :do-not-backtrace flags", thisAgent->ebChunker->ebc_params->ignore_dnb, 61);
        PrintCLIMessage_Header("Mechanisms Enabled", 61);
        PrintCLIMessage_Item("Variablize rule based on identity analysis", thisAgent->ebChunker->ebc_params->mechanism_identity_analysis, 61);
        PrintCLIMessage_Item("Enforce problem-solving constraints", thisAgent->ebChunker->ebc_params->mechanism_constraints, 61);
        PrintCLIMessage_Item("Incorporate reasoning used to select operators", thisAgent->ebChunker->ebc_params->mechanism_OSK, 61);
        PrintCLIMessage_Item("Compose and variablize RHS functions", thisAgent->ebChunker->ebc_params->mechanism_variablize_rhs_funcs, 61);
        PrintCLIMessage_Item("Repair rules with unconnected RHS actions", thisAgent->ebChunker->ebc_params->mechanism_repair_rhs, 61);
        PrintCLIMessage_Item("Repair rules with unconnected LHS conditions", thisAgent->ebChunker->ebc_params->mechanism_repair_lhs, 61);
        PrintCLIMessage_Item("Repair rules that augment previous results", thisAgent->ebChunker->ebc_params->mechanism_promotion_tracking, 61);
        PrintCLIMessage_Item("Merge redundant conditions", thisAgent->ebChunker->ebc_params->mechanism_merge, 61);
        PrintCLIMessage_Item("Unify identities using domain-specific singletons", thisAgent->ebChunker->ebc_params->mechanism_user_singletons, 61);
        PrintCLIMessage_Header("Learn rules even when underlying problem-solving...", 61);
        PrintCLIMessage_Item("Used local negative reasoning", thisAgent->ebChunker->ebc_params->allow_missing_negative_reasoning, 61);
        PrintCLIMessage_Item("Used operator selection knowledge", thisAgent->ebChunker->ebc_params->allow_missing_OSK, 61);
        PrintCLIMessage_Item("Used knowledge retrieved from semantic memory", thisAgent->ebChunker->ebc_params->allow_smem_knowledge, 61);
        PrintCLIMessage_Item("Selects operators probabilistically", thisAgent->ebChunker->ebc_params->allow_probabilistic_operators, 61);
        PrintCLIMessage_Item("Tests a WME that has multiple reasons it exists", thisAgent->ebChunker->ebc_params->allow_multiple_prefs, 61);
        PrintCLIMessage_Item("Tests retrieved LTM that already exists in higher goal", thisAgent->ebChunker->ebc_params->allow_temporal_constraint, 61);
        PrintCLIMessage_Item("Creates a result by augmenting a previous result", thisAgent->ebChunker->ebc_params->allow_local_promotion, 61);

        if (thisAgent->sysparams[LEARNING_ONLY_SYSPARAM])
        {
            PrintCLIMessage_Section("Only Learning In States", 61);
            if (!thisAgent->ebChunker->chunky_problem_spaces)
            {
                PrintCLIMessage("No current learning states.\n");
            } else
            {
                for (cons* c = thisAgent->ebChunker->chunky_problem_spaces; c != NIL; c = c->rest)
                {
                    thisAgent->outputManager->sprinta_sf(thisAgent, tempString, "%y\n", static_cast<Symbol*>(c->first));
                    PrintCLIMessage(tempString.c_str());
                    tempString.clear();
                }
            }
        } else if (thisAgent->sysparams[LEARNING_EXCEPT_SYSPARAM])
        {
            PrintCLIMessage_Section("Learning in All States Except", 61);
            if (!thisAgent->ebChunker->chunky_problem_spaces)
            {
                PrintCLIMessage("Currently learning in all states.\n");
            } else
            {
                for (cons* c = thisAgent->ebChunker->chunk_free_problem_spaces; c != NIL; c = c->rest)
                {
                    thisAgent->outputManager->sprinta_sf(thisAgent, tempString, "%y\n", static_cast<Symbol*>(c->first));
                    PrintCLIMessage(tempString.c_str());
                    tempString.clear();
                }
            }
        }

        return true;
    }
    else if (pOp == 'g')
    {
        soar_module::param* my_param = thisAgent->ebChunker->ebc_params->get(pAttr->c_str());
        if (!my_param)
        {
            return SetError("Invalid chunking parameter.  Use 'help chunk' to see a list of valid settings.");
        }

        PrintCLIMessage_Item("", my_param, 0);
        return true;
    }
    else if (pOp == 's')
    {
        soar_module::param* my_param = thisAgent->ebChunker->ebc_params->get(pAttr->c_str());
        if (!my_param)
        {
            return SetError("Invalid chunking parameter.");
        }

        if (!my_param->validate_string(pVal->c_str()))
        {
            return SetError("Invalid setting for chunking parameter.");
        }

        bool result = my_param->set_string(pVal->c_str());

        if (!result)
        {
            SetError("Error:  The chunking parameter could not be changed.");
        }
        else
        {
            tempStringStream << pAttr->c_str() << " = " << pVal->c_str();
            PrintCLIMessage(&tempStringStream);
        }
        /* The following code assumes that all parameters except learn are boolean */
        if (!strcmp(pAttr->c_str(), "learn"))
        {
            thisAgent->ebChunker->ebc_params->update_ebc_settings(thisAgent, NULL);
        } else {
            thisAgent->ebChunker->ebc_params->update_ebc_settings(thisAgent, static_cast<soar_module::boolean_param*>(my_param));
        }
        return result;
    }
    else if (pOp == 'S')
    {

        PrintCLIMessage_Header("Semantic Memory Statistics", 40);
        PrintCLIMessage_Item("SQLite Version", thisAgent->smem_stats->db_lib_version, 40);
        PrintCLIMessage_Item("Memory Usage", thisAgent->smem_stats->mem_usage, 40);
        PrintCLIMessage_Item("Memory Highwater", thisAgent->smem_stats->mem_high, 40);
        PrintCLIMessage_Item("Retrieves", thisAgent->smem_stats->expansions, 40);
        PrintCLIMessage_Item("Queries", thisAgent->smem_stats->cbr, 40);
        PrintCLIMessage_Item("Stores", thisAgent->smem_stats->stores, 40);
        PrintCLIMessage_Item("Activation Updates", thisAgent->smem_stats->act_updates, 40);
        PrintCLIMessage_Item("Mirrors", thisAgent->smem_stats->mirrors, 40);
        PrintCLIMessage_Item("Nodes", thisAgent->smem_stats->chunks, 40);
        PrintCLIMessage_Item("Edges", thisAgent->smem_stats->slots, 40);

        return true;
    }

    return true;
}

