/////////////////////////////////////////////////////////////////
// o-support-mode command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "cli_CommandLineInterface.h"
#include "cli_CLIError.h"

#include "cli_Commands.h"
#include "sml_Names.h"

#include "agent.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseOSupportMode(std::vector<std::string>& argv) {
	
	if (argv.size() > 2) return SetError(CLIError::kTooManyArgs);

	int mode = -1;
	if (argv.size() == 2) {
		if (!isdigit(argv[1][0])) {
			SetErrorDetail("Expected an integer 0, 2, 3, or 4.");
			return SetError(CLIError::kIntegerOutOfRange);
		}
		from_string(mode, argv[1]);
		if (mode < 0 || mode > 4 || mode == 1) {
			SetErrorDetail("Expected an integer 0, 2, 3, or 4.");
			return SetError(CLIError::kIntegerOutOfRange);
		}
	}

	return DoOSupportMode(mode);
}

bool CommandLineInterface::DoOSupportMode(int mode) {
	if (mode < 0) {
		mode = m_pAgentSoar->o_support_calculation_type;

		if (m_RawOutput) {
			m_Result << mode;
		} else {
			std::stringstream buffer;
			buffer << mode;
			AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeInt, buffer.str());
		}
	} else {
		assert(mode != 1);
		m_pAgentSoar->o_support_calculation_type = mode;
	}

	return true;
}

