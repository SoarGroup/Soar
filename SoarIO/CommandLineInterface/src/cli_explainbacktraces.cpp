#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Constants.h"

#include "cli_GetOpt.h"

#include "sml_Names.h"
#include "sml_StringOps.h"

#include "IgSKI_Agent.h"
#include "IgSKI_Kernel.h"
#include "IgSKI_DoNotTouch.h"
#include "IgSKI_ProductionManager.h"
#include "IgSKI_Production.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseExplainBacktraces(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	static struct GetOpt::option longOptions[] = {
		{"condition",	1, 0, 'c'},
		{"full",		0, 0, 'f'},
		{0, 0, 0, 0}
	};

	bool full = false;
	int condition = 0;

	for (;;) {
		int option = m_pGetOpt->GetOpt_Long(argv, ":c:f", longOptions, 0);
		if (option == -1) break;

		switch (option) {
			case 'f':
				condition = 0;
				full = true;
				break;

			case 'c':
				full = false;
				if (!IsInteger(m_pGetOpt->GetOptArg())) return SetError(CLIError::kIntegerExpected);
				condition = atoi(m_pGetOpt->GetOptArg());
				if (condition <= 0) return SetError(CLIError::kIntegerMustBePositive);
				break;

			case ':':
				return SetError(CLIError::kMissingOptionArg);
			case '?':
				return SetError(CLIError::kUnrecognizedOption);
			default:
				return SetError(CLIError::kGetOptError);
		}
	}

	if (m_pGetOpt->GetAdditionalArgCount() > 1) return SetError(CLIError::kTooManyArgs);

	if (full || condition) {
		if (m_pGetOpt->GetAdditionalArgCount() < 1) return SetError(CLIError::kTooFewArgs);
	}

	if (m_pGetOpt->GetAdditionalArgCount() == 1) return DoExplainBacktraces(pAgent, &argv[m_pGetOpt->GetOptind()], full, condition);;
	
	return DoExplainBacktraces(pAgent);
}

bool CommandLineInterface::DoExplainBacktraces(gSKI::IAgent* pAgent, std::string* pProduction, bool full, int condition) {

	if (!RequireAgent(pAgent)) return false;

	if (condition < 0) return SetError(CLIError::kInvalidConditionNumber);

	// Attain the evil back door of doom, even though we aren't the TgD
	gSKI::EvilBackDoor::ITgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();

	if (!full && !condition && (!pProduction || !pProduction->size())) {
		// query
		AddListenerAndDisableCallbacks(pAgent);
		pKernelHack->ExplainListChunks(pAgent);
		RemoveListenerAndEnableCallbacks(pAgent);
		return true;
	}

	if (!pProduction || !pProduction->size()) return SetError(CLIError::kProductionRequired);
	if (full) condition = -1;

	AddListenerAndDisableCallbacks(pAgent);
	pKernelHack->ExplainChunks(pAgent, pProduction->c_str(), condition);
	RemoveListenerAndEnableCallbacks(pAgent);

	return true;
}

