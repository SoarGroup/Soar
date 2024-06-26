#include "cli_CommandLineInterface.h"
#include "sml_AgentSML.h"
#include "agent.h"
#ifndef NO_SVS
#include "svs_interface.h"
#include "symbol.h"
#endif

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoSVS(const std::vector<std::string>& args)
{
#ifndef NO_SVS
    std::string out;
    agent* thisAgent = m_pAgentSML->GetSoarAgent();

    if (args.size() == 1)
    {
        m_Result << "Spatial Visual System is " << (thisAgent->svs->is_enabled() ? "enabled." : "disabled.");
        return true;
    }
    else if (args.size() == 2)
    {
        if ((args[1] == "--enable") || (args[1] == "-e") || (args[1] == "--on"))
        {
            if (thisAgent->svs->is_enabled())
            {
                m_Result << "Spatial Visual System is already enabled. ";
            }
            else
            {
                thisAgent->svs->set_enabled(true);
                for (Symbol* lState = thisAgent->top_goal; lState; lState = lState->id->lower_goal)
                {
                    thisAgent->svs->state_creation_callback(lState);
                }
                m_Result << "Spatial Visual System enabled. ";
            }
            return true;
        }
        else if ((args[1] == "--disable") || (args[1] == "-d") || (args[1] == "--off"))
        {
            if (!thisAgent->svs->is_enabled())
            {
                m_Result << "Spatial Visual System is already disabled. ";
            }
            else
            {
                if (thisAgent->svs->is_in_substate()) {
                    m_Result << "Cannot disable Spatial Visual System while in a substate. ";
                    return false;
                }
                thisAgent->svs->set_enabled(false);
                m_Result << "Spatial Visual System disabled. ";
            }
            return true;
        }
        else if(args[1] == "--enable-in-substates")
        {
            if (thisAgent->svs->is_enabled_in_substates())
            {
                m_Result << "Spatial Visual System is already enabled in substates. ";
            }
            else
            {
                thisAgent->svs->set_enabled_in_substates(true);
                m_Result << "Spatial Visual System enabled in substates. ";
            }
        }
        else if(args[1] == "--disable-in-substates")
        {
            if (!thisAgent->svs->is_enabled_in_substates())
            {
                m_Result << "Spatial Visual System is already disabled in substates. ";
            }
            else
            {
                if (thisAgent->svs->is_in_substate()) {
                    m_Result << "Cannot disable Spatial Visual System in substates while in a substate. ";
                    return false;
                }
                thisAgent->svs->set_enabled_in_substates(false);
                m_Result << "Spatial Visual System disabled in substates. ";
            }
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
    }
    else
    {
#endif
        m_Result << "Spatial Visual System is currently disabled.  Please enable to execute SVS commands.";
        return false;
#ifndef NO_SVS
    }
#endif
}
