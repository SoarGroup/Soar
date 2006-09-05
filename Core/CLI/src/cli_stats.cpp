/////////////////////////////////////////////////////////////////
// stats command file.
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

#include "gSKI_Agent.h"
#include "gSKI_AgentPerformanceMonitor.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseStats(gSKI::Agent* pAgent, std::vector<std::string>& argv) {
	Options optionsData[] = {
		{'m', "memory",	0},
		{'r', "rete",	0},
		{'s', "system",	0},
		{0, 0, 0}
	};

	StatsBitset options(0);

	for (;;) {
		if (!ProcessOptions(argv, optionsData)) return false;
		if (m_Option == -1) break;

		switch (m_Option) {
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
			default:
				return SetError(CLIError::kGetOptError);
		}
	}

	// No arguments
	if (m_NonOptionArguments) return SetError(CLIError::kTooManyArgs);

	return DoStats(pAgent, options);
}

bool CommandLineInterface::DoStats(gSKI::Agent* pAgent, const StatsBitset& options) {
	// Need agent pointer for function calls
	if (!RequireAgent(pAgent)) return false;

	gSKI::AgentPerformanceMonitor* pPerfMon = pAgent->GetPerformanceMonitor();

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
		AppendArgTagFast(sml_Names::kParamStatsKernelCPUTime,						sml_Names::kTypeDouble, Double2String(stats.kernelCPUTime, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsTotalCPUTime,						sml_Names::kTypeDouble, Double2String(stats.totalCPUTime, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsPhaseTimeInputPhase,					sml_Names::kTypeDouble, Double2String(stats.phaseTimeInputPhase, buf, kMinBufferSize)); 
		AppendArgTagFast(sml_Names::kParamStatsPhaseTimeProposePhase,				sml_Names::kTypeDouble, Double2String(stats.phaseTimeProposePhase, buf, kMinBufferSize)); 
		AppendArgTagFast(sml_Names::kParamStatsPhaseTimeDecisionPhase,				sml_Names::kTypeDouble, Double2String(stats.phaseTimeDecisionPhase, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsPhaseTimeApplyPhase,					sml_Names::kTypeDouble, Double2String(stats.phaseTimeApplyPhase, buf, kMinBufferSize));  
		AppendArgTagFast(sml_Names::kParamStatsPhaseTimeOutputPhase,				sml_Names::kTypeDouble, Double2String(stats.phaseTimeOutputPhase, buf, kMinBufferSize)); 
		AppendArgTagFast(sml_Names::kParamStatsPhaseTimePreferencePhase,			sml_Names::kTypeDouble, Double2String(stats.phaseTimePreferencePhase, buf, kMinBufferSize)); 
		AppendArgTagFast(sml_Names::kParamStatsPhaseTimeWorkingMemoryPhase,			sml_Names::kTypeDouble, Double2String(stats.phaseTimeWorkingMemoryPhase, buf, kMinBufferSize)); 
		AppendArgTagFast(sml_Names::kParamStatsMonitorTimeInputPhase,				sml_Names::kTypeDouble, Double2String(stats.monitorTimeInputPhase, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsMonitorTimeProposePhase,				sml_Names::kTypeDouble, Double2String(stats.monitorTimeProposePhase, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsMonitorTimeDecisionPhase,			sml_Names::kTypeDouble, Double2String(stats.monitorTimeDecisionPhase, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsMonitorTimeApplyPhase,				sml_Names::kTypeDouble, Double2String(stats.monitorTimeApplyPhase, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsMonitorTimeOutputPhase,				sml_Names::kTypeDouble, Double2String(stats.monitorTimeOutputPhase, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsMonitorTimePreferencePhase,			sml_Names::kTypeDouble, Double2String(stats.monitorTimePreferencePhase, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsMonitorTimeWorkingMemoryPhase,		sml_Names::kTypeDouble, Double2String(stats.monitorTimeWorkingMemoryPhase, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsInputFunctionTime,					sml_Names::kTypeDouble, Double2String(stats.inputFunctionTime, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsOutputFunctionTime,					sml_Names::kTypeDouble, Double2String(stats.outputFunctionTime, buf, kMinBufferSize));	
		AppendArgTagFast(sml_Names::kParamStatsMatchTimeInputPhase,					sml_Names::kTypeDouble, Double2String(stats.matchTimeInputPhase, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsMatchTimePreferencePhase,			sml_Names::kTypeDouble, Double2String(stats.matchTimePreferencePhase, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsMatchTimeWorkingMemoryPhase,			sml_Names::kTypeDouble, Double2String(stats.matchTimeWorkingMemoryPhase, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsMatchTimeOutputPhase,				sml_Names::kTypeDouble, Double2String(stats.matchTimeOutputPhase, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsMatchTimeDecisionPhase,				sml_Names::kTypeDouble, Double2String(stats.matchTimeDecisionPhase, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsMatchTimeProposePhase,				sml_Names::kTypeDouble, Double2String(stats.matchTimeProposePhase, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsMatchTimeApplyPhase,					sml_Names::kTypeDouble, Double2String(stats.matchTimeApplyPhase, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsOwnershipTimeInputPhase,				sml_Names::kTypeDouble, Double2String(stats.ownershipTimeInputPhase, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsOwnershipTimePreferencePhase,		sml_Names::kTypeDouble, Double2String(stats.ownershipTimePreferencePhase, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsOwnershipTimeWorkingMemoryPhase,		sml_Names::kTypeDouble, Double2String(stats.ownershipTimeWorkingMemoryPhase, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsOwnershipTimeOutputPhase,			sml_Names::kTypeDouble, Double2String(stats.ownershipTimeOutputPhase, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsOwnershipTimeDecisionPhase,			sml_Names::kTypeDouble, Double2String(stats.ownershipTimeDecisionPhase, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsOwnershipTimeProposePhase,			sml_Names::kTypeDouble, Double2String(stats.ownershipTimeProposePhase, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsOwnershipTimeApplyPhase,				sml_Names::kTypeDouble, Double2String(stats.ownershipTimeApplyPhase, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsChunkingTimeInputPhase,				sml_Names::kTypeDouble, Double2String(stats.chunkingTimeInputPhase, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsChunkingTimePreferencePhase,			sml_Names::kTypeDouble, Double2String(stats.chunkingTimePreferencePhase, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsChunkingTimeWorkingMemoryPhase,		sml_Names::kTypeDouble, Double2String(stats.chunkingTimeWorkingMemoryPhase, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsChunkingTimeOutputPhase,				sml_Names::kTypeDouble, Double2String(stats.chunkingTimeOutputPhase, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsChunkingTimeDecisionPhase,			sml_Names::kTypeDouble, Double2String(stats.chunkingTimeDecisionPhase, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsChunkingTimeProposePhase,			sml_Names::kTypeDouble, Double2String(stats.chunkingTimeProposePhase, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsChunkingTimeApplyPhase,				sml_Names::kTypeDouble, Double2String(stats.chunkingTimeApplyPhase, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsMemoryUsageMiscellaneous,			sml_Names::kTypeInt,	Int2String(stats.memoryUsageMiscellaneous, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsMemoryUsageHash,						sml_Names::kTypeInt,	Int2String(stats.memoryUsageHash, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsMemoryUsageString,					sml_Names::kTypeInt,	Int2String(stats.memoryUsageString, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsMemoryUsagePool,						sml_Names::kTypeInt,	Int2String(stats.memoryUsagePool, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamStatsMemoryUsageStatsOverhead,			sml_Names::kTypeInt,	Int2String(stats.memoryUsageStatsOverhead, buf, kMinBufferSize));
	}
	return true;
}

