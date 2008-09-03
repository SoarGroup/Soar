/////////////////////////////////////////////////////////////////
// memories command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include <algorithm>

#include "cli_Commands.h"
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

	Options optionsData[] = {
		{'c', "chunks",			0},
		{'d', "default",		0},
		{'j', "justifications",	0},
		{'u', "user",			0},
		{0, 0, 0}
	};

	MemoriesBitset options(0);

	for (;;) {
		if (!ProcessOptions(argv, optionsData)) return false;
		if (m_Option == -1) break;

		switch (m_Option) {
			case 'c':
				options.set(MEMORIES_CHUNKS);
				break;
			case 'd':
				options.set(MEMORIES_DEFAULT);
				break;
			case 'j':
				options.set(MEMORIES_JUSTIFICATIONS);
				break;
			case 'u':
				options.set(MEMORIES_USER);
				break;
			default:
				return SetError(CLIError::kGetOptError);
		}
	}

	// Max one additional argument
	if (m_NonOptionArguments > 1) {
		SetErrorDetail("Expected at most one additional argument, either a production or a number.");
		return SetError(CLIError::kTooManyArgs);		
	}

	// It is either a production or a number
	int n = 0;
	if (m_NonOptionArguments == 1) {
		int optind = m_Argument - m_NonOptionArguments;
		if (!IsInteger(argv[optind])) {
			// production
			if (options.any()) return SetError(CLIError::kNoProdTypeWhenProdName);
			return DoMemories(pAgent, options, 0, &argv[optind]);
		}
		// number
		n = atoi(argv[optind].c_str());
		if (n <= 0) return SetError(CLIError::kIntegerMustBePositive);
	}

	// Default to all types when no production and no type specified
	if (options.none()) options.flip();

	// handle production/number cases
	return DoMemories(pAgent, options, n);
}

bool CommandLineInterface::DoMemories(gSKI::IAgent* pAgent, const MemoriesBitset options, int n, const std::string* pProduction) {
	if (!RequireAgent(pAgent)) return false;

	gSKI::IProductionManager* pProductionManager = pAgent->GetProductionManager();
	gSKI::tIProductionIterator* pIter = 0;
	gSKI::IProduction* pProd = 0;
	std::vector< std::pair< std::string, unsigned long > > memories;

	bool foundProduction = false;

	// get either one production or all of them
	if (options.none()) {
		if (!pProduction) return SetError(CLIError::kProductionRequired);
		pIter = pProductionManager->GetProduction(pProduction->c_str());
	} else {
		pIter = pProductionManager->GetAllProductions(false, &m_gSKIError);
		if (gSKI::isError(m_gSKIError)) {
			SetErrorDetail("Unable to get all productions.");
			return SetError(CLIError::kgSKIError);
		}
	}
	if (!pIter) return SetError(CLIError::kgSKIError);

	// walk the iter, looking for productions
	for(; pIter->IsValid(); pIter->Next()) {

		pProd = pIter->GetVal();

		switch (pProd->GetType()) {
			case gSKI_CHUNK:
				if (!options.test(MEMORIES_CHUNKS)) continue;
				break;
			case gSKI_DEFAULT:
				if (!options.test(MEMORIES_DEFAULT)) continue;
				break;
			case gSKI_JUSTIFICATION:
				if (!options.test(MEMORIES_JUSTIFICATIONS)) continue;
				break;
			case gSKI_USER:
				if (!options.test(MEMORIES_USER)) continue;
				break;

			default:
				pProd->Release();
				pIter->Release();
				return SetError(CLIError::kInvalidProductionType);
		}

		foundProduction = true;
		
		// save the tokens/name pair
		std::pair< std::string, unsigned long > memory;
		memory.first = pProd->GetName();
		memory.second = pProd->CountReteTokens();
		memories.push_back(memory);
		pProd->Release();
	}

	pIter->Release();
	pIter = 0;

	if (!foundProduction) return SetError(CLIError::kProductionNotFound);

	// sort them
	MemoriesSort s;
	sort(memories.begin(), memories.end(), s);

	// print them
	char buf[1024];
	int i = 0;
	for (std::vector< std::pair< std::string, unsigned long > >::reverse_iterator j = memories.rbegin(); 
		j != memories.rend() && (n == 0 || i < n); 
		++j, ++i) 
	{
		if (m_RawOutput) {
			snprintf(buf, 1023, "\n%6lu:  %s", j->second, j->first.c_str());
			buf[1023] = 0;
			m_Result << buf;
		} else {
			AppendArgTagFast(sml_Names::kParamName, sml_Names::kTypeString, j->first.c_str());
			AppendArgTagFast(sml_Names::kParamCount, sml_Names::kTypeInt, Int2String(j->second, buf, 1024));
		}
	}
	return true;
}

