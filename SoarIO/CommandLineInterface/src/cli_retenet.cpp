#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_GetOpt.h"
#include "cli_Constants.h"

#include "IgSKI_Agent.h"
#include "IgSKI_ProductionManager.h"
#include "IgSKI_Production.h"

using namespace cli;

bool CommandLineInterface::ParseReteNet(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	static struct GetOpt::option longOptions[] = {
		{"load",		1, 0, 'l'},
		{"restore",		1, 0, 'r'},
		{"save",		1, 0, 's'},
		{0, 0, 0, 0}
	};

	GetOpt::optind = 0;
	GetOpt::opterr = 0;

	bool save = false;
	bool load = false;
	std::string filename;

	for (;;) {
		int option = m_pGetOpt->GetOpt_Long(argv, "l:r:s:", longOptions, 0);
		if (option == -1) break;

		switch (option) {
			case 'l':
			case 'r':
				load = true;
				save = false;
				filename = GetOpt::optarg;
				break;
			case 's':
				save = true;
				load = false;
				filename = GetOpt::optarg;
				break;
			case '?':
				return HandleSyntaxError(Constants::kCLIReteNet, Constants::kCLIUnrecognizedOption);
			default:
				return HandleGetOptError((char)option);
		}
	}

	// Must have a save or load operation
	if (!save && !load) return HandleSyntaxError(Constants::kCLIReteNet, Constants::kCLITooFewArgs);
	if ((unsigned)GetOpt::optind != argv.size()) return HandleSyntaxError(Constants::kCLIReteNet, Constants::kCLITooManyArgs);
	return DoReteNet(pAgent, save, filename);
}

bool CommandLineInterface::DoReteNet(gSKI::IAgent* pAgent, bool save, std::string filename) {
	if (!RequireAgent(pAgent)) return false;

	if (!filename.size()) return HandleError("rete-net command must have a filename.");

	gSKI::IProductionManager* pProductionManager = pAgent->GetProductionManager();
	gSKI::tIProductionIterator* pIter = 0;

	pIter = save ? pProductionManager->GetJustifications() : pProductionManager->GetAllProductions();

	// Make sure there are no justifications (save) or productions (load)
	if (pIter->GetNumElements()) {
		// There are, clean up and return
		while (pIter->IsValid()) {
			pIter->GetVal()->Release();
			pIter->Next();
		}
		pIter->Release();

		return HandleError(save ? "Can't save rete while justifications are present." : "Can't load rete unless production memory is empty.");
	}
	pIter->Release();

	if (save) {
		pProductionManager->SaveRete(filename.c_str(), m_pError);
	} else {
		pProductionManager->LoadRete(filename.c_str(), m_pError);
	}

	if(m_pError->Id != gSKI::gSKIERR_NONE) return HandleError(save ? "Error saving rete to file." : "Error loading rete from file.", m_pError);	
	return true;
}

