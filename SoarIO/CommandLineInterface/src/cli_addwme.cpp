#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Constants.h"

#include "sml_Names.h"

#include "IgSKI_WorkingMemory.h"
#include "IgSKI_Agent.h"
#include "IgSKI_Kernel.h"
#include "IgSKI_DoNotTouch.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseAddWME(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	if (argv.size() < 4) return m_Error.SetError(CLIError::kTooFewArgs);

	unsigned attributeIndex = (argv[2] == "^") ? 3 : 2;

	if (argv.size() < (attributeIndex + 2)) return m_Error.SetError(CLIError::kTooFewArgs);
    if (argv.size() > (attributeIndex + 3)) return m_Error.SetError(CLIError::kTooManyArgs);

	bool acceptable = false;
	if (argv.size() > (attributeIndex + 2)) {
		if (argv[attributeIndex + 2] != "+") return m_Error.SetError("Expected acceptable preference (+) or nothing, got '" + argv[attributeIndex + 2] + "'.");
		acceptable = true;
	}

	return DoAddWME(pAgent, argv[1], argv[attributeIndex], argv[attributeIndex + 1], acceptable);
}

bool CommandLineInterface::DoAddWME(gSKI::IAgent* pAgent, std::string id, std::string attribute, std::string value, bool acceptable) {
	// Need agent pointer for function calls
	if (!RequireAgent(pAgent)) return false;

	// Attain the evil back door of doom, even though we aren't the TgD
	gSKI::EvilBackDoor::ITgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();

	if (pKernelHack->AddWme(pAgent, id.c_str(), attribute.c_str(), value.c_str(), acceptable) <= 0) {
		return m_Error.SetError("Add Wme returned non-positive");
	}
	return true;
}



