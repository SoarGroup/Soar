#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Constants.h"

#include "IgSKI_Agent.h"

#ifdef _MSC_VER
#define snprintf _snprintf 
#endif // _MSC_VER

using namespace cli;

bool CommandLineInterface::ParseMaxElaborations(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {

	// n defaults to 0 (print current value)
	int n = 0;

	if (argv.size() > 2) return m_Error.SetError(CLIError::kTooManyArgs);

	// one argument, figure out if it is a positive integer
	if (argv.size() == 2) {
		n = atoi(argv[1].c_str());
		if (n <= 0) return m_Error.SetError(CLIError::kIntegerMustBePositive);
	}

	return DoMaxElaborations(pAgent, n);
}

bool CommandLineInterface::DoMaxElaborations(gSKI::IAgent* pAgent, int n) {

	if (!RequireAgent(pAgent)) return false;

	if (!n) {
		char buf[32];
		snprintf(buf, 31, "%d", pAgent->GetMaxElaborations());
		buf[31] = 0;
		AppendToResult(buf);
		return true;
	}

	pAgent->SetMaxElaborations(n);
	return true;
}

