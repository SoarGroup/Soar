/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/* ======================================================================
                              parser.h

                     The Production (SP) Parser 

   Init_parser() should be called at startup time.  Parse_production()
   reads an SP (starting from the production name), builds a production,
   adds it to the rete, and returns a pointer to the new production
   (or NIL if any error occurred).  Parse_lhs() reads just an LHS,
   and returns a condition list (or NIL if an error occurred).  
   Parse_rhs() reads an RHS, setting "dest_rhs" to point to the resulting
   action list; it returns TRUE if successful, FALSE if any error occurred.
====================================================================== */

#ifndef PARSER_H
#define PARSER_H

#ifdef __cplusplus
extern "C"
{
#endif
   typedef char Bool;
   typedef char * test;
   typedef struct condition_struct condition;
   typedef struct action_struct action;
   typedef struct agent_struct agent;
   typedef struct kernel_struct Kernel;
   typedef union symbol_union Symbol;

   extern void init_parser (void);
   extern condition *parse_lhs (agent* thisAgent);
   extern Bool parse_rhs (agent* thisAgent, action **dest_rhs);
   extern struct production_struct *parse_production (agent* thisAgent, byte* rete_addition_result);
   extern Symbol *make_symbol_for_current_lexeme (agent* thisAgent);

#ifdef __cplusplus
}
#endif

#endif
