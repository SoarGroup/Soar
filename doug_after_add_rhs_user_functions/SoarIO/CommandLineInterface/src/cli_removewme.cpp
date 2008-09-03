#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Constants.h"

using namespace cli;

bool CommandLineInterface::ParseRemoveWME(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	// Exactly one argument
	if (argv.size() < 2) return m_Error.SetError(CLIError::kTooFewArgs);
	if (argv.size() > 2) return m_Error.SetError(CLIError::kTooManyArgs);

	int timetag = atoi(argv[1].c_str());
	if (!timetag) return m_Error.SetError(CLIError::kIntegerMustBePositive);

	return DoRemoveWME(pAgent, timetag);
}

bool CommandLineInterface::DoRemoveWME(gSKI::IAgent* pAgent, int timetag) {
	unused(pAgent);
	unused(timetag);

	return false;
}

