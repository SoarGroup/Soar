/////////////////////////////////////////////////////////////////
// wait-snc command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"
#include "sml_Names.h"

#include "IgSKI_Agent.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseWaitSNC(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	Options optionsData[] = {
		{'d', "disable",	0},
		{'e', "enable",		0},
		{'d', "off",		0},
		{'e', "on",			0},
		{0, 0, 0}
	};

	bool query = true;
	bool enable = false;

	for (;;) {
		if (!ProcessOptions(argv, optionsData)) return false;
		if (m_Option == -1) break;

		switch (m_Option) {
			case 'd':
				query = false;
				enable = false;
				break;
			case 'e':
				query = false;
				enable = true;
				break;
			default:
				return SetError(CLIError::kGetOptError);
		}
	}

	// No additional arguments
	if (m_NonOptionArguments) return SetError(CLIError::kTooManyArgs);		

	return DoWaitSNC(pAgent, query ? 0 : &enable);
}

bool CommandLineInterface::DoWaitSNC(gSKI::IAgent* pAgent, bool* pSetting) {
	if (!RequireAgent(pAgent)) return false;

	if (!pSetting) {
		if (m_RawOutput) {
			m_Result << "Current waitsnc setting: " << (pAgent->IsWaitingOnStateNoChange() ? "enabled" : "disabled");
		} else {
			AppendArgTagFast(sml_Names::kParamWaitSNC, sml_Names::kTypeBoolean, pAgent->IsWaitingOnStateNoChange() ? sml_Names::kTrue : sml_Names::kFalse);
		}
		return true;
	}

	pAgent->SetWaitOnStateNoChange(*pSetting);
	return true;
}
