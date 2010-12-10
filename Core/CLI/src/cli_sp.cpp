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

#include "agent.h"
#include "production.h"
#include "symtab.h"
#include "rete.h"
#include "parser.h"

using namespace cli;

bool CommandLineInterface::ParseSP(std::vector<std::string>& argv) {
	// One argument (the stuff in the brackets, minus the brackets
	if (argv.size() < 2) {
		return SetError(kTooFewArgs);
	}
	if (argv.size() > 2) {
		SetErrorDetail("Expected one argument (the production) enclosed in braces.");
		return SetError(kTooManyArgs);
	}

	return DoSP(argv[1]);
}

// FIXME: copied from gSKI
void soarAlternateInput(agent *ai_agent, const char  *ai_string, char  *ai_suffix, Bool   ai_exit   )
{
	// Side effects:
	//	The soar agents alternate input values are updated and its
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
	soarAlternateInput( m_pAgentSoar, productionString.c_str(), const_cast<char*>(") "), true );
	set_lexer_allow_ids( m_pAgentSoar, false );
	get_lexeme( m_pAgentSoar );

	production* p;
	unsigned char rete_addition_result = 0;
	p = parse_production( m_pAgentSoar, &rete_addition_result );

	set_lexer_allow_ids( m_pAgentSoar, true );
	soarAlternateInput( m_pAgentSoar, 0, 0, true ); 

	if (!p) { 
		// There was an error, but duplicate production is just a warning
		if (rete_addition_result != DUPLICATE_PRODUCTION) {
		  return SetError( kProductionAddFailed );
		}
		// production ignored
		m_NumProductionsIgnored += 1;
	} else {
		if (!m_SourceFileStack.empty())
			p->filename = make_memory_block_for_string(m_pAgentSoar, m_SourceFileStack.top().c_str());

		// production was sourced
        m_NumProductionsSourced += 1;
		if (m_RawOutput) {
			m_Result << '*';
		}
	}
	return true;
}

