#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Constants.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseHelpEx(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	unused(pAgent);

	if (argv.size() > 2) return SetError(CLIError::kTooManyArgs);
	if (argv.size() < 2) return SetError(CLIError::kTooFewArgs);

	return DoHelpEx(argv[1]);
}

bool CommandLineInterface::DoHelpEx(const std::string& command) {
	std::string output;

	if (!m_pConstants->IsUsageFileAvailable()) return SetError(CLIError::kNoUsageFile);
	if (!m_pConstants->GetExtendedUsageFor(command, output)) return SetError(CLIError::kNoUsageInfo);

	AppendToResult(output);
	return true;
}

