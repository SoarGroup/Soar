#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Constants.h"

#include "IgSKI_Agent.h"
#include "IgSKI_AgentPerformanceMonitor.h"

using namespace cli;

bool CommandLineInterface::ParseStats(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	// No arguments
	if (argv.size() != 1) {
		return HandleSyntaxError(Constants::kCLIStats, Constants::kCLITooManyArgs);
	}

	return DoStats(pAgent);
}

bool CommandLineInterface::DoStats(gSKI::IAgent* pAgent) {
	// Need agent pointer for function calls
	if (!RequireAgent(pAgent)) return false;

	gSKI::IAgentPerformanceMonitor* pPerfMon = pAgent->GetPerformanceMonitor();

	// This next block needs to be redone pending a rewrite on the gSKI side.
	int argc = 1;
	char* argv[3];
	argv[0] = new char[6];
	memset(argv[0], 0, 6);
	strncpy(argv[0], "stats", 5);

	argv[1] = new char[7];
	memset(argv[1], 0, 7);
	strncpy(argv[1], "-stats", 6);

	argv[2] = 0;

	const char* pResult = 0;

	bool ret = pPerfMon->GetStatsString(argc, argv, &pResult);

	if(strlen(pResult) > 0) {
		AppendToResult(pResult);
	}
	delete [] argv[0];
	delete [] argv[1];
	return ret;
}

