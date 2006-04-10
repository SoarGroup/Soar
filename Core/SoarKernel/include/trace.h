/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/* ======================================================================
                                trace.h

   Object and stack trace formats are managed by this module.

   Init_tracing() initializes the tables; at this point, there are no trace
   formats for anything.  This routine should be called at startup time.

   Trace formats are changed by calls to add_trace_format() and
   remove_trace_format().  Add_trace_format() returns TRUE if the
   format was successfully added, or FALSE if the format string didn't
   parse right.  Remove_trace_format() returns TRUE if a trace format
   was actually removed, or FALSE if there was no such trace format for
   the given type/name restrictions.  These routines take a "stack_trace"
   argument, which should be TRUE if the stack trace format is intended,
   or FALSE if the object trace format is intended.  Their
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
   allow %dc and %ec escapes (this flag should normally be TRUE for
   watch 0 traces but FALSE during a "print -stack" command).  It prints
   the stack trace for that context object.
====================================================================== */

#ifndef TRACE_H
#define TRACE_H

#ifdef __cplusplus
extern "C"
{
#endif

/* trace format type restrictions */
#define FOR_ANYTHING_TF 0          /* format applies to any object */
#define FOR_STATES_TF 1            /* format applies only to states */
#define FOR_OPERATORS_TF 2         /* format applies only to operators */

typedef char Bool;
typedef struct agent_struct agent;
typedef union symbol_union Symbol;

extern void init_tracing (agent* thisAgent);
extern Bool add_trace_format (agent* thisAgent, Bool stack_trace, int type_restriction,
                              Symbol *name_restriction, char *format_string);
extern Bool remove_trace_format (agent* thisAgent, Bool stack_trace, int type_restriction,
                                 Symbol *name_restriction);
extern void print_all_trace_formats (agent* thisAgent, Bool stack_trace);
//#ifdef USE_TCL
extern void print_all_trace_formats_tcl (Bool stack_trace);
//#endif /* USE_TCL */
extern void print_object_trace (agent* thisAgent, Symbol *object);
extern void print_stack_trace (agent* thisAgent, Symbol *object, Symbol *state, int slot_type,
                               Bool allow_cycle_counts);

extern char * help_on_trace_format_escapes[];

#ifdef __cplusplus
}
#endif

#endif

