/*************************************************************************
 *
 *  file:  print.c
 *
 * =======================================================================
 *  These are the routines that support printing Soar data structures.
 *  
 *  Everything eventually ends up in print_string which calls the Tcl
 *  interface routine Soar_LogAndPrint.  Logging and I/O redirection are
 *  now done through Tcl.  There's also one odd piece of user i/o done
 *  in decide.c to manage indifferent-selection -ask.  (it needs work)
 *  see more detailed comments in soarkernel.h
 * =======================================================================
 *
 * Copyright 1995-2004 Carnegie Mellon University,
 *										 University of Michigan,
 *										 University of Southern California/Information
 *										 Sciences Institute. All rights reserved.
 *										
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1.	Redistributions of source code must retain the above copyright notice,
 *		this list of conditions and the following disclaimer. 
 * 2.	Redistributions in binary form must reproduce the above copyright notice,
 *		this list of conditions and the following disclaimer in the documentation
 *		and/or other materials provided with the distribution. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE SOAR CONSORTIUM ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE SOAR CONSORTIUM  OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * The views and conclusions contained in the software and documentation are
 * those of the authors and should not be interpreted as representing official
 * policies, either expressed or implied, of Carnegie Mellon University, the
 * University of Michigan, the University of Southern California/Information
 * Sciences Institute, or the Soar consortium.
 * =======================================================================
 */
/* =================================================================
                 Printing Utility Routines for Soar 6
   ================================================================= */

#include "soarkernel.h"
#include <stdio.h>
#include <ctype.h>

#ifdef USE_STDARGS
#include <stdarg.h>
#else
#include <varargs.h>
#endif

/* -------------------------------------------------------------------
    Printing with an Optional Log File and with Redirection to a File

   We want to print stuff not only to the screen but also to a log
   file (if one is currently being used).  The print_string(), print(),
   print_with_symbols(), and print_spaces() routines do this.

   Start_log_file() and stop_log_file() open and close the current log
   file.  Print_string_to_log_file_only() is called by the lexer to
   echo keyboard input to the log file (it's already on the screen, so
   we don't want to print it there too).

   Print_string() and print_spaces() do the obvious things.
   Print() is exactly like printf() in C, except it prints to both
   the screen and log file (if there is one).  Print_with_symbols()
   is sort of like print, but only takes two kinds of escape sequences
   in the format string: 
       %y  -- print a symbol
       %%  -- print a "%" sign

   Sometimes we need to know the current output column so we can put
   a line break in the right place.  Get_printer_output_column() returns
   the current column number (1 means the start of the line). 
   Tell_printer_that_output_column_has_been_reset () is called from the
   lexer every time it reads a line from the keyboard--since after the
   user types a line (and hits return) the output column is reset.

   We also support temporarily redirecting all printing output to
   another file.  This is done by calling start_redirection_to_file()
   and stop_redirection_to_file().  In between these calls, all screen
   and log file output is turned off, and printing is done only to the
   redirection file.
------------------------------------------------------------------- */

void start_log_file(char *filename, bool append)
{
    if (current_agent(logging_to_file))
        stop_log_file();

    sys_chdir(current_agent(top_dir_stack)->directory); /* AGR 568 */
    current_agent(log_file) = fopen(filename, (append ? "a" : "w"));

    if (current_agent(log_file)) {
        current_agent(logging_to_file) = TRUE;
        current_agent(log_file_name) = make_memory_block_for_string(filename);
        print("Logging to file %s\n", filename);
    } else {
        /* --- error when opening the file --- */
        print("Error: unable to open file %s\n", filename);
    }
}

void stop_log_file(void)
{
    if (!current_agent(logging_to_file))
        return;
    print("Closing log file %s\n", current_agent(log_file_name));
    if (fclose(current_agent(log_file)))
        print("Error: unable to close file %s\n", current_agent(log_file_name));
    free_memory_block_for_string(current_agent(log_file_name));
    current_agent(logging_to_file) = FALSE;
}

void print_string_to_log_file_only(char *string)
{
    fputs(string, current_agent(log_file));
}

int get_printer_output_column(void)
{
    return current_agent(printer_output_column);
}

void tell_printer_that_output_column_has_been_reset(void)
{
    current_agent(printer_output_column) = 1;
}

void start_redirection_to_file(FILE * already_opened_file)
{
    current_agent(saved_printer_output_column) = current_agent(printer_output_column);
    current_agent(printer_output_column) = 1;
    current_agent(redirecting_to_file) = TRUE;
    current_agent(redirection_file) = already_opened_file;
}

void stop_redirection_to_file(void)
{
    current_agent(redirecting_to_file) = FALSE;
    current_agent(printer_output_column) = current_agent(saved_printer_output_column);
}

/* -----------------------------------------------------------------------
                             Print_string

   This routine prints the given string, and updates printer_output_column.  
   (This routine is called from the other print(), etc. routines.)
----------------------------------------------------------------------- */

void print_string(char *s)
{
    char *ch;

    for (ch = s; *ch != 0; ch++) {
        if (*ch == '\n') {
            current_agent(printer_output_column) = 1;
        } else {
            current_agent(printer_output_column)++;
        }
    }

    if (current_agent(redirecting_to_file)) {
        fputs(s, current_agent(redirection_file));
    } else {
        /* this code is never executed since using_output_string is never true
           if (current_agent(using_output_string)) 
           strcat(current_agent(output_string),s);
           else {
         */
        /* 
           A bunch of crazy, interface dependent functions used to be called
           here.  I am removing all of that, and using the Tcl interface
           as a model of what the generalized behavior should in fact be.
           The old version, which used the Tcl interface simply made a function
           call to the interface layer which then simply invoked the 
           LOG and PRINT callbacks, which seems like the way to go.

           081699 SW
         */

        soar_invoke_first_callback(soar_agent, LOG_CALLBACK, s);
        soar_invoke_first_callback(soar_agent, PRINT_CALLBACK, s);
    }
    if (current_agent(logging_to_file)) {
        fputs(s, current_agent(log_file));
    }
}

/* ---------------------------------------------------------------
               Print, Print_with_symbols, Print_spaces
  
   These are the main printing routines.  (The code is ugly because
   it has to take a variable number of arguments, and there are two
   ways to do this, depending on whether we're using a fully ANSI
   compatible compiler or not.)
--------------------------------------------------------------- */

/* --- size of output buffer for a single call to one of these routines --- */
#define PRINT_BUFSIZE 4096      /* This better be large enough!! */

#ifdef USE_STDARGS
void print(char *format, ...)
{
    va_list args;
    char buf[PRINT_BUFSIZE];

    va_start(args, format);
#else
void print(va_alist)
va_dcl
{
    va_list args;
    char *format;
    char buf[PRINT_BUFSIZE];

    va_start(args);
    format = va_arg(args, char *);
#endif
    vsnprintf(buf, PRINT_BUFSIZE, format, args);
    va_end(args);
    print_string(buf);
}

#ifdef USE_STDARGS
void print_with_symbols(char *format, ...)
{
    va_list args;
    char buf[PRINT_BUFSIZE];
    char *ch;

    va_start(args, format);
#else
void print_with_symbols(va_alist)
va_dcl
{
    va_list args;
    char buf[PRINT_BUFSIZE];
    char *ch, *format;

    va_start(args);
    format = va_arg(args, char *);
#endif
    ch = buf;

    for (;;) {
        /* --- copy anything up to the first "%" --- */
        while ((*format != '%') && (*format != 0))
            *(ch++) = *(format++);

        if (*format == 0)
            break;

        /* --- handle the %-thingy --- */
        if (*(format + 1) == 'y') {
            /* the size of the remaining buffer (after ch) is
               the difference between the address of ch and
               the address of the beginning of the buffer
             */
            symbol_to_string(va_arg(args, Symbol *), TRUE, ch, PRINT_BUFSIZE - (ch - buf));
            while (*ch)
                ch++;

        } else {
            *(ch++) = '%';
        }

        format += 2;
    }

    va_end(args);

    *ch = 0;
    print_string(buf);
}

void print_spaces(int n)
{
    /*char *ch;
       ch = buf;
       while (n) { *(ch++)=' '; n--; }
       *ch=0; */

    char buf[PRINT_BUFSIZE];

    if (n >= PRINT_BUFSIZE) {
        n = PRINT_BUFSIZE - 1;
    }

    memset(buf, ' ', n);

    buf[n] = 0;

    print_string(buf);
}

/* ------------------------------------------------------------------------
                String to Escaped String Conversion
           {Symbol, Test, RHS Value} to String Conversion

   These routines produce strings.  Each takes an optional parameter "dest"
   which, if non-nil, points to the destination buffer for the result string.
   If dest is nil, these routines use a global buffer, and return a pointer
   to it.  (Otherwise "dest" itself is returned.)  Note that a single global
   buffer is shared by all three routines, so callers should assume the
   buffer will be destroyed by the next call to these routines with dest=NIL.

   String_to_escaped_string() takes a string and a first/last char,
   and produces an "escaped string" representation of the string; i.e.,
   a string that uses '\' escapes to include special characters.
   For example, input 'ab"c' with first/last character '"' yields
   '"ab\"c"'.  This is used for printing quoted strings and for printing
   symbols using |vbar| notation.
 
   Symbol_to_string() converts a symbol to a string.  The "rereadable"
   parameter indicates whether a rereadable representation is desired.
   Normally symbols are printed rereadably, but for (write) and Text I/O,
   we don't want this.

   Test_to_string() takes a test and produces a string representation.
   Rhs_value_to_string() takes an rhs_value and produces a string
   representation.  The rhs_value MUST NOT be a reteloc.
----------------------------------------------------------------------- */

char *string_to_escaped_string(char *s, char first_and_last_char, char *dest)
{
    char *ch;

    if (!dest)
        dest = current_agent(printed_output_string);
    ch = dest;
    *ch++ = first_and_last_char;
    while (*s) {
        if ((*s == first_and_last_char) || (*s == '\\'))
            *ch++ = '\\';
        *ch++ = *s++;
    }
    *ch++ = first_and_last_char;
    *ch = 0;
    return dest;
}

char *symbol_to_string(Symbol * sym, bool rereadable, char *dest, size_t dest_size)
{
    bool possible_id, possible_var, possible_sc, possible_ic, possible_fc;
    bool is_rereadable;
    bool has_angle_bracket;

    if (!sym) {
		if (!dest) {
			dest = current_agent(printed_output_string);
			dest_size = PRINTED_OUTPUT_STRING_SIZE;
		}

        strncpy(dest, "(NULL)", dest_size);
        dest[dest_size - 1] = 0;
        return dest;
    }

    switch (sym->common.symbol_type) {
    case VARIABLE_SYMBOL_TYPE:
        if (!dest) {
            return sym->var.name;
		}

        strncpy(dest, sym->var.name, dest_size);
        dest[dest_size - 1] = 0;
        return dest;

    case IDENTIFIER_SYMBOL_TYPE:
        if (!dest) {
            dest = current_agent(printed_output_string);
            dest_size = PRINTED_OUTPUT_STRING_SIZE;
        }
        snprintf(dest, dest_size, "%c%lu", sym->id.name_letter, sym->id.name_number);
        dest[dest_size - 1] = 0;        /* snprintf doesn't set last char to null if output is truncated */
        return dest;

    case INT_CONSTANT_SYMBOL_TYPE:
        if (!dest) {
            dest = current_agent(printed_output_string);
            dest_size = PRINTED_OUTPUT_STRING_SIZE;
        }
        snprintf(dest, dest_size, "%ld", sym->ic.value);
        dest[dest_size - 1] = 0;        /* snprintf doesn't set last char to null if output is truncated */
        return dest;

    case FLOAT_CONSTANT_SYMBOL_TYPE:
        if (!dest) {
            dest = current_agent(printed_output_string);
            dest_size = PRINTED_OUTPUT_STRING_SIZE;
        }
        snprintf(dest, dest_size, "%#g", sym->fc.value);
        dest[dest_size - 1] = 0;        /* snprintf doesn't set last char to null if output is truncated */
        {                       /* --- strip off trailing zeros --- */
            char *start_of_exponent;
            char *end_of_mantissa;
            start_of_exponent = dest;
            while ((*start_of_exponent != 0) && (*start_of_exponent != 'e'))
                start_of_exponent++;
            end_of_mantissa = start_of_exponent - 1;
            while (*end_of_mantissa == '0')
                end_of_mantissa--;
            end_of_mantissa++;
            while (*start_of_exponent)
                *end_of_mantissa++ = *start_of_exponent++;
            *end_of_mantissa = 0;
        }
        return dest;

    case SYM_CONSTANT_SYMBOL_TYPE:
        if (!rereadable) {
            if (!dest)
                return sym->sc.name;
            strncpy(dest, sym->sc.name, dest_size);
            dest[dest_size - 1] = 0;
            return dest;
        }
        determine_possible_symbol_types_for_string(sym->sc.name,
                                                   strlen(sym->sc.name),
                                                   &possible_id,
                                                   &possible_var,
                                                   &possible_sc, &possible_ic, &possible_fc, &is_rereadable);

        has_angle_bracket = (bool) (sym->sc.name[0] == '<' || sym->sc.name[strlen(sym->sc.name) - 1] == '>');

        if ((!possible_sc) || possible_var || possible_ic || possible_fc || (!is_rereadable) || has_angle_bracket) {
            /* BUGBUG if in context where id's could occur, should check
               possible_id flag here also */
            return string_to_escaped_string(sym->sc.name, '|', dest);
        }
        if (!dest)
            return sym->sc.name;
        strncpy(dest, sym->sc.name, dest_size);
        dest[dest_size - 1] = 0;
        return dest;

    default:
        {
            char msg[MESSAGE_SIZE];
            strncpy(msg, "Internal Soar Error:  symbol_to_string called on bad symbol\n", MESSAGE_SIZE);
            msg[MESSAGE_SIZE - 1] = 0;

            abort_with_fatal_error(msg);
        }
    }
    return NIL;                 /* unreachable, but without it, gcc -Wall warns here */
}

char *test_to_string(test t, char *dest, size_t dest_size)
{
    cons *c;
    complex_test *ct;
    char *ch;

    if (test_is_blank_test(t)) {
        if (!dest) {
            dest = current_agent(printed_output_string);
			dest_size = PRINTED_OUTPUT_STRING_SIZE;
		}
        snprintf(dest, dest_size, "[BLANK TEST]");      /* this should never get executed */
        dest[dest_size - 1] = 0;        /* snprintf doesn't set last char to null if output is truncated */
        return dest;
    }

    if (test_is_blank_or_equality_test(t)) {
        return symbol_to_string(referent_of_equality_test(t), TRUE, dest, dest_size);
    }

    if (!dest) {
        dest = current_agent(printed_output_string);
		dest_size = PRINTED_OUTPUT_STRING_SIZE;
	}

    ch = dest;
    ct = complex_test_from_test(t);

    switch (ct->type) {
    case NOT_EQUAL_TEST:
        strncpy(ch, "<> ", dest_size - (ch - dest));
        ch[dest_size - (ch - dest) - 1] = 0;
        while (*ch)
            ch++;
        symbol_to_string(ct->data.referent, TRUE, ch, dest_size - (ch - dest));
        break;
    case LESS_TEST:
        strncpy(ch, "< ", dest_size - (ch - dest));
        ch[dest_size - (ch - dest) - 1] = 0;
        while (*ch)
            ch++;
        symbol_to_string(ct->data.referent, TRUE, ch, dest_size - (ch - dest));
        break;
    case GREATER_TEST:
        strncpy(ch, "> ", dest_size - (ch - dest));
        ch[dest_size - (ch - dest) - 1] = 0;
        while (*ch)
            ch++;
        symbol_to_string(ct->data.referent, TRUE, ch, dest_size - (ch - dest));
        break;
    case LESS_OR_EQUAL_TEST:
        strncpy(ch, "<= ", dest_size - (ch - dest));
        ch[dest_size - (ch - dest) - 1] = 0;
        while (*ch)
            ch++;
        symbol_to_string(ct->data.referent, TRUE, ch, dest_size - (ch - dest));
        break;
    case GREATER_OR_EQUAL_TEST:
        strncpy(ch, ">= ", dest_size - (ch - dest));
        ch[dest_size - (ch - dest) - 1] = 0;
        while (*ch)
            ch++;
        symbol_to_string(ct->data.referent, TRUE, ch, dest_size - (ch - dest));
        break;
    case SAME_TYPE_TEST:
        strncpy(ch, "<=> ", dest_size - (ch - dest));
        ch[dest_size - (ch - dest) - 1] = 0;
        while (*ch)
            ch++;
        symbol_to_string(ct->data.referent, TRUE, ch, dest_size - (ch - dest));
        break;
    case DISJUNCTION_TEST:
        strncpy(ch, "<< ", dest_size - (ch - dest));
        ch[dest_size - (ch - dest) - 1] = 0;
        while (*ch)
            ch++;
        for (c = ct->data.disjunction_list; c != NIL; c = c->rest) {
            symbol_to_string(c->first, TRUE, ch, dest_size - (ch - dest));
            while (*ch)
                ch++;
            *(ch++) = ' ';
        }
        strncpy(ch, ">>", dest_size - (ch - dest));
        ch[dest_size - (ch - dest) - 1] = 0;
        break;
    case CONJUNCTIVE_TEST:
        strncpy(ch, "{ ", dest_size - (ch - dest));
        ch[dest_size - (ch - dest) - 1] = 0;
        while (*ch)
            ch++;
        for (c = ct->data.conjunct_list; c != NIL; c = c->rest) {
            test_to_string(c->first, ch, dest_size - (ch - dest));
            while (*ch)
                ch++;
            *(ch++) = ' ';
        }
        strncpy(ch, "}", dest_size - (ch - dest));
        ch[dest_size - (ch - dest) - 1] = 0;
        break;
    case GOAL_ID_TEST:
        strncpy(dest, "[GOAL ID TEST]", dest_size);     /* this should never get executed */
        dest[dest_size - 1] = 0;
        break;
    case IMPASSE_ID_TEST:
        strncpy(dest, "[IMPASSE ID TEST]", dest_size);  /* this should never get executed */
        dest[dest_size - 1] = 0;
        break;
    }
    return dest;
}

char *rhs_value_to_string(rhs_value rv, char *dest, size_t dest_size)
{
    cons *c;
    list *fl;
    rhs_function *rf;
    char *ch;

    if (rhs_value_is_reteloc(rv)) {
        char msg[MESSAGE_SIZE];
        strncpy(msg, "Internal error: rhs_value_to_string called on reteloc.\n", MESSAGE_SIZE);
        msg[MESSAGE_SIZE - 1] = 0;
        abort_with_fatal_error(msg);
    }

    if (rhs_value_is_symbol(rv)) {
        return symbol_to_string(rhs_value_to_symbol(rv), TRUE, dest, dest_size);
    }

    fl = rhs_value_to_funcall_list(rv);
    rf = fl->first;

    if (!dest) {
        dest = current_agent(printed_output_string);
		dest_size = PRINTED_OUTPUT_STRING_SIZE;
	}
    ch = dest;

    strncpy(ch, "(", dest_size - (ch - dest));
    ch[dest_size - (ch - dest) - 1] = 0;
    while (*ch)
        ch++;

    if (!strcmp(rf->name->sc.name, "+")) {
        strncpy(ch, "(", dest_size - (ch - dest));
        ch[dest_size - (ch - dest) - 1] = 0;
    } else if (!strcmp(rf->name->sc.name, "-")) {
        strncpy(ch, "-", dest_size - (ch - dest));
        ch[dest_size - (ch - dest) - 1] = 0;
    } else {
        symbol_to_string(rf->name, TRUE, ch, dest_size - (ch - dest));
    }

    while (*ch)
        ch++;
    for (c = fl->rest; c != NIL; c = c->rest) {
        strncpy(ch, " ", dest_size - (ch - dest));
        ch[dest_size - (ch - dest) - 1] = 0;
        while (*ch)
            ch++;
        rhs_value_to_string(c->first, ch, dest_size - (ch - dest));
        while (*ch)
            ch++;
    }
    strncpy(ch, ")", dest_size - (ch - dest));
    ch[dest_size - (ch - dest) - 1] = 0;
    return dest;
}

/* ------------------------------------------------------------------
                        Print Condition List

   This prints a list of conditions.  The "indent" parameter tells
   how many spaces to indent each line other than the first--the first
   line is not indented (the caller must handle this).  The last line
   is printed without a trailing linefeed.  The "internal" parameter,
   if TRUE, indicates that the condition list should be printed in
   internal format--one condition per line, without grouping all the
   conditions for the same id into one line.
------------------------------------------------------------------ */

test id_test_to_match;

bool pick_conds_with_matching_id_test(dl_cons * dc)
{
    condition *cond;
    cond = dc->item;
    if (cond->type == CONJUNCTIVE_NEGATION_CONDITION)
        return FALSE;
    return tests_are_equal(id_test_to_match, cond->data.tests.id_test);
}

#define PRINT_CONDITION_LIST_TEMP_SIZE 10000
void print_condition_list(condition * conds, int indent, bool internal)
{
    bool did_one_line_already;
    dl_list *conds_not_yet_printed, *tail_of_conds_not_yet_printed;
    dl_list *conds_for_this_id;
    dl_cons *dc;
    condition *c;
    bool removed_goal_test, removed_impasse_test;
    test id_test;

    if (!conds)
        return;

    did_one_line_already = FALSE;

    /* --- build dl_list of all the actions --- */
    conds_not_yet_printed = NIL;
    tail_of_conds_not_yet_printed = NIL;
    for (c = conds; c != NIL; c = c->next) {
        allocate_with_pool(&current_agent(dl_cons_pool), &dc);
        dc->item = c;
        if (conds_not_yet_printed)
            tail_of_conds_not_yet_printed->next = dc;
        else
            conds_not_yet_printed = dc;
        dc->prev = tail_of_conds_not_yet_printed;
        tail_of_conds_not_yet_printed = dc;
    }
    tail_of_conds_not_yet_printed->next = NIL;

    /* --- main loop: find all conds for first id, print them together --- */
    while (conds_not_yet_printed) {
        if (did_one_line_already) {
            print("\n");
            print_spaces(indent);
        } else {
            did_one_line_already = TRUE;
        }
        dc = conds_not_yet_printed;
        remove_from_dll(conds_not_yet_printed, dc, next, prev);
        c = dc->item;

        if (c->type == CONJUNCTIVE_NEGATION_CONDITION) {
            free_with_pool(&current_agent(dl_cons_pool), dc);
            print_string("-{");
            print_condition_list(c->data.ncc.top, indent + 2, internal);
            print_string("}");
            continue;
        }

        /* --- normal pos/neg conditions --- */
        removed_goal_test = removed_impasse_test = FALSE;
        id_test = copy_test_removing_goal_impasse_tests
            (c->data.tests.id_test, &removed_goal_test, &removed_impasse_test);
        id_test_to_match = copy_of_equality_test_found_in_test(id_test);

        /* --- collect all cond's whose id test matches this one --- */
        conds_for_this_id = dc;
        dc->prev = NIL;
        if (internal) {
            dc->next = NIL;

        } else {
            dc->next = extract_dl_list_elements(&conds_not_yet_printed, pick_conds_with_matching_id_test);
        }

        /* --- print the collected cond's all together --- */
        print_string(" (");
        if (removed_goal_test)
            print_string("state ");
        if (removed_impasse_test)
            print_string("impasse ");
        print_string(test_to_string(id_test, NULL, 0));
        deallocate_test(id_test_to_match);
        deallocate_test(id_test);
        while (conds_for_this_id) {
            dc = conds_for_this_id;
            conds_for_this_id = conds_for_this_id->next;
            c = dc->item;
            free_with_pool(&current_agent(dl_cons_pool), dc);

            {                   /* --- build and print attr/value test for condition c --- */
                char temp[PRINT_CONDITION_LIST_TEMP_SIZE], *ch;

                ch = temp;

                strncpy(ch, " ", PRINT_CONDITION_LIST_TEMP_SIZE - (ch - temp));
                ch[PRINT_CONDITION_LIST_TEMP_SIZE - (ch - temp) - 1] = 0;
                if (c->type == NEGATIVE_CONDITION) {
                    strncat(ch, "-", PRINT_CONDITION_LIST_TEMP_SIZE - (ch - temp));
                    ch[PRINT_CONDITION_LIST_TEMP_SIZE - (ch - temp) - 1] = 0;
                    while (*ch)
                        ch++;
                }

                strncat(ch, "^", PRINT_CONDITION_LIST_TEMP_SIZE - (ch - temp));
                ch[PRINT_CONDITION_LIST_TEMP_SIZE - (ch - temp) - 1] = 0;
                while (*ch)
                    ch++;

                test_to_string(c->data.tests.attr_test, ch, PRINT_CONDITION_LIST_TEMP_SIZE - (ch - temp));
                while (*ch)
                    ch++;
                if (!test_is_blank_test(c->data.tests.value_test)) {
                    *(ch++) = ' ';
                    test_to_string(c->data.tests.value_test, ch, PRINT_CONDITION_LIST_TEMP_SIZE - (ch - temp));
                    while (*ch)
                        ch++;
                    if (c->test_for_acceptable_preference) {
                        strncpy(ch, " +", PRINT_CONDITION_LIST_TEMP_SIZE - (ch - temp));
                        ch[PRINT_CONDITION_LIST_TEMP_SIZE - (ch - temp) - 1] = 0;
                        while (*ch)
                            ch++;
                    }
                }
                *ch = 0;
                if (current_agent(printer_output_column) + (ch - temp) >= COLUMNS_PER_LINE) {
                    print_string("\n");
                    print_spaces(indent + 6);
                }
                print_string(temp);
            }
        }

        print_string(")");
    }                           /* end of while (conds_not_yet_printed) */
}

/* ------------------------------------------------------------------
                        Print Action List

   This prints a list of actions.  The "indent" parameter tells how
   many spaces to indent each line other than the first--the first
   line is not indented (the caller must handle this).  The last line
   is printed without a trailing linefeed.  The "internal" parameter,
   if TRUE, indicates that the action list should be printed in
   internal format--one action per line, without grouping all the
   actions for the same id into one line.
   Note:  the actions MUST NOT contain any reteloc's.
------------------------------------------------------------------ */

Symbol *action_id_to_match;

bool pick_actions_with_matching_id(dl_cons * dc)
{
    action *a;
    a = dc->item;
    if (a->type != MAKE_ACTION)
        return FALSE;
    return (bool) (rhs_value_to_symbol(a->id) == action_id_to_match);
}

#define PRINT_ACTION_LIST_TEMP_SIZE 10000
void print_action_list(action * actions, int indent, bool internal)
{
    bool did_one_line_already;
    dl_list *actions_not_yet_printed, *tail_of_actions_not_yet_printed;
    dl_list *actions_for_this_id;
    dl_cons *dc;
    action *a;

    if (!actions)
        return;

    did_one_line_already = FALSE;

    /* --- build dl_list of all the actions --- */
    actions_not_yet_printed = NIL;
    tail_of_actions_not_yet_printed = NIL;
    for (a = actions; a != NIL; a = a->next) {
        allocate_with_pool(&current_agent(dl_cons_pool), &dc);
        dc->item = a;
        if (actions_not_yet_printed)
            tail_of_actions_not_yet_printed->next = dc;
        else
            actions_not_yet_printed = dc;
        dc->prev = tail_of_actions_not_yet_printed;
        tail_of_actions_not_yet_printed = dc;
    }
    tail_of_actions_not_yet_printed->next = NIL;

    /* --- main loop: find all actions for first id, print them together --- */
    while (actions_not_yet_printed) {
        if (did_one_line_already) {
            print("\n");
            print_spaces(indent);
        } else {
            did_one_line_already = TRUE;
        }
        dc = actions_not_yet_printed;
        remove_from_dll(actions_not_yet_printed, dc, next, prev);
        a = dc->item;
        if (a->type == FUNCALL_ACTION) {
            free_with_pool(&current_agent(dl_cons_pool), dc);
            print_string(rhs_value_to_string(a->value, NULL, 0));
            continue;
        }

        /* --- normal make actions --- */
        /* --- collect all actions whose id matches the first action's id --- */
        actions_for_this_id = dc;
        action_id_to_match = rhs_value_to_symbol(a->id);
        dc->prev = NIL;
        if (internal) {
            dc->next = NIL;
        } else {
            dc->next = extract_dl_list_elements(&actions_not_yet_printed, pick_actions_with_matching_id);
        }

        /* --- print the collected actions all together --- */
        print_with_symbols("(%y", action_id_to_match);
        while (actions_for_this_id) {
            dc = actions_for_this_id;
            actions_for_this_id = actions_for_this_id->next;
            a = dc->item;
            free_with_pool(&current_agent(dl_cons_pool), dc);

            {                   /* --- build and print attr/value test for action a --- */
                char temp[PRINT_ACTION_LIST_TEMP_SIZE], *ch;

                ch = temp;
                strncpy(ch, " ^", PRINT_ACTION_LIST_TEMP_SIZE - (ch - temp));
                ch[PRINT_ACTION_LIST_TEMP_SIZE - (ch - temp) - 1] = 0;
                while (*ch)
                    ch++;
                rhs_value_to_string(a->attr, ch, PRINT_ACTION_LIST_TEMP_SIZE - (ch - temp));
                while (*ch)
                    ch++;
                *(ch++) = ' ';
                rhs_value_to_string(a->value, ch, PRINT_ACTION_LIST_TEMP_SIZE - (ch - temp));
                while (*ch)
                    ch++;
                *(ch++) = ' ';
                *(ch++) = preference_type_indicator(a->preference_type);
                if (preference_is_binary(a->preference_type)) {
                    *(ch++) = ' ';
                    rhs_value_to_string(a->referent, ch, PRINT_ACTION_LIST_TEMP_SIZE - (ch - temp));
                    while (*ch)
                        ch++;
                }
                *ch = 0;
                if (current_agent(printer_output_column) + (ch - temp) >= COLUMNS_PER_LINE) {
                    print_string("\n");
                    print_spaces(indent + 6);
                }
                print_string(temp);
            }
        }
        print_string(")");
    }                           /* end of while (actions_not_yet_printed) */
}

/* ------------------------------------------------------------------
                         Print Production

   This prints a production.  The "internal" parameter, if TRUE,
   indicates that the LHS and RHS should be printed in internal format.
------------------------------------------------------------------ */

void print_production(production * p, bool internal)
{
    condition *top, *bottom;
    action *rhs;

    /* --- print "sp" and production name --- */
    print_with_symbols("sp {%y\n", p->name);
    /* --- print optional documention string --- */
    if (p->documentation) {
        char temp[MAX_LEXEME_LENGTH * 2 + 10];
        string_to_escaped_string(p->documentation, '"', temp);
        print("    %s\n", temp);
    }
    /* --- print any flags --- */
    switch (p->type) {
    case DEFAULT_PRODUCTION_TYPE:
        print_string("    :default\n");
        break;
    case USER_PRODUCTION_TYPE:
        break;
    case CHUNK_PRODUCTION_TYPE:
        print_string("    :chunk\n");
        break;
    case JUSTIFICATION_PRODUCTION_TYPE:
        print_string("    :justification ;# not reloadable\n");
        break;
    }
    if (p->declared_support == DECLARED_O_SUPPORT)
        print_string("    :o-support\n");
    else if (p->declared_support == DECLARED_I_SUPPORT)
        print_string("    :i-support\n");
#ifdef MATCHTIME_INTERRUPT
    if (p->interrupt)
        print_string("    :interrupt\n");
#endif

    /* --- print the LHS and RHS --- */
    p_node_to_conditions_and_nots(p->p_node, NIL, NIL, &top, &bottom, NIL, &rhs);
    print_string("   ");
    print_condition_list(top, 3, internal);
    deallocate_condition_list(top);
    print_string("\n    -->\n  ");
    print_string("  ");
    print_action_list(rhs, 4, internal);
    print_string("\n}\n");
    deallocate_action_list(rhs);
}

/* ------------------------------------------------------------------
                       Other Printing Utilities

   Print_condition() prints a single condition.  Print_action() prints
   a single action (which MUST NOT contain any reteloc's).
   Note that these routines work by calling print_condition_list() and
   print_action_list(), respectively, so they print a linefeed if the
   output would go past COLUMNS_PER_LINE.

   Preference_type_indicator() returns a character corresponding to
   a given preference type (byte)--for example, given BEST_PREFERENCE_TYPE,
   it returns '>'.

   Print_preference() prints a given preference.  Print_wme() prints a
   wme (including the timetag).  Print_instantiation_with_wmes() prints
   an instantiation's production name and the wmes it matched, using a
   given wme_trace_type (e.g., TIMETAG_WME_TRACE).
------------------------------------------------------------------ */

void print_condition(condition * cond)
{
    condition *old_next, *old_prev;

    old_next = cond->next;
    old_prev = cond->prev;
    cond->next = NIL;
    cond->prev = NIL;
    print_condition_list(cond, 0, TRUE);
    cond->next = old_next;
    cond->prev = old_prev;
}

void print_action(action * a)
{
    action *old_next;

    old_next = a->next;
    a->next = NIL;
    print_action_list(a, 0, TRUE);
    a->next = old_next;
}

char preference_type_indicator(byte type)
{
    switch (type) {
    case ACCEPTABLE_PREFERENCE_TYPE:
        return '+';
    case REQUIRE_PREFERENCE_TYPE:
        return '!';
    case REJECT_PREFERENCE_TYPE:
        return '-';
    case PROHIBIT_PREFERENCE_TYPE:
        return '~';
    case RECONSIDER_PREFERENCE_TYPE:
        return '@';
    case UNARY_INDIFFERENT_PREFERENCE_TYPE:
        return '=';
    case BINARY_INDIFFERENT_PREFERENCE_TYPE:
        return '=';
    case UNARY_PARALLEL_PREFERENCE_TYPE:
        return '&';
    case BINARY_PARALLEL_PREFERENCE_TYPE:
        return '&';
    case BEST_PREFERENCE_TYPE:
        return '>';
    case BETTER_PREFERENCE_TYPE:
        return '>';
    case WORST_PREFERENCE_TYPE:
        return '<';
    case WORSE_PREFERENCE_TYPE:
        return '<';
    default:
        {
            char msg[MESSAGE_SIZE];
            strncpy(msg, "print.c: Error: bad type passed to preference_type_indicator\n", MESSAGE_SIZE);
            msg[MESSAGE_SIZE - 1] = 0;
            abort_with_fatal_error(msg);
        }
    }
    return 0;                   /* unreachable, but without it, gcc -Wall warns here */
}

void print_preference(preference * pref)
{
    print_with_symbols("(%y ^%y %y ", pref->id, pref->attr, pref->value);
    print("%c", preference_type_indicator(pref->type));
    if (preference_is_binary(pref->type)) {
        print_with_symbols(" %y", pref->referent);
    }
    if (pref->o_supported)
        print_string("  :O ");
    print_string(")");
    print("\n");
}

void detailed_print_preference(preference * pref)
{
    int i, end;
    char space[48];

    print_with_symbols("(%y ^%y %y ", pref->id, pref->attr, pref->value);
    print("%c", preference_type_indicator(pref->type));
    if (preference_is_binary(pref->type)) {
        print_with_symbols(" %y", pref->referent);
    }
    if (pref->o_supported)
        print_string("  :O ");

    end = 37 - get_printer_output_column();
    for (i = 0; i < end; i++) {
        space[i] = ' ';

    }
    space[i] = '\0';

    print(")%s", space);
    print(" (refs, in-tm, addr) (%lu, %d, %p)", pref->reference_count, pref->in_tm, pref);
    print("\n");
}

void watchful_print_preference(preference * pref)
{
    int i, end;
    char space[48];

    print_with_symbols("(%y ^%y %y ", pref->id, pref->attr, pref->value);
    print("%c", preference_type_indicator(pref->type));
    if (preference_is_binary(pref->type)) {
        print_with_symbols(" %y", pref->referent);
    }
    if (pref->o_supported)
        print_string("  :O ");

    end = 37 - get_printer_output_column();
    for (i = 0; i < end; i++) {
        space[i] = ' ';

    }
    space[i] = '\0';

    print(")%s", space);
    print(" (id->id.level) (%d)", pref->id->id.level);
    print("\n");
}

/* kjh(CUSP-B2) begin */
extern bool passes_wme_filtering(wme * w, bool isAdd);
void filtered_print_wme_add(wme * w)
{
    if (passes_wme_filtering(w, TRUE)) {
        print("=>WM: ");
        print_wme(w);
    }
}
void filtered_print_wme_remove(wme * w)
{
    if (passes_wme_filtering(w, FALSE)) {
        print("<=WM: ");
        print_wme(w);
    }
}

/* kjh(CUSP-B2) end */

void print_wme(wme * w)
{
    print("(%lu: ", w->timetag);
    print_with_symbols("%y ^%y %y", w->id, w->attr, w->value);
    if (w->acceptable)
        print_string(" +");
    print(")\n");
}

void detailed_print_wme(wme * w)
{
    int end, i;
    char space[48];

    print("(%lu: ", w->timetag);
    print_with_symbols("%y ^%y %y", w->id, w->attr, w->value);
    if (w->acceptable)
        print_string(" +");

    end = 37 - get_printer_output_column();
    for (i = 0; i < end; i++) {
        space[i] = ' ';

    }
    space[i] = '\0';

    print(")%s", space);

    print("(refs, addr) (%lu, %p)", w->reference_count, w);
    print("\n");

}

void print_instantiation_with_wmes(instantiation * inst, wme_trace_type wtt)
{
    condition *cond;

    if (inst->prod)
        print_with_symbols("%y", inst->prod->name);
    else
        print("[dummy production]");
    print("\n");

    if (wtt == NONE_WME_TRACE)
        return;

    for (cond = inst->top_of_instantiated_conditions; cond != NIL; cond = cond->next)
        if (cond->type == POSITIVE_CONDITION) {
            switch (wtt) {
            case TIMETAG_WME_TRACE:
                print(" %lu", cond->bt.wme->timetag);
                break;
            case FULL_WME_TRACE:
                print(" ");
                print_wme(cond->bt.wme);
                break;
            }
        }
}

/***************************************************************************
* Function     : print_list_of_conditions
**************************************************************************/

void print_list_of_conditions(condition * cond)
{

    while (cond != NULL) {
        if (get_printer_output_column() >= COLUMNS_PER_LINE - 20)
            print("\n      ");
        print_condition(cond);
        print("\n");

        cond = cond->next;
    }
}
