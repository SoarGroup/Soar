/////////////////////////////////////////////////////////////////
// learn command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "sml_Utils.h"
#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"
#include "cli_CLIError.h"

#include "sml_Names.h"

#include "sml_KernelHelpers.h"
#include "sml_KernelSML.h"
#include "gsysparam.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseLearn(std::vector<std::string>& argv) {
	Options optionsData[] = {
		{'a', "all-levels",	OPTARG_NONE},
		{'b', "bottom-up",	OPTARG_NONE},
		{'d', "disable",	OPTARG_NONE},
		{'d', "off",		OPTARG_NONE},
		{'e', "enable",		OPTARG_NONE},
		{'e', "on",			OPTARG_NONE},
		{'E', "except",		OPTARG_NONE},
		{'l', "list",		OPTARG_NONE},
		{'o', "only",		OPTARG_NONE},
		{'n', "enable-through-local-negations", OPTARG_NONE},
		{'N', "disable-through-local-negations", OPTARG_NONE},
		{0, 0, OPTARG_NONE}
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
			case 'n':
				options.set(LEARN_ENABLE_THROUGH_LOCAL_NEGATIONS);
				break;
			case 'N':
				options.set(LEARN_DISABLE_THROUGH_LOCAL_NEGATIONS);
				break;
			default:
				return SetError(CLIError::kGetOptError);
		}
	}

	// No non-option arguments
	if (m_NonOptionArguments) return SetError(CLIError::kTooManyArgs);

	return DoLearn(options);
}

bool CommandLineInterface::DoLearn(const LearnBitset& options) {
	// Attain the evil back door of doom, even though we aren't the TgD, because we'll need it
	sml::KernelHelpers* pKernelHack = m_pKernelSML->GetKernelHelpers() ;

	// No options means print current settings
	if (options.none() || options.test(LEARN_LIST)) {

		const long* const pSysparams = pKernelHack->GetSysparams(m_pAgentSML);

		if (m_RawOutput) {
			if (pSysparams[LEARNING_ON_SYSPARAM]) {
				m_Result << "Learning is enabled.";
				if (pSysparams[LEARNING_ONLY_SYSPARAM]) m_Result << " (only)";
				if (pSysparams[LEARNING_EXCEPT_SYSPARAM]) m_Result << " (except)";
				if (pSysparams[LEARNING_ALL_GOALS_SYSPARAM]) m_Result << " (all-levels)";
				if (pSysparams[CHUNK_THROUGH_LOCAL_NEGATIONS_SYSPARAM]) {
					m_Result << " (through-local-negations)";
				} else {
					m_Result << " (not through-local-negations)";
				}

			} else {
				m_Result << "Learning is disabled.";
			}
		} else {
			AppendArgTagFast(sml_Names::kParamLearnSetting, sml_Names::kTypeBoolean, pSysparams[LEARNING_ON_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);
			AppendArgTagFast(sml_Names::kParamLearnOnlySetting, sml_Names::kTypeBoolean, pSysparams[LEARNING_ONLY_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);
			AppendArgTagFast(sml_Names::kParamLearnExceptSetting, sml_Names::kTypeBoolean, pSysparams[LEARNING_EXCEPT_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);
			AppendArgTagFast(sml_Names::kParamLearnAllLevelsSetting, sml_Names::kTypeBoolean, pSysparams[LEARNING_ALL_GOALS_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);
		}

		if (options.test(LEARN_LIST)) {
			std::stringstream output;
			if (m_RawOutput) {
				m_Result << "\nforce-learn states (when learn 'only'):";
				pKernelHack->GetForceLearnStates(m_pAgentSML, output);
				if (output.str().size()) m_Result << '\n' << output.str();

				m_Result << "\ndont-learn states (when learn 'except'):";
				pKernelHack->GetDontLearnStates(m_pAgentSML, output);
				if (output.str().size()) m_Result << '\n' << output.str();

			} else {
				pKernelHack->GetForceLearnStates(m_pAgentSML, output);
				AppendArgTagFast(sml_Names::kParamLearnForceLearnStates, sml_Names::kTypeString, output.str());
				pKernelHack->GetDontLearnStates(m_pAgentSML, output);
				AppendArgTagFast(sml_Names::kParamLearnDontLearnStates, sml_Names::kTypeString, output.str());
			}
		}
		return true;
	}

	if (options.test(LEARN_ONLY)) {
		pKernelHack->SetSysparam(m_pAgentSML, LEARNING_ON_SYSPARAM, true);
		pKernelHack->SetSysparam(m_pAgentSML, LEARNING_ONLY_SYSPARAM, true);
		pKernelHack->SetSysparam(m_pAgentSML, LEARNING_EXCEPT_SYSPARAM, false);
	}

	if (options.test(LEARN_EXCEPT)) {
		pKernelHack->SetSysparam(m_pAgentSML, LEARNING_ON_SYSPARAM, true);
		pKernelHack->SetSysparam(m_pAgentSML, LEARNING_ONLY_SYSPARAM, false);
		pKernelHack->SetSysparam(m_pAgentSML, LEARNING_EXCEPT_SYSPARAM, true);
	}

	if (options.test(LEARN_ENABLE)) {
		pKernelHack->SetSysparam(m_pAgentSML, LEARNING_ON_SYSPARAM, true);
		pKernelHack->SetSysparam(m_pAgentSML, LEARNING_ONLY_SYSPARAM, false);
		pKernelHack->SetSysparam(m_pAgentSML, LEARNING_EXCEPT_SYSPARAM, false);
	}

	if (options.test(LEARN_DISABLE)) {
		pKernelHack->SetSysparam(m_pAgentSML, LEARNING_ON_SYSPARAM, false);
		pKernelHack->SetSysparam(m_pAgentSML, LEARNING_ONLY_SYSPARAM, false);
		pKernelHack->SetSysparam(m_pAgentSML, LEARNING_EXCEPT_SYSPARAM, false);
	}

	if (options.test(LEARN_ALL_LEVELS)) {
		pKernelHack->SetSysparam(m_pAgentSML, LEARNING_ALL_GOALS_SYSPARAM, true);
	}

	if (options.test(LEARN_BOTTOM_UP)) {
		pKernelHack->SetSysparam(m_pAgentSML, LEARNING_ALL_GOALS_SYSPARAM, false);
	}

	if (options.test(LEARN_ENABLE_THROUGH_LOCAL_NEGATIONS)) {
		pKernelHack->SetSysparam(m_pAgentSML, CHUNK_THROUGH_LOCAL_NEGATIONS_SYSPARAM, true);
	}

	if (options.test(LEARN_DISABLE_THROUGH_LOCAL_NEGATIONS)) {
		pKernelHack->SetSysparam(m_pAgentSML, CHUNK_THROUGH_LOCAL_NEGATIONS_SYSPARAM, false);
	}

	return true;
}

