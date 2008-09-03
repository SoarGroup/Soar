/********************************************************************
* @file igski_agentperformancemonitor.h 
*********************************************************************
* @remarks Copyright (C) 2004 Soar Technology, All rights reserved. 
* The U.S. government has non-exclusive license to this software 
* for government purposes. 
*********************************************************************
* created:	   6/2/2004   10:40
*
* purpose: 
*********************************************************************/
#ifndef IGSKI_AGENTPERFORMANCEMONITOR_H
#define IGSKI_AGENTPERFORMANCEMONITOR_H

namespace gSKI {
	typedef struct {
		unsigned long productionCountDefault;
		unsigned long productionCountUser;
		unsigned long productionCountChunk;
		unsigned long productionCountJustification;

		unsigned long cycleCountDecision;
		unsigned long cycleCountElaboration;

		unsigned long productionFiringCount;

		unsigned long wmeCountAddition;
		unsigned long wmeCountRemoval;
		unsigned long wmeCount;
		double        wmeCountAverage;
		unsigned long wmeCountMax;

		double        kernelTimeTotal;

		double        matchTimeInputPhase;
		double        matchTimeDetermineLevelPhase;
		double        matchTimePreferencePhase;
		double        matchTimeWorkingMemoryPhase;
		double        matchTimeOutputPhase;
		double        matchTimeDecisionPhase;

		double        ownershipTimeInputPhase;
		double        ownershipTimeDetermineLevelPhase;
		double        ownershipTimePreferencePhase;
		double        ownershipTimeWorkingMemoryPhase;
		double        ownershipTimeOutputPhase;
		double        ownershipTimeDecisionPhase;

		double        chunkingTimeInputPhase;
		double        chunkingTimeDetermineLevelPhase;
		double        chunkingTimePreferencePhase;
		double        chunkingTimeWorkingMemoryPhase;
		double        chunkingTimeOutputPhase;
		double        chunkingTimeDecisionPhase;

		unsigned long memoryUsageMiscellaneous;
		unsigned long memoryUsageHash;
		unsigned long memoryUsageString;
		unsigned long memoryUsagePool;
		unsigned long memoryUsageStatsOverhead;
	} AgentPerformanceData;

	class IAgentPerformanceMonitor
	{
	public:
		virtual ~IAgentPerformanceMonitor() {}

		virtual void GetStats(AgentPerformanceData* pStats) = 0;

		virtual bool GetStatsString(int argc, char* argv[], 
			const char** result) = 0;

	}; // IAgentPerformanceMonitor

}

#endif // IGSKI_AGENTPERFORMANCEMONITOR_H
