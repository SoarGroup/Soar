#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_GetOpt.h"
#include "cli_Constants.h"

#include "sml_Names.h"

#include "IgSKI_Agent.h"
#include "IgSKI_DoNotTouch.h"
#include "IgSKI_Kernel.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseWarnings(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	static struct GetOpt::option longOptions[] = {
		{"enable",		0, 0, 'e'},
		{"disable",		0, 0, 'd'},
		{"on",			0, 0, 'e'},
		{"off",			0, 0, 'd'},
		{0, 0, 0, 0}
	};

	bool query = true;
	bool setting = true;

	for (;;) {
		int option = m_pGetOpt->GetOpt_Long(argv, "ed", longOptions, 0);
		if (option == -1) break;

		switch (option) {
			case 'e':
				setting = true;
				query = false;
				break;
			case 'd':
				setting = false;
				query = false;
				break;
			case '?':
				return SetError(CLIError::kUnrecognizedOption);
			default:
				return SetError(CLIError::kGetOptError);
		}
	}

	if (m_pGetOpt->GetAdditionalArgCount()) SetError(CLIError::kTooManyArgs);

	return DoWarnings(pAgent, query, setting);
}

bool CommandLineInterface::DoWarnings(gSKI::IAgent* pAgent, bool query, bool setting) {
	if (!RequireAgent(pAgent)) return false;

	// Attain the evil back door of doom, even though we aren't the TgD, because we'll probably need it
	gSKI::EvilBackDoor::ITgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();

	if (!query) {
		pKernelHack->SetSysparam(pAgent, PRINT_WARNINGS_SYSPARAM, setting);
		return true;
	}

	if (m_RawOutput) {
		m_ResultStream << "Printing of warnings is " << pKernelHack->GetSysparam(pAgent, PRINT_WARNINGS_SYSPARAM) ? "enabled." : "disabled.";
	} else {
		const char* setting = pKernelHack->GetSysparam(pAgent, PRINT_WARNINGS_SYSPARAM) ? sml_Names::kTrue : sml_Names::kFalse;
		AppendArgTagFast(sml_Names::kParamWarningsSetting, sml_Names::kTypeBoolean, setting);
	}
	return true;
}

