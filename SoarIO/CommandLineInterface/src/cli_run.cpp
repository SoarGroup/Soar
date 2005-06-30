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

#include "cli_Constants.h"
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
		{'o', "output",			0},
		{'p', "phase",			0},
		{'s', "self",			0},
		{'u', "update",			0},
		{'n', "noupdate",		0},
		{0, 0, 0}
	};

	RunBitset options(0);

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

	// Count defaults to 0
	int count = 0;
	if (m_NonOptionArguments == 1) {
		int optind = m_Argument - m_NonOptionArguments;
		if (!IsInteger(argv[optind])) return SetError(CLIError::kIntegerExpected);
		count = atoi(argv[optind].c_str());
		if (count <= 0) return SetError(CLIError::kIntegerMustBePositive);
	} 

	return DoRun(pAgent, options, count);
}

bool CommandLineInterface::DoRun(gSKI::IAgent* pAgent, const RunBitset& options, int count) {
	if (!RequireAgent(pAgent)) return false;

	// Default run type is forever
	egSKIRunType runType = gSKI_RUN_FOREVER;
	// ... unless there is a count, then the default is a decision cycle:
	if (count) runType = gSKI_RUN_DECISION_CYCLE;

	// Override run type with option flag:
	if (options.test(RUN_ELABORATION)) {
		runType = gSKI_RUN_ELABORATION_CYCLE;

	} else if (options.test(RUN_PHASE)) {
		runType = gSKI_RUN_PHASE;

	} else if (options.test(RUN_DECISION)) {
		runType = gSKI_RUN_DECISION_CYCLE;

	} else if (options.test(RUN_OUTPUT)) {
		runType = gSKI_RUN_UNTIL_OUTPUT;

	} else if (options.test(RUN_FOREVER)) {
		runType = gSKI_RUN_FOREVER;	
	}

	if (!count && runType != gSKI_RUN_FOREVER) {
		count = 1;
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

	// Do the run
	runResult = pScheduler->RunScheduledAgents(runType, count, runFlags, &m_gSKIError) ;

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
			break;

		case gSKI_RUN_COMPLETED:
            // Do not print anything
			break;
		default:
			return SetError(CLIError::kgSKIError);
	}
	return true;
}
