#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include <algorithm>

#include "cli_Constants.h"
#include "cli_GetOpt.h"
#include "sml_Names.h"
#include "sml_StringOps.h"

#include "IgSKI_Agent.h"
#include "IgSKI_ProductionManager.h"
#include "IgSKI_Production.h"

#ifdef _MSC_VER
#define snprintf _snprintf 
#endif // _MSC_VER

using namespace cli;
using namespace sml;

struct MemoriesSort {
	bool operator()(std::pair< std::string, unsigned long > a, std::pair< std::string, unsigned long > b) const {
		return a.second < b.second;
	}
};

bool CommandLineInterface::ParseMemories(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {

	static struct GetOpt::option longOptions[] = {
		{"chunks",			0, 0, 'c'},
		{"default",			0, 0, 'd'},
		{"justifications",	0, 0, 'j'},
		{"user",			0, 0, 'u'},
		{0, 0, 0, 0}
	};

	unsigned int productionType = 0;

	for (;;) {
		int option = m_pGetOpt->GetOpt_Long(argv, "cdju", longOptions, 0);
		if (option == -1) break;

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
				return SetError(CLIError::kUnrecognizedOption);
			default:
				return SetError(CLIError::kGetOptError);
		}
	}

	// Max one additional argument
	if (m_pGetOpt->GetAdditionalArgCount() > 1) return SetError(CLIError::kTooManyArgs);		

	// It is either a production or a number
	std::string production;
	int n = 0;
	if (m_pGetOpt->GetAdditionalArgCount() == 1) {

		// explicitly check for 0 since that's atoi's error value
		int optind = m_pGetOpt->GetOptind();
		if (argv[optind][0] == '0') return SetError(CLIError::kIntegerMustBePositive);

		n = atoi(argv[optind].c_str());
		if (!n) {
			if (productionType) return SetError(CLIError::kNoProdTypeWhenProdName);
			production = argv[optind];
		}
	}

	// Default to all types when no production and no type specified
	if (!production.size() && !productionType) productionType = OPTION_MEMORIES_CHUNKS | OPTION_MEMORIES_DEFAULT | OPTION_MEMORIES_JUSTIFICATIONS | OPTION_MEMORIES_USER;

	return DoMemories(pAgent, productionType, n, production);
}

bool CommandLineInterface::DoMemories(gSKI::IAgent* pAgent, unsigned int productionType, int n, std::string production) {
	if (!RequireAgent(pAgent)) return false;

	gSKI::IProductionManager* pProductionManager = pAgent->GetProductionManager();
	gSKI::tIProductionIterator* pIter = 0;
	gSKI::IProduction* pProd = 0;
	std::vector< std::pair< std::string, unsigned long > > memories;

	bool foundProduction = false;

	if (!productionType) {
		pIter = pProductionManager->GetProduction(production.c_str());
	} else {
		pIter = pProductionManager->GetAllProductions(m_pgSKIError);
	}

	if (!pIter) return SetError(CLIError::kgSKIError);

	for(; pIter->IsValid(); pIter->Next()) {

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
				pIter->Release();
				return SetError(CLIError::kInvalidProductionType);
		}

		foundProduction = true;
		
		std::pair< std::string, unsigned long > memory;
		memory.first = pProd->GetName();
		memory.second = pProd->CountReteTokens();
		memories.push_back(memory);
		pProd->Release();
	}

	pIter->Release();
	pIter = 0;

	if (!foundProduction) return SetError(CLIError::kProductionNotFound);

	MemoriesSort s;
	sort(memories.begin(), memories.end(), s);
	char buf[1024];
	int i = 0;
	for (std::vector< std::pair< std::string, unsigned long > >::reverse_iterator j = memories.rbegin(); 
		j != memories.rend() && (n == 0 || i < n); 
		++j, ++i) 
	{
		if (m_RawOutput) {
			snprintf(buf, 1023, "\n%6lu:  %s", j->second, j->first.c_str());
			buf[1023] = 0;
			m_ResultStream << buf;
		} else {
			AppendArgTag(sml_Names::kParamName, sml_Names::kTypeString, j->first.c_str());
			AppendArgTag(sml_Names::kParamCount, sml_Names::kTypeInt, Int2String(j->second, buf, 1024));
		}
	}
	return true;
}

