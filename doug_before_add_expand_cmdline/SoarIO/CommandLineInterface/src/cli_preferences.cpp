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

	GetOpt::optind = 0;
	GetOpt::opterr = 0;

	int detail = 0;

	for (;;) {
		int option = m_pGetOpt->GetOpt_Long(argv, "0123nNtw", longOptions, 0);
		if (option == -1) break;

		switch (option) {
			case '0':
			case 'n':
				detail = 0;
				break;
			case '1':
			case 'N':
				detail = 1;
				break;
			case '2':
			case 't':
				detail = 2;
				break;
			case '3':
			case 'w':
				detail = 3;
				break;
			case '?':
				return m_Error.SetError(CLIError::kUnrecognizedOption);
			default:
				return m_Error.SetError(CLIError::kGetOptError);
		}
	}

	// Up to two non-option arguments allowed, id/attribute
	if (argv.size() > static_cast<unsigned>(GetOpt::optind) + 2) {
		return m_Error.SetError(CLIError::kTooManyArgs);
	}
	if (argv.size() == static_cast<unsigned>(GetOpt::optind) + 2) {
		// id & attribute
		return DoPreferences(pAgent, detail, &argv[GetOpt::optind], &argv[GetOpt::optind + 1]);
	}
	if (argv.size() == static_cast<unsigned>(GetOpt::optind) + 1) {
		// id
		return DoPreferences(pAgent, detail, &argv[GetOpt::optind]);
	}

	return DoPreferences(pAgent, detail);
}

bool CommandLineInterface::DoPreferences(gSKI::IAgent* pAgent, int detail, std::string* pId, std::string* pAttribute) {

	if (!RequireAgent(pAgent)) return false;

	const char* _preferences = "preferences";
	const char* _0 = "0";
	const char* _1 = "1";
	const char* _2 = "2";
	const char* _3 = "3";

	char* argv[5];
	int argc = 0;
	argv[argc++] = const_cast<char*>(_preferences);

	if (pId) {
		argv[argc++] = const_cast<char*>(pId->c_str());
	}

	if (pAttribute) {
		argv[argc++] = const_cast<char*>(pAttribute->c_str());
	}

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

	argv[argc] = 0;

	// Attain the evil back door of doom, even though we aren't the TgD, because we'll need it
	gSKI::EvilBackDoor::ITgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();
	AddListenerAndDisableCallbacks(pAgent);
	bool ret = pKernelHack->Preferences(pAgent, argc, argv);
	RemoveListenerAndEnableCallbacks(pAgent);

	if (!ret) return m_Error.SetError(CLIError::kgSKIError);

	if (!m_RawOutput) {
		AppendArgTagFast(sml_Names::kParamMessage, sml_Names::kTypeString, m_Result.c_str());
		m_Result.clear();
	}
	return true;
}
