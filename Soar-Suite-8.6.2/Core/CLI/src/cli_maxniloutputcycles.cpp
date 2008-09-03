/////////////////////////////////////////////////////////////////
// max-nil-output-cycles command file.
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

#include "IgSKI_Agent.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseMaxNilOutputCycles(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	// n defaults to 0 (print current value)
	int n = 0;

	if (argv.size() > 2) return SetError(CLIError::kTooManyArgs);

	// one argument, figure out if it is a positive integer
	if (argv.size() == 2) {
		n = atoi(argv[1].c_str());
		if (n <= 0) return SetError(CLIError::kIntegerMustBePositive);
	}

	return DoMaxNilOutputCycles(pAgent, n);
}

bool CommandLineInterface::DoMaxNilOutputCycles(gSKI::IAgent* pAgent, const int n) {
	if (!RequireAgent(pAgent)) return false;

	if (!n) {
		// query
		if (m_RawOutput) {
			m_Result << pAgent->GetMaxNilOutputCycles();
		} else {
			char buf[kMinBufferSize];
			AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeInt, Int2String(pAgent->GetMaxNilOutputCycles(), buf, kMinBufferSize));
		}
		return true;
	}

	pAgent->SetMaxNilOutputCycles(n);
	return true;
}

