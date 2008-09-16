/////////////////////////////////////////////////////////////////
// input-period command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2005
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"
#include "cli_CLIError.h"
#include "sml_Names.h"
#include "sml_StringOps.h"

#include "agent.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseInputPeriod(std::vector<std::string>& argv) {
	if (argv.size() > 2) return SetError(CLIError::kTooManyArgs);
	if (argv.size() == 1) return DoInputPeriod();

	if (!IsInteger(argv[1])) return SetError(CLIError::kIntegerExpected);
	
	int period = atoi(argv[1].c_str());
	if (period < 0) return SetError(CLIError::kIntegerMustBeNonNegative);

	return DoInputPeriod(&period);
}

bool CommandLineInterface::DoInputPeriod(int* pPeriod) {
	if (m_pAgentSoar->operand2_mode) return SetError(CLIError::kSoar7Command);

	if (!pPeriod) {
		if (m_RawOutput) {
			m_Result << m_pAgentSoar->input_period;
		} else {
			char buf[kMinBufferSize];
			AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeInt, Int2String(m_pAgentSoar->input_period, buf, kMinBufferSize));
		}
		return true;
	}

	m_pAgentSoar->input_period = *pPeriod;
	return true;
}

