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

	bool save = false;
	bool load = false;
	std::string filename;

	for (;;) {
		int option = m_pGetOpt->GetOpt_Long(argv, ":l:r:s:", longOptions, 0);
		if (option == -1) break;

		switch (option) {
			case 'l':
			case 'r':
				load = true;
				save = false;
				filename = m_pGetOpt->GetOptArg();
				break;
			case 's':
				save = true;
				load = false;
				filename = m_pGetOpt->GetOptArg();
				break;
			case ':':
				return SetError(CLIError::kMissingOptionArg);
			case '?':
				return SetError(CLIError::kUnrecognizedOption);
			default:
				return SetError(CLIError::kGetOptError);
		}
	}

	// Must have a save or load operation
	// TODO: these errors are misleading
	if (!save && !load) return SetError(CLIError::kTooFewArgs);
	if (m_pGetOpt->GetAdditionalArgCount()) return SetError(CLIError::kTooManyArgs);

	return DoReteNet(pAgent, save, filename);
}

/*************************************************************
* @brief rete-net command
* @param pAgent The pointer to the gSKI agent interface
* @param save true to save, false to load
* @param filename the rete-net file
*************************************************************/
EXPORT bool CommandLineInterface::DoReteNet(gSKI::IAgent* pAgent, bool save, const std::string& filename) {
	if (!RequireAgent(pAgent)) return false;

	if (!filename.size()) return SetError(CLIError::kMissingFilenameArg);

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

		return SetError(save ? CLIError::kCantSaveReteWithJustifications : CLIError::kCantLoadReteWithProductions);
	}
	pIter->Release();

	if (save) {
		pProductionManager->SaveRete(filename.c_str(), m_pgSKIError);
	} else {
		pProductionManager->LoadRete(filename.c_str(), m_pgSKIError);
	}

	if(m_pgSKIError->Id != gSKI::gSKIERR_NONE) return SetError(save ? CLIError::kReteSaveOperationFail : CLIError::kReteLoadOperationFail);	
	return true;
}

