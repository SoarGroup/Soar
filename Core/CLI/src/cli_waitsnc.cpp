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

#include "agent.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoWaitSNC(bool* pSetting) {
    agent* agnt = m_pAgentSML->GetSoarAgent();
	if (!pSetting) {
		if (m_RawOutput) {
			m_Result << "Current waitsnc setting: " << (agnt->waitsnc ? "enabled" : "disabled");
		} else {
			AppendArgTagFast(sml_Names::kParamWaitSNC, sml_Names::kTypeBoolean, agnt->waitsnc ? sml_Names::kTrue : sml_Names::kFalse);
		}
		return true;
	}

	agnt->waitsnc = *pSetting;
	return true;
}
