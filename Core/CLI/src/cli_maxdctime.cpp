/////////////////////////////////////////////////////////////////
// max-dc-time command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2011
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"
#include "sml_Names.h"
#include "sml_AgentSML.h"

#include "agent.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoMaxDCTime(const int n) {
    agent* agnt = m_pAgentSML->GetSoarAgent();
    if (!n) {
        // query
        if (m_RawOutput) {
            if (agnt->sysparams[DECISION_CYCLE_MAX_USEC_INTERRUPT] > 0)
                m_Result << agnt->sysparams[DECISION_CYCLE_MAX_USEC_INTERRUPT] << " microseconds";
            else
                m_Result << "Decision cycle time interrupt disabled.";
        } else {
            std::string temp;
            AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeInt, to_string(agnt->sysparams[DECISION_CYCLE_MAX_USEC_INTERRUPT], temp));
        }
        return true;
    }

    agnt->sysparams[DECISION_CYCLE_MAX_USEC_INTERRUPT] = n < 0 ? -1 : n;
    return true;
}

