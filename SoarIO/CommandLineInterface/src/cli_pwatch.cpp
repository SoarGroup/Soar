#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Constants.h"

#include "cli_GetOpt.h"

#include "sml_Names.h"
#include "sml_StringOps.h"

#include "IgSKI_Agent.h"
#include "IgSKI_Kernel.h"
#include "IgSKI_DoNotTouch.h"
#include "IgSKI_ProductionManager.h"
#include "IgSKI_Production.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParsePWatch(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	static struct GetOpt::option longOptions[] = {
		{"disable",	0, 0, 'd'},
		{"enable",	0, 0, 'e'},
		{"off",		0, 0, 'd'},
		{"on",		0, 0, 'e'},
		{0, 0, 0, 0}
	};

	bool setting = true;
	bool query = true;

	for (;;) {
		int option = m_pGetOpt->GetOpt_Long(argv, "de", longOptions, 0);
		if (option == -1) break;

		switch (option) {
			case 'd':
				setting = false;
				query = false;
				break;
			case 'e':
				setting = true;
				break;
			case '?':
				return m_Error.SetError(CLIError::kUnrecognizedOption);
			default:
				return m_Error.SetError(CLIError::kGetOptError);
		}
	}
	if (m_pGetOpt->GetAdditionalArgCount() > 1) return m_Error.SetError(CLIError::kTooManyArgs);

	if (m_pGetOpt->GetAdditionalArgCount() == 1) return DoPWatch(pAgent, false, &argv[m_pGetOpt->GetOptind()], setting);
	return DoPWatch(pAgent, query, 0);
}

bool CommandLineInterface::DoPWatch(gSKI::IAgent* pAgent, bool query, std::string* pProduction, bool setting) {

	if (!RequireAgent(pAgent)) return false;

	// Attain the evil back door of doom, even though we aren't the TgD
	gSKI::EvilBackDoor::ITgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();

	gSKI::IProductionManager* pProductionManager = pAgent->GetProductionManager();
	gSKI::tIProductionIterator* pIter = 0;
	gSKI::IProduction* pProd = 0;

	// check for query
	if (query) {
		// list all productions currently being traced
		pIter = pProductionManager->GetAllProductions(m_pgSKIError);
		if (!pIter) return m_Error.SetError(CLIError::kgSKIError);

		int productionCount = 0;

		for(; pIter->IsValid(); pIter->Next()) {

			pProd = pIter->GetVal();

			// is it being watched
			if (pProd->IsWatched()) {
				++productionCount;
				if (m_RawOutput) {
					AppendToResult(pProd->GetName());
					AppendToResult('\n');
				} else {
					AppendArgTag(sml_Names::kParamName, sml_Names::kTypeString, pProd->GetName());
				}
			}
			pProd->Release();
		}
		pIter->Release();

		if (m_RawOutput) {
			if (productionCount) {
				// trim trailing newline
				m_Result = m_Result.substr(0, m_Result.size() - 1);
			} else {
				AppendToResult("No watched productions found.");
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
		pIter = pProductionManager->GetAllProductions(m_pgSKIError);
		if (!pIter) return m_Error.SetError(CLIError::kgSKIError);

		for(; pIter->IsValid(); pIter->Next()) {

			pProd = pIter->GetVal();

			// is it being watched
			if (pProd->IsWatched()) {
				// shut it off
				if (!pKernelHack->StopTracingProduction(pAgent, pProd->GetName())) {
					// really shouldn't happen
					return m_Error.SetError(CLIError::kgSKIError);
				}
			}
			pProd->Release();
		}
		pIter->Release();		
		return true;
	}

	// we have a production
	if (setting) {
		if (!pKernelHack->BeginTracingProduction(pAgent, pProduction->c_str())) return m_Error.SetError(CLIError::kProductionNotFound);
	} else {
		if (!pKernelHack->StopTracingProduction(pAgent, pProduction->c_str())) return m_Error.SetError(CLIError::kProductionNotFound);
	}
	return true;
}

