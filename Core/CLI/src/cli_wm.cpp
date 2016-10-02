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

bool CommandLineInterface::DoWM(std::vector<std::string>& argv, const std::string& pCmd)
{
    agent* thisAgent = m_pAgentSML->GetSoarAgent();

    if (pCmd.empty())
    {
        thisAgent->command_params->wm_params->print_summary(thisAgent);
        return true;
    }
    soar_module::param* my_param = thisAgent->command_params->wm_params->get(pCmd.c_str());
    if (!my_param)
    {
            return SetError("Invalid wm command.  Use 'wm ?' to see a list of valid settings.");
    }
    if (my_param == thisAgent->command_params->wm_params->add_cmd)
    {
        return ParseWMEAdd(argv);
    }
    else if (my_param == thisAgent->command_params->wm_params->remove_cmd)
    {
        return ParseWMERemove(argv);
    }
    else if (my_param == thisAgent->command_params->wm_params->watch_cmd)
    {
        return ParseWMEWatch(argv);
    }
    else if (my_param == thisAgent->command_params->wm_params->wma_cmd)
    {
        return ParseWMA(argv);
    }
    else if ((my_param == thisAgent->command_params->wm_params->help_cmd) || (my_param == thisAgent->command_params->wm_params->qhelp_cmd))
    {
        thisAgent->command_params->wm_params->print_settings(thisAgent);
    }
    return false;
}
bool CommandLineInterface::ParseWMEAdd(std::vector< std::string >& argv)
{
}
bool CommandLineInterface::ParseWMERemove(std::vector< std::string >& argv)
{
}
bool CommandLineInterface::ParseWMEWatch(std::vector< std::string >& argv)
{
}
bool CommandLineInterface::ParseWMA(std::vector< std::string >& argv)
{
}
