#include "portability.h"

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/*************************************************************************
 *
 *  file:  reorder.cpp
 *
 * =======================================================================
 *  
 *  Need header/description comments here
 *  
 *  
 * =======================================================================
 */

#include "reorder.h"
#include "kernel.h"
#include "rhsfun.h"
#include "gdatastructs.h"
#include "production.h"
#include "mem.h"
#include "symtab.h"
#include "print.h"
#include "agent.h"
#include "init_soar.h"
#include "xmlTraceNames.h" // for constants for XML function types, tags and attributes
#include "gski_event_system_functions.h" // support for triggering XML events

/* *********************************************************************

                             Reordering

********************************************************************* */

/* =====================================================================

                  Name of production being reordered

   In case any errors are encountered during reordering, this variable
   holds the name of the production currently being reordered, so it
   can be printed with the error message.
===================================================================== */

/*#define symbol_is_constant_or_marked_variable(sym,tc) \
  ( ((sym)->common.symbol_type!=VARIABLE_SYMBOL_TYPE) || \
    ((sym)->var.tc_num == (tc)) )*/
inline Bool symbol_is_constant_or_marked_variable(Symbol * sym, tc_number tc)
{
  return ( ((sym)->common.symbol_type!=VARIABLE_SYMBOL_TYPE) ||
    ((sym)->var.tc_num == (tc)) );
}

/* =====================================================================

                      Reordering for RHS Actions

  Whenever a new identifier is created, we need to know (at creation time)
  what level of the goal stack it's at.  If the <newid> appears in the
  attribute or value slot of a make, we just give it the same level as
  whatever's in the id slot of that make.  But if <newid> appears in the
  id slot of a make, we can't tell what level it goes at.  

  To avoid this problem, we reorder the list of RHS actions so that when
  the actions are executed (in the resulting order), each <newid> is
  encountered in an attribute or value slot *before* it is encountered
  in an id slot.
  
  Furthermore, we make sure all arguments to function calls are bound
  before the function call is executed.
  
  Reorder_action_list() does the reordering.  Its parameter action_list
  is reordered in place (destructively modified).  It also requires at entry
  that the variables bound on the LHS are marked.  The function returns
  TRUE if successful, FALSE if it was unable to produce a legal ordering.
===================================================================== */

Bool legal_to_execute_action (action *a, tc_number tc);

Bool reorder_action_list (agent* thisAgent, action **action_list, 
						  tc_number lhs_tc) {
  list *new_bound_vars;
  action *remaining_actions;
  action *first_action, *last_action;
  action *a, *prev_a;
  Bool result_flag;

  new_bound_vars = NIL;
  remaining_actions = *action_list;
  first_action = NIL;
  last_action = NIL;
  
  while (remaining_actions) {
    /* --- scan through remaining_actions, look for one that's legal --- */
    prev_a = NIL;
    a = remaining_actions;
    while (TRUE) {
      if (!a) break; /* looked at all candidates, but none were legal */
      if (legal_to_execute_action (a, lhs_tc)) break;
      prev_a = a;
      a = a->next;
    }
    if (!a) break;
    /* --- move action a from remaining_actions to reordered list --- */
    if (prev_a) prev_a->next = a->next; else remaining_actions = a->next;
    a->next = NIL;
    if (last_action) last_action->next = a; else first_action = a;
    last_action = a;
    /* --- add new variables from a to new_bound_vars --- */
    add_all_variables_in_action (thisAgent, a, lhs_tc, &new_bound_vars);
  }

  if (remaining_actions) {
    /* --- there are remaining_actions but none can be legally added --- */
    print (thisAgent, "Error: production %s has a bad RHS--\n",
           thisAgent->name_of_production_being_reordered);
    print (thisAgent, "       Either it creates structure not connected to anything\n");
    print (thisAgent, "       else in WM, or it tries to pass an unbound variable as\n");
    print (thisAgent, "       an argument to a function.\n");
    /* --- reconstruct list of all actions --- */
    if (last_action)
      last_action->next = remaining_actions;
    else
      first_action = remaining_actions;
    result_flag = FALSE;
  } else {
    result_flag = TRUE;
  }

  /* --- unmark variables that we just marked --- */
  unmark_variables_and_free_list (thisAgent, new_bound_vars);

  /* --- return final result --- */
  *action_list = first_action;
  return result_flag;
}

Bool all_variables_in_rhs_value_bound (rhs_value rv, tc_number tc) {
  cons *c;
  list *fl;
  Symbol *sym;
  
  if (rhs_value_is_funcall(rv)) {
    /* --- function calls --- */
    fl = rhs_value_to_funcall_list (rv);
    for (c=fl->rest; c!=NIL; c=c->rest)
      if (! all_variables_in_rhs_value_bound (static_cast<char *>(c->first), tc))
        return FALSE;
    return TRUE;
  } else {
    /* --- ordinary (symbol) rhs values --- */
    sym = rhs_value_to_symbol (rv);
    if (sym->common.symbol_type==VARIABLE_SYMBOL_TYPE)
      return (sym->var.tc_num == tc);
    return TRUE;
  }
}

Bool legal_to_execute_action (action *a, tc_number tc) {
  if (a->type==MAKE_ACTION) {
    if (! all_variables_in_rhs_value_bound (a->id, tc)) return FALSE;
    if (rhs_value_is_funcall(a->attr) &&
        (! all_variables_in_rhs_value_bound (a->attr, tc))) return FALSE;
    if (rhs_value_is_funcall(a->value) &&
        (! all_variables_in_rhs_value_bound (a->value, tc))) return FALSE;
    if (preference_is_binary(a->preference_type) &&
        rhs_value_is_funcall(a->referent) &&
        (! all_variables_in_rhs_value_bound (a->referent, tc))) return FALSE;
    return TRUE;
  }
  /* --- otherwise it's a function call; make sure args are all bound  --- */
  return all_variables_in_rhs_value_bound (a->value, tc);
}

/* =====================================================================

                 Condition Simplification / Restoration

  In order to be able to move tests from one condition to another, we
  reorder using the following high-level technique.  (This procedure is
  applied separately at each level of nesting.)

    1. Simplify the positive conditions, by stripping off all tests other
       than equality.  When this is done, all tests in positive conditions
       are either equality tests or conjunctions of equality tests.  All
       other tests are saved away for later restoration.
    2. Then do the reordering...
    3. Then go back and restore all the tests that were previously saved
       away.  The restored tests might end up on different conditions
       than they started--they're inserted in the first place possible
       (i.e., as soon as all the necessary things are bound).

  The two main routines here are simplify_condition_list() and
  restore_and_deallocate_saved_tests().
       
===================================================================== */

typedef struct saved_test_struct {
  struct saved_test_struct *next;
  Symbol *var;
  test the_test;
} saved_test;


void print_saved_test (agent* thisAgent, saved_test *st) {
  print_with_symbols (thisAgent, "  Symbol: %y  Test: ", st->var);
  print_string (thisAgent, test_to_string (thisAgent, st->the_test, NULL, 0));
}

void print_saved_test_list (agent* thisAgent, saved_test *st) {
  while (st) {
    print_saved_test (thisAgent, st);
    print (thisAgent, "\n");
    st = st->next;
  }
}

saved_test *simplify_test (agent* thisAgent, test *t, saved_test *old_sts) {
  test New, subtest;
  saved_test *saved;
  Symbol *var, *sym;
  cons *c, *prev_c, *next_c;
  complex_test *ct;

  if (test_is_blank_test(*t)) {
    sym = generate_new_variable (thisAgent, "dummy-");
    *t = make_equality_test_without_adding_reference (sym);
    return old_sts;
  }

  if (test_is_blank_or_equality_test(*t)) {
    return old_sts;
  }

  ct = complex_test_from_test(*t);
  
  switch (ct->type) {
    
  case CONJUNCTIVE_TEST:
    /* --- look at subtests for an equality test --- */
    sym = NIL;
    for (c=ct->data.conjunct_list; c!=NIL; c=c->rest) {
      subtest = static_cast<char *>(c->first);
      if (test_is_blank_or_equality_test(subtest))
        sym = referent_of_equality_test(subtest);
    }
    /* --- if no equality test was found, generate a variable for it --- */
    if (!sym) {
      sym = generate_new_variable (thisAgent, "dummy-");
      New = make_equality_test_without_adding_reference (sym);
      allocate_cons (thisAgent, &c);
      c->first = New;
      c->rest = ct->data.conjunct_list;
      ct->data.conjunct_list = c;
    }
    /* --- scan through, create saved_test for subtests except equality --- */
    prev_c = NIL;
    c = ct->data.conjunct_list;
    while (c) {
      next_c = c->rest;
      subtest = static_cast<char *>(c->first);
      if (! test_is_blank_or_equality_test(subtest)) {
        /* --- create saved_test, splice this cons out of conjunct_list --- */
        allocate_with_pool (thisAgent, &thisAgent->saved_test_pool, &saved);
        saved->next = old_sts;
        old_sts = saved;
        saved->var = sym;
        symbol_add_ref (sym);
        saved->the_test = subtest;
        if (prev_c)
          prev_c->rest = next_c;
        else
          ct->data.conjunct_list = next_c;
        free_cons (thisAgent, c);
      } else {
        prev_c = c;
      }
      c = next_c;
    }
    break;
    
  default:
    /* --- goal/impasse, disjunction, and non-equality relational tests --- */
    var = generate_new_variable (thisAgent, "dummy-");
    New = make_equality_test_without_adding_reference (var);
    allocate_with_pool (thisAgent, &thisAgent->saved_test_pool, &saved);
    saved->next = old_sts;
    old_sts = saved;
    saved->var = var;
    symbol_add_ref (var);
    saved->the_test = *t;
    *t = New;
    break;
  }
  return old_sts;
}

saved_test *simplify_condition_list (agent* thisAgent, condition *conds_list) {
  condition *c;
  saved_test *sts;

  sts = NIL;
  for (c=conds_list; c!=NIL; c=c->next) {
    if (c->type==POSITIVE_CONDITION) {
      sts = simplify_test (thisAgent, &(c->data.tests.id_test), sts);
      sts = simplify_test (thisAgent, &(c->data.tests.attr_test), sts);
      sts = simplify_test (thisAgent, &(c->data.tests.value_test), sts);
    }
  }
  return sts;
}

byte reverse_direction_of_relational_test (agent* thisAgent, byte type) {
  switch (type) {
    case NOT_EQUAL_TEST: return NOT_EQUAL_TEST;
    case LESS_TEST: return GREATER_TEST;
    case GREATER_TEST: return LESS_TEST;
    case LESS_OR_EQUAL_TEST: return GREATER_OR_EQUAL_TEST;
    case GREATER_OR_EQUAL_TEST: return LESS_OR_EQUAL_TEST;
    case SAME_TYPE_TEST: return SAME_TYPE_TEST;
    default:
      { char msg[BUFFER_MSG_SIZE];
      strncpy (msg,
	      "Internal error: arg to reverse_direction_of_relational_test\n", BUFFER_MSG_SIZE);
      msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
      abort_with_fatal_error(thisAgent, msg);
      }
  }
  return 0; /* unreachable, but without it, gcc -Wall warns here */
}

saved_test *restore_saved_tests_to_test (agent* thisAgent, 
										 test *t,
                                         Bool is_id_field,
                                         tc_number bound_vars_tc_number,
                                         saved_test *tests_to_restore) {
  saved_test *st, *prev_st, *next_st;
  Bool added_it;
  Symbol *referent;
  complex_test *ct;
  
  prev_st = NIL;
  st = tests_to_restore;
  while (st) {
    next_st = st->next;
    added_it = FALSE;
    ct = complex_test_from_test(st->the_test);
    switch (ct->type) {
    case GOAL_ID_TEST:
    case IMPASSE_ID_TEST:
      if (! is_id_field) break; /* goal/impasse tests only go in id fields */
      /* ... otherwise fall through to the next case below ... */
    case DISJUNCTION_TEST:
      if (test_includes_equality_test_for_symbol (*t, st->var)) {
        add_new_test_to_test_if_not_already_there (thisAgent, t, st->the_test);
        added_it = TRUE;
      }
      break;
    default:  /* --- st->test is a relational test other than equality --- */
      referent = ct->data.referent;
      if (test_includes_equality_test_for_symbol (*t, st->var)) {
        if (symbol_is_constant_or_marked_variable (referent,
                                                   bound_vars_tc_number) ||
           (st->var == referent)) {
          add_new_test_to_test_if_not_already_there (thisAgent, t, st->the_test);
          added_it = TRUE;
        } 
      } else if (test_includes_equality_test_for_symbol (*t, referent)) {
        if (symbol_is_constant_or_marked_variable (st->var,
                                                   bound_vars_tc_number) ||
           (st->var == referent)) {
          ct->type = reverse_direction_of_relational_test (thisAgent, ct->type);
          ct->data.referent = st->var;
          st->var = referent;
          add_new_test_to_test_if_not_already_there (thisAgent, t, st->the_test);
          added_it = TRUE;
        }
      }
      break;
    } /* end of switch statement */
    if (added_it) {
      if (prev_st) prev_st->next = next_st; else tests_to_restore = next_st;
      symbol_remove_ref (thisAgent, st->var);
      free_with_pool (&thisAgent->saved_test_pool, st);
    } else {
      prev_st = st;
    }
    st = next_st;
  } /* end of while (st) */
  return tests_to_restore;
}

void restore_and_deallocate_saved_tests (agent* thisAgent, 
										 condition *conds_list,
                                         /* tc number for vars bound outside */
                                         tc_number tc, 
                                         saved_test *tests_to_restore) {
  condition *cond;
  list *new_vars;

  new_vars = NIL;
  for (cond=conds_list; cond!=NIL; cond=cond->next) {
    if (cond->type!=POSITIVE_CONDITION) continue;
    tests_to_restore = restore_saved_tests_to_test
      (thisAgent, (&cond->data.tests.id_test), TRUE, tc, tests_to_restore);
    add_bound_variables_in_test (thisAgent, cond->data.tests.id_test, tc, &new_vars);
    tests_to_restore = restore_saved_tests_to_test
      (thisAgent, (&cond->data.tests.attr_test), FALSE, tc, tests_to_restore);
    add_bound_variables_in_test (thisAgent, cond->data.tests.attr_test, tc, &new_vars);
    tests_to_restore = restore_saved_tests_to_test
      (thisAgent, (&cond->data.tests.value_test), FALSE, tc, tests_to_restore);
    add_bound_variables_in_test (thisAgent, cond->data.tests.value_test, tc, &new_vars);
  }
  if (tests_to_restore) {
    if (thisAgent->sysparams[PRINT_WARNINGS_SYSPARAM]) {
      print (thisAgent, "\nWarning:  in production %s,\n",
             thisAgent->name_of_production_being_reordered);
      print (thisAgent, "      ignoring test(s) whose referent is unbound:\n");
      print_saved_test_list (thisAgent, tests_to_restore);
      // TODO: XML tagged output -- how to create this string?
      // KJC TODO:  need a tagged output version of print_saved_test_list

      // XML generation
      growable_string gs = make_blank_growable_string(thisAgent);
      add_to_growable_string(thisAgent, &gs, "Warning:  in production ");
      add_to_growable_string(thisAgent, &gs, thisAgent->name_of_production_being_reordered);
      add_to_growable_string(thisAgent, &gs, "\n      ignoring test(s) whose referent is unbound:");
      //TODO: fill in XML print_saved_test_list. Possibile methods include:
      //   1) write a version which adds to a growable string
      //   2) write a version which generates XML tags/attributes, so we get "typed" output for this warning
      //      i.e. "<warning><string value="beginning of message"></string><test att="val"></test><string value="rest of message"></string></warning>
      GenerateWarningXML(thisAgent, text_of_growable_string(gs));

      free_growable_string(thisAgent, gs);
    }
    /* ought to deallocate the saved tests, but who cares */
  }
  unmark_variables_and_free_list (thisAgent, new_vars);
}

/* =====================================================================

           Finding The Variables in a Negated Condition (or NCC)
                That Refer to Variables Bound Outside

  If a variable occurs within a negated condition (or NCC), and that 
  same variable is bound by some positive condition outside the negation,
  then the reorderer must ensure that the positive (binding) condition 
  comes before the negated (testing) condition.  To do this, we put
  a list on every NC or NCC of all the variables whose bindings it requires.

  When the reorderer is finished, these lists are removed.
  
  The main routines here are fill_in_vars_requiring_bindings() and
  remove_vars_requiring_bindings().  Each of these recursively traverses
  the lhs and does its work at all nesting levels.
  Fill_in_vars_requiring_bindings() takes a tc_number parameter which
  indicates the variables that are bound outside the given condition list.
  (At the top level, this should be *no* variables.)
  
===================================================================== */

list *collect_vars_tested_by_test_that_are_bound (agent* thisAgent, test t,
                                                  tc_number tc,
                                                  list *starting_list) {
  cons *c;
  complex_test *ct;
  Symbol *referent;

  if (test_is_blank_test(t)) return starting_list;

  if (test_is_blank_or_equality_test(t)) {
    referent = referent_of_equality_test(t);
    if (referent->common.symbol_type==VARIABLE_SYMBOL_TYPE)
      if (referent->var.tc_num == tc)
        starting_list = add_if_not_member (thisAgent, referent, starting_list);
    return starting_list;
  }

  ct = complex_test_from_test(t);
  
  switch (ct->type) {
  case GOAL_ID_TEST:
  case IMPASSE_ID_TEST:
  case DISJUNCTION_TEST:
    return starting_list; 
    
  case CONJUNCTIVE_TEST:
    for (c=ct->data.conjunct_list; c!=NIL; c=c->rest)
      starting_list = collect_vars_tested_by_test_that_are_bound
        (thisAgent, static_cast<char *>(c->first), tc, starting_list);
    return starting_list;
    
  default:
    /* --- relational tests other than equality --- */
    referent = ct->data.referent;
    if (referent->common.symbol_type==VARIABLE_SYMBOL_TYPE)
      if (referent->var.tc_num == tc)
        starting_list = add_if_not_member (thisAgent, referent, starting_list);
    return starting_list; 
  }
}
                          
list *collect_vars_tested_by_cond_that_are_bound (agent* thisAgent, 
												  condition *cond,
                                                  tc_number tc,
                                                  list *starting_list) {
  condition *c;

  if (cond->type==CONJUNCTIVE_NEGATION_CONDITION) {
    /* --- conjuctive negations --- */
    for (c=cond->data.ncc.top; c!=NIL; c=c->next)
      starting_list = collect_vars_tested_by_cond_that_are_bound
        (thisAgent, c, tc, starting_list);
  } else {
    /* --- positive, negative conditions --- */
    starting_list = collect_vars_tested_by_test_that_are_bound
      (thisAgent, cond->data.tests.id_test, tc, starting_list);
    starting_list = collect_vars_tested_by_test_that_are_bound
      (thisAgent, cond->data.tests.attr_test, tc, starting_list);
    starting_list = collect_vars_tested_by_test_that_are_bound
      (thisAgent, cond->data.tests.value_test, tc, starting_list);
  }
  return starting_list;
}

void fill_in_vars_requiring_bindings (agent* thisAgent, condition *cond_list, tc_number tc) {
  list *new_bound_vars;
  condition *c;

  /* --- add anything bound in a positive condition at this level --- */
  new_bound_vars = NIL;
  for (c=cond_list; c!=NIL; c=c->next)
    if (c->type==POSITIVE_CONDITION)
      add_bound_variables_in_condition (thisAgent, c, tc, &new_bound_vars);

  /* --- scan through negated and NC cond's, fill in stuff --- */
  for (c=cond_list; c!=NIL; c=c->next) {
    if (c->type!=POSITIVE_CONDITION)
      c->reorder.vars_requiring_bindings =
        collect_vars_tested_by_cond_that_are_bound (thisAgent, c, tc, NIL);
    if (c->type==CONJUNCTIVE_NEGATION_CONDITION)
      fill_in_vars_requiring_bindings (thisAgent, c->data.ncc.top, tc);
  }

  unmark_variables_and_free_list (thisAgent, new_bound_vars);
}

void remove_vars_requiring_bindings (agent* thisAgent, 
									 condition *cond_list) {
  condition *c;

  /* --- scan through negated and NC cond's, remove lists from them --- */
  for (c=cond_list; c!=NIL; c=c->next) {
    if (c->type!=POSITIVE_CONDITION)
      free_list (thisAgent, c->reorder.vars_requiring_bindings);
    if (c->type==CONJUNCTIVE_NEGATION_CONDITION)
      remove_vars_requiring_bindings (thisAgent, c->data.ncc.top);
  }
}

/* =====================================================================

             Finding the Root Variables in a Condition List

   This routine finds the root variables in a given condition list.
   The caller should setup the current tc to be the set of variables
   bound outside the given condition list.  (This should normally be
   an empty TC, except when the condition list is the subconditions
   of an NCC.)  If the "allow_printing_warnings" flag is TRUE, then
   this routine makes sure each root variable is accompanied by a
   goal or impasse id test, and prints a warning message if it isn't.
===================================================================== */

list *collect_root_variables (agent* thisAgent, 
							  condition *cond_list,
                              tc_number tc, /* for vars bound outside */
                              Bool allow_printing_warnings) {

  list *new_vars_from_value_slot;
  list *new_vars_from_id_slot;
  cons *c;
  condition *cond;
  Bool found_goal_impasse_test;
 
  /* --- find everthing that's in the value slot of some condition --- */
  new_vars_from_value_slot = NIL;
  for (cond=cond_list; cond!=NIL; cond=cond->next)
    if (cond->type==POSITIVE_CONDITION)
      add_bound_variables_in_test (thisAgent, cond->data.tests.value_test, tc,
                                   &new_vars_from_value_slot);

  /* --- now see what else we can add by throwing in the id slot --- */
  new_vars_from_id_slot = NIL;
  for (cond=cond_list; cond!=NIL; cond=cond->next)
    if (cond->type==POSITIVE_CONDITION)
      add_bound_variables_in_test (thisAgent, cond->data.tests.id_test, tc,
                                   &new_vars_from_id_slot);

  /* --- unmark everything we just marked --- */
  unmark_variables_and_free_list (thisAgent, new_vars_from_value_slot);
  for (c=new_vars_from_id_slot; c!=NIL; c=c->rest)
    ((Symbol *)(c->first))->var.tc_num = 0;
  
  /* --- make sure each root var has some condition with goal/impasse --- */
  if (allow_printing_warnings && thisAgent->sysparams[PRINT_WARNINGS_SYSPARAM]) {
    for (c=new_vars_from_id_slot; c!=NIL; c=c->rest) {
      found_goal_impasse_test = FALSE;
      for (cond=cond_list; cond!=NIL; cond=cond->next) {
        if (cond->type!=POSITIVE_CONDITION) continue;
        if (test_includes_equality_test_for_symbol (cond->data.tests.id_test,
                                                    static_cast<symbol_union *>(c->first)))
          if (test_includes_goal_or_impasse_id_test (cond->data.tests.id_test,
                                                     TRUE, TRUE)) {
            found_goal_impasse_test = TRUE;
            break;
          }
      }
      if (! found_goal_impasse_test) {
        print (thisAgent, "\nWarning: On the LHS of production %s, identifier ",
               thisAgent->name_of_production_being_reordered);
        print_with_symbols (thisAgent, "%y is not connected to any goal or impasse.\n",
                            (Symbol *)(c->first));

        // XML geneneration
        growable_string gs = make_blank_growable_string(thisAgent);
        add_to_growable_string(thisAgent, &gs, "Warning: On the LHS of production ");
        add_to_growable_string(thisAgent, &gs, thisAgent->name_of_production_being_reordered);
        add_to_growable_string(thisAgent, &gs, ", identifier ");
        add_to_growable_string(thisAgent, &gs, symbol_to_string (thisAgent, (Symbol *)(c->first), true, 0, 0));
        add_to_growable_string(thisAgent, &gs, " is not connected to any goal or impasse.");
        GenerateWarningXML(thisAgent, text_of_growable_string(gs));
        free_growable_string(thisAgent, gs);

      }
    }
  }
  
  return new_vars_from_id_slot;
}

/* =====================================================================

                     Reordering for LHS Conditions

   (Sorry for the poor comments here.  I think the reorderer needs
   substantial revisions in order to make Soar reliably scalable, so
   most of this code will eventually get thrown out anyway...)
===================================================================== */

/* --- estimated k-search branching factors --- */ 
#define MAX_COST 10000005           /* cost of a disconnected condition */
#define BF_FOR_ACCEPTABLE_PREFS 8   /* cost of (. ^. <var> +) */
#define BF_FOR_VALUES 8             /* cost of (. ^. <var>) */
#define BF_FOR_ATTRIBUTES 8         /* cost of (. ^<var> .) */

/* -------------------------------------------------------------
   Return TRUE iff the given test is covered by the previously
   bound variables.  The set of previously bound variables is
   given by the current TC, PLUS any variables in the list
   "extra_vars."
------------------------------------------------------------- */

Bool test_covered_by_bound_vars (test t, tc_number tc, list *extra_vars) {
  cons *c;
  Symbol *referent;
  complex_test *ct;

  if (test_is_blank_test(t)) return FALSE;
  
  if (test_is_blank_or_equality_test(t)) {
    referent = referent_of_equality_test(t);
    if (symbol_is_constant_or_marked_variable (referent, tc))
      return TRUE;
    if (extra_vars) return member_of_list (referent, extra_vars);
    return FALSE;
  }

  ct = complex_test_from_test(t);
  if (ct->type==CONJUNCTIVE_TEST) {
    for (c=ct->data.conjunct_list; c!=NIL; c=c->rest)
      if (test_covered_by_bound_vars (static_cast<char *>(c->first), tc, extra_vars))
        return TRUE;
  }
  return FALSE;
}

/* -------------------------------------------------------------
   Returns the user set value of the expected match cost of the
   multi-attribute, or 1 if the input symbol isn't in the user
   set list.
------------------------------------------------------------- */

long get_cost_of_possible_multi_attribute(agent* thisAgent, Symbol *sym)
{
  multi_attribute *m = thisAgent->multi_attributes;
  while(m) {
    if(m->symbol == sym) return m->value;
    m = m->next;
  }
  return 1;
}

/* -------------------------------------------------------------
   Return an estimate of the "cost" of the given condition.
   The current TC should be the set of previously bound variables;
   "root_vars_not_bound_yet" should be the set of other root
   variables.
------------------------------------------------------------- */

long cost_of_adding_condition (agent* thisAgent, 
							   condition *cond,
                               tc_number tc,
                               list *root_vars_not_bound_yet) {
  cons *c;
  long result;

  /* --- handle the common simple case quickly up front --- */
  if ((! root_vars_not_bound_yet) &&
      (cond->type==POSITIVE_CONDITION) &&
      (test_is_blank_or_equality_test (cond->data.tests.id_test)) &&
      (test_is_blank_or_equality_test (cond->data.tests.attr_test)) &&
      (test_is_blank_or_equality_test (cond->data.tests.value_test)) &&
      (! test_is_blank_test (cond->data.tests.id_test)) &&
      (! test_is_blank_test (cond->data.tests.attr_test)) &&
      (! test_is_blank_test (cond->data.tests.value_test)) ) {

    if (! symbol_is_constant_or_marked_variable
          (referent_of_equality_test (cond->data.tests.id_test), tc))
      return MAX_COST;
    if (symbol_is_constant_or_marked_variable
          (referent_of_equality_test (cond->data.tests.attr_test), tc))
      result = get_cost_of_possible_multi_attribute
          (thisAgent, referent_of_equality_test (cond->data.tests.attr_test));
    else
      result =  BF_FOR_ATTRIBUTES;

    if (! symbol_is_constant_or_marked_variable
          (referent_of_equality_test(cond->data.tests.value_test),tc)){
      if (cond->test_for_acceptable_preference)
        result = result * BF_FOR_ACCEPTABLE_PREFS;
      else
        result = result * BF_FOR_VALUES;
    }
    return result;
  } /* --- end of common simple case --- */

  if (cond->type==POSITIVE_CONDITION) {
    /* --- for pos cond's, check what's bound, etc. --- */
    if (! test_covered_by_bound_vars (cond->data.tests.id_test, tc,
                                      root_vars_not_bound_yet))
      return MAX_COST;
    if (test_covered_by_bound_vars (cond->data.tests.attr_test, tc,
                                    root_vars_not_bound_yet))
      result = 1;
    else
      result =  BF_FOR_ATTRIBUTES;
    if (! test_covered_by_bound_vars (cond->data.tests.value_test, tc,
                                      root_vars_not_bound_yet)) {
      if (cond->test_for_acceptable_preference)
        result = result * BF_FOR_ACCEPTABLE_PREFS;
      else
        result = result * BF_FOR_VALUES;
    }
    return result;
  }
  /* --- negated or NC conditions:  just check whether all variables
     requiring bindings are actually bound.  If so, return 1, else
     return MAX_COST --- */
  for (c=cond->reorder.vars_requiring_bindings; c!=NIL; c=c->rest) {
    if (((Symbol *)(c->first))->var.tc_num != tc) return MAX_COST;
  }
  return 1;
}

/* -------------------------------------------------------------
   Return an estimate of the "cost" of the lowest-cost condition
   that could be added next, IF the given "chosen" condition is
   added first.
------------------------------------------------------------- */

long find_lowest_cost_lookahead (agent* thisAgent, 
								 condition *candidates,
                                 condition *chosen,
                                 tc_number tc,
                                 list *root_vars_not_bound_yet) {
  condition *c;
  long min_cost, cost;
  list *new_vars;

  new_vars = NIL;
  add_bound_variables_in_condition (thisAgent, chosen, tc, &new_vars);
  min_cost = MAX_COST + 1;
  for (c=candidates; c!=NIL; c=c->next) {
    if (c==chosen) continue;
    cost = cost_of_adding_condition (thisAgent, c, tc, root_vars_not_bound_yet);
    if (cost < min_cost) {
      min_cost = cost;
      if (cost <= 1) break;
    }
  }
  unmark_variables_and_free_list (thisAgent, new_vars);
  return min_cost;
}

/* -------------------------------------------------------------
   Reorder the given list of conditions.  The "top_of_conds" and
   "bottom_of_conds" arguments are destructively modified to reflect
   the reordered conditions.  The "bound_vars_tc_number"
   should reflect the variables bound outside the given condition list.
   The "reorder_nccs" flag indicates whether it is necessary to
   recursively reorder the subconditions of NCC's.  (For newly
   built chunks, this is never necessary.)
------------------------------------------------------------- */

void reorder_condition_list (agent* thisAgent, 
							 condition **top_of_conds,
                             condition **bottom_of_conds,
                             list *roots,
                             tc_number tc,
                             Bool reorder_nccs);

void reorder_simplified_conditions (agent* thisAgent, 
									condition **top_of_conds,
                                    condition **bottom_of_conds,
                                    list *roots,
                                    tc_number bound_vars_tc_number,
                                    Bool reorder_nccs) {
  condition *remaining_conds;           /* header of dll */
  condition *first_cond, *last_cond;
  condition *cond, *next_cond;
  condition *min_cost_conds, *chosen;
  long cost, min_cost;
  list *new_vars;

  remaining_conds = *top_of_conds;
  first_cond = NIL;
  last_cond = NIL;
  new_vars = NIL;
  
  /* repeat:  scan through remaining_conds
              rate each one
              if tie, call lookahead routine
              add min-cost item to conds
  */
  
  while (remaining_conds) {
    /* --- find min-cost set --- */
    min_cost_conds = NIL;
    min_cost = 0;
    for (cond=remaining_conds; cond!=NIL; cond=cond->next) {
      cost = cost_of_adding_condition (thisAgent, cond, bound_vars_tc_number, roots);
      if ((! min_cost_conds) || (cost < min_cost)) {
        min_cost = cost;
        min_cost_conds = cond;
        cond->reorder.next_min_cost = NIL;
      } else if (cost==min_cost) {
        cond->reorder.next_min_cost = min_cost_conds;
        min_cost_conds = cond;
      }
      /* if (min_cost <= 1) break;  This optimization needs to be removed,
                                    otherwise the tie set is not created.
                                    Without the tie set we can't check the
                                    canonical order. */
    }
    /* --- if min_cost==MAX_COST, print error message --- */
    if ((min_cost==MAX_COST) &&
        thisAgent->sysparams[PRINT_WARNINGS_SYSPARAM]) {
      print (thisAgent, "Warning:  in production %s,\n",
             thisAgent->name_of_production_being_reordered);
      print (thisAgent, "     The LHS conditions are not all connected.\n");
      /* BUGBUG I'm not sure whether this can ever happen. */

      // XML geneneration
      growable_string gs = make_blank_growable_string(thisAgent);
      add_to_growable_string(thisAgent, &gs, "Warning:  in production ");
      add_to_growable_string(thisAgent, &gs, thisAgent->name_of_production_being_reordered);
      add_to_growable_string(thisAgent, &gs, "\n     The LHS conditions are not all connected.");
      GenerateWarningXML(thisAgent, text_of_growable_string(gs));
      free_growable_string(thisAgent, gs);

     }
    /* --- if more than one min-cost item, and cost>1, do lookahead --- */
    if ((min_cost > 1) && (min_cost_conds->reorder.next_min_cost)) {
      min_cost = MAX_COST + 1;
      for (cond=min_cost_conds, next_cond = cond->reorder.next_min_cost;
           cond!=NIL;
           cond=next_cond, next_cond=(cond?cond->reorder.next_min_cost:NIL)) {
        cost = find_lowest_cost_lookahead (thisAgent, remaining_conds, cond,
                                           bound_vars_tc_number, roots);
        if (cost < min_cost) {
          min_cost = cost;
          min_cost_conds = cond;
          cond->reorder.next_min_cost = NIL;
        } else {
/*******************************************************************
 These code segments find the condition in the tie set with the smallest
 value in the canonical order. This ensures that productions with the
 same set of conditions are ordered the same. Except if the variables
 are assigned differently.
*********************************************************************/
          if (cost == min_cost && cond->type == POSITIVE_CONDITION) {
              if (canonical_cond_greater(min_cost_conds,cond)) {
                min_cost = cost;
                min_cost_conds = cond;
                cond->reorder.next_min_cost = NIL;
              }
            }
        }
/*******************************************************************/

      }
    }
/*******************************************************************/
    if (min_cost == 1 && (min_cost_conds->reorder.next_min_cost)) {
      for (cond=min_cost_conds; cond!=NIL; cond=cond->reorder.next_min_cost) {
        if (cond->type == POSITIVE_CONDITION &&
          min_cost_conds->type == POSITIVE_CONDITION &&
          canonical_cond_greater(min_cost_conds,cond))
        {
          min_cost = cost;
          min_cost_conds = cond;
        }
       else if (cond->type != POSITIVE_CONDITION &&
                 min_cost_conds->type == POSITIVE_CONDITION)
        {
          min_cost = cost;
          min_cost_conds = cond;
        }
      }
    }
/*******************************************************************/

    /* --- install the first item in the min-cost set --- */
    chosen = min_cost_conds;
    remove_from_dll (remaining_conds, chosen, next, prev);
    if (!first_cond) first_cond = chosen;
    /* Note: args look weird on the next line, because we're really
       inserting the chosen item at the *end* of the list */
    insert_at_head_of_dll (last_cond, chosen, prev, next);

    /* --- if a conjunctive negation, recursively reorder its conditions --- */
    if ((chosen->type==CONJUNCTIVE_NEGATION_CONDITION) && reorder_nccs) {
      list *ncc_roots;
      ncc_roots = collect_root_variables (thisAgent, chosen->data.ncc.top,
                                          bound_vars_tc_number, TRUE);
      reorder_condition_list (thisAgent, &(chosen->data.ncc.top),
                              &(chosen->data.ncc.bottom),
                              ncc_roots,
                              bound_vars_tc_number,
                              reorder_nccs);
      free_list (thisAgent, ncc_roots);
    }

    /* --- update set of bound variables for newly added condition --- */
    add_bound_variables_in_condition (thisAgent, chosen, bound_vars_tc_number, &new_vars);
    
    /* --- if all roots are bound, set roots=NIL: don't need 'em anymore --- */
    if (roots) {
      cons *c;
      for (c=roots; c!=NIL; c=c->rest)
        if (((Symbol *)(c->first))->var.tc_num != bound_vars_tc_number)
          break;
      if (!c) roots=NIL;
    }

  } /* end of while (remaining_conds) */

  unmark_variables_and_free_list (thisAgent, new_vars);
  *top_of_conds = first_cond;
  *bottom_of_conds = last_cond;
}

void reorder_condition_list (agent* thisAgent, 
							 condition **top_of_conds,
                             condition **bottom_of_conds,
                             list *roots,
                             tc_number tc, /* for vars bound outside */
                             Bool reorder_nccs) {
  saved_test *saved_tests;

  saved_tests = simplify_condition_list (thisAgent, *top_of_conds);
  reorder_simplified_conditions (thisAgent, top_of_conds, bottom_of_conds, roots, tc,
                                 reorder_nccs);
  restore_and_deallocate_saved_tests (thisAgent, *top_of_conds, tc, saved_tests);
}

/* -------------------------------------------------------------
   Reorders the LHS.
------------------------------------------------------------- */

/* SBH/MVP 6-24-94 */

Bool test_tests_for_root(test t, list *roots) {

  cons *c;
  complex_test *ct;
  Symbol *referent;

  /* Gather variables from test. */

  if (test_is_blank_test(t)) return FALSE;

  if (test_is_blank_or_equality_test(t)) {
    referent = referent_of_equality_test(t);
    if ((referent->common.symbol_type==VARIABLE_SYMBOL_TYPE) &&
        member_of_list(referent,roots)) return TRUE;
    return FALSE;
  }
  ct = complex_test_from_test(t);

  switch (ct->type) {
  case GOAL_ID_TEST:
  case IMPASSE_ID_TEST:
  case DISJUNCTION_TEST:
    return FALSE;
    break;

  case CONJUNCTIVE_TEST:
    for (c=ct->data.conjunct_list; c!=NIL; c=c->rest)
      if (test_tests_for_root(static_cast<char *>(c->first), roots)) return TRUE;
    return FALSE;
    break;

  default:
    /* --- relational tests other than equality --- */
    referent = ct->data.referent;
    if ((referent->common.symbol_type==VARIABLE_SYMBOL_TYPE) &&
        member_of_list(referent,roots)) return TRUE;
    return FALSE;
    break;
  }
}

void remove_isa_state_tests_for_non_roots(agent* thisAgent, condition **lhs_top, 
										  condition **lhs_bottom, list *roots)
{
  condition *cond;
  Bool a,b;
  test temp;

  a = FALSE; 
  b = FALSE;

  for (cond = *lhs_top; cond != NIL; cond = cond->next) {
    if ((cond->type == POSITIVE_CONDITION) &&
        (test_is_complex_test(cond->data.tests.id_test)) &&
        (test_includes_goal_or_impasse_id_test (cond->data.tests.id_test,
                                                TRUE, FALSE)) &&
        (!test_tests_for_root(cond->data.tests.id_test, roots))) {
      temp = cond->data.tests.id_test;
      cond->data.tests.id_test =
        copy_test_removing_goal_impasse_tests (thisAgent, temp,&a,&b);
      deallocate_test (thisAgent, temp); /* RBD fixed memory leak 3/29/95 */
    }
  }
}

Bool reorder_lhs (agent* thisAgent, condition **lhs_top, 
				  condition **lhs_bottom, Bool reorder_nccs) {
  tc_number tc;
  list *roots;

  tc = get_new_tc_number (thisAgent);
  /* don't mark any variables, since nothing is bound outside the LHS */

  roots = collect_root_variables (thisAgent, *lhs_top, tc, TRUE);


  /* SBH/MVP 6-24-94 Fix to include only root "STATE" test in the LHS of a chunk.*/
  if (roots) remove_isa_state_tests_for_non_roots(thisAgent, lhs_top,lhs_bottom,roots);

  /* MVP 6-8-94 - fix provided by Bob */
  if (!roots) {
    condition *cond;
 
    for (cond = *lhs_top; cond!=NIL; cond=cond->next) {
      if ((cond->type == POSITIVE_CONDITION) &&
          (test_includes_goal_or_impasse_id_test (cond->data.tests.id_test,
            TRUE, FALSE))) {
        add_bound_variables_in_test (thisAgent, cond->data.tests.id_test, tc, &roots);
        if (roots) break;
      }
    }
  }

  if (!roots) {
    print (thisAgent, "Error:  in production %s,\n", thisAgent->name_of_production_being_reordered);
    print (thisAgent, "        The LHS has no roots.\n");
    /* hmmm... most people aren't going to understand this error message */
    return FALSE;
  }

  fill_in_vars_requiring_bindings (thisAgent, *lhs_top, tc);
  reorder_condition_list (thisAgent, lhs_top, lhs_bottom, roots, tc, reorder_nccs);
  remove_vars_requiring_bindings (thisAgent, *lhs_top);
  free_list (thisAgent, roots);
  return TRUE;
}

void init_reorderer (agent* thisAgent) {  /* called from init_production_utilities() */
  init_memory_pool (thisAgent, &thisAgent->saved_test_pool, sizeof(saved_test), "saved test");
}

