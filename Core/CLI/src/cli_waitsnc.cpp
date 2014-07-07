/////////////////////////////////////////////////////////////////
// wait-snc command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
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

bool CommandLineInterface::DoWaitSNC(bool* pSetting) {
    agent* thisAgent = m_pAgentSML->GetSoarAgent();
    if (!pSetting) {
        if (m_RawOutput) {
            m_Result << "Current waitsnc setting: " << (thisAgent->waitsnc ? "enabled" : "disabled");
        } else {
            AppendArgTagFast(sml_Names::kParamWaitSNC, sml_Names::kTypeBoolean, thisAgent->waitsnc ? sml_Names::kTrue : sml_Names::kFalse);
        }
        return true;
    }

    thisAgent->waitsnc = *pSetting;
    return true;
}
