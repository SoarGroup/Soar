#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_GetOpt.h"
#include "cli_Constants.h"
#include "sml_Names.h"
#include "sml_StringOps.h"

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
		{"operator",	0, 0, 'O'},
		{"output",		0, 0, 'o'},
		{"phase",		0, 0, 'p'},
		{"self",		0, 0, 'S'},
		{"state",		0, 0, 's'},
		{0, 0, 0, 0}
	};

	GetOpt::optind = 0;
	GetOpt::opterr = 0;

	unsigned int options = 0;

	for (;;) {
		int option = m_pGetOpt->GetOpt_Long(argv, "defoOpsS", longOptions, 0);
		if (option == -1) break;

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
			case 'O':
				options |= OPTION_RUN_OPERATOR;
				break;
			case 'o':
				options |= OPTION_RUN_OUTPUT;
				break;
			case 'p':
				options |= OPTION_RUN_PHASE;
				break;
			case 'S':
				options |= OPTION_RUN_SELF;
				break;
			case 's':
				options |= OPTION_RUN_STATE;
				break;
			case '?':
				return m_Error.SetError(CLIError::kUnrecognizedOption);
			default:
				return m_Error.SetError(CLIError::kGetOptError);
		}
	}

	// Count defaults to 0
	int count = 0;

	// Only one non-option argument allowed, count
	if ((unsigned)GetOpt::optind == argv.size() - 1) {

		if (!IsInteger(argv[GetOpt::optind])) {
			return m_Error.SetError(CLIError::kIntegerExpected);
		}
		count = atoi(argv[GetOpt::optind].c_str());
		if (count <= 0) {
			return m_Error.SetError(CLIError::kIntegerMustBePositive);
		}

	} else if ((unsigned)GetOpt::optind < argv.size()) {
		return m_Error.SetError(CLIError::kTooManyArgs);
	}

	return DoRun(pAgent, options, count);
}

bool CommandLineInterface::DoRun(gSKI::IAgent* pAgent, const unsigned int options, int count) {
	// TODO: structured output

	if (!RequireAgent(pAgent)) return false;

	// TODO: Rather tricky options
	if ((options & OPTION_RUN_OPERATOR) || (options & OPTION_RUN_STATE)) {
		return m_Error.SetError(CLIError::kOptionNotImplemented);
	}

	// Default run type is forever
	egSKIRunType runType = gSKI_RUN_FOREVER;
	// ... unless there is a count, then the default is a decision cycle:
	if (count) runType = gSKI_RUN_DECISION_CYCLE;

	// Override run type with option flag:
	if (options & OPTION_RUN_ELABORATION) {
		runType = gSKI_RUN_SMALLEST_STEP;

	} else if (options & OPTION_RUN_PHASE) {
		runType = gSKI_RUN_PHASE;

	} else if (options & OPTION_RUN_DECISION) {
		runType = gSKI_RUN_DECISION_CYCLE;

	} else if (options & OPTION_RUN_OUTPUT) {
		runType = gSKI_RUN_UNTIL_OUTPUT;

	} else if (options & OPTION_RUN_FOREVER) {
		runType = gSKI_RUN_FOREVER;	
	}

	if (!count && runType != gSKI_RUN_FOREVER) {
		count = 1;
	}

	// If running self, an agent pointer is necessary.  Otherwise, a Kernel pointer is necessary.
	egSKIRunResult runResult;
	if (options & OPTION_RUN_SELF) {
		runResult = pAgent->RunInClientThread(runType, count, m_pgSKIError);
	} else {
        m_pKernel->GetAgentManager()->ClearAllInterrupts();
        m_pKernel->GetAgentManager()->AddAllAgentsToRunList();
		runResult = m_pKernel->GetAgentManager()->RunInClientThread(runType, count, gSKI_INTERLEAVE_SMALLEST_STEP, m_pgSKIError);
	}

	// Check for error
	if (runResult == gSKI_RUN_ERROR) {
		AppendToResult("Run failed.");
		return false;	// Hopefully details are in gSKI error message
	}

	char buf[kMinBufferSize];
	if (m_RawOutput) AppendToResult("\nRun successful: ");
	switch (runResult) {
		case gSKI_RUN_EXECUTING:
			if (m_RawOutput) {
				AppendToResult("(gSKI_RUN_EXECUTING)");						// the run is still executing
			} else {
				AppendArgTag(sml_Names::kParamRunResult, sml_Names::kTypeInt, Int2String((int)runResult, buf, kMinBufferSize));
			}
			break;
		case gSKI_RUN_INTERRUPTED:
			if (m_RawOutput) {
				AppendToResult("(gSKI_RUN_INTERRUPTED)");					// the run was interrupted
			} else {
				AppendArgTag(sml_Names::kParamRunResult, sml_Names::kTypeInt, Int2String((int)runResult, buf, kMinBufferSize));
			}
			break;
		case gSKI_RUN_COMPLETED:
			if (m_RawOutput) {
				AppendToResult("(gSKI_RUN_COMPLETED)");						// the run completed normally
			} else {
				AppendArgTag(sml_Names::kParamRunResult, sml_Names::kTypeInt, Int2String((int)runResult, buf, kMinBufferSize));
			}
			break;
		case gSKI_RUN_COMPLETED_AND_INTERRUPTED:					// an interrupt was requested, but the run completed first
			if (m_RawOutput) {
				AppendToResult("(gSKI_RUN_COMPLETED_AND_INTERRUPTED)");
			} else {
				AppendArgTag(sml_Names::kParamRunResult, sml_Names::kTypeInt, Int2String((int)runResult, buf, kMinBufferSize));
			}
			break;
		default:
			return m_Error.SetError(CLIError::kgSKIError);
	}
	return true;
}