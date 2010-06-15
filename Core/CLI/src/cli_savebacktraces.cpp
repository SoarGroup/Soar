/////////////////////////////////////////////////////////////////
// save-backtraces command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "sml_Utils.h"
#include "cli_CommandLineInterface.h"
#include "cli_CLIError.h"

#include "sml_Names.h"

#include "sml_KernelSML.h"
#include "gsysparam.h"
#include "agent.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseSaveBacktraces(std::vector<std::string>& argv) {
	Options optionsData[] = {
		{'d', "disable",	OPTARG_NONE},
		{'e', "enable",		OPTARG_NONE},
		{'d', "off",		OPTARG_NONE},
		{'e', "on",			OPTARG_NONE},
		{0, 0, OPTARG_NONE}
	};

	bool setting = true;
	bool query = true;

	for (;;) {
		if (!ProcessOptions(argv, optionsData)) return false;
		if (m_Option == -1) break;

		switch (m_Option) {
			case 'd':
				setting = false;
				query = false;
				break;
			case 'e':
				setting = true;
				query = false;
				break;
			default:
				return SetError(CLIError::kGetOptError);
		}
	}
	if (m_NonOptionArguments) return SetError(CLIError::kTooManyArgs);
	return DoSaveBacktraces(query ? 0 : &setting);
}

bool CommandLineInterface::DoSaveBacktraces(bool* pSetting) {
	if (!pSetting) {
		if (m_RawOutput) {
			m_Result << "Save bactraces is " << (m_pAgentSoar->sysparams[EXPLAIN_SYSPARAM] ? "enabled." : "disabled.");
		} else {
			AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeBoolean, m_pAgentSoar->sysparams[EXPLAIN_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);
		}
		return true;
	}

	set_sysparam(m_pAgentSoar, EXPLAIN_SYSPARAM, *pSetting);
	return true;
}

