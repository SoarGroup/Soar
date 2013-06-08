#include "cli_CommandLineInterface.h"
#include "sml_AgentSML.h"
#include "agent.h"
#include "svs_interface.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoSVS(const std::vector<std::string> &args) {
    std::string out;
    bool res = m_pAgentSML->GetSoarAgent()->svs->do_cli_command(args, out);
    if ( m_RawOutput )
    {
        m_Result << out;
    }
    else
    {
        AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeString, out.c_str() );
    }
    return res;
}
