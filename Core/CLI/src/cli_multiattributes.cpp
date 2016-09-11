/////////////////////////////////////////////////////////////////
// multi-attributes command file.
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
#include "memory_manager.h"
#include "production.h"
#include "print.h"
#include "symbol.h"
#include "symbol_manager.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoMultiAttributes(const std::string* pAttribute, int n)
{
    agent* thisAgent = m_pAgentSML->GetSoarAgent();
    multi_attribute* maList = thisAgent->multi_attributes;

    if (!pAttribute && !n)
    {

        // No args, print current setting
        int count = 0;

        if (!maList)
        {
            m_Result << "No multi-attributes found.";
        }

        std::stringstream buffer;

        if (m_RawOutput)
        {
            m_Result << "Value\tSymbol";
        }

        while (maList)
        {
            // Arbitrary buffer and size
            char attributeName[1024];
            maList->symbol->to_string(true, attributeName, 1024);

            if (m_RawOutput)
            {
                m_Result  << maList->value << "\t" << maList->symbol->to_string(true, attributeName, 1024) << std::endl;

            }
            else
            {
                buffer << maList->value;
                // Value
                AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeInt, buffer.str());
                buffer.clear();

                // Symbol
                AppendArgTagFast(sml_Names::kParamName, sml_Names::kTypeString, attributeName);
            }

            ++count;

            maList = maList->next;
        }

        buffer << count;
        if (!m_RawOutput)
        {
            PrependArgTagFast(sml_Names::kParamCount, sml_Names::kTypeInt, buffer.str());
        }
        return true;
    }

    // Setting defaults to 10
    if (!n)
    {
        n = 10;
    }

    // Set it
    Symbol* s = thisAgent->symbolManager->make_str_constant(pAttribute->c_str());

    while (maList)
    {
        if (maList->symbol == s)
        {
            maList->value = n;
            thisAgent->symbolManager->symbol_remove_ref(&s);
            return true;
        }

        maList = maList->next;
    }

    /* sym wasn't in the table if we get here, so add it */
    maList = static_cast<multi_attribute*>(thisAgent->memoryManager->allocate_memory(sizeof(multi_attribute), MISCELLANEOUS_MEM_USAGE));
    assert(maList);

    maList->value = n;
    maList->symbol = s;
    maList->next = thisAgent->multi_attributes;
    thisAgent->multi_attributes = maList;

    return true;
}

