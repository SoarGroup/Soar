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



char* Symbol::to_string(bool rereadable, char* dest, size_t dest_size)
{

    bool possible_id, possible_var, possible_sc, possible_ic, possible_fc;
    bool is_rereadable;
    bool has_angle_bracket;

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
            if (!dest)
            {
                dest_size = output_string_size; /* from agent.h */;
                dest = Output_Manager::Get_OM().get_printed_output_string();
            }
            if (is_lti())
            {
                SNPRINTF(dest, dest_size, "@%c%llu", id->name_letter, static_cast<long long unsigned>(id->name_number));
            }
            else
            {
                SNPRINTF(dest, dest_size, "%c%llu", id->name_letter, static_cast<long long unsigned>(id->name_number));
            }
            dest[dest_size - 1] = 0; /* ensure null termination */
            return dest;

        case INT_CONSTANT_SYMBOL_TYPE:
            if (!dest)
            {
                dest_size = output_string_size; /* from agent.h */;
                dest = Output_Manager::Get_OM().get_printed_output_string();
            }
            SNPRINTF(dest, dest_size, "%ld", static_cast<long int>(ic->value));
            dest[dest_size - 1] = 0; /* ensure null termination */
            return dest;

        case FLOAT_CONSTANT_SYMBOL_TYPE:
            if (!dest)
            {
                dest_size = output_string_size; /* from agent.h */;
                dest = Output_Manager::Get_OM().get_printed_output_string();
            }
            SNPRINTF(dest, dest_size, "%#.16g", fc->value);
            dest[dest_size - 1] = 0; /* ensure null termination */
            {
                /* --- strip off trailing zeros --- */
                char* start_of_exponent;
                char* end_of_mantissa;
                start_of_exponent = dest;
                while ((*start_of_exponent != 0) && (*start_of_exponent != 'e'))
                {
                    start_of_exponent++;
                }
                end_of_mantissa = start_of_exponent - 1;
                while (*end_of_mantissa == '0')
                {
                    end_of_mantissa--;
                }
                end_of_mantissa++;
                while (*start_of_exponent)
                {
                    *end_of_mantissa++ = *start_of_exponent++;
                }
                *end_of_mantissa = 0;
            }
            return dest;

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

            soar::Lexer::determine_possible_symbol_types_for_string(sc->name, strlen(sc->name),
                    &possible_id, &possible_var, &possible_sc, &possible_ic, &possible_fc, &is_rereadable);

            has_angle_bracket = sc->name[0] == '<' || sc->name[strlen(sc->name) - 1] == '>';

            if ((!possible_sc)   || possible_var || possible_ic || possible_fc ||
                    (!is_rereadable) || has_angle_bracket)
            {
                /* BUGBUG - if in context where id's could occur, should check possible_id flag here also
                 *        - Shouldn't it also check whether dest char * was passed in and get a printed
                 *          output string instead?  */
                return string_to_escaped_string(sc->name, '|', dest);
            }
            if (!dest)
            {
                return sc->name;
            }
            strcpy(dest, sc->name);
            return dest;

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

