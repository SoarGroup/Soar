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
#include "decider.h"
#include "decider_settings.h"
#include "ebc.h"
#include "ebc_settings.h"
#include "explanation_memory.h"
#include "output_manager.h"
#include "print.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoSoar(const char pOp, const std::string* pAttr, const std::string* pVal)
{
    agent* thisAgent = m_pAgentSML->GetSoarAgent();
    std::ostringstream tempStringStream;
    std::string tempString;

    if (!pOp)
    {
        thisAgent->Decider->print_soar_status();
        return true;
    }
    else if (pOp == 'G')
    {
        /* Single command argument */
        soar_module::param* my_param = thisAgent->Decider->decider_params->get(pAttr->c_str());
        if (!my_param)
        {
            return SetError("Invalid soar sub-command.  Use 'soar ?' to see a list of valid sub-commands and settings.");
        }

        /* Process dummy parameters that are stand-in for commands */
        if ((my_param == thisAgent->Decider->decider_params->help_cmd) || (my_param == thisAgent->Decider->decider_params->qhelp_cmd))
        {
            thisAgent->Decider->decider_params->print_soar_settings(thisAgent);
        }
        else {
            /* Command was a valid ebc_param name, so print it's value */
            tempStringStream << pAttr->c_str() << " =" ;
            PrintCLIMessage_Item(tempStringStream.str().c_str(), my_param, 0);
        }
        return true;
    }
    else if (pOp == 'S')
    {
        soar_module::param* my_param = thisAgent->Decider->decider_params->get(pAttr->c_str());
        if (!my_param)
        {
            return SetError("Invalid command.  Use 'soar ?' to see a list of valid settings.");
        }

        if (!my_param->validate_string(pVal->c_str()))
        {
            return SetError("Invalid argument. Use 'soar ?' to see a list of valid settings.");
        }

        bool result = my_param->set_string(pVal->c_str());

        if (!result)
        {
            return SetError("That parameter could not be changed.");
        }
        else
        {
            tempStringStream << pAttr->c_str() << " is now " << pVal->c_str();
            PrintCLIMessage(&tempStringStream);
        }
        const char* lCmdName = pAttr->c_str();
        if (!strcmp(lCmdName, "o-support-mode") || !strcmp(lCmdName, "stop-phase"))
        {
            thisAgent->Decider->decider_params->update_enum_setting(thisAgent, my_param);
        } else if (
            !strcmp(lCmdName, "max-gp") ||
            !strcmp(lCmdName, "max-dc-time") ||
            !strcmp(lCmdName, "max-elaborations") ||
            !strcmp(lCmdName, "max-goal-depth") ||
            !strcmp(lCmdName, "max-memory-usage") ||
            !strcmp(lCmdName, "max-nil-output-cycles")) {
            thisAgent->Decider->decider_params->update_int_setting(thisAgent, static_cast<soar_module::integer_param*>(my_param));
        } else {
            thisAgent->Decider->decider_params->update_bool_setting(thisAgent, static_cast<soar_module::boolean_param*>(my_param));
        }
        return result;
    }

    return true;
}
