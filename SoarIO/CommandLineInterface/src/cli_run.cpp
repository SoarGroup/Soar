#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_GetOpt.h"
#include "cli_Constants.h"

#include "IgSKI_Agent.h"
#include "IgSKI_Kernel.h"
#include "IgSKI_AgentManager.h"

using namespace cli;

RunForeverThread::RunForeverThread(bool self, gSKI::IKernel* pKernel, gSKI::IAgent* pAgent, gSKI::Error* pError) {
	m_bSelf = self;
	m_pKernel = pKernel;
	m_pAgent = pAgent;
	m_pError = pError;
}

void RunForeverThread::Run() {
	while (!m_QuitNow) {
		egSKIRunResult runResult;
		if (m_bSelf) {
			runResult = m_pAgent->RunInClientThread(gSKI_RUN_DECISION_CYCLE, 1, m_pError);
		} else {
			m_pKernel->GetAgentManager()->ClearAllInterrupts();
			m_pKernel->GetAgentManager()->AddAllAgentsToRunList();
			runResult = m_pKernel->GetAgentManager()->RunInClientThread(gSKI_RUN_DECISION_CYCLE, 1, gSKI_INTERLEAVE_SMALLEST_STEP, m_pError);
		}
	}
}

bool CommandLineInterface::ParseRun(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	static struct GetOpt::option longOptions[] = {
		{"decision",	0, 0, 'd'},
		{"elaboration",	0, 0, 'e'},
		{"forever",		0, 0, 'f'},
		{"operator",	0, 0, 'o'},
		{"output",		0, 0, 'O'},
		{"phase",		0, 0, 'p'},
		{"self",		0, 0, 's'},
		{"state",		0, 0, 'S'},
		{0, 0, 0, 0}
	};

	GetOpt::optind = 0;
	GetOpt::opterr = 0;

	int option;
	unsigned int options = 0;

	for (;;) {
		option = m_pGetOpt->GetOpt_Long(argv, "defoOpsS", longOptions, 0);
		if (option == -1) {
			break;
		}

		switch (option) {
			case 'd':
				options |= OPTION_RUN_DECISION;
				break;
			case 'e':
				options |= OPTION_RUN_ELABORATION;
				break;
			case 'f':
				options |= OPTION_RUN_FOREVER;
				break;
			case 'o':
				options |= OPTION_RUN_OPERATOR;
				break;
			case 'O':
				options |= OPTION_RUN_OUTPUT;
				break;
			case 'p':
				options |= OPTION_RUN_PHASE;
				break;
			case 's':
				options |= OPTION_RUN_SELF;
				break;
			case 'S':
				options |= OPTION_RUN_STATE;
				break;
			case '?':
				return HandleSyntaxError(Constants::kCLIRun, Constants::kCLIUnrecognizedOption);
			default:
				return HandleGetOptError((char)option);
		}
	}

	// Count defaults to 1 (which are ignored if the options default, since they default to forever)
	int count = 1;

	// Only one non-option argument allowed, count
	if ((unsigned)GetOpt::optind == argv.size() - 1) {

		if (!IsInteger(argv[GetOpt::optind])) {
			return HandleSyntaxError(Constants::kCLIRun, "Count must be an integer.");
		}
		count = atoi(argv[GetOpt::optind].c_str());
		if (count <= 0) {
			return HandleSyntaxError(Constants::kCLIRun, "Count must be greater than 0.");
		}

	} else if ((unsigned)GetOpt::optind < argv.size()) {
		return HandleSyntaxError(Constants::kCLIRun, Constants::kCLITooManyArgs);
	}

	return DoRun(pAgent, options, count);
}

bool CommandLineInterface::DoRun(gSKI::IAgent* pAgent, const unsigned int options, int count) {
	// TODO: structured output

	if (!RequireAgent(pAgent)) return false;
	if (!RequireKernel()) return false;

	// TODO: Rather tricky options
	if ((options & OPTION_RUN_OPERATOR) || (options & OPTION_RUN_OUTPUT) || (options & OPTION_RUN_STATE)) {
		return HandleError("Options { o, O, S } not implemented yet.");
	}

	// Can't run if already running
	if (m_pRunForever) {
		return HandleError("Already running!");
	}

	// Determine run unit, mutually exclusive so give smaller steps precedence, default to gSKI_RUN_FOREVER
	egSKIRunType runType = gSKI_RUN_FOREVER;
	if (options & OPTION_RUN_ELABORATION) {
		runType = gSKI_RUN_SMALLEST_STEP;

	} else if (options & OPTION_RUN_DECISION) {
		runType = gSKI_RUN_DECISION_CYCLE;

	} else if (options & OPTION_RUN_FOREVER) {
		runType = gSKI_RUN_FOREVER;	
	}

	if (runType == gSKI_RUN_FOREVER) {
		m_pRunForever = new RunForeverThread(options & OPTION_RUN_SELF, m_pKernel, pAgent, m_pError);
		m_pRunForever->Start();
		return true;
	}

	// If running self, an agent pointer is necessary.  Otherwise, a Kernel pointer is necessary.
	egSKIRunResult runResult;
	if (options & OPTION_RUN_SELF) {
		runResult = pAgent->RunInClientThread(runType, count, m_pError);
	} else {
        m_pKernel->GetAgentManager()->ClearAllInterrupts();
        m_pKernel->GetAgentManager()->AddAllAgentsToRunList();
		runResult = m_pKernel->GetAgentManager()->RunInClientThread(runType, count, gSKI_INTERLEAVE_SMALLEST_STEP, m_pError);
	}

	// Check for error
	if (runResult == gSKI_RUN_ERROR) {
		AppendToResult("Run failed.");
		return false;	// Hopefully details are in gSKI error message
	}

	AppendToResult("\nRun successful: ");
	switch (runResult) {
		case gSKI_RUN_EXECUTING:
			AppendToResult("(gSKI_RUN_EXECUTING)");						// the run is still executing
			break;
		case gSKI_RUN_INTERRUPTED:
			AppendToResult("(gSKI_RUN_INTERRUPTED)");					// the run was interrupted
			break;
		case gSKI_RUN_COMPLETED:
			AppendToResult("(gSKI_RUN_COMPLETED)");						// the run completed normally
			break;
		case gSKI_RUN_COMPLETED_AND_INTERRUPTED:					// an interrupt was requested, but the run completed first
			AppendToResult("(gSKI_RUN_COMPLETED_AND_INTERRUPTED)");
			break;
		default:
			return HandleError("Unknown egSKIRunResult code returned.");
	}
	return true;
}

