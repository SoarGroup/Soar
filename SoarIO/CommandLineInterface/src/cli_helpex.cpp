#include "cli_CommandLineInterface.h"

#include "cli_Constants.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseHelpEx(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	unused(pAgent);

	if (argv.size() != 2) {
		return HandleSyntaxError(Constants::kCLIHelpEx);
	}

	return DoHelpEx(argv[1]);
}

bool CommandLineInterface::DoHelpEx(const std::string& command) {
	std::string output;

	if (!m_pConstants->IsUsageFileAvailable()) {
		return HandleError(Constants::kCLINoUsageFile);
	}

	if (!m_pConstants->GetExtendedUsageFor(command, output)) {
		return HandleError("Extended help for command '" + command + "' not found.");
	}
	AppendToResult(output);
	return true;
}

