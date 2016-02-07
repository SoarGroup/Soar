/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/* ======================================================================
                                trace.h

   Object and stack trace formats are managed by this module.

   Init_tracing() initializes the tables; at this point, there are no trace
   formats for anything.  This routine should be called at startup time.

   Trace formats are changed by calls to add_trace_format() and
   remove_trace_format().  Add_trace_format() returns true if the
   format was successfully added, or false if the format string didn't
   parse right.  Remove_trace_format() returns true if a trace format
   was actually removed, or false if there was no such trace format for
   the given type/name restrictions.  These routines take a "stack_trace"
   argument, which should be true if the stack trace format is intended,
   or false if the object trace format is intended.  Their
   "type_restriction" argument should be one of FOR_ANYTHING_TF, ...,
   FOR_OPERATORS_TF.  The "name_restriction" argument should be either
   a pointer to a symbol, if the trace format is  restricted to apply
   to objects with that name, or NIL if the format can apply to any object.

   Print_all_trace_formats() prints out either all existing stack trace
   or object trace formats.

   Print_object_trace() takes an object (any symbol).  It prints the
   trace for that object.  Print_stack_trace() takes a (context)
   object (the state or op), the current state, the "slot_type"
   (one of FOR_OPERATORS_TF, etc.), and a flag indicating whether to
   allow %dc and %ec escapes (this flag should normally be true for
   watch 0 traces but false during a "print -stack" command).  It prints
   the stack trace for that context object.
====================================================================== */

#ifndef TRACE_H
#define TRACE_H

#include "kernel.h"

/* trace format type restrictions */
#define FOR_ANYTHING_TF 0          /* format applies to any object */
#define FOR_STATES_TF 1            /* format applies only to states */
#define FOR_OPERATORS_TF 2         /* format applies only to operators */

extern void init_tracing(agent* thisAgent);
extern bool add_trace_format(agent* thisAgent, bool stack_trace, int type_restriction,
                             Symbol* name_restriction, const char* format_string);
extern bool remove_trace_format(agent* thisAgent, bool stack_trace, int type_restriction,
                                Symbol* name_restriction);
extern void print_all_trace_formats(agent* thisAgent, bool stack_trace);
extern void print_all_trace_formats_tcl(bool stack_trace);
extern void print_object_trace(agent* thisAgent, Symbol* object);
extern void print_stack_trace(agent* thisAgent, Symbol* object, Symbol* state, int slot_type,
                              bool allow_cycle_counts);

extern char* help_on_trace_format_escapes[];

#endif
