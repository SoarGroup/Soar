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

bool CommandLineInterface::ParseMaxChunks(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {

	// n defaults to 0 (print current value)
	int n = 0;

	if (argv.size() > 2) return HandleSyntaxError(Constants::kCLIMaxChunks, Constants::kCLITooManyArgs);

	// one argument, figure out if it is a positive integer
	if (argv.size() == 2) {
		n = atoi(argv[1].c_str());
		if (n <= 0) return HandleSyntaxError(Constants::kCLIMaxChunks, "Integer argument must be positive.");
	}

	return DoMaxChunks(pAgent, n);
}

bool CommandLineInterface::DoMaxChunks(gSKI::IAgent* pAgent, int n) {

	if (!RequireAgent(pAgent)) return false;

	if (!n) {
		char buf[32];
		snprintf(buf, 31, "%d", pAgent->GetMaxChunks());
		AppendToResult(buf);
		return true;
	}

	pAgent->SetMaxChunks(n);
	return true;
}

