#include "cli_CommandLineInterface.h"

#include "IgSKI_Agent.h"
#include "IgSKI_AgentPerformanceMonitor.h"

using namespace cli;

// ____                     ____  _        _
//|  _ \ __ _ _ __ ___  ___/ ___|| |_ __ _| |_ ___
//| |_) / _` | '__/ __|/ _ \___ \| __/ _` | __/ __|
//|  __/ (_| | |  \__ \  __/___) | || (_| | |_\__ \
//|_|   \__,_|_|  |___/\___|____/ \__\__,_|\__|___/
//
bool CommandLineInterface::ParseStats(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	// No arguments
	if (argv.size() != 1) {
		return HandleSyntaxError(Constants::kCLIStats, Constants::kCLITooManyArgs);
	}

	return DoStats(pAgent);
}

// ____       ____  _        _
//|  _ \  ___/ ___|| |_ __ _| |_ ___
//| | | |/ _ \___ \| __/ _` | __/ __|
//| |_| | (_) |__) | || (_| | |_\__ \
//|____/ \___/____/ \__\__,_|\__|___/
//
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

	pAgent->AddPrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);
	bool ret = pPerfMon->GetStatsString(argc, argv, &pResult);
	pAgent->RemovePrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);

	if(strlen(pResult) > 0) {
		AppendToResult(pResult);
	}
	delete [] argv[0];
	delete [] argv[1];
	return ret;
}

