#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Constants.h"
#include "cli_GetOpt.h"

#include "IgSKI_Agent.h"
#include "IgSKI_ProductionManager.h"
#include "IgSKI_Production.h"

#ifdef _MSC_VER
#define snprintf _snprintf 
#endif // _MSC_VER

using namespace cli;

bool CommandLineInterface::ParseMemories(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {

	static struct GetOpt::option longOptions[] = {
		{"chunks",			0, 0, 'c'},
		{"default",			0, 0, 'd'},
		{"justifications",	0, 0, 'j'},
		{"user",			0, 0, 'u'},
		{0, 0, 0, 0}
	};

	GetOpt::optind = 0;
	GetOpt::opterr = 0;

	unsigned int productionType = 0;

	for (;;) {
		int option = m_pGetOpt->GetOpt_Long(argv, "cdju", longOptions, 0);
		if (option == -1) {
			break;
		}

		switch (option) {
			case 'c':
				productionType |= OPTION_MEMORIES_CHUNKS;
				break;
			case 'd':
				productionType |= OPTION_MEMORIES_DEFAULT;
				break;
			case 'j':
				productionType |= OPTION_MEMORIES_JUSTIFICATIONS;
				break;
			case 'u':
				productionType |= OPTION_MEMORIES_USER;
				break;
			case '?':
				return HandleSyntaxError(Constants::kCLIMemories, Constants::kCLIUnrecognizedOption);
			default:
				return HandleGetOptError((char)option);
		}
	}

	// Max one additional argument
	if ((argv.size() - GetOpt::optind) > 1) return HandleSyntaxError(Constants::kCLIMemories, Constants::kCLITooManyArgs);		

	// It is either a production or a number
	std::string production;
	int n = 0;
	if ((argv.size() - GetOpt::optind) == 1) {
		n = atoi(argv[GetOpt::optind].c_str());
		if (!n) {
			if (productionType) return HandleSyntaxError(Constants::kCLIMemories, "Do not specify production type (-c, -j, etc) when specifying a production name.");
			production = argv[GetOpt::optind];
		}
	}

	// Default to all types when no production and no type specified
	if (!production.size() && !productionType) productionType = OPTION_MEMORIES_CHUNKS | OPTION_MEMORIES_DEFAULT | OPTION_MEMORIES_JUSTIFICATIONS | OPTION_MEMORIES_USER;

	return DoMemories(pAgent, productionType, n, production);
}

bool CommandLineInterface::DoMemories(gSKI::IAgent* pAgent, unsigned int productionType, int n, std::string production) {
	RequireAgent(pAgent);

	if (n && (n < 0)) return HandleError("n must be a positive integer.");

	gSKI::IProductionManager* pProductionManager = pAgent->GetProductionManager();
	gSKI::tIProductionIterator* pIter = 0;
	gSKI::IProduction* pProd = 0;

	if (productionType) {
		pIter = pProductionManager->GetAllProductions();
		bool foundProduction = false;

		for(int i = 0; pIter->IsValid() && (n == 0 || i < n); pIter->Next(), ++i) {

			foundProduction = true;

			pProd = pIter->GetVal();

			switch (pProd->GetType()) {
				case gSKI_CHUNK:
					if (!(productionType & OPTION_MEMORIES_CHUNKS)) continue;
					break;
				case gSKI_DEFAULT:
					if (!(productionType & OPTION_MEMORIES_DEFAULT)) continue;
					break;
				case gSKI_JUSTIFICATION:
					if (!(productionType & OPTION_MEMORIES_JUSTIFICATIONS)) continue;
					break;
				case gSKI_USER:
					if (!(productionType & OPTION_MEMORIES_USER)) continue;
					break;

				default:
					pProd->Release();
					pProd = 0;
					pIter->Release();
					pIter = 0;
					return HandleError("Invalid production type.");
			}

			char buf[1024];
			snprintf(buf, 1023, "%s: %ld\n", pProd->GetName(), pProd->CountReteTokens());
			AppendToResult(buf);

			pProd->Release();
		}
		pIter->Release();
		pIter = 0;

		if (!foundProduction) return HandleError("No productions found.");
		return true;
	}

	pIter = pProductionManager->GetProduction(production.c_str());
	pProd = pIter->IsValid() ? pIter->GetVal() : 0;

	pIter->Release(); 
	pIter = 0;

	if (!pProd) return HandleError("Production not found.");

	char buf[1024];
	snprintf(buf, 1023, "\n Memory use for %s: %ld\n\n", production.c_str(), pProd->CountReteTokens());
	AppendToResult(buf);
	pProd->Release(); 
	pProd = 0;
	return true;
}

