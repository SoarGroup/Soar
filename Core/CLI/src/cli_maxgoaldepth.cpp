/////////////////////////////////////////////////////////////////
// max-goal-depth command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2009
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"
#include "sml_Names.h"

#include "agent.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseMaxGoalDepth(std::vector<std::string>& argv) {
	// n defaults to 0 (print current value)
	int n = 0;

	if (argv.size() > 2) return SetError(kTooManyArgs);

	// one argument, figure out if it is a positive integer
	if (argv.size() == 2) {
		from_string(n, argv[1]);
		if (n <= 0) return SetError(kIntegerMustBePositive);
	}

	return DoMaxGoalDepth(n);
}

bool CommandLineInterface::DoMaxGoalDepth(const int n) {
	if (!n) {
		// query
		if (m_RawOutput) {
			m_Result << m_pAgentSoar->sysparams[MAX_GOAL_DEPTH];
		} else {
			std::string temp;
			AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeInt, to_string(m_pAgentSoar->sysparams[MAX_GOAL_DEPTH], temp));
		}
		return true;
	}

	m_pAgentSoar->sysparams[MAX_GOAL_DEPTH] = n;
	return true;
}

