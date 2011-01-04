/////////////////////////////////////////////////////////////////
// pwatch command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "sml_Utils.h"
#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"

#include "sml_Names.h"

#include "sml_KernelSML.h"
#include "sml_AgentSML.h"

#include "agent.h"
#include "production.h"
#include "symtab.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoPWatch(bool query, const std::string* pProduction, bool setting) {
    agent* agnt = m_pAgentSML->GetSoarAgent();
    // check for query or not production 
    if (query || !pProduction) {
        // list all productions currently being traced
        production* pSoarProduction = 0;
        int productionCount = 0;
        for(unsigned int i = 0; i < NUM_PRODUCTION_TYPES; ++i)
        {
            for( pSoarProduction = agnt->all_productions_of_type[i]; 
                pSoarProduction != 0; 
                pSoarProduction = pSoarProduction->next )
            {
                // is it being watched
                if ( pSoarProduction->trace_firings ) {

                    if (query) 
                    {
                        ++productionCount;
                        if (m_RawOutput) 
                        {
                            m_Result << '\n' << pSoarProduction->name->sc.name;
                        } else {
                            AppendArgTagFast(sml_Names::kParamName, sml_Names::kTypeString, pSoarProduction->name->sc.name);
                        }
                    }
                    else
                    {
                        // not querying, shut it off
                        remove_pwatch( agnt, pSoarProduction );
                    }
                }

            }
        }

        if ( query )
        {
            // we're querying, summarize
            if (m_RawOutput) {
                if (!productionCount) {
                    m_Result << "No watched productions found.";
                }
            } else if (!m_RawOutput) {
                std::stringstream buffer;
                buffer << productionCount;
                PrependArgTagFast( sml_Names::kParamCount, sml_Names::kTypeInt, buffer.str() );
            }
        }

        return true;
    }

    Symbol* sym = find_sym_constant( agnt, pProduction->c_str() );

    if (!sym || !(sym->sc.production))
    {
        return SetError("Production not found.");
    }

    // we have a production
    if (setting) {
        add_pwatch( agnt, sym->sc.production );
    } else {
        remove_pwatch( agnt, sym->sc.production );
    }
    return true;
}

