/////////////////////////////////////////////////////////////////
// firingcounts command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include "portability.h"

#include "cli_CommandLineInterface.h"

#include <algorithm>

#include "cli_Commands.h"

#include "sml_Names.h"
#include "sml_AgentSML.h"

#include "agent.h"
#include "production.h"
#include "symtab.h"

using namespace cli;
using namespace sml;

struct FiringsSort
{
    bool operator()(std::pair< std::string, uint64_t > a, std::pair< std::string, uint64_t > b) const
    {
        return a.second < b.second;
    }
};

void  add_prods_of_type_to_fc_list(agent* thisAgent,
                                   unsigned int productionType, bool printThisType, bool printRL,
                                   const int numberToList,
                                   std::vector< std::pair< std::string, uint64_t > > *firings)
{
    if (printThisType || printRL)
    {
        for (production* pSoarProduction = thisAgent->all_productions_of_type[productionType];
            pSoarProduction != 0;
            pSoarProduction = pSoarProduction->next)
        {
            if (!numberToList)
            {
                if (pSoarProduction->firing_count)
                {
                    // this one has fired, skip it
                    continue;
                }
            }

            if (printThisType || (printRL && pSoarProduction->rl_rule))
            {
                if (!printRL && pSoarProduction->rl_rule)
                {
                    continue;
                }
                // store the name and count
                std::pair< std::string, uint64_t > firing;
                firing.first = pSoarProduction->name->sc->name;
                firing.second = pSoarProduction->firing_count;
                firings->push_back(firing);
            }
        }
    }
}

bool CommandLineInterface::DoFiringCounts(PrintBitset options, const int numberToList, const std::string* pProduction)
{
    std::vector< std::pair< std::string, uint64_t > > firings;
    agent* thisAgent = m_pAgentSML->GetSoarAgent();

    // if we have a production, just get that one, otherwise get them all
    if (pProduction && !pProduction->empty())
    {
        Symbol* sym = find_str_constant(thisAgent, pProduction->c_str());

        if (!sym || !(sym->sc->production))
        {
            return SetError("Production not found.");
        }

        std::pair< std::string, uint64_t > firing;
        firing.first = *pProduction;
        firing.second = sym->sc->production->firing_count;
        firings.push_back(firing);
    }
    else
    {
        bool foundProduction = false;
        bool printRL = options.test(PRINT_RL);

        // Print all productions if there are no other production flags set
        if (options.test(PRINT_ALL) ||
                (!options.test(PRINT_CHUNKS) &&
                 !options.test(PRINT_DEFAULTS) &&
                 !options.test(PRINT_JUSTIFICATIONS) &&
                 !options.test(PRINT_USER) &&
                 !printRL &&
                 !options.test(PRINT_TEMPLATE)))
        {
            options.set(PRINT_CHUNKS);
            options.set(PRINT_DEFAULTS);
            options.set(PRINT_JUSTIFICATIONS);
            options.set(PRINT_USER);
            options.set(PRINT_TEMPLATE);
            printRL = true;
        }

        add_prods_of_type_to_fc_list(thisAgent, CHUNK_PRODUCTION_TYPE,
            options.test(PRINT_CHUNKS), printRL, numberToList, &firings);
        add_prods_of_type_to_fc_list(thisAgent, DEFAULT_PRODUCTION_TYPE,
            options.test(PRINT_DEFAULTS), printRL, numberToList, &firings);
        add_prods_of_type_to_fc_list(thisAgent, JUSTIFICATION_PRODUCTION_TYPE,
            options.test(PRINT_JUSTIFICATIONS), printRL, numberToList, &firings);
        add_prods_of_type_to_fc_list(thisAgent, USER_PRODUCTION_TYPE,
            options.test(PRINT_USER), printRL, numberToList, &firings);
        add_prods_of_type_to_fc_list(thisAgent, TEMPLATE_PRODUCTION_TYPE,
            options.test(PRINT_TEMPLATE), printRL, numberToList, &firings);

        if (firings.empty())
        {
            return SetError("No productions of that type found.");
        }
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
        if (m_RawOutput)
        {
            m_Result << std::setw(6) << j->second << ":  " << j->first << "\n";
        }
        else
        {
            std::string temp;
            AppendArgTagFast(sml_Names::kParamName, sml_Names::kTypeString, j->first);
            AppendArgTagFast(sml_Names::kParamCount, sml_Names::kTypeInt, to_string(j->second, temp));
        }
    }
    return true;
}
