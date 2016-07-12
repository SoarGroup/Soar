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
#include "output_manager.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoLearn(const LearnBitset& options)
{
    // No options means print current settings
    agent* thisAgent = m_pAgentSML->GetSoarAgent();
    if (options.none() || options.test(LEARN_LIST))
    {
        if (!m_RawOutput)
        {
            AppendArgTagFast(sml_Names::kParamLearnSetting, sml_Names::kTypeBoolean, thisAgent->ebChunker->ebc_settings[SETTING_EBC_LEARNING_ON] ? sml_Names::kTrue : sml_Names::kFalse);
            AppendArgTagFast(sml_Names::kParamLearnOnlySetting, sml_Names::kTypeBoolean, thisAgent->ebChunker->ebc_settings[SETTING_EBC_ONLY]  ? sml_Names::kTrue : sml_Names::kFalse);
            AppendArgTagFast(sml_Names::kParamLearnExceptSetting, sml_Names::kTypeBoolean, thisAgent->ebChunker->ebc_settings[SETTING_EBC_EXCEPT] ? sml_Names::kTrue : sml_Names::kFalse);
            AppendArgTagFast(sml_Names::kParamLearnAllLevelsSetting, sml_Names::kTypeBoolean, thisAgent->ebChunker->ebc_settings[SETTING_EBC_BOTTOM_ONLY]  ? sml_Names::kTrue : sml_Names::kFalse);
        }
        PrintCLIMessage_Header("Learn Settings", 40);
        PrintCLIMessage_Justify("learning:", (thisAgent->ebChunker->ebc_settings[SETTING_EBC_LEARNING_ON] ? "on" : "off"), 40);
        PrintCLIMessage_Justify("only:", (thisAgent->ebChunker->ebc_settings[SETTING_EBC_ONLY]  ? "on" : "off"), 40);
        PrintCLIMessage_Justify("except:", (thisAgent->ebChunker->ebc_settings[SETTING_EBC_EXCEPT] ? "on" : "off"), 40);
        PrintCLIMessage_Justify("all-levels:", (thisAgent->ebChunker->ebc_settings[SETTING_EBC_BOTTOM_ONLY]  ? "on" : "off"), 40);
        PrintCLIMessage_Justify("local-negations:", (thisAgent->ebChunker->ebc_settings[SETTING_EBC_ALLOW_CONFLATED] ? "on" : "off"), 40);
        PrintCLIMessage_Justify("desirability-prefs:", (thisAgent->ebChunker->ebc_settings[SETTING_EBC_OSK] ? "on" : "off"), 40);

        if (options.test(LEARN_LIST))
        {
            std::string output;
            if (thisAgent->ebChunker->ebc_settings[SETTING_EBC_ONLY] )
            {
                PrintCLIMessage_Section("Only Learning In States", 40);
                if (!thisAgent->ebChunker->chunky_problem_spaces)
                {
                    PrintCLIMessage("No current learning states.\n");
                } else
                {
                    for (cons* c = thisAgent->ebChunker->chunky_problem_spaces; c != NIL; c = c->rest)
                    {
                        thisAgent->outputManager->sprinta_sf(thisAgent, output, "%y\n", static_cast<Symbol*>(c->first));
                        PrintCLIMessage(output.c_str());
                        output.clear();
                    }
                }
            } else if (thisAgent->ebChunker->ebc_settings[SETTING_EBC_EXCEPT])
            {
                PrintCLIMessage_Section("Learning in All States Except", 40);
                if (!thisAgent->ebChunker->chunky_problem_spaces)
                {
                    PrintCLIMessage("Currently learning in all states.\n");
                } else
                {
                    for (cons* c = thisAgent->ebChunker->chunk_free_problem_spaces; c != NIL; c = c->rest)
                    {
                        thisAgent->outputManager->sprinta_sf(thisAgent, output, "%y\n", static_cast<Symbol*>(c->first));
                        PrintCLIMessage(output.c_str());
                        output.clear();
                    }
                }
            }
        }
        return true;
    }

    if (options.test(LEARN_ONLY))
    {
        thisAgent->ebChunker->ebc_settings[SETTING_EBC_LEARNING_ON] = true;
        thisAgent->ebChunker->ebc_settings[SETTING_EBC_ONLY] = true;
        thisAgent->ebChunker->ebc_settings[SETTING_EBC_EXCEPT] = false;
        PrintCLIMessage("Learn| only = on");
    }

    if (options.test(LEARN_EXCEPT))
    {
        thisAgent->ebChunker->ebc_settings[SETTING_EBC_LEARNING_ON] = true;
        thisAgent->ebChunker->ebc_settings[SETTING_EBC_ONLY] = false;
        thisAgent->ebChunker->ebc_settings[SETTING_EBC_EXCEPT] = true;
        PrintCLIMessage("Learn| except = on");
    }

    if (options.test(LEARN_ENABLE))
    {
        thisAgent->ebChunker->ebc_settings[SETTING_EBC_LEARNING_ON] = true;
        thisAgent->ebChunker->ebc_settings[SETTING_EBC_ONLY] = false;
        thisAgent->ebChunker->ebc_settings[SETTING_EBC_EXCEPT] = false;
        PrintCLIMessage("Learn| learning = on");
    }

    if (options.test(LEARN_DISABLE))
    {
        thisAgent->ebChunker->ebc_settings[SETTING_EBC_LEARNING_ON] = false;
        thisAgent->ebChunker->ebc_settings[SETTING_EBC_ONLY] = false;
        thisAgent->ebChunker->ebc_settings[SETTING_EBC_EXCEPT] = false;
        PrintCLIMessage("Learn| learning = off");
    }

    if (options.test(LEARN_ALL_LEVELS))
    {
        thisAgent->ebChunker->ebc_settings[SETTING_EBC_BOTTOM_ONLY] = false;
        PrintCLIMessage("Learn| all-levels = on");
    }

    if (options.test(LEARN_BOTTOM_UP))
    {
        thisAgent->ebChunker->ebc_settings[SETTING_EBC_BOTTOM_ONLY] = true;
        PrintCLIMessage("Learn| all-levels = off");
    }

    if (options.test(LEARN_ENABLE_THROUGH_LOCAL_NEGATIONS))
    {
        thisAgent->ebChunker->ebc_settings[SETTING_EBC_ALLOW_LOCAL_NEGATIONS] = true;
        PrintCLIMessage("Learn| local-negations = on");
    }

    if (options.test(LEARN_DISABLE_THROUGH_LOCAL_NEGATIONS))
    {
        thisAgent->ebChunker->ebc_settings[SETTING_EBC_ALLOW_LOCAL_NEGATIONS] = false;
        PrintCLIMessage("Learn| local-negations = off");
    }

    if (options.test(LEARN_ENABLE_THROUGH_EVALUATION_RULES))
    {
        thisAgent->ebChunker->ebc_settings[SETTING_EBC_OSK] = true;
        PrintCLIMessage("Learn| desirability-prefs = on");
    }

    if (options.test(LEARN_DISABLE_THROUGH_EVALUATION_RULES))
    {
        thisAgent->ebChunker->ebc_settings[SETTING_EBC_OSK] = false;
        PrintCLIMessage("Learn| desirability-prefs = off");
    }

    thisAgent->ebChunker->update_learning_on();
    return true;
}

