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

#include "cli_GetOpt.h"
#include "cli_Constants.h"
#include "sml_Names.h"
#include "sml_StringOps.h"
#include "sml_KernelSML.h"
#include "sml_RunScheduler.h"

#include "IgSKI_Agent.h"
#include "IgSKI_Kernel.h"
#include "IgSKI_AgentManager.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseRun(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	static struct GetOpt::option longOptions[] = {
		{"decision",	0, 0, 'd'},
		{"elaboration",	0, 0, 'e'},
		{"forever",		0, 0, 'f'},
		{"output",		0, 0, 'o'},
		{"phase",		0, 0, 'p'},
		{"self",		0, 0, 's'},
		{0, 0, 0, 0}
	};

	RunBitset options(0);

	for (;;) {
		int option = m_pGetOpt->GetOpt_Long(argv, "defops", longOptions, 0);
		if (option == -1) break;

		switch (option) {
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
			case '?':
				{
					std::string detail;
					if (m_pGetOpt->GetOptOpt()) {
						detail = static_cast<char>(m_pGetOpt->GetOptOpt());
					} else {
						detail = argv[m_pGetOpt->GetOptind() - 1];
					}
					SetErrorDetail("Bad option '" + detail + "'.");
				}
				return SetError(CLIError::kUnrecognizedOption);
			default:
				return SetError(CLIError::kGetOptError);
		}
	}

	// Only one non-option argument allowed, count
	if (m_pGetOpt->GetAdditionalArgCount() > 1) return SetError(CLIError::kTooManyArgs);

	// Count defaults to 0
	int count = 0;
	if (m_pGetOpt->GetAdditionalArgCount() == 1) {
		int optind = m_pGetOpt->GetOptind();
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
		runType = gSKI_RUN_SMALLEST_STEP;

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

// This flag determines whether we use the existing gSKI scheduler or switch to the new SML scheduler.
// The flag should be removed once we've decided which scheduler we're using.
#define USE_SML_SCHEDULER

#ifdef USE_SML_SCHEDULER
	RunScheduler* pScheduler = m_pKernelSML->GetRunScheduler() ;

	if (options.test(RUN_SELF))
	{
		AgentSML* pAgentSML = m_pKernelSML->GetAgentSML(pAgent) ;

		// Schedule just this one agent to run
		pScheduler->ScheduleAllAgentsToRun(false) ;
		pScheduler->ScheduleAgentToRun(pAgentSML, true) ;
	}
	else
	{
		// Ask all agents to run
		pScheduler->ScheduleAllAgentsToRun(true) ;
	}

	// Do the run
	runResult = pScheduler->RunScheduledAgents(runType, count, 0, &m_gSKIError) ;

#else // USE_SML_SCHEDULER
	// If running self, an agent pointer is necessary.  Otherwise, a Kernel pointer is necessary.
	// Decide which agents to run
	if (options.test(RUN_SELF)) {
		m_pKernel->GetAgentManager()->RemoveAllAgentsFromRunList() ;
		m_pKernel->GetAgentManager()->AddAgentToRunList(pAgent, &m_gSKIError) ;
		if (gSKI::isError(m_gSKIError)) {
			SetErrorDetail("Error adding agent to run list.");
			return SetError(CLIError::kgSKIError);
		}
	} else {
        m_pKernel->GetAgentManager()->AddAllAgentsToRunList();
	}

	// Do the run
    m_pKernel->GetAgentManager()->ClearAllInterrupts();
	runResult = m_pKernel->GetAgentManager()->RunInClientThread(runType, count, gSKI_INTERLEAVE_SMALLEST_STEP, &m_gSKIError);
#endif // USE_SML_SCHEDULER

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
