#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Constants.h"
#include "cli_GetOpt.h"

#include "IgSKI_Agent.h"

using namespace cli;

bool CommandLineInterface::ParseWaitSNC(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	static struct GetOpt::option longOptions[] = {
		{"disable",	0, 0, 'd'},
		{"enable",	0, 0, 'e'},
		{"off",		0, 0, 'd'},
		{"on",		0, 0, 'e'},
		{0, 0, 0, 0}
	};

	GetOpt::optind = 0;
	GetOpt::opterr = 0;

	bool query = true;
	bool enable = false;

	for (;;) {
		int option = m_pGetOpt->GetOpt_Long(argv, "de", longOptions, 0);
		if (option == -1) break;

		switch (option) {
			case 'd':
				query = false;
				enable = false;
				break;
			case 'e':
				query = false;
				enable = true;
				break;
			case '?':
				return HandleSyntaxError(Constants::kCLIWaitSNC, Constants::kCLIUnrecognizedOption);
			default:
				return HandleGetOptError((char)option);
		}
	}

	// No additional arguments
	if (argv.size() != (unsigned)GetOpt::optind) return HandleSyntaxError(Constants::kCLIWaitSNC, Constants::kCLITooManyArgs);		

	return DoWaitSNC(pAgent, query, enable);
}

bool CommandLineInterface::DoWaitSNC(gSKI::IAgent* pAgent, bool query, bool enable) {
	if (!RequireAgent(pAgent)) return false;

	if (query) {
		AppendToResult("Current waitsnc setting: ");
		AppendToResult(pAgent->IsWaitingOnStateNoChange() ? "enabled" : "disabled");
		return true;
	}

	pAgent->SetWaitOnStateNoChange(enable);
	return true;
}
