/////////////////////////////////////////////////////////////////
// max-memory-usage command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2006
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"
#include "sml_Names.h"

#include "agent.h"
#include "sml_AgentSML.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoMaxMemoryUsage(const int n) {
    agent* agnt = m_pAgentSML->GetSoarAgent();
    if (!n) {
        // query
        if (m_RawOutput) {
            m_Result << agnt->sysparams[MAX_MEMORY_USAGE_SYSPARAM] << " bytes";
        } else {
            std::string temp;
            AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeInt, to_string(agnt->sysparams[MAX_CHUNKS_SYSPARAM], temp));
        }
        return true;
    }

    agnt->sysparams[MAX_MEMORY_USAGE_SYSPARAM] = n;
    return true;
}

