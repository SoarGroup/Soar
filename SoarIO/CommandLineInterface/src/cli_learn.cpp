#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_GetOpt.h"
#include "cli_Constants.h"

#include "sml_Names.h"

#include "IgSKI_Agent.h"
#include "IgSKI_Kernel.h"
#include "IgSKI_DoNotTouch.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseLearn(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	static struct GetOpt::option longOptions[] = {
		{"all-levels",		0, 0, 'a'},
		{"bottom-up",		0, 0, 'b'},
		{"disable",			0, 0, 'd'},
		{"off",				0, 0, 'd'},
		{"enable",			0, 0, 'e'},
		{"on",				0, 0, 'e'},
		{"except",			0, 0, 'E'},
		{"list",			0, 0, 'l'},
		{"only",			0, 0, 'o'},
		{0, 0, 0, 0}
	};

	GetOpt::optind = 0;
	GetOpt::opterr = 0;

	unsigned int options = 0;

	for (;;) {
		int option = m_pGetOpt->GetOpt_Long(argv, "abdeElo", longOptions, 0);
		if (option == -1) break;

		switch (option) {
			case 'a':
				options |= OPTION_LEARN_ALL_LEVELS;
				break;
			case 'b':
				options |= OPTION_LEARN_BOTTOM_UP;
				break;
			case 'd':
				options |= OPTION_LEARN_DISABLE;
				break;
			case 'e':
				options |= OPTION_LEARN_ENABLE;
				break;
			case 'E':
				options |= OPTION_LEARN_EXCEPT;
				break;
			case 'l':
				options |= OPTION_LEARN_LIST;
				break;
			case 'o':
				options |= OPTION_LEARN_ONLY;
				break;
			case '?':
				return m_Error.SetError(CLIError::kUnrecognizedOption);
			default:
				return m_Error.SetError(CLIError::kGetOptError);
		}
	}

	// No non-option arguments
	if ((unsigned)GetOpt::optind != argv.size()) {
		return m_Error.SetError(CLIError::kTooManyArgs);
	}

	return DoLearn(pAgent, options);
}

bool CommandLineInterface::DoLearn(gSKI::IAgent* pAgent, const unsigned int options) {
	// Need agent pointer for function calls
	if (!RequireAgent(pAgent)) return false;

	// Attain the evil back door of doom, even though we aren't the TgD, because we'll need it
	gSKI::EvilBackDoor::ITgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();

	// No options means print current settings
	if (!options || (options & OPTION_LEARN_LIST)) {

		const long* pSysparams = pKernelHack->GetSysparams(pAgent);

		if (m_RawOutput) {
			if (pAgent->IsLearningOn()) {
				AppendToResult("Learning is enabled.");
				if (pSysparams[LEARNING_ONLY_SYSPARAM]) AppendToResult(" (only)");
				if (pSysparams[LEARNING_EXCEPT_SYSPARAM]) AppendToResult(" (except)");
				if (pSysparams[LEARNING_ALL_GOALS_SYSPARAM]) AppendToResult(" (all-levels)");
			} else {
				AppendToResult("Learning is disabled.");
			}
		} else {
			AppendArgTag(sml_Names::kParamLearnSetting, sml_Names::kTypeBoolean, pAgent->IsLearningOn() ? sml_Names::kTrue : sml_Names::kFalse);
			AppendArgTag(sml_Names::kParamLearnOnlySetting, sml_Names::kTypeBoolean, pSysparams[LEARNING_ONLY_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);
			AppendArgTag(sml_Names::kParamLearnExceptSetting, sml_Names::kTypeBoolean, pSysparams[LEARNING_EXCEPT_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);
			AppendArgTag(sml_Names::kParamLearnAllLevelsSetting, sml_Names::kTypeBoolean, pSysparams[LEARNING_ALL_GOALS_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);
		}

		if (options & OPTION_LEARN_LIST) {
			std::string output;
			if (m_RawOutput) {
				AppendToResult("\nforce-learn states (when learn 'only'):");
				pKernelHack->GetForceLearnStates(pAgent, output);
				if (output.size()) AppendToResult('\n' + output);
				AppendToResult("\ndont-learn states (when learn 'except'):");
				pKernelHack->GetDontLearnStates(pAgent, output);
				if (output.size()) AppendToResult('\n' + output);
			} else {
				pKernelHack->GetForceLearnStates(pAgent, output);
				AppendArgTag(sml_Names::kParamLearnForceLearnStates, sml_Names::kTypeString, output.c_str());
				pKernelHack->GetDontLearnStates(pAgent, output);
				AppendArgTag(sml_Names::kParamLearnDontLearnStates, sml_Names::kTypeString, output.c_str());
			}
		}
		return true;
	}

	if (options & OPTION_LEARN_ONLY) {
		pAgent->SetLearning(true);
		pKernelHack->SetSysparam(pAgent, LEARNING_ONLY_SYSPARAM, true);
		pKernelHack->SetSysparam(pAgent, LEARNING_EXCEPT_SYSPARAM, false);
	}

	if (options & OPTION_LEARN_EXCEPT) {
		pAgent->SetLearning(true);
		pKernelHack->SetSysparam(pAgent, LEARNING_ONLY_SYSPARAM, false);
		pKernelHack->SetSysparam(pAgent, LEARNING_EXCEPT_SYSPARAM, true);
	}

	if (options & OPTION_LEARN_ENABLE) {
		pAgent->SetLearning(true);
		pKernelHack->SetSysparam(pAgent, LEARNING_ONLY_SYSPARAM, false);
		pKernelHack->SetSysparam(pAgent, LEARNING_EXCEPT_SYSPARAM, false);
	}

	if (options & OPTION_LEARN_DISABLE) {
		pAgent->SetLearning(false);
		pKernelHack->SetSysparam(pAgent, LEARNING_ONLY_SYSPARAM, false);
		pKernelHack->SetSysparam(pAgent, LEARNING_EXCEPT_SYSPARAM, false);
	}

	if (options & OPTION_LEARN_ALL_LEVELS) {
		pKernelHack->SetSysparam(pAgent, LEARNING_ALL_GOALS_SYSPARAM, true);
	}

	if (options & OPTION_LEARN_BOTTOM_UP) {
		pKernelHack->SetSysparam(pAgent, LEARNING_ALL_GOALS_SYSPARAM, false);
	}

	return true;
}

