#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Constants.h"

using namespace cli;

bool CommandLineInterface::ParseIO(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	unused(pAgent);
	unused(argv);

	return m_Error.SetError(CLIError::kNotImplemented);
}

bool CommandLineInterface::DoIO() {
	
	return m_Error.SetError(CLIError::kNotImplemented);
}

