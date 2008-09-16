/////////////////////////////////////////////////////////////////
// firingcounts command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "cli_CommandLineInterface.h"

#include <algorithm>

#include "cli_Commands.h"
#include "cli_CLIError.h"

#include "sml_Names.h"
#include "sml_StringOps.h"

#include "agent.h"
#include "production.h"
#include "symtab.h"

using namespace cli;
using namespace sml;

struct FiringsSort {
	bool operator()(std::pair< std::string, unsigned long > a, std::pair< std::string, unsigned long > b) const {
		return a.second < b.second;
	}
};

bool CommandLineInterface::ParseFiringCounts(std::vector<std::string>& argv) {

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

	return DoFiringCounts(numberToList, pProduction);
}

bool CommandLineInterface::DoFiringCounts(const int numberToList, const std::string* pProduction) {
	std::vector< std::pair< std::string, unsigned long > > firings;

	// if we have a production, just get that one, otherwise get them all
	if (pProduction) 
	{
		Symbol* sym = find_sym_constant( m_pAgentSoar, pProduction->c_str() );

		if (!sym || !(sym->sc.production))
		{
			return SetError(CLIError::kProductionNotFound);
		}

		std::pair< std::string, unsigned long > firing;
		firing.first = *pProduction;
		firing.second = sym->sc.production->firing_count;
		firings.push_back(firing);
	} 
	else 
	{
		bool foundProduction = false;

		for(unsigned int i = 0; i < NUM_PRODUCTION_TYPES; ++i)
		{
			for( production* pSoarProduction = m_pAgentSoar->all_productions_of_type[i]; 
				pSoarProduction != 0; 
				pSoarProduction = pSoarProduction->next )
			{
				if (!numberToList) {
					if ( pSoarProduction->firing_count ) {
						// this one has fired, skip it
						continue;
					}
				}

				foundProduction = true;

				// store the name and count
				std::pair< std::string, unsigned long > firing;
				firing.first = pSoarProduction->name->sc.name;
				firing.second = pSoarProduction->firing_count;
				firings.push_back(firing);
			}
		}
	
		if (!foundProduction) return SetError(CLIError::kProductionNotFound);
	}

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
			SNPRINTF(buf, 1023, "\n%6lu:  %s", j->second, j->first.c_str());
			buf[1023] = 0;
			m_Result << buf;
		} else {
			AppendArgTagFast(sml_Names::kParamName, sml_Names::kTypeString, j->first.c_str());
			AppendArgTagFast(sml_Names::kParamCount, sml_Names::kTypeInt, Int2String(j->second, buf, 1024));
		}
	}
	return true;
}

