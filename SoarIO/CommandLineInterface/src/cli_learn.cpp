#include "cli_CommandLineInterface.h"

#include "cli_GetOpt.h"

#include "sml_Names.h"

#include "IgSKI_Agent.h"

using namespace cli;
using namespace sml;

// ____                     _
//|  _ \ __ _ _ __ ___  ___| |    ___  __ _ _ __ _ __
//| |_) / _` | '__/ __|/ _ \ |   / _ \/ _` | '__| '_ \
//|  __/ (_| | |  \__ \  __/ |__|  __/ (_| | |  | | | |
//|_|   \__,_|_|  |___/\___|_____\___|\__,_|_|  |_| |_|
//
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

	int option;
	unsigned int options = 0;

	for (;;) {
		option = m_pGetOpt->GetOpt_Long(argv, "abdeElo", longOptions, 0);
		if (option == -1) {
			break;
		}

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
				return HandleSyntaxError(Constants::kCLILearn, Constants::kCLIUnrecognizedOption);
			default:
				return HandleGetOptError((char)option);
		}
	}

	// No non-option arguments
	if ((unsigned)GetOpt::optind != argv.size()) {
		return HandleSyntaxError(Constants::kCLILearn, Constants::kCLITooManyArgs);
	}

	return DoLearn(pAgent, options);
}

// ____        _
//|  _ \  ___ | |    ___  __ _ _ __ _ __
//| | | |/ _ \| |   / _ \/ _` | '__| '_ \
//| |_| | (_) | |__|  __/ (_| | |  | | | |
//|____/ \___/|_____\___|\__,_|_|  |_| |_|
//
bool CommandLineInterface::DoLearn(gSKI::IAgent* pAgent, const unsigned int options) {
	// Need agent pointer for function calls
	if (!RequireAgent(pAgent)) return false;

	// No options means print current settings
	if (!options) {
		if (m_RawOutput) {
			m_Result += "Learning is ";
			m_Result += pAgent->IsLearningOn() ? "enabled." : "disabled.";
		} else {
			const char* setting = pAgent->IsLearningOn() ? sml_Names::kTrue : sml_Names::kFalse;
			AppendArgTagFast(sml_Names::kParamLearnSetting, sml_Names::kTypeBoolean, setting);
		}
		return true;
	}

	// Check for unimplemented options
	if ((options & OPTION_LEARN_ALL_LEVELS) || (options & OPTION_LEARN_BOTTOM_UP) || (options & OPTION_LEARN_EXCEPT) || (options & OPTION_LEARN_LIST) || (options & OPTION_LEARN_ONLY)) {
		return HandleError("Options {abElo} are not implemented.");
	}

	// Enable or disable, priority to disable
	if (options & OPTION_LEARN_ENABLE) {
		pAgent->SetLearning(true);
	}

	if (options & OPTION_LEARN_DISABLE) {
		pAgent->SetLearning(false);
	}
	return true;
}

