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
#include "ebc_settings.h"
#include "output_manager.h"
#include "output_settings.h"
#include "print.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoOutput(const char pOp, const std::string* pAttr, const std::string* pVal)
{
    agent* thisAgent = m_pAgentSML->GetSoarAgent();
    std::ostringstream tempStringStream;
    std::string tempString;

    if (!pOp)
    {
        thisAgent->outputManager->m_params->print_output_settings(thisAgent);
        return true;
    }
    else if (pOp == 'G')
    {
        /* Single command argument */
        soar_module::param* my_param = thisAgent->outputManager->m_params->get(pAttr->c_str());
        if (!my_param)
        {
            return SetError("Invalid output sub-command.  Use 'soar ?' to see a list of valid sub-commands and settings.");
        }

        else if ((my_param == thisAgent->outputManager->m_params->help_cmd) || (my_param == thisAgent->outputManager->m_params->qhelp_cmd))
        {
            thisAgent->outputManager->m_params->print_output_settings(thisAgent);
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
        soar_module::param* my_param = thisAgent->outputManager->m_params->get(pAttr->c_str());
        if (!my_param)
        {
            return SetError("Invalid output command.  Use 'output ?' to see a list of valid sub-commands.");
        }

        if (!my_param->validate_string(pVal->c_str()))
        {
            return SetError("Invalid argument for output command. Use 'output ?' to see a list of valid sub-commands.");
        }

        bool result = my_param->set_string(pVal->c_str());

        if (!result)
        {
            return SetError("The output parameter could not be changed.");
        }
        else
        {
            tempStringStream << pAttr->c_str() << " = " << pVal->c_str();
            PrintCLIMessage(&tempStringStream);
        }
        /* The following code assumes that all parameters except learn are boolean */
        if (!strcmp(pAttr->c_str(), "print-depth"))
        {
            thisAgent->outputManager->m_params->update_int_setting(thisAgent, static_cast<soar_module::integer_param*>(my_param));
        } else {
            thisAgent->outputManager->m_params->update_bool_setting(thisAgent, static_cast<soar_module::boolean_param*>(my_param), m_pKernelSML);
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
