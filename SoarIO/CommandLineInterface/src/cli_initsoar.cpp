#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Constants.h"

#include "IgSKI_Agent.h"

using namespace cli;

bool CommandLineInterface::ParseInitSoar(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	// No arguments
	if (argv.size() != 1) {
		return m_Error.SetError(CLIError::kTooManyArgs);
	}
	return DoInitSoar(pAgent);
}

bool CommandLineInterface::DoInitSoar(gSKI::IAgent* pAgent) {
	// Need agent pointer for function calls
	if (!RequireAgent(pAgent)) return false;

	// Simply call reinitialize
	pAgent->Halt();
	pAgent->Reinitialize();
	AppendToResult("Agent reinitialized.");
	return true;
}

