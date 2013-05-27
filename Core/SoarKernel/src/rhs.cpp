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
#include "gdatastructs.h"

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
#ifdef DEBUG_TRACE_RHS_REFCOUNTS
    print(thisAgent, "Debug | make_rhs_value_symbol_no_refcount called with nil.\n");
#endif
    return reinterpret_cast<rhs_value>(NIL);
  }
  allocate_with_pool (thisAgent, &thisAgent->rhs_symbol_pool, &new_rhs_symbol);
  new_rhs_symbol->referent = sym;
  new_rhs_symbol->original_variable = original_sym;
#ifdef DEBUG_TRACE_RHS_REFCOUNTS
  print(thisAgent, "Debug | make_rhs_value_symbol_no_refcount creating rhs_symbol %s (%s).\n",
         symbol_to_string(thisAgent, new_rhs_symbol->referent, FALSE, NULL, 0),
         (new_rhs_symbol->original_variable ? symbol_to_string(thisAgent, new_rhs_symbol->original_variable, FALSE, NULL, 0) : "no orig"));
#endif
  /* -- Must always increase original_sym refcount if it exists because this function
   *    is only called when the newly generate rhs value is created with a brand new
   *    sym that already had its refcount incremented -- */

  if (original_sym)
  {
    symbol_add_ref(thisAgent, original_sym);
#ifdef DEBUG_TRACE_RHS_REFCOUNTS
    print(thisAgent, "Debug | make_rhs_value_symbol_no_refcount adding refcount to %s.\n",
           symbol_to_string(thisAgent, original_sym, FALSE, NULL, 0));
#endif
  }
  return rhs_symbol_to_rhs_value(new_rhs_symbol);
}

/* Debug| symbol_to_rhs_value() (what this function used to be) didn't symbol_add_ref. The
 * caller had to do the reference count update.  Possible bug source. */
inline rhs_value make_rhs_value_symbol(agent* thisAgent, Symbol * sym, Symbol * original_sym)
{
  if (sym)
  {
    symbol_add_ref(thisAgent, sym);
#ifdef DEBUG_TRACE_RHS_REFCOUNTS
    print(thisAgent, "Debug | make_rhs_value_symbol adding refcount to %s.\n",
           symbol_to_string(thisAgent, sym, FALSE, NULL, 0));
#endif
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
#ifdef DEBUG_TRACE_RHS_REFCOUNTS
      print(thisAgent, "Debug | deallocate_rhs_value decreasing refcount of %s from %ld to %ld.\n",
             symbol_to_string(thisAgent, r->referent, FALSE, NULL, 0),
             r->referent->common.data.reference_count, (r->referent->common.data.reference_count)-1);
#endif
      symbol_remove_ref (thisAgent, r->referent);
    }
    if (r->original_variable)
    {
#ifdef DEBUG_TRACE_RHS_REFCOUNTS
      print(thisAgent, "Debug | deallocate_rhs_value decreasing refcount of original %s from %ld to %ld.\n",
             symbol_to_string(thisAgent, r->original_variable, FALSE, NULL, 0),
             r->original_variable->common.data.reference_count, (r->original_variable->common.data.reference_count)-1);
#endif
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
#ifdef DEBUG_TRACE_RHS_UNIQUE_VARIABLIZATION
    print(thisAgent, "RHS UV| copy_rhs_value copying rhs_symbol %s(%s).\n",
          symbol_to_string(thisAgent, r->referent),
         (r->original_variable ? symbol_to_string(thisAgent, r->original_variable) : "NIL"));
#endif
    return make_rhs_value_symbol(thisAgent, r->referent, r->original_variable);
  }
}


/* ----------------------------------------------------------------
   Deallocates the given action (singly-linked) list.
---------------------------------------------------------------- */

void deallocate_action_list (agent* thisAgent, action *actions) {
  action *a;

#ifdef DEBUG_TRACE_RHS_REFCOUNTS
  print(thisAgent, "Debug | deallocating action list...\n");
#endif
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

/* =====================================================================

   Finding all variables from rhs_value's, actions, and action lists

   These routines collect all the variables in rhs_value's, etc.  Their
   "var_list" arguments should either be NIL or else should point to
   the header of the list of marked variables being constructed.

   Warning: These are part of the reorderer and handle only productions
   in non-reteloc, etc. format.  They don't handle reteloc's or
   RHS unbound variables.
===================================================================== */

void add_all_variables_in_rhs_value (agent* thisAgent,
                   rhs_value rv, tc_number tc,
                                     list **var_list) {
  list *fl;
  cons *c;
  Symbol *sym;

  if (rhs_value_is_symbol(rv)) {
    /* --- ordinary values (i.e., symbols) --- */
    sym = rhs_value_to_symbol(rv);
    if (sym->common.data.symbol_type==VARIABLE_SYMBOL_TYPE)
      mark_variable_if_unmarked (thisAgent, sym, tc, var_list);
  } else {
    /* --- function calls --- */
    fl = rhs_value_to_funcall_list(rv);
    for (c=fl->rest; c!=NIL; c=c->rest)
      add_all_variables_in_rhs_value (thisAgent, static_cast<char *>(c->first), tc, var_list);
  }
}

void add_all_variables_in_action (agent* thisAgent, action *a,
                  tc_number tc, list **var_list){
  Symbol *id;

  if (a->type==MAKE_ACTION) {
    /* --- ordinary make actions --- */
    id = rhs_value_to_symbol(a->id);
    if (id->common.data.symbol_type==VARIABLE_SYMBOL_TYPE)
      mark_variable_if_unmarked (thisAgent, id, tc, var_list);
    add_all_variables_in_rhs_value (thisAgent, a->attr, tc, var_list);
    add_all_variables_in_rhs_value (thisAgent, a->value, tc, var_list);
    if (preference_is_binary(a->preference_type))
      add_all_variables_in_rhs_value (thisAgent, a->referent, tc, var_list);
  } else {
    /* --- function call actions --- */
    add_all_variables_in_rhs_value (thisAgent, a->value, tc, var_list);
  }
}

void add_all_variables_in_action_list (agent* thisAgent, action *actions, tc_number tc,
                                       list **var_list) {
  action *a;

  for (a=actions; a!=NIL; a=a->next)
    add_all_variables_in_action (thisAgent, a, tc, var_list);
}

/* =====================================================================

   Finding the variables bound in tests, conditions, and condition lists

   These routines collect the variables that are bound in equality tests.
   Their "var_list" arguments should either be NIL or else should point
   to the header of the list of marked variables being constructed.

===================================================================== */

void add_bound_variables_in_rhs_value (agent* thisAgent,
                   rhs_value rv, tc_number tc,
                                     list **var_list) {
  list *fl;
  cons *c;
  Symbol *sym;

  if (rhs_value_is_symbol(rv)) {
    /* --- ordinary values (i.e., symbols) --- */
    sym = rhs_value_to_symbol(rv);
    if (sym->common.data.symbol_type==VARIABLE_SYMBOL_TYPE)
      mark_variable_if_unmarked (thisAgent, sym, tc, var_list);
  } else {
    /* --- function calls --- */
    fl = rhs_value_to_funcall_list(rv);
    for (c=fl->rest; c!=NIL; c=c->rest)
      add_bound_variables_in_rhs_value (thisAgent, static_cast<char *>(c->first), tc, var_list);
  }
}

void add_bound_variables_in_action (agent* thisAgent, action *a,
                  tc_number tc, list **var_list){
  Symbol *id;

  if (a->type==MAKE_ACTION) {
    /* --- ordinary make actions --- */
    id = rhs_value_to_symbol(a->id);
    add_bound_variables_in_rhs_value (thisAgent, a->id, tc, var_list);
    add_bound_variables_in_rhs_value (thisAgent, a->attr, tc, var_list);
    add_bound_variables_in_rhs_value (thisAgent, a->value, tc, var_list);
    if (preference_is_binary(a->preference_type))
      add_bound_variables_in_rhs_value (thisAgent, a->referent, tc, var_list);
  } else {
    /* --- function call actions --- */
    add_bound_variables_in_rhs_value (thisAgent, a->value, tc, var_list);
  }
}

void add_bound_variables_in_action_list (agent* thisAgent, action *actions, tc_number tc,
                                       list **var_list) {
  action *a;

  for (a=actions; a!=NIL; a=a->next)
    add_bound_variables_in_action (thisAgent, a, tc, var_list);
}

/* -------------------------------------------------------------------
             Reconstructing the RHS Actions of a Production

   When we print a production (but not when we fire one), we have to
   reconstruct the RHS actions.  This is because many of the variables
   in the RHS have been replaced by references to Rete locations (i.e.,
   rather than specifying <v>, we specify "value field 3 levels up"
   or "the 7th RHS unbound variable".  The routines below copy rhs_value's
   and actions, and substitute variable names for such references.
   For RHS unbound variables, we gensym new variable names.
------------------------------------------------------------------- */

rhs_value copy_rhs_value_and_substitute_varnames (agent* thisAgent,
                                                  rhs_value rv,
                                                  condition *cond,
                                                  char first_letter,
                                                  bool should_add_original_vars)
{
  cons *c, *new_c, *prev_new_c;
  list *fl, *new_fl;
  Symbol *sym, *original_sym=NULL;
  int64_t index;
  char prefix[2];

  if (rhs_value_is_reteloc(rv)) {
    /* -- rv is a symbol pointed to by a rete location -- */
    if (should_add_original_vars)
    {
      original_sym = var_bound_in_reconstructed_original_conds (thisAgent, cond,
                                   rhs_value_to_reteloc_field_num(rv),
                                   rhs_value_to_reteloc_levels_up(rv));
      // Debug | May not need these b/c rhs_to_symbol did not increase refcount, but make_rhs_value_symbol does
      // symbol_add_ref(thisAgent, original_sym);
      //#ifdef DEBUG_TRACE_RHS_REFCOUNTS
      //      print(thisAgent, "Debug | copy_rhs_value_and_substitute_varnames increasing refcount of original %s from %ld.\n",
      //             symbol_to_string(thisAgent, original_sym, FALSE, NULL, 0),
      //             original_sym->common.data.reference_count);
      //#endif
    }
    sym = var_bound_in_reconstructed_conds (thisAgent, cond,
        rhs_value_to_reteloc_field_num(rv),
        rhs_value_to_reteloc_levels_up(rv));
    // Debug | May not need these b/c rhs_to_symbol did not increase refcount, but make_rhs_value_symbol does
    // symbol_add_ref(thisAgent, sym);
#ifdef DEBUG_TRACE_RHS_UNIQUE_VARIABLIZATION
    print(thisAgent, "RHS UV| copy_rhs_value_and_substitute_varnames creating, from reteloc, rhs_symbol %s(%s).\n",
          symbol_to_string(thisAgent, sym),
         (original_sym ? symbol_to_string(thisAgent, original_sym) : "NIL"));
#endif
    return make_rhs_value_symbol(thisAgent, sym, original_sym);
  }

  if (rhs_value_is_unboundvar(rv))
  {
    /* -- rv is a an unbound variable -- */
    index = static_cast<int64_t>(rhs_value_to_unboundvar(rv));
    if (! *(thisAgent->rhs_variable_bindings+index))
    {
      prefix[0] = first_letter;
      prefix[1] = 0;

      sym = generate_new_variable (thisAgent, prefix);
      *(thisAgent->rhs_variable_bindings+index) = sym;

      if (thisAgent->highest_rhs_unboundvar_index < index)
      {
        thisAgent->highest_rhs_unboundvar_index = index;
      }
    }
    else
    {
      sym = *(thisAgent->rhs_variable_bindings+index);
      // Debug | May not need these b/c rhs_to_symbol did not increase refcount, but make_rhs_value_symbol does
      //symbol_add_ref(thisAgent, sym);
    }
#ifdef DEBUG_TRACE_RHS_UNIQUE_VARIABLIZATION
    print(thisAgent, "RHS UV| copy_rhs_value_and_substitute_varnames created unbound var rhs_symbol %s(%s).\n",
          symbol_to_string(thisAgent, sym),
         (original_sym ? symbol_to_string(thisAgent, original_sym) : "NIL"));
#endif
    return make_rhs_value_symbol(thisAgent, sym);
  }

  if (rhs_value_is_funcall(rv)) {
    /* -- rv is a rhs function -- */
    fl = rhs_value_to_funcall_list(rv);
    allocate_cons (thisAgent, &new_fl);
    new_fl->first = fl->first;
    prev_new_c = new_fl;
    for (c=fl->rest; c!=NIL; c=c->rest) {
      allocate_cons (thisAgent, &new_c);
      new_c->first = copy_rhs_value_and_substitute_varnames (thisAgent,
                                                             static_cast<char *>(c->first),
                                                             cond,
                                                             first_letter,
                                                             should_add_original_vars);
      prev_new_c->rest = new_c;
      prev_new_c = new_c;
    }
    prev_new_c->rest = NIL;
    return funcall_list_to_rhs_value (new_fl);
  } else {
    /* -- rv is a rhs_symbol -- */
    rhs_symbol rs = rhs_value_to_rhs_symbol(rv);
#ifdef DEBUG_TRACE_RHS_UNIQUE_VARIABLIZATION
    print(thisAgent, "RHS UV| copy_rhs_value_and_substitute_varnames copying rhs_symbol %s(%s).\n",
          symbol_to_string(thisAgent, rs->referent),
         (rs->original_variable ? symbol_to_string(thisAgent, rs->original_variable) : "NIL"));
#endif
    return make_rhs_value_symbol(thisAgent, rs->referent, rs->original_variable);
  }
}

action *copy_action_list_and_substitute_varnames (agent* thisAgent,
                                                  action *actions,
                                                  condition *cond,
                                                  bool should_add_original_vars) {
  action *old, *New, *prev, *first;
  char first_letter;

#ifdef DEBUG_TRACE_RHS_UNIQUE_VARIABLIZATION
    print(thisAgent, "RHS UV| copy_action_list_and_substitute_varnames beginning.\n");
#endif
  prev = NIL;
  first = NIL;
  old = actions;
  while (old) {
    New = make_action(thisAgent);
    if (prev)
      prev->next = New;
    else
      first = New;
    prev = New;
    New->type = old->type;
    New->preference_type = old->preference_type;
    New->support = old->support;
    if (old->type==FUNCALL_ACTION) {
      New->value = copy_rhs_value_and_substitute_varnames (thisAgent,
                                                           old->value, cond,
                                                           'v', should_add_original_vars);
    } else {
      New->id = copy_rhs_value_and_substitute_varnames (thisAgent, old->id, cond, 's', should_add_original_vars);
      New->attr = copy_rhs_value_and_substitute_varnames (thisAgent, old->attr, cond,'a', should_add_original_vars);
      first_letter = first_letter_from_rhs_value (New->attr);
      New->value = copy_rhs_value_and_substitute_varnames (thisAgent, old->value, cond,
                          first_letter, should_add_original_vars);
      if (preference_is_binary(old->preference_type))
        New->referent = copy_rhs_value_and_substitute_varnames (thisAgent, old->referent,
                                              cond, first_letter, should_add_original_vars);
    }
    old = old->next;
  }
  if (prev) prev->next = NIL; else first = NIL;
#ifdef DEBUG_TRACE_RHS_UNIQUE_VARIABLIZATION
    print(thisAgent, "RHS UV| copy_action_list_and_substitute_varnames ending.\n");
#endif
  return first;
}

action *make_action(agent *thisAgent)
{
  action *new_action;
  allocate_with_pool (thisAgent, &thisAgent->action_pool,  &new_action);
  new_action->next = NIL;
  new_action->id = NIL;
  new_action->attr = NIL;
  new_action->value = NIL;
  new_action->referent = NIL;
  return new_action;
}
