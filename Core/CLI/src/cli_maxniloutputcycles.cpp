/////////////////////////////////////////////////////////////////
// max-nil-output-cycles command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"
#include "sml_Names.h"

#include "agent.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoMaxNilOutputCycles(const int n) {
    agent* agnt = m_pAgentSML->GetSoarAgent();
	if (!n) {
		// query
		if (m_RawOutput) {
			m_Result << agnt->sysparams[MAX_NIL_OUTPUT_CYCLES_SYSPARAM];
		} else {
			std::string temp;
			AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeInt, to_string(agnt->sysparams[MAX_NIL_OUTPUT_CYCLES_SYSPARAM], temp));
		}
		return true;
	}

	agnt->sysparams[MAX_NIL_OUTPUT_CYCLES_SYSPARAM] = n;
	return true;
}

