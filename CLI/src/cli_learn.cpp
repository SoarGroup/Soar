/////////////////////////////////////////////////////////////////
// learn command file.
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

#include "IgSKI_Agent.h"
#include "IgSKI_Kernel.h"
#include "IgSKI_DoNotTouch.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseLearn(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	Options optionsData[] = {
		{'a', "all-levels",	0},
		{'b', "bottom-up",	0},
		{'d', "disable",	0},
		{'d', "off",		0},
		{'e', "enable",		0},
		{'e', "on",			0},
		{'E', "except",		0},
		{'l', "list",		0},
		{'o', "only",		0},
		{0, 0, 0}
	};

	LearnBitset options(0);

	for (;;) {
		if (!ProcessOptions(argv, optionsData)) return false;
		if (m_Option == -1) break;

		switch (m_Option) {
			case 'a':
				options.set(LEARN_ALL_LEVELS);
				break;
			case 'b':
				options.set(LEARN_BOTTOM_UP);
				break;
			case 'd':
				options.set(LEARN_DISABLE);
				break;
			case 'e':
				options.set(LEARN_ENABLE);
				break;
			case 'E':
				options.set(LEARN_EXCEPT);
				break;
			case 'l':
				options.set(LEARN_LIST);
				break;
			case 'o':
				options.set(LEARN_ONLY);
				break;
			default:
				return SetError(CLIError::kGetOptError);
		}
	}

	// No non-option arguments
	if (m_NonOptionArguments) return SetError(CLIError::kTooManyArgs);

	return DoLearn(pAgent, options);
}

bool CommandLineInterface::DoLearn(gSKI::IAgent* pAgent, const LearnBitset& options) {
	// Need agent pointer for function calls
	if (!RequireAgent(pAgent)) return false;

	// Attain the evil back door of doom, even though we aren't the TgD, because we'll need it
	gSKI::EvilBackDoor::ITgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();

	// No options means print current settings
	if (options.none() || options.test(LEARN_LIST)) {

		const long* pSysparams = pKernelHack->GetSysparams(pAgent);

		if (m_RawOutput) {
			if (pAgent->IsLearningOn()) {
				m_Result << "Learning is enabled.";
				if (pSysparams[LEARNING_ONLY_SYSPARAM]) m_Result << " (only)";
				if (pSysparams[LEARNING_EXCEPT_SYSPARAM]) m_Result << " (except)";
				if (pSysparams[LEARNING_ALL_GOALS_SYSPARAM]) m_Result << " (all-levels)";
			} else {
				m_Result << "Learning is disabled.";
			}
		} else {
			AppendArgTagFast(sml_Names::kParamLearnSetting, sml_Names::kTypeBoolean, pAgent->IsLearningOn() ? sml_Names::kTrue : sml_Names::kFalse);
			AppendArgTagFast(sml_Names::kParamLearnOnlySetting, sml_Names::kTypeBoolean, pSysparams[LEARNING_ONLY_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);
			AppendArgTagFast(sml_Names::kParamLearnExceptSetting, sml_Names::kTypeBoolean, pSysparams[LEARNING_EXCEPT_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);
			AppendArgTagFast(sml_Names::kParamLearnAllLevelsSetting, sml_Names::kTypeBoolean, pSysparams[LEARNING_ALL_GOALS_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);
		}

		if (options.test(LEARN_LIST)) {
			std::string output;
			if (m_RawOutput) {
				m_Result << "\nforce-learn states (when learn 'only'):";
				pKernelHack->GetForceLearnStates(pAgent, output);
				if (output.size()) m_Result << '\n' + output;

				m_Result << "\ndont-learn states (when learn 'except'):";
				pKernelHack->GetDontLearnStates(pAgent, output);
				if (output.size()) m_Result << '\n' + output;

			} else {
				pKernelHack->GetForceLearnStates(pAgent, output);
				AppendArgTagFast(sml_Names::kParamLearnForceLearnStates, sml_Names::kTypeString, output.c_str());
				pKernelHack->GetDontLearnStates(pAgent, output);
				AppendArgTagFast(sml_Names::kParamLearnDontLearnStates, sml_Names::kTypeString, output.c_str());
			}
		}
		return true;
	}

	if (options.test(LEARN_ONLY)) {
		pAgent->SetLearning(true);
		pKernelHack->SetSysparam(pAgent, LEARNING_ONLY_SYSPARAM, true);
		pKernelHack->SetSysparam(pAgent, LEARNING_EXCEPT_SYSPARAM, false);
	}

	if (options.test(LEARN_EXCEPT)) {
		pAgent->SetLearning(true);
		pKernelHack->SetSysparam(pAgent, LEARNING_ONLY_SYSPARAM, false);
		pKernelHack->SetSysparam(pAgent, LEARNING_EXCEPT_SYSPARAM, true);
	}

	if (options.test(LEARN_ENABLE)) {
		pAgent->SetLearning(true);
		pKernelHack->SetSysparam(pAgent, LEARNING_ONLY_SYSPARAM, false);
		pKernelHack->SetSysparam(pAgent, LEARNING_EXCEPT_SYSPARAM, false);
	}

	if (options.test(LEARN_DISABLE)) {
		pAgent->SetLearning(false);
		pKernelHack->SetSysparam(pAgent, LEARNING_ONLY_SYSPARAM, false);
		pKernelHack->SetSysparam(pAgent, LEARNING_EXCEPT_SYSPARAM, false);
	}

	if (options.test(LEARN_ALL_LEVELS)) {
		pKernelHack->SetSysparam(pAgent, LEARNING_ALL_GOALS_SYSPARAM, true);
	}

	if (options.test(LEARN_BOTTOM_UP)) {
		pKernelHack->SetSysparam(pAgent, LEARNING_ALL_GOALS_SYSPARAM, false);
	}

	return true;
}

