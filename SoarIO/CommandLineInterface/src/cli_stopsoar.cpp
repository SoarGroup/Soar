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

	bool self = false;

	for (;;) {
		int option = m_pGetOpt->GetOpt_Long(argv, "s", longOptions, 0);
		if (option == -1) break;

		switch (option) {
			case 's':
				self = true;
				break;
			case '?':
				return SetError(CLIError::kUnrecognizedOption);
			default:
				return SetError(CLIError::kGetOptError);
		}
	}

	// Concatinate remaining args for 'reason'
	if (m_pGetOpt->GetAdditionalArgCount()) {
		std::string reasonForStopping;
		unsigned int optind = m_pGetOpt->GetOptind();
		while (optind < argv.size()) reasonForStopping += argv[optind++] + ' ';
		return DoStopSoar(pAgent, self, &reasonForStopping);
	}
	return DoStopSoar(pAgent, self);
}

/*************************************************************
* @brief stop-soar command
* @param pAgent The pointer to the gSKI agent interface
* @param self Stop the only pAgent (false means stop all agents in kernel)
* @param reasonForStopping optional reason for stopping
*************************************************************/
EXPORT bool CommandLineInterface::DoStopSoar(gSKI::IAgent* pAgent, bool self, const std::string* reasonForStopping) {

	unused(reasonForStopping);

	if (self) {
		if (!RequireAgent(pAgent)) return false;
		if (!pAgent->Interrupt(gSKI_STOP_AFTER_SMALLEST_STEP, gSKI_STOP_BY_RETURNING, &m_gSKIError)) return SetError(CLIError::kgSKIError);
		if (gSKI::isError(m_gSKIError)) return SetError(CLIError::kgSKIError);
		return true;
	} else {
		if (!RequireKernel()) return false;
		if (!m_pKernel->GetAgentManager()->InterruptAll(gSKI_STOP_AFTER_SMALLEST_STEP, &m_gSKIError)) return SetError(CLIError::kgSKIError);
		if (gSKI::isError(m_gSKIError)) return SetError(CLIError::kgSKIError);
		return true;
	}
}

