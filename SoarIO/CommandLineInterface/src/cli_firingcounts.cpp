#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include <algorithm>

#include "cli_Constants.h"

#include "IgSKI_Agent.h"
#include "IgSKI_ProductionManager.h"
#include "IgSKI_Production.h"

#ifdef _MSC_VER
#define snprintf _snprintf 
#endif // _MSC_VER

using namespace cli;

struct FiringsSort {
	bool operator()(std::pair< std::string, unsigned long > a, std::pair< std::string, unsigned long > b) const {
		return a.second < b.second;
	}
};

bool CommandLineInterface::ParseFiringCounts(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {

	// The number to list defaults to -1 (list all)
	int numberToList = -1;

	// Production defaults to no production
	std::string* pProduction = 0;

	if (argv.size() == 2) {
		// one argument, figure out if it is a non-negative integer
		if (argv[1][0] == '-') {
			return HandleSyntaxError(Constants::kCLIFiringCounts, "Integer argument must be non-negative");
		}
		if (isdigit(argv[1][0])) {
			// integer argument, set numberToList
			numberToList = atoi(argv[1].c_str());
			if (numberToList < 0) {
				return HandleSyntaxError(Constants::kCLIFiringCounts, "Integer argument must be non-negative");
			}
		} else {
			// non-integer argument, hopfully a production
			pProduction = &(argv[1]);
		}
	} else if (argv.size() > 2) {
		return HandleSyntaxError(Constants::kCLIFiringCounts, Constants::kCLITooManyArgs);
	}

	return DoFiringCounts(pAgent, pProduction, numberToList);
}

bool CommandLineInterface::DoFiringCounts(gSKI::IAgent* pAgent, std::string* pProduction, int numberToList) {
	if (!RequireAgent(pAgent)) return false;

	gSKI::IProductionManager* pProductionManager = pAgent->GetProductionManager();
	gSKI::tIProductionIterator* pIter = 0;
	gSKI::IProduction* pProd = 0;
	std::vector< std::pair< std::string, unsigned long > > firings;

	bool foundProduction = false;

	if (pProduction) {
		pIter = pProductionManager->GetProduction(pProduction->c_str());
	} else {
		pIter = pProductionManager->GetAllProductions(m_pError);
	}

	if (!pIter) return HandleError("Production manager returned NULL iterator.", m_pError);

	while(pIter->IsValid()) {

		foundProduction = true;

		pProd = pIter->GetVal();

		if (!numberToList) {
			if (pProd->GetFiringCount()) {
				pProd->Release();
				pIter->Next();
				continue;
			}
		}

		std::pair< std::string, unsigned long > firing;
		firing.first = pProd->GetName();
		firing.second = pProd->GetFiringCount();
		firings.push_back(firing);

		pProd->Release();

		pIter->Next();
	}

	pIter->Release();
	pIter = 0;

	if (!foundProduction) {
		AppendToResult("No productions found.");
		return true;
	}

	FiringsSort s;
	sort(firings.begin(), firings.end(), s);
	char buf[1024];
	int i = 0;
	for (std::vector< std::pair< std::string, unsigned long > >::reverse_iterator j = firings.rbegin(); 
		j != firings.rend() && (numberToList <= 0 || i < numberToList); 
		++j, ++i) {
		snprintf(buf, 1023, "%6lu:  %s\n", j->second, j->first.c_str());
		AppendToResult(buf);
	}
	return true;
}

