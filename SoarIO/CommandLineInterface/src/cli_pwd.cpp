#include "cli_CommandLineInterface.h"

using namespace cli;

bool CommandLineInterface::ParsePWD(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	unused(pAgent);

	// No arguments to print working directory
	if (argv.size() != 1) {
		return HandleSyntaxError(Constants::kCLIPWD, Constants::kCLITooManyArgs);
	}
	return DoPWD();
}

bool CommandLineInterface::DoPWD() {

	std::string directory;
	bool ret = GetCurrentWorkingDirectory(directory);

	// On success, working dir is in parameter, on failure it is empty so this statement has no effect
	AppendToResult(directory);

	return ret;
}

