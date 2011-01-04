/////////////////////////////////////////////////////////////////
// sp command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"
#include "sml_AgentSML.h"

#include "agent.h"
#include "production.h"
#include "symtab.h"
#include "rete.h"
#include "parser.h"

using namespace cli;

// FIXME: copied from gSKI
void soarAlternateInput(agent *ai_agent, const char  *ai_string, char  *ai_suffix, Bool   ai_exit   )
{
    // Side effects:
    //    The soar agents alternate input values are updated and its
    //      current character is reset to a whitespace value.
    ai_agent->alternate_input_string = const_cast<char*>(ai_string);
    ai_agent->alternate_input_suffix = ai_suffix;
    ai_agent->current_char = ' ';
    ai_agent->alternate_input_exit = ai_exit;
    return;
}

bool CommandLineInterface::DoSP(const std::string& productionString) {
    // Load the production
    // voigtjr: note: this TODO from gSKI:
    // TODO: This should not be needed, FIX!
    // contents of gSKI ProductionManager::soarAlternateInput function:
    agent* agnt = m_pAgentSML->GetSoarAgent();
    soarAlternateInput( agnt, productionString.c_str(), const_cast<char*>(") "), true );
    set_lexer_allow_ids( agnt, false );
    get_lexeme( agnt );

    production* p;
    unsigned char rete_addition_result = 0;
    p = parse_production( agnt, &rete_addition_result );

    set_lexer_allow_ids( agnt, true );
    soarAlternateInput( agnt, 0, 0, true ); 

    if (!p) { 
        // There was an error, but duplicate production is just a warning
        if (rete_addition_result != DUPLICATE_PRODUCTION) {
          return SetError("Production addition failed.");
        }
        // production ignored
        m_NumProductionsIgnored += 1;
    } else {
        if (!m_SourceFileStack.empty())
            p->filename = make_memory_block_for_string(agnt, m_SourceFileStack.top().c_str());

        // production was sourced
        m_NumProductionsSourced += 1;
        if (m_RawOutput) {
            m_Result << '*';
        }
    }
    return true;
}

