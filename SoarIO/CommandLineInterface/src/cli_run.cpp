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

/*************************************************************
* @brief run command
* @param pAgent The pointer to the gSKI agent interface
* @param options Options for the run command, see cli_CommandData.h
* @param count The count, units or applicability depends on options
*************************************************************/
EXPORT bool CommandLineInterface::DoRun(gSKI::IAgent* pAgent, const RunBitset& options, int count) {
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

	// If running self, an agent pointer is necessary.  Otherwise, a Kernel pointer is necessary.
	egSKIRunResult runResult;
	if (options.test(RUN_SELF)) {
		runResult = pAgent->RunInClientThread(runType, count, m_pgSKIError);
	} else {
        m_pKernel->GetAgentManager()->ClearAllInterrupts();
        m_pKernel->GetAgentManager()->AddAllAgentsToRunList();
		runResult = m_pKernel->GetAgentManager()->RunInClientThread(runType, count, gSKI_INTERLEAVE_SMALLEST_STEP, m_pgSKIError);
	}

	// Check for error
	if (runResult == gSKI_RUN_ERROR) {
		m_Result << "Run failed.";
		return false;	// Hopefully details are in gSKI error message
	}

	char buf[kMinBufferSize];
	if (m_RawOutput) m_Result << "\nRun successful: ";
	switch (runResult) {
		case gSKI_RUN_EXECUTING:
			if (m_RawOutput) {
				m_Result << "(gSKI_RUN_EXECUTING)";						// the run is still executing
			} else {
				AppendArgTagFast(sml_Names::kParamRunResult, sml_Names::kTypeInt, Int2String((int)runResult, buf, kMinBufferSize));
			}
			break;
		case gSKI_RUN_INTERRUPTED:
			if (m_RawOutput) {
				m_Result << "(gSKI_RUN_INTERRUPTED)";					// the run was interrupted
			} else {
				AppendArgTagFast(sml_Names::kParamRunResult, sml_Names::kTypeInt, Int2String((int)runResult, buf, kMinBufferSize));
			}
			break;
		case gSKI_RUN_COMPLETED:
			if (m_RawOutput) {
				m_Result << "(gSKI_RUN_COMPLETED)";						// the run completed normally
			} else {
				AppendArgTagFast(sml_Names::kParamRunResult, sml_Names::kTypeInt, Int2String((int)runResult, buf, kMinBufferSize));
			}
			break;
		case gSKI_RUN_COMPLETED_AND_INTERRUPTED:					// an interrupt was requested, but the run completed first
			if (m_RawOutput) {
				m_Result << "(gSKI_RUN_COMPLETED_AND_INTERRUPTED)";
			} else {
				AppendArgTagFast(sml_Names::kParamRunResult, sml_Names::kTypeInt, Int2String((int)runResult, buf, kMinBufferSize));
			}
			break;
		default:
			return SetError(CLIError::kgSKIError);
	}
	return true;
}
