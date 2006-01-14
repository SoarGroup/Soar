/////////////////////////////////////////////////////////////////
// rete-net command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"

#include "IgSKI_Agent.h"
#include "IgSKI_ProductionManager.h"
#include "IgSKI_Production.h"

using namespace cli;

bool CommandLineInterface::ParseReteNet(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	Options optionsData[] = {
		{'l', "load",		1},
		{'r', "restore",	1},
		{'s', "save",		1},
		{0, 0, 0}
	};

	bool save = false;
	bool load = false;
	std::string filename;

	for (;;) {
		if (!ProcessOptions(argv, optionsData)) return false;
		if (m_Option == -1) break;

		switch (m_Option) {
			case 'l':
			case 'r':
				load = true;
				save = false;
				filename = m_OptionArgument;
				break;
			case 's':
				save = true;
				load = false;
				filename = m_OptionArgument;
				break;
			default:
				return SetError(CLIError::kGetOptError);
		}
	}

	// Must have a save or load operation
	if (!save && !load) return SetError(CLIError::kMustSaveOrLoad);
	if (m_NonOptionArguments) return SetError(CLIError::kTooManyArgs);

	return DoReteNet(pAgent, save, filename);
}

bool CommandLineInterface::DoReteNet(gSKI::IAgent* pAgent, bool save, std::string filename) {
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

	StripQuotes(filename);

	if (save) {
		pProductionManager->SaveRete(filename.c_str(), &m_gSKIError);
		if (gSKI::isError(m_gSKIError)) {
			SetErrorDetail("Error saving rete: " + filename);
			return SetError(CLIError::kgSKIError);
		}
	} else {
		pProductionManager->LoadRete(filename.c_str(), &m_gSKIError);
		if (gSKI::isError(m_gSKIError)) {
			SetErrorDetail("Error loading rete: " + filename);
			return SetError(CLIError::kgSKIError);
		}
	}

	if (gSKI::isError(m_gSKIError)) return SetError(save ? CLIError::kReteSaveOperationFail : CLIError::kReteLoadOperationFail);
	return true;
}

