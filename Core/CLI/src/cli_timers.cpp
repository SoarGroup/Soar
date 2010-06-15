/////////////////////////////////////////////////////////////////
// timers command file.
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

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseTimers(std::vector<std::string>& argv) {
	Options optionsData[] = {
		{'e', "enable",		OPTARG_NONE},
		{'d', "disable",	OPTARG_NONE},
		{'d', "off",		OPTARG_NONE},
		{'e', "on",			OPTARG_NONE},
		{0, 0, OPTARG_NONE}
	};

	bool print = true;
	bool setting = false;	// enable or disable timers, default of false ignored

	for (;;) {
		if (!ProcessOptions(argv, optionsData)) return false;
		if (m_Option == -1) break;

		switch (m_Option) {
			case 'e':
				print = false;
				setting = true; // enable timers
				break;
			case 'd':
				print = false;
				setting = false; // disable timers
				break;
			default:
				return SetError(CLIError::kGetOptError);
		}
	}

	// No non-option arguments
	if (m_NonOptionArguments) return SetError(CLIError::kTooManyArgs);

	return DoTimers(print ? 0 : &setting);
}

bool CommandLineInterface::DoTimers(bool* pSetting) {
	if (pSetting) {
		// set, don't print
		set_sysparam(m_pAgentSoar, TIMERS_ENABLED, *pSetting);

	} else {
		// print current setting
		if (m_RawOutput) {
			m_Result << (m_pAgentSoar->sysparams[TIMERS_ENABLED] ? "Timers are enabled." : "Timers are disabled.");
		} else {
			// adds <arg name="timers">true</arg> (or false) if the timers are
			// enabled (or disabled)
			AppendArgTagFast(sml_Names::kParamTimers, sml_Names::kTypeBoolean, m_pAgentSoar->sysparams[TIMERS_ENABLED] ? sml_Names::kTrue : sml_Names::kFalse);
		}
	}
	return true;
}

