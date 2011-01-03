/////////////////////////////////////////////////////////////////
// warnings command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "sml_Utils.h"
#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"

#include "sml_Names.h"

#include "sml_KernelSML.h"
#include "gsysparam.h"
#include "agent.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoWarnings(bool* pSetting) {
    agent* agnt = m_pAgentSML->GetSoarAgent();
	if (pSetting) {
		set_sysparam(agnt, PRINT_WARNINGS_SYSPARAM, *pSetting);
		return true;
	}

	if (m_RawOutput) {
		m_Result << "Printing of warnings is " << (agnt->sysparams[PRINT_WARNINGS_SYSPARAM] ? "enabled." : "disabled.");
	} else {
		const char* setting = agnt->sysparams[PRINT_WARNINGS_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse;
		AppendArgTagFast(sml_Names::kParamWarningsSetting, sml_Names::kTypeBoolean, setting);
	}
	return true;
}

