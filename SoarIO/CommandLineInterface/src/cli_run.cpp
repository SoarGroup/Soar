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
	egSKIRunResult runResult;
	egSKIRunState runState;

	// If we're running ourself, we need an agent pointer
	if (m_bSelf && !m_pAgent) {
		return;
	}

	// If we're running all agents, we need a kernel pointer
	if (!m_bSelf && !m_pKernel) {
		return;
	}

	// Loop until stopped or return
	while (!m_QuitNow) {
		if (m_bSelf) {
			// Running self, run a cycle
			runResult = m_pAgent->RunInClientThread(gSKI_RUN_DECISION_CYCLE, 1, m_pError);
			runState = m_pAgent->GetRunState();

			// Check if we're halted or interrupted
			if ((runState == gSKI_RUNSTATE_HALTED) || (runState == gSKI_RUNSTATE_INTERRUPTED)) {
				return;
			}

		} else {
			// Running all agents, run a cycle
			m_pKernel->GetAgentManager()->ClearAllInterrupts();
			m_pKernel->GetAgentManager()->AddAllAgentsToRunList();
			runResult = m_pKernel->GetAgentManager()->RunInClientThread(gSKI_RUN_DECISION_CYCLE, 1, gSKI_INTERLEAVE_SMALLEST_STEP, m_pError);

			// Walk list of agents and stop if any are halted/interrupted
			gSKI::tIAgentIterator* iter = m_pKernel->GetAgentManager()->GetAgentIterator(m_pError);
			while (iter->IsValid()) {
				gSKI::IAgent* pAgent = iter->GetVal();
				runState = pAgent->GetRunState();
				if ((runState == gSKI_RUNSTATE_HALTED) || (runState == gSKI_RUNSTATE_INTERRUPTED)) {
					return;
				}
				iter->Next() ;
			}
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

	// Count defaults to 0
	int count = 0;

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

	// Stop and reset the current thread
	if (m_pRunForever) {
		m_pRunForever->Stop(true);
		delete m_pRunForever;
		m_pRunForever = 0;
	}

	// Determine run unit, mutually exclusive so give smaller steps precedence, default to gSKI_RUN_FOREVER if there is no count
	egSKIRunType runType = gSKI_RUN_FOREVER;
	if (count) {
		runType = gSKI_RUN_DECISION_CYCLE;
	}
	if (options & OPTION_RUN_ELABORATION) {
		runType = gSKI_RUN_SMALLEST_STEP;

	} else if (options & OPTION_RUN_DECISION) {
		runType = gSKI_RUN_DECISION_CYCLE;

	} else if (options & OPTION_RUN_FOREVER) {
		runType = gSKI_RUN_FOREVER;	
	}

	if (runType == gSKI_RUN_FOREVER) {
		m_pRunForever = new RunForeverThread((options & OPTION_RUN_SELF) ? true : false, m_pKernel, pAgent, m_pError);
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
	return true;
}

