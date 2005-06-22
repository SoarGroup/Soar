/////////////////////////////////////////////////////////////////
// soar8 command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Constants.h"

using namespace cli;

bool CommandLineInterface::ParseSoar8(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	unused(pAgent);

	Options optionsData[] = {
		{'e', "on",			0},
		{'e', "enable",		0},
		{'d', "off",		0},
		{'d', "disable",	0},
		{0, 0, 0}
	};

	bool query = true;
	bool soar8 = true;

	for (;;) {
		if (!ProcessOptions(argv, optionsData)) return false;
		if (m_Option == -1) break;

		switch (m_Option) {
			case 'd':
				query = false;
				soar8 = false;
				break;
			case 'e':
				query = false;
				soar8 = true;
				break;
			default:
				return SetError(CLIError::kGetOptError);
		}
	}

	// No non-option arguments
	if (m_NonOptionArguments) return SetError(CLIError::kTooManyArgs);

	return DoSoar8(query ? &soar8 : 0);
}

bool CommandLineInterface::DoSoar8(bool* pSoar8) {
	unused(pSoar8);
	SetError(CLIError::kNotImplemented);
	return false;
}

