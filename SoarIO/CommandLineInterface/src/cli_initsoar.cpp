#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Constants.h"

#include "IgSKI_Agent.h"

using namespace cli;

bool CommandLineInterface::ParseInitSoar(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	unused(argv);
	return DoInitSoar(pAgent);
}

/*************************************************************
* @brief init-soar command
* @param pAgent The pointer to the gSKI agent interface
*************************************************************/
EXPORT bool CommandLineInterface::DoInitSoar(gSKI::IAgent* pAgent) {
	// Need agent pointer for function calls
	if (!RequireAgent(pAgent)) return false;

	// Simply call reinitialize
	pAgent->Halt();
	// BUGBUG: Init soar sends output through print callback!
	AddListenerAndDisableCallbacks(pAgent);
	pAgent->Reinitialize();
	m_Result.str(""); // BUGBUG: This may be erasing more than it should
	RemoveListenerAndEnableCallbacks(pAgent);
	if (m_RawOutput) m_Result << "Agent reinitialized.";
	return true;
}

