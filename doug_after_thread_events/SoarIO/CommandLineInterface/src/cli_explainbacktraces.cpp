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

	int condition = 0;

	for (;;) {
		int option = m_pGetOpt->GetOpt_Long(argv, ":c:f", longOptions, 0);
		if (option == -1) break;

		switch (option) {
			case 'f':
				condition = -1;
				break;

			case 'c':
				if (!IsInteger(m_pGetOpt->GetOptArg())) return SetError(CLIError::kIntegerExpected);
				condition = atoi(m_pGetOpt->GetOptArg());
				if (condition <= 0) return SetError(CLIError::kIntegerMustBePositive);
				break;

			case ':':
				{
					std::string detail;
					detail = static_cast<char>(m_pGetOpt->GetOptOpt());
					SetErrorDetail("Option '" + detail + "' needs an argument.");
				}
				return SetError(CLIError::kMissingOptionArg);
			case '?':
				{
					std::string detail;
					if (m_pGetOpt->GetOptOpt()) {
						detail = static_cast<char>(m_pGetOpt->GetOptOpt());
					} else {
						detail = argv[m_pGetOpt->GetOptind() - 1];
					}
					SetErrorDetail("Bad option '" + detail + "'.");
				}
				return SetError(CLIError::kUnrecognizedOption);
			default:
				return SetError(CLIError::kGetOptError);
		}
	}

	// never more than one arg
	if (m_pGetOpt->GetAdditionalArgCount() > 1) return SetError(CLIError::kTooManyArgs);

	// we need a production if full or condition given
	if (condition) if (m_pGetOpt->GetAdditionalArgCount() < 1) {
		SetErrorDetail("Production name required for that option.");
		return SetError(CLIError::kTooFewArgs);
	}

	// we have a production
	if (m_pGetOpt->GetAdditionalArgCount() == 1) return DoExplainBacktraces(pAgent, &argv[m_pGetOpt->GetOptind()], condition);
	
	// query
	return DoExplainBacktraces(pAgent);
}

bool CommandLineInterface::DoExplainBacktraces(gSKI::IAgent* pAgent, const std::string* pProduction, const int condition) {
	if (!RequireAgent(pAgent)) return false;

	// quick sanity check
	if (condition < -1) return SetError(CLIError::kInvalidConditionNumber);

	// Attain the evil back door of doom, even though we aren't the TgD
	gSKI::EvilBackDoor::ITgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();

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

