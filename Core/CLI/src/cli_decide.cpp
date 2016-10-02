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

bool CommandLineInterface::DoDecide(std::vector<std::string>& argv, const std::string& pCmd)
{
    agent* thisAgent = m_pAgentSML->GetSoarAgent();

    if (pCmd.empty())
    {
        thisAgent->command_params->decide_params->print_summary(thisAgent);
        return true;
    }
    soar_module::param* my_param = thisAgent->command_params->decide_params->get(pCmd.c_str());
    if (!my_param)
    {
            return SetError("Invalid decide command.  Use 'decide ?' to see a list of valid settings.");
    }
    if (my_param == thisAgent->command_params->decide_params->indifferent_selection_cmd)
    {
        return ParseIndifferentSelection(argv);
    }
    else if (my_param == thisAgent->command_params->decide_params->numeric_indifferent_mode_cmd)
    {
        return ParseNumericIndifferentMode(argv);
    }
    else if (my_param == thisAgent->command_params->decide_params->predict_cmd)
    {
        return ParsePredict(argv);
    }
    else if (my_param == thisAgent->command_params->decide_params->select_cmd)
    {
        return ParseSelect(argv);
    }
    else if (my_param == thisAgent->command_params->decide_params->srand_cmd)
    {
        return ParseSRand(argv);
    }
    else if ((my_param == thisAgent->command_params->decide_params->help_cmd) || (my_param == thisAgent->command_params->decide_params->qhelp_cmd))
    {
        thisAgent->command_params->decide_params->print_settings(thisAgent);
    }
    return false;
}
bool CommandLineInterface::ParseIndifferentSelection(std::vector< std::string >& argv)
{
}
bool CommandLineInterface::ParseNumericIndifferentMode(std::vector< std::string >& argv)
{
}
bool CommandLineInterface::ParsePredict(std::vector< std::string >& argv)
{
}
bool CommandLineInterface::ParseSelect(std::vector< std::string >& argv)
{
}
bool CommandLineInterface::ParseSRand(std::vector< std::string >& argv)
{
}
