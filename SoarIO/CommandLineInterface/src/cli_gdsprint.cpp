#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Constants.h"
#include "sml_Names.h"

#include "IgSKI_Kernel.h"
#include "IgSKI_DoNotTouch.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseGDSPrint(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	unused(argv);
	return DoGDSPrint(pAgent);
}

/*************************************************************
* @brief gds-print command
* @param pAgent The pointer to the gSKI agent interface
*************************************************************/
EXPORT bool CommandLineInterface::DoGDSPrint(gSKI::IAgent* pAgent) {

	// Need agent pointer for function calls
	if (!RequireAgent(pAgent)) return false;

	// Attain the evil back door of desolation, even though we aren't the TgD
	gSKI::EvilBackDoor::ITgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();

	AddListenerAndDisableCallbacks(pAgent);
	bool ret = pKernelHack->GDSPrint(pAgent);
	RemoveListenerAndEnableCallbacks(pAgent);

	// put the result into a message(string) arg tag
	if (!m_RawOutput) ResultToArgTag();
	return ret;
}

