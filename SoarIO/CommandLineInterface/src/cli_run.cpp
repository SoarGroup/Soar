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

RunThread::RunThread(int count, egSKIRunType runType, bool self, gSKI::IKernel* pKernel, gSKI::IAgent* pAgent, gSKI::Error* pError) {
	// if count comes in as zero, set to negative (run forever)
	if (count) {
		m_Count = count;
		m_RunType = runType;
	} else {
		m_Count = -1;
		m_RunType = gSKI_RUN_DECISION_CYCLE;
	}
	m_bSelf = self;
	m_pKernel = pKernel;
	m_pAgent = pAgent;
	m_pError = pError;
}

void RunThread::Run() {
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

	// Loop until stopped (this->Stop()), count reduced to 0, or halted/interrupted
	while (!m_QuitNow) {
		if (m_bSelf) {
			// Running self, run a cycle
			runResult = m_pAgent->RunInClientThread(m_RunType, 1, m_pError);
			runState = m_pAgent->GetRunState();

			// Check if we're halted or interrupted
			if ((runState == gSKI_RUNSTATE_HALTED) || (runState == gSKI_RUNSTATE_INTERRUPTED)) {
				return;
			}

		} else {
			// Running all agents, run a cycle
			m_pKernel->GetAgentManager()->ClearAllInterrupts();
			m_pKernel->GetAgentManager()->AddAllAgentsToRunList();
			runResult = m_pKernel->GetAgentManager()->RunInClientThread(m_RunType, 1, gSKI_INTERLEAVE_SMALLEST_STEP, m_pError);

			// Walk list of agents and stop if any are halted/interrupted
			gSKI::tIAgentIterator* iter = m_pKernel->GetAgentManager()->GetAgentIterator(m_pError);
			gSKI::IAgent* pAgent;
			while (iter->IsValid()) {
				pAgent = iter->GetVal();
				runState = pAgent->GetRunState();
				if ((runState == gSKI_RUNSTATE_HALTED) || (runState == gSKI_RUNSTATE_INTERRUPTED)) {
					return;
				}
				iter->Next() ;
			}
		}

		// If the count is positive, decrement it and return if that makes it zero
		// If the count is negative, loop forever
		if (m_Count > 0) {
			if (!(--m_Count)) {
				return;
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

	// Count defaults to 0 (forever)
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

	// Are we halted?
	egSKIRunState runState;
	if (options & OPTION_RUN_SELF) {
		runState = pAgent->GetRunState();
		if (runState == gSKI_RUNSTATE_HALTED) {
			return HandleError("System halted (try 'init-soar').");
		}

	} else {
		gSKI::tIAgentIterator* iter = m_pKernel->GetAgentManager()->GetAgentIterator(m_pError);
		gSKI::IAgent* pAgent;
		while (iter->IsValid()) {
			pAgent = iter->GetVal();
			runState = pAgent->GetRunState();
			if (runState == gSKI_RUNSTATE_HALTED) {
				return HandleError("System halted (try 'init-soar').");
			}
			iter->Next() ;
		}
	}

	// Stop and reset the current thread
	if (m_pRun) {
		m_pRun->Stop(true);
		delete m_pRun;
		m_pRun = 0;
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

	if (count) {
		if (options & OPTION_RUN_SELF) {
			pAgent->RunInClientThread(runType, count, m_pError);
		} else {
			// Running all agents
			m_pKernel->GetAgentManager()->ClearAllInterrupts();
			m_pKernel->GetAgentManager()->AddAllAgentsToRunList();
			m_pKernel->GetAgentManager()->RunInClientThread(runType, count, gSKI_INTERLEAVE_SMALLEST_STEP, m_pError);
		}
	} else {
		m_pRun = new RunThread(count, runType, (options & OPTION_RUN_SELF) ? true : false, m_pKernel, pAgent, m_pError);
		m_pRun->Start();
	}
	return true;
}

