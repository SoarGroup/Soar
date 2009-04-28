/////////////////////////////////////////////////////////////////
// memories command file.
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
#include "cli_CLIError.h"

#include "agent.h"
#include "production.h"
#include "symtab.h"
#include "rete.h"

using namespace cli;
using namespace sml;

struct MemoriesSort {
	bool operator()(std::pair< std::string, unsigned long > a, std::pair< std::string, unsigned long > b) const {
		return a.second < b.second;
	}
};

bool CommandLineInterface::ParseMemories(std::vector<std::string>& argv) {

	Options optionsData[] = {
		{'c', "chunks",			OPTARG_NONE},
		{'d', "default",		OPTARG_NONE},
		{'j', "justifications",	OPTARG_NONE},
		{'T', "template",		OPTARG_NONE},
		{'u', "user",			OPTARG_NONE},
		{0, 0, OPTARG_NONE}
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
			case 'T':
				options.set(MEMORIES_TEMPLATES);
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
		if ( from_string( n, argv[optind] ) ) {
			// number
			if (n <= 0) return SetError(CLIError::kIntegerMustBePositive);
		} else {
			// production
			if (options.any()) return SetError(CLIError::kNoProdTypeWhenProdName);
			return DoMemories(options, 0, &argv[optind]);
		}
	}

	// Default to all types when no production and no type specified
	if (options.none()) options.flip();

	// handle production/number cases
	return DoMemories(options, n);
}

bool CommandLineInterface::DoMemories(const MemoriesBitset options, int n, const std::string* pProduction) {
	std::vector< std::pair< std::string, unsigned long > > memories;

	// get either one production or all of them
	if (options.none()) {
		if (!pProduction)
		{
			return SetError(CLIError::kProductionRequired);
		}

		Symbol* sym = find_sym_constant( m_pAgentSoar, pProduction->c_str() );

		if (!sym || !(sym->sc.production))
		{
			return SetError(CLIError::kProductionNotFound);
		}

		// save the tokens/name pair
		std::pair< std::string, unsigned long > memory;
		memory.first = *pProduction;
		memory.second = count_rete_tokens_for_production(m_pAgentSoar, sym->sc.production);
		memories.push_back(memory);

	} else {
		bool foundProduction = false;

		for(unsigned int i = 0; i < NUM_PRODUCTION_TYPES; ++i)
		{
			// if filter is set, skip types that are not specified
			if (!options.none()) {
				switch ( i )
				{
				case USER_PRODUCTION_TYPE:
					if ( !options.test(MEMORIES_USER) ) 
					{
						continue;
					}
					break;

				case DEFAULT_PRODUCTION_TYPE:
					if ( !options.test(MEMORIES_DEFAULT) ) 
					{
						continue;
					}
					break;

				case CHUNK_PRODUCTION_TYPE:
					if ( !options.test(MEMORIES_CHUNKS) ) 
					{
						continue;
					}
					break;

				case JUSTIFICATION_PRODUCTION_TYPE:
					if ( !options.test(MEMORIES_JUSTIFICATIONS) ) 
					{
						continue;
					}
					break;

				case TEMPLATE_PRODUCTION_TYPE:
					if ( !options.test(MEMORIES_TEMPLATES) ) 
					{
						continue;
					}
					break;

				default:
					assert(false);
					break;
				}
			}

			for( production* pSoarProduction = m_pAgentSoar->all_productions_of_type[i]; 
				pSoarProduction != 0; 
				pSoarProduction = pSoarProduction->next )
			{
				foundProduction = true;
				
				// save the tokens/name pair
				std::pair< std::string, unsigned long > memory;
				memory.first = pSoarProduction->name->sc.name;
				memory.second = count_rete_tokens_for_production(m_pAgentSoar, pSoarProduction);
				memories.push_back(memory);
			}
		}
	
		if (!foundProduction) return SetError(CLIError::kProductionNotFound);
	}

	// sort them
	MemoriesSort s;
	sort(memories.begin(), memories.end(), s);

	// print them
	int i = 0;
	for (std::vector< std::pair< std::string, unsigned long > >::reverse_iterator j = memories.rbegin(); 
		j != memories.rend() && (n == 0 || i < n); 
		++j, ++i) 
	{
		if (m_RawOutput) {
			m_Result << "\n" << std::setw(6) << j->second << ":  " << j->first;
		} else {
			std::string temp;
			AppendArgTagFast(sml_Names::kParamName, sml_Names::kTypeString, j->first);
			AppendArgTagFast(sml_Names::kParamCount, sml_Names::kTypeInt, to_string(j->second, temp));
		}
	}
	return true;
}
