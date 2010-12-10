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

bool CommandLineInterface::ParseWarnings(std::vector<std::string>& argv) {
	Options optionsData[] = {
		{'e', "enable",		OPTARG_NONE},
		{'d', "disable",	OPTARG_NONE},
		{'e', "on",			OPTARG_NONE},
		{'d', "off",		OPTARG_NONE},
		{0, 0, OPTARG_NONE}
	};

	bool query = true;
	bool setting = true;

	for (;;) {
		if (!ProcessOptions(argv, optionsData)) return false;
		if (m_Option == -1) break;

		switch (m_Option) {
			case 'e':
				setting = true;
				query = false;
				break;
			case 'd':
				setting = false;
				query = false;
				break;
			default:
				return SetError(kGetOptError);
		}
	}

	if (m_NonOptionArguments) SetError(kTooManyArgs);

	return DoWarnings(query ? 0 : &setting);
}

bool CommandLineInterface::DoWarnings(bool* pSetting) {
	if (pSetting) {
		set_sysparam(m_pAgentSoar, PRINT_WARNINGS_SYSPARAM, *pSetting);
		return true;
	}

	if (m_RawOutput) {
		m_Result << "Printing of warnings is " << (m_pAgentSoar->sysparams[PRINT_WARNINGS_SYSPARAM] ? "enabled." : "disabled.");
	} else {
		const char* setting = m_pAgentSoar->sysparams[PRINT_WARNINGS_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse;
		AppendArgTagFast(sml_Names::kParamWarningsSetting, sml_Names::kTypeBoolean, setting);
	}
	return true;
}

