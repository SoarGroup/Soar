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

EXPORT bool CommandLineInterface::DoHelpEx(const std::string& command) {
	unused(command);
	m_Result << "Help deprecated until release, please see\n\thttp://winter.eecs.umich.edu/soarwiki";
	return SetError(CLIError::kNoUsageFile);
	//std::string output;

	//if (!m_pConstants->IsUsageFileAvailable()) return SetError(CLIError::kNoUsageFile);
	//if (!m_pConstants->GetExtendedUsageFor(command, output)) return SetError(CLIError::kNoUsageInfo);

	//AppendToResult(output);
	//return true;
}

