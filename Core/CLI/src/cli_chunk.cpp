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
#include "explain.h"
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
            PrintCLIMessage_Item("Learn rules from states", thisAgent->ebChunker->ebc_params->enabled, 61);
            PrintCLIMessage_Item("Learn only from lowest sub-state", thisAgent->ebChunker->ebc_params->bottom_level_only, 61);
            PrintCLIMessage_Item("Interrupt after learning rule", thisAgent->ebChunker->ebc_params->interrupt_on_chunk, 61);
            PrintCLIMessage_Item("Record utility instead of firing", thisAgent->ebChunker->ebc_params->utility_mode, 61);
            PrintCLIMessage_Header("Mechanisms Enabled", 61);
            PrintCLIMessage_Item("Learn from operator selection knowledge", thisAgent->ebChunker->ebc_params->mechanism_OSK, 61);
            PrintCLIMessage_Item("Variablize symbols based on identity analysis", thisAgent->ebChunker->ebc_params->mechanism_identity_analysis, 61);
            PrintCLIMessage_Item("Variablize and compose RHS functions", thisAgent->ebChunker->ebc_params->mechanism_variablize_rhs_funcs, 61);
            PrintCLIMessage_Item("Track and enforce transitive constraints", thisAgent->ebChunker->ebc_params->mechanism_constraints, 61);
            PrintCLIMessage_Item("Repair rules with unconnected RHS actions", thisAgent->ebChunker->ebc_params->mechanism_repair_rhs, 61);
            PrintCLIMessage_Item("Repair rules with unconnected LHS conditions", thisAgent->ebChunker->ebc_params->mechanism_repair_lhs, 61);
            PrintCLIMessage_Item("Repair rules that augment previous results", thisAgent->ebChunker->ebc_params->mechanism_promotion_tracking, 61);
            PrintCLIMessage_Item("Merge redundant conditions", thisAgent->ebChunker->ebc_params->mechanism_merge, 61);
            PrintCLIMessage_Item("Unify identities using domain-specific singletons", thisAgent->ebChunker->ebc_params->mechanism_user_singletons, 61);
            PrintCLIMessage_Header("Learn rules despite problem-solving that...", 61);
            PrintCLIMessage_Item("Used local negative reasoning", thisAgent->ebChunker->ebc_params->allow_missing_negative_reasoning, 61);
            PrintCLIMessage_Item("Used operator selection knowledge", thisAgent->ebChunker->ebc_params->allow_missing_OSK, 61);
            PrintCLIMessage_Item("Used knowledge retrieved from memory subsystem", thisAgent->ebChunker->ebc_params->allow_opaque_knowledge, 61);
            PrintCLIMessage_Item("Selects operators probabilistically", thisAgent->ebChunker->ebc_params->allow_probabilistic_operators, 61);
            PrintCLIMessage_Item("Tests a WME that has multiple reasons it exists", thisAgent->ebChunker->ebc_params->allow_multiple_prefs, 61);
            PrintCLIMessage_Item("Tests retrieved LTM that already existed in higher goal", thisAgent->ebChunker->ebc_params->allow_temporal_constraint, 61);
            PrintCLIMessage_Item("Creates a result by augmenting a previous result", thisAgent->ebChunker->ebc_params->allow_local_promotion, 61);

            if (thisAgent->ebChunker->ebc_settings[SETTING_EBC_ONLY] )
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
            } else if (thisAgent->ebChunker->ebc_settings[SETTING_EBC_EXCEPT])
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
    else if (pOp == 'G')
    {
        soar_module::param* my_param = thisAgent->ebChunker->ebc_params->get(pAttr->c_str());
        if (!my_param)
        {
            if(thisAgent->ebChunker->ebc_params->enabled->validate_string(pAttr->c_str()))
            {
                thisAgent->ebChunker->ebc_params->enabled->set_string(pAttr->c_str());
                tempStringStream << "Chunking parameter " << pAttr->c_str() << " = " << pVal->c_str();
                PrintCLIMessage(&tempStringStream);
                thisAgent->ebChunker->ebc_params->update_ebc_settings(thisAgent, NULL);
                return true;
            } else {
                return SetError("Invalid chunking parameter.  Use 'chunk -?' to see a list of valid settings.");
            }
        }

        PrintCLIMessage_Item("", my_param, 0);
        return true;
    }
    else if (pOp == 'S')
    {
        soar_module::param* my_param = thisAgent->ebChunker->ebc_params->get(pAttr->c_str());
        if (!my_param)
        {
            return SetError("Invalid chunking parameter.  Use 'chunk -?' to see a list of valid settings.");
        }

        if (!my_param->validate_string(pVal->c_str()))
        {
            return SetError("Invalid setting for chunking parameter. Use 'chunk -?' to see a list of valid settings.");
        }

        bool result = my_param->set_string(pVal->c_str());

        if (!result)
        {
            return SetError("The chunking parameter could not be changed.");
        }
        else
        {
            tempStringStream << "Chunking parameter " << pAttr->c_str() << " = " << pVal->c_str();
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
    else if (pOp == 's')
    {
        thisAgent->explanationLogger->print_global_stats();
        return true;
    }
    else if (pOp == 'h')
    {

        PrintCLIMessage_Header("History", 40);
        return true;
    }

    return true;
}

