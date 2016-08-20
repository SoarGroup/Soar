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
    Symbol* pId = 0;
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
    Symbol* pAttr = 0;
    if (attribute == "*")
    {
        pAttr = thisAgent->symbolManager->make_new_identifier('I', pId->id->level);
    }
    else
    {
        lexeme = soar::Lexer::get_lexeme_from_string(thisAgent, attribute.c_str());

        switch (lexeme.type)
        {
            case STR_CONSTANT_LEXEME:
                pAttr = thisAgent->symbolManager->make_str_constant(lexeme.string());
                break;
            case INT_CONSTANT_LEXEME:
                pAttr = thisAgent->symbolManager->make_int_constant(lexeme.int_val);
                break;
            case FLOAT_CONSTANT_LEXEME:
                pAttr = thisAgent->symbolManager->make_float_constant(lexeme.float_val);
                break;
            case IDENTIFIER_LEXEME:
            case VARIABLE_LEXEME:
                pAttr = read_identifier_or_context_variable(thisAgent, &lexeme);
                if (!pAttr)
                {
                    return SetError("Invalid attribute.");
                }
                thisAgent->symbolManager->symbol_add_ref(pAttr);
                break;
            default:
                return SetError("Unknown attribute type.");
        }
    }

    // get value or '*'
    Symbol* pValue = 0;
    if (value == "*")
    {
        pValue = thisAgent->symbolManager->make_new_identifier('I', pId->id->level);
    }
    else
    {
        lexeme = soar::Lexer::get_lexeme_from_string(thisAgent, value.c_str());
        switch (lexeme.type)
        {
            case STR_CONSTANT_LEXEME:
                pValue = thisAgent->symbolManager->make_str_constant(lexeme.string());
                break;
            case INT_CONSTANT_LEXEME:
                pValue = thisAgent->symbolManager->make_int_constant(lexeme.int_val);
                break;
            case FLOAT_CONSTANT_LEXEME:
                pValue = thisAgent->symbolManager->make_float_constant(lexeme.float_val);
                break;
            case IDENTIFIER_LEXEME:
            case VARIABLE_LEXEME:
                pValue = read_identifier_or_context_variable(thisAgent, &lexeme);
                if (!pValue)
                {
                    thisAgent->symbolManager->symbol_remove_ref(&pAttr);
                    return SetError("Invalid value.");
                }
                thisAgent->symbolManager->symbol_add_ref(pValue);
                break;
            default:
                thisAgent->symbolManager->symbol_remove_ref(&pAttr);
                return SetError("Unknown value type.");
        }
    }

    // now create and add the wme
    wme* pWme = make_wme(thisAgent, pId, pAttr, pValue, acceptable);

    thisAgent->symbolManager->symbol_remove_ref(&pWme->attr);
    thisAgent->symbolManager->symbol_remove_ref(&pWme->value);
    insert_at_head_of_dll(pWme->id->id->input_wmes, pWme, next, prev);

    if (wma_enabled(thisAgent))
    {
        wma_activate_wme(thisAgent, pWme);
    }

    add_wme_to_wm(thisAgent, pWme);

    /* This was previously using #ifndef NO_TOP_LEVEL_REFS, which is a macro constant that
     * no longer exists.  We now use DO_TOP_LEVEL_REF_CTS.  Top level refcounting is now
     * also disabled by default so changing it to #ifdef DO_TOP_LEVEL_REF_CTS would
     * change the current behavior.  Other uses of DO_TOP_LEVEL_REF_CTS seem to only be used
     * when adding refcounts to top-state wme's, so I'm not sure why the old macro prevented
     * this entire call.  So, I'm just going to comment it out for now and preserve existing
     * behavior. */
    //#ifdef DO_TOP_LEVEL_REF_CTS
    do_buffered_wm_and_ownership_changes(thisAgent);
    //#endif // DO_TOP_LEVEL_REF_CTS

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



