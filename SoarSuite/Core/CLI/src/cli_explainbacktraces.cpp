/////////////////////////////////////////////////////////////////
// explain-backtraces command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "sml_Utils.h"
#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"
#include "cli_CLIError.h"

#include "sml_Names.h"
#include "sml_StringOps.h"

#include "sml_KernelSML.h"
#include "sml_KernelHelpers.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseExplainBacktraces(std::vector<std::string>& argv) {
	Options optionsData[] = {
		{'c', "condition",	OPTARG_REQUIRED},
		{'f', "full",		OPTARG_NONE},
		{0, 0, OPTARG_NONE}
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
	if (m_NonOptionArguments == 1) return DoExplainBacktraces(&argv[m_Argument - m_NonOptionArguments], condition);
	
	// query
	return DoExplainBacktraces();
}

bool CommandLineInterface::DoExplainBacktraces(const std::string* pProduction, const int condition) {
	// quick sanity check
	if (condition < -1) return SetError(CLIError::kInvalidConditionNumber);

	// Attain the evil back door of doom, even though we aren't the TgD
	sml::KernelHelpers* pKernelHack = m_pKernelSML->GetKernelHelpers() ;

	if (!pProduction) {
		// no production means query, ignore other args
		pKernelHack->ExplainListChunks(m_pAgentSML);
		return true;
	}

	pKernelHack->ExplainChunks(m_pAgentSML, pProduction->c_str(), condition);
	return true;
}

