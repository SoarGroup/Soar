#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Constants.h"

#include "sml_Names.h"
#include "sml_StringOps.h"

#include "IgSKI_WorkingMemory.h"
#include "IgSKI_Agent.h"
#include "IgSKI_Kernel.h"
#include "IgSKI_DoNotTouch.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseRemoveWME(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	// Exactly one argument
	if (argv.size() < 2) return m_Error.SetError(CLIError::kTooFewArgs);
	if (argv.size() > 2) return m_Error.SetError(CLIError::kTooManyArgs);

	int timetag = atoi(argv[1].c_str());
	if (!timetag) return m_Error.SetError(CLIError::kIntegerMustBePositive);

	return DoRemoveWME(pAgent, timetag);
}

bool CommandLineInterface::DoRemoveWME(gSKI::IAgent* pAgent, int timetag) {
	// Need agent pointer for function calls
	if (!RequireAgent(pAgent)) return false;

	// Attain the evil back door of doom, even though we aren't the TgD
	gSKI::EvilBackDoor::ITgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();

	if (pKernelHack->RemoveWmeByTimetag(pAgent, timetag)) {
		// this is failure
		return m_Error.SetError("Failed to remove WME.");
	}
	return true;
}

