#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_GetOpt.h"
#include "cli_Constants.h"

#include "sml_Names.h"

#include "IgSKI_Agent.h"
#include "IgSKI_Kernel.h"
#include "IgSKI_DoNotTouch.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseVerbose(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	static struct GetOpt::option longOptions[] = {
		{"disable",	0, 0, 'd'},
		{"enable",	0, 0, 'e'},
		{"off",		0, 0, 'd'},
		{"onn",		0, 0, 'e'},
		{0, 0, 0, 0}
	};

	bool setting = false;
	bool query = true;

	for (;;) {
		int option = m_pGetOpt->GetOpt_Long(argv, "de", longOptions, 0);
		if (option == -1) break;

		switch (option) {
			case 'd':
				setting = false;
				query = false;
				break;
			case 'e':
				setting = true;
				query = false;
				break;
			case '?':
				return m_Error.SetError(CLIError::kUnrecognizedOption);
			default:
				return m_Error.SetError(CLIError::kGetOptError);
		}
	}

	if (m_pGetOpt->GetAdditionalArgCount()) return m_Error.SetError(CLIError::kTooManyArgs);

	return DoVerbose(pAgent, query, setting);
}

bool CommandLineInterface::DoVerbose(gSKI::IAgent* pAgent, bool query, bool setting) {

	if (!RequireAgent(pAgent)) return false;

	// Attain the evil back door of doom, even though we aren't the TgD
	gSKI::EvilBackDoor::ITgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();

	if (query) {
		if (m_RawOutput) {
			AppendToResult("Verbose is ");
			AppendToResult(pKernelHack->GetVerbosity(pAgent) ? "on." : "off.");
		} else {
			AppendArgTag(sml_Names::kParamValue, sml_Names::kTypeBoolean, pKernelHack->GetVerbosity(pAgent) ? sml_Names::kTrue : sml_Names::kFalse);
		}
		return true;
	}

	pKernelHack->SetVerbosity(pAgent, setting);
	return true;
}

