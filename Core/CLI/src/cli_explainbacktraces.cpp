/////////////////////////////////////////////////////////////////
// explain-backtraces command file.
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
#include "sml_StringOps.h"

#include "gSKI_Kernel.h"
#include "gSKI_ProductionManager.h"
#include "IgSKI_Production.h"
#include "gSKI_DoNotTouch.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseExplainBacktraces(gSKI::Agent* pAgent, std::vector<std::string>& argv) {
	Options optionsData[] = {
		{'c', "condition",	1},
		{'f', "full",		0},
		{0, 0, 0}
	};

	int condition = 0;

	for (;;) {
		if (!ProcessOptions(argv, optionsData)) return false;
		if (m_Option == -1) break;

		switch (m_Option) {
			case 'f':
				condition = -1;
				break;

			case 'c':
				if (!IsInteger(m_OptionArgument)) return SetError(CLIError::kIntegerExpected);
				condition = atoi(m_OptionArgument.c_str());
				if (condition <= 0) return SetError(CLIError::kIntegerMustBePositive);
				break;
			default:
				return SetError(CLIError::kGetOptError);
		}
	}

	// never more than one arg
	if (m_NonOptionArguments > 1) return SetError(CLIError::kTooManyArgs);

	// we need a production if full or condition given
	if (condition) if (m_NonOptionArguments < 1) {
		SetErrorDetail("Production name required for that option.");
		return SetError(CLIError::kTooFewArgs);
	}

	// we have a production
	if (m_NonOptionArguments == 1) return DoExplainBacktraces(pAgent, &argv[m_Argument - m_NonOptionArguments], condition);
	
	// query
	return DoExplainBacktraces(pAgent);
}

bool CommandLineInterface::DoExplainBacktraces(gSKI::Agent* pAgent, const std::string* pProduction, const int condition) {
	if (!RequireAgent(pAgent)) return false;

	// quick sanity check
	if (condition < -1) return SetError(CLIError::kInvalidConditionNumber);

	// Attain the evil back door of doom, even though we aren't the TgD
	gSKI::EvilBackDoor::TgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();

	if (!pProduction) {
		// no production means query, ignore other args
		AddListenerAndDisableCallbacks(pAgent);
		pKernelHack->ExplainListChunks(pAgent);
		RemoveListenerAndEnableCallbacks(pAgent);
		return true;
	}

	AddListenerAndDisableCallbacks(pAgent);
	pKernelHack->ExplainChunks(pAgent, pProduction->c_str(), condition);
	RemoveListenerAndEnableCallbacks(pAgent);
	return true;
}

