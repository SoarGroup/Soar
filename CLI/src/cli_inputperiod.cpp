/////////////////////////////////////////////////////////////////
// input-period command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2005
//
/////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"
#include "IgSKI_Agent.h"
#include "sml_Names.h"
#include "sml_StringOps.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseInputPeriod(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	if (argv.size() > 2) return SetError(CLIError::kTooManyArgs);
	if (argv.size() == 1) return DoInputPeriod(pAgent);

	if (!IsInteger(argv[1])) return SetError(CLIError::kIntegerExpected);
	
	int period = atoi(argv[1].c_str());
	if (period < 0) return SetError(CLIError::kIntegerMustBeNonNegative);

	return DoInputPeriod(pAgent, &period);
}

bool CommandLineInterface::DoInputPeriod(gSKI::IAgent* pAgent, int* pPeriod) {
	if (pAgent->GetOperand2Mode()) return SetError(CLIError::kSoar7Command);
	
	if (!pPeriod) {
		if (m_RawOutput) {
			m_Result << pAgent->GetInputPeriod();
		} else {
			char buf[kMinBufferSize];
			AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeInt, Int2String(pAgent->GetInputPeriod(), buf, kMinBufferSize));
		}
		return true;
	}

	pAgent->SetInputPeriod(*pPeriod);
	return true;
}

