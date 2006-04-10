#ifdef HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H
#include "portability.h"

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/*************************************************************************
 *
 *  file:  osupport.cpp
 *
 * =======================================================================
 * Calculate_support_for_instantiation_preferences() does run-time o-support
 * calculations -- it fills in pref->o_supported in each pref. on the
 * instantiation.  Calculate_compile_time_o_support() does the compile-time
 * version:  it takes the LHS and RHS, and fills in the a->support field in 
 * each RHS action with either UNKNOWN_SUPPORT, O_SUPPORT, or I_SUPPORT.
 * =======================================================================
 */


/* =========================================================================
             O Support calculation routines.
   ========================================================================= */

#include "osupport.h"
#include "symtab.h"
#include "wmem.h"
#include "gdatastructs.h"
#include "agent.h"
#include "kernel.h"
#include "production.h"
#include "instantiations.h"
#include "rhsfun.h"
#include "print.h"
#include "reorder.h"
#include "rete.h"
#include "gski_event_system_functions.h" // for XML trace output

/* -----------------------------------------------------------------------
                  O-Support Transitive Closure Routines

   These routines are used by the o-support calculations to mark transitive
   closures through TM (= WM+PM) plus (optionally) the RHS-generated pref's.

   The caller should first call begin_os_tc (rhs_prefs_or_nil).  Then
   add_to_os_tc (id) should be called any number of times to add stuff
   to the TC.  (Note that the rhs_prefs shouldn't be modified between the
   begin_os_tc() call and the last add_to_os_tc() call.)

   Each identifier in the TC is marked with id.tc_num=o_support_tc; the
   caller can check for TC membership by looking at id.tc_num on any id.
----------------------------------------------------------------------- */

/* This prototype is needed by the following macros. */
void add_to_os_tc (agent* thisAgent, Symbol *id, Bool isa_state);

/*#define add_to_os_tc_if_needed(sym) \
  { if ((sym)->common.symbol_type==IDENTIFIER_SYMBOL_TYPE) \
      add_to_os_tc (sym,FALSE); }*/
inline void add_to_os_tc_if_needed(agent* thisAgent, Symbol * sym)
{
  if ((sym)->common.symbol_type==IDENTIFIER_SYMBOL_TYPE)
    add_to_os_tc (thisAgent, sym,FALSE);
}

/*#define add_to_os_tc_if_id(sym,flag) \
  { if ((sym)->common.symbol_type==IDENTIFIER_SYMBOL_TYPE) \
      add_to_os_tc (sym,flag); }*/
inline void add_to_os_tc_if_id(agent* thisAgent, Symbol * sym, Bool flag)
{
  if ((sym)->common.symbol_type==IDENTIFIER_SYMBOL_TYPE) \
    add_to_os_tc (thisAgent, sym,flag);
}

/* SBH 4/14/93
 * For NNPSCM, we must exclude the operator slot from the transitive closure of a state.
 * Do that by passing a boolean argument, "isa_state" to this routine.
 * If it isa_state, check for the operator slot before the recursive call.
 */

void add_to_os_tc (agent* thisAgent, Symbol *id, Bool isa_state) {
  slot *s;
  preference *pref;
  wme *w;

  /* --- if id is already in the TC, exit; else mark it as in the TC --- */
  if (id->id.tc_num==thisAgent->o_support_tc) return;
  id->id.tc_num = thisAgent->o_support_tc;
  
  /* --- scan through all preferences and wmes for all slots for this id --- */
  for (w=id->id.input_wmes; w!=NIL; w=w->next)
    add_to_os_tc_if_needed (thisAgent, w->value);
  for (s=id->id.slots; s!=NIL; s=s->next) {
    if ((!isa_state) || (s->attr != thisAgent->operator_symbol)) {
      for (pref=s->all_preferences; pref!=NIL; pref=pref->all_of_slot_next) {
	add_to_os_tc_if_needed (thisAgent, pref->value);
	if (preference_is_binary(pref->type))
	  add_to_os_tc_if_needed (thisAgent, pref->referent);
      }
      for (w=s->wmes; w!=NIL; w=w->next)
	add_to_os_tc_if_needed (thisAgent, w->value);
    }
  } /* end of for slots loop */
  /* --- now scan through RHS prefs and look for any with this id --- */
  for (pref=thisAgent->rhs_prefs_from_instantiation; pref!=NIL; pref=pref->inst_next) {
    if (pref->id==id) {
    if ((!isa_state) || (pref->attr != thisAgent->operator_symbol)) {
      add_to_os_tc_if_needed (thisAgent, pref->value);
      if (preference_is_binary(pref->type))
        add_to_os_tc_if_needed (thisAgent, pref->referent);
    }
  }
  }
  /* We don't need to worry about goal/impasse wmes here, since o-support tc's
     never start there and there's never a pointer to a goal or impasse from
     something else. */
}

void begin_os_tc (agent* thisAgent, preference *rhs_prefs_or_nil) {
  thisAgent->o_support_tc = get_new_tc_number(thisAgent);
  thisAgent->rhs_prefs_from_instantiation = rhs_prefs_or_nil;
}

/* -----------------------------------------------------------------------
           Utilities for Testing Inclusion in the O-Support TC

   After a TC has been marked with the above routine, these utility
   routines are used for checking whether certain things are in the TC.
   Test_has_id_in_os_tc() checks whether a given test contains an equality
   test for any identifier in the TC, other than the identifier
   "excluded_sym".  Id_or_value_of_condition_list_is_in_os_tc() checks whether
   any id or value test in the given condition list (including id/value tests
   inside NCC's) has a test for an id in the TC.  In the case of value tests,
   the id is not allowed to be "sym_excluded_from_value".
----------------------------------------------------------------------- */

Bool test_has_id_in_os_tc (agent* thisAgent, test t, Symbol *excluded_sym) {
  cons *c;
  Symbol *referent;
  complex_test *ct;

  if (test_is_blank_test(t)) return FALSE;
  if (test_is_blank_or_equality_test(t)) {
    referent = referent_of_equality_test(t);
    if (referent->common.symbol_type==IDENTIFIER_SYMBOL_TYPE)
      if (referent->id.tc_num==thisAgent->o_support_tc)
        if (referent!=excluded_sym)
          return TRUE;
    return FALSE;
  }
  ct = complex_test_from_test(t);
  if (ct->type==CONJUNCTIVE_TEST) {
    for (c=ct->data.conjunct_list; c!=NIL; c=c->rest)
      if (test_has_id_in_os_tc (thisAgent, static_cast<char *>(c->first), excluded_sym)) return TRUE;
    return FALSE;
  }
  return FALSE;
}

Bool id_or_value_of_condition_list_is_in_os_tc (agent* thisAgent, condition *conds,
                 Symbol *sym_excluded_from_value,
                 Symbol *match_state_to_exclude_test_of_the_operator_off_of) {
  /* RBD 8/19/94 Under NNPSCM, when we use this routine to look for "something
     off the state", we want to exclude tests of (match_state ^operator _). */
  for ( ; conds!=NIL; conds=conds->next) {
    switch (conds->type) {
    case POSITIVE_CONDITION:
    case NEGATIVE_CONDITION:
      if (test_includes_equality_test_for_symbol (conds->data.tests.id_test,
                       match_state_to_exclude_test_of_the_operator_off_of) &&
          test_includes_equality_test_for_symbol (conds->data.tests.attr_test,
                                           thisAgent->operator_symbol))
        break;
      if (test_has_id_in_os_tc (thisAgent, conds->data.tests.id_test, NIL))
        return TRUE;
      if (test_has_id_in_os_tc (thisAgent, conds->data.tests.value_test,
                                sym_excluded_from_value))
        return TRUE;
      break;
    case CONJUNCTIVE_NEGATION_CONDITION:
      if (id_or_value_of_condition_list_is_in_os_tc (thisAgent, conds->data.ncc.top,
                                              sym_excluded_from_value
                     , match_state_to_exclude_test_of_the_operator_off_of
                                              ))
        return TRUE;
      break;
    }
  }
  return FALSE;
}

/* -----------------------------------------------------------------------

   is_state_id

   GAP 10-6-94

   This routine checks to see if the identifier is one of the context
   objects i.e. it is the state somewhere in the context stack.
   This is used to ensure that O-support is not given to context objects 
   in super-states.

----------------------------------------------------------------------- */
Bool is_state_id(agent* thisAgent, Symbol *sym,Symbol *match_state)
{
  Symbol *c;
  
  for(c = thisAgent->top_goal; c != match_state; c = c->id.lower_goal) {
    if (sym == c)
      return TRUE;
  }

  if (sym == match_state)
    return TRUE;
  else
    return FALSE;
}

/* -----------------------------------------------------------------------
                    Run-Time O-Support Calculation

   This routine calculates o-support for each preference for the given
   instantiation, filling in pref->o_supported (TRUE or FALSE) on each one.

   The following predicates are used for support calculations.  In the
   following, "lhs has some elt. ..." means the lhs has some id or value
   at any nesting level.

     lhs_oa_support:
       (1) does lhs test (match_goal ^operator match_operator NO) ?
       (2) mark TC (match_operator) using TM;
           does lhs has some elt. in TC but != match_operator ?
       (3) mark TC (match_state) using TM;
           does lhs has some elt. in TC ?
     lhs_oc_support:
       (1) mark TC (match_state) using TM;
           does lhs has some elt. in TC but != match_state ?
     lhs_om_support:
       (1) does lhs tests (match_goal ^operator) ?
       (2) mark TC (match_state) using TM;
           does lhs has some elt. in TC but != match_state ?

     rhs_oa_support:
       mark TC (match_state) using TM+RHS;
       if pref.id is in TC, give support
     rhs_oc_support:
       mark TC (inst.rhsoperators) using TM+RHS;
       if pref.id is in TC, give support
     rhs_om_support:
       mark TC (inst.lhsoperators) using TM+RHS;
       if pref.id is in TC, give support

   BUGBUG the code does a check of whether the lhs tests the match state via
          looking just at id and value fields of top-level positive cond's.
          It doesn't look at the attr field, or at any negative or NCC's.
          I'm not sure whether this is right or not.  (It's a pretty
          obscure case, though.)
----------------------------------------------------------------------- */

/* RBD 8/91/94 changed calls to add_to_os_tc() in this routine to use
   add_to_os_tc_if_id() instead -- in case people use constant-symbols 
   (instead of objects) for states or operators */

void calculate_support_for_instantiation_preferences (agent* thisAgent, instantiation *inst) {
  Symbol *match_goal, *match_state, *match_operator;
  wme *match_operator_wme;
  Bool lhs_tests_operator_installed;
  Bool lhs_tests_operator_acceptable_or_installed;
  Bool lhs_is_known_to_test_something_off_match_state;
  Bool lhs_is_known_to_test_something_off_match_operator;
  Bool rhs_does_an_operator_creation;
  Bool oc_support_possible;
  Bool om_support_possible;
  Bool oa_support_possible;
  preference *rhs, *pref;
  wme *w;
  condition *lhs, *c;


/* RCHONG: begin 10.11 */

  action    *act;
  Bool      o_support,op_elab;
  Bool      operator_proposal;
  char      action_attr[50];
  int       pass;
  wme       *lowest_goal_wme;

/* RCHONG: end 10.11 */



/* REW: begin 09.15.96 */

  if (thisAgent->operand2_mode == TRUE) {

      if (thisAgent->soar_verbose_flag == TRUE) {
        printf("\n      in calculate_support_for_instantiation_preferences:");
        GenerateVerboseXML(thisAgent, "in calculate_support_for_instantiation_preferences:");
      }
     o_support = FALSE;
	 op_elab = FALSE;

     if (inst->prod->declared_support == DECLARED_O_SUPPORT)
	o_support = TRUE;
     else if (inst->prod->declared_support == DECLARED_I_SUPPORT)
	o_support = FALSE;
     else if (inst->prod->declared_support == UNDECLARED_SUPPORT) {

	/*
        check if the instantiation is proposing an operator.  if it
        is, then this instantiation is i-supported.
	*/

        operator_proposal = FALSE;
        for (act = inst->prod->action_list; act != NIL ; act = act->next) {
           if ((act->type == MAKE_ACTION)  &&
	       (rhs_value_is_symbol(act->attr))) {
	      if ((strcmp(rhs_value_to_string (thisAgent, act->attr, action_attr, 50),
			  "operator") == NIL) &&
		  (act->preference_type == ACCEPTABLE_PREFERENCE_TYPE)) {
		/* REW: 09.30.96.  Bug fix (next line was
		   operator_proposal == TRUE;) */
		 operator_proposal = TRUE;
		 o_support = FALSE;
		 break;
	      }
	   }
	}


        if (operator_proposal == FALSE) {

	   /*
	   an operator wasn't being proposed, so now we need to test if
	   the operator is being tested on the LHS.

	   i'll need to make two passes over the wmes that pertain to
	   this instantiation.  the first pass looks for the lowest goal
	   identifier.  the second pass looks for a wme of the form:

	      (<lowest-goal-id> ^operator ...)

	   if such a wme is found, then this o-support = TRUE; FALSE otherwise.

	   this code is essentially identical to that in
	   p_node_left_addition() in rete.c.

	   BUGBUG this check only looks at positive conditions.  we
	   haven't really decided what testing the absence of the
	   operator will do.  this code assumes that such a productions
	   (instantiation) would get i-support.
	   */

	   lowest_goal_wme = NIL;

	   for (pass = 0; pass != 2; pass++) {

	      for (c=inst->top_of_instantiated_conditions; c!=NIL; c=c->next) {
		 if (c->type==POSITIVE_CONDITION) {
		    w = c->bt.wme_;

		    if (pass == 0) {

		       if (w->id->id.isa_goal == TRUE) {

			  if (lowest_goal_wme == NIL)
			     lowest_goal_wme = w;

			  else {
			     if (w->id->id.level > lowest_goal_wme->id->id.level)
				lowest_goal_wme = w;
			  }
		       }

		    }

			else {
				if ((w->attr == thisAgent->operator_symbol) &&
					(w->acceptable == FALSE) &&
					(w->id == lowest_goal_wme->id)) {
 					if (thisAgent->o_support_calculation_type == 3 || thisAgent->o_support_calculation_type == 4) {

					/* iff RHS has only operator elaborations 
				    	then it's IE_PROD, otherwise PE_PROD, so
						look for non-op-elabs in the actions  KJC 1/00 */
						for (act = inst->prod->action_list;
						act != NIL ; act = act->next) {
							if (act->type == MAKE_ACTION) {
								if ((rhs_value_is_symbol(act->id)) &&
									(rhs_value_to_symbol(act->id) == w->value)) {
									op_elab = TRUE;
                                } else if ( thisAgent->o_support_calculation_type == 4 
										&& (rhs_value_is_reteloc(act->id)) 
										&& w->value == get_symbol_from_rete_loc( (unsigned short)rhs_value_to_reteloc_levels_up( act->id ), (byte)rhs_value_to_reteloc_field_num( act->id ), inst->rete_token, w )) {
								    op_elab = TRUE;
                                } else {
                                    /* this is not an operator elaboration */
                                    o_support = TRUE;
								}
							}
						}
					} else {					
						o_support = TRUE;
						break;
					}
				}
			}



		 }
	      }
	   }
	}
     }


    /* KJC 01/00: Warn if operator elabs mixed w/ applications */
    if ( (thisAgent->o_support_calculation_type == 3 
			|| thisAgent->o_support_calculation_type == 4 ) 
			&& (o_support == TRUE)) {

        if (op_elab == TRUE ) {

			/* warn user about mixed actions */
			if ( thisAgent->o_support_calculation_type == 3 ) {

				print_with_symbols(thisAgent, "\nWARNING:  operator elaborations mixed with operator applications\nget o_support in prod %y", inst->prod->name);
				
				growable_string gs = make_blank_growable_string(thisAgent);
				add_to_growable_string(thisAgent, &gs, "WARNING:  operator elaborations mixed with operator applications\nget o_support in prod ");
				add_to_growable_string(thisAgent, &gs, symbol_to_string(thisAgent, inst->prod->name, true, 0, 0));
				GenerateWarningXML(thisAgent, text_of_growable_string(gs));
				free_growable_string(thisAgent, gs);

				o_support = TRUE;
			} else if ( thisAgent->o_support_calculation_type == 4 ) {
				print_with_symbols(thisAgent, "\nWARNING:  operator elaborations mixed with operator applications\nget i_support in prod %y", inst->prod->name);

				growable_string gs = make_blank_growable_string(thisAgent);
				add_to_growable_string(thisAgent, &gs, "WARNING:  operator elaborations mixed with operator applications\nget i_support in prod ");
				add_to_growable_string(thisAgent, &gs, symbol_to_string(thisAgent, inst->prod->name, true, 0, 0));
				GenerateWarningXML(thisAgent, text_of_growable_string(gs));
				free_growable_string(thisAgent, gs);

				o_support = FALSE;
			}
		}
    }

	 /*
     assign every preference the correct support
     */  

     for (pref=inst->preferences_generated; pref!=NIL; pref=pref->inst_next)
       pref->o_supported = o_support;

	 goto o_support_done;
  }

/* REW: end   09.15.96 */


  /* --- initialize by giving everything NO o_support --- */  
  for (pref=inst->preferences_generated; pref!=NIL; pref=pref->inst_next)
    pref->o_supported = FALSE;

  /* --- find the match goal, match state, and match operator --- */
  match_goal = inst->match_goal;
  if (!match_goal) goto o_support_done;  /* nothing gets o-support */

  match_state = match_goal;

  match_operator_wme = match_goal->id.operator_slot->wmes;
  if (match_operator_wme)
    match_operator = match_operator_wme->value;
  else
    match_operator = NIL;

  lhs = inst->top_of_instantiated_conditions;
  rhs = inst->preferences_generated;
  
  /* --- scan through rhs to look for various things --- */
  rhs_does_an_operator_creation = FALSE;  

  for (pref=rhs; pref!=NIL; pref=pref->inst_next) {
    if ((pref->id==match_goal) &&
        (pref->attr==thisAgent->operator_symbol) &&
        ((pref->type==ACCEPTABLE_PREFERENCE_TYPE) ||
         (pref->type==REQUIRE_PREFERENCE_TYPE)) )
      rhs_does_an_operator_creation = TRUE;
  }

  /* --- scan through lhs to look for various tests --- */
  lhs_tests_operator_acceptable_or_installed = FALSE;
  lhs_tests_operator_installed = FALSE;
  lhs_is_known_to_test_something_off_match_state = FALSE;
  lhs_is_known_to_test_something_off_match_operator = FALSE;

  for (c=lhs; c!=NIL; c=c->next) {
    if (c->type!=POSITIVE_CONDITION) continue;
    w = c->bt.wme_;
    /* For NNPSCM, count something as "off the match state" only
       if it's not the OPERATOR. */
    if ((w->id==match_state) && (w->attr != thisAgent->operator_symbol))
      lhs_is_known_to_test_something_off_match_state = TRUE;
    if (w->id==match_operator)
      lhs_is_known_to_test_something_off_match_operator = TRUE;
    if (w==match_operator_wme) lhs_tests_operator_installed = TRUE;
    if ((w->id==match_goal)&&(w->attr==thisAgent->operator_symbol))
      lhs_tests_operator_acceptable_or_installed = TRUE;
  }

  /* --- calcluate lhs support flags --- */
  oa_support_possible = lhs_tests_operator_installed;
  oc_support_possible = rhs_does_an_operator_creation; 
  om_support_possible = lhs_tests_operator_acceptable_or_installed;

  if ((!oa_support_possible)&&(!oc_support_possible)&&(!om_support_possible))
    goto o_support_done;

  if (! lhs_is_known_to_test_something_off_match_state) {
    begin_os_tc (thisAgent, NIL);
    add_to_os_tc_if_id (thisAgent, match_state, TRUE);
    if (! id_or_value_of_condition_list_is_in_os_tc (thisAgent, lhs, match_state,
                                                     match_state)) {
      oc_support_possible = FALSE;
      om_support_possible = FALSE;
    }
  }

  if (oa_support_possible) {
    if (! lhs_is_known_to_test_something_off_match_operator) {
      begin_os_tc (thisAgent, NIL);
      add_to_os_tc_if_id (thisAgent, match_operator,FALSE);
      if (! id_or_value_of_condition_list_is_in_os_tc (thisAgent, lhs, match_operator,
                                                       NIL))
        oa_support_possible = FALSE;
    }
  }

  /* --- look for rhs oa support --- */
  if (oa_support_possible) {
    begin_os_tc (thisAgent, rhs);
    add_to_os_tc_if_id (thisAgent, match_state,TRUE);
    for (pref=rhs; pref!=NIL; pref=pref->inst_next) {
      if (pref->id->id.tc_num==thisAgent->o_support_tc)
        /* RBD 8/19/94 added extra NNPSCM test -- ^operator augs on the state
                                                  don't get o-support */
/* AGR 639 begin 94.11.01 */
	/* gap 10/6/94 You need to check the id on all preferences that have
	   an attribute of operator to see if this is an operator slot of a
	   context being modified. */
	if (!((pref->attr == thisAgent->operator_symbol) && 
	    (is_state_id(thisAgent, pref->id,match_state))))
/* AGR 639 end */
        pref->o_supported = TRUE;
    }
  }

  /* --- look for rhs oc support --- */
  if (oc_support_possible) {
    begin_os_tc (thisAgent, rhs);
    for (pref=rhs; pref!=NIL; pref=pref->inst_next) {
      if ((pref->id==match_goal) &&
          (pref->attr==thisAgent->operator_symbol) &&
          ((pref->type==ACCEPTABLE_PREFERENCE_TYPE) ||
           (pref->type==REQUIRE_PREFERENCE_TYPE)) ) {
          add_to_os_tc_if_id (thisAgent, pref->value,FALSE);
      }
    }
    for (pref=rhs; pref!=NIL; pref=pref->inst_next) {
      /* SBH 6/23/94 */
      if ((pref->id->id.tc_num==thisAgent->o_support_tc) &&
	  (pref->id != match_state))
	/* SBH: Added 2nd test to avoid circular assignment of o-support
	   to augmentations of the state: in, e.g.
	   (sp p2
             (state <g> ^problem-space)(state <ig> ^problem-space.name top-ps)
	      -->
	      (<g> ^operator <o>)(<o> ^name opx ^circular-goal-test <ig>))
	   Here, the op acc. pref would get o-support (it's in the transitive
	   closure); this test rules it out.
	   
	   BUGBUG: this is not fully general; it does not rule out assiging
	   o-support to substructures of the state that are in the TC of an
	   operator creation; e.g.
	   (sp p2
             (state <g> ^problem-space)(state <ig> ^problem-space.name top-ps)
	      -->
	      (<g> ^operator <o> ^other <x>)
	      (<o> ^name opx ^circular-goal-test <ig>)
	      (<x> ^THIS-GETS-O-SUPPORT T))
	 */
      /* end SBH 6/23/94 */
        pref->o_supported = TRUE;
    }
  }
  
  /* --- look for rhs om support --- */
  if (om_support_possible) {
    begin_os_tc (thisAgent, rhs);
    for (c=inst->top_of_instantiated_conditions; c!=NIL; c=c->next)
      if (c->type==POSITIVE_CONDITION) {
        w = c->bt.wme_;
        if ((w->id==match_goal) && (w->attr==thisAgent->operator_symbol))
          add_to_os_tc_if_id (thisAgent, w->value, FALSE);
      }
    for (pref=rhs; pref!=NIL; pref=pref->inst_next)
      if (pref->id->id.tc_num==thisAgent->o_support_tc)
        pref->o_supported = TRUE;
  }

  o_support_done:  {}
}

/* -----------------------------------------------------------------------
            Run-Time O-Support Calculation:  Doug Pearson's Scheme

   This routine calculates o-support for each preference for the given
   instantiation, filling in pref->o_supported (TRUE or FALSE) on each one.

   This is basically Doug's original scheme (from email August 16, 1994)
   modified by John's response (August 17) points #2 (don't give o-c
   support unless pref in TC of RHS op.) and #3 (all support calc's should
   be local to a goal).  In detail:

   For a particular preference p=(id ^attr ...) on the RHS of an
   instantiation [LHS,RHS]:

   RULE #1 (Context pref's): If id is the match state and attr="operator", 
   then p does NOT get o-support.  This rule overrides all other rules.

   RULE #2 (O-A support):  If LHS includes (match-state ^operator ...),
   then p gets o-support.

   RULE #3 (O-M support):  If LHS includes (match-state ^operator ... +),
   then p gets o-support.

   RULE #4 (O-C support): If RHS creates (match-state ^operator ... +/!),
   and p is in TC(RHS-operators, RHS), then p gets o-support.

   Here "TC" means transitive closure; the starting points for the TC are 
   all operators the RHS creates an acceptable/require preference for (i.e., 
   if the RHS includes (match-state ^operator such-and-such +/!), then 
   "such-and-such" is one of the starting points for the TC).  The TC
   is computed only through the preferences created by the RHS, not
   through any other existing preferences or WMEs.

   If none of rules 1-4 apply, then p does NOT get o-support.

   Note that rules 1 through 3 can be handled in linear time (linear in 
   the size of the LHS and RHS); rule 4 can be handled in time quadratic 
   in the size of the RHS (and typical behavior will probably be linear).
----------------------------------------------------------------------- */

void dougs_calculate_support_for_instantiation_preferences (agent* thisAgent, instantiation *inst) {
  Symbol *match_state;
  Bool rule_2_or_3, anything_added;
  preference *rhs, *pref;
  wme *w;
  condition *lhs, *c;

  lhs = inst->top_of_instantiated_conditions;
  rhs = inst->preferences_generated;
  match_state = inst->match_goal;

  /* --- First, check whether rule 2 or 3 applies. --- */
  rule_2_or_3 = FALSE;
  for (c=lhs; c!=NIL; c=c->next) {
    if (c->type!=POSITIVE_CONDITION) continue;
    w = c->bt.wme_;
    if ((w->id==match_state)&&(w->attr==thisAgent->operator_symbol)) {
      rule_2_or_3 = TRUE;
      break;
    }
  }
  
  /* --- Initialize all pref's according to rules 2 and 3 --- */  
  for (pref=rhs; pref!=NIL; pref=pref->inst_next)
    pref->o_supported = rule_2_or_3;

  /* --- If they didn't apply, check rule 4 --- */
  if (! rule_2_or_3) {
    thisAgent->o_support_tc = get_new_tc_number(thisAgent);
    /* BUGBUG With Doug's scheme, o_support_tc no longer needs to be a
       global variable -- it could simply be local to this procedure */
    anything_added = FALSE;
    /* --- look for RHS operators, add 'em (starting points) to the TC --- */
    for (pref=rhs; pref!=NIL; pref=pref->inst_next) {
      if ((pref->id==match_state) &&
          (pref->attr==thisAgent->operator_symbol) &&
          ((pref->type==ACCEPTABLE_PREFERENCE_TYPE) ||
           (pref->type==REQUIRE_PREFERENCE_TYPE)) &&
          (pref->value->common.symbol_type==IDENTIFIER_SYMBOL_TYPE)) {
        pref->value->id.tc_num = thisAgent->o_support_tc;
        anything_added = TRUE;
      }
    }
    /* --- Keep adding stuff to the TC until nothing changes anymore --- */
    while (anything_added) {
      anything_added = FALSE;
      for (pref=rhs; pref!=NIL; pref=pref->inst_next) {
        if (pref->id->id.tc_num != thisAgent->o_support_tc) continue;
        if (pref->o_supported) continue; /* already added this thing */
        pref->o_supported = TRUE;
        anything_added = TRUE;
        if (pref->value->common.symbol_type==IDENTIFIER_SYMBOL_TYPE)
          pref->value->id.tc_num = thisAgent->o_support_tc;
        if ((preference_is_binary(pref->type)) &&
            (pref->referent->common.symbol_type==IDENTIFIER_SYMBOL_TYPE))
          pref->referent->id.tc_num = thisAgent->o_support_tc;
      }
    }
  }

  /* --- Finally, use rule 1, which overrides all the other rules. --- */
  for (pref=rhs; pref!=NIL; pref=pref->inst_next)
    if ((pref->id==match_state) &&
        (pref->attr==thisAgent->operator_symbol))
      pref->o_supported = FALSE;
}

/* *********************************************************************

                   Compile-Time O-Support Calculations

********************************************************************* */

/* ------------------------------------------------------------------
                         Test Is For Symbol

   This function determines whether a given symbol could be the match
   for a given test.  It returns YES if the symbol is the only symbol
   that could pass the test (i.e., the test *forces* that symbol to be
   present in WM), NO if the symbol couldn't possibly pass the test,
   and MAYBE if it can't tell for sure.  The symbol may be a variable;
   the test may contain variables.
------------------------------------------------------------------ */

typedef enum yes_no_maybe_enum { YES, NO, MAYBE } yes_no_maybe;

yes_no_maybe test_is_for_symbol (test t, Symbol *sym) {
  cons *c;
  yes_no_maybe temp;
  Bool maybe_found;
  complex_test *ct;
  Symbol *referent;

  if (test_is_blank_test(t)) return MAYBE;

  if (test_is_blank_or_equality_test(t)) {
    referent = referent_of_equality_test(t);
    if (referent==sym) return YES;
    if (referent->common.symbol_type==VARIABLE_SYMBOL_TYPE) return MAYBE;
    if (sym->common.symbol_type==VARIABLE_SYMBOL_TYPE) return MAYBE;
    return NO;
  }

  ct = complex_test_from_test(t);
  
  switch (ct->type) {
  case DISJUNCTION_TEST:
    if (sym->common.symbol_type==VARIABLE_SYMBOL_TYPE) return MAYBE;
    if (member_of_list (sym, ct->data.disjunction_list)) return MAYBE;
    return NO;
  case CONJUNCTIVE_TEST:
    maybe_found = FALSE;
    for (c=ct->data.conjunct_list; c!=NIL; c=c->rest) {
      temp = test_is_for_symbol (static_cast<char *>(c->first), sym);
      if (temp==YES) return YES;
      if (temp==MAYBE) maybe_found = TRUE;
    }
    if (maybe_found) return MAYBE;
    return NO;
  default:  /* goal/impasse tests, relational tests other than equality */
    return MAYBE;
  }
}

/* ------------------------------------------------------------------
                         Find Known Goals

   This routine looks at the LHS and returns a list of variables that
   are certain to be bound to goals.

   Note:  this uses the TC routines and clobbers any existing TC.
                         
   BUGBUG should follow ^object links up the goal stack if possible
------------------------------------------------------------------ */

list *find_known_goals (agent* thisAgent, condition *lhs) {
  tc_number tc;
  list *vars;
  condition *c;

  tc = get_new_tc_number(thisAgent);
  vars = NIL;
  for (c=lhs; c!=NIL; c=c->next) {
    if (c->type != POSITIVE_CONDITION) continue;
    if (test_includes_goal_or_impasse_id_test (c->data.tests.id_test,
                                               TRUE,
                                               FALSE))
      add_bound_variables_in_test (thisAgent, c->data.tests.id_test, tc, &vars);
  }
  return vars;
}

/* ------------------------------------------------------------------
                  Find Compile Time Match Goal

   Given the LHS and a list of known goals (i.e., variables that must
   be bound to goals at run-time), this routine tries to determine
   which variable will be the match goal.  If successful, it returns
   that variable; if it can't tell which variable will be the match
   goal, it returns NIL.

   Note:  this uses the TC routines and clobbers any existing TC.
------------------------------------------------------------------ */

Symbol *find_compile_time_match_goal (agent* thisAgent, condition *lhs, list *known_goals) {
  tc_number tc;
  list *roots;
  list *root_goals;
  int num_root_goals;
  cons *c, *prev_c, *next_c;
  Symbol *result;
  condition *cond;
  
  /* --- find root variables --- */
  tc = get_new_tc_number(thisAgent);
  roots = collect_root_variables (thisAgent, lhs, tc, FALSE);
  
  /* --- intersect roots with known_goals, producing root_goals --- */
  root_goals = NIL;
  num_root_goals = 0;
  for (c=roots; c!=NIL; c=c->rest)
    if (member_of_list (c->first, known_goals)) {
      push (thisAgent, c->first, root_goals);
      num_root_goals++;
    }
  free_list (thisAgent, roots);

  /* --- if more than one goal, remove any with "^object nil" --- */
  if (num_root_goals > 1) {
    for (cond=lhs; cond!=NIL; cond=cond->next) {
      if ((cond->type==POSITIVE_CONDITION) &&
          (test_is_for_symbol(cond->data.tests.attr_test,thisAgent->superstate_symbol)==YES)&&
          (test_is_for_symbol(cond->data.tests.value_test,thisAgent->nil_symbol)==YES)) {
        prev_c = NIL;
        for (c=root_goals; c!=NIL; c=next_c) {
          next_c = c->rest;
          if (test_is_for_symbol (cond->data.tests.id_test, static_cast<symbol_union *>(c->first))==YES) {
            /* --- remove c from the root_goals list --- */
            if (prev_c) prev_c->rest = next_c; else root_goals = next_c;
            free_cons (thisAgent, c);
            num_root_goals--;
            if (num_root_goals==1) break; /* be sure not to remove them all */
          } else {
            prev_c = c;
          }
        } /* end of for (c) loop */
        if (num_root_goals==1) break; /* be sure not to remove them all */
      }
    } /* end of for (cond) loop */
  }
  
  /* --- if there's only one root goal, that's it! --- */
  if (num_root_goals==1)
    result = static_cast<symbol_union *>(root_goals->first);
  else
    result = NIL;

  /* --- clean up and return result --- */
  free_list (thisAgent, root_goals);
  return result;      
}

/* ------------------------------------------------------------------
                       Find Thing Off Goal

   Given the LHS and a the match goal variable, this routine looks
   for a positive condition testing (goal ^attr) for the given attribute
   "attr".  If such a condition exists, and the value field contains
   an equality test for a variable, then that variable is returned.
   (If more than one such variable exists, one is chosen arbitrarily
   and returned.)  Otherwise the function returns NIL.

   Note:  this uses the TC routines and clobbers any existing TC.
------------------------------------------------------------------ */

Symbol *find_thing_off_goal (agent* thisAgent, condition *lhs, 
							 Symbol *goal, Symbol *attr) {
  condition *c;
  list *vars;
  tc_number tc;
  Symbol *result;

  for (c=lhs; c!=NIL; c=c->next) {
    if (c->type != POSITIVE_CONDITION) continue;
    if (test_is_for_symbol (c->data.tests.id_test, goal) != YES) continue;
    if (test_is_for_symbol (c->data.tests.attr_test, attr) != YES) continue;
    if (c->test_for_acceptable_preference) continue;
    tc = get_new_tc_number(thisAgent);
    vars = NIL;
    add_bound_variables_in_test (thisAgent, c->data.tests.value_test, tc, &vars);
    if (vars) {
      result = static_cast<symbol_union *>(vars->first);
      free_list (thisAgent, vars);
      return result;
    }
  }
  return NIL;
}

/* ------------------------------------------------------------------
                 Condition List Has Id Test For Sym

   This checks whether a given condition list has an equality test for
   a given symbol in the id field of any condition (at any nesting level
   within NCC's).
------------------------------------------------------------------ */

Bool condition_list_has_id_test_for_sym (condition *conds, Symbol *sym) {
  for ( ; conds!=NIL; conds=conds->next) {
    switch (conds->type) {
    case POSITIVE_CONDITION:
    case NEGATIVE_CONDITION:
      if (test_includes_equality_test_for_symbol (conds->data.tests.id_test,
                                                  sym))
        return TRUE;
      break;
    case CONJUNCTIVE_NEGATION_CONDITION:
      if (condition_list_has_id_test_for_sym (conds->data.ncc.top, sym))
        return TRUE;
      break;
    }
  }
  return FALSE;
}


/* SBH 7/1/94 #2 */

/* ------------------------------------------------------------------

------------------------------------------------------------------ */

Bool match_state_tests_non_operator_slot (agent* thisAgent, condition *conds, 
										  Symbol *match_state) {
  yes_no_maybe ynm;

  for ( ; conds!=NIL; conds=conds->next) {
    switch (conds->type) {
    case POSITIVE_CONDITION:
    case NEGATIVE_CONDITION:
      if (test_includes_equality_test_for_symbol (conds->data.tests.id_test,
                                                  match_state)) {
	ynm = test_is_for_symbol (conds->data.tests.attr_test, thisAgent->operator_symbol);
	if (ynm == NO) return TRUE;
      }
      break;
    case CONJUNCTIVE_NEGATION_CONDITION:
      if (match_state_tests_non_operator_slot (thisAgent, conds->data.ncc.top, 
											   match_state))
        return TRUE;
      break;
    }
  }
  return FALSE;
}

/* end SBH 7/1/94 #2 */

/* ------------------------------------------------------------------
                      Add TC Through LHS and RHS

   This enlarges a given TC by adding to it any connected conditions
   in the LHS or actions in the RHS.
------------------------------------------------------------------ */

void add_tc_through_lhs_and_rhs (agent* thisAgent, condition *lhs, action *rhs, 
								 tc_number tc, list **id_list, list **var_list) {
  condition *c;
  action *a;
  Bool anything_changed;
  
  for (c=lhs; c!=NIL; c=c->next) c->already_in_tc = FALSE;
  for (a=rhs; a!=NIL; a=a->next) a->already_in_tc = FALSE;

  /* --- keep trying to add new stuff to the tc --- */  
  while (TRUE) {
    anything_changed = FALSE;
    for (c=lhs; c!=NIL; c=c->next)
      if (! c->already_in_tc)
        if (cond_is_in_tc (thisAgent, c, tc)) {
          add_cond_to_tc (thisAgent, c, tc, id_list, var_list);
          c->already_in_tc = TRUE;
          anything_changed = TRUE;
        }
    for (a=rhs; a!=NIL; a=a->next)
      if (! a->already_in_tc)
        if (action_is_in_tc (a, tc)) {
          add_action_to_tc (thisAgent, a, tc, id_list, var_list);
          a->already_in_tc = TRUE;
          anything_changed = TRUE;
        }
    if (! anything_changed) break;
  }
}

/* -----------------------------------------------------------------------
                   Calculate Compile Time O-Support

   This takes the LHS and RHS, and fills in the a->support field in each
   RHS action with either UNKNOWN_SUPPORT, O_SUPPORT, or I_SUPPORT.
   (Actually, it only does this for MAKE_ACTION's--for FUNCALL_ACTION's,
   the support doesn't matter.)
----------------------------------------------------------------------- */

void calculate_compile_time_o_support (agent* thisAgent, condition *lhs, 
									   action *rhs) {
  list *known_goals;
  cons *c;
  Symbol  *match_state, *match_operator;
  yes_no_maybe lhs_oa_support, lhs_oc_support, lhs_om_support;
  action *a;
  condition *cond;
  yes_no_maybe ynm;
  Bool operator_found, possible_operator_found;
  tc_number tc;

  /* --- initialize:  mark all rhs actions as "unknown" --- */
  for (a=rhs; a!=NIL; a=a->next)
    if (a->type==MAKE_ACTION) a->support=UNKNOWN_SUPPORT;

  /* --- if "operator" doesn't appear in any LHS attribute slot, and there
         are no RHS +/! makes for "operator", then nothing gets support --- */
  operator_found = FALSE;
  possible_operator_found = FALSE;
  for (cond=lhs; cond!=NIL; cond=cond->next) {
    if (cond->type != POSITIVE_CONDITION) continue;
    ynm = test_is_for_symbol (cond->data.tests.attr_test, thisAgent->operator_symbol);
    if (ynm==YES) { operator_found = possible_operator_found = TRUE; break; }
    if (ynm==MAYBE) possible_operator_found = TRUE;
  }
  if (! operator_found)
    for (a=rhs; a!=NIL; a=a->next) {
      if (a->type != MAKE_ACTION) continue;
      if (rhs_value_is_symbol(a->attr)) { /* RBD 3/29/95 general RHS attr's */
        Symbol *attr;
        attr = rhs_value_to_symbol(a->attr);
        if (attr==thisAgent->operator_symbol)
          { operator_found = possible_operator_found = TRUE; break; }
        if (attr->common.symbol_type==VARIABLE_SYMBOL_TYPE)
          possible_operator_found = TRUE;
      } else {
        possible_operator_found = TRUE; /* for funcall, must play it safe */
      }
    }
  if (! possible_operator_found) {
    for (a=rhs; a!=NIL; a=a->next) {
      if (a->type == MAKE_ACTION) a->support=I_SUPPORT;
    }
    return;
  }


  /* --- find known goals; RHS augmentations of goals get no support --- */
  known_goals = find_known_goals (thisAgent, lhs);
 /* SBH: In NNPSCM, the only RHS-goal augmentations that can't get support are
    preferences for the "operator" slot. */
  for (c=known_goals; c!=NIL; c=c->rest)
    for (a=rhs; a!=NIL; a=a->next) {
      if (a->type != MAKE_ACTION) continue;
      if (rhs_value_is_symbol(a->attr) &&  /* RBD 3/29/95 */
          rhs_value_to_symbol(a->attr)==thisAgent->operator_symbol &&
          (rhs_value_to_symbol(a->id) == c->first))
        a->support = I_SUPPORT;
    }

  /* --- find match goal, state, and operator --- */
  match_state = find_compile_time_match_goal (thisAgent, lhs, known_goals);
  free_list (thisAgent, known_goals);
  if (!match_state) return;
  match_operator = find_thing_off_goal (thisAgent, lhs, match_state, thisAgent->operator_symbol);
  /* --- If when checking (above) for "operator" appearing anywhere, we
     found a possible operator but not a definite operator, now go back and
     see if the possible operator was actually the match goal or match state;
     if so, it's not a possible operator.  (Note:  by "possible operator" I
     mean something appearing in the *attribute* field that might get bound
     to the symbol "operator".)  --- */
  if (possible_operator_found && !operator_found) {
    possible_operator_found = FALSE;
    for (cond=lhs; cond!=NIL; cond=cond->next) {
      if (cond->type != POSITIVE_CONDITION) continue;
      ynm = test_is_for_symbol (cond->data.tests.attr_test, thisAgent->operator_symbol);
      if ((ynm!=NO) &&
          (test_is_for_symbol (cond->data.tests.attr_test, match_state)!=YES))
        { possible_operator_found = TRUE; break; }
    }
    if (! possible_operator_found) {
      for (a=rhs; a!=NIL; a=a->next) {
        if (a->type != MAKE_ACTION) continue;
        /* we're looking for "operator" augs of goals only, and match_state
           couldn't get bound to a goal */
        if (rhs_value_to_symbol(a->id) == match_state) continue;
        if (rhs_value_is_symbol(a->attr)) { /* RBD 3/29/95 */
          Symbol *attr;
          attr = rhs_value_to_symbol(a->attr);
          if ((attr->common.symbol_type==VARIABLE_SYMBOL_TYPE) &&
              (attr != match_state))
            { possible_operator_found = TRUE; break; }
        } else { /* RBD 3/29/95 */
          possible_operator_found = TRUE; break;
        }
      }
    }
    if (! possible_operator_found) {
      for (a=rhs; a!=NIL; a=a->next)
        if (a->type == MAKE_ACTION) a->support=I_SUPPORT;
      return;
    }
  }
  
  /* --- calculate LHS support predicates --- */
  lhs_oa_support = MAYBE;
  if (match_operator)

/* SBH 7/1/94 #2 */
    if ((condition_list_has_id_test_for_sym (lhs, match_operator)) &&
	(match_state_tests_non_operator_slot(thisAgent, lhs,match_state)))
/* end SBH 7/1/94 #2 */

      lhs_oa_support = YES;

  lhs_oc_support = MAYBE;
  lhs_om_support = MAYBE;

/* SBH 7/1/94 #2 */
  /* For NNPSCM, must test that there is a test of a non-operator slot off 
     of the match_state. */
  if (match_state_tests_non_operator_slot(thisAgent, lhs,match_state)) 
    {
/* end SBH 7/1/94 #2 */

    lhs_oc_support = YES; 
    for (cond=lhs; cond!=NIL; cond=cond->next) {
      if (cond->type != POSITIVE_CONDITION) continue;
      if (test_is_for_symbol (cond->data.tests.id_test, match_state) != YES) continue;
      if (test_is_for_symbol (cond->data.tests.attr_test, thisAgent->operator_symbol)
          != YES)
        continue;
      lhs_om_support = YES;
      break;
    }
  }     

  if (lhs_oa_support == YES) {    /* --- look for RHS o-a support --- */
    /* --- do TC(match_state) --- */
    tc = get_new_tc_number(thisAgent);
    add_symbol_to_tc (thisAgent, match_state, tc, NIL, NIL);
    add_tc_through_lhs_and_rhs (thisAgent, lhs, rhs, tc, NIL, NIL);

    /* --- any action with id in the TC gets support --- */
    for (a=rhs; a!=NIL; a=a->next)  {

      if (action_is_in_tc (a, tc)) 
	/* SBH 7/1/94 Avoid resetting of support that was previously set to I_SUPPORT. */
	/* gap 10/6/94 If the action has an attribue of operator, then you
	   don't know if it should get o-support until run time because of
	   the vagaries of knowing when this is matching a context object
	   or not. */
        if (rhs_value_is_symbol(a->attr) &&
            (rhs_value_to_symbol(a->attr)==thisAgent->operator_symbol)) {
	  if (a->support != I_SUPPORT) a->support = UNKNOWN_SUPPORT;
	} else {
	  if (a->support != I_SUPPORT) a->support = O_SUPPORT;
	}
        /* end SBH 7/1/94 */
    }
  }
  
  if (lhs_oc_support == YES) {    /* --- look for RHS o-c support --- */
    /* --- do TC(rhs operators) --- */
    tc = get_new_tc_number(thisAgent);
    for (a=rhs; a!=NIL; a=a->next) {
      if (a->type != MAKE_ACTION) continue;
      if (
	  (rhs_value_to_symbol(a->id)==match_state) &&
          (rhs_value_is_symbol(a->attr)) &&
          (rhs_value_to_symbol(a->attr)==thisAgent->operator_symbol) &&
          ((a->preference_type==ACCEPTABLE_PREFERENCE_TYPE) ||
           (a->preference_type==REQUIRE_PREFERENCE_TYPE)) ) {
        if (rhs_value_is_symbol(a->value)) {
          add_symbol_to_tc (thisAgent, rhs_value_to_symbol(a->value), tc, NIL,NIL);
	}
      }
    }
    add_tc_through_lhs_and_rhs (thisAgent, lhs, rhs, tc, NIL, NIL);

    /* --- any action with id in the TC gets support --- */
    for (a=rhs; a!=NIL; a=a->next) 


      if (action_is_in_tc (a, tc)) {

	/* SBH 6/7/94:
	   Make sure the action is not already marked as "I_SUPPORT".  This
	   avoids giving o-support in the case where the operator
	   points back to the goal, thus adding the goal to the TC,
	   thus adding the operator proposal itself to the TC; thus
	   giving o-support to an operator proposal.
	*/
	if (a->support != I_SUPPORT) a->support = O_SUPPORT;
	/* End SBH 6/7/94 */


       /* REW: begin 09.15.96 */
       /*
       in operand, operator proposals are now only i-supported.
       */

       if (thisAgent->operand2_mode == TRUE) {
           if (thisAgent->soar_verbose_flag == TRUE) {
               printf("\n         operator creation: setting a->support to I_SUPPORT");
               GenerateVerboseXML(thisAgent, "operator creation: setting a->support to I_SUPPORT");
           }

	  a->support = I_SUPPORT;
       }
       /* REW: end   09.15.96 */

      }
  }

  if (lhs_om_support == YES) {    /* --- look for RHS o-m support --- */
    /* --- do TC(lhs operators) --- */
    tc = get_new_tc_number(thisAgent);
    for (cond=lhs; cond!=NIL; cond=cond->next) {
      if (cond->type != POSITIVE_CONDITION) continue;
      if (test_is_for_symbol (cond->data.tests.id_test, match_state) == YES)
        if (test_is_for_symbol (cond->data.tests.attr_test, thisAgent->operator_symbol)
            == YES)
          add_bound_variables_in_test (thisAgent, cond->data.tests.value_test, tc, NIL);
    }
    add_tc_through_lhs_and_rhs (thisAgent, lhs, rhs, tc, NIL, NIL);

    /* --- any action with id in the TC gets support --- */
    for (a=rhs; a!=NIL; a=a->next) 

      if (action_is_in_tc (a, tc)) {
	/* SBH 7/1/94 Avoid resetting of support that was previously set to I_SUPPORT. */
	if (a->support != I_SUPPORT) a->support = O_SUPPORT;
	/* end SBH 7/1/94 */
      }
  }
}

