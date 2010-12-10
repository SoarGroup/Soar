/////////////////////////////////////////////////////////////////
// verbose command file.
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
#include "agent.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseVerbose(std::vector<std::string>& argv) {
	Options optionsData[] = {
		{'d', "disable",	OPTARG_NONE},
		{'e', "enable",		OPTARG_NONE},
		{'d', "off",		OPTARG_NONE},
		{'e', "onn",		OPTARG_NONE},
		{0, 0, OPTARG_NONE}
	};

	bool setting = false;
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
				return SetError(kGetOptError);
		}
	}

	if (m_NonOptionArguments) return SetError(kTooManyArgs);

	return DoVerbose(query ? 0 : &setting);
}

bool CommandLineInterface::DoVerbose(bool* pSetting) {
	if (!pSetting) {
		if (m_RawOutput) {
			m_Result << "Verbose is " << (m_pAgentSoar->soar_verbose_flag ? "on." : "off.");
		} else {
			AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeBoolean, m_pAgentSoar->soar_verbose_flag ? sml_Names::kTrue : sml_Names::kFalse);
		}
		return true;
	}

	m_pAgentSoar->soar_verbose_flag = *pSetting;
	return true;
}

