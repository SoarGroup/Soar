#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Constants.h"
#include "cli_GetOpt.h"

#include "sml_Names.h"

#include "IgSKI_Kernel.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParsePreferences(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	static struct GetOpt::option longOptions[] = {
		{"none",		0, 0, '0'},
		{"names",		0, 0, '1'},
		{"timetags",	0, 0, '2'},
		{"wmes",		0, 0, '3'},
		{0, 0, 0, 0}
	};

	ePreferencesDetail detail = PREFERENCES_ONLY;

	for (;;) {
		int option = m_pGetOpt->GetOpt_Long(argv, "0123nNtw", longOptions, 0);
		if (option == -1) break;

		switch (option) {
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
			case '?':
				return SetError(CLIError::kUnrecognizedOption);
			default:
				return SetError(CLIError::kGetOptError);
		}
	}

	// Up to two non-option arguments allowed, id/attribute
	if (m_pGetOpt->GetAdditionalArgCount() > 2) return SetError(CLIError::kTooManyArgs);

	int optind = m_pGetOpt->GetOptind();
	if (m_pGetOpt->GetAdditionalArgCount() == 2) {
		// id & attribute
		return DoPreferences(pAgent, detail, &argv[optind], &argv[optind + 1]);
	}
	if (m_pGetOpt->GetAdditionalArgCount() == 1) {
		// id
		return DoPreferences(pAgent, detail, &argv[optind]);
	}

	return DoPreferences(pAgent, detail);
}

bool CommandLineInterface::DoPreferences(gSKI::IAgent* pAgent, const ePreferencesDetail detail, const std::string* pId, const std::string* pAttribute) {

	if (!RequireAgent(pAgent)) return false;

	// This whole implementation is a big hack.

	// First, define some constants
	const char* _preferences = "preferences";
	const char* _0 = "0";
	const char* _1 = "1";
	const char* _2 = "2";
	const char* _3 = "3";

	// Next, start arranging the argv
	char* argv[5];
	int argc = 0;
	argv[argc++] = const_cast<char*>(_preferences);

	// Include the ID and Attribute as needed
	if (pId) argv[argc++] = const_cast<char*>(pId->c_str());
	if (pAttribute) argv[argc++] = const_cast<char*>(pAttribute->c_str());

	// Include the detail level
	switch (detail) {
		default:
		case 0:
			argv[argc++] = const_cast<char*>(_0);
			break;
		case 1:
			argv[argc++] = const_cast<char*>(_1);
			break;
		case 2:
			argv[argc++] = const_cast<char*>(_2);
			break;
		case 3:
			argv[argc++] = const_cast<char*>(_3);
			break;
	}

	// Set the final arg to 0
	argv[argc] = 0;

	// Attain the evil back door of doom, even though we aren't the TgD, because we'll need it
	gSKI::EvilBackDoor::ITgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();

	// Fire the hack off
	AddListenerAndDisableCallbacks(pAgent);
	bool ret = pKernelHack->Preferences(pAgent, argc, argv);
	RemoveListenerAndEnableCallbacks(pAgent);

	if (!ret) return SetError(CLIError::kgSKIError);

	// put the result into a message(string) arg tag
	if (!m_RawOutput) ResultToArgTag();
	return true;
}
