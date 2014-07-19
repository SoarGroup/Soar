#include "cli_CommandLineInterface.h"
#include "sml_AgentSML.h"
#include "agent.h"
#include "svs_interface.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoSVS(const std::vector<std::string>& args)
{
    std::string out;
    agent* thisAgent = m_pAgentSML->GetSoarAgent();

    if (args.size() == 1)
    {
        m_Result << "Soar Visual System is " << (thisAgent->svs->is_enabled() ? "enabled." : "disabled.");
        return true;
    }
    else if (args.size() == 2)
    {
        if ((args[1] == "--enable") || (args[1] == "-e") || (args[1] == "--on"))
        {
            if (thisAgent->svs->is_enabled())
            {
                m_Result << "Soar Visual System is already enabled.";
            } else {
                thisAgent->svs->set_enabled(true);
                m_Result << "Soar Visual System enabled.";
            }
            return true;
        }
        else if ((args[1] == "--disable") || (args[1] == "-d") || (args[1] == "--off"))
        {
            if (!thisAgent->svs->is_enabled())
            {
                m_Result << "Soar Visual System is already disabled.";
            } else {
                thisAgent->svs->set_enabled(false);
                m_Result << "Soar Visual System disabled.";
            }
            return true;
        }
    }
    if (thisAgent->svs->is_enabled())
    {
        bool res = thisAgent->svs->do_cli_command(args, out);
        if (m_RawOutput)
        {
            m_Result << out;
        }
        else
        {
            AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeString, out.c_str());
        }
        return res;
    } else {
        m_Result << "Soar Visual System is currently disabled.  Please enable to execute SVS commands.";
        return false;
    }
}
