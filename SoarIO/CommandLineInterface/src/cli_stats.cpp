#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Constants.h"
#include "cli_GetOpt.h"
#include "sml_Names.h"
#include "sml_StringOps.h"

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

	StatsBitset options(0);

	for (;;) {
		int option = m_pGetOpt->GetOpt_Long(argv, "mrs", longOptions, 0);
		if (option == -1) break;

		switch (option) {
			case 'm':
				options.reset();
				options.set(STATS_MEMORY);
				break;
			case 'r':
				options.reset();
				options.set(STATS_RETE);
				break;
			case 's':
				options.reset();
				options.set(STATS_SYSTEM);
				break;
			case '?':
				SetErrorDetail("Bad option '" + m_pGetOpt->GetOptOpt() + "'.");
				return SetError(CLIError::kUnrecognizedOption);
			default:
				return SetError(CLIError::kGetOptError);
		}
	}

	// No arguments
	if (m_pGetOpt->GetAdditionalArgCount()) return SetError(CLIError::kTooManyArgs);

	return DoStats(pAgent, options);
}

bool CommandLineInterface::DoStats(gSKI::IAgent* pAgent, const StatsBitset& options) {
	// Need agent pointer for function calls
	if (!RequireAgent(pAgent)) return false;

	gSKI::IAgentPerformanceMonitor* pPerfMon = pAgent->GetPerformanceMonitor();

	if (m_RawOutput) {
		const char* _stats = "stats";
		const char* _memory = "-memory";
		const char* _rete = "-rete";
		const char* _system = "-system";

		char* argv[3];
		int argc = 0;

		argv[argc++] = const_cast<char*>(_stats);

		if (options.test(STATS_MEMORY)) {
			argv[argc++] = const_cast<char*>(_memory);
		} else if (options.test(STATS_RETE)) {
			argv[argc++] = const_cast<char*>(_rete);
		} else if (options.test(STATS_SYSTEM)) {
			argv[argc++] = const_cast<char*>(_system);
		}
		argv[argc] = 0;

		const char* pResult = 0;

		this->AddListenerAndDisableCallbacks(pAgent);
		bool ret = pPerfMon->GetStatsString(argc, argv, &pResult);
		this->RemoveListenerAndEnableCallbacks(pAgent);

		if (!ret) {
			SetErrorDetail("Error getting stats string.");
			return SetError(CLIError::kgSKIError);
		}

		// Ideally all stats options would be put in pResult, but that is a lot of work
		// so only -system does
		if (options.test(STATS_SYSTEM) || options.none()) m_Result << pResult;

	} else {
		// structured output
		gSKI::AgentPerformanceData stats;
		pPerfMon->GetStats(&stats);

		char buf[kMinBufferSize];

		AppendArgTagFast(sml_Names::kParamStatsProductionCountDefault,				sml_Names::kTypeInt,	Int2String(stats.productionCountDefault, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsProductionCountUser,					sml_Names::kTypeInt,	Int2String(stats.productionCountUser, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsProductionCountChunk,				sml_Names::kTypeInt,	Int2String(stats.productionCountChunk, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsProductionCountJustification,		sml_Names::kTypeInt,	Int2String(stats.productionCountJustification, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsCycleCountDecision,					sml_Names::kTypeInt,	Int2String(stats.cycleCountDecision, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsCycleCountElaboration,				sml_Names::kTypeInt,	Int2String(stats.cycleCountElaboration, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsProductionFiringCount,				sml_Names::kTypeInt,	Int2String(stats.productionFiringCount, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsWmeCountAddition,					sml_Names::kTypeInt,	Int2String(stats.wmeCountAddition, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsWmeCountRemoval,						sml_Names::kTypeInt,	Int2String(stats.wmeCountRemoval, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsWmeCount,							sml_Names::kTypeInt,	Int2String(stats.wmeCount, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsWmeCountAverage,						sml_Names::kTypeDouble, Double2String(stats.wmeCountAverage, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsWmeCountMax,							sml_Names::kTypeInt,	Int2String(stats.wmeCountMax, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsKernelTimeTotal,						sml_Names::kTypeDouble, Double2String(stats.kernelTimeTotal, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsMatchTimeInputPhase,					sml_Names::kTypeDouble, Double2String(stats.matchTimeInputPhase, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsMatchTimeDetermineLevelPhase,		sml_Names::kTypeDouble, Double2String(stats.matchTimeDetermineLevelPhase, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsMatchTimePreferencePhase,			sml_Names::kTypeDouble, Double2String(stats.matchTimePreferencePhase, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsMatchTimeWorkingMemoryPhase,			sml_Names::kTypeDouble, Double2String(stats.matchTimeWorkingMemoryPhase, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsMatchTimeOutputPhase,				sml_Names::kTypeDouble, Double2String(stats.matchTimeOutputPhase, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsMatchTimeDecisionPhase,				sml_Names::kTypeDouble, Double2String(stats.matchTimeDecisionPhase, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsOwnershipTimeInputPhase,				sml_Names::kTypeDouble, Double2String(stats.ownershipTimeInputPhase, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsOwnershipTimeDetermineLevelPhase,	sml_Names::kTypeDouble, Double2String(stats.ownershipTimeDetermineLevelPhase, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsOwnershipTimePreferencePhase,		sml_Names::kTypeDouble, Double2String(stats.ownershipTimePreferencePhase, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsOwnershipTimeWorkingMemoryPhase,		sml_Names::kTypeDouble, Double2String(stats.ownershipTimeWorkingMemoryPhase, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsOwnershipTimeOutputPhase,			sml_Names::kTypeDouble, Double2String(stats.ownershipTimeOutputPhase, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsOwnershipTimeDecisionPhase,			sml_Names::kTypeDouble, Double2String(stats.ownershipTimeDecisionPhase, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsChunkingTimeInputPhase,				sml_Names::kTypeDouble, Double2String(stats.chunkingTimeInputPhase, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsChunkingTimeDetermineLevelPhase,		sml_Names::kTypeDouble, Double2String(stats.chunkingTimeDetermineLevelPhase, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsChunkingTimePreferencePhase,			sml_Names::kTypeDouble, Double2String(stats.chunkingTimePreferencePhase, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsChunkingTimeWorkingMemoryPhase,		sml_Names::kTypeDouble, Double2String(stats.chunkingTimeWorkingMemoryPhase, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsChunkingTimeOutputPhase,				sml_Names::kTypeDouble, Double2String(stats.chunkingTimeOutputPhase, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsChunkingTimeDecisionPhase,			sml_Names::kTypeDouble, Double2String(stats.chunkingTimeDecisionPhase, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsMemoryUsageMiscellaneous,			sml_Names::kTypeInt,	Int2String(stats.memoryUsageMiscellaneous, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsMemoryUsageHash,						sml_Names::kTypeInt,	Int2String(stats.memoryUsageHash, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsMemoryUsageString,					sml_Names::kTypeInt,	Int2String(stats.memoryUsageString, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsMemoryUsagePool,						sml_Names::kTypeInt,	Int2String(stats.memoryUsagePool, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsMemoryUsageStatsOverhead,			sml_Names::kTypeInt,	Int2String(stats.memoryUsageStatsOverhead, buf, kMinBufferSize));
	}
	return true;
}

