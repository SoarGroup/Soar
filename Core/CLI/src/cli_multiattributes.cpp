/////////////////////////////////////////////////////////////////
// multi-attributes command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"

#include "sml_Names.h"

#include "agent.h"
#include "production.h"
#include "print.h"
#include "symtab.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseMultiAttributes(std::vector<std::string>& argv) {
	// No more than three arguments
	if (argv.size() > 3) return SetError(kTooManyArgs);

	int n = 0;
	// If we have 3 arguments, third one is an integer
	if (argv.size() > 2) {
		if ( !from_string( n, argv[2] ) ) return SetError(kIntegerExpected);
		if (n <= 0) return SetError(kIntegerMustBeNonNegative);
	}

	// If we have two arguments, second arg is an attribute/identifer/whatever
	if (argv.size() > 1) return DoMultiAttributes(&argv[1], n);

	return DoMultiAttributes();
}

bool CommandLineInterface::DoMultiAttributes(const std::string* pAttribute, int n) {
	multi_attribute* maList = m_pAgentSoar->multi_attributes;

	if (!pAttribute && !n) {

		// No args, print current setting
		int count = 0;

		if ( !maList ) 
		{
			m_Result << "No multi-attributes found.";
		}

		std::stringstream buffer;

		if ( m_RawOutput ) m_Result << "Value\tSymbol";

		while( maList )
		{
			// Arbitrary buffer and size
			char attributeName[1024];
			symbol_to_string(m_pAgentSoar, maList->symbol, TRUE, attributeName, 1024);

			if (m_RawOutput) {
				m_Result << "\n" << maList->value << "\t" << symbol_to_string(m_pAgentSoar, maList->symbol, TRUE, attributeName, 1024);

			} else {
				buffer << maList->value;
				// Value
				AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeInt, buffer.str() );
				buffer.clear();

				// Symbol
				AppendArgTagFast( sml_Names::kParamName, sml_Names::kTypeString, attributeName );
			}

			++count;

			maList = maList->next;
		}

		buffer << count;
		if (!m_RawOutput) {
			PrependArgTagFast(sml_Names::kParamCount, sml_Names::kTypeInt, buffer.str() );
		}
		return true;
	}

	// Setting defaults to 10
	if (!n) n = 10;

	// Set it
	Symbol* s = make_sym_constant( m_pAgentSoar, pAttribute->c_str() );

	while (maList) 
	{
		if (maList->symbol == s) 
		{
			maList->value = n;
			symbol_remove_ref(m_pAgentSoar, s);
			return true;
		}

		maList = maList->next;
	}

	/* sym wasn't in the table if we get here, so add it */
	maList = static_cast<multi_attribute *>(allocate_memory(m_pAgentSoar, sizeof(multi_attribute), MISCELLANEOUS_MEM_USAGE));
	assert(maList);

	maList->value = n;
	maList->symbol = s;
	maList->next = m_pAgentSoar->multi_attributes;
	m_pAgentSoar->multi_attributes = maList;

 	return true;
}

