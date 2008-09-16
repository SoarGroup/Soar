/////////////////////////////////////////////////////////////////
// preferences command file.
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
#include "sml_KernelHelpers.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParsePreferences(std::vector<std::string>& argv) {
	Options optionsData[] = {
		{'0', "none",		0},
		{'n', "names",		0},
		{'1', "names",		0},
		{'N', "names",		0},
		{'2', "timetags",	0},
		{'t', "timetags",	0},
		{'3', "wmes",		0},
		{'w', "wmes",		0},
		{'o', "object",    0},
		{0, 0, 0}
	};

	ePreferencesDetail detail = PREFERENCES_ONLY;
	bool object = false;

	for (;;) {
		if (!ProcessOptions(argv, optionsData)) return false;
		if (m_Option == -1) break;

		switch (m_Option) {
			case '0':
				detail = PREFERENCES_ONLY;
				break;
			case '1':
			case 'n':
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
				
			case 'o':
			case 'O':
				object = true;
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
		return DoPreferences(detail, object, &argv[optind], &argv[optind + 1]);
	}
	if (m_NonOptionArguments == 1) {
		// id
		return DoPreferences(detail, object, &argv[optind]);
	}

	return DoPreferences(detail, object);
}

bool CommandLineInterface::DoPreferences(const ePreferencesDetail detail, bool object, const std::string* pId, const std::string* pAttribute) {
	// Attain the evil back door of doom, even though we aren't the TgD, because we'll need it
	sml::KernelHelpers* pKernelHack = m_pKernelSML->GetKernelHelpers() ;

	//bool object = 1;
	bool ret = pKernelHack->Preferences(m_pAgentSML, static_cast<int>(detail), object, pId ? pId->c_str() : 0, pAttribute ? pAttribute->c_str() : 0);

	// put the result into a message(string) arg tag
	if (!ret) return SetError(CLIError::kPreferencesError);
	return ret;
}
