/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/* ======================================================================
                              
								print.h                               

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
====================================================================== */

#ifndef PRINT_H
#define PRINT_H

#ifdef __cplusplus
extern "C"
{
#endif

typedef char Bool;
typedef char * test;
typedef char * rhs_value;
typedef unsigned char byte;
typedef byte wme_trace_type;
typedef struct wme_struct wme;
typedef struct agent_struct agent;
typedef struct action_struct action;
typedef struct production_struct production;
typedef struct preference_struct preference;
typedef struct condition_struct condition;
typedef struct instantiation_struct instantiation;
typedef union symbol_union Symbol;

extern void start_log_file (agent* thisAgent, char *filename, Bool append);
extern void stop_log_file (agent* thisAgent);
extern void print_string_to_log_file_only (agent* thisAgent, char *string);

extern int get_printer_output_column (agent* thisAgent);
extern void tell_printer_that_output_column_has_been_reset (agent* thisAgent);

extern void start_redirection_to_file (agent* thisAgent, FILE *already_opened_file);
extern void stop_redirection_to_file (agent* thisAgent);

extern void print_string (agent* thisAgent, char *s);
extern void print_phase  (agent* thisAgent, char *s, bool end_phase);

#ifdef USE_STDARGS
extern void print (agent* thisAgent, char *format, ... );
extern void print_with_symbols (agent* thisAgent, char *format, ...);
#else
extern void print (); /* Can't have two functions with the same name
                         under C linkage */
extern void print_with_symbols (agent* thisAgent);
#endif
extern void print_spaces (agent* thisAgent, int n);

extern void Soar_LogAndPrint (agent* thisAgent, agent * the_agent, char * str);
extern void Soar_Print (agent* thisAgent, agent * the_agent, char * str);
extern void Soar_Log (agent* thisAgent, agent * the_agent, char * str);
extern void filtered_print_wme_remove(agent* thisAgent, wme *w);
extern void filtered_print_wme_add(agent* thisAgent, wme *w);



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

extern char *string_to_escaped_string (agent* thisAgent, char *s, char first_and_last_char,
                                       char *dest);
extern char const* symbol_to_typeString (agent* thisAgent, Symbol *sym);
extern char *symbol_to_string (agent* thisAgent, Symbol *sym, Bool rereadable, char *dest, size_t dest_size);
extern char *test_to_string (agent* thisAgent, test t, char *dest, size_t dest_size);
extern char *rhs_value_to_string (agent* thisAgent, rhs_value rv, char *dest, size_t dest_size);

/* -----------------------------------------------------------------------
             Print Condition List, Action List, Production

   Print_condition_list() prints a list of conditions.  The "indent"
   parameter tells how many spaces to indent each line other than the
   first--the first line is not indented (the caller must handle this).
   The last line is printed without a trailing linefeed.  The "internal"
   parameter, if TRUE, indicates that the condition list should be printed
   in internal format--one condition per line, without grouping all the
   conditions for the same id into one line.

   Print_action_list() is similar except it prints actions instead of
   conditions.  The actions MUST NOT contain any reteloc's.

   Print_production() prints a given production, optionally using internal
   format.
----------------------------------------------------------------------- */

extern void print_condition_list (agent* thisAgent, condition *conds, int indent, Bool internal);
extern void print_action_list (agent* thisAgent, action *actions, int indent, Bool internal);
extern void print_production (agent* thisAgent, production *p, Bool internal);

/* -----------------------------------------------------------------------
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
   given wme_trace_type (e.g., TIMETAG_WME_TRACE). Action is printing, 
   firing or retracting -- added March 05 KJC.
----------------------------------------------------------------------- */

extern void print_condition (agent* thisAgent, condition *cond);
extern void print_action (agent* thisAgent, action *a);
extern char preference_type_indicator (agent* thisAgent, byte type);
extern void print_preference (agent* thisAgent, preference *pref);
extern void print_wme (agent* thisAgent, wme *w);
extern void print_wme_for_tcl (wme *w);
extern void print_instantiation_with_wmes (agent* thisAgent, 
										   instantiation *inst,
                                           wme_trace_type wtt,
										   int action);

extern void print_list_of_conditions(agent* thisAgent, condition *cond); 

#ifdef __cplusplus
}
#endif

#endif
