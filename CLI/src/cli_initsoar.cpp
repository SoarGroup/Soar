/////////////////////////////////////////////////////////////////
// init-soar command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include <portability.h>

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"

#include "gSKI_Agent.h"

using namespace cli;

bool CommandLineInterface::ParseInitSoar(gSKI::Agent* pAgent, std::vector<std::string>& argv) {
	unused(argv);
	return DoInitSoar(pAgent);
}

bool CommandLineInterface::DoInitSoar(gSKI::Agent* pAgent) {
	// Need agent pointer for function calls
	if (!RequireAgent(pAgent)) return false;

	// Save the current result
	std::string oldResult = m_Result.str();

	AddListenerAndDisableCallbacks(pAgent);
	bool ok = pAgent->Reinitialize();
	RemoveListenerAndEnableCallbacks(pAgent);

	// restore the old result, ignoring output from init-soar
	m_Result.str(oldResult); 

	if (!ok)
	{
		m_Result << "Agent failed to reinitialize" ;
		return SetError(CLIError::kInitSoarFailed);
	}

	if (m_RawOutput) m_Result << "Agent reinitialized.";

	return ok;
}

