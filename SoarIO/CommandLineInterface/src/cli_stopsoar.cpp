#include "cli_CommandLineInterface.h"

#include "cli_GetOpt.h"

#include "IgSKI_Agent.h"

using namespace cli;

// ____                     ____  _             ____
//|  _ \ __ _ _ __ ___  ___/ ___|| |_ ___  _ __/ ___|  ___   __ _ _ __
//| |_) / _` | '__/ __|/ _ \___ \| __/ _ \| '_ \___ \ / _ \ / _` | '__|
//|  __/ (_| | |  \__ \  __/___) | || (_) | |_) |__) | (_) | (_| | |
//|_|   \__,_|_|  |___/\___|____/ \__\___/| .__/____/ \___/ \__,_|_|
//                                        |_|
bool CommandLineInterface::ParseStopSoar(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	static struct GetOpt::option longOptions[] = {
		{"self",		0, 0, 's'},
		{0, 0, 0, 0}
	};

	GetOpt::optind = 0;
	GetOpt::opterr = 0;

	int option;
	bool self = false;

	for (;;) {
		option = m_pGetOpt->GetOpt_Long(argv, "s", longOptions, 0);
		if (option == -1) {
			break;
		}

		switch (option) {
			case 's':
				self = true;
				break;
			case '?':
				return HandleSyntaxError(Constants::kCLIStopSoar, Constants::kCLIUnrecognizedOption);
			default:
				return HandleGetOptError((char)option);
		}
	}

	// Concatinate remaining args for 'reason'
	std::string reasonForStopping;
	if ((unsigned)GetOpt::optind < argv.size()) {
		while ((unsigned)GetOpt::optind < argv.size()) {
			reasonForStopping += argv[GetOpt::optind++] + ' ';
		}
	}
	return DoStopSoar(pAgent, self, reasonForStopping);
}

// ____       ____  _             ____
//|  _ \  ___/ ___|| |_ ___  _ __/ ___|  ___   __ _ _ __
//| | | |/ _ \___ \| __/ _ \| '_ \___ \ / _ \ / _` | '__|
//| |_| | (_) |__) | || (_) | |_) |__) | (_) | (_| | |
//|____/ \___/____/ \__\___/| .__/____/ \___/ \__,_|_|
//                          |_|
bool CommandLineInterface::DoStopSoar(gSKI::IAgent* pAgent, bool self, const std::string& reasonForStopping) {
	unused(pAgent);
	m_Result += "TODO: do stop-soar ";
	m_Result += self;
	m_Result += reasonForStopping;
	return true;
}

