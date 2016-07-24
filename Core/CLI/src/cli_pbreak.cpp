/////////////////////////////////////////////////////////////////
// break command file.
//
// Author: Mitchell Keith Bloch, bazald@umich.edu
// Date  : 2014
//
/////////////////////////////////////////////////////////////////

#include "portability.h"

#include "sml_Utils.h"
#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"

#include <assert.h>

#include "sml_Names.h"

#include "sml_KernelSML.h"
#include "gsysparam.h"
#include "rete.h"
#include "sml_AgentSML.h"
#include "symbol.h"
#include "production.h"
#include "agent.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoPbreak(const char& mode, const std::string& production)
{
    agent* thisAgent = m_pAgentSML->GetSoarAgent();
    
    if (mode == 's' || mode == 'c')
    {
        Symbol sym = find_str_constant(thisAgent, production.c_str());
        rete_node* prod = (sym && sym->sc->production) ? sym->sc->production->p_node : 0;
        
        if (!prod)
        {
            return SetError("Production not found: " + production);
        }
        
        if (mode == 's' && !sym->sc->production->interrupt)
        {
            sym->sc->production->interrupt = true;
            sym->sc->production->interrupt_break = true;
        }
        else if (mode == 'c' && sym->sc->production->interrupt)
        {
            sym->sc->production->interrupt = false;
            sym->sc->production->interrupt_break = false;
        }
    }
    else
    {
        assert(mode == 'p');
        
        for (int i = 0; i != NUM_PRODUCTION_TYPES; ++i)
        {
            for (struct production_struct* prod = thisAgent->all_productions_of_type[i]; prod; prod = prod->next)
            {
                if (prod->interrupt_break)
                {
                    m_Result << prod->name->sc->name << std::endl;
                }
            }
        }
    }
    
    // Transfer the result from m_XMLResult into pResponse
    // We pass back the name of the command we just executed which becomes the tag name
    // used in the resulting XML.
    if (!m_RawOutput)
    {
        XMLResultToResponse("break") ;
    }
    
    return true;
}
