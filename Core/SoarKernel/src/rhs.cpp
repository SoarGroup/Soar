#include <portability.h>

/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/*************************************************************************
 *
 *  file:  rhsfun.cpp
 *
 * =======================================================================
 *                   RHS Function Management for Soar 6
 *
 * The system maintains a list of available RHS functions.  Functions
 * can appear on the RHS of productions either as values (in make actions
 * or as arguments to other function calls) or as stand-alone actions
 * (e.g., "write" and "halt").  When a function is executed, its C code
 * is called with one parameter--a (consed) list of the arguments (symbols).
 * The C function should return either a symbol (if all goes well) or NIL
 * (if an error occurred, or if the function is a stand-alone action).
 *
 * All available RHS functions should be setup at system startup time via
 * calls to add_rhs_function().  It takes as arguments the name of the
 * function (a symbol), a pointer to the corresponding C function, the
 * number of arguments the function expects (-1 if the function can take
 * any number of arguments), and flags indicating whether the function can
 * be a RHS value or a stand-alone action.
 *
 * Lookup_rhs_function() takes a symbol and returns the corresponding
 * rhs_function structure (or NIL if there is no such function).
 *
 * Init_built_in_rhs_functions() should be called at system startup time
 * to setup all the built-in functions.
 * =======================================================================
 */

#include <stdlib.h>

#include "kernel.h"
#include "print.h"
#include "mem.h"
#include "symtab.h"
#include "init_soar.h"
#include "gsysparam.h"
#include "agent.h"
#include "production.h"
#include "rhs.h"
#include "rhs_functions.h"
#include "rhs_functions_math.h"
#include "io_soar.h"
#include "recmem.h"
#include "wmem.h"
#include "test.h"
#include "decide.h"

#include "xml.h"
#include "soar_TraceNames.h"

#include <map>
#include <string>
#include <time.h>

using namespace soar_TraceNames;

/* =================================================================

              Utility Routines for Actions and RHS Values

================================================================= */

/* ----------------------------------------------------------------
   Deallocates the given rhs_value.
---------------------------------------------------------------- */

void deallocate_rhs_value (agent* thisAgent, rhs_value rv) {
  cons *c;
  list *fl;

  if (rhs_value_is_reteloc(rv)) return;
  if (rhs_value_is_unboundvar(rv)) return;
  if (rhs_value_is_funcall(rv)) {
    fl = rhs_value_to_funcall_list(rv);
    for (c=fl->rest; c!=NIL; c=c->rest)
      deallocate_rhs_value (thisAgent, static_cast<char *>(c->first));
    free_list (thisAgent, fl);
  } else {
    symbol_remove_ref (thisAgent, rhs_value_to_symbol(rv));
  }
}

/* ----------------------------------------------------------------
   Returns a new copy of the given rhs_value.
---------------------------------------------------------------- */

rhs_value copy_rhs_value (agent* thisAgent, rhs_value rv) {
  cons *c, *new_c, *prev_new_c;
  list *fl, *new_fl;

  if (rhs_value_is_reteloc(rv)) return rv;
  if (rhs_value_is_unboundvar(rv)) return rv;
  if (rhs_value_is_funcall(rv)) {
    fl = rhs_value_to_funcall_list(rv);
    allocate_cons (thisAgent, &new_fl);
    new_fl->first = fl->first;
    prev_new_c = new_fl;
    for (c=fl->rest; c!=NIL; c=c->rest) {
      allocate_cons (thisAgent, &new_c);
      new_c->first = copy_rhs_value (thisAgent, static_cast<char *>(c->first));
      prev_new_c->rest = new_c;
      prev_new_c = new_c;
    }
    prev_new_c->rest = NIL;
    return funcall_list_to_rhs_value (new_fl);
  } else {
    symbol_add_ref (thisAgent, rhs_value_to_symbol(rv));
    return rv;
  }
}

/* ----------------------------------------------------------------
   Deallocates the given action (singly-linked) list.
---------------------------------------------------------------- */

void deallocate_action_list (agent* thisAgent, action *actions) {
  action *a;

  while (actions) {
    a = actions;
    actions = actions->next;
    if (a->type==FUNCALL_ACTION) {
      deallocate_rhs_value (thisAgent, a->value);
    } else {
      /* --- make actions --- */
      deallocate_rhs_value (thisAgent, a->id);
      deallocate_rhs_value (thisAgent, a->attr);
      deallocate_rhs_value (thisAgent, a->value);
      if (preference_is_binary(a->preference_type))
        deallocate_rhs_value (thisAgent, a->referent);
    }
    free_with_pool (&thisAgent->action_pool,a);
  }
}

/* -----------------------------------------------------------------
   Find first letter of rhs_value, or '*' if nothing appropriate.
   (See comments on first_letter_from_symbol for more explanation.)
----------------------------------------------------------------- */

char first_letter_from_rhs_value (rhs_value rv) {
  if (rhs_value_is_symbol(rv))
    return first_letter_from_symbol (rhs_value_to_symbol(rv));
  return '*'; /* function calls, reteloc's, unbound variables */
}

