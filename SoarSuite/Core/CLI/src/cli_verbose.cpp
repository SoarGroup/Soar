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
#include "cli_CLIError.h"

#include "sml_KernelSML.h"
#include "sml_KernelHelpers.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseVerbose(std::vector<std::string>& argv) {
	Options optionsData[] = {
		{'d', "disable",	0},
		{'e', "enable",		0},
		{'d', "off",		0},
		{'e', "onn",		0},
		{0, 0, 0}
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
				return SetError(CLIError::kGetOptError);
		}
	}

	if (m_NonOptionArguments) return SetError(CLIError::kTooManyArgs);

	return DoVerbose(query ? 0 : &setting);
}

bool CommandLineInterface::DoVerbose(bool* pSetting) {
	// Attain the evil back door of doom, even though we aren't the TgD
	sml::KernelHelpers* pKernelHack = m_pKernelSML->GetKernelHelpers() ;

	if (!pSetting) {
		if (m_RawOutput) {
			m_Result << "Verbose is " << (pKernelHack->GetVerbosity(m_pAgentSML) ? "on." : "off.");
		} else {
			AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeBoolean, pKernelHack->GetVerbosity(m_pAgentSML) ? sml_Names::kTrue : sml_Names::kFalse);
		}
		return true;
	}

	pKernelHack->SetVerbosity(m_pAgentSML, *pSetting);
	return true;
}

