#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Constants.h"
#include "sml_Names.h"

#include "IgSKI_Agent.h"
#include "IgSKI_AgentPerformanceMonitor.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseStats(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	// No arguments
	if (argv.size() != 1) {
		return m_Error.SetError(CLIError::kTooManyArgs);
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

	delete [] argv[0];
	delete [] argv[1];

	if (!ret) return m_Error.SetError(CLIError::kgSKIError);

	if (m_RawOutput) {
		AppendToResult(pResult);
	} else {
		AppendArgTagFast(sml_Names::kParamMessage, sml_Names::kTypeString, pResult);
	}
	return true;
}

