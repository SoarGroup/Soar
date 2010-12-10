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

#include "sml_Names.h"

#include "agent.h"
#include "production.h"
#include "symtab.h"

using namespace cli;
using namespace sml;

struct FiringsSort {
	bool operator()(std::pair< std::string, uint64_t > a, std::pair< std::string, uint64_t > b) const {
		return a.second < b.second;
	}
};

bool CommandLineInterface::ParseFiringCounts(std::vector<std::string>& argv) {

	// The number to list defaults to -1 (list all)
	int numberToList = -1;

	// Production defaults to no production
	std::string* pProduction = 0;

	// no more than 1 arg
	if (argv.size() > 2) return SetError(kTooManyArgs);

	if (argv.size() == 2) {
		// one argument, figure out if it is a non-negative integer or a production
		if ( from_string( numberToList, argv[1] ) ){
			if (numberToList < 0) return SetError(kIntegerMustBeNonNegative);

		} else {
			numberToList = -1;

			// non-integer argument, hopfully a production
			pProduction = &(argv[1]);
		}
	}

	return DoFiringCounts(numberToList, pProduction);
}

bool CommandLineInterface::DoFiringCounts(const int numberToList, const std::string* pProduction) {
	std::vector< std::pair< std::string, uint64_t > > firings;

	// if we have a production, just get that one, otherwise get them all
	if (pProduction) 
	{
		Symbol* sym = find_sym_constant( m_pAgentSoar, pProduction->c_str() );

		if (!sym || !(sym->sc.production))
		{
			return SetError(kProductionNotFound);
		}

		std::pair< std::string, uint64_t > firing;
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
				std::pair< std::string, uint64_t > firing;
				firing.first = pSoarProduction->name->sc.name;
				firing.second = pSoarProduction->firing_count;
				firings.push_back(firing);
			}
		}
	
		if (!foundProduction) return SetError(kProductionNotFound);
	}

	// Sort the list
	FiringsSort s;
	sort(firings.begin(), firings.end(), s);

	// print the list
	int i = 0;
	for (std::vector< std::pair< std::string, uint64_t > >::reverse_iterator j = firings.rbegin(); 
		j != firings.rend() && (numberToList <= 0 || i < numberToList); 
		++j, ++i) 
	{
		if (m_RawOutput) {
			m_Result << "\n"<< std::setw(6) << j->second << ":  " << j->first;
		} else {
			std::string temp;
			AppendArgTagFast(sml_Names::kParamName, sml_Names::kTypeString, j->first);
			AppendArgTagFast(sml_Names::kParamCount, sml_Names::kTypeInt, to_string(j->second, temp));
		}
	}
	return true;
}

