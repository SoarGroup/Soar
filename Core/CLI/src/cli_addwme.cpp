/////////////////////////////////////////////////////////////////
// add-wme command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "sml_Utils.h"
#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"
#include "cli_CLIError.h"

#include "sml_Names.h"

#include "sml_KernelSML.h"
#include "sml_KernelHelpers.h"

#include "utilities.h"
#include "wmem.h"
#include "symtab.h"
#include "decide.h"

#include "wma.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseAddWME(std::vector<std::string>& argv) {
	if (argv.size() < 4) return SetError(CLIError::kTooFewArgs);

	unsigned attributeIndex = (argv[2] == "^") ? 3 : 2;

	if (argv.size() < (attributeIndex + 2)) return SetError(CLIError::kTooFewArgs);
    if (argv.size() > (attributeIndex + 3)) return SetError(CLIError::kTooManyArgs);

	bool acceptable = false;
	if (argv.size() > (attributeIndex + 2)) {
		if (argv[attributeIndex + 2] != "+") {
			SetErrorDetail("Got: " + argv[attributeIndex + 2]);
			return SetError(CLIError::kAcceptableOrNothingExpected);
		}
		acceptable = true;
	}

	return DoAddWME(argv[1], argv[attributeIndex], argv[attributeIndex + 1], acceptable);
}

bool CommandLineInterface::DoAddWME(const std::string& id, std::string attribute, const std::string& value, bool acceptable) {
	// Get ID
	Symbol* pId = 0;
	if ( !read_id_or_context_var_from_string( m_pAgentSoar, id.c_str(), &pId ) ) 
	{
		return SetError(CLIError::kInvalidID);
	}

	// skip optional '^', if present
	if ( attribute[0] == '^' ) 
	{
		attribute = attribute.substr( 1 );
	}

	// get attribute or '*'
	Symbol* pAttr = 0;
	if ( attribute == "*" ) 
	{
		pAttr = make_new_identifier( m_pAgentSoar, 'I', pId->id.level );
	} 
	else 
	{
		get_lexeme_from_string( m_pAgentSoar, attribute.c_str() );

		switch (m_pAgentSoar->lexeme.type) 
		{
		case SYM_CONSTANT_LEXEME:
			pAttr = make_sym_constant( m_pAgentSoar, m_pAgentSoar->lexeme.string );
			break;
		case INT_CONSTANT_LEXEME:
			pAttr = make_int_constant( m_pAgentSoar, m_pAgentSoar->lexeme.int_val );
			break;
		case FLOAT_CONSTANT_LEXEME:
			pAttr = make_float_constant( m_pAgentSoar, m_pAgentSoar->lexeme.float_val );
			break;
		case IDENTIFIER_LEXEME:
		case VARIABLE_LEXEME:
			pAttr = read_identifier_or_context_variable( m_pAgentSoar );
			if ( !pAttr ) 
			{
				return SetError( CLIError::kInvalidAttribute );
			}
			symbol_add_ref( pAttr );
			break;
		default:
			return SetError( CLIError::kInvalidAttribute );
		}
	}

	// get value or '*'
	Symbol* pValue = 0;
	if ( value == "*" ) 
	{
		pValue = make_new_identifier( m_pAgentSoar, 'I', pId->id.level );
	} 
	else 
	{
		get_lexeme_from_string( m_pAgentSoar, value.c_str() );
		switch ( m_pAgentSoar->lexeme.type ) 
		{
		case SYM_CONSTANT_LEXEME:
			pValue = make_sym_constant( m_pAgentSoar, m_pAgentSoar->lexeme.string );
			break;
		case INT_CONSTANT_LEXEME:
			pValue = make_int_constant( m_pAgentSoar, m_pAgentSoar->lexeme.int_val );
			break;
		case FLOAT_CONSTANT_LEXEME:
			pValue = make_float_constant( m_pAgentSoar, m_pAgentSoar->lexeme.float_val );
			break;
		case IDENTIFIER_LEXEME:
		case VARIABLE_LEXEME:
			pValue = read_identifier_or_context_variable( m_pAgentSoar );
			if (!pValue) 
			{
				symbol_remove_ref( m_pAgentSoar, pAttr );
				return SetError( CLIError::kInvalidValue );
			}
			symbol_add_ref(pValue);
			break;
		default:
			symbol_remove_ref( m_pAgentSoar, pAttr );
			return SetError( CLIError::kInvalidValue );
		}
	}

	// now create and add the wme
	wme* pWme = make_wme( m_pAgentSoar, pId, pAttr, pValue, acceptable );			

	symbol_remove_ref( m_pAgentSoar, pWme->attr );
	symbol_remove_ref( m_pAgentSoar, pWme->value );
	insert_at_head_of_dll( pWme->id->id.input_wmes, pWme, next, prev );

	if ( wma_enabled( m_pAgentSoar ) )
	{
		wma_activate_wme( m_pAgentSoar, pWme );
	}

	add_wme_to_wm( m_pAgentSoar, pWme );

#ifndef NO_TOP_LEVEL_REFS
	do_buffered_wm_and_ownership_changes( m_pAgentSoar );
#endif // NO_TOP_LEVEL_REFS

	if (m_RawOutput) 
	{
		m_Result << "Timetag: " << pWme->timetag;
	} 
	else 
	{
		std::stringstream timetagString;
		timetagString << pWme->timetag;
		AppendArgTagFast( sml_Names::kParamValue, sml_Names::kTypeInt, timetagString.str() );
	}
	return true;
}



