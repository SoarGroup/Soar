/*************************************************************************
* PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
* FOR LICENSE AND COPYRIGHT INFORMATION. 
*************************************************************************/

/********************************************************************
* @file gski_agentperformancemonitor.h
*********************************************************************
* created:	   6/2/2004   10:48
*
* purpose: 
*********************************************************************/
#ifndef GSKI_AGENTPERFORMANCEMONITOR_H
#define GSKI_AGENTPERFORMANCEMONITOR_H

#ifdef _WIN32
#pragma warning(disable: 4786)
#endif

#include <string>

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

		double        kernelCPUTime;
		double        totalCPUTime;

		double		  inputFunctionTime;
		double        outputFunctionTime;

		double        phaseTimeInputPhase;
		double        phaseTimePreferencePhase;
		double        phaseTimeWorkingMemoryPhase;
		double        phaseTimeOutputPhase;
		double        phaseTimeDecisionPhase;
		double        phaseTimeProposePhase;
		double        phaseTimeApplyPhase;

		double        monitorTimeInputPhase;
		double        monitorTimePreferencePhase;
		double        monitorTimeWorkingMemoryPhase;
		double        monitorTimeOutputPhase;
		double        monitorTimeDecisionPhase;
		double        monitorTimeProposePhase;
		double        monitorTimeApplyPhase;

		double        matchTimeInputPhase;
		double        matchTimePreferencePhase;
		double        matchTimeWorkingMemoryPhase;
		double        matchTimeOutputPhase;
		double        matchTimeDecisionPhase;
		double        matchTimeProposePhase;
		double        matchTimeApplyPhase;

		double        ownershipTimeInputPhase;
		double        ownershipTimePreferencePhase;
		double        ownershipTimeWorkingMemoryPhase;
		double        ownershipTimeOutputPhase;
		double        ownershipTimeDecisionPhase;
		double        ownershipTimeProposePhase;
		double        ownershipTimeApplyPhase;

		double        chunkingTimeInputPhase;
		double        chunkingTimePreferencePhase;
		double        chunkingTimeWorkingMemoryPhase;
		double        chunkingTimeOutputPhase;
		double        chunkingTimeDecisionPhase;
		double        chunkingTimeProposePhase;
		double        chunkingTimeApplyPhase;

		unsigned long memoryUsageMiscellaneous;
		unsigned long memoryUsageHash;
		unsigned long memoryUsageString;
		unsigned long memoryUsagePool;
		unsigned long memoryUsageStatsOverhead;
	} AgentPerformanceData;

	class Agent;

	class AgentPerformanceMonitor
	{
	public:

		AgentPerformanceMonitor(Agent* pAgent);

		virtual ~AgentPerformanceMonitor();

		virtual bool GetStatsString(int argc, char* argv[], 
			const char** result);

		virtual void GetStats(AgentPerformanceData* pStats);

	private:
		Agent* m_pAgent;
		std::string m_result;

		bool parse_system_stats(int argc, char *argv[]);
		bool parse_memory_stats(int argc, char *argv[]);
		bool parse_rete_stats(int argc, char *argv[]);
		void soar_ecPrintSystemStatistics();
		void soar_ecPrintReteStatistics();
		void soar_ecPrintMemoryStatistics();
		void soar_ecPrintMemoryPoolStatistics();
		void print_null_activation_stats();
		void printTimingInfo();

	};
}

#endif // GSKI_AGENTPERFORMANCEMONITOR_H
