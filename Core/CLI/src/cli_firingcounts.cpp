/////////////////////////////////////////////////////////////////
// firingcounts command file.
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

#include "gSKI_Agent.h"
#include "gSKI_ProductionManager.h"
#include "IgSKI_Production.h"

#ifdef _MSC_VER
#define snprintf _snprintf 
#endif // _MSC_VER

using namespace cli;
using namespace sml;

struct FiringsSort {
	bool operator()(std::pair< std::string, unsigned long > a, std::pair< std::string, unsigned long > b) const {
		return a.second < b.second;
	}
};

bool CommandLineInterface::ParseFiringCounts(gSKI::Agent* pAgent, std::vector<std::string>& argv) {

	// The number to list defaults to -1 (list all)
	int numberToList = -1;

	// Production defaults to no production
	std::string* pProduction = 0;

	// no more than 1 arg
	if (argv.size() > 2) return SetError(CLIError::kTooManyArgs);

	if (argv.size() == 2) {
		// one argument, figure out if it is a non-negative integer or a production
		if (IsInteger(argv[1])) {
			numberToList = atoi(argv[1].c_str());
			if (numberToList < 0) return SetError(CLIError::kIntegerMustBeNonNegative);

		} else {
			// non-integer argument, hopfully a production
			pProduction = &(argv[1]);
		}
	}

	return DoFiringCounts(pAgent, numberToList, pProduction);
}

bool CommandLineInterface::DoFiringCounts(gSKI::Agent* pAgent, const int numberToList, const std::string* pProduction) {
	if (!RequireAgent(pAgent)) return false;

	// get the production stuff
	gSKI::ProductionManager* pProductionManager = pAgent->GetProductionManager();
	gSKI::tIProductionIterator* pIter = 0;
	gSKI::IProduction* pProd = 0;
	std::vector< std::pair< std::string, unsigned long > > firings;

	bool foundProduction = false;

	// if we have a production, just get that one, otherwise get them all
	if (pProduction) {
		pIter = pProductionManager->GetProduction(pProduction->c_str());
		if (gSKI::isError(m_gSKIError)) {
			SetErrorDetail("Unable to get production: " + *pProduction);
			return SetError(CLIError::kgSKIError);
		}
	} else {
		pIter = pProductionManager->GetAllProductions(false, &m_gSKIError);
		if (gSKI::isError(m_gSKIError)) {
			SetErrorDetail("Unable to get all productions.");
			return SetError(CLIError::kgSKIError);
		}
	}
	if (!pIter) {
		SetErrorDetail("Unable to get production(s).");
		return SetError(CLIError::kgSKIError);
	}

	// walk with the iter and 
	for(; pIter->IsValid(); pIter->Next()) {

		pProd = pIter->GetVal();

		// if numberToList is 0, only list those who haven't fired
		if (!numberToList) {
			if (pProd->GetFiringCount()) {
				// this one has fired, skip it
				pProd->Release();
				continue;
			}
		}

		foundProduction = true;

		// store the name and count
		std::pair< std::string, unsigned long > firing;
		firing.first = pProd->GetName();
		firing.second = pProd->GetFiringCount();
		firings.push_back(firing);

		pProd->Release();
	}

	pIter->Release();
	pIter = 0;

	if (!foundProduction) return SetError(CLIError::kProductionNotFound);

	// Sort the list
	FiringsSort s;
	sort(firings.begin(), firings.end(), s);

	// print the list
	char buf[1024];
	int i = 0;
	for (std::vector< std::pair< std::string, unsigned long > >::reverse_iterator j = firings.rbegin(); 
		j != firings.rend() && (numberToList <= 0 || i < numberToList); 
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

