/////////////////////////////////////////////////////////////////
// max-memory-usage command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2006
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
#include "agent.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseMaxMemoryUsage(gSKI::Agent* pAgent, std::vector<std::string>& argv) {

	// n defaults to 0 (print current value)
	int n = 0;

	if (argv.size() > 2) return SetError(CLIError::kTooManyArgs);

	// one argument, figure out if it is a positive integer
	if (argv.size() == 2) {
		n = atoi(argv[1].c_str());
		if (n <= 0) return SetError(CLIError::kIntegerMustBePositive);
	}

	return DoMaxMemoryUsage(pAgent->GetSoarAgent(), n);
}

bool CommandLineInterface::DoMaxMemoryUsage(agent* pAgent, const int n) {

	if (!RequireAgent(pAgent)) return false;

	if (!n) {
		// query
		if (m_RawOutput) {
			m_Result << pAgent->sysparams[MAX_MEMORY_USAGE_SYSPARAM] << " bytes";
		} else {
			char buf[kMinBufferSize];
			AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeInt, Int2String(pAgent->sysparams[MAX_CHUNKS_SYSPARAM], buf, kMinBufferSize));
		}
		return true;
	}

	pAgent->sysparams[MAX_MEMORY_USAGE_SYSPARAM] = n;
	return true;
}

