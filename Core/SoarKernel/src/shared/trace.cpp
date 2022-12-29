/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/*************************************************************************
 *
 *  file:  trace.cpp
 *
 * =======================================================================
 *
 *                   Trace Format Routines
 *
 * This file contains definitions and routines for dealing with the trace
 * formats used by Soar 6.  Trace format are specified by the user as
 * strings (with % escape sequences in them).  At entry time, Soar 6
 * parses these strings into trace_format structures.
 *
 * =======================================================================
 */

#include "trace.h"

#include "agent.h"
#include "decide.h"
#include "lexer.h"
#include "mem.h"
#include "output_manager.h"
#include "print.h"
#include "production.h"
#include "run_soar.h"
#include "slot.h"
#include "soar_TraceNames.h"
#include "symbol_manager.h"
#include "symbol.h"
#include "working_memory.h"
#include "xml.h"

#include <ctype.h>
#include <cinttypes>
#include <stdlib.h>

using namespace soar_TraceNames;

/* --- trace format types --- */

enum trace_format_type
{
    STRING_TFT,                        /* print a string */
    PERCENT_TFT,                       /* print a percent sign */
    L_BRACKET_TFT,                     /* print a left bracket */
    R_BRACKET_TFT,                     /* print a right bracket */
    VALUES_TFT,                        /* print values of attr path or '*' */
    VALUES_RECURSIVELY_TFT,            /* ditto only print recursively */
    ATTS_AND_VALUES_TFT,               /* ditto only print attr's too */
    ATTS_AND_VALUES_RECURSIVELY_TFT,   /* combination of the two above */
    CURRENT_STATE_TFT,                 /* print current state */
    CURRENT_OPERATOR_TFT,              /* print current operator */
    DECISION_CYCLE_COUNT_TFT,          /* print # of dc's */
    ELABORATION_CYCLE_COUNT_TFT,       /* print # of ec's */
    IDENTIFIER_TFT,                    /* print identifier of object */
    IF_ALL_DEFINED_TFT,                /* print subformat if it's defined */
    LEFT_JUSTIFY_TFT,                  /* left justify the subformat */
    RIGHT_JUSTIFY_TFT,                 /* right justify the subformat */
    SUBGOAL_DEPTH_TFT,                 /* print # of subgoal depth */
    REPEAT_SUBGOAL_DEPTH_TFT,          /* repeat subformat s.d. times */
    NEWLINE_TFT
};                     /* print a newline */

/* --- trace_format structure --- */

typedef struct trace_format_struct
{
    struct trace_format_struct* next; /* next in linked list of format items */
    enum trace_format_type type;      /* what kind of item this is */
    int num;                          /* for formats with extra numeric arg */
    union trace_format_data_union     /* data depending on trace format type */
    {
        char* string;                           /* string to print */
        struct trace_format_struct* subformat;  /* [subformat in brackets] */
        cons* attribute_path;  /* list.of.attr.path.symbols (NIL if path is '*') */
    } data;
} trace_format;

/* ----------------------------------------------------------------------
                     Deallocate Trace Format List

   This deallocates all the memory used by a (list of) trace_format.
---------------------------------------------------------------------- */

void deallocate_trace_format_list(agent* thisAgent, trace_format* tf)
{
    trace_format* next;

    while (tf)
    {
        switch (tf->type)
        {
            case STRING_TFT:
                free_memory_block_for_string(thisAgent, tf->data.string);
                break;

            case VALUES_TFT:
            case VALUES_RECURSIVELY_TFT:
            case ATTS_AND_VALUES_TFT:
            case ATTS_AND_VALUES_RECURSIVELY_TFT:
                thisAgent->symbolManager->deallocate_symbol_list_removing_references(tf->data.attribute_path);
                break;

            case IF_ALL_DEFINED_TFT:
            case LEFT_JUSTIFY_TFT:
            case RIGHT_JUSTIFY_TFT:
            case REPEAT_SUBGOAL_DEPTH_TFT:
                deallocate_trace_format_list(thisAgent, tf->data.subformat);
                break;

            default:
                break; /* do nothing */
        }
        next = tf->next;
        thisAgent->memoryManager->free_memory(tf, MISCELLANEOUS_MEM_USAGE);
        tf = next;
    }
}

/* ----------------------------------------------------------------------
                   Trace Format String Parsing Routines

   Parse_format_string() parses a format string and returns a trace_format
   structure for it, or NIL if any error occurred.  This is the top-level
   routine here.

   While parsing is in progress, the global variable "format" points to
   the current character in the string.  This is advanced through the
   string as parsing proceeds.  Parsing is done by recursive descent.
   If any parsing routine encouters an error, it sets the global variable
   "format_string_error_message" to be some appropriate error message,
   and leaves "format" pointing to the location of the error.

   Parse_attribute_path_in_brackets() reads "[attr.path.or.star]" and
   returns the path (consed list, or NIL for the '*' path).

   Parse_pattern_in_brackets() reads "[subformat pattern]" and returns
   the trace format.

   Parse_item_from_format_string() is the main workhorse.  It reads from
   the current location up to the end of the item--a string, an escape
   sequence (with arguments), etc.  The end of an item is delineated by
   the end-of-string, a "[" or "]", or the end of the escape sequence.
---------------------------------------------------------------------- */

const char* format;
const char* format_string_error_message;

trace_format* parse_item_from_format_string(agent* thisAgent);

trace_format* parse_format_string(agent* thisAgent, const char* string)
{
    trace_format* first, *prev, *New;

    format = string;
    format_string_error_message = NIL;

    prev = NIL;
    first = NIL; /* unnecessary, but gcc -Wall warns without it */
    while (*format != 0)
    {
        New = parse_item_from_format_string(thisAgent);
        if (!New)
        {
            if (prev)
            {
                prev->next = NIL;
            }
            else
            {
                first = NIL;
            }
            deallocate_trace_format_list(thisAgent, first);
            thisAgent->outputManager->printa_sf(thisAgent,  "Error:  bad trace format string: %s\n", string);
            if (format_string_error_message)
            {
                thisAgent->outputManager->printa_sf(thisAgent,  "        %s\n", format_string_error_message);
                thisAgent->outputManager->printa_sf(thisAgent,  "        Error found at: %s\n", format);
            }
            return NIL;
        }
        if (prev)
        {
            prev->next = New;
        }
        else
        {
            first = New;
        }
        prev = New;
    }
    if (prev)
    {
        prev->next = NIL;
    }
    else
    {
        first = NIL;
    }

    return first;
}

cons* parse_attribute_path_in_brackets(agent* thisAgent)
{
    cons* path;
    char name[MAX_LEXEME_LENGTH + 20], *ch;
    Symbol* sym;

    /* --- look for opening bracket --- */
    if (*format != '[')
    {
        format_string_error_message = "Expected '[' followed by attribute (path)";
        return NIL;
    }
    format++;

    /* --- check for '*' (null path) --- */
    if (*format == '*')
    {
        path = NIL;
        format++;
    }
    else
    {
        /* --- normal case: read the attribute path --- */
        path = NIL;
        while (true)
        {
            ch = name;
            while ((*format != 0) && (*format != ']') && (*format != '.'))
            {
                *ch++ = *format++;
            }
            if (*format == 0)
            {
                format_string_error_message = "'[' without closing ']'";
                thisAgent->symbolManager->deallocate_symbol_list_removing_references(path);
                return NIL;
            }
            if (ch == name)
            {
                format_string_error_message = "null attribute found in attribute path";
                thisAgent->symbolManager->deallocate_symbol_list_removing_references(path);
                return NIL;
            }
            *ch = 0;
            sym = thisAgent->symbolManager->make_str_constant(name);
            push(thisAgent, sym, path);
            if (*format == ']')
            {
                break;
            }
            format++;  /* skip past '.' */
        }
        path = destructively_reverse_list(path);
    }

    /* --- look for closing bracket --- */
    if (*format != ']')
    {
        format_string_error_message = "'[' without closing ']'";
        thisAgent->symbolManager->deallocate_symbol_list_removing_references(path);
        return NIL;
    }
    format++;

    return path;
}

trace_format* parse_pattern_in_brackets(agent* thisAgent, bool read_opening_bracket)
{
    trace_format* first, *prev, *New;

    /* --- look for opening bracket --- */
    if (read_opening_bracket)
    {
        if (*format != '[')
        {
            format_string_error_message = "Expected '[' followed by attribute path";
            return NIL;
        }
        format++;
    }

    /* --- read pattern --- */
    prev = NIL;
    first = NIL; /* unnecessary, but gcc -Wall warns without it */
    while ((*format != 0) && (*format != ']'))
    {
        New = parse_item_from_format_string(thisAgent);
        if (!New)
        {
            if (prev)
            {
                prev->next = NIL;
            }
            else
            {
                first = NIL;
            }
            deallocate_trace_format_list(thisAgent, first);
            return NIL;
        }
        if (prev)
        {
            prev->next = New;
        }
        else
        {
            first = New;
        }
        prev = New;
    }
    if (prev)
    {
        prev->next = NIL;
    }
    else
    {
        first = NIL;
    }

    /* --- look for closing bracket --- */
    if (*format != ']')
    {
        format_string_error_message = "'[' without closing ']'";
        deallocate_trace_format_list(thisAgent, first);
        return NIL;
    }
    format++;

    return first;
}

trace_format* parse_item_from_format_string(agent* thisAgent)
{
    trace_format* tf, *pattern;
    char* ch;
    cons* attribute_path;
    int n;

    if (*format == 0)
    {
        return NIL;
    }
    if (*format == ']')
    {
        return NIL;
    }
    if (*format == '[')
    {
        format_string_error_message = "unexpected '[' character";
        return NIL;
    }

    if (*format != '%')
    {
        char buf[MAX_LEXEME_LENGTH + 20];

        ch = buf;
        while ((*format != 0) && (*format != '%') &&
                (*format != '[') && (*format != ']'))
        {
            *ch++ = *format++;
        }
        *ch = 0;
        tf = static_cast<trace_format_struct*>(thisAgent->memoryManager->allocate_memory(sizeof(trace_format),
                                               MISCELLANEOUS_MEM_USAGE));
        tf->type = STRING_TFT;
        tf->data.string = make_memory_block_for_string(thisAgent, buf);
        return tf;
    }

    /* --- otherwise *format is '%', so parse the escape sequence --- */

    if (!strncmp(format, "%v", 2))
    {
        format += 2;
        attribute_path = parse_attribute_path_in_brackets(thisAgent);
        if (format_string_error_message)
        {
            return NIL;
        }
        tf = static_cast<trace_format_struct*>(thisAgent->memoryManager->allocate_memory(sizeof(trace_format),
                                               MISCELLANEOUS_MEM_USAGE));
        tf->type = VALUES_TFT;
        tf->data.attribute_path = attribute_path;
        return tf;
    }

    if (!strncmp(format, "%o", 2))
    {
        format += 2;
        attribute_path = parse_attribute_path_in_brackets(thisAgent);
        if (format_string_error_message)
        {
            return NIL;
        }
        tf = static_cast<trace_format_struct*>(thisAgent->memoryManager->allocate_memory(sizeof(trace_format),
                                               MISCELLANEOUS_MEM_USAGE));
        tf->type = VALUES_RECURSIVELY_TFT;
        tf->data.attribute_path = attribute_path;
        return tf;
    }

    if (!strncmp(format, "%av", 3))
    {
        format += 3;
        attribute_path = parse_attribute_path_in_brackets(thisAgent);
        if (format_string_error_message)
        {
            return NIL;
        }
        tf = static_cast<trace_format_struct*>(thisAgent->memoryManager->allocate_memory(sizeof(trace_format),
                                               MISCELLANEOUS_MEM_USAGE));
        tf->type = ATTS_AND_VALUES_TFT;
        tf->data.attribute_path = attribute_path;
        return tf;
    }

    if (!strncmp(format, "%ao", 3))
    {
        format += 3;
        attribute_path = parse_attribute_path_in_brackets(thisAgent);
        if (format_string_error_message)
        {
            return NIL;
        }
        tf = static_cast<trace_format_struct*>(thisAgent->memoryManager->allocate_memory(sizeof(trace_format),
                                               MISCELLANEOUS_MEM_USAGE));
        tf->type = ATTS_AND_VALUES_RECURSIVELY_TFT;
        tf->data.attribute_path = attribute_path;
        return tf;
    }

    if (!strncmp(format, "%cs", 3))
    {
        format += 3;
        tf = static_cast<trace_format_struct*>(thisAgent->memoryManager->allocate_memory(sizeof(trace_format),
                                               MISCELLANEOUS_MEM_USAGE));
        tf->type = CURRENT_STATE_TFT;
        return tf;
    }

    if (!strncmp(format, "%co", 3))
    {
        format += 3;
        tf = static_cast<trace_format_struct*>(thisAgent->memoryManager->allocate_memory(sizeof(trace_format),
                                               MISCELLANEOUS_MEM_USAGE));
        tf->type = CURRENT_OPERATOR_TFT;
        return tf;
    }

    if (!strncmp(format, "%dc", 3))
    {
        format += 3;
        tf = static_cast<trace_format_struct*>(thisAgent->memoryManager->allocate_memory(sizeof(trace_format),
                                               MISCELLANEOUS_MEM_USAGE));
        tf->type = DECISION_CYCLE_COUNT_TFT;
        return tf;
    }

    if (!strncmp(format, "%ec", 3))
    {
        format += 3;
        tf = static_cast<trace_format_struct*>(thisAgent->memoryManager->allocate_memory(sizeof(trace_format),
                                               MISCELLANEOUS_MEM_USAGE));
        tf->type = ELABORATION_CYCLE_COUNT_TFT;
        return tf;
    }

    if (!strncmp(format, "%%", 2))
    {
        format += 2;
        tf = static_cast<trace_format_struct*>(thisAgent->memoryManager->allocate_memory(sizeof(trace_format),
                                               MISCELLANEOUS_MEM_USAGE));
        tf->type = PERCENT_TFT;
        return tf;
    }

    if (!strncmp(format, "%[", 2))
    {
        format += 2;
        tf = static_cast<trace_format_struct*>(thisAgent->memoryManager->allocate_memory(sizeof(trace_format),
                                               MISCELLANEOUS_MEM_USAGE));
        tf->type = L_BRACKET_TFT;
        return tf;
    }

    if (!strncmp(format, "%]", 2))
    {
        format += 2;
        tf = static_cast<trace_format_struct*>(thisAgent->memoryManager->allocate_memory(sizeof(trace_format),
                                               MISCELLANEOUS_MEM_USAGE));
        tf->type = R_BRACKET_TFT;
        return tf;
    }

    if (!strncmp(format, "%sd", 3))
    {
        format += 3;
        tf = static_cast<trace_format_struct*>(thisAgent->memoryManager->allocate_memory(sizeof(trace_format),
                                               MISCELLANEOUS_MEM_USAGE));
        tf->type = SUBGOAL_DEPTH_TFT;
        return tf;
    }

    if (!strncmp(format, "%id", 3))
    {
        format += 3;
        tf = static_cast<trace_format_struct*>(thisAgent->memoryManager->allocate_memory(sizeof(trace_format),
                                               MISCELLANEOUS_MEM_USAGE));
        tf->type = IDENTIFIER_TFT;
        return tf;
    }

    if (! strncmp(format, "%ifdef", 6))
    {
        format += 6;
        pattern = parse_pattern_in_brackets(thisAgent, true);
        if (format_string_error_message)
        {
            return NIL;
        }
        tf = static_cast<trace_format_struct*>(thisAgent->memoryManager->allocate_memory(sizeof(trace_format),
                                               MISCELLANEOUS_MEM_USAGE));
        tf->type = IF_ALL_DEFINED_TFT;
        tf->data.subformat = pattern;
        return tf;
    }

    if (! strncmp(format, "%left", 5))
    {
        format += 5;
        if (*format != '[')
        {
            format_string_error_message = "Expected '[' after %left";
            return NIL;
        }
        format++;
        if (! isdigit(*format))
        {
            format_string_error_message = "Expected number with %left";
            return NIL;
        }
        n = 0;
        while (isdigit(*format))
        {
            n = 10 * n + (*format++ - '0');
        }
        if (*format != ',')
        {
            format_string_error_message = "Expected ',' after number in %left";
            return NIL;
        }
        format++;
        pattern = parse_pattern_in_brackets(thisAgent, false);
        if (format_string_error_message)
        {
            return NIL;
        }
        tf = static_cast<trace_format_struct*>(thisAgent->memoryManager->allocate_memory(sizeof(trace_format),
                                               MISCELLANEOUS_MEM_USAGE));
        tf->type = LEFT_JUSTIFY_TFT;
        tf->num = n;
        tf->data.subformat = pattern;
        return tf;
    }

    if (! strncmp(format, "%right", 6))
    {
        format += 6;
        if (*format != '[')
        {
            format_string_error_message = "Expected '[' after %right";
            return NIL;
        }
        format++;
        if (! isdigit(*format))
        {
            format_string_error_message = "Expected number with %right";
            return NIL;
        }
        n = 0;
        while (isdigit(*format))
        {
            n = 10 * n + (*format++ - '0');
        }
        if (*format != ',')
        {
            format_string_error_message = "Expected ',' after number in %right";
            return NIL;
        }
        format++;
        pattern = parse_pattern_in_brackets(thisAgent, false);
        if (format_string_error_message)
        {
            return NIL;
        }
        tf = static_cast<trace_format_struct*>(thisAgent->memoryManager->allocate_memory(sizeof(trace_format),
                                               MISCELLANEOUS_MEM_USAGE));
        tf->type = RIGHT_JUSTIFY_TFT;
        tf->num = n;
        tf->data.subformat = pattern;
        return tf;
    }

    if (! strncmp(format, "%rsd", 4))
    {
        format += 4;
        pattern = parse_pattern_in_brackets(thisAgent, true);
        if (format_string_error_message)
        {
            return NIL;
        }
        tf = static_cast<trace_format_struct*>(thisAgent->memoryManager->allocate_memory(sizeof(trace_format),
                                               MISCELLANEOUS_MEM_USAGE));
        tf->type = REPEAT_SUBGOAL_DEPTH_TFT;
        tf->data.subformat = pattern;
        return tf;
    }

    if (!strncmp(format, "%nl", 3))
    {
        format += 3;
        tf = static_cast<trace_format_struct*>(thisAgent->memoryManager->allocate_memory(sizeof(trace_format),
                                               MISCELLANEOUS_MEM_USAGE));
        tf->type = NEWLINE_TFT;
        return tf;
    }

    /* --- if we haven't recognized it yet, we don't understand it --- */
    format_string_error_message = "Unrecognized escape sequence";
    return NIL;
}

/* ----------------------------------------------------------------------
                      Print Trace Format List

   This routine takes a trace format (list) and prints it out as a format
   string (without the surrounding quotation marks).
---------------------------------------------------------------------- */

void print_trace_format_list(agent* thisAgent, trace_format* tf)
{
    cons* c;

    for (; tf != NIL; tf = tf->next)
    {
        switch (tf->type)
        {
            case STRING_TFT:
            {
                std::string temp;
                temp = string_to_escaped_string(tf->data.string, '"');
                thisAgent->outputManager->printa_sf(thisAgent,  "%s", temp.c_str());

//                char* s;
//                size_t i, len;
//
//                s = string_to_escaped_string(tf->data.string, '"', NULL);
//                len = strlen(s);
//                for (i = 1; i < len - 1; i++)
//                {
//                    thisAgent->outputManager->printa_sf(thisAgent,  "%c", *(s + i));
//                }
            }
            break;
            case PERCENT_TFT:
                thisAgent->outputManager->printa(thisAgent, "%%");
                break;
            case L_BRACKET_TFT:
                thisAgent->outputManager->printa(thisAgent, "%[");
                break;
            case R_BRACKET_TFT:
                thisAgent->outputManager->printa(thisAgent, "%]");
                break;

            case VALUES_TFT:
            case VALUES_RECURSIVELY_TFT:
            case ATTS_AND_VALUES_TFT:
            case ATTS_AND_VALUES_RECURSIVELY_TFT:
                if (tf->type == VALUES_TFT)
                {
                    thisAgent->outputManager->printa(thisAgent, "%v[");
                }
                else if (tf->type == VALUES_RECURSIVELY_TFT)
                {
                    thisAgent->outputManager->printa(thisAgent, "%o[");
                }
                else if (tf->type == ATTS_AND_VALUES_TFT)
                {
                    thisAgent->outputManager->printa(thisAgent, "%av[");
                }
                else
                {
                    thisAgent->outputManager->printa(thisAgent, "%ao[");
                }
                if (tf->data.attribute_path)
                {
                    for (c = tf->data.attribute_path; c != NIL; c = c->rest)
                    {
                        thisAgent->outputManager->printa(thisAgent, static_cast<Symbol*>(c->first)->sc->name);
                        if (c->rest)
                        {
                            thisAgent->outputManager->printa(thisAgent, ".");
                        }
                    }
                }
                else
                {
                    thisAgent->outputManager->printa(thisAgent, "*");
                }
                thisAgent->outputManager->printa(thisAgent, "]");
                break;

            case CURRENT_STATE_TFT:
                thisAgent->outputManager->printa(thisAgent, "%cs");
                break;
            case CURRENT_OPERATOR_TFT:
                thisAgent->outputManager->printa(thisAgent, "%co");
                break;
            case DECISION_CYCLE_COUNT_TFT:
                thisAgent->outputManager->printa(thisAgent, "%dc");
                break;
            case ELABORATION_CYCLE_COUNT_TFT:
                thisAgent->outputManager->printa(thisAgent, "%ec");
                break;
            case IDENTIFIER_TFT:
                thisAgent->outputManager->printa(thisAgent, "%id");
                break;

            case IF_ALL_DEFINED_TFT:
                thisAgent->outputManager->printa(thisAgent, "%ifdef[");
                print_trace_format_list(thisAgent, tf->data.subformat);
                thisAgent->outputManager->printa(thisAgent, "]");
                break;

            case LEFT_JUSTIFY_TFT:
            case RIGHT_JUSTIFY_TFT:
                if (tf->type == LEFT_JUSTIFY_TFT)
                {
                    thisAgent->outputManager->printa(thisAgent, "%left[");
                }
                else
                {
                    thisAgent->outputManager->printa(thisAgent, "%right[");
                }
                thisAgent->outputManager->printa_sf(thisAgent,  "%d,", static_cast<int64_t>(tf->num));
                print_trace_format_list(thisAgent, tf->data.subformat);
                thisAgent->outputManager->printa(thisAgent, "]");
                break;

            case SUBGOAL_DEPTH_TFT:
                thisAgent->outputManager->printa(thisAgent, "%sd");
                break;

            case REPEAT_SUBGOAL_DEPTH_TFT:
                thisAgent->outputManager->printa(thisAgent, "%rsd[");
                print_trace_format_list(thisAgent, tf->data.subformat);
                thisAgent->outputManager->printa(thisAgent, "]");
                break;

            case NEWLINE_TFT:
                thisAgent->outputManager->printa(thisAgent, "%nl");
                break;

            default:
            {
                char msg[BUFFER_MSG_SIZE];
                strncpy(msg, "Internal error: bad trace format type\n", BUFFER_MSG_SIZE);
                msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
                abort_with_fatal_error(thisAgent, msg);
            }
        }
    }
}

/* ======================================================================
                    Trace Format Specification Tables

   We maintain tables of object trace formats and selection trace formats.
   Trace formats that apply to any *|g|p|s|o are stored in the arrays
   object_tf_for_anything[] and stack_tf_for_anything[].  (The array
   entry is NIL if no trace format has been specified.)  Trace formats that
   apply to *|g|p|s|o's with a certain name are stored in the hash tables
   object_tr_ht[] and stack_tr_ht[].  (Hash tables are used so we can
   look up the trace format from an object's name quickly.)

   Init_tracing() initializes the tables; at this point, there are no trace
   formats for anything.  This routine should be called at startup time.

   Trace formats are changed by calls to add_trace_format() and
   remove_trace_format().  Lookup_trace_format() returns the trace
   format matching a given type restriction and/or name restriction,
   or NIL if no such format has been specified.  Add_trace_format() returns
   true if the format was successfully added, or false if the format
   string didn't parse right.  Remove_trace_format() returns true if
   a trace format was actually removed, or false if there was no such
   trace format for the given type/name restrictions.  These routines take
   a "stack_trace" argument, which should be true if the stack trace
   format is intended, or false if the object trace format is intended.
   Their "type_restriction" argument should be one of FOR_ANYTHING_TF,
   ..., FOR_OPERATORS_TF .  The "name_restriction"
   argument should be either a pointer to a symbol, if the trace format
   is restricted to apply to objects with that name, or NIL if the format
   can apply to any object.

   Print_all_trace_formats() prints out either all existing stack trace
   or object trace formats.
====================================================================== */

/* --- trace formats that don't test the object name --- */

typedef struct tracing_rule_struct
{
    /* Warning: this MUST be the first field, for the hash table routines */
    struct tracing_rule_struct* next_in_hash_bucket;
    int type_restriction;      /* FOR_STATES_TF, etc. */
    Symbol* name_restriction;  /* points to name Symbol, or NIL */
    trace_format* format;      /* indicates the format to use */
} tracing_rule;

/*#define hash_name_restriction(name,num_bits) \
  ((name)->hash_id & masks_for_n_low_order_bits[(num_bits)])*/
inline uint32_t hash_name_restriction(Symbol* name, short num_bits)
{
    return name->hash_id & masks_for_n_low_order_bits[num_bits];
}

/* --- hash function for resizable hash table routines --- */
uint32_t tracing_rule_hash_function(void* item, short num_bits)
{
    tracing_rule* tr;

    tr = static_cast<tracing_rule_struct*>(item);
    return hash_name_restriction(tr->name_restriction, num_bits);
}

/* --- hash tables for stack and object traces --- */


void init_tracing(agent* thisAgent)
{
    int i;

    for (i = 0; i < 3; i++)
    {
        thisAgent->object_tr_ht[i] = make_hash_table(thisAgent, 0, tracing_rule_hash_function);
        thisAgent->stack_tr_ht[i] = make_hash_table(thisAgent, 0, tracing_rule_hash_function);
        thisAgent->object_tf_for_anything[i] = NIL;
        thisAgent->stack_tf_for_anything[i] = NIL;
    }
}

trace_format* lookup_trace_format(agent* thisAgent,
                                  bool stack_trace,
                                  int type_restriction,
                                  Symbol* name_restriction)
{
    uint32_t hash_value;
    hash_table* ht;
    tracing_rule* tr;

    if (name_restriction)
    {
        if (stack_trace)
        {
            ht = thisAgent->stack_tr_ht[type_restriction];
        }
        else
        {
            ht = thisAgent->object_tr_ht[type_restriction];
        }
        hash_value = hash_name_restriction(name_restriction, ht->log2size);
        tr = reinterpret_cast<tracing_rule*>(*(ht->buckets + hash_value));
        for (; tr != NIL; tr = tr->next_in_hash_bucket)
            if (tr->name_restriction == name_restriction)
            {
                return tr->format;
            }
        return NIL;
    }
    /* --- no name restriction --- */
    if (stack_trace)
    {
        return thisAgent->stack_tf_for_anything[type_restriction];
    }
    else
    {
        return thisAgent->object_tf_for_anything[type_restriction];
    }
}

bool remove_trace_format(agent* thisAgent,
                         bool stack_trace,
                         int type_restriction,
                         Symbol* name_restriction)
{
    uint32_t hash_value;
    hash_table* ht;
    tracing_rule* tr;
    trace_format** format;

    if (name_restriction)
    {
        if (stack_trace)
        {
            ht = thisAgent->stack_tr_ht[type_restriction];
        }
        else
        {
            ht = thisAgent->object_tr_ht[type_restriction];
        }
        hash_value = hash_name_restriction(name_restriction, ht->log2size);
        tr = reinterpret_cast<tracing_rule*>(*(ht->buckets + hash_value));
        for (; tr != NIL; tr = tr->next_in_hash_bucket)
            if (tr->name_restriction == name_restriction)
            {
                break;
            }
        if (! tr)
        {
            return false;
        }
        deallocate_trace_format_list(thisAgent, tr->format);
        remove_from_hash_table(thisAgent, ht, tr);
        thisAgent->memoryManager->free_memory(tr, MISCELLANEOUS_MEM_USAGE);
        thisAgent->symbolManager->symbol_remove_ref(&name_restriction);
        return true;
    }
    /* --- no name restriction --- */
    if (stack_trace)
    {
        format = &(thisAgent->stack_tf_for_anything[type_restriction]);
    }
    else
    {
        format = &(thisAgent->object_tf_for_anything[type_restriction]);
    }
    if (! *format)
    {
        return false;
    }
    deallocate_trace_format_list(thisAgent, *format);
    *format = NIL;
    return true;
}

bool add_trace_format(agent* thisAgent,
                      bool stack_trace,
                      int type_restriction,
                      Symbol* name_restriction,
                      const char* format_string)
{
    tracing_rule* tr;
    trace_format* new_tf;
    hash_table* ht;

    /* --- parse the format string into a trace_format --- */
    new_tf = parse_format_string(thisAgent, format_string);
    if (!new_tf)
    {
        return false;
    }

    /* --- first remove any existing trace format with same conditions --- */
    remove_trace_format(thisAgent, stack_trace, type_restriction, name_restriction);

    /* --- now add the new one --- */
    if (name_restriction)
    {
        thisAgent->symbolManager->symbol_add_ref(name_restriction);
        if (stack_trace)
        {
            ht = thisAgent->stack_tr_ht[type_restriction];
        }
        else
        {
            ht = thisAgent->object_tr_ht[type_restriction];
        }
        tr = static_cast<tracing_rule_struct*>(thisAgent->memoryManager->allocate_memory(sizeof(tracing_rule),
                                               MISCELLANEOUS_MEM_USAGE));
        tr->type_restriction = type_restriction;
        tr->name_restriction = name_restriction;
        tr->format = new_tf;
        add_to_hash_table(thisAgent, ht, tr);
        return true;
    }
    /* --- no name restriction --- */
    if (stack_trace)
    {
        thisAgent->stack_tf_for_anything[type_restriction] = new_tf;
    }
    else
    {
        thisAgent->object_tf_for_anything[type_restriction] = new_tf;
    }

    return true;
}

char tracing_object_letters[3] = {'*', 's', 'o'};

void print_tracing_rule(agent* thisAgent, int type_restriction, Symbol* name_restriction,
                        trace_format* format)
{
    if (thisAgent->printing_stack_traces)
    {
        thisAgent->outputManager->printa(thisAgent, "stack-trace-format");
    }
    else
    {
        thisAgent->outputManager->printa(thisAgent, "object-trace-format");
    }
    thisAgent->outputManager->printa_sf(thisAgent,  " :add %c ", tracing_object_letters[type_restriction]);
    if (name_restriction)
    {
        thisAgent->outputManager->printa_sf(thisAgent, "%y ", name_restriction);
    }
    thisAgent->outputManager->printa(thisAgent, "\"");
    print_trace_format_list(thisAgent, format);
    thisAgent->outputManager->printa_sf(thisAgent,  "\"\n");
}

void print_tracing_rule_tcl(agent* thisAgent, int type_restriction, Symbol* name_restriction,
                            trace_format* format)
{
    thisAgent->outputManager->printa_sf(thisAgent,  "%c ", tracing_object_letters[type_restriction]);
    if (name_restriction)
    {
        thisAgent->outputManager->printa_sf(thisAgent, "%y ", name_restriction);
    }
    thisAgent->outputManager->printa(thisAgent, "{");
    print_trace_format_list(thisAgent, format);
    thisAgent->outputManager->printa_sf(thisAgent,  "}\n");
}


bool print_trace_callback_fn(agent* thisAgent, void* item, void*)
{
    tracing_rule* tr;

    tr = static_cast<tracing_rule_struct*>(item);
    print_tracing_rule(thisAgent, tr->type_restriction, tr->name_restriction, tr->format);
    return false;
}

bool print_trace_callback_fn_tcl(agent* thisAgent, void* item, void*)
{
    tracing_rule* tr;

    tr = static_cast<tracing_rule_struct*>(item);
    print_tracing_rule_tcl(thisAgent, tr->type_restriction, tr->name_restriction,
                           tr->format);
    return false;
}

void print_all_trace_formats(agent* thisAgent, bool stack_trace, FILE* f)
{
    int i;

    thisAgent->printing_stack_traces = stack_trace;
    if (stack_trace)
    {
        for (i = 0; i < 3; i++)
        {
            if (thisAgent->stack_tf_for_anything[i])
            {
                print_tracing_rule(thisAgent, i, NIL, thisAgent->stack_tf_for_anything[i]);
            }
            do_for_all_items_in_hash_table(thisAgent, thisAgent->stack_tr_ht[i], print_trace_callback_fn, reinterpret_cast<void*>(f));
        }
    }
    else
    {
        for (i = 0; i < 3; i++)
        {
            if (thisAgent->object_tf_for_anything[i])
            {
                print_tracing_rule(thisAgent, i, NIL, thisAgent->object_tf_for_anything[i]);
            }
            do_for_all_items_in_hash_table(thisAgent, thisAgent->object_tr_ht[i], print_trace_callback_fn, reinterpret_cast<void*>(f));
        }
    }
}

void print_all_trace_formats_tcl(agent* thisAgent, bool stack_trace, FILE* f)
{
    int i;

    thisAgent->printing_stack_traces = stack_trace;
    if (stack_trace)
    {
        for (i = 0; i < 3; i++)
        {
            if (thisAgent->stack_tf_for_anything[i])
            {
                print_tracing_rule_tcl(thisAgent, i, NIL, thisAgent->stack_tf_for_anything[i]);
            }
            do_for_all_items_in_hash_table(thisAgent, thisAgent->stack_tr_ht[i], print_trace_callback_fn_tcl, reinterpret_cast<void*>(f));
        }
    }
    else
    {
        for (i = 0; i < 3; i++)
        {
            if (thisAgent->object_tf_for_anything[i])
            {
                print_tracing_rule_tcl(thisAgent, i, NIL, thisAgent->object_tf_for_anything[i]);
            }
            do_for_all_items_in_hash_table(thisAgent, thisAgent->object_tr_ht[i], print_trace_callback_fn_tcl, reinterpret_cast<void*>(f));
        }
    }
}

inline void set_print_trace_formats(agent* thisAgent)
{
    /* --- add default object trace formats --- */
    add_trace_format(thisAgent, false, FOR_ANYTHING_TF, NIL,
                     "%id %ifdef[(%v[name])]");
    add_trace_format(thisAgent, false, FOR_STATES_TF, NIL,
                     "%id %ifdef[(%v[attribute] %v[impasse])]");
    /***********  enable when determine tagged output format
    { Symbol *evaluate_object_sym;
      evaluate_object_sym = make_str_constant (thisAgent, "evaluate-object");
      add_trace_format (thisAgent, false, FOR_OPERATORS_TF, evaluate_object_sym,
                        "%id (evaluate-object %o[object])");
      symbol_remove_ref (thisAgent, evaluate_object_sym);
    }
    *************/
    /* --- add default stack trace formats --- */
    add_trace_format(thisAgent, true, FOR_STATES_TF, NIL,
                     "%right[6,%dc]: %rsd[   ]==>S: %cs");
    add_trace_format(thisAgent, true, FOR_OPERATORS_TF, NIL,
                     "%right[6,%dc]: %rsd[   ]   O: %co");
}
inline void set_tagged_trace_formats(agent* thisAgent)
{
    // KJC 03/05:  trying this for tagged output

    /* --- add tagged object trace formats --- */
    add_trace_format(thisAgent, false, FOR_ANYTHING_TF, NIL,
                     "%id\" %ifdef[name=\"%v[name]\"]");
    add_trace_format(thisAgent, false, FOR_STATES_TF, NIL,
                     "%id\" %ifdef[impasse_object=\"%v[attribute]\" impasse_type=\"%v[impasse]\"]");
    /***********  enable when determine tagged output format
    { Symbol *evaluate_object_sym;
      evaluate_object_sym = make_str_constant (thisAgent, "evaluate-object");
      add_trace_format (thisAgent, false, FOR_OPERATORS_TF, evaluate_object_sym,
                        "%id (evaluate-object %o[object])");
      symbol_remove_ref (thisAgent, evaluate_object_sym);
    }
    *************/
    /* --- add tagged stack trace formats --- */

    add_trace_format(thisAgent, true, FOR_STATES_TF, NIL,
                     "<state stack_level=\"%sd\" decision_cycle_count=\"%dc\" current_state_id=\"%cs>");
    add_trace_format(thisAgent, true, FOR_OPERATORS_TF, NIL,
                     "<operator stack_level=\"%sd\" decision_cycle_count=\"%dc\" current_operator_id=\"%co>");
}


/* ======================================================================
                      Trace Format List To String

   Trace_format_list_to_string() is the main routine which, given a
   trace format and a current object, builds and returns a growable_string
   for that object's printout.  A number of helper routines are used by
   trace_format_list_to_string().
====================================================================== */

growable_string object_to_trace_string(agent* thisAgent, Symbol* object);


bool found_undefined;   /* set to true whenever an escape sequence result is
                           undefined--for use with %ifdef */

struct tracing_parameters
{
    Symbol* current_s;          /* current state, etc. -- for use in %cs, etc. */
    Symbol* current_o;
    bool allow_cycle_counts;    /* true means allow %dc and %ec */
} tparams;

/* ----------------------------------------------------------------
   Adds all values of the given attribute path off the given object
   to the "*result" growable_string.  If "recursive" is true, the
   values are printed recursively as objects, rather than as simple
   atomic values.  "*count" is incremented each time a value is
   printed.  (To get a count of how many values were printed, the
   caller should initialize this to 0, then call this routine.)
---------------------------------------------------------------- */

void add_values_of_attribute_path(agent* thisAgent,
                                  Symbol* object,
                                  cons* path,
                                  growable_string* result,
                                  bool recursive,
                                  int* count)
{
    slot* s;
    wme* w;
    char* ch;
    growable_string gs;

    if (!path)    /* path is NIL, so we've reached the end of the path */
    {
        add_to_growable_string(thisAgent, result, " ");
        if (recursive)
        {
            gs = object_to_trace_string(thisAgent, object);
            add_to_growable_string(thisAgent, result, text_of_growable_string(gs));
            free_growable_string(thisAgent, gs);
        }
        else
        {
            ch = object->to_string(true, false, NULL, 0);
            add_to_growable_string(thisAgent, result, ch);
        }
        (*count)++;
        return;
    }

    /* --- not at end of path yet --- */
    /* --- can't follow any more path segments off of a non-identifier --- */
    if (object->symbol_type != IDENTIFIER_SYMBOL_TYPE)
    {
        return;
    }

    /* --- call this routine recursively on any wme matching the first segment
       of the attribute path --- */
    for (w = object->id->impasse_wmes; w != NIL; w = w->next)
        if (w->attr == path->first)
            add_values_of_attribute_path(thisAgent, w->value, path->rest, result, recursive,
                                         count);
    for (w = object->id->input_wmes; w != NIL; w = w->next)
        if (w->attr == path->first)
            add_values_of_attribute_path(thisAgent, w->value, path->rest, result, recursive,
                                         count);
    s = find_slot(object, static_cast<symbol_struct*>(path->first));
    if (s)
    {
        for (w = s->wmes; w != NIL; w = w->next)
            add_values_of_attribute_path(thisAgent, w->value, path->rest, result, recursive,
                                         count);
    }
}

/* ----------------------------------------------------------------
   Adds info for a wme to the given "*result" growable_string. If
   "print_attribute" is true, then "^att-name" is included.  If
   "recursive" is true, the value is printed recursively as an
   object, rather than as a simple atomic value.
---------------------------------------------------------------- */

void add_trace_for_wme(agent* thisAgent,
                       growable_string* result,
                       wme* w,
                       bool print_attribute,
                       bool recursive)
{
    char* ch;
    growable_string gs;

    add_to_growable_string(thisAgent, result, " ");
    if (print_attribute)
    {
        add_to_growable_string(thisAgent, result, "^");
        ch = w->attr->to_string(true);
        add_to_growable_string(thisAgent, result, ch);
        add_to_growable_string(thisAgent, result, " ");
    }
    if (recursive)
    {
        gs = object_to_trace_string(thisAgent, w->value);
        add_to_growable_string(thisAgent, result, text_of_growable_string(gs));
        free_growable_string(thisAgent, gs);
    }
    else
    {
        ch = w->value->to_string(true);
        add_to_growable_string(thisAgent, result, ch);
    }
}

/* ----------------------------------------------------------------
   Adds the trace for values of a given attribute path off a given
   object, to the given "*result" growable_string.  If
   "print_attributes" is true, then "^att-name" is included.  If
   "recursive" is true, the values are printed recursively as
   objects, rather than as a simple atomic value.  If the given path
   is NIL, then all values of all attributes of the given object
   are printed.
---------------------------------------------------------------- */

void add_trace_for_attribute_path(agent* thisAgent,
                                  Symbol* object,
                                  cons* path,
                                  growable_string* result,
                                  bool print_attributes,
                                  bool recursive)
{
    growable_string values;
    cons* c;
    char* ch;
    int count;
    slot* s;
    wme* w;

    values = make_blank_growable_string(thisAgent);

    if (! path)
    {
        if (object->symbol_type != IDENTIFIER_SYMBOL_TYPE)
        {
            return;
        }
        for (s = object->id->slots; s != NIL; s = s->next)
            for (w = s->wmes; w != NIL; w = w->next)
            {
                add_trace_for_wme(thisAgent, &values, w, print_attributes, recursive);
            }
        for (w = object->id->impasse_wmes; w != NIL; w = w->next)
        {
            add_trace_for_wme(thisAgent, &values, w, print_attributes, recursive);
        }
        for (w = object->id->input_wmes; w != NIL; w = w->next)
        {
            add_trace_for_wme(thisAgent, &values, w, print_attributes, recursive);
        }
        if (length_of_growable_string(values) > 0)
        {
            add_to_growable_string(thisAgent, result, text_of_growable_string(values) + 1);
        }
        free_growable_string(thisAgent, values);
        return;
    }

    count = 0;
    add_values_of_attribute_path(thisAgent, object, path, &values, recursive, &count);
    if (! count)
    {
        found_undefined = true;
        free_growable_string(thisAgent, values);
        return;
    }

    if (print_attributes)
    {
        add_to_growable_string(thisAgent, result, "^");
        for (c = path; c != NIL; c = c->rest)
        {
            ch = static_cast<symbol_struct*>(c->first)->to_string(true);
            add_to_growable_string(thisAgent, result, ch);
            if (c->rest)
            {
                add_to_growable_string(thisAgent, result, ".");
            }
        }
        add_to_growable_string(thisAgent, result, " ");
    }
    if (length_of_growable_string(values) > 0)
    {
        add_to_growable_string(thisAgent, result, text_of_growable_string(values) + 1);
    }
    free_growable_string(thisAgent, values);
}

/* ----------------------------------------------------------------
   This is the main routine here.  It returns a growable_string,
   given a trace format list (the format to use) and an object (the
   object being printed).
---------------------------------------------------------------- */

growable_string trace_format_list_to_string(agent* thisAgent, trace_format* tf, Symbol* object)
{
#define GROWABLE_STRING_TRACE_FORMAT_LIST_TO_STRING_BUFFER_SIZE 50
    char buf[GROWABLE_STRING_TRACE_FORMAT_LIST_TO_STRING_BUFFER_SIZE], *ch;
    growable_string result, temp_gs;
    int i;

    result = make_blank_growable_string(thisAgent);

    for (; tf != NIL; tf = tf->next)
    {
        switch (tf->type)
        {
            case STRING_TFT:
                add_to_growable_string(thisAgent, &result, tf->data.string);
                break;
            case PERCENT_TFT:
                add_to_growable_string(thisAgent, &result, "%");
                break;
            case L_BRACKET_TFT:
                add_to_growable_string(thisAgent, &result, "[");
                break;
            case R_BRACKET_TFT:
                add_to_growable_string(thisAgent, &result, "]");
                break;

            case VALUES_TFT:
                add_trace_for_attribute_path(thisAgent, object, tf->data.attribute_path, &result,
                                             false, false);
                break;
            case VALUES_RECURSIVELY_TFT:
                add_trace_for_attribute_path(thisAgent, object, tf->data.attribute_path, &result,
                                             false, true);
                break;
            case ATTS_AND_VALUES_TFT:
                add_trace_for_attribute_path(thisAgent, object, tf->data.attribute_path, &result,
                                             true, false);
                break;
            case ATTS_AND_VALUES_RECURSIVELY_TFT:
                add_trace_for_attribute_path(thisAgent, object, tf->data.attribute_path, &result,
                                             true, true);
                break;

            case CURRENT_STATE_TFT:
                if (! tparams.current_s)
                {
                    found_undefined = true;
                }
                else
                {
                    temp_gs = object_to_trace_string(thisAgent, tparams.current_s);

                    // KJC added to play with tagged output...
                    //add_to_growable_string (thisAgent, &result, "id=");

                    add_to_growable_string(thisAgent, &result, text_of_growable_string(temp_gs));

                    free_growable_string(thisAgent, temp_gs);
                }
                break;
            case CURRENT_OPERATOR_TFT:
                if (! tparams.current_o)
                {
                    found_undefined = true;
                }
                else
                {
                    temp_gs = object_to_trace_string(thisAgent, tparams.current_o);
                    add_to_growable_string(thisAgent, &result, text_of_growable_string(temp_gs));
                    free_growable_string(thisAgent, temp_gs);
                }
                break;

            case DECISION_CYCLE_COUNT_TFT:
                if (tparams.allow_cycle_counts)
                {
                    SNPRINTF(buf, GROWABLE_STRING_TRACE_FORMAT_LIST_TO_STRING_BUFFER_SIZE, "%" SCNu64, thisAgent->d_cycle_count);
                    buf[GROWABLE_STRING_TRACE_FORMAT_LIST_TO_STRING_BUFFER_SIZE - 1] = 0; /* ensure null termination */
                    add_to_growable_string(thisAgent, &result, buf);
                }
                else
                {
                    found_undefined = true;
                }
                break;
            case ELABORATION_CYCLE_COUNT_TFT:
                if (tparams.allow_cycle_counts)
                {
                    SNPRINTF(buf, GROWABLE_STRING_TRACE_FORMAT_LIST_TO_STRING_BUFFER_SIZE, "%" SCNu64, thisAgent->e_cycle_count);
                    buf[GROWABLE_STRING_TRACE_FORMAT_LIST_TO_STRING_BUFFER_SIZE - 1] = 0; /* ensure null termination */
                    add_to_growable_string(thisAgent, &result, buf);
                }
                else
                {
                    found_undefined = true;
                }
                break;

            case IDENTIFIER_TFT:
                ch = object->to_string(true);
                add_to_growable_string(thisAgent, &result, ch);
                break;

            case IF_ALL_DEFINED_TFT:
            {
                bool saved_found_undefined;
                saved_found_undefined = found_undefined;
                found_undefined = false;
                temp_gs = trace_format_list_to_string(thisAgent, tf->data.subformat, object);
                if (! found_undefined)
                {
                    add_to_growable_string(thisAgent, &result, text_of_growable_string(temp_gs));
                }
                free_growable_string(thisAgent, temp_gs);
                found_undefined = saved_found_undefined;
            }
            break;

            case LEFT_JUSTIFY_TFT:
                temp_gs = trace_format_list_to_string(thisAgent, tf->data.subformat, object);
                add_to_growable_string(thisAgent, &result, text_of_growable_string(temp_gs));
                for (i = tf->num - length_of_growable_string(temp_gs); i > 0; i--)
                {
                    add_to_growable_string(thisAgent, &result, " ");
                }
                free_growable_string(thisAgent, temp_gs);
                break;

            case RIGHT_JUSTIFY_TFT:
                temp_gs = trace_format_list_to_string(thisAgent, tf->data.subformat, object);
                for (i = tf->num - length_of_growable_string(temp_gs); i > 0; i--)
                {
                    add_to_growable_string(thisAgent, &result, " ");
                }
                add_to_growable_string(thisAgent, &result, text_of_growable_string(temp_gs));
                free_growable_string(thisAgent, temp_gs);
                break;

            case SUBGOAL_DEPTH_TFT:
                if (tparams.current_s)
                {
                    SNPRINTF(buf, GROWABLE_STRING_TRACE_FORMAT_LIST_TO_STRING_BUFFER_SIZE, "%u", tparams.current_s->id->level - 1);
                    buf[GROWABLE_STRING_TRACE_FORMAT_LIST_TO_STRING_BUFFER_SIZE - 1] = 0; /* ensure null termination */
                    add_to_growable_string(thisAgent, &result, buf);
                }
                else
                {
                    found_undefined = true;
                }
                break;

            case REPEAT_SUBGOAL_DEPTH_TFT:
                if (tparams.current_s)
                {
                    temp_gs = trace_format_list_to_string(thisAgent, tf->data.subformat, object);
                    for (i = tparams.current_s->id->level - 1; i > 0; i--)
                    {
                        add_to_growable_string(thisAgent, &result, text_of_growable_string(temp_gs));
                    }
                    free_growable_string(thisAgent, temp_gs);
                }
                else
                {
                    found_undefined = true;
                }
                break;

            case NEWLINE_TFT:
                add_to_growable_string(thisAgent, &result, "\n");
                break;

            default:
            {
                char msg[BUFFER_MSG_SIZE];
                strncpy(msg, "Internal error: bad trace format type\n", BUFFER_MSG_SIZE);
                msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
                abort_with_fatal_error(thisAgent, msg);
            }
        }
    }
    return result;
}

/* ======================================================================
               Building Traces for Object and Selections

   Find_appropriate_trace_format() looks for an applicable trace_format
   among the current set of tracing rules.

   Object_to_trace_string() takes an object and returns a growable_string
   to use for its printed trace.

   Selection_to_trace_string() takes an object (the current selection),
   the current state, a "selection_type" (one of FOR_OPERATORS_TF, etc.),
   and a flag indicating whether %dc, %ec, etc. escapes should be
   allowed, and returns a growable_string to use for the trace.
====================================================================== */


/* prevents infinite loops when printing circular
     structures */

trace_format* find_appropriate_trace_format(agent* thisAgent,
        bool stack_trace,
        int type,
        Symbol* name)
{
    trace_format* tf;

    /* --- first try to find the exact one --- */
    tf = lookup_trace_format(thisAgent, stack_trace, type, name);
    if (tf)
    {
        return tf;
    }

    /* --- failing that, try ignoring the type but retaining the name --- */
    if (type != FOR_ANYTHING_TF)
    {
        tf = lookup_trace_format(thisAgent, stack_trace, FOR_ANYTHING_TF, name);
        if (tf)
        {
            return tf;
        }
    }

    /* --- failing that, try ignoring the name but retaining the type --- */
    if (name)
    {
        tf = lookup_trace_format(thisAgent, stack_trace, type, NIL);
        if (tf)
        {
            return tf;
        }
    }

    /* --- last resort: find a format that applies to anything at all --- */
    return lookup_trace_format(thisAgent, stack_trace, FOR_ANYTHING_TF, NIL);
}

growable_string object_to_trace_string(agent* thisAgent, Symbol* object)
{
    growable_string gs;
    int type_of_object;
    trace_format* tf;
    Symbol* name;
    struct tracing_parameters saved_tparams;

    /* --- If it's not an identifier, just print it as an atom.  Also, if it's
       already being printed, print it as an atom to avoid getting into an
       infinite loop. --- */
    if ((object->symbol_type != IDENTIFIER_SYMBOL_TYPE) ||
            (object->tc_num == thisAgent->tf_printing_tc))
    {
        gs = make_blank_growable_string(thisAgent);
        add_to_growable_string(thisAgent, &gs, object->to_string(true));
        return gs;
    }

    /* --- mark it as being printed --- */
    object->tc_num = thisAgent->tf_printing_tc;

    /* --- determine the type and name of the object --- */
    if (object->id->isa_goal)
    {
        type_of_object = FOR_STATES_TF;
    }
    else if (object->id->isa_operator)
    {
        type_of_object = FOR_OPERATORS_TF;
    }
    else
    {
        type_of_object = FOR_ANYTHING_TF;
    }

    name = find_name_of_object(thisAgent, object);

    /* --- find the trace format to use --- */
    tf = find_appropriate_trace_format(thisAgent, false, type_of_object, name);

    /* --- now call trace_format_list_to_string() --- */
    if (tf)
    {
        saved_tparams = tparams;
        tparams.current_s = tparams.current_o = NIL;
        tparams.allow_cycle_counts = false;
        gs = trace_format_list_to_string(thisAgent, tf, object);
        tparams = saved_tparams;
    }
    else
    {
        /* --- no applicable trace format, so just print the object itself --- */
        gs = make_blank_growable_string(thisAgent);
        add_to_growable_string(thisAgent, &gs, object->to_string(true));
    }

    object->tc_num = 0;  /* unmark it now that we're done */
    return gs;
}

growable_string selection_to_trace_string(agent* thisAgent,
        Symbol* object,
        Symbol* current_state,
        int selection_type,
        bool allow_cycle_counts)
{
    trace_format* tf;
    Symbol* name;
    growable_string gs;
    struct tracing_parameters saved_tparams;

    /* --- find the problem space name --- */
    name = NIL;

    /* --- find the trace format to use --- */
    tf = find_appropriate_trace_format(thisAgent, true, selection_type, name);

    /* --- if there's no applicable trace format, print nothing --- */
    if (!tf)
    {
        return make_blank_growable_string(thisAgent);
    }

    /* --- save/restore tparams, and call trace_format_list_to_string() --- */
    saved_tparams = tparams;
    tparams.current_s = tparams.current_o = NIL;
    if (current_state)
    {
        tparams.current_s = current_state;
        if (current_state->id->operator_slot->wmes)
        {
            tparams.current_o = current_state->id->operator_slot->wmes->value;
        }
    }
    tparams.allow_cycle_counts = allow_cycle_counts;
    gs = trace_format_list_to_string(thisAgent, tf, object);
    tparams = saved_tparams;

    return gs;
}

/* ======================================================================
                   Printing Object and Stack Traces

   Print_object_trace() takes an object (any symbol).  It prints
   the trace for that object.  Print_stack_trace() takes a (context)
   object (the state or op), the current state, the "slot_type" (one
   of FOR_OPERATORS_TF, etc.), and a flag indicating whether to allow
   %dc and %ec escapes (this flag should normally be true for watch 0
   traces but false during a "pgs" command).  It prints the trace for
   that context object.
====================================================================== */

void print_object_trace(agent* thisAgent, Symbol* object)
{
    growable_string gs;

    thisAgent->tf_printing_tc  = get_new_tc_number(thisAgent);
    gs = object_to_trace_string(thisAgent, object);
    thisAgent->outputManager->printa(thisAgent, text_of_growable_string(gs));
    free_growable_string(thisAgent, gs);
}

void print_stack_trace_xml(agent* thisAgent, Symbol* object, Symbol* state, int slot_type, bool /*allow_cycle_counts*/)
{

    Symbol* current_o = 0;

    switch (slot_type)
    {
        case FOR_STATES_TF:
            //create XML trace for state object
            xml_begin_tag(thisAgent, kTagState);
            xml_att_val(thisAgent, kState_StackLevel, state->id->level - 1);
            xml_att_val(thisAgent, kState_DecisionCycleCt, thisAgent->d_cycle_count);
            xml_att_val(thisAgent, kState_ID, object);

            // find impasse object and add it to XML
            wme* w;
            for (w = object->id->impasse_wmes; w != NIL; w = w->next)
            {
                if (w->attr == thisAgent->symbolManager->soarSymbols.attribute_symbol)
                {
                    xml_att_val(thisAgent, kState_ImpasseObject, w->value->sc->name);
                    break;
                }
            }

            // find impasse type and add it to XML
            for (w = object->id->impasse_wmes; w != NIL; w = w->next)
            {
                if (w->attr == thisAgent->symbolManager->soarSymbols.impasse_symbol)
                {
                    xml_att_val(thisAgent, kState_ImpasseType, w->value->sc->name);
                    break;
                }
            }

            xml_end_tag(thisAgent, kTagState);
            break;

        case FOR_OPERATORS_TF:
            //create XML trace for operator object
            xml_begin_tag(thisAgent, kTagOperator);
            /* -- Prior to Soar 9.4, the following line printed (object->id.level - 1)
             *    instead of (state->id->level - 1).  This was causing the
             *    unit test to fail.  Changing it to state seems to make sense in this
             *    context, since the level of the state should also be the level of the
             *    operator.  (Could be wrong about this.)
             *    Leaving this comment because it's possible that the real problem is
             *    that the level of the operator isn't being set somewhere else to
             *    the proper value. - MMA 9-2014*/
            xml_att_val(thisAgent, kState_StackLevel, state->id->level - 1);
            xml_att_val(thisAgent, kOperator_DecisionCycleCt, thisAgent->d_cycle_count);

            if (state->id->operator_slot->wmes)
            {
                current_o = state->id->operator_slot->wmes->value;
            }
            if (current_o)
            {
                xml_att_val(thisAgent, kOperator_ID, current_o);
                Symbol* name = find_name_of_object(thisAgent, current_o);
                if (name)
                {
                    xml_att_val(thisAgent, kOperator_Name, name);
                }
            }

            xml_end_tag(thisAgent, kTagOperator);
            break;

        default:
            // technically it looks like we should try to print a name here, but I think that is an artifact (at least it doesn't make sense for the XML)
            break;
    }

    /* These are the trace formats which I've directly implemented above
    add_trace_format (thisAgent, true, FOR_STATES_TF, NIL,
                    "<state stack_level=\"%sd\" decision_cycle_count=\"%dc\" current_state_id=\"%cs>");
    add_trace_format (thisAgent, true, FOR_OPERATORS_TF, NIL,
                    "<operator stack_level=\"%sd\" decision_cycle_count=\"%dc\" current_operator_id=\"%co>");

    add_trace_format (thisAgent, false, FOR_ANYTHING_TF, NIL,
                    "%id\" %ifdef[name=\"%v[name]\"]");
    add_trace_format (thisAgent, false, FOR_STATES_TF, NIL,
                    "%id\" %ifdef[impasse_object=\"%v[attribute]\" impasse_type=\"%v[impasse]\"]");
    */
}


void print_stack_trace(agent* thisAgent, Symbol* object, Symbol* state, int slot_type,
                       bool allow_cycle_counts)
{
    growable_string gs;

    thisAgent->tf_printing_tc  = get_new_tc_number(thisAgent);
    gs = selection_to_trace_string(thisAgent, object, state, slot_type, allow_cycle_counts);
    thisAgent->outputManager->printa(thisAgent, text_of_growable_string(gs));
    free_growable_string(thisAgent, gs);

    // RPM 5/05
    print_stack_trace_xml(thisAgent, object, state, slot_type, allow_cycle_counts);
}

/* kjh(B1) begin */
void print_object_trace_using_provided_format_string(agent* thisAgent,
        Symbol* object,
        Symbol* current_goal,
        char*   format_string)
{
    growable_string gs;
    struct tracing_parameters saved_tparams;
    trace_format* fs;

    fs = parse_format_string(thisAgent, format_string);

    thisAgent->tf_printing_tc  = get_new_tc_number(thisAgent);

    saved_tparams = tparams;

    if (current_goal)
    {
        tparams.current_s = current_goal;
    }
    tparams.allow_cycle_counts = true;

    gs = trace_format_list_to_string(thisAgent, fs, object);

    tparams = saved_tparams;

    thisAgent->outputManager->start_fresh_line(thisAgent);

    thisAgent->outputManager->printa(thisAgent, text_of_growable_string(gs));
    free_growable_string(thisAgent, gs);
}
/* kjh(B1) end */

