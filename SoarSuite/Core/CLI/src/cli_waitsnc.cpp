/////////////////////////////////////////////////////////////////
// wait-snc command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "cli_CommandLineInterface.h"
#include "cli_CLIError.h"

#include "cli_Commands.h"
#include "sml_Names.h"

#include "agent.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseWaitSNC(std::vector<std::string>& argv) {
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

	return DoWaitSNC(query ? 0 : &enable);
}

bool CommandLineInterface::DoWaitSNC(bool* pSetting) {
	if (!pSetting) {
		if (m_RawOutput) {
			m_Result << "Current waitsnc setting: " << (m_pAgentSoar->waitsnc ? "enabled" : "disabled");
		} else {
			AppendArgTagFast(sml_Names::kParamWaitSNC, sml_Names::kTypeBoolean, m_pAgentSoar->waitsnc ? sml_Names::kTrue : sml_Names::kFalse);
		}
		return true;
	}

	m_pAgentSoar->waitsnc = *pSetting;
	return true;
}
