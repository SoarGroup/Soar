/////////////////////////////////////////////////////////////////
// attribute-preferences command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2005
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"
#include "sml_Names.h"
#include "sml_StringOps.h"
#include "agent.h"
#include "cli_CLIError.h"


using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseAttributePreferencesMode(std::vector<std::string>& argv) {
	if (argv.size() > 2) return SetError(CLIError::kTooManyArgs);
	
	// Display current mode if no args
	if (argv.size() < 2) return DoAttributePreferencesMode();

	// Set new mode
	if (!IsInteger(argv[1])) return SetError(CLIError::kIntegerExpected);
	int mode = atoi(argv[1].c_str());
	if (mode < 0) return SetError(CLIError::kIntegerMustBeNonNegative);
	if (mode > 2) return SetError(CLIError::kIntegerOutOfRange);
	return DoAttributePreferencesMode(&mode);
}

bool CommandLineInterface::DoAttributePreferencesMode(int* pMode) {

	if (m_pAgentSoar->operand2_mode) return SetError(CLIError::kSoar7Command);

	if (!pMode) {
		// query
		if (m_RawOutput) {
			m_Result << m_pAgentSoar->attribute_preferences_mode;
		} else {
			char buf[kMinBufferSize];
			AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeInt, Int2String(m_pAgentSoar->attribute_preferences_mode, buf, kMinBufferSize));
		}
		return true;
	}

	m_pAgentSoar->attribute_preferences_mode = *pMode;

	return true;
}

