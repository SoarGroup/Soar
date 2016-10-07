/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/* ====================================================================
                            rhs_functions.h

   The system maintains a list of available RHS functions.  Functions
   can appear on the RHS of productions either as values (in make actions
   or as arguments to other function calls) or as stand-alone actions
   (e.g., "write" and "halt").  When a function is executed, its C code
   is called with one parameter--a (consed) list of the arguments (symbols).
   The C function should return either a symbol (if all goes well) or NIL
   (if an error occurred, or if the function is a stand-alone action).

   All available RHS functions should be setup at system startup time via
   calls to add_rhs_function().  It takes as arguments the name of the
   function (a symbol), a pointer to the corresponding C function, the
   number of arguments the function expects (-1 if the function can take
   any number of arguments), and flags indicating whether the function can
   be a RHS value or a stand-alone action.

   Lookup_rhs_function() takes a symbol and returns the corresponding
   rhs_function structure (or NIL if there is no such function).

   Init_built_in_rhs_functions() should be called at system startup time
   to setup all the built-in functions.
==================================================================== */

#ifndef RHS_FUNCTIONS_H
#define RHS_FUNCTIONS_H

#include "kernel.h"
#include "Export.h"

typedef Symbol* ((*rhs_function_routine)(agent* thisAgent, cons* args, void* user_data));

typedef struct rhs_function_struct
{
    struct rhs_function_struct* next;
    Symbol* name;
    rhs_function_routine f;
    int num_args_expected;     /* -1 means it can take any number of args */
//    agent* thisAgent;
//    char* cached_print_str;
    bool can_be_rhs_value;
    bool can_be_stand_alone_action;
    void* user_data;           /* Pointer to anything the user may want to pass into the function */
} rhs_function;

extern EXPORT void add_rhs_function(agent* thisAgent,
                             Symbol* name,
                             rhs_function_routine f,
                             int num_args_expected,
                             bool can_be_rhs_value,
                             bool can_be_stand_alone_action,
                             void* user_data);
extern EXPORT void remove_rhs_function(agent* thisAgent, Symbol* name);
extern EXPORT rhs_function* lookup_rhs_function(agent* thisAgent, Symbol* name);
extern EXPORT void init_built_in_rhs_functions(agent* thisAgent);
extern EXPORT void remove_built_in_rhs_functions(agent* thisAgent);

extern void init_built_in_rhs_math_functions(agent* thisAgent);
extern void remove_built_in_rhs_math_functions(agent* thisAgent);

#endif
