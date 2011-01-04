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
#include "sml_AgentSML.h"

#include "agent.h"
#include "production.h"
#include "symtab.h"
#include "rete.h"

using namespace cli;
using namespace sml;

struct MemoriesSort {
    bool operator()(std::pair< std::string, uint64_t > a, std::pair< std::string, uint64_t > b) const {
        return a.second < b.second;
    }
};

bool CommandLineInterface::DoMemories(const MemoriesBitset options, int n, const std::string* pProduction) {
    std::vector< std::pair< std::string, uint64_t > > memories;
    agent* agnt = m_pAgentSML->GetSoarAgent();

    // get either one production or all of them
    if (options.none()) {
        if (!pProduction)
        {
            return SetError("Production required.");
        }

        Symbol* sym = find_sym_constant( agnt, pProduction->c_str() );

        if (!sym || !(sym->sc.production))
        {
            return SetError("Production not found.");
        }

        // save the tokens/name pair
        std::pair< std::string, uint64_t > memory;
        memory.first = *pProduction;
        memory.second = count_rete_tokens_for_production(agnt, sym->sc.production);
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

            for( production* pSoarProduction = agnt->all_productions_of_type[i]; 
                pSoarProduction != 0; 
                pSoarProduction = pSoarProduction->next )
            {
                foundProduction = true;
                
                // save the tokens/name pair
                std::pair< std::string, uint64_t > memory;
                memory.first = pSoarProduction->name->sc.name;
                memory.second = count_rete_tokens_for_production(agnt, pSoarProduction);
                memories.push_back(memory);
            }
        }
    
        if (!foundProduction) return SetError("Production not found.");
    }

    // sort them
    MemoriesSort s;
    sort(memories.begin(), memories.end(), s);

    // print them
    int i = 0;
    for (std::vector< std::pair< std::string, uint64_t > >::reverse_iterator j = memories.rbegin(); 
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
