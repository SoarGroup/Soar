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
	if (!save && !load) return SetError(CLIError::kMustSaveOrLoad);
	if (m_pGetOpt->GetAdditionalArgCount()) return SetError(CLIError::kMissingOptionArg);

	return DoReteNet(pAgent, save, filename);
}

bool CommandLineInterface::DoReteNet(gSKI::IAgent* pAgent, bool save, const std::string& filename) {
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
		pProductionManager->SaveRete(filename.c_str(), &m_gSKIError);
		if (gSKI::isError(m_gSKIError)) return SetError(CLIError::kgSKIError);
	} else {
		pProductionManager->LoadRete(filename.c_str(), &m_gSKIError);
		if (gSKI::isError(m_gSKIError)) return SetError(CLIError::kgSKIError);
	}

	if (gSKI::isError(m_gSKIError)) return SetError(save ? CLIError::kReteSaveOperationFail : CLIError::kReteLoadOperationFail);
	return true;
}

