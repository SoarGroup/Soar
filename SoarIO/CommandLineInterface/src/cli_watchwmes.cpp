#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_GetOpt.h"
#include "cli_Constants.h"

#include "sml_StringOps.h"
#include "sml_Names.h"

#include "IgSKI_Agent.h"
#include "IgSKI_Kernel.h"
#include "IgSKI_DoNotTouch.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseWatchWMEs(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	static struct GetOpt::option longOptions[] = {
		{"add-filter",		0, 0, 'a'},
		{"remove-filter",	0, 0, 'r'},
		{"list-filter",		0, 0, 'l'},
		{"reset-filter",	0, 0, 'R'},
		{"type",			1, 0, 't'},
		{0, 0, 0, 0}
	};

	for (;;) {
		int option = m_pGetOpt->GetOpt_Long(argv, ":arlRt:", longOptions, 0);
		if (option == -1) break;

		switch (option) {
			case 'a':
				break;
			case 'r':
				break;
			case 'l':
				break;
			case 'R':
				break;
			case 't':
				break;
			case '?':
				return m_Error.SetError(CLIError::kUnrecognizedOption);
			default:
				return m_Error.SetError(CLIError::kGetOptError);
		}
	}

	return DoWatchWMEs(pAgent);
}

bool CommandLineInterface::DoWatchWMEs(gSKI::IAgent* pAgent) {
	return false;
}
