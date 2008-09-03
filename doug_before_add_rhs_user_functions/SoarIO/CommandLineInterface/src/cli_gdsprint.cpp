#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Constants.h"

#include "IgSKI_Kernel.h"
#include "IgSKI_DoNotTouch.h"

using namespace cli;

bool CommandLineInterface::ParseGDSPrint(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	unused(argv);

	return DoGDSPrint(pAgent);
}

bool CommandLineInterface::DoGDSPrint(gSKI::IAgent* pAgent) {

	// Need agent pointer for function calls
	if (!RequireAgent(pAgent)) return false;
	if (!RequireKernel()) return false;

	// Attain the evil back door of desolation, even though we aren't the TgD
	gSKI::EvilBackDoor::ITgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();

	return pKernelHack->GDSPrint(pAgent);
}

