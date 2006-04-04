/////////////////////////////////////////////////////////////////
// run command file.
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
#include "sml_KernelSML.h"
#include "sml_ClientEvents.h"
#include "sml_RunScheduler.h"

#include "IgSKI_Agent.h"
#include "IgSKI_Kernel.h"
#include "IgSKI_AgentManager.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseRun(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	Options optionsData[] = {
		{'d', "decision",		0},
		{'e', "elaboration",	0},
		{'f', "forever",		0},
		{'i', "interleave",		1},
		{'n', "noupdate",		0},
		{'o', "output",			0},
		{'p', "phase",			0},
		{'s', "self",			0},
		{'u', "update",			0},
		{0, 0, 0}
	};

	RunBitset options(0);
	eRunInterleaveMode interleaveMode = RUN_INTERLEAVE_DEFAULT;

	for (;;) {
		if (!ProcessOptions(argv, optionsData)) return false;
		if (m_Option == -1) break;

		switch (m_Option) {
			case 'd':
				options.set(RUN_DECISION);
				break;
			case 'e':
				options.set(RUN_ELABORATION);
				break;
			case 'f':
				options.set(RUN_FOREVER);
				break;
			case 'i':
				options.set(RUN_INTERLEAVE);
				interleaveMode = ParseRunInterleaveOptarg();
				if (interleaveMode == RUN_INTERLEAVE_DEFAULT) {
					return false; // error set in parse function
				}
				break;
			case 'o':
				options.set(RUN_OUTPUT);
				break;
			case 'p':
				options.set(RUN_PHASE);
				break;
			case 's':
				options.set(RUN_SELF);
				break;
			case 'u':
				options.set(RUN_UPDATE) ;
				break ;
			case 'n':
				options.set(RUN_NO_UPDATE) ;
				break ;
			default:
				return SetError(CLIError::kGetOptError);
		}
	}

	// Only one non-option argument allowed, count
	if (m_NonOptionArguments > 1) return SetError(CLIError::kTooManyArgs);

	// Decide if we explicitly indicated how to run
	bool specifiedType = (options.test(RUN_FOREVER) || options.test(RUN_ELABORATION) || options.test(RUN_DECISION) || options.test(RUN_PHASE) || options.test(RUN_OUTPUT)) ;

	// Count defaults to -1
	int count = -1;
	if (m_NonOptionArguments == 1) {
		int optind = m_Argument - m_NonOptionArguments;
		if (!IsInteger(argv[optind])) return SetError(CLIError::kIntegerExpected);
		count = atoi(argv[optind].c_str());
		// Allow "run 0" for decisions -- which means run agents to the current stop-before phase
		if (count < 0 || (count == 0 && specifiedType && !options.test(RUN_DECISION))) return SetError(CLIError::kIntegerMustBePositive);
	} 

	return DoRun(pAgent, options, count, interleaveMode);
}

eRunInterleaveMode CommandLineInterface::ParseRunInterleaveOptarg() {
	if (m_OptionArgument == "e") {
		return RUN_INTERLEAVE_ELABORATION;
	} else if (m_OptionArgument == "o") {
		return RUN_INTERLEAVE_OUTPUT;
	} else if (m_OptionArgument == "p") {
		return RUN_INTERLEAVE_PHASE;
	}

	SetErrorDetail("Invalid switch: " + m_OptionArgument);
	SetError(CLIError::kInvalidRunInterleaveSetting);
	return RUN_INTERLEAVE_DEFAULT;
}

bool CommandLineInterface::DoRun(gSKI::IAgent* pAgent, const RunBitset& options, int count, eRunInterleaveMode interleaveIn) {
	if (!RequireAgent(pAgent)) return false;

	// Default run type is forever
	egSKIRunType runType = gSKI_RUN_FOREVER;
	// ... unless there is a count, then the default is a decision cycle:
	if (count >= 0) runType = gSKI_RUN_DECISION_CYCLE;

	// Override run type with option flag:
	if (options.test(RUN_ELABORATION)) {
		runType = gSKI_RUN_ELABORATION_CYCLE;

	} else if (options.test(RUN_PHASE)) {
		runType = gSKI_RUN_PHASE;

	} else if (options.test(RUN_DECISION)) {
		runType = gSKI_RUN_DECISION_CYCLE;

	} else if (options.test(RUN_OUTPUT)) {
		runType = gSKI_RUN_UNTIL_OUTPUT;
	}  
	if (options.test(RUN_FOREVER)) {
		runType = gSKI_RUN_FOREVER;	
	}

	// if -f (run_forever) && -e | -d | -p | -o, then use the other
	// args to set the interleave for running forever.
	egSKIInterleaveType interleave = gSKI_INTERLEAVE_SMALLEST_STEP; // need to set a default based on RunType
	if (options.test(RUN_FOREVER)) {
		if (options.test(RUN_ELABORATION)) interleave = gSKI_INTERLEAVE_ELABORATION_PHASE;
		if (options.test(RUN_PHASE))    interleave = gSKI_INTERLEAVE_PHASE;
        if (options.test(RUN_DECISION)) interleave = gSKI_INTERLEAVE_PHASE;
		if (options.test(RUN_OUTPUT))   interleave = gSKI_INTERLEAVE_OUTPUT;
	}
 
	if (count == -1)
	{
		count = 1 ;
	}

	egSKIRunResult runResult ;

	// NOTE: We use a scheduler implemented in kernelSML rather than
	// the gSKI scheduler implemented by AgentManager.  This gives us
	// more flexibility to adjust the behavior of the SML scheduler without
	// impacting SoarTech systems that may rely on the gSKI scheduler.
	RunScheduler* pScheduler = m_pKernelSML->GetRunScheduler() ;
	smlRunFlags runFlags = sml_NONE ;

	if (options.test(RUN_UPDATE))
		runFlags = sml_UPDATE_WORLD ;
	else if (options.test(RUN_NO_UPDATE))
		runFlags = sml_DONT_UPDATE_WORLD ;

	if (options.test(RUN_SELF))
	{
		AgentSML* pAgentSML = m_pKernelSML->GetAgentSML(pAgent) ;
		runFlags = (smlRunFlags)(runFlags | sml_RUN_SELF) ;

		// Schedule just this one agent to run
		pScheduler->ScheduleAllAgentsToRun(false) ;
		pScheduler->ScheduleAgentToRun(pAgentSML, true) ;
	}
	else
	{
		runFlags = (smlRunFlags)(runFlags | sml_RUN_ALL) ;

		// Ask all agents to run
		pScheduler->ScheduleAllAgentsToRun(true) ;
	}

	// Decide how large of a step to run each agent before switching to the next agent
	// By default, we run one phase per agent but this isn't always appropriate.
	egSKIRunType interleaveStepSize = gSKI_RUN_PHASE ;
#ifdef USE_NEW_SCHEDULER
 
	switch(interleaveIn) {
		case RUN_INTERLEAVE_DEFAULT:
		default:
			interleave  = pScheduler->DefaultInterleaveStepSize(runType) ;
			break;
		case RUN_INTERLEAVE_ELABORATION:
			interleave = gSKI_INTERLEAVE_ELABORATION_PHASE;
			break;
		case RUN_INTERLEAVE_PHASE:
			interleave = gSKI_INTERLEAVE_PHASE;
			break;
		case RUN_INTERLEAVE_OUTPUT:
			interleave = gSKI_INTERLEAVE_OUTPUT;
			break;
	}

#endif

	//interleaveStepSize is left over from the old SML scheduler and should be removed.
	switch (runType)
	{
		// If the entire system is running by elaboration cycles, then we need to run each agent by elaboration cycles (they're usually
		// smaller than a phase).
		case gSKI_RUN_ELABORATION_CYCLE: interleaveStepSize = gSKI_RUN_ELABORATION_CYCLE ; break ;

		// If we're running the system to output we want to run each agent until it generates output.  This can be many decisions.
		// The reason is actually to do with stopping the agent after n decisions (default 15) if no output occurs.
		// DJP -- We need to rethink this design so using phase interleaving until we do.
		// case gSKI_RUN_UNTIL_OUTPUT: interleaveStepSize = gSKI_RUN_UNTIL_OUTPUT ; break ;

		default: interleaveStepSize = gSKI_RUN_PHASE ; break ;
	}

#ifdef USE_NEW_SCHEDULER
	if (!pScheduler->VerifyStepSizeForRunType( runType, interleave) ) {
		SetError(CLIError::kInvalidRunInterleaveSetting);
		SetErrorDetail("Run type and interleave setting incompatible.");
		return false;
	}
#endif

	// If we're running by decision cycle synchronize up the agents to the same phase before we start
	bool synchronizeAtStart = (runType == gSKI_RUN_DECISION_CYCLE) ;

	// Do the run
#ifdef USE_OLD_SCHEDULER
	runResult = pScheduler->RunScheduledAgents(runType, count, runFlags, interleaveStepSize, synchronizeAtStart, &m_gSKIError) ;
#endif
#ifdef USE_NEW_SCHEDULER
	runResult = pScheduler->RunScheduledAgents(runType, count, runFlags, interleave, synchronizeAtStart, &m_gSKIError) ;
#endif

	// Check for error
	if (runResult == gSKI_RUN_ERROR) {
        if (m_gSKIError.Id == gSKI::gSKIERR_AGENT_RUNNING) {
            return SetError(CLIError::kAlreadyRunning);
        } else if (gSKI::isError(m_gSKIError)) {
		    return SetError(CLIError::kgSKIError);
	    }
        return SetError(CLIError::kRunFailed);
	}


	char buf[kMinBufferSize];
	switch (runResult) {
		case gSKI_RUN_EXECUTING:
			if (m_RawOutput) {
				// NOTE: I don't think this is currently possible
				m_Result << "\nRun stopped (still executing).";
			} else {
				AppendArgTagFast(sml_Names::kParamRunResult, sml_Names::kTypeInt, Int2String((int)runResult, buf, kMinBufferSize));
			}
			break;

		case gSKI_RUN_COMPLETED_AND_INTERRUPTED:					// an interrupt was requested, but the run completed first
			// falls through
		case gSKI_RUN_INTERRUPTED:
			if (m_RawOutput) {
				m_Result << "\nRun stopped (interrupted).";
			} else {
				AppendArgTagFast(sml_Names::kParamRunResult, sml_Names::kTypeInt, Int2String((int)runResult, buf, kMinBufferSize));
			}
			if (pScheduler->AnAgentHaltedDuringRun())
			{
				if (m_RawOutput) {
					m_Result << "\nAn agent halted during the run.";
				} else {                    
					AppendArgTagFast(sml_Names::kParamMessage, sml_Names::kTypeString, "\nAn agent halted during the run.");		
			    }
			}
			break;

		case gSKI_RUN_COMPLETED:
            // Do not print anything
			// might be helpful if we checked agents to see if any halted...
			// retval is gSKI_RUN_COMPLETED, but agent m_RunState == gSKI_RUNSTATE_HALTED
			// should only check the agents pAgentSML->WasOnRunList()
			if (pScheduler->AnAgentHaltedDuringRun())
			{
				if (m_RawOutput) {
					m_Result << "\nAn agent halted during the run.";
				} else {                    
					AppendArgTagFast(sml_Names::kParamMessage, sml_Names::kTypeString, "\nAn agent halted during the run.");		
			    }
			}
			break;

		default:
			return SetError(CLIError::kgSKIError);
	}
	return true;
}
