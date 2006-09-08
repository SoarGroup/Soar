/////////////////////////////////////////////////////////////////
// preferences command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include <portability.h>

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"

#include "sml_Names.h"

#include "gSKI_Kernel.h"
#include "gSKI_DoNotTouch.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParsePreferences(gSKI::Agent* pAgent, std::vector<std::string>& argv) {
	Options optionsData[] = {
		{'0', "none",		0},
		{'1', "names",		0},
		{'2', "timetags",	0},
		{'3', "wmes",		0},
		{0, 0, 0}
	};

	ePreferencesDetail detail = PREFERENCES_ONLY;

	for (;;) {
		if (!ProcessOptions(argv, optionsData)) return false;
		if (m_Option == -1) break;

		switch (m_Option) {
			case '0':
			case 'n':
				detail = PREFERENCES_ONLY;
				break;
			case '1':
			case 'N':
				detail = PREFERENCES_NAMES;
				break;
			case '2':
			case 't':
				detail = PREFERENCES_TIMETAGS;
				break;
			case '3':
			case 'w':
				detail = PREFERENCES_WMES;
				break;
			default:
				return SetError(CLIError::kGetOptError);
		}
	}

	// Up to two non-option arguments allowed, id/attribute
	if (m_NonOptionArguments > 2) return SetError(CLIError::kTooManyArgs);

	int optind = m_Argument - m_NonOptionArguments;
	if (m_NonOptionArguments == 2) {
		// id & attribute
		return DoPreferences(pAgent, detail, &argv[optind], &argv[optind + 1]);
	}
	if (m_NonOptionArguments == 1) {
		// id
		return DoPreferences(pAgent, detail, &argv[optind]);
	}

	return DoPreferences(pAgent, detail);
}

bool CommandLineInterface::DoPreferences(gSKI::Agent* pAgent, const ePreferencesDetail detail, const std::string* pId, const std::string* pAttribute) {
	if (!RequireAgent(pAgent)) return false;

	// Attain the evil back door of doom, even though we aren't the TgD, because we'll need it
	gSKI::EvilBackDoor::TgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();

	AddListenerAndDisableCallbacks(pAgent);
	bool ret = pKernelHack->Preferences(pAgent, static_cast<int>(detail), pId ? pId->c_str() : 0, pAttribute ? pAttribute->c_str() : 0);
	RemoveListenerAndEnableCallbacks(pAgent);

	// put the result into a message(string) arg tag
	if (!m_RawOutput) ResultToArgTag();
	if (!ret) return SetError(CLIError::kPreferencesError);
	return ret;
}
