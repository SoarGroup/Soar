#include "cli_CommandLineInterface.h"

using namespace cli;

// ____                     ______        ______
//|  _ \ __ _ _ __ ___  ___|  _ \ \      / /  _ \
//| |_) / _` | '__/ __|/ _ \ |_) \ \ /\ / /| | | |
//|  __/ (_| | |  \__ \  __/  __/ \ V  V / | |_| |
//|_|   \__,_|_|  |___/\___|_|     \_/\_/  |____/
//
bool CommandLineInterface::ParsePWD(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	unused(pAgent);

	// No arguments to print working directory
	if (argv.size() != 1) {
		return HandleSyntaxError(Constants::kCLIPWD, Constants::kCLITooManyArgs);
	}
	return DoPWD();
}

// ____        ______        ______
//|  _ \  ___ |  _ \ \      / /  _ \
//| | | |/ _ \| |_) \ \ /\ / /| | | |
//| |_| | (_) |  __/ \ V  V / | |_| |
//|____/ \___/|_|     \_/\_/  |____/
//
bool CommandLineInterface::DoPWD() {

	std::string directory;
	bool ret = GetCurrentWorkingDirectory(directory);

	// On success, working dir is in parameter, on failure it is empty so this statement has no effect
	m_Result += directory;

	return ret;
}

