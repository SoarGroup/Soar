#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_GetOpt.h"
#include "cli_Constants.h"

#include "IgSKI_Agent.h"

using namespace cli;

bool CommandLineInterface::ParseStopSoar(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	static struct GetOpt::option longOptions[] = {
		{"self",		0, 0, 's'},
		{0, 0, 0, 0}
	};

	GetOpt::optind = 0;
	GetOpt::opterr = 0;

	int option;
	bool self = false;

	for (;;) {
		option = m_pGetOpt->GetOpt_Long(argv, "s", longOptions, 0);
		if (option == -1) {
			break;
		}

		switch (option) {
			case 's':
				self = true;
				break;
			case '?':
				return HandleSyntaxError(Constants::kCLIStopSoar, Constants::kCLIUnrecognizedOption);
			default:
				return HandleGetOptError((char)option);
		}
	}

	// Concatinate remaining args for 'reason'
	std::string reasonForStopping;
	if ((unsigned)GetOpt::optind < argv.size()) {
		while ((unsigned)GetOpt::optind < argv.size()) {
			reasonForStopping += argv[GetOpt::optind++] + ' ';
		}
	}
	return DoStopSoar(pAgent, self, reasonForStopping);
}

bool CommandLineInterface::DoStopSoar(gSKI::IAgent* pAgent, bool self, const std::string& reasonForStopping) {

	unused(pAgent);
	unused(self);
	unused(reasonForStopping);

	if (!m_pRunForever) {
		return HandleError("Not running!");
	}

	// Stop and reset the current thread
	m_pRunForever->Stop(true);
	delete m_pRunForever;
	m_pRunForever = 0;
	return true;
}

