/////////////////////////////////////////////////////////////////
// run command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "sml_Utils.h"
#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"
#include "sml_Names.h"
#include "sml_StringOps.h"
#include "sml_KernelSML.h"
#include "sml_Events.h"
#include "sml_RunScheduler.h"
#include "cli_CLIError.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseRun(std::vector<std::string>& argv) {
	Options optionsData[] = {
		{'d', "decision",		OPTARG_NONE},
		{'e', "elaboration",	OPTARG_NONE},
		{'f', "forever",		OPTARG_NONE},
		{'i', "interleave",		OPTARG_REQUIRED},
		{'n', "noupdate",		OPTARG_NONE},
		{'o', "output",			OPTARG_NONE},
		{'p', "phase",			OPTARG_NONE},
		{'s', "self",			OPTARG_NONE},
		{'u', "update",			OPTARG_NONE},
		{0, 0, OPTARG_NONE}
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

	return DoRun(options, count, interleaveMode);
}

eRunInterleaveMode CommandLineInterface::ParseRunInterleaveOptarg() {
	if (m_OptionArgument == "d") {
		return RUN_INTERLEAVE_DECISION;
	} else if (m_OptionArgument == "e") {
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

bool CommandLineInterface::DoRun(const RunBitset& options, int count, eRunInterleaveMode interleaveIn) {
	// Default run type is sml_DECISION
	smlRunStepSize runType = sml_DECISION;
	//// ... unless there is a count, then the default is a decision cycle:
	//if (count >= 0) runType = sml_DECISION;

	bool forever = false;

	// Override run type with option flag:
	if (options.test(RUN_ELABORATION)) {
		runType = sml_ELABORATION;

	} else if (options.test(RUN_PHASE)) {
		runType = sml_PHASE;

	} else if (options.test(RUN_DECISION)) {
		runType = sml_DECISION;

	} else if (options.test(RUN_OUTPUT)) {
		runType = sml_UNTIL_OUTPUT;
	} else {
		// if there is no step size given and no count, we're going forever
		forever = (count < 0);
	}

	if (count == -1)
	{
		count = 1 ;
	}

	smlRunResult runResult ;

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
		runFlags = (smlRunFlags)(runFlags | sml_RUN_SELF) ;

		// Schedule just this one agent to run
		pScheduler->ScheduleAllAgentsToRun(false) ;
		pScheduler->ScheduleAgentToRun(m_pAgentSML, true) ;
	}
	else
	{
		runFlags = (smlRunFlags)(runFlags | sml_RUN_ALL) ;

		// Ask all agents to run
		pScheduler->ScheduleAllAgentsToRun(true) ;
	}

	smlRunStepSize interleave;

	switch(interleaveIn) {
		case RUN_INTERLEAVE_DEFAULT:
		default:
			interleave  = pScheduler->DefaultInterleaveStepSize(forever, runType) ;
			break;
		case RUN_INTERLEAVE_ELABORATION:
			interleave = sml_ELABORATION;
			break;
		case RUN_INTERLEAVE_DECISION:
			interleave = sml_DECISION;
			break;
		case RUN_INTERLEAVE_PHASE:
			interleave = sml_PHASE;
			break;
		case RUN_INTERLEAVE_OUTPUT:
			interleave = sml_UNTIL_OUTPUT;
			break;
	}

	if (!pScheduler->VerifyStepSizeForRunType(forever, runType, interleave) ) {
		SetError(CLIError::kInvalidRunInterleaveSetting);
		SetErrorDetail("Run type and interleave setting incompatible.");
		return false;
	}

	// If we're running by decision cycle synchronize up the agents to the same phase before we start
	bool synchronizeAtStart = (runType == sml_DECISION) ; 

	SetTrapPrintCallbacks( false );

	// Do the run
	runResult = pScheduler->RunScheduledAgents(forever, runType, count, runFlags, interleave, synchronizeAtStart) ;

	SetTrapPrintCallbacks( true );

	// Check for error
	if (runResult == sml_RUN_ERROR) {
		// FIXME: report extended run error
        return SetError(CLIError::kRunFailed);
	} else if (runResult == sml_RUN_ERROR_ALREADY_RUNNING) {
		SetErrorDetail( "Soar is already running" );
        return SetError(CLIError::kRunFailed);
	}


	char buf[kMinBufferSize];
	switch (runResult) {
		case sml_RUN_ERROR:
			return SetError(CLIError::kRunFailed);
			break;

		case sml_RUN_EXECUTING:
			if (m_RawOutput) {
				// NOTE: I don't think this is currently possible
				m_Result << "\nRun stopped (still executing).";
			} else {
				AppendArgTagFast(sml_Names::kParamRunResult, sml_Names::kTypeInt, Int2String((int)runResult, buf, kMinBufferSize));
			}
			break;

		case sml_RUN_COMPLETED_AND_INTERRUPTED:					// an interrupt was requested, but the run completed first
			// falls through
		case sml_RUN_INTERRUPTED:
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

		case sml_RUN_COMPLETED:
            // Do not print anything
			// might be helpful if we checked agents to see if any halted...
			// retval is sml_RUN_COMPLETED, but agent m_RunState == gSKI_RUNSTATE_HALTED
			// should only check the agents m_pAgentSML->WasOnRunList()
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
			assert(false);
			return SetError(CLIError::kRunFailed);
	}
	return true;
}
