/////////////////////////////////////////////////////////////////
// pwatch command file.
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

#include "sml_Names.h"
#include "sml_StringOps.h"

#include "gSKI_Agent.h"
#include "gSKI_Kernel.h"
#include "gSKI_DoNotTouch.h"
#include "gSKI_ProductionManager.h"
#include "IgSKI_Production.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParsePWatch(gSKI::Agent* pAgent, std::vector<std::string>& argv) {
	Options optionsData[] = {
		{'d', "disable",	0},
		{'e', "enable",		0},
		{'d', "off",		0},
		{'e', "on",			0},
		{0, 0, 0}
	};

	bool setting = true;
	bool query = true;

	for (;;) {
		if (!ProcessOptions(argv, optionsData)) return false;
		if (m_Option == -1) break;

		switch (m_Option) {
			case 'd':
				setting = false;
				query = false;
				break;
			case 'e':
				setting = true;
				break;
			default:
				return SetError(CLIError::kGetOptError);
		}
	}
	if (m_NonOptionArguments > 1) return SetError(CLIError::kTooManyArgs);

	if (m_NonOptionArguments == 1) return DoPWatch(pAgent, false, &argv[m_Argument - m_NonOptionArguments], setting);
	return DoPWatch(pAgent, query, 0);
}

bool CommandLineInterface::DoPWatch(gSKI::Agent* pAgent, bool query, const std::string* pProduction, bool setting) {

	if (!RequireAgent(pAgent)) return false;

	// Attain the evil back door of doom, even though we aren't the TgD
	gSKI::EvilBackDoor::TgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();

	gSKI::ProductionManager* pProductionManager = pAgent->GetProductionManager();
	gSKI::tIProductionIterator* pIter = 0;
	gSKI::IProduction* pProd = 0;

	// check for query
	if (query) {
		// list all productions currently being traced
		pIter = pProductionManager->GetAllProductions(false, &m_gSKIError);
		if (gSKI::isError(m_gSKIError)) {
			SetErrorDetail("Error getting all productions.");
			return SetError(CLIError::kgSKIError);
		}
		if (!pIter) return SetError(CLIError::kgSKIError);

		int productionCount = 0;

		for(; pIter->IsValid(); pIter->Next()) {

			pProd = pIter->GetVal();

			// is it being watched
			if (pProd->IsWatched()) {
				++productionCount;
				if (m_RawOutput) {
					m_Result << '\n' << pProd->GetName();
				} else {
					AppendArgTagFast(sml_Names::kParamName, sml_Names::kTypeString, pProd->GetName());
				}
			}
			pProd->Release();
		}
		pIter->Release();

		if (m_RawOutput) {
			if (!productionCount) {
				m_Result << "No watched productions found.";
			}
		} else if (!m_RawOutput) {
			char buf[kMinBufferSize];
			PrependArgTag(sml_Names::kParamCount, sml_Names::kTypeInt, Int2String(productionCount, buf, kMinBufferSize));
		}
		return true;
	}

	// we are not querying
	if (!pProduction) {
		// disable tracing of all productions
		pIter = pProductionManager->GetAllProductions(false, &m_gSKIError);
		if (gSKI::isError(m_gSKIError)) {
			SetErrorDetail("Error getting all productions.");
			return SetError(CLIError::kgSKIError);
		}
		if (!pIter) return SetError(CLIError::kgSKIError);

		for(; pIter->IsValid(); pIter->Next()) {

			pProd = pIter->GetVal();

			// is it being watched
			if (pProd->IsWatched()) {
				// shut it off
				if (!pKernelHack->StopTracingProduction(pAgent, pProd->GetName())) {
					// really shouldn't happen
					return SetError(CLIError::kgSKIError);
				}
			}
			pProd->Release();
		}
		pIter->Release();		
		return true;
	}

	// we have a production
	if (setting) {
		if (!pKernelHack->BeginTracingProduction(pAgent, pProduction->c_str())) return SetError(CLIError::kProductionNotFound);
	} else {
		if (!pKernelHack->StopTracingProduction(pAgent, pProduction->c_str())) return SetError(CLIError::kProductionNotFound);
	}
	return true;
}

