/////////////////////////////////////////////////////////////////
// warnings command file.
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

#include "gSKI_Kernel.h"
#include "gSKI_DoNotTouch.h"
#include "gsysparam.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseWarnings(gSKI::Agent* pAgent, std::vector<std::string>& argv) {
	Options optionsData[] = {
		{'e', "enable",		0},
		{'d', "disable",	0},
		{'e', "on",			0},
		{'d', "off",		0},
		{0, 0, 0}
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
				return SetError(CLIError::kGetOptError);
		}
	}

	if (m_NonOptionArguments) SetError(CLIError::kTooManyArgs);

	return DoWarnings(pAgent, query ? 0 : &setting);
}

bool CommandLineInterface::DoWarnings(gSKI::Agent* pAgent, bool* pSetting) {
	if (!RequireAgent(pAgent)) return false;

	// Attain the evil back door of doom, even though we aren't the TgD, because we'll probably need it
	gSKI::EvilBackDoor::TgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();

	if (pSetting) {
		pKernelHack->SetSysparam(pAgent, PRINT_WARNINGS_SYSPARAM, *pSetting);
		return true;
	}

	if (m_RawOutput) {
		m_Result << "Printing of warnings is " << (pKernelHack->GetSysparam(pAgent, PRINT_WARNINGS_SYSPARAM) ? "enabled." : "disabled.");
	} else {
		const char* setting = pKernelHack->GetSysparam(pAgent, PRINT_WARNINGS_SYSPARAM) ? sml_Names::kTrue : sml_Names::kFalse;
		AppendArgTagFast(sml_Names::kParamWarningsSetting, sml_Names::kTypeBoolean, setting);
	}
	return true;
}

