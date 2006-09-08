/////////////////////////////////////////////////////////////////
// attribute-preferences command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2005
//
/////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include <portability.h>

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"
#include "sml_Names.h"
#include "sml_StringOps.h"

#include "gSKI_Agent.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseAttributePreferencesMode(gSKI::Agent* pAgent, std::vector<std::string>& argv) {
	if (argv.size() > 2) return SetError(CLIError::kTooManyArgs);
	
	// Display current mode if no args
	if (argv.size() < 2) return DoAttributePreferencesMode(pAgent);

	// Set new mode
	if (!IsInteger(argv[1])) return SetError(CLIError::kIntegerExpected);
	int mode = atoi(argv[1].c_str());
	if (mode < 0) return SetError(CLIError::kIntegerMustBeNonNegative);
	if (mode > 2) return SetError(CLIError::kIntegerOutOfRange);
	return DoAttributePreferencesMode(pAgent, &mode);
}

bool CommandLineInterface::DoAttributePreferencesMode(gSKI::Agent* pAgent, int* pMode) {
	if (pAgent->GetOperand2Mode()) return SetError(CLIError::kSoar7Command);

	if (!pMode) {
		// query
		if (m_RawOutput) {
			m_Result << pAgent->GetAttributePreferencesMode();
		} else {
			char buf[kMinBufferSize];
			AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeInt, Int2String(pAgent->GetAttributePreferencesMode(), buf, kMinBufferSize));
		}
		return true;
	}
	
	pAgent->SetAttributePreferencesMode(*pMode);
	return true;
}

