#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_GetOpt.h"
#include "cli_Constants.h"

#include "IgSKI_Agent.h"
#include "IgSKI_AgentManager.h"
#include "IgSKI_Kernel.h"

using namespace cli;

bool CommandLineInterface::ParseStopSoar(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	static struct GetOpt::option longOptions[] = {
		{"self",		0, 0, 's'},
		{0, 0, 0, 0}
	};

	GetOpt::optind = 0;
	GetOpt::opterr = 0;

	bool self = false;

	for (;;) {
		int option = m_pGetOpt->GetOpt_Long(argv, "s", longOptions, 0);
		if (option == -1) break;

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

	unused(reasonForStopping);

	if (self) {
		if (!RequireAgent(pAgent)) return false;
		return pAgent->Interrupt(gSKI_STOP_AFTER_SMALLEST_STEP, gSKI_STOP_BY_RETURNING, m_pError);
	} else {
		return m_pKernel->GetAgentManager()->InterruptAll(gSKI_STOP_AFTER_SMALLEST_STEP, m_pError);
	}
}

