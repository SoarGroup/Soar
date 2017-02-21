/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/*************************************************************************
 *
 *  file:  sybol.cpp
 *
 * =======================================================================
 *
 *                 Symbol Table Routines
 *
 * Soar uses five kinds of symbols:  symbolic constants, integer
 * constants, floating-point constants, identifiers, and variables.
 * We use five resizable hash tables, one for each kind of symbol.
 *
 * =======================================================================
 */

#include "symbol.h"

#include "agent.h"
#include "lexer.h"
#include "output_manager.h"
#include "print.h"
#include "run_soar.h"
#include "symbol_manager.h"

/* -----------------------------------------------------------------
                       First Letter From Symbol

   When creating dummy variables or identifiers, we try to give them
   names that start with a "reasonable" letter.  For example, ^foo <dummy>
   becomes ^foo <f*37>, where the variable starts with "f" because
   the attribute test starts with "f" also.  This routine looks at
   a symbol and tries to figure out a reasonable choice of starting
   letter for a variable or identifier to follow it.  If it can't
   find a reasonable choice, it returns '*'.
----------------------------------------------------------------- */

char first_letter_from_symbol(Symbol* sym)
{
    switch (sym->symbol_type)
    {
        case VARIABLE_SYMBOL_TYPE:
            return *(sym->var->name + 1);
        case IDENTIFIER_SYMBOL_TYPE:
            return sym->id->name_letter;
        case STR_CONSTANT_SYMBOL_TYPE:
            return *(sym->sc->name);
        default:
            return '*';
    }
}

/***************************************************************************
 * Function     : get_number_from_symbol
 **************************************************************************/
double get_number_from_symbol(Symbol* sym)
{
    if (sym->symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE)
    {
        return sym->fc->value;
    }
    else if (sym->symbol_type == INT_CONSTANT_SYMBOL_TYPE)
    {
        return static_cast<double>(sym->ic->value);
    }

    return 0.0;
}

bool make_string_rereadable(std::string &pStr)
{
    bool possible_id, possible_var, possible_sc, possible_ic, possible_fc;
    bool is_rereadable;
    bool has_angle_bracket;

    const char* pCStr = pStr.c_str();
    short pLength = pStr.size();

    soar::Lexer::determine_possible_symbol_types_for_string(pCStr, pLength,
            &possible_id, &possible_var, &possible_sc, &possible_ic, &possible_fc, &is_rereadable);

    has_angle_bracket = pCStr[0] == '<' || pCStr[pLength - 1] == '>';

    if ((!possible_sc)   || possible_var || possible_ic || possible_fc || (!is_rereadable))
    {
        /* BUGBUG - if in context where id's could occur, should check possible_id flag here also */
        pStr = string_to_escaped_string(pCStr, '|');
        return true;
    }
    return false;
}

char* Symbol::to_string(bool rereadable, bool showLTILink, char* dest, size_t dest_size)
{

    bool possible_id, possible_var, possible_sc, possible_ic, possible_fc;
    bool is_rereadable;
    bool has_angle_bracket;
    bool wasModified;
    std::string lStr;

    switch (symbol_type)
    {
        case VARIABLE_SYMBOL_TYPE:
            if (!dest)
            {
                return var->name;
            }
            strcpy(dest, var->name);
            dest[dest_size - 1] = 0; /* ensure null termination */
            return dest;

        case IDENTIFIER_SYMBOL_TYPE:
            if (id->cached_print_str && !id->is_lti())
             {
                if (!dest)
                 {
                    return id->cached_print_str;
                 } else {
                     strcpy(dest, id->cached_print_str);
                     dest[dest_size - 1] = 0; /* ensure null termination */
                     return dest;
                 }
             }
            lStr.push_back(id->name_letter);
            lStr.append(std::to_string(id->name_number));
            if (showLTILink && id->is_lti())
            {
                lStr += " (@";
                lStr.append(std::to_string(id->LTI_ID));
                lStr += ')';
                if (id->cached_print_str)
                {
                    free_memory_block_for_string(id->thisAgent, id->cached_print_str);
                }
            }
            id->cached_print_str =  make_memory_block_for_string(id->thisAgent, lStr.c_str());
            if (!dest)
            {
                return id->cached_print_str;
            } else {
                strcpy(dest, id->cached_print_str);
                dest[dest_size - 1] = 0; /* ensure null termination */
                return dest;
            }

        case INT_CONSTANT_SYMBOL_TYPE:
            if (ic->cached_print_str)
             {
                if (!dest)
                 {
                    return ic->cached_print_str;
                 } else {
                     strcpy(dest, ic->cached_print_str);
                     dest[dest_size - 1] = 0; /* ensure null termination */
                     return dest;
                 }
             }
            lStr = std::to_string(ic->value);
            ic->cached_print_str =  make_memory_block_for_string(ic->thisAgent, lStr.c_str());
            if (!dest)
            {
                return ic->cached_print_str;
            } else {
                strcpy(dest, ic->cached_print_str);
                dest[dest_size - 1] = 0; /* ensure null termination */
                return dest;
            }

        case FLOAT_CONSTANT_SYMBOL_TYPE:
            if (fc->cached_print_str)
             {
                if (!dest)
                 {
                    return fc->cached_print_str;
                 } else {
                     strcpy(dest, fc->cached_print_str);
                     dest[dest_size - 1] = 0; /* ensure null termination */
                     return dest;
                 }
             }
            lStr = std::to_string(fc->value);
            fc->cached_print_str =  make_memory_block_for_string(fc->thisAgent, lStr.c_str());
            if (!dest)
            {
                return fc->cached_print_str;
            } else {
                strcpy(dest, fc->cached_print_str);
                dest[dest_size - 1] = 0; /* ensure null termination */
                return dest;
            }

        case STR_CONSTANT_SYMBOL_TYPE:
            if (!rereadable)
            {
                if (!dest)
                {
                    return sc->name;
                }
                strcpy(dest, sc->name);
                return dest;
            }

            lStr = sc->name;
            wasModified = make_string_rereadable(lStr);
            if (wasModified)
            {
                sc->cached_print_str =  make_memory_block_for_string(sc->thisAgent, lStr.c_str());
            } else {
                sc->cached_print_str =  make_memory_block_for_string(sc->thisAgent, sc->name);
            }

            if (!dest)
            {
                return sc->cached_print_str;
            } else {
                strcpy(dest, sc->cached_print_str);
                dest[dest_size - 1] = 0; /* ensure null termination */
                return dest;
            }

        default:
        {
            char msg[BUFFER_MSG_SIZE];
            strcpy(msg, "Internal Soar Error:  symbol->to_string() called on bad symbol!\n");
            msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
            abort_with_fatal_error_noagent(msg);
            break;
        }
    }
    return NIL; /* unreachable, but without it, gcc -Wall warns here */
}

