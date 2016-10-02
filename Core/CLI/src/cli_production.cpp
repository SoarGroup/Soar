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

bool CommandLineInterface::DoProduction(std::vector<std::string>& argv, const std::string& pCmd)
{
    agent* thisAgent = m_pAgentSML->GetSoarAgent();

    if (pCmd.empty())
    {
        thisAgent->command_params->production_params->print_summary(thisAgent);
        return true;
    }
    soar_module::param* my_param = thisAgent->command_params->production_params->get(pCmd.c_str());
    if (!my_param)
    {
            return SetError("Invalid production command.  Use 'production ?' to see a list of valid settings.");
    }
    if (my_param == thisAgent->command_params->production_params->excise_cmd)
    {
        return ParseExcise(argv);
    }
    else if (my_param == thisAgent->command_params->production_params->firing_counts_cmd)
    {
        return ParseFC(argv);
    }
    else if (my_param == thisAgent->command_params->production_params->matches_cmd)
    {
        return ParseMatches(argv);
    }
    else if (my_param == thisAgent->command_params->production_params->multi_attributes_cmd)
    {
        return ParseMultiAttributes(argv);
    }
    else if (my_param == thisAgent->command_params->production_params->break_cmd)
    {
        return ParsePBreak(argv);
    }
    else if (my_param == thisAgent->command_params->production_params->find_cmd)
    {
        return ParsePFind(argv);
    }
    else if (my_param == thisAgent->command_params->production_params->watch_cmd)
    {
        return ParsePWatch(argv);
    }
    else if ((my_param == thisAgent->command_params->production_params->help_cmd) || (my_param == thisAgent->command_params->production_params->qhelp_cmd))
    {
        thisAgent->command_params->production_params->print_settings(thisAgent);
    }
    return false;
}
bool CommandLineInterface::ParseExcise(std::vector< std::string >& argv)
{
}
bool CommandLineInterface::ParseFC(std::vector< std::string >& argv)
{
}
bool CommandLineInterface::ParseMatches(std::vector< std::string >& argv)
{
}
bool CommandLineInterface::ParseMultiAttributes(std::vector< std::string >& argv)
{
}
bool CommandLineInterface::ParsePBreak(std::vector< std::string >& argv)
{
}
bool CommandLineInterface::ParsePFind(std::vector< std::string >& argv)
{
}
bool CommandLineInterface::ParsePWatch(std::vector< std::string >& argv)
{
}
