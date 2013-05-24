/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/*************************************************************************
 *
 *  file:  rhs.cpp
 *
 * =======================================================================
 *                    RHS Utilities
 * This file contains various utility routines for rhs values and actions.
 *
 * =======================================================================
 */

#include "rhs.h"
#include "production.h"
#include "kernel.h"

/* =================================================================

              Utility Routines for Actions and RHS Values

================================================================= */

/* Warning: symbol_to_rhs_value() doesn't symbol_add_ref.  The caller must
   do the reference count update */
// Debug | May not need these b/c rhs_to_symbol did not increase refcount, but make_rhs_value_symbol does

inline rhs_value make_rhs_value_symbol_no_refcount(agent* thisAgent, Symbol * sym, Symbol * original_sym)
{
  rhs_symbol new_rhs_symbol;

  if (!sym )
  {
    print(thisAgent, "Debug | make_rhs_value_symbol_no_refcount called with nil.\n");
    return reinterpret_cast<rhs_value>(NIL);
  }
  allocate_with_pool (thisAgent, &thisAgent->rhs_symbol_pool, &new_rhs_symbol);
  new_rhs_symbol->referent = sym;
  new_rhs_symbol->original_variable = original_sym;
  print(thisAgent, "Debug | make_rhs_value_symbol_no_refcount creating rhs_symbol %s (%s).\n",
         symbol_to_string(thisAgent, new_rhs_symbol->referent, FALSE, NULL, 0),
         (new_rhs_symbol->original_variable ? symbol_to_string(thisAgent, new_rhs_symbol->original_variable, FALSE, NULL, 0) : "no orig"));

  /* -- Must always increase original_sym refcount if it exists because this function
   *    is only called when the newly generate rhs value is created with a brand new
   *    sym that already had its refcount incremented -- */

  if (original_sym)
  {
    symbol_add_ref(thisAgent, original_sym);
    print(thisAgent, "Debug | make_rhs_value_symbol_no_refcount adding refcount to %s.\n",
           symbol_to_string(thisAgent, original_sym, FALSE, NULL, 0));
  }
  return rhs_symbol_to_rhs_value(new_rhs_symbol);
}

inline rhs_value make_rhs_value_symbol(agent* thisAgent, Symbol * sym, Symbol * original_sym)
{
  if (sym)
  {
    symbol_add_ref(thisAgent, sym);
    print(thisAgent, "Debug | make_rhs_value_symbol adding refcount to %s.\n",
           symbol_to_string(thisAgent, sym, FALSE, NULL, 0));
  }
  return make_rhs_value_symbol_no_refcount(thisAgent, sym, original_sym);
}

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
    rhs_symbol r = rhs_value_to_rhs_symbol(rv);
    if (r->referent)
    {
      print(thisAgent, "Debug | deallocate_rhs_value decreasing refcount of %s from %ld to %ld.\n",
             symbol_to_string(thisAgent, r->referent, FALSE, NULL, 0),
             r->referent->common.reference_count, (r->referent->common.reference_count)-1);
      symbol_remove_ref (thisAgent, r->referent);
    }
    if (r->original_variable)
    {
      print(thisAgent, "Debug | deallocate_rhs_value decreasing refcount of original %s from %ld to %ld.\n",
             symbol_to_string(thisAgent, r->original_variable, FALSE, NULL, 0),
             r->original_variable->common.reference_count, (r->original_variable->common.reference_count)-1);
      symbol_remove_ref (thisAgent, r->original_variable);
    }
    free_with_pool (&thisAgent->rhs_symbol_pool, r);
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
    rhs_symbol r = rhs_value_to_rhs_symbol(rv);
    return make_rhs_value_symbol(thisAgent, r->referent, r->original_variable);
  }
}


/* ----------------------------------------------------------------
   Deallocates the given action (singly-linked) list.
---------------------------------------------------------------- */

void deallocate_action_list (agent* thisAgent, action *actions) {
  action *a;

  print(thisAgent, "Debug | deallocating action list...\n");
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
