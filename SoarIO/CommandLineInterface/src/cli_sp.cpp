#include "cli_CommandLineInterface.h"

#include "IgSKI_Agent.h"
#include "IgSKI_ProductionManager.h"
// BADBAD: I think we should be using an error class instead to work with error objects.
#include "../../gSKI/src/gSKI_Error.h"

using namespace cli;

// ____                     ____  ____
//|  _ \ __ _ _ __ ___  ___/ ___||  _ \
//| |_) / _` | '__/ __|/ _ \___ \| |_) |
//|  __/ (_| | |  \__ \  __/___) |  __/
//|_|   \__,_|_|  |___/\___|____/|_|
//
bool CommandLineInterface::ParseSP(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	// One argument (in brackets)
	if (argv.size() != 2) {
		return HandleSyntaxError(Constants::kCLISP);
	}

	// Remove first and last characters (the braces)
	std::string production = argv[1];
	if (production.length() < 3) {
		return HandleSyntaxError(Constants::kCLISP);
	}
	production = production.substr(1, production.length() - 2);

	return DoSP(pAgent, production);
}

// ____       ____  ____
//|  _ \  ___/ ___||  _ \
//| | | |/ _ \___ \| |_) |
//| |_| | (_) |__) |  __/
//|____/ \___/____/|_|
//
bool CommandLineInterface::DoSP(gSKI::IAgent* pAgent, const std::string& production) {
	// Must have agent to give production to
	if (!RequireAgent(pAgent)) return false;

	// Acquire production manager
	gSKI::IProductionManager *pProductionManager = pAgent->GetProductionManager();

	// Load the production
	pAgent->AddPrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);
	pProductionManager->AddProduction(const_cast<char*>(production.c_str()), m_pError);
	pAgent->RemovePrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);

	if(m_pError->Id != gSKI::gSKIERR_NONE) {
		return HandleError("Unable to add the production: " + production);
	}

	if (m_RawOutput) {
		// TODO: The kernel is supposed to print this but doesnt!
		AppendToResult('*');
	}
	return true;
}

