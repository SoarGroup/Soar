#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Constants.h"
#include "cli_GetOpt.h"
#include "sml_Names.h"

#include "IgSKI_Agent.h"
#include "IgSKI_AgentPerformanceMonitor.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseStats(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	static struct GetOpt::option longOptions[] = {
		{"memory",	0, 0, 'm'},
		{"rete",	0, 0, 'r'},
		{"system",	0, 0, 's'},
		{0, 0, 0, 0}
	};

	unsigned int options = 0;

	for (;;) {
		int option = m_pGetOpt->GetOpt_Long(argv, "mrs", longOptions, 0);
		if (option == -1) break;

		switch (option) {
			case 'm':
				options |= OPTION_STATS_MEMORY;
				break;
			case 'r':
				options |= OPTION_STATS_RETE;
				break;
			case 's':
				options |= OPTION_STATS_SYSTEM;
				break;
			case '?':
				return SetError(CLIError::kUnrecognizedOption);
			default:
				return SetError(CLIError::kGetOptError);
		}
	}

	// No arguments
	if (m_pGetOpt->GetAdditionalArgCount()) return SetError(CLIError::kTooManyArgs);

	return DoStats(pAgent, options);
}

bool CommandLineInterface::DoStats(gSKI::IAgent* pAgent, const int options) {
	// Need agent pointer for function calls
	if (!RequireAgent(pAgent)) return false;

	gSKI::IAgentPerformanceMonitor* pPerfMon = pAgent->GetPerformanceMonitor();

	const char* _stats = "stats";
	const char* _memory = "-memory";
	const char* _rete = "-rete";
	const char* _system = "-system";

	char* argv[5];
	int argc = 0;

	argv[argc++] = const_cast<char*>(_stats);

	if (options & OPTION_STATS_MEMORY) {
		argv[argc++] = const_cast<char*>(_memory);
	}

	if (options & OPTION_STATS_RETE) {
		argv[argc++] = const_cast<char*>(_rete);
	}

	if (options & OPTION_STATS_SYSTEM) {
		argv[argc++] = const_cast<char*>(_system);
	}
	argv[argc] = 0;

	const char* pResult = 0;

	bool ret = pPerfMon->GetStatsString(argc, argv, &pResult);

	if (!ret) return SetError(CLIError::kgSKIError);

	if (m_RawOutput) {
		AppendToResult(pResult);
	} else {
		AppendArgTagFast(sml_Names::kParamMessage, sml_Names::kTypeString, pResult);
	}
	return true;
}

