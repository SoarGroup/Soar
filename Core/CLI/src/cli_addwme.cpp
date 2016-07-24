#include "portability.h"

#include "cli_CommandLineInterface.h"
#include "agent.h"
#include "working_memory.h"
#include "symbol.h"
#include "decide.h"
#include "working_memory_activation.h"
#include "sml_Names.h"
#include "sml_AgentSML.h"
#include "lexer.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoAddWME(const std::string& id, std::string attribute, const std::string& value, bool acceptable)
{
    agent* thisAgent = m_pAgentSML->GetSoarAgent();
    soar::Lexeme lexeme;

    // Get ID
    Symbol pId = 0;
    if (!read_id_or_context_var_from_string(thisAgent, id.c_str(), &pId))
    {
        return SetError("Invalid identifier");
    }

    // skip optional '^', if present
    if (attribute[0] == '^')
    {
        attribute = attribute.substr(1);
    }

    // get attribute or '*'
    Symbol pAttr = 0;
    if (attribute == "*")
    {
        pAttr = make_new_identifier(thisAgent, 'I', pId->id->level);
    }
    else
    {
        lexeme = soar::Lexer::get_lexeme_from_string(thisAgent, attribute.c_str());

        switch (lexeme.type)
        {
            case STR_CONSTANT_LEXEME:
                pAttr = make_str_constant(thisAgent, lexeme.string());
                break;
            case INT_CONSTANT_LEXEME:
                pAttr = make_int_constant(thisAgent, lexeme.int_val);
                break;
            case FLOAT_CONSTANT_LEXEME:
                pAttr = make_float_constant(thisAgent, lexeme.float_val);
                break;
            case IDENTIFIER_LEXEME:
            case VARIABLE_LEXEME:
                pAttr = read_identifier_or_context_variable(thisAgent, &lexeme);
                if (!pAttr)
                {
                    return SetError("Invalid attribute.");
                }
                symbol_add_ref(thisAgent, pAttr);
                break;
            default:
                return SetError("Unknown attribute type.");
        }
    }

    // get value or '*'
    Symbol pValue = 0;
    if (value == "*")
    {
        pValue = make_new_identifier(thisAgent, 'I', pId->id->level);
    }
    else
    {
        lexeme = soar::Lexer::get_lexeme_from_string(thisAgent, value.c_str());
        switch (lexeme.type)
        {
            case STR_CONSTANT_LEXEME:
                pValue = make_str_constant(thisAgent, lexeme.string());
                break;
            case INT_CONSTANT_LEXEME:
                pValue = make_int_constant(thisAgent, lexeme.int_val);
                break;
            case FLOAT_CONSTANT_LEXEME:
                pValue = make_float_constant(thisAgent, lexeme.float_val);
                break;
            case IDENTIFIER_LEXEME:
            case VARIABLE_LEXEME:
                pValue = read_identifier_or_context_variable(thisAgent, &lexeme);
                if (!pValue)
                {
                    symbol_remove_ref(thisAgent, pAttr);
                    return SetError("Invalid value.");
                }
                symbol_add_ref(thisAgent, pValue);
                break;
            default:
                symbol_remove_ref(thisAgent, pAttr);
                return SetError("Unknown value type.");
        }
    }

    // now create and add the wme
    wme* pWme = make_wme(thisAgent, pId, pAttr, pValue, acceptable);

    symbol_remove_ref(thisAgent, pWme->attr);
    symbol_remove_ref(thisAgent, pWme->value);
    insert_at_head_of_dll(pWme->id->id->input_wmes, pWme, next, prev);

    if (wma_enabled(thisAgent))
    {
        wma_activate_wme(thisAgent, pWme);
    }

    add_wme_to_wm(thisAgent, pWme);

#ifndef NO_TOP_LEVEL_REFS
    do_buffered_wm_and_ownership_changes(thisAgent);
#endif // NO_TOP_LEVEL_REFS

    if (m_RawOutput)
    {
        m_Result << "Timetag: " << pWme->timetag;
    }
    else
    {
        std::stringstream timetagString;
        timetagString << pWme->timetag;
        AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeInt, timetagString.str());
    }
    return true;
}



