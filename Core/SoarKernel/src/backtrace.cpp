#include <portability.h>

/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/*************************************************************************
 *
 *  file:  backtrace.cpp
 *
 * =======================================================================
 *  Backtracing structures and routines.  See also explain.c
 * =======================================================================
 */

/* ====================================================================
                        Backtracing routines
   ==================================================================== */

#include <stdlib.h>

#include "backtrace.h"
#include "mem.h"
#include "kernel.h"
#include "print.h"
#include "wmem.h"

#include "agent.h"
#include "instantiations.h"
#include "production.h"
#include "symtab.h"
#include "explain.h"
#include "recmem.h"
#include "xml.h"
#include "soar_TraceNames.h"
#include "test.h"
#include "debug.h"
#include "prefmem.h"
#include "variablization_manager.h"

using namespace soar_TraceNames;
//void dprint_condition (TraceMode mode, condition *cond, const char *indent_string);

//#define BACKTRACE_COMBINE_CONDITIONS
#define BACKTRACE_ADD_ALL_CONDITIONS
//#define BACKTRACE_ADD_ONLY_UNIQUE (not implemented yet, may not)

/* ====================================================================

                            Backtracing

   Four sets of conditions are maintained during backtracing:  locals,
   grounds, positive potentials, and negateds.  Negateds are really
   potentials, but we keep them separately throughout backtracing, and
   ground them at the very end.  Note that this means during backtracing,
   the grounds, positive potentials, and locals are all instantiated
   top-level positive conditions, so they all have a bt.wme_ on them.

   In order to avoid backtracing through the same instantiation twice,
   we mark each instantiation as we BT it, by setting
   inst->backtrace_number = backtrace_number (this is a global variable
   which gets incremented each time we build a chunk).

   Locals, grounds, and positive potentials are kept on lists (see the
   global variables below).  These are consed lists of the conditions
   (that is, the original instantiated conditions).  Furthermore,
   we mark the bt.wme_'s on each condition so we can quickly determine
   whether a given condition is already in a given set.  The "grounds_tc",
   "potentials_tc", "locals_tc", and "chunker_bt_pref" fields on wme's
   are used for this.  Wmes are marked as "in the grounds" by setting
   wme->grounds_tc = grounds_tc.  For potentials and locals, we also
   must set wme->chunker_bt_pref:  if the same wme was tested by two
   instantiations created at different times--times at which the wme
   was supported by two different preferences--then we really need to
   BT through *both* preferences.  Marking the wmes with just "locals_tc"
   or "potentials_tc" alone would prevent the second preference from
   being BT'd.

   The add_to_grounds(), add_to_potentials(), and add_to_locals()
   macros below are used to add conditions to these sets.  The negated
   conditions are maintained in the chunk_cond_set "negated_set."

   As we backtrace, each instantiation that has some Nots is added to
   the list instantiations_with_nots.  We have to go back afterwards
   and figure out which Nots are between identifiers that ended up in
   the grounds.
==================================================================== */


/* -- This version of the add functions combines the tests from two conditions if they both
 *    match the same wme.  Not currently used because there were significant problems when
 *    reordering and adding to rete.  Keeping around for now if adding all conditions proves
 *    to be an issue. -- */

#ifdef BACKTRACE_COMBINE_CONDITIONS
inline condition *find_cond_for_match_wme(agent* thisAgent, wme *matched_wme, ::list *search_list)
{
  assert(matched_wme && search_list);
  dprint(DT_BACKTRACE, "Looking in search_list...\n");

  for (cons *c = search_list; c ; c = c->rest)
  {
    dprint(DT_BACKTRACE, "Comparing:\n");
    dprint_condition(DT_BACKTRACE, static_cast<condition *> (c->first));
    if (static_cast<condition *>(c->first)->bt.wme_ == matched_wme)
    {
      dprint(DT_BACKTRACE, "Equivalent!  Condition is in list.\n");
      return static_cast<condition *>(c->first);
    } else {
      dprint(DT_BACKTRACE, "Not a match.\n");
    }
  }
  dprint(DT_BACKTRACE, "Condition not found in list.  Big bug b/c it's marked!\n");
  dprint_condition_cons(DT_BACKTRACE, search_list);
  assert(false);
  return NIL;
}

inline void add_to_grounds(agent* thisAgent, condition * cond)
{
  /* MToDo | Remove */
  cons *c;

  check_symbol_in_test(thisAgent, cond->data.tests.value_test, "Add_To_Grounds | ");

  if ((cond)->bt.wme_->grounds_tc != thisAgent->grounds_tc)
  {
    (cond)->bt.wme_->grounds_tc = thisAgent->grounds_tc;
    dprint(DT_BACKTRACE, "Pushing condition to ground list:\n");
    dprint_condition(DT_BACKTRACE, cond);
    push (thisAgent, (cond), thisAgent->grounds);
  } else
  {
    condition *first_condition = find_cond_for_match_wme(thisAgent, (cond)->bt.wme_, thisAgent->grounds);
    add_non_identical_tests (thisAgent, &(first_condition->data.tests.value_test), cond->data.tests.value_test);
    add_non_identical_tests (thisAgent, &(first_condition->data.tests.attr_test), cond->data.tests.attr_test);
  }
}

inline void add_to_potentials(agent* thisAgent, condition * cond)
{
  if ((cond)->bt.wme_->potentials_tc != thisAgent->potentials_tc)
  {
    (cond)->bt.wme_->potentials_tc = thisAgent->potentials_tc;
    (cond)->bt.wme_->chunker_bt_pref = (cond)->bt.trace;
    push (thisAgent, (cond), thisAgent->positive_potentials);
  } else
    if ((cond)->bt.wme_->chunker_bt_pref != (cond)->bt.trace)
  {
    push (thisAgent, (cond), thisAgent->positive_potentials);
  }
}

inline void add_to_locals(agent* thisAgent, condition * cond)
{
  if ((cond)->bt.wme_->locals_tc != thisAgent->locals_tc)
  {
    (cond)->bt.wme_->locals_tc = thisAgent->locals_tc;
    (cond)->bt.wme_->chunker_bt_pref = (cond)->bt.trace;
    push (thisAgent, (cond), thisAgent->locals);
  } else
    if ((cond)->bt.wme_->chunker_bt_pref != (cond)->bt.trace)
    {
      push (thisAgent, (cond), thisAgent->locals);
    }
}
#endif

#ifdef BACKTRACE_ADD_ALL_CONDITIONS
inline void add_to_grounds(agent* thisAgent, condition * cond)
{
  cons *c;

  if ((cond)->bt.wme_->grounds_tc != thisAgent->grounds_tc)
  {
    (cond)->bt.wme_->grounds_tc = thisAgent->grounds_tc;
  }
  push (thisAgent, (cond), thisAgent->grounds);
}

inline void add_to_potentials(agent* thisAgent, condition * cond)
{
  if ((cond)->bt.wme_->potentials_tc != thisAgent->potentials_tc)
  {
    (cond)->bt.wme_->potentials_tc = thisAgent->potentials_tc;
    (cond)->bt.wme_->chunker_bt_pref = (cond)->bt.trace;
  }
  push (thisAgent, (cond), thisAgent->positive_potentials);
}

inline void add_to_locals(agent* thisAgent, condition * cond)
{
  if ((cond)->bt.wme_->locals_tc != thisAgent->locals_tc)
  {
    (cond)->bt.wme_->locals_tc = thisAgent->locals_tc;
    (cond)->bt.wme_->chunker_bt_pref = (cond)->bt.trace;
  }
  push (thisAgent, (cond), thisAgent->locals);
}
#endif

/* -------------------------------------------------------------------
                     Backtrace Through Instantiation

   This routine BT's through a given instantiation.  The general method
   is as follows:

     1. If we've already BT'd this instantiation, then skip it.
     2. Mark the TC (in the instantiated conditions) of all higher goal
        ids tested in top-level positive conditions
     3. Scan through the instantiated conditions; add each one to the
        appropriate set (locals, positive_potentials, grounds, negated_set).
     4. If the instantiation has any Nots, add this instantiation to
        the list of instantiations_with_nots.
------------------------------------------------------------------- */

/* mvp 5-17-94 */
void print_consed_list_of_conditions (agent* thisAgent, list *c, int indent) {
  for (; c!=NIL; c=c->rest) {
    if (get_printer_output_column(thisAgent) >= COLUMNS_PER_LINE-20) print(thisAgent,  "\n      ");

    /* mvp 5-17-94 */
    print_spaces (thisAgent, indent);
    print_condition (thisAgent, static_cast<condition_struct *>(c->first));
  }
}

/* mvp 5-17-94 */
void print_consed_list_of_condition_wmes (agent* thisAgent, list *c, int indent) {
  for (; c!=NIL; c=c->rest) {
    if (get_printer_output_column(thisAgent) >= COLUMNS_PER_LINE-20) print(thisAgent,  "\n      ");

    /* mvp 5-17-94 */
    print_spaces (thisAgent, indent);
    print(thisAgent,  "     ");
    print_wme (thisAgent, (static_cast<condition *>(c->first))->bt.wme_);
  }
}

void cache_relational_constraints_for_test (agent* thisAgent, test t)
{
  /* -- Only conjunctive tests can have relational tests here -- */
  if (t->type != CONJUNCTIVE_TEST)
  {
    assert(t->type == EQUALITY_TEST);
    return;
  }

  test equality_test=NULL, referent_test, ctest;
  cons *c;
//  dprint(DT_CONSTRAINTS, "Looking for equality constraint...\n");
  for (c=t->data.conjunct_list; c!=NIL; c=c->rest)
  {
    if (static_cast<test>(c->first)->type == EQUALITY_TEST)
    {
//      dprint(DT_CONSTRAINTS, "...found %s.\n", test_to_string(static_cast<test>(c->first)));
      equality_test = static_cast<test>(c->first);
      break;
    }
  }
  assert(equality_test);
  for (c=t->data.conjunct_list; c!=NIL; c=c->rest)
  {
    ctest = static_cast<test>(c->first);
    switch (ctest->type) {
      case EQUALITY_TEST:
        break;
      case GREATER_TEST:
      case GREATER_OR_EQUAL_TEST:
      case LESS_TEST:
      case LESS_OR_EQUAL_TEST:
      case NOT_EQUAL_TEST:
      case SAME_TYPE_TEST:
      case DISJUNCTION_TEST:
        thisAgent->variablizationManager->cache_relational_constraint(equality_test, ctest);
        break;
      default:
        break;
    }
  }
}

void cache_relational_constraints (agent* thisAgent, condition *c)
{
  /* Probably don't need to do id.  Code in backtrace always refers to it as an equality test */
  //cache_relational_constraints_for_test(thisAgent, c->data.tests.id_test);
  cache_relational_constraints_for_test(thisAgent, c->data.tests.attr_test);
  cache_relational_constraints_for_test(thisAgent, c->data.tests.value_test);
}

/* This is the wme which is causing this production to be backtraced through.
   It is NULL when backtracing for a result preference.                   */

/* mvp 5-17-94 */
void backtrace_through_instantiation (agent* thisAgent,
                                      instantiation *inst,
                                      goal_stack_level grounds_level,
                                      condition *trace_cond,
                                      bool *reliable,
                                      int indent) {

  tc_number tc;   /* use this to mark ids in the ground set */
  tc_number tc2;  /* use this to mark other ids we see */
  condition *c;
  list *grounds_to_print, *pots_to_print, *locals_to_print, *negateds_to_print;
  bool need_another_pass;
  backtrace_str temp_explain_backtrace;

  if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM]) {

    /* mvp 5-17-94 */
    print_spaces (thisAgent, indent);
    print(thisAgent,  "... BT through instantiation of ");
    if (inst->prod) print_with_symbols (thisAgent, "%y\n",inst->prod->name);
    else print(thisAgent,  "[dummy production]\n");

    xml_begin_tag(thisAgent, kTagBacktrace);
    if (inst->prod) xml_att_val(thisAgent, kProduction_Name, inst->prod->name);
    else xml_att_val(thisAgent, kProduction_Name, "[dummy production]");

  }

  /* --- if the instantiation has already been BT'd, don't repeat it --- */
  if (inst->backtrace_number == thisAgent->backtrace_number) {
    if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM]) {

      /* mvp 5-17-94 */
      print_spaces (thisAgent, indent);
      print(thisAgent,  "(We already backtraced through this instantiation.)\n");
      xml_att_val(thisAgent, kBacktracedAlready, "true");
      xml_end_tag(thisAgent, kTagBacktrace);
    }
    return;
  }
  inst->backtrace_number = thisAgent->backtrace_number;

  /* Record information on the production being backtraced through */
  /* if (thisAgent->explain_flag) { */
  if (thisAgent->sysparams[EXPLAIN_SYSPARAM]) {
    temp_explain_backtrace.trace_cond = trace_cond;  /* Not copied yet */
    if (trace_cond == NULL)   /* Backtracing for a result */
      temp_explain_backtrace.result = true;
    else
      temp_explain_backtrace.result = false;

    temp_explain_backtrace.grounds    = NIL;
    temp_explain_backtrace.potentials = NIL;
    temp_explain_backtrace.locals     = NIL;
    temp_explain_backtrace.negated    = NIL;

	if (inst->prod) {
      strncpy(temp_explain_backtrace.prod_name,inst->prod->name->data.sc.name, BUFFER_PROD_NAME_SIZE);
	} else {
      strncpy(temp_explain_backtrace.prod_name,"Dummy production", BUFFER_PROD_NAME_SIZE);
	}
	(temp_explain_backtrace.prod_name)[BUFFER_PROD_NAME_SIZE - 1] = 0; /* ensure null termination */

    temp_explain_backtrace.next_backtrace = NULL;
  }

  if (!inst->reliable)
    *reliable = false;

  /* --- mark transitive closure of each higher goal id that was tested in
     the id field of a top-level positive condition --- */
  tc = get_new_tc_number (thisAgent);
  tc2 = get_new_tc_number (thisAgent);
  need_another_pass = false;

  for (c=inst->top_of_instantiated_conditions; c!=NIL; c=c->next) {
    Symbol *id, *value;

    if (c->type!=POSITIVE_CONDITION) continue;

//    dprint(DT_CONSTRAINTS, "Backtracing through condition: ");
//    dprint_condition(DT_CONSTRAINTS, c, "", true, true, true);
    cache_relational_constraints(thisAgent, c);

    id = c->data.tests.id_test->data.referent;

    if (id->tc_num == tc) {
      /* --- id is already in the TC, so add in the value --- */
      value = c->data.tests.value_test->data.referent;
      if (value->symbol_type==IDENTIFIER_SYMBOL_TYPE) {
        /* --- if we already saw it before, we're going to have to go back
           and make another pass to get the complete TC --- */
        if (value->tc_num == tc2) need_another_pass = true;
        value->tc_num = tc;
      }
    } else if ((id->data.id.isa_goal) && (c->bt.level <= grounds_level)) {
      /* --- id is a higher goal id that was tested: so add id to the TC --- */
      id->tc_num = tc;
      value = c->data.tests.value_test->data.referent;
      if (value->symbol_type==IDENTIFIER_SYMBOL_TYPE) {
        /* --- if we already saw it before, we're going to have to go back
           and make another pass to get the complete TC --- */
        if (value->tc_num == tc2) need_another_pass = true;
        value->tc_num = tc;
      }
    } else {
      /* --- as far as we know so far, id shouldn't be in the tc: so mark it
         with number "tc2" to indicate that it's been seen already --- */
      id->tc_num = tc2;
    }
  }

  /* --- if necessary, make more passes to get the complete TC through the
     top-level positive conditions (recall that they're all super-simple
     wme tests--all three fields are equality tests --- */
  while (need_another_pass) {
    Symbol *value;

    need_another_pass = false;
    for (c=inst->top_of_instantiated_conditions; c!=NIL; c=c->next) {
      if (c->type!=POSITIVE_CONDITION)
        continue;
      if (c->data.tests.id_test->data.referent->tc_num != tc)
        continue;
      value = c->data.tests.value_test->data.referent;
      if (value->symbol_type==IDENTIFIER_SYMBOL_TYPE)
        if (value->tc_num != tc) {
          value->tc_num = tc;
          need_another_pass = true;
        }
    } /* end of for loop */
  } /* end of while loop */

  /* --- scan through conditions, collect grounds, potentials, & locals --- */
  grounds_to_print = NIL;
  pots_to_print = NIL;
  locals_to_print = NIL;
  negateds_to_print = NIL;

  /* Record the conds in the print_lists even if not going to be printed */

  for (c=inst->top_of_instantiated_conditions; c!=NIL; c=c->next) {
    if (c->type==POSITIVE_CONDITION) {

      /* --- positive cond's are grounds, potentials, or locals --- */
      if (c->data.tests.id_test->data.referent->tc_num == tc) {
        dprint(DT_BACKTRACE, "Backtracing adding ground condition...\n");
        dprint_condition(DT_BACKTRACE, c);
        add_to_grounds (thisAgent, c);

        if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM] ||
            thisAgent->sysparams[EXPLAIN_SYSPARAM])
          push (thisAgent, c, grounds_to_print);
      }
      else if (c->bt.level <= grounds_level) {
        add_to_potentials (thisAgent, c);
        if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM] ||
            thisAgent->sysparams[EXPLAIN_SYSPARAM])
          push (thisAgent, c, pots_to_print);
      }
      else {
        add_to_locals (thisAgent, c);
        if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM] ||
            thisAgent->sysparams[EXPLAIN_SYSPARAM])
          push (thisAgent, c, locals_to_print);
      }
    }
    else {
      /* --- negative or nc cond's are either grounds or potentials --- */
      add_to_chunk_cond_set (thisAgent, &thisAgent->negated_set,
          make_chunk_cond_for_negated_condition(thisAgent, c));
      if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM] ||
          thisAgent->sysparams[EXPLAIN_SYSPARAM])
        push (thisAgent, c, negateds_to_print);
    }
  } /* end of for loop */

  dprint(DT_BACKTRACE, "Grounds in backtrace:\n");
  dprint_condition_cons(DT_BACKTRACE, thisAgent->grounds);

  /* Now record the sets of conditions.  Note that these are not necessarily */
  /* the final resting place for these wmes.  In particular potentials may   */
  /* move over to become grounds, but since all we really need for explain is*/
  /* the list of wmes, this will do as a place to record them.               */

  if (thisAgent->sysparams[EXPLAIN_SYSPARAM])
    explain_add_temp_to_backtrace_list(thisAgent, &temp_explain_backtrace,grounds_to_print,
                                       pots_to_print,locals_to_print,negateds_to_print);

  /* --- if tracing BT, print the resulting conditions, etc. --- */
  if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM]) {
    /* mvp 5-17-94 */
    print_spaces (thisAgent, indent);
    print(thisAgent,  "  -->Grounds:\n");
    xml_begin_tag(thisAgent, kTagGrounds);
    print_consed_list_of_condition_wmes (thisAgent, grounds_to_print, indent);
    xml_end_tag(thisAgent, kTagGrounds);
    print(thisAgent,  "\n");
    print_spaces (thisAgent, indent);
    print(thisAgent,  "\n  -->Potentials:\n");
    xml_begin_tag(thisAgent, kTagPotentials);
    print_consed_list_of_condition_wmes (thisAgent, pots_to_print, indent);
    xml_end_tag(thisAgent, kTagPotentials);
    print(thisAgent,  "\n");
    print_spaces (thisAgent, indent);
    print(thisAgent,  "  -->Locals:\n");
    xml_begin_tag(thisAgent, kTagLocals);
    print_consed_list_of_condition_wmes (thisAgent, locals_to_print, indent);
    xml_end_tag(thisAgent, kTagLocals);
    print(thisAgent,  "\n");
    print_spaces (thisAgent, indent);
    print(thisAgent,  "  -->Negated:\n");
    xml_begin_tag(thisAgent, kTagNegated);
    print_consed_list_of_conditions (thisAgent, negateds_to_print, indent);
    xml_end_tag(thisAgent, kTagNegated);
    print(thisAgent,  "\n");
    /* mvp done */

    xml_begin_tag(thisAgent, kTagNots);
	  xml_end_tag(thisAgent, kTagNot);
    xml_end_tag(thisAgent, kTagNots);
    xml_end_tag(thisAgent, kTagBacktrace);
  }

  /* Moved these free's down to here, to ensure they are cleared even if we're
     not printing these lists     */

  free_list (thisAgent, grounds_to_print);
  free_list (thisAgent, pots_to_print);
  free_list (thisAgent, locals_to_print);
  free_list (thisAgent, negateds_to_print);
}

/* ---------------------------------------------------------------
                             Trace Locals

   This routine backtraces through locals, and keeps doing so until
   there are no more locals to BT.
--------------------------------------------------------------- */

void trace_locals (agent* thisAgent, goal_stack_level grounds_level, bool *reliable) {

  /* mvp 5-17-94 */
  cons *c, *CDPS;
  condition *cond;
  preference *bt_pref, *p;

  if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM]) {
    print(thisAgent,  "\n\n*** Tracing Locals ***\n");
    xml_begin_tag(thisAgent, kTagLocals);
  }

  while (thisAgent->locals) {
    c = thisAgent->locals;
    thisAgent->locals = thisAgent->locals->rest;
    cond = static_cast<condition_struct *>(c->first);
    free_cons (thisAgent, c);

    if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM]) {
      print(thisAgent,  "\nFor local ");
      xml_begin_tag(thisAgent, kTagLocal);
      print_wme (thisAgent, cond->bt.wme_);
      print(thisAgent,  " ");
    }

    bt_pref = find_clone_for_level (cond->bt.trace,
		                            static_cast<goal_stack_level>(grounds_level+1));
    /* --- if it has a trace at this level, backtrace through it --- */
    if (bt_pref) {

      backtrace_through_instantiation (thisAgent, bt_pref->inst, grounds_level,cond, reliable, 0);

      /* MMA 8-2012: Check for any CDPS prefs and backtrace through them */
      if (cond->bt.CDPS) {
        for (CDPS=cond->bt.CDPS; CDPS!=NIL; CDPS=CDPS->rest) {
          p = static_cast<preference_struct *>(CDPS->first);
          if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM]) {
            print(thisAgent,  "     Backtracing through CDPS preference: ");
            xml_begin_tag(thisAgent, kTagCDPSPreference);
            print_preference (thisAgent, p);
          }
          backtrace_through_instantiation (thisAgent, p->inst, grounds_level, cond, reliable, 6);

          if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM]) {
            xml_end_tag(thisAgent, kTagCDPSPreference);
          }
        }
      }
      /* MMA 8-2012 end */

      if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM]) {
          xml_end_tag(thisAgent, kTagLocal);
      }
      continue;
    }

	if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM]) {
      print(thisAgent,  "...no trace, can't BT");
	  // add an empty <backtrace> tag to make parsing XML easier
	  xml_begin_tag(thisAgent, kTagBacktrace);
	  xml_end_tag(thisAgent, kTagBacktrace);
	}
    /* --- for augmentations of the local goal id, either handle the
       "^quiescence t" test or discard it --- */
    if (cond->data.tests.id_test->data.referent->data.id.isa_goal) {
      if ((cond->data.tests.attr_test->data.referent ==
           thisAgent->quiescence_symbol) &&
          (cond->data.tests.value_test->data.referent ==
           thisAgent->t_symbol) &&
          (! cond->test_for_acceptable_preference)) {
        *reliable = false;
      }
      if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM]) {
          xml_end_tag(thisAgent, kTagLocal);
      }
      continue;
    }

    /* --- otherwise add it to the potential set --- */
    if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM]) {
      print(thisAgent,  " --> make it a potential.");
      xml_begin_tag(thisAgent, kTagAddToPotentials);
      xml_end_tag(thisAgent, kTagAddToPotentials);
    }
    add_to_potentials (thisAgent, cond);

    if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM]) {
        xml_end_tag(thisAgent, kTagLocal);
    }

  } /* end of while locals loop */

  if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM]) {
    xml_end_tag(thisAgent, kTagLocals);
  }
}

/* ---------------------------------------------------------------
                       Trace Grounded Potentials

   This routine looks for positive potentials that are in the TC
   of the ground set, and moves them over to the ground set.  This
   process is repeated until no more positive potentials are in
   the TC of the grounds.
--------------------------------------------------------------- */

void trace_grounded_potentials (agent* thisAgent) {
  tc_number tc;
  cons *c, *next_c, *prev_c;
  condition *pot;
  bool need_another_pass;

  if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM]) {
    print(thisAgent,  "\n\n*** Tracing Grounded Potentials ***\n");
    xml_begin_tag(thisAgent, kTagGroundedPotentials);
  }

  /* --- setup the tc of the ground set --- */
  tc = get_new_tc_number(thisAgent);
  for (c=thisAgent->grounds; c!=NIL; c=c->rest)
	  add_cond_to_tc (thisAgent, static_cast<condition_struct *>(c->first), tc, NIL, NIL);

  need_another_pass = true;
  while (need_another_pass) {
    need_another_pass = false;
    /* --- look for any potentials that are in the tc now --- */
    prev_c = NIL;
    for (c=thisAgent->positive_potentials; c!=NIL; c=next_c) {
      next_c = c->rest;
      pot = static_cast<condition_struct *>(c->first);
      if (cond_is_in_tc (thisAgent, pot, tc)) {
        /* --- pot is a grounded potential, move it over to ground set --- */
        if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM]) {
          print(thisAgent,  "\n-->Moving to grounds: ");
          print_wme (thisAgent, pot->bt.wme_);
        }
        if (prev_c) prev_c->rest = next_c; else thisAgent->positive_potentials = next_c;
        if (pot->bt.wme_->grounds_tc != thisAgent->grounds_tc) { /* add pot to grounds */
          pot->bt.wme_->grounds_tc = thisAgent->grounds_tc;
          c->rest = thisAgent->grounds; thisAgent->grounds = c;
          add_cond_to_tc (thisAgent, pot, tc, NIL, NIL);
          need_another_pass = true;
        } else { /* pot was already in the grounds, do don't add it */
          free_cons (thisAgent, c);
        }
      } else {
        prev_c = c;
      }
    } /* end of for c */
  } /* end of while need_another_pass */

  if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM]) {
    xml_end_tag(thisAgent, kTagGroundedPotentials);
  }
}

/* ---------------------------------------------------------------
                     Trace Ungrounded Potentials

   This routine backtraces through ungrounded potentials.  At entry,
   all potentials must be ungrounded.  This BT's through each
   potential that has some trace (at the right level) that we can
   BT through.  Other potentials are left alone.  true is returned
   if anything was BT'd; false if nothing changed.
--------------------------------------------------------------- */

bool trace_ungrounded_potentials (agent* thisAgent, goal_stack_level grounds_level, bool *reliable) {

  /* mvp 5-17-94 */
  cons *c, *next_c, *prev_c, *CDPS;
  cons *pots_to_bt;
  condition *potential;
  preference *bt_pref, *p;

  if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM]) {
    print(thisAgent,  "\n\n*** Tracing Ungrounded Potentials ***\n");
    xml_begin_tag(thisAgent, kTagUngroundedPotentials);
  }

  /* --- scan through positive potentials, pick out the ones that have
     a preference we can backtrace through --- */
  pots_to_bt = NIL;
  prev_c = NIL;
  for (c=thisAgent->positive_potentials; c!=NIL; c=next_c) {
    next_c = c->rest;
    potential = static_cast<condition_struct *>(c->first);
    bt_pref = find_clone_for_level (potential->bt.trace,
		                            static_cast<goal_stack_level>(grounds_level+1));
    if (bt_pref) {
      if (prev_c) prev_c->rest = next_c; else thisAgent->positive_potentials = next_c;
      c->rest = pots_to_bt; pots_to_bt = c;
    } else {
      prev_c = c;
    }
  }

  /* --- if none to BT, exit --- */
  if (!pots_to_bt) {
      if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM]) {
          xml_end_tag(thisAgent, kTagUngroundedPotentials);
      }
      return false;
  }

  /* --- backtrace through each one --- */
  while (pots_to_bt) {
    c = pots_to_bt;
    pots_to_bt = pots_to_bt->rest;
    potential = static_cast<condition_struct *>(c->first);
    free_cons (thisAgent, c);
    if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM]) {
      print(thisAgent,  "\nFor ungrounded potential ");
      xml_begin_tag(thisAgent, kTagUngroundedPotential);
      print_wme (thisAgent, potential->bt.wme_);
      print(thisAgent,  " ");
    }
    bt_pref = find_clone_for_level (potential->bt.trace,
		                            static_cast<goal_stack_level>(grounds_level+1));

    backtrace_through_instantiation (thisAgent, bt_pref->inst, grounds_level,potential, reliable, 0);

    /* MMA 8-2012: now backtrace through CDPS of potentials */
    if (potential->bt.CDPS) {
      for (CDPS=potential->bt.CDPS; CDPS!=NIL; CDPS=CDPS->rest) {
        p = static_cast<preference_struct *>(CDPS->first);
        if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM]) {
          print(thisAgent,  "     Backtracing through CDPS preference: ");
          xml_begin_tag(thisAgent, kTagCDPSPreference);
          print_preference (thisAgent, p);
        }
        backtrace_through_instantiation (thisAgent, p->inst, grounds_level, potential, reliable, 6);

        if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM]) {
            xml_end_tag(thisAgent, kTagCDPSPreference);
        }
      }
    }
    /* MMA end */

    if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM]) {
        xml_end_tag(thisAgent, kTagUngroundedPotential);
    }
  }

  if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM]) {
      xml_end_tag(thisAgent, kTagUngroundedPotentials);
  }

  return true;
}

void report_local_negation (agent* thisAgent, condition* c) {
    if (thisAgent->sysparams[TRACE_CHUNK_NAMES_SYSPARAM]) {
		// use the same code as the backtracing above
		list* negated_to_print = NIL;
		push (thisAgent, c, negated_to_print);

		print(thisAgent,  "\n*** Chunk won't be formed due to local negation in backtrace ***\n");
		xml_begin_tag(thisAgent, kTagLocalNegation);
		print_consed_list_of_conditions (thisAgent, negated_to_print, 2);
		xml_end_tag(thisAgent, kTagLocalNegation);

		free_list (thisAgent, negated_to_print);
	}
}
