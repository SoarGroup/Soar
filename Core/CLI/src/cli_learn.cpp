/////////////////////////////////////////////////////////////////
// learn command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "sml_Utils.h"
#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"

#include "sml_Names.h"
#include "sml_AgentSML.h"

#include "sml_KernelSML.h"
#include "gsysparam.h"
#include "agent.h"
#include "print.h"

using namespace cli;
using namespace sml;

void GetForceLearnStates(agent* thisAgent, std::stringstream& res)
{
    cons* c;
    char buff[1024];
    
    for (c = thisAgent->chunky_problem_spaces; c != NIL; c = c->rest)
    {
        symbol_to_string(thisAgent, static_cast<Symbol*>(c->first), true, buff, 1024);
        res << buff;
    }
}

void GetDontLearnStates(agent* thisAgent, std::stringstream& res)
{
    cons* c;
    char buff[1024];
    
    for (c = thisAgent->chunk_free_problem_spaces; c != NIL; c = c->rest)
    {
        symbol_to_string(thisAgent, static_cast<Symbol*>(c->first), true, buff, 1024);
        res << buff;
    }
}

bool CommandLineInterface::DoLearn(const LearnBitset& options)
{
    // No options means print current settings
    agent* thisAgent = m_pAgentSML->GetSoarAgent();
    if (options.none() || options.test(LEARN_LIST))
    {
        if (!m_RawOutput)
        {
            AppendArgTagFast(sml_Names::kParamLearnSetting, sml_Names::kTypeBoolean, thisAgent->sysparams[LEARNING_ON_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);
            AppendArgTagFast(sml_Names::kParamLearnOnlySetting, sml_Names::kTypeBoolean, thisAgent->sysparams[LEARNING_ONLY_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);
            AppendArgTagFast(sml_Names::kParamLearnExceptSetting, sml_Names::kTypeBoolean, thisAgent->sysparams[LEARNING_EXCEPT_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);
            AppendArgTagFast(sml_Names::kParamLearnAllLevelsSetting, sml_Names::kTypeBoolean, thisAgent->sysparams[LEARNING_ALL_GOALS_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);
        }
        PrintCLIMessage_Header("Learn Settings", 40);
        PrintCLIMessage_Justify("learning:", (thisAgent->sysparams[LEARNING_ON_SYSPARAM] ? "on" : "off"), 40);
        PrintCLIMessage_Justify("only:", (thisAgent->sysparams[LEARNING_ONLY_SYSPARAM] ? "on" : "off"), 40);
        PrintCLIMessage_Justify("except:", (thisAgent->sysparams[LEARNING_EXCEPT_SYSPARAM] ? "on" : "off"), 40);
        PrintCLIMessage_Justify("all-levels:", (thisAgent->sysparams[LEARNING_ALL_GOALS_SYSPARAM] ? "on" : "off"), 40);
        PrintCLIMessage_Justify("local-negations:", (thisAgent->sysparams[CHUNK_THROUGH_LOCAL_NEGATIONS_SYSPARAM] ? "on" : "off"), 40);
        PrintCLIMessage_Justify("desirability-prefs:", (thisAgent->sysparams[CHUNK_THROUGH_EVALUATION_RULES_SYSPARAM] ? "on" : "off"), 40);
        
        if (options.test(LEARN_LIST))
        {
            std::stringstream output;
            PrintCLIMessage_Section("Force-Learn States", 40);
            GetForceLearnStates(thisAgent, output);
            if (output.str().size())
            {
                PrintCLIMessage(output.str().c_str());
            }
            PrintCLIMessage_Section("Dont-Learn States", 40);
            GetDontLearnStates(thisAgent, output);
            if (output.str().size())
            {
                PrintCLIMessage(output.str().c_str());
            }
        }
        return true;
    }
    
    if (options.test(LEARN_ONLY))
    {
        set_sysparam(thisAgent, LEARNING_ON_SYSPARAM, true);
        set_sysparam(thisAgent, LEARNING_ONLY_SYSPARAM, true);
        set_sysparam(thisAgent, LEARNING_EXCEPT_SYSPARAM, false);
        PrintCLIMessage("Learn| only = on");
    }
    
    if (options.test(LEARN_EXCEPT))
    {
        set_sysparam(thisAgent, LEARNING_ON_SYSPARAM, true);
        set_sysparam(thisAgent, LEARNING_ONLY_SYSPARAM, false);
        set_sysparam(thisAgent, LEARNING_EXCEPT_SYSPARAM, true);
        PrintCLIMessage("Learn| except = on");
    }
    
    if (options.test(LEARN_ENABLE))
    {
        set_sysparam(thisAgent, LEARNING_ON_SYSPARAM, true);
        set_sysparam(thisAgent, LEARNING_ONLY_SYSPARAM, false);
        set_sysparam(thisAgent, LEARNING_EXCEPT_SYSPARAM, false);
        PrintCLIMessage("Learn| learning = on");
    }
    
    if (options.test(LEARN_DISABLE))
    {
        set_sysparam(thisAgent, LEARNING_ON_SYSPARAM, false);
        set_sysparam(thisAgent, LEARNING_ONLY_SYSPARAM, false);
        set_sysparam(thisAgent, LEARNING_EXCEPT_SYSPARAM, false);
        PrintCLIMessage("Learn| learning = off");
    }
    
    if (options.test(LEARN_ALL_LEVELS))
    {
        set_sysparam(thisAgent, LEARNING_ALL_GOALS_SYSPARAM, true);
        PrintCLIMessage("Learn| all-levels = on");
    }
    
    if (options.test(LEARN_BOTTOM_UP))
    {
        set_sysparam(thisAgent, LEARNING_ALL_GOALS_SYSPARAM, false);
        PrintCLIMessage("Learn| all-levels = off");
    }
    
    if (options.test(LEARN_ENABLE_THROUGH_LOCAL_NEGATIONS))
    {
        set_sysparam(thisAgent, CHUNK_THROUGH_LOCAL_NEGATIONS_SYSPARAM, true);
        PrintCLIMessage("Learn| local-negations = on");
    }
    
    if (options.test(LEARN_DISABLE_THROUGH_LOCAL_NEGATIONS))
    {
        set_sysparam(thisAgent, CHUNK_THROUGH_LOCAL_NEGATIONS_SYSPARAM, false);
        PrintCLIMessage("Learn| local-negations = off");
    }
    
    if (options.test(LEARN_ENABLE_THROUGH_EVALUATION_RULES))
    {
        set_sysparam(thisAgent, CHUNK_THROUGH_EVALUATION_RULES_SYSPARAM, true);
        PrintCLIMessage("Learn| desirability-prefs = on");
    }
    
    if (options.test(LEARN_DISABLE_THROUGH_EVALUATION_RULES))
    {
        set_sysparam(thisAgent, CHUNK_THROUGH_EVALUATION_RULES_SYSPARAM, false);
        PrintCLIMessage("Learn| desirability-prefs = off");
    }
    
    return true;
}

