/////////////////////////////////////////////////////////////////
// stats command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include <time.h>

#include "cli_CommandLineInterface.h"
#include "cli_CLIError.h"

#include "cli_Commands.h"
#include "sml_Names.h"
#include "sml_StringOps.h"
#include "sml_KernelSML.h"

#include "agent.h"
#include "utilities.h" // for timer_value
#include "print.h"
#include "rete.h" // for get_node_count_statistics

#include <iomanip>

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseStats(std::vector<std::string>& argv) {
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

	return DoStats(options);
}


bool CommandLineInterface::DoStats(const StatsBitset& options) {
	if (m_RawOutput) {
		if ( options.test(STATS_MEMORY) )
		{
			return SetError( CLIError::kNotImplemented );
		}
		if ( options.test(STATS_RETE) )
		{
			return SetError( CLIError::kNotImplemented );
		}
		if ( (!options.test(STATS_MEMORY) && !options.test(STATS_RETE)) || options.test(STATS_SYSTEM) )
		{
			GetSystemStats();
		}

	} else {
		return SetError( CLIError::kNotImplemented );
		//// structured output
		//gSKI::AgentPerformanceData stats;
		//pPerfMon->GetStats(&stats);

		//char buf[kMinBufferSize];

		//AppendArgTagFast(sml_Names::kParamStatsProductionCountDefault,				sml_Names::kTypeInt,	Int2String(stats.productionCountDefault, buf, kMinBufferSize));
		//AppendArgTagFast(sml_Names::kParamStatsProductionCountUser,					sml_Names::kTypeInt,	Int2String(stats.productionCountUser, buf, kMinBufferSize));
		//AppendArgTagFast(sml_Names::kParamStatsProductionCountChunk,				sml_Names::kTypeInt,	Int2String(stats.productionCountChunk, buf, kMinBufferSize));
		//AppendArgTagFast(sml_Names::kParamStatsProductionCountJustification,		sml_Names::kTypeInt,	Int2String(stats.productionCountJustification, buf, kMinBufferSize));
		//AppendArgTagFast(sml_Names::kParamStatsCycleCountDecision,					sml_Names::kTypeInt,	Int2String(stats.cycleCountDecision, buf, kMinBufferSize));
		//AppendArgTagFast(sml_Names::kParamStatsCycleCountElaboration,				sml_Names::kTypeInt,	Int2String(stats.cycleCountElaboration, buf, kMinBufferSize));
		//AppendArgTagFast(sml_Names::kParamStatsProductionFiringCount,				sml_Names::kTypeInt,	Int2String(stats.productionFiringCount, buf, kMinBufferSize));
		//AppendArgTagFast(sml_Names::kParamStatsWmeCountAddition,					sml_Names::kTypeInt,	Int2String(stats.wmeCountAddition, buf, kMinBufferSize));
		//AppendArgTagFast(sml_Names::kParamStatsWmeCountRemoval,						sml_Names::kTypeInt,	Int2String(stats.wmeCountRemoval, buf, kMinBufferSize));
		//AppendArgTagFast(sml_Names::kParamStatsWmeCount,							sml_Names::kTypeInt,	Int2String(stats.wmeCount, buf, kMinBufferSize));
		//AppendArgTagFast(sml_Names::kParamStatsWmeCountAverage,						sml_Names::kTypeDouble, Double2String(stats.wmeCountAverage, buf, kMinBufferSize));
		//AppendArgTagFast(sml_Names::kParamStatsWmeCountMax,							sml_Names::kTypeInt,	Int2String(stats.wmeCountMax, buf, kMinBufferSize));
		//AppendArgTagFast(sml_Names::kParamStatsKernelCPUTime,						sml_Names::kTypeDouble, Double2String(stats.kernelCPUTime, buf, kMinBufferSize));
		//AppendArgTagFast(sml_Names::kParamStatsTotalCPUTime,						sml_Names::kTypeDouble, Double2String(stats.totalCPUTime, buf, kMinBufferSize));
		//AppendArgTagFast(sml_Names::kParamStatsPhaseTimeInputPhase,					sml_Names::kTypeDouble, Double2String(stats.phaseTimeInputPhase, buf, kMinBufferSize)); 
		//AppendArgTagFast(sml_Names::kParamStatsPhaseTimeProposePhase,				sml_Names::kTypeDouble, Double2String(stats.phaseTimeProposePhase, buf, kMinBufferSize)); 
		//AppendArgTagFast(sml_Names::kParamStatsPhaseTimeDecisionPhase,				sml_Names::kTypeDouble, Double2String(stats.phaseTimeDecisionPhase, buf, kMinBufferSize));
		//AppendArgTagFast(sml_Names::kParamStatsPhaseTimeApplyPhase,					sml_Names::kTypeDouble, Double2String(stats.phaseTimeApplyPhase, buf, kMinBufferSize));  
		//AppendArgTagFast(sml_Names::kParamStatsPhaseTimeOutputPhase,				sml_Names::kTypeDouble, Double2String(stats.phaseTimeOutputPhase, buf, kMinBufferSize)); 
		//AppendArgTagFast(sml_Names::kParamStatsPhaseTimePreferencePhase,			sml_Names::kTypeDouble, Double2String(stats.phaseTimePreferencePhase, buf, kMinBufferSize)); 
		//AppendArgTagFast(sml_Names::kParamStatsPhaseTimeWorkingMemoryPhase,			sml_Names::kTypeDouble, Double2String(stats.phaseTimeWorkingMemoryPhase, buf, kMinBufferSize)); 
		//AppendArgTagFast(sml_Names::kParamStatsMonitorTimeInputPhase,				sml_Names::kTypeDouble, Double2String(stats.monitorTimeInputPhase, buf, kMinBufferSize));
		//AppendArgTagFast(sml_Names::kParamStatsMonitorTimeProposePhase,				sml_Names::kTypeDouble, Double2String(stats.monitorTimeProposePhase, buf, kMinBufferSize));
		//AppendArgTagFast(sml_Names::kParamStatsMonitorTimeDecisionPhase,			sml_Names::kTypeDouble, Double2String(stats.monitorTimeDecisionPhase, buf, kMinBufferSize));
		//AppendArgTagFast(sml_Names::kParamStatsMonitorTimeApplyPhase,				sml_Names::kTypeDouble, Double2String(stats.monitorTimeApplyPhase, buf, kMinBufferSize));
		//AppendArgTagFast(sml_Names::kParamStatsMonitorTimeOutputPhase,				sml_Names::kTypeDouble, Double2String(stats.monitorTimeOutputPhase, buf, kMinBufferSize));
		//AppendArgTagFast(sml_Names::kParamStatsMonitorTimePreferencePhase,			sml_Names::kTypeDouble, Double2String(stats.monitorTimePreferencePhase, buf, kMinBufferSize));
		//AppendArgTagFast(sml_Names::kParamStatsMonitorTimeWorkingMemoryPhase,		sml_Names::kTypeDouble, Double2String(stats.monitorTimeWorkingMemoryPhase, buf, kMinBufferSize));
		//AppendArgTagFast(sml_Names::kParamStatsInputFunctionTime,					sml_Names::kTypeDouble, Double2String(stats.inputFunctionTime, buf, kMinBufferSize));
		//AppendArgTagFast(sml_Names::kParamStatsOutputFunctionTime,					sml_Names::kTypeDouble, Double2String(stats.outputFunctionTime, buf, kMinBufferSize));	
		//AppendArgTagFast(sml_Names::kParamStatsMatchTimeInputPhase,					sml_Names::kTypeDouble, Double2String(stats.matchTimeInputPhase, buf, kMinBufferSize));
		//AppendArgTagFast(sml_Names::kParamStatsMatchTimePreferencePhase,			sml_Names::kTypeDouble, Double2String(stats.matchTimePreferencePhase, buf, kMinBufferSize));
		//AppendArgTagFast(sml_Names::kParamStatsMatchTimeWorkingMemoryPhase,			sml_Names::kTypeDouble, Double2String(stats.matchTimeWorkingMemoryPhase, buf, kMinBufferSize));
		//AppendArgTagFast(sml_Names::kParamStatsMatchTimeOutputPhase,				sml_Names::kTypeDouble, Double2String(stats.matchTimeOutputPhase, buf, kMinBufferSize));
		//AppendArgTagFast(sml_Names::kParamStatsMatchTimeDecisionPhase,				sml_Names::kTypeDouble, Double2String(stats.matchTimeDecisionPhase, buf, kMinBufferSize));
		//AppendArgTagFast(sml_Names::kParamStatsMatchTimeProposePhase,				sml_Names::kTypeDouble, Double2String(stats.matchTimeProposePhase, buf, kMinBufferSize));
		//AppendArgTagFast(sml_Names::kParamStatsMatchTimeApplyPhase,					sml_Names::kTypeDouble, Double2String(stats.matchTimeApplyPhase, buf, kMinBufferSize));
		//AppendArgTagFast(sml_Names::kParamStatsOwnershipTimeInputPhase,				sml_Names::kTypeDouble, Double2String(stats.ownershipTimeInputPhase, buf, kMinBufferSize));
		//AppendArgTagFast(sml_Names::kParamStatsOwnershipTimePreferencePhase,		sml_Names::kTypeDouble, Double2String(stats.ownershipTimePreferencePhase, buf, kMinBufferSize));
		//AppendArgTagFast(sml_Names::kParamStatsOwnershipTimeWorkingMemoryPhase,		sml_Names::kTypeDouble, Double2String(stats.ownershipTimeWorkingMemoryPhase, buf, kMinBufferSize));
		//AppendArgTagFast(sml_Names::kParamStatsOwnershipTimeOutputPhase,			sml_Names::kTypeDouble, Double2String(stats.ownershipTimeOutputPhase, buf, kMinBufferSize));
		//AppendArgTagFast(sml_Names::kParamStatsOwnershipTimeDecisionPhase,			sml_Names::kTypeDouble, Double2String(stats.ownershipTimeDecisionPhase, buf, kMinBufferSize));
		//AppendArgTagFast(sml_Names::kParamStatsOwnershipTimeProposePhase,			sml_Names::kTypeDouble, Double2String(stats.ownershipTimeProposePhase, buf, kMinBufferSize));
		//AppendArgTagFast(sml_Names::kParamStatsOwnershipTimeApplyPhase,				sml_Names::kTypeDouble, Double2String(stats.ownershipTimeApplyPhase, buf, kMinBufferSize));
		//AppendArgTagFast(sml_Names::kParamStatsChunkingTimeInputPhase,				sml_Names::kTypeDouble, Double2String(stats.chunkingTimeInputPhase, buf, kMinBufferSize));
		//AppendArgTagFast(sml_Names::kParamStatsChunkingTimePreferencePhase,			sml_Names::kTypeDouble, Double2String(stats.chunkingTimePreferencePhase, buf, kMinBufferSize));
		//AppendArgTagFast(sml_Names::kParamStatsChunkingTimeWorkingMemoryPhase,		sml_Names::kTypeDouble, Double2String(stats.chunkingTimeWorkingMemoryPhase, buf, kMinBufferSize));
		//AppendArgTagFast(sml_Names::kParamStatsChunkingTimeOutputPhase,				sml_Names::kTypeDouble, Double2String(stats.chunkingTimeOutputPhase, buf, kMinBufferSize));
		//AppendArgTagFast(sml_Names::kParamStatsChunkingTimeDecisionPhase,			sml_Names::kTypeDouble, Double2String(stats.chunkingTimeDecisionPhase, buf, kMinBufferSize));
		//AppendArgTagFast(sml_Names::kParamStatsChunkingTimeProposePhase,			sml_Names::kTypeDouble, Double2String(stats.chunkingTimeProposePhase, buf, kMinBufferSize));
		//AppendArgTagFast(sml_Names::kParamStatsChunkingTimeApplyPhase,				sml_Names::kTypeDouble, Double2String(stats.chunkingTimeApplyPhase, buf, kMinBufferSize));
		//AppendArgTagFast(sml_Names::kParamStatsMemoryUsageMiscellaneous,			sml_Names::kTypeInt,	Int2String(stats.memoryUsageMiscellaneous, buf, kMinBufferSize));
		//AppendArgTagFast(sml_Names::kParamStatsMemoryUsageHash,						sml_Names::kTypeInt,	Int2String(stats.memoryUsageHash, buf, kMinBufferSize));
		//AppendArgTagFast(sml_Names::kParamStatsMemoryUsageString,					sml_Names::kTypeInt,	Int2String(stats.memoryUsageString, buf, kMinBufferSize));
		//AppendArgTagFast(sml_Names::kParamStatsMemoryUsagePool,						sml_Names::kTypeInt,	Int2String(stats.memoryUsagePool, buf, kMinBufferSize));
		//AppendArgTagFast(sml_Names::kParamStatsMemoryUsageStatsOverhead,			sml_Names::kTypeInt,	Int2String(stats.memoryUsageStatsOverhead, buf, kMinBufferSize));
	}
	return true;
}

void CommandLineInterface::GetSystemStats()
{

	// Set precision now, RESET BEFORE RETURN
	size_t oldPrecision = m_Result.precision(3);
	m_Result << std::setiosflags( std::ios_base::fixed );

	// Hostname
	char hostname[256];
	memset( hostname, 0, 256 );
	if ( gethostname( hostname, 255 ) == SOCKET_ERROR )
	{
		strncpy( hostname, "[host name unknown]", 255 );
	}

	// Time
	time_t current_time = time(NULL);

	double total_kernel_time = timer_value( &m_pAgentSoar->total_kernel_time );
	double total_kernel_msec = total_kernel_time * 1000.0;

	/* derived_kernel_time := Total of the time spent in the phases of the decision cycle, 
	excluding Input Function, Output function, and pre-defined callbacks. 
	This computed time should be roughly equal to total_kernel_time, 
	as determined above. */

	double derived_kernel_time = timer_value(&m_pAgentSoar->decision_cycle_phase_timers[INPUT_PHASE] )
		+ timer_value( &m_pAgentSoar->decision_cycle_phase_timers[PROPOSE_PHASE] )
		+ timer_value( &m_pAgentSoar->decision_cycle_phase_timers[APPLY_PHASE] )
		+ timer_value( &m_pAgentSoar->decision_cycle_phase_timers[PREFERENCE_PHASE] )
		+ timer_value( &m_pAgentSoar->decision_cycle_phase_timers[WM_PHASE] )
		+ timer_value( &m_pAgentSoar->decision_cycle_phase_timers[OUTPUT_PHASE] )
		+ timer_value( &m_pAgentSoar->decision_cycle_phase_timers[DECISION_PHASE] );

	double input_function_time = timer_value(&m_pAgentSoar->input_function_cpu_time);
	double output_function_time = timer_value(&m_pAgentSoar->output_function_cpu_time);

	/* Total of the time spent in callback routines. */
	double monitors_sum = timer_value(&m_pAgentSoar->monitors_cpu_time[INPUT_PHASE])
		+ timer_value(&m_pAgentSoar->monitors_cpu_time[PROPOSE_PHASE])
		+ timer_value(&m_pAgentSoar->monitors_cpu_time[APPLY_PHASE])
		+ timer_value(&m_pAgentSoar->monitors_cpu_time[PREFERENCE_PHASE])
		+ timer_value(&m_pAgentSoar->monitors_cpu_time[WM_PHASE])
		+ timer_value(&m_pAgentSoar->monitors_cpu_time[OUTPUT_PHASE])
		+ timer_value(&m_pAgentSoar->monitors_cpu_time[DECISION_PHASE]);

	double derived_total_cpu_time = derived_kernel_time + monitors_sum + input_function_time + output_function_time;

	/* Total time spent in the input phase */
	double input_phase_total_time = timer_value(&m_pAgentSoar->decision_cycle_phase_timers[INPUT_PHASE])
		+ timer_value(&m_pAgentSoar->monitors_cpu_time[INPUT_PHASE])
		+ timer_value(&m_pAgentSoar->input_function_cpu_time);

	/* Total time spent in the propose phase */
	double propose_phase_total_time = timer_value(&m_pAgentSoar->decision_cycle_phase_timers[PROPOSE_PHASE])
		+ timer_value(&m_pAgentSoar->monitors_cpu_time[PROPOSE_PHASE]);

	/* Total time spent in the apply phase */
	double apply_phase_total_time = timer_value(&m_pAgentSoar->decision_cycle_phase_timers[APPLY_PHASE])
		+ timer_value(&m_pAgentSoar->monitors_cpu_time[APPLY_PHASE]);

	/* Total time spent in the output phase */
	double output_phase_total_time = timer_value(&m_pAgentSoar->decision_cycle_phase_timers[OUTPUT_PHASE])
		+ timer_value(&m_pAgentSoar->monitors_cpu_time[OUTPUT_PHASE])
		+ timer_value(&m_pAgentSoar->output_function_cpu_time);

	/* Total time spent in the decision phase */
	double decision_phase_total_time = timer_value(&m_pAgentSoar->decision_cycle_phase_timers[DECISION_PHASE])
		+ timer_value(&m_pAgentSoar->monitors_cpu_time[DECISION_PHASE]);

	/* The sum of these phase timers is exactly equal to the 
	* derived_total_cpu_time
	*/

	m_Result << "Soar " << sml_Names::kSoarVersionValue << " on " << hostname << " at" << ctime(&current_time) << "\n";

	unsigned long totalProductions = m_pAgentSoar->num_productions_of_type[DEFAULT_PRODUCTION_TYPE];
	totalProductions += m_pAgentSoar->num_productions_of_type[USER_PRODUCTION_TYPE];
	totalProductions += m_pAgentSoar->num_productions_of_type[CHUNK_PRODUCTION_TYPE];

	m_Result << totalProductions << " productions ("
		<< m_pAgentSoar->num_productions_of_type[DEFAULT_PRODUCTION_TYPE] << " default, "
		<< m_pAgentSoar->num_productions_of_type[USER_PRODUCTION_TYPE] << " user, "
		<< m_pAgentSoar->num_productions_of_type[CHUNK_PRODUCTION_TYPE] << " chunks)\n";

	m_Result << "   + " << m_pAgentSoar->num_productions_of_type[CHUNK_PRODUCTION_TYPE] << " justifications\n";

	/* The fields for the timers are 8.3, providing an upper limit of 
	approximately 2.5 hours the printing of the run time calculations.  
	Obviously, these will need to be increased if you plan on needing 
	run-time data for a process that you expect to take longer than 
	2 hours. :) */

	if ( m_pAgentSoar->operand2_mode ) {
		m_Result << "                                                        |   Computed\n";
		m_Result << "Phases:      Input   Propose   Decide   Apply    Output |     Totals\n";
		m_Result << "========================================================|===========\n";

		m_Result << "Kernel:   "
			<< std::setw(8)  << timer_value( &m_pAgentSoar->decision_cycle_phase_timers[INPUT_PHASE] ) << " "
			<< std::setw(8)  << timer_value( &m_pAgentSoar->decision_cycle_phase_timers[PROPOSE_PHASE] ) << " "
			<< std::setw(8)  << timer_value( &m_pAgentSoar->decision_cycle_phase_timers[DECISION_PHASE] ) << " "
			<< std::setw(8)  << timer_value( &m_pAgentSoar->decision_cycle_phase_timers[APPLY_PHASE] ) << " "
			<< std::setw(8)  << timer_value( &m_pAgentSoar->decision_cycle_phase_timers[OUTPUT_PHASE] ) << "  | "
			<< std::setw(10) << derived_kernel_time << "\n";

	} else { 
		m_Result << "                                                        |   Computed\n";
		m_Result << "Phases:      Input      Pref      W/M   Output Decision |     Totals\n";
		m_Result << "========================================================|===========\n";

		m_Result << "Kernel:   "
			<< std::setw(8) << timer_value( &m_pAgentSoar->decision_cycle_phase_timers[INPUT_PHASE] ) << " "
			<< std::setw(8) << timer_value( &m_pAgentSoar->decision_cycle_phase_timers[PREFERENCE_PHASE] ) << " "
			<< std::setw(8) << timer_value( &m_pAgentSoar->decision_cycle_phase_timers[WM_PHASE] ) << " "
			<< std::setw(8) << timer_value( &m_pAgentSoar->decision_cycle_phase_timers[OUTPUT_PHASE] ) << " "
			<< std::setw(8) << timer_value( &m_pAgentSoar->decision_cycle_phase_timers[DECISION_PHASE] ) << " | "
			<< std::setw(10) << derived_kernel_time << "\n";
	}

	m_Result << "========================================================|===========\n";

	m_Result << "Input fn: "
		<< std::setw(8) << input_function_time << "                                      | "
		<< std::setw(10) << input_function_time << "\n";

	m_Result << "========================================================|===========\n";
	m_Result << "Outpt fn:                                     "
		<< std::setw(8) << output_function_time << "  | "
		<< std::setw(10) << output_function_time << "\n";

	m_Result << "========================================================|===========\n";

	m_Result << "Callbcks: "
		<< std::setw(8) << timer_value( &m_pAgentSoar->monitors_cpu_time[INPUT_PHASE] ) << " "
		<< std::setw(8) << timer_value( &m_pAgentSoar->monitors_cpu_time[PROPOSE_PHASE] ) << " "
		<< std::setw(8) << timer_value( &m_pAgentSoar->monitors_cpu_time[DECISION_PHASE] ) << " "
		<< std::setw(8) << timer_value( &m_pAgentSoar->monitors_cpu_time[APPLY_PHASE] ) << " "
		<< std::setw(8) << timer_value( &m_pAgentSoar->monitors_cpu_time[OUTPUT_PHASE] ) << "  | "
		<< std::setw(10) << monitors_sum << "\n";

	m_Result << "========================================================|===========\n";
	m_Result << "Computed------------------------------------------------+-----------\n";
	m_Result << "Totals:   %8.3f %8.3f %8.3f %8.3f %8.3f  | %10.3f\n\n",
	m_Result << "Totals:   "
		<< std::setw(8) << input_phase_total_time << " "
		<< std::setw(8) << propose_phase_total_time << " "
		<< std::setw(8) << decision_phase_total_time << " "
		<< std::setw(8) << apply_phase_total_time << " "
		<< std::setw(8) << output_phase_total_time << "  | "
		<< std::setw(10) << derived_total_cpu_time << "\n\n";

	m_Result << "Values from single timers:\n";

	m_Result << " Kernel CPU Time: "
		<< std::setw(11) << total_kernel_time << " sec. \n";

	m_Result << " Total  CPU Time: "
		<< std::setw(11) << timer_value(&m_pAgentSoar->total_cpu_time) << " sec.\n\n";

	///* v8.6.2: print out decisions executed, not # full cycles */

	m_Result << m_pAgentSoar->decision_phases_count << " decisions ("
		<< (m_pAgentSoar->decision_phases_count ? total_kernel_msec / m_pAgentSoar->decision_phases_count : 0.0)
		<< " msec/decision)\n";
	m_Result << m_pAgentSoar->e_cycle_count << " elaboration cycles ("
		<< (m_pAgentSoar->decision_phases_count ? (double) m_pAgentSoar->e_cycle_count / m_pAgentSoar->decision_phases_count : 0)
		<< " ec's per dc, "
		<< (m_pAgentSoar->e_cycle_count ? total_kernel_msec / m_pAgentSoar->e_cycle_count : 0)
		<< " msec/ec)\n";

	if ( m_pAgentSoar->operand2_mode ) {

		m_Result << m_pAgentSoar->pe_cycle_count << " p-elaboration cycles ("
			<< (m_pAgentSoar->decision_phases_count ? (double) m_pAgentSoar->pe_cycle_count / m_pAgentSoar->decision_phases_count : 0) 
			<< " pe's per dc, "
			<< (m_pAgentSoar->pe_cycle_count ? total_kernel_msec / m_pAgentSoar->pe_cycle_count : 0)
			<< " msec/pe)\n";
	}

	m_Result << m_pAgentSoar->production_firing_count << " production firings ("
		<< (m_pAgentSoar->e_cycle_count ? (double) m_pAgentSoar->production_firing_count / m_pAgentSoar->e_cycle_count : 0.0)
		<< " pf's per ec, "
		<< (m_pAgentSoar->production_firing_count ? total_kernel_msec / m_pAgentSoar->production_firing_count : 0.0) 
		<< " msec/pf)\n";

	unsigned long wme_changes = m_pAgentSoar->wme_addition_count + m_pAgentSoar->wme_removal_count;
	m_Result << wme_changes << " wme changes ("
		<< m_pAgentSoar->wme_addition_count << " additions, "
		<< m_pAgentSoar->wme_removal_count << " removals)\n";

	m_Result << "WM size: "
		<< m_pAgentSoar->num_wmes_in_rete << " current, "
		<< (m_pAgentSoar->num_wm_sizes_accumulated ? (m_pAgentSoar->cumulative_wm_size / m_pAgentSoar->num_wm_sizes_accumulated) : 0.0) 
		<< " mean, "
		<< m_pAgentSoar->max_wm_size << " maximum\n";

	m_Result << "\n";
	m_Result << "    *** Time/<x> statistics use the total kernel time from a    ***\n";
	m_Result << "    *** single kernel timer.  Differences between this value    ***\n";
	m_Result << "    *** and the computed total kernel time  are expected. See   ***\n";
	m_Result << "    *** help  for the  stats command  for more  information.    ***\n";

	m_Result << std::resetiosflags( std::ios_base::fixed );
	m_Result.precision( oldPrecision );
}

