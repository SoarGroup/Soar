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
#include "output_manager.h"
#include "print.h"
#include "production.h"
#include "symbol.h"
#include "symbol_manager.h"
#include "rete.h"

#include <algorithm>
#include "../../SoarKernel/src/settings/cmd_settings.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoLoad(std::vector<std::string>& argv, const std::string& pCmd)
{
    agent* thisAgent = m_pAgentSML->GetSoarAgent();

    if (pCmd.empty())
    {
        thisAgent->command_params->load_params->print_summary(thisAgent);
        return true;
    }
    soar_module::param* my_param = thisAgent->command_params->load_params->get(pCmd.c_str());
    if (!my_param)
    {
            return SetError("Invalid load command.  Use 'load ?' to see a list of valid settings.");
    }
    if (my_param == thisAgent->command_params->load_params->input_cmd)
    {
        return ParseReplayInput(argv);
    }
    else if (my_param == thisAgent->command_params->load_params->file_cmd)
    {
        return ParseSource(argv);
    }
    else if (my_param == thisAgent->command_params->load_params->rete_cmd)
    {
        return ParseReteLoad(argv);
    }
    else if (my_param == thisAgent->command_params->load_params->library_cmd)
    {
        return ParseLoadLibrary(argv);
    }
    else if ((my_param == thisAgent->command_params->load_params->help_cmd) || (my_param == thisAgent->command_params->load_params->qhelp_cmd))
    {
        thisAgent->command_params->load_params->print_settings(thisAgent);
    }
    return false;
}

bool CommandLineInterface::DoSave(std::vector<std::string>& argv, const std::string& pCmd)
{
    agent* thisAgent = m_pAgentSML->GetSoarAgent();

    if (pCmd.empty())
    {
        thisAgent->command_params->save_params->print_summary(thisAgent);
        return true;
    }
    soar_module::param* my_param = thisAgent->command_params->save_params->get(pCmd.c_str());
    if (!my_param)
    {
            return SetError("Invalid save command.  Use 'save ?' to see a list of valid settings.");
    }
    if (my_param == thisAgent->command_params->save_params->input_cmd)
    {
        return ParseCaptureInput(argv);
    }
    else if (my_param == thisAgent->command_params->save_params->rete_cmd)
    {
        return ParseReteSave(argv);
    }
    else if ((my_param == thisAgent->command_params->save_params->help_cmd) || (my_param == thisAgent->command_params->save_params->qhelp_cmd))
    {
        thisAgent->command_params->save_params->print_settings(thisAgent);
    }
    return false;
}
bool CommandLineInterface::ParseReplayInput(std::vector< std::string >& argv)
{
}
bool CommandLineInterface::ParseSource(std::vector< std::string >& argv)
{
}
bool CommandLineInterface::ParseReteLoad(std::vector< std::string >& argv)
{
}
bool CommandLineInterface::ParseLoadLibrary(std::vector< std::string >& argv)
{
}
bool CommandLineInterface::ParseCaptureInput(std::vector< std::string >& argv)
{
}
bool CommandLineInterface::ParseReteSave(std::vector< std::string >& argv)
{
}
