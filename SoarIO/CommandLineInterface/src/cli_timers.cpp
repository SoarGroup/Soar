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

bool CommandLineInterface::ParseTimers(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	static struct GetOpt::option longOptions[] = {
		{"enable",					0, 0, 'e'},
		{"disable",					0, 0, 'd'},
		{"off",						0, 0, 'd'},
		{"on",						0, 0, 'e'},
		{0, 0, 0, 0}
	};

	GetOpt::optind = 0;
	GetOpt::opterr = 0;

	bool print = true;
	bool setting = false;	// enable or disable timers, default of false ignored
	int option;

	for (;;) {
		option = m_pGetOpt->GetOpt_Long(argv, "ed", longOptions, 0);
		if (option == -1) {
			break;
		}

		switch (option) {
			case 'e':
				print = false;
				setting = true; // enable timers
				break;
			case 'd':
				print = false;
				setting = false; // disable timers
				break;
			case '?':
				return HandleSyntaxError(Constants::kCLITimers, Constants::kCLIUnrecognizedOption);
			default:
				return HandleGetOptError((char)option);
		}
	}

	// No non-option arguments
	if ((unsigned)GetOpt::optind != argv.size()) {
		return HandleSyntaxError(Constants::kCLITimers, Constants::kCLITooManyArgs);
	}

	return DoTimers(pAgent, print, setting);
}

bool CommandLineInterface::DoTimers(gSKI::IAgent* pAgent, bool print, bool setting) {
	// Need agent pointer and kernel pointer for sysparam
	if (!RequireAgent(pAgent)) return false;
	if (!RequireKernel()) return false;

	// Attain the evil back door of doom, even though we aren't the TgD, because we'll probably need it
	gSKI::EvilBackDoor::ITgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();

	if (print) {
		// print current setting
		const long* pSysparams = pKernelHack->GetSysparams(pAgent);

		// adds <arg name="timers">true</arg> (or false) if the timers are
		// enabled (or disabled)
		AppendArgTagFast(sml_Names::kParamTimers, sml_Names::kTypeBoolean,
			pSysparams[TIMERS_ENABLED] ? sml_Names::kTrue : sml_Names::kFalse);

	} else {
		// set, don't print
		pKernelHack->SetSysparam(pAgent, TIMERS_ENABLED, setting);
	}
	return true;
}

