/////////////////////////////////////////////////////////////////
// sp command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include "portability.h"

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"
#include "sml_AgentSML.h"

#include "agent.h"
#include "production.h"
#include "symtab.h"
#include "rete.h"
#include "parser.h"

using namespace cli;

bool CommandLineInterface::DoSP(const std::string& productionString)
{
    // Load the production
    agent* thisAgent = m_pAgentSML->GetSoarAgent();
    set_lexer_input(thisAgent, productionString.c_str());
    set_lexer_allow_ids(thisAgent, false);
    get_lexeme(thisAgent);

    production* p;
    unsigned char rete_addition_result = 0;
    p = parse_production(thisAgent, &rete_addition_result);

    set_lexer_allow_ids(thisAgent, true);
    set_lexer_input( thisAgent, NULL);

    if (!p)
    {
        // There was an error, but duplicate production is just a warning
        if (rete_addition_result != DUPLICATE_PRODUCTION)
        {
            return SetError("Production addition failed.");
        }
        // production ignored
        m_NumProductionsIgnored += 1;
    }
    else
    {
        if (!m_SourceFileStack.empty())
        {
            p->filename = make_memory_block_for_string(thisAgent, m_SourceFileStack.top().c_str());
        }

        // production was sourced
        m_NumProductionsSourced += 1;
        if (m_RawOutput)
        {
            m_Result << '*';
        }
    }
    return true;
}

