#include "cli_CommandLineInterface.h"

using namespace cli;
using namespace sml;

// ____                     _   _      _       _____
//|  _ \ __ _ _ __ ___  ___| | | | ___| |_ __ | ____|_  __
//| |_) / _` | '__/ __|/ _ \ |_| |/ _ \ | '_ \|  _| \ \/ /
//|  __/ (_| | |  \__ \  __/  _  |  __/ | |_) | |___ >  <
//|_|   \__,_|_|  |___/\___|_| |_|\___|_| .__/|_____/_/\_\
//                                      |_|
bool CommandLineInterface::ParseHelpEx(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	unused(pAgent);

	if (argv.size() != 2) {
		return HandleSyntaxError(Constants::kCLIHelpEx);
	}

	return DoHelpEx(argv[1]);
}

// ____        _   _      _       _____
//|  _ \  ___ | | | | ___| |_ __ | ____|_  __
//| | | |/ _ \| |_| |/ _ \ | '_ \|  _| \ \/ /
//| |_| | (_) |  _  |  __/ | |_) | |___ >  <
//|____/ \___/|_| |_|\___|_| .__/|_____/_/\_\
//                         |_|
bool CommandLineInterface::DoHelpEx(const std::string& command) {
	std::string output;

	if (!m_Constants.IsUsageFileAvailable()) {
		return HandleError(Constants::kCLINoUsageFile);
	}

	if (!m_Constants.GetExtendedUsageFor(command, output)) {
		return HandleError("Extended help for command '" + command + "' not found.");
	}
	m_Result += output;
	return true;
}

