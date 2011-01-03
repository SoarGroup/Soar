#include <portability.h>

#include "cli_CommandLineInterface.h"
#include "utilities.h"
#include "wmem.h"
#include "symtab.h"
#include "decide.h"
#include "wma.h"
#include "sml_Names.h"
#include "sml_AgentSML.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoAddWME(const std::string& id, std::string attribute, const std::string& value, bool acceptable) {
    agent* agnt = m_pAgentSML->GetSoarAgent();

    // Get ID
    Symbol* pId = 0;
    if ( !read_id_or_context_var_from_string( agnt, id.c_str(), &pId ) ) 
        return SetError("Invalid identifier");

    // skip optional '^', if present
    if ( attribute[0] == '^' ) 
        attribute = attribute.substr( 1 );

    // get attribute or '*'
    Symbol* pAttr = 0;
    if ( attribute == "*" ) 
        pAttr = make_new_identifier( agnt, 'I', pId->id.level );
    else 
    {
        get_lexeme_from_string( agnt, attribute.c_str() );

        switch (agnt->lexeme.type) 
        {
        case SYM_CONSTANT_LEXEME:
            pAttr = make_sym_constant( agnt, agnt->lexeme.string );
            break;
        case INT_CONSTANT_LEXEME:
            pAttr = make_int_constant( agnt, agnt->lexeme.int_val );
            break;
        case FLOAT_CONSTANT_LEXEME:
            pAttr = make_float_constant( agnt, agnt->lexeme.float_val );
            break;
        case IDENTIFIER_LEXEME:
        case VARIABLE_LEXEME:
            pAttr = read_identifier_or_context_variable( agnt );
            if ( !pAttr ) 
                return SetError( "Invalid attribute." );
            symbol_add_ref( pAttr );
            break;
        default:
            return SetError( "Unknown attribute type." );
        }
    }

    // get value or '*'
    Symbol* pValue = 0;
    if ( value == "*" ) 
        pValue = make_new_identifier( agnt, 'I', pId->id.level );
    else 
    {
        get_lexeme_from_string( agnt, value.c_str() );
        switch ( agnt->lexeme.type ) 
        {
        case SYM_CONSTANT_LEXEME:
            pValue = make_sym_constant( agnt, agnt->lexeme.string );
            break;
        case INT_CONSTANT_LEXEME:
            pValue = make_int_constant( agnt, agnt->lexeme.int_val );
            break;
        case FLOAT_CONSTANT_LEXEME:
            pValue = make_float_constant( agnt, agnt->lexeme.float_val );
            break;
        case IDENTIFIER_LEXEME:
        case VARIABLE_LEXEME:
            pValue = read_identifier_or_context_variable( agnt );
            if (!pValue) 
            {
                symbol_remove_ref( agnt, pAttr );
                return SetError( "Invalid value." );
            }
            symbol_add_ref(pValue);
            break;
        default:
            symbol_remove_ref( agnt, pAttr );
            return SetError( "Unknown value type." );
        }
    }

    // now create and add the wme
    wme* pWme = make_wme( agnt, pId, pAttr, pValue, acceptable );			

    symbol_remove_ref( agnt, pWme->attr );
    symbol_remove_ref( agnt, pWme->value );
    insert_at_head_of_dll( pWme->id->id.input_wmes, pWme, next, prev );

    if ( wma_enabled( agnt ) )
    {
        wma_activate_wme( agnt, pWme );
    }

    add_wme_to_wm( agnt, pWme );

#ifndef NO_TOP_LEVEL_REFS
    do_buffered_wm_and_ownership_changes( agnt );
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



