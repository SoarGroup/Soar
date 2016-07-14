/////////////////////////////////////////////////////////////////
// learn command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include "portability.h"

#include "cli_CommandLineInterface.h"
#include "cli_Commands.h"

#include "sml_AgentSML.h"
#include "sml_Names.h"
#include "sml_KernelSML.h"
#include "sml_Utils.h"

#include "agent.h"
#include "ebc.h"
#include "explanation_memory.h"
#include "gsysparam.h"
#include "print.h"
#include "ebc_settings.h"
#include "output_manager.h"

using namespace cli;
using namespace sml;

const char* capitalizeOnOff(bool isEnabled) {
    if (isEnabled)
    {
        return "[ ON | off ]";
    } else {
        return "[ on | OFF ]";
    }
}
bool CommandLineInterface::DoChunk(const char pOp, const std::string* pAttr, const std::string* pVal)
{
    // No options means print current settings
    agent* thisAgent = m_pAgentSML->GetSoarAgent();
    std::ostringstream tempStringStream;
    std::string tempString;

    if (!pOp)
    {

            PrintCLIMessage_Header("Chunking Settings", 65);
            PrintCLIMessage_Item("Learn rules from states", thisAgent->explanationBasedChunker->ebc_params->chunk_in_states, 65);
            PrintCLIMessage_Item("Learn only from lowest sub-state", thisAgent->explanationBasedChunker->ebc_params->bottom_level_only, 65);
            PrintCLIMessage_Item("Maximum rules that can be learned", thisAgent->explanationBasedChunker->ebc_params->max_chunks, 65);
            PrintCLIMessage_Item("Maximum duplicate rules per sub-state (per rule, per phase)", thisAgent->explanationBasedChunker->ebc_params->max_dupes, 65);
            PrintCLIMessage_Item("Interrupt after learning rule", thisAgent->explanationBasedChunker->ebc_params->interrupt_on_chunk, 65);
            PrintCLIMessage_Item("Record utility instead of firing", thisAgent->explanationBasedChunker->ebc_params->utility_mode, 65);
            PrintCLIMessage_Header("Mechanisms Enabled", 65);
            PrintCLIMessage_Item("Learn from operator selection knowledge", thisAgent->explanationBasedChunker->ebc_params->mechanism_OSK, 65);
            PrintCLIMessage_Item("Variablize symbols based on identity analysis", thisAgent->explanationBasedChunker->ebc_params->mechanism_identity_analysis, 65);
            PrintCLIMessage_Item("Variablize and compose RHS functions", thisAgent->explanationBasedChunker->ebc_params->mechanism_variablize_rhs_funcs, 65);
            PrintCLIMessage_Item("Track and enforce transitive constraints", thisAgent->explanationBasedChunker->ebc_params->mechanism_constraints, 65);
            PrintCLIMessage_Item("Repair rules with unconnected RHS actions", thisAgent->explanationBasedChunker->ebc_params->mechanism_repair_rhs, 65);
            PrintCLIMessage_Item("Repair rules with unconnected LHS conditions", thisAgent->explanationBasedChunker->ebc_params->mechanism_repair_lhs, 65);
            PrintCLIMessage_Item("Repair rules that augment previous results", thisAgent->explanationBasedChunker->ebc_params->mechanism_promotion_tracking, 65);
            PrintCLIMessage_Item("Merge redundant conditions", thisAgent->explanationBasedChunker->ebc_params->mechanism_merge, 65);
            PrintCLIMessage_Item("Unify identities using domain-specific singletons", thisAgent->explanationBasedChunker->ebc_params->mechanism_user_singletons, 65);
            PrintCLIMessage_Header("Allow rules to be learned despite problem-solving that...", 65);
            PrintCLIMessage_Item("Used local negative reasoning", thisAgent->explanationBasedChunker->ebc_params->allow_missing_negative_reasoning, 65);
            PrintCLIMessage_Item("Used operator selection rules to choose operator", thisAgent->explanationBasedChunker->ebc_params->allow_missing_OSK, 65);
            PrintCLIMessage_Item("Used knowledge retrieved from memory subsystem", thisAgent->explanationBasedChunker->ebc_params->allow_opaque_knowledge, 65);
            PrintCLIMessage_Item("Used operators selected probabilistically", thisAgent->explanationBasedChunker->ebc_params->allow_probabilistic_operators, 65);
            PrintCLIMessage_Item("Tests a WME that has multiple reasons it exists", thisAgent->explanationBasedChunker->ebc_params->allow_multiple_prefs, 65);
            PrintCLIMessage_Item("Tests retrieved LTM that already existed in higher goal", thisAgent->explanationBasedChunker->ebc_params->allow_temporal_constraint, 65);
            PrintCLIMessage_Item("Creates a result by augmenting a previous result", thisAgent->explanationBasedChunker->ebc_params->allow_local_promotion, 65);

            if (thisAgent->explanationBasedChunker->ebc_settings[SETTING_EBC_ONLY] )
            {
                PrintCLIMessage_Section("Only Learning In States", 65);
                if (!thisAgent->explanationBasedChunker->chunky_problem_spaces)
                {
                    PrintCLIMessage("No current learning states.\n");
                } else
                {
                    for (cons* c = thisAgent->explanationBasedChunker->chunky_problem_spaces; c != NIL; c = c->rest)
                    {
                        thisAgent->outputManager->sprinta_sf(thisAgent, tempString, "%y\n", static_cast<Symbol*>(c->first));
                        PrintCLIMessage(tempString.c_str());
                        tempString.clear();
                    }
                }
            } else if (thisAgent->explanationBasedChunker->ebc_settings[SETTING_EBC_EXCEPT])
            {
                PrintCLIMessage_Section("Learning in All States Except", 65);
                if (!thisAgent->explanationBasedChunker->chunky_problem_spaces)
                {
                    PrintCLIMessage("Currently learning in all states.\n");
                } else
                {
                    for (cons* c = thisAgent->explanationBasedChunker->chunk_free_problem_spaces; c != NIL; c = c->rest)
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
        /* Single command argument */
        soar_module::param* my_param = thisAgent->explanationBasedChunker->ebc_params->get(pAttr->c_str());
        if (!my_param)
        {
            /* Command was not a valid ebc_param name, so it must be a single word command */
            /* Check if it's one of the four chunk enable commands.  (Means no ebc_param name
             * can be named enabled, on off, all disabled, none only all-except*/
            if(thisAgent->explanationBasedChunker->ebc_params->chunk_in_states->validate_string(pAttr->c_str()))
            {
                thisAgent->explanationBasedChunker->ebc_params->chunk_in_states->set_string(pAttr->c_str());
                tempStringStream << "Learns rules in states: " << pAttr->c_str();
                PrintCLIMessage(&tempStringStream);
                thisAgent->explanationBasedChunker->ebc_params->update_ebc_settings(thisAgent, NULL);
                return true;
            } else {
                return SetError("Invalid chunking command.  Use 'chunk ?' to see a list of valid settings.");
            }
        }
        if (my_param == thisAgent->explanationBasedChunker->ebc_params->stats_cmd)
        {
            thisAgent->explanationMemory->print_global_stats();

        }
        else if (my_param == thisAgent->explanationBasedChunker->ebc_params->history_cmd)
        {
            thisAgent->explanationMemory->print_global_stats();
        }
        else if ((my_param == thisAgent->explanationBasedChunker->ebc_params->help_cmd) || (my_param == thisAgent->explanationBasedChunker->ebc_params->qhelp_cmd))
        {
            tempStringStream << "chunk <command> [parameter value]    (leave empty to see current value)\n";
            tempStringStream << "      ============= When to chunk =================== Value ===\n";
            tempStringStream << "      always | never | only | all-except\n";
            tempStringStream << "      bottom-only                                  " << capitalizeOnOff(thisAgent->explanationBasedChunker->ebc_params->bottom_level_only->get_value()) << "\n";
            tempStringStream << "      =============== Settings ====================== Value ===\n";
            tempStringStream << "      naming-style                          [ " <<
                                ((thisAgent->explanationBasedChunker->ebc_params->naming_style->get_value() == ruleFormat) ?  "numbered" : "NUMBERED") << " | " <<
                                ((thisAgent->explanationBasedChunker->ebc_params->naming_style->get_value() == ruleFormat) ?  "RULE" : "rule") <<" ]\n";
            std::string sep_string("");
            std::string left_string("      max-chunks");
            std::string right_string(thisAgent->explanationBasedChunker->ebc_params->max_chunks->get_string());
            sep_string.insert(0, 63 - left_string.length() - right_string.length(), ' ');
            tempStringStream << left_string << sep_string << right_string << '\n';
            sep_string = "";
            left_string = "      max-dupes";
            right_string = thisAgent->explanationBasedChunker->ebc_params->max_dupes->get_string();
            sep_string.insert(0, 63 - left_string.length() - right_string.length(), ' ');
            tempStringStream << left_string << sep_string << right_string << '\n';
            tempStringStream << "      =========== Debugging Commands ================ Value ===\n";
            tempStringStream << "      ? | help | history | stats \n";
            tempStringStream << "      interrupt                                    " << capitalizeOnOff(thisAgent->explanationBasedChunker->ebc_params->bottom_level_only->get_value()) << "\n";
            tempStringStream << "      record-utility                               " << capitalizeOnOff(thisAgent->explanationBasedChunker->ebc_params->utility_mode->get_value()) << "\n";
            tempStringStream << "      ============= EBC Mechanisms ================== Value ===\n";
            tempStringStream << "      add-osk                                      " << capitalizeOnOff(thisAgent->explanationBasedChunker->ebc_params->mechanism_OSK->get_value()) << "\n";
            tempStringStream << "      variablize-identity                          " << capitalizeOnOff(thisAgent->explanationBasedChunker->ebc_params->mechanism_identity_analysis->get_value()) << "\n";
            tempStringStream << "      variablize-rhs-funcs                         " << capitalizeOnOff(thisAgent->explanationBasedChunker->ebc_params->mechanism_variablize_rhs_funcs->get_value()) << "\n";
            tempStringStream << "      enforce-constraints                          " << capitalizeOnOff(thisAgent->explanationBasedChunker->ebc_params->mechanism_constraints->get_value()) << "\n";
            tempStringStream << "      repair-rhs                                   " << capitalizeOnOff(thisAgent->explanationBasedChunker->ebc_params->mechanism_repair_rhs->get_value()) << "\n";
            tempStringStream << "      repair-lhs                                   " << capitalizeOnOff(thisAgent->explanationBasedChunker->ebc_params->mechanism_repair_lhs->get_value()) << "\n";
            tempStringStream << "      repair-rhs-promotion                         " << capitalizeOnOff(thisAgent->explanationBasedChunker->ebc_params->mechanism_promotion_tracking->get_value()) << "\n";
            tempStringStream << "      merge                                        " << capitalizeOnOff(thisAgent->explanationBasedChunker->ebc_params->mechanism_merge->get_value()) << "\n";
            tempStringStream << "      user-singletons                              " << capitalizeOnOff(thisAgent->explanationBasedChunker->ebc_params->mechanism_user_singletons->get_value()) << "\n";
            tempStringStream << "      ========== Correctness Filters ================ Value ===\n";
            tempStringStream << "      allow-local-negations                        " << capitalizeOnOff(thisAgent->explanationBasedChunker->ebc_params->allow_missing_negative_reasoning->get_value()) << "\n";
            tempStringStream << "      allow-missing-osk                            " << capitalizeOnOff(thisAgent->explanationBasedChunker->ebc_params->allow_missing_OSK->get_value()) << "\n";
            tempStringStream << "      allow-opaque                                 " << capitalizeOnOff(thisAgent->explanationBasedChunker->ebc_params->allow_opaque_knowledge->get_value()) << "\n";
            tempStringStream << "      allow-uncertain-operators                    " << capitalizeOnOff(thisAgent->explanationBasedChunker->ebc_params->allow_probabilistic_operators->get_value()) << "\n";
            tempStringStream << "      allow-multiple-prefs                         " << capitalizeOnOff(thisAgent->explanationBasedChunker->ebc_params->allow_multiple_prefs->get_value()) << "\n";
            tempStringStream << "      allow-pre-existing-ltm                       " << capitalizeOnOff(thisAgent->explanationBasedChunker->ebc_params->allow_temporal_constraint->get_value()) << "\n";
            tempStringStream << "      allow-local-promotion                        " << capitalizeOnOff(thisAgent->explanationBasedChunker->ebc_params->allow_local_promotion->get_value()) << "\n";
            PrintCLIMessage(tempStringStream.str().c_str(), false);
        }
        else {
            /* Command was a valid ebc_param name, so print it's value */
            tempStringStream << "Chunking parameter " << pAttr->c_str() << " =" ;
            PrintCLIMessage_Item(tempStringStream.str().c_str(), my_param, 0);
        }
        return true;
    }
    else if (pOp == 'S')
    {
        soar_module::param* my_param = thisAgent->explanationBasedChunker->ebc_params->get(pAttr->c_str());
        if (!my_param)
        {
            return SetError("Invalid chunking command.  Use 'chunk ?' to see a list of valid settings.");
        }

        if (!my_param->validate_string(pVal->c_str()))
        {
            return SetError("Invalid argument for chunking command. Use 'chunk ?' to see a list of valid settings.");
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
            thisAgent->explanationBasedChunker->ebc_params->update_ebc_settings(thisAgent);
        } else if (!strcmp(pAttr->c_str(), "max-chunks") || !strcmp(pAttr->c_str(), "max-dupes")) {
            thisAgent->explanationBasedChunker->ebc_params->update_ebc_settings(thisAgent, NULL, static_cast<soar_module::integer_param*>(my_param));
        } else {
            thisAgent->explanationBasedChunker->ebc_params->update_ebc_settings(thisAgent, static_cast<soar_module::boolean_param*>(my_param));
        }
        return result;
    }
    else if (pOp == 's')
    {
        thisAgent->explanationMemory->print_global_stats();
        return true;
    }
    else if (pOp == 'h')
    {

        PrintCLIMessage_Header("History", 40);
        return true;
    }

    return true;
}

