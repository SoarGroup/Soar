/////////////////////////////////////////////////////////////////
// excise command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include "portability.h"

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"

#include "sml_Names.h"
#include "sml_AgentSML.h"

#include "agent.h"
#include "production.h"
#include "symtab.h"

#include "reinforcement_learning.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoExcise(const ExciseBitset& options, const std::string* pProduction)
{
    int64_t exciseCount = 0;
    agent* thisAgent = m_pAgentSML->GetSoarAgent();
    
    // Process the general options
    if (options.test(EXCISE_ALL))
    {
        exciseCount += thisAgent->num_productions_of_type[USER_PRODUCTION_TYPE];
        exciseCount += thisAgent->num_productions_of_type[CHUNK_PRODUCTION_TYPE];
        exciseCount += thisAgent->num_productions_of_type[JUSTIFICATION_PRODUCTION_TYPE];
        exciseCount += thisAgent->num_productions_of_type[DEFAULT_PRODUCTION_TYPE];
        
        excise_all_productions(thisAgent, false);
        
        this->DoInitSoar();    // from the manual, init when --all or --task are executed
    }
    if (options.test(EXCISE_CHUNKS))
    {
        exciseCount += thisAgent->num_productions_of_type[CHUNK_PRODUCTION_TYPE];
        exciseCount += thisAgent->num_productions_of_type[JUSTIFICATION_PRODUCTION_TYPE];
        
        excise_all_productions_of_type(thisAgent, CHUNK_PRODUCTION_TYPE, false);
        excise_all_productions_of_type(thisAgent, JUSTIFICATION_PRODUCTION_TYPE, false);
    }
    if (options.test(EXCISE_DEFAULT))
    {
        exciseCount += thisAgent->num_productions_of_type[DEFAULT_PRODUCTION_TYPE];
        
        excise_all_productions_of_type(thisAgent, DEFAULT_PRODUCTION_TYPE, false);
    }
    if (options.test(EXCISE_RL))
    {
        for (production* prod = thisAgent->all_productions_of_type[DEFAULT_PRODUCTION_TYPE]; prod != NIL; prod = prod->next)
        {
            if (prod->rl_rule)
            {
                exciseCount++;
                excise_production(thisAgent, prod, static_cast<bool>(thisAgent->sysparams[TRACE_LOADING_SYSPARAM]));
            }
        }
        
        for (production* prod = thisAgent->all_productions_of_type[USER_PRODUCTION_TYPE]; prod != NIL; prod = prod->next)
        {
            if (prod->rl_rule)
            {
                exciseCount++;
                excise_production(thisAgent, prod, static_cast<bool>(thisAgent->sysparams[TRACE_LOADING_SYSPARAM]));
            }
        }
        
        for (production* prod = thisAgent->all_productions_of_type[CHUNK_PRODUCTION_TYPE]; prod != NIL; prod = prod->next)
        {
            if (prod->rl_rule)
            {
                exciseCount++;
                excise_production(thisAgent, prod, static_cast<bool>(thisAgent->sysparams[TRACE_LOADING_SYSPARAM]));
            }
        }
        
        rl_initialize_template_tracking(thisAgent);
    }
    if (options.test(EXCISE_TASK))
    {
        exciseCount += thisAgent->num_productions_of_type[USER_PRODUCTION_TYPE];
        exciseCount += thisAgent->num_productions_of_type[DEFAULT_PRODUCTION_TYPE];
        
        excise_all_productions_of_type(thisAgent, USER_PRODUCTION_TYPE, false);
        excise_all_productions_of_type(thisAgent, DEFAULT_PRODUCTION_TYPE, false);
        
        this->DoInitSoar();    // from the manual, init when --all or --task are executed
    }
    if (options.test(EXCISE_TEMPLATE))
    {
        exciseCount += thisAgent->num_productions_of_type[TEMPLATE_PRODUCTION_TYPE];
        
        excise_all_productions_of_type(thisAgent, TEMPLATE_PRODUCTION_TYPE, false);
    }
    if (options.test(EXCISE_USER))
    {
        exciseCount += thisAgent->num_productions_of_type[USER_PRODUCTION_TYPE];
        
        excise_all_productions_of_type(thisAgent, USER_PRODUCTION_TYPE, false);
    }
    
    // Excise specific production
    if (pProduction)
    {
        Symbol* sym = find_str_constant(thisAgent, pProduction->c_str());
        
        if (!sym || !(sym->sc->production))
        {
            return SetError("Production not found.");
        }
        
        if (!m_RawOutput)
        {
            // Save the name for the structured response
            AppendArgTagFast(sml_Names::kParamName, sml_Names::kTypeString, *pProduction);
        }
        
        // Increment the count for the structured response
        ++exciseCount;
        
        excise_production(thisAgent, sym->sc->production, false);
    }
    
    if (m_RawOutput)
    {
        m_Result << exciseCount << " production" << (exciseCount == 1 ? " " : "s ") << "excised.\n";
    }
    else
    {
        // Add the count tag to the front
        std::string temp;
        PrependArgTag(sml_Names::kParamCount, sml_Names::kTypeInt, to_string(exciseCount, temp));
    }
    
    return true;
}
