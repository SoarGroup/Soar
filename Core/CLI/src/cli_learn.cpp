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

#include "sml_KernelSML.h"
#include "gsysparam.h"
#include "agent.h"
#include "print.h"

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

void GetForceLearnStates(agent* agnt, std::stringstream& res) {
	cons *c;
	char buff[1024];

	for (c = agnt->chunky_problem_spaces; c != NIL; c = c->rest) {
		symbol_to_string(agnt, static_cast<Symbol *>(c->first), TRUE, buff, 1024);
		res << buff;
	}
}

void GetDontLearnStates(agent* agnt, std::stringstream& res) {
	cons *c;
	char buff[1024];

	for (c = agnt->chunk_free_problem_spaces; c != NIL; c = c->rest) {
		symbol_to_string(agnt, static_cast<Symbol *>(c->first), TRUE, buff, 1024);
		res << buff;
	}
}

bool CommandLineInterface::DoLearn(const LearnBitset& options) {
	// No options means print current settings
	if (options.none() || options.test(LEARN_LIST)) {

		if (m_RawOutput) {
			if (m_pAgentSoar->sysparams[LEARNING_ON_SYSPARAM]) {
				m_Result << "Learning is enabled.";
				if (m_pAgentSoar->sysparams[LEARNING_ONLY_SYSPARAM]) m_Result << " (only)";
				if (m_pAgentSoar->sysparams[LEARNING_EXCEPT_SYSPARAM]) m_Result << " (except)";
				if (m_pAgentSoar->sysparams[LEARNING_ALL_GOALS_SYSPARAM]) m_Result << " (all-levels)";
				if (m_pAgentSoar->sysparams[CHUNK_THROUGH_LOCAL_NEGATIONS_SYSPARAM]) {
					m_Result << " (through-local-negations)";
				} else {
					m_Result << " (not through-local-negations)";
				}

			} else {
				m_Result << "Learning is disabled.";
			}
		} else {
			AppendArgTagFast(sml_Names::kParamLearnSetting, sml_Names::kTypeBoolean, m_pAgentSoar->sysparams[LEARNING_ON_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);
			AppendArgTagFast(sml_Names::kParamLearnOnlySetting, sml_Names::kTypeBoolean, m_pAgentSoar->sysparams[LEARNING_ONLY_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);
			AppendArgTagFast(sml_Names::kParamLearnExceptSetting, sml_Names::kTypeBoolean, m_pAgentSoar->sysparams[LEARNING_EXCEPT_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);
			AppendArgTagFast(sml_Names::kParamLearnAllLevelsSetting, sml_Names::kTypeBoolean, m_pAgentSoar->sysparams[LEARNING_ALL_GOALS_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);
		}

		if (options.test(LEARN_LIST)) {
			std::stringstream output;
			if (m_RawOutput) {
				m_Result << "\nforce-learn states (when learn 'only'):";
				GetForceLearnStates(m_pAgentSoar, output);
				if (output.str().size()) m_Result << '\n' << output.str();

				m_Result << "\ndont-learn states (when learn 'except'):";
				GetDontLearnStates(m_pAgentSoar, output);
				if (output.str().size()) m_Result << '\n' << output.str();

			} else {
				GetForceLearnStates(m_pAgentSoar, output);
				AppendArgTagFast(sml_Names::kParamLearnForceLearnStates, sml_Names::kTypeString, output.str());
				GetDontLearnStates(m_pAgentSoar, output);
				AppendArgTagFast(sml_Names::kParamLearnDontLearnStates, sml_Names::kTypeString, output.str());
			}
		}
		return true;
	}

	if (options.test(LEARN_ONLY)) {
		set_sysparam(m_pAgentSoar, LEARNING_ON_SYSPARAM, true);
		set_sysparam(m_pAgentSoar, LEARNING_ONLY_SYSPARAM, true);
		set_sysparam(m_pAgentSoar, LEARNING_EXCEPT_SYSPARAM, false);
	}

	if (options.test(LEARN_EXCEPT)) {
		set_sysparam(m_pAgentSoar, LEARNING_ON_SYSPARAM, true);
		set_sysparam(m_pAgentSoar, LEARNING_ONLY_SYSPARAM, false);
		set_sysparam(m_pAgentSoar, LEARNING_EXCEPT_SYSPARAM, true);
	}

	if (options.test(LEARN_ENABLE)) {
		set_sysparam(m_pAgentSoar, LEARNING_ON_SYSPARAM, true);
		set_sysparam(m_pAgentSoar, LEARNING_ONLY_SYSPARAM, false);
		set_sysparam(m_pAgentSoar, LEARNING_EXCEPT_SYSPARAM, false);
	}

	if (options.test(LEARN_DISABLE)) {
		set_sysparam(m_pAgentSoar, LEARNING_ON_SYSPARAM, false);
		set_sysparam(m_pAgentSoar, LEARNING_ONLY_SYSPARAM, false);
		set_sysparam(m_pAgentSoar, LEARNING_EXCEPT_SYSPARAM, false);
	}

	if (options.test(LEARN_ALL_LEVELS)) {
		set_sysparam(m_pAgentSoar, LEARNING_ALL_GOALS_SYSPARAM, true);
	}

	if (options.test(LEARN_BOTTOM_UP)) {
		set_sysparam(m_pAgentSoar, LEARNING_ALL_GOALS_SYSPARAM, false);
	}

	if (options.test(LEARN_ENABLE_THROUGH_LOCAL_NEGATIONS)) {
		set_sysparam(m_pAgentSoar, CHUNK_THROUGH_LOCAL_NEGATIONS_SYSPARAM, true);
	}

	if (options.test(LEARN_DISABLE_THROUGH_LOCAL_NEGATIONS)) {
		set_sysparam(m_pAgentSoar, CHUNK_THROUGH_LOCAL_NEGATIONS_SYSPARAM, false);
	}

	return true;
}

