/////////////////////////////////////////////////////////////////
// verbose command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include <portability.h>

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"

#include "sml_Names.h"

#include "gSKI_Kernel.h"
#include "gSKI_DoNotTouch.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseVerbose(gSKI::Agent* pAgent, std::vector<std::string>& argv) {
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

	return DoVerbose(pAgent, query ? 0 : &setting);
}

bool CommandLineInterface::DoVerbose(gSKI::Agent* pAgent, bool* pSetting) {

	if (!RequireAgent(pAgent)) return false;

	// Attain the evil back door of doom, even though we aren't the TgD
	gSKI::EvilBackDoor::TgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();

	if (!pSetting) {
		if (m_RawOutput) {
			m_Result << "Verbose is " << (pKernelHack->GetVerbosity(pAgent) ? "on." : "off.");
		} else {
			AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeBoolean, pKernelHack->GetVerbosity(pAgent) ? sml_Names::kTrue : sml_Names::kFalse);
		}
		return true;
	}

	pKernelHack->SetVerbosity(pAgent, *pSetting);
	return true;
}

