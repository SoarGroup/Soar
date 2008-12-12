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
#include "sml_StringOps.h"

#include "sml_KernelSML.h"
#include "sml_KernelHelpers.h"
#include "gsysparam.h"

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
	// Attain the evil back door of doom, even though we aren't the TgD
	sml::KernelHelpers* pKernelHack = m_pKernelSML->GetKernelHelpers() ;

	if (!pSetting) {
		if (m_RawOutput) {
			m_Result << "Save bactraces is " << (pKernelHack->GetSysparam(m_pAgentSML, EXPLAIN_SYSPARAM) ? "enabled." : "disabled.");
		} else {
			AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeBoolean, pKernelHack->GetSysparam(m_pAgentSML, EXPLAIN_SYSPARAM) ? sml_Names::kTrue : sml_Names::kFalse);
		}
		return true;
	}

	pKernelHack->SetSysparam(m_pAgentSML, EXPLAIN_SYSPARAM, *pSetting);
	return true;
}

