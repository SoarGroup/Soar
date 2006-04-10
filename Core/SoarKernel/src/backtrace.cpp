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
 *  file:  backtrace.cpp
 *
 * =======================================================================
 *  Backtracing structures and routines.  See also explain.c
 * =======================================================================
 */

/* ====================================================================
                        Backtracing routines
   ==================================================================== */


#include "backtrace.h"
#include "mem.h"
#include "kernel.h"
#include "print.h"
#include "wmem.h"
#include "gdatastructs.h"
#include "agent.h"
#include "instantiations.h"
#include "production.h"
#include "symtab.h"
#include "explain.h"
#include "recmem.h"
#include "xmlTraceNames.h" // for XML trace stuff
#include "gski_event_system_functions.h"  //for XML trace stuff

using namespace xmlTraceNames;

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

#ifdef USE_MACROS

#define add_to_grounds(thisAgent, cond) { \
  if ((cond)->bt.wme_->grounds_tc != thisAgent->grounds_tc) { \
    (cond)->bt.wme_->grounds_tc = thisAgent->grounds_tc; \
    push (thisAgent, (cond), thisAgent->grounds); } }

#define add_to_potentials(thisAgent, cond) { \
  if ((cond)->bt.wme_->potentials_tc != thisAgent->potentials_tc) { \
    (cond)->bt.wme_->potentials_tc = thisAgent->potentials_tc; \
    (cond)->bt.wme_->chunker_bt_pref = (cond)->bt.trace; \
    push (thisAgent, (cond), thisAgent->positive_potentials); \
  } else if ((cond)->bt.wme_->chunker_bt_pref != (cond)->bt.trace) { \
    push (thisAgent, (cond), thisAgent->positive_potentials); } }

#define add_to_locals(thisAgent, cond) { \
  if ((cond)->bt.wme_->locals_tc != thisAgent->locals_tc) { \
    (cond)->bt.wme_->locals_tc = thisAgent->locals_tc; \
    (cond)->bt.wme_->chunker_bt_pref = (cond)->bt.trace; \
    push (thisAgent, (cond), thisAgent->locals); \
  } else if ((cond)->bt.wme_->chunker_bt_pref != (cond)->bt.trace) { \
    push (thisAgent, (cond), thisAgent->locals); } }

#else

inline void add_to_grounds(agent* thisAgent, condition * cond)
{
  if ((cond)->bt.wme_->grounds_tc != thisAgent->grounds_tc) {
    (cond)->bt.wme_->grounds_tc = thisAgent->grounds_tc;
    push (thisAgent, (cond), thisAgent->grounds); }
}

inline void add_to_potentials(agent* thisAgent, condition * cond)
{
  if ((cond)->bt.wme_->potentials_tc != thisAgent->potentials_tc) {
    (cond)->bt.wme_->potentials_tc = thisAgent->potentials_tc;
    (cond)->bt.wme_->chunker_bt_pref = (cond)->bt.trace;
    push (thisAgent, (cond), thisAgent->positive_potentials);
  } else if ((cond)->bt.wme_->chunker_bt_pref != (cond)->bt.trace) {
    push (thisAgent, (cond), thisAgent->positive_potentials); }
}

inline void add_to_locals(agent* thisAgent, condition * cond)
{
  if ((cond)->bt.wme_->locals_tc != thisAgent->locals_tc) {
    (cond)->bt.wme_->locals_tc = thisAgent->locals_tc;
    (cond)->bt.wme_->chunker_bt_pref = (cond)->bt.trace;
    push (thisAgent, (cond), thisAgent->locals);
  } else if ((cond)->bt.wme_->chunker_bt_pref != (cond)->bt.trace) {
    push (thisAgent, (cond), thisAgent->locals); }
}
#endif /* USE_MACROS */

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
    if (get_printer_output_column(thisAgent) >= COLUMNS_PER_LINE-20) print (thisAgent, "\n      ");

    /* mvp 5-17-94 */
    print_spaces (thisAgent, indent);
    print_condition (thisAgent, static_cast<condition_struct *>(c->first));
  }
}

/* mvp 5-17-94 */
void print_consed_list_of_condition_wmes (agent* thisAgent, list *c, int indent) {
  for (; c!=NIL; c=c->rest) {
    if (get_printer_output_column(thisAgent) >= COLUMNS_PER_LINE-20) print (thisAgent, "\n      ");

    /* mvp 5-17-94 */
    print_spaces (thisAgent, indent);
    print (thisAgent, "     ");
    print_wme (thisAgent, ((condition *)(c->first))->bt.wme_);
  }
}

/* This is the wme which is causing this production to be backtraced through. 
   It is NULL when backtracing for a result preference.                   */

/* mvp 5-17-94 */
void backtrace_through_instantiation (agent* thisAgent, 
                                      instantiation *inst,
                                      goal_stack_level grounds_level,
		                                condition *trace_cond,
                                      int indent) {

  tc_number tc;   /* use this to mark ids in the ground set */
  tc_number tc2;  /* use this to mark other ids we see */
  condition *c;
  list *grounds_to_print, *pots_to_print, *locals_to_print, *negateds_to_print;
  Bool need_another_pass;
  backtrace_str temp_explain_backtrace;

  if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM]) {

    /* mvp 5-17-94 */
    print_spaces (thisAgent, indent);
    print (thisAgent, "... BT through instantiation of ");
    if (inst->prod) print_with_symbols (thisAgent, "%y\n",inst->prod->name);
    else print_string (thisAgent, "[dummy production]\n");
    
    gSKI_MakeAgentCallbackXML(thisAgent, kFunctionBeginTag, kTagBacktrace);
    if (inst->prod) gSKI_MakeAgentCallbackXML(thisAgent, kFunctionAddAttribute, kProduction_Name, symbol_to_string(thisAgent, inst->prod->name, true, 0, 0));
    else gSKI_MakeAgentCallbackXML(thisAgent, kFunctionAddAttribute, kProduction_Name, "[dummy production]");
      
  }

  /* --- if the instantiation has already been BT'd, don't repeat it --- */
  if (inst->backtrace_number == thisAgent->backtrace_number) {
    if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM]) {

      /* mvp 5-17-94 */
      print_spaces (thisAgent, indent);
      print_string (thisAgent, "(We already backtraced through this instantiation.)\n");
      gSKI_MakeAgentCallbackXML(thisAgent, kFunctionAddAttribute, kBacktracedAlready, "true");
      gSKI_MakeAgentCallbackXML(thisAgent, kFunctionEndTag, kTagBacktrace);
    }
    return;
  }
  inst->backtrace_number = thisAgent->backtrace_number;

  /* Record information on the production being backtraced through */
  /* if (thisAgent->explain_flag) { */
  if (thisAgent->sysparams[EXPLAIN_SYSPARAM]) {
    temp_explain_backtrace.trace_cond = trace_cond;  /* Not copied yet */
    if (trace_cond == NULL)   /* Backtracing for a result */
      temp_explain_backtrace.result = TRUE;
    else
      temp_explain_backtrace.result = FALSE;

    temp_explain_backtrace.grounds    = NIL;
    temp_explain_backtrace.potentials = NIL;
    temp_explain_backtrace.locals     = NIL;
    temp_explain_backtrace.negated    = NIL;

	if (inst->prod) {
      strncpy(temp_explain_backtrace.prod_name,inst->prod->name->sc.name, BUFFER_PROD_NAME_SIZE);
	} else {
      strncpy(temp_explain_backtrace.prod_name,"Dummy production", BUFFER_PROD_NAME_SIZE);
	}
	(temp_explain_backtrace.prod_name)[BUFFER_PROD_NAME_SIZE - 1] = 0; /* ensure null termination */

    temp_explain_backtrace.next_backtrace = NULL;
  }

  /* --- check okay_to_variablize flag --- */
  if (! inst->okay_to_variablize) thisAgent->variablize_this_chunk = FALSE;

  /* --- mark transitive closure of each higher goal id that was tested in
     the id field of a top-level positive condition --- */
  tc = get_new_tc_number (thisAgent);
  tc2 = get_new_tc_number (thisAgent);
  need_another_pass = FALSE;

  for (c=inst->top_of_instantiated_conditions; c!=NIL; c=c->next) {
    Symbol *id, *value;
    
    if (c->type!=POSITIVE_CONDITION) continue;
    id = referent_of_equality_test (c->data.tests.id_test);

    if (id->id.tc_num == tc) {
      /* --- id is already in the TC, so add in the value --- */
      value = referent_of_equality_test (c->data.tests.value_test);
      if (value->common.symbol_type==IDENTIFIER_SYMBOL_TYPE) {
        /* --- if we already saw it before, we're going to have to go back
           and make another pass to get the complete TC --- */
        if (value->id.tc_num == tc2) need_another_pass = TRUE;
        value->id.tc_num = tc;
      }
    } else if ((id->id.isa_goal) && (c->bt.level <= grounds_level)) {
      /* --- id is a higher goal id that was tested: so add id to the TC --- */
      id->id.tc_num = tc;
      value = referent_of_equality_test (c->data.tests.value_test);
      if (value->common.symbol_type==IDENTIFIER_SYMBOL_TYPE) {
        /* --- if we already saw it before, we're going to have to go back
           and make another pass to get the complete TC --- */
        if (value->id.tc_num == tc2) need_another_pass = TRUE;
        value->id.tc_num = tc;
      }
    } else {
      /* --- as far as we know so far, id shouldn't be in the tc: so mark it
         with number "tc2" to indicate that it's been seen already --- */
      id->id.tc_num = tc2;
    }
  }
  
  /* --- if necessary, make more passes to get the complete TC through the
     top-level positive conditions (recall that they're all super-simple
     wme tests--all three fields are equality tests --- */
  while (need_another_pass) {
    Symbol *value;
    
    need_another_pass = FALSE;
    for (c=inst->top_of_instantiated_conditions; c!=NIL; c=c->next) {
      if (c->type!=POSITIVE_CONDITION)
        continue;
      if (referent_of_equality_test(c->data.tests.id_test)->id.tc_num != tc)
        continue;
      value = referent_of_equality_test(c->data.tests.value_test);
      if (value->common.symbol_type==IDENTIFIER_SYMBOL_TYPE)
        if (value->id.tc_num != tc) {
          value->id.tc_num = tc;
          need_another_pass = TRUE;
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

      /* REW: begin 11.22.97 */
      /* print (thisAgent, "\n Checking...");print_wme(c->bt.wme_);
      if (c->bt.trace) print ("c->bt.trace exists..."); else print("\n    no c->bt.trace...");
      if (c->bt.wme_) { 
	print ("c->bt.wme_....");
	if  (c->bt.wme_->preference)
	  print("c->bt.wme_->preference");
	else 
	  print("\n no c->bt.wme_->preference");
      }	else
	print ("\nNo WME No Preference!!!!!!");
      print("\n"); 
      if ((c->bt.trace) && (c->bt.wme_->preference)){
      if (c->bt.trace != c->bt.wme_->preference) {
	print("\n bt.trace and WME preferences not equal:\n");
	print(thisAgent, "\nWME:"); print_wme(c->bt.wme_);
	print("\n bt.trace:"); 
	if (c->bt.trace) print_preference(c->bt.trace); else print(" NIL\n");
	print("\n bt.wme_->preference:"); 
        if (c->bt.wme_->preference) print_preference(c->bt.wme_->preference);
	else print(" NIL\n");
	c->bt.trace = c->bt.wme_->preference;
	c->bt.level = c->bt.wme_->id->id.level;
      }
      }*/
      /* REW: end   11.22.97 */ 
      /* --- positive cond's are grounds, potentials, or locals --- */
      if (referent_of_equality_test(c->data.tests.id_test)->id.tc_num == tc) {
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
		                     make_chunk_cond_for_condition(thisAgent, c));
      if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM] || 
            thisAgent->sysparams[EXPLAIN_SYSPARAM])
	push (thisAgent, c, negateds_to_print);
    }
  } /* end of for loop */

  /* --- add new nots to the not set --- */
  if (inst->nots) push (thisAgent, inst, thisAgent->instantiations_with_nots);

  /* Now record the sets of conditions.  Note that these are not necessarily */
  /* the final resting place for these wmes.  In particular potentials may   */
  /* move over to become grounds, but since all we really need for explain is*/
  /* the list of wmes, this will do as a place to record them.               */

  if (thisAgent->sysparams[EXPLAIN_SYSPARAM])
    explain_add_temp_to_backtrace_list(thisAgent, &temp_explain_backtrace,grounds_to_print,
                                       pots_to_print,locals_to_print,negateds_to_print);

  /* --- if tracing BT, print the resulting conditions, etc. --- */
  if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM]) {
    not_struct *not1;

    /* mvp 5-17-94 */
    print_spaces (thisAgent, indent);
    print_string (thisAgent, "  -->Grounds:\n");
    gSKI_MakeAgentCallbackXML(thisAgent, kFunctionBeginTag, kTagGrounds);
    print_consed_list_of_condition_wmes (thisAgent, grounds_to_print, indent);
    gSKI_MakeAgentCallbackXML(thisAgent, kFunctionEndTag, kTagGrounds);
    print (thisAgent, "\n");
    print_spaces (thisAgent, indent);
    print_string (thisAgent, "\n  -->Potentials:\n");
    gSKI_MakeAgentCallbackXML(thisAgent, kFunctionBeginTag, kTagPotentials);
    print_consed_list_of_condition_wmes (thisAgent, pots_to_print, indent);
    gSKI_MakeAgentCallbackXML(thisAgent, kFunctionEndTag, kTagPotentials);
    print (thisAgent, "\n");
    print_spaces (thisAgent, indent);
    print_string (thisAgent, "  -->Locals:\n");
    gSKI_MakeAgentCallbackXML(thisAgent, kFunctionBeginTag, kTagLocals);
    print_consed_list_of_condition_wmes (thisAgent, locals_to_print, indent);
    gSKI_MakeAgentCallbackXML(thisAgent, kFunctionEndTag, kTagLocals);
    print (thisAgent, "\n");
    print_spaces (thisAgent, indent);
    print_string (thisAgent, "  -->Negated:\n");
    gSKI_MakeAgentCallbackXML(thisAgent, kFunctionBeginTag, kTagNegated);
    print_consed_list_of_conditions (thisAgent, negateds_to_print, indent);
    gSKI_MakeAgentCallbackXML(thisAgent, kFunctionEndTag, kTagNegated);
    print (thisAgent, "\n");
    print_spaces (thisAgent, indent);
    print_string (thisAgent, "  -->Nots:\n");
    /* mvp done */

    gSKI_MakeAgentCallbackXML(thisAgent, kFunctionBeginTag, kTagNots);
    for (not1=inst->nots; not1!=NIL; not1=not1->next) {
      print_with_symbols (thisAgent, "    %y <> %y\n", not1->s1, not1->s2);
	  gSKI_MakeAgentCallbackXML(thisAgent, kFunctionBeginTag, kTagNot);
      gSKI_MakeAgentCallbackXML(thisAgent, kFunctionAddAttribute, kBacktraceSymbol1, symbol_to_string(thisAgent, not1->s1, true, 0, 0));
      gSKI_MakeAgentCallbackXML(thisAgent, kFunctionAddAttribute, kBacktraceSymbol2, symbol_to_string(thisAgent, not1->s2, true, 0, 0));
	  gSKI_MakeAgentCallbackXML(thisAgent, kFunctionEndTag, kTagNot);
    }
    gSKI_MakeAgentCallbackXML(thisAgent, kFunctionEndTag, kTagNots);
    gSKI_MakeAgentCallbackXML(thisAgent, kFunctionEndTag, kTagBacktrace);
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

void trace_locals (agent* thisAgent, goal_stack_level grounds_level) {

  /* mvp 5-17-94 */
  cons *c, *prohibits;
  condition *cond;
  preference *bt_pref, *p;

  if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM]) {
    print_string (thisAgent, "\n\n*** Tracing Locals ***\n");
    gSKI_MakeAgentCallbackXML(thisAgent, kFunctionBeginTag, kTagLocals);
  }
  
  while (thisAgent->locals) {
    c = thisAgent->locals;
    thisAgent->locals = thisAgent->locals->rest;
    cond = static_cast<condition_struct *>(c->first);
    free_cons (thisAgent, c);

    if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM]) {
      print_string (thisAgent, "\nFor local ");
      gSKI_MakeAgentCallbackXML(thisAgent, kFunctionBeginTag, kTagLocal);
      print_wme (thisAgent, cond->bt.wme_);
      print_string (thisAgent, " ");
    }

    bt_pref = find_clone_for_level (cond->bt.trace, 
		                            (goal_stack_level)(grounds_level+1));
    /* --- if it has a trace at this level, backtrace through it --- */
    if (bt_pref) {

      /* mvp 5-17-94 */
      backtrace_through_instantiation (thisAgent, bt_pref->inst, grounds_level,cond, 0);

      /* check if any prohibit preferences */
      if (cond->bt.prohibits) {
        for (prohibits=cond->bt.prohibits; prohibits!=NIL; prohibits=prohibits->rest) {
          p = static_cast<preference_struct *>(prohibits->first);
          if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM]) {
            print_string (thisAgent, "     For prohibit preference: ");
            gSKI_MakeAgentCallbackXML(thisAgent, kFunctionBeginTag, kTagProhibitPreference);
            print_preference (thisAgent, p);
          }
          backtrace_through_instantiation (thisAgent, p->inst, grounds_level, cond, 6);

          if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM]) {
            gSKI_MakeAgentCallbackXML(thisAgent, kFunctionEndTag, kTagProhibitPreference);
          }
        }
      }
      /* mvp done */
      if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM]) {
          gSKI_MakeAgentCallbackXML(thisAgent, kFunctionEndTag, kTagLocal);
      }
      continue;
    }

	if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM]) {
      print_string (thisAgent, "...no trace, can't BT");
	  // add an empty <backtrace> tag to make parsing XML easier
	  gSKI_MakeAgentCallbackXML(thisAgent, kFunctionBeginTag, kTagBacktrace);
	  gSKI_MakeAgentCallbackXML(thisAgent, kFunctionEndTag, kTagBacktrace);
	}
    /* --- for augmentations of the local goal id, either handle the
       "^quiescence t" test or discard it --- */
    if (referent_of_equality_test(cond->data.tests.id_test)->id.isa_goal) {
      if ((referent_of_equality_test(cond->data.tests.attr_test) ==
           thisAgent->quiescence_symbol) &&
          (referent_of_equality_test(cond->data.tests.value_test) ==
           thisAgent->t_symbol) &&
          (! cond->test_for_acceptable_preference)) {
        thisAgent->variablize_this_chunk = FALSE;
	thisAgent->quiescence_t_flag = TRUE;
      }
      if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM]) {
          gSKI_MakeAgentCallbackXML(thisAgent, kFunctionEndTag, kTagLocal);
      }
      continue;
    }
    
    /* --- otherwise add it to the potential set --- */
    if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM]) {
      print_string (thisAgent, " --> make it a potential.");
      gSKI_MakeAgentCallbackXML(thisAgent, kFunctionBeginTag, kTagAddToPotentials);
      gSKI_MakeAgentCallbackXML(thisAgent, kFunctionEndTag, kTagAddToPotentials);
    }
    add_to_potentials (thisAgent, cond);
    
    if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM]) {
        gSKI_MakeAgentCallbackXML(thisAgent, kFunctionEndTag, kTagLocal);
    }

  } /* end of while locals loop */

  if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM]) {
    gSKI_MakeAgentCallbackXML(thisAgent, kFunctionEndTag, kTagLocals);
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
  Bool need_another_pass;
  
  if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM]) {
    print_string (thisAgent, "\n\n*** Tracing Grounded Potentials ***\n");
    gSKI_MakeAgentCallbackXML(thisAgent, kFunctionBeginTag, kTagGroundedPotentials);
  }
  
  /* --- setup the tc of the ground set --- */
  tc = get_new_tc_number(thisAgent);
  for (c=thisAgent->grounds; c!=NIL; c=c->rest) 
	  add_cond_to_tc (thisAgent, static_cast<condition_struct *>(c->first), tc, NIL, NIL); 

  need_another_pass = TRUE;
  while (need_another_pass) {
    need_another_pass = FALSE;
    /* --- look for any potentials that are in the tc now --- */
    prev_c = NIL;
    for (c=thisAgent->positive_potentials; c!=NIL; c=next_c) {
      next_c = c->rest;
      pot = static_cast<condition_struct *>(c->first);
      if (cond_is_in_tc (thisAgent, pot, tc)) {
        /* --- pot is a grounded potential, move it over to ground set --- */
        if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM]) {
          print_string (thisAgent, "\n-->Moving to grounds: ");
          print_wme (thisAgent, pot->bt.wme_);
        }
        if (prev_c) prev_c->rest = next_c; else thisAgent->positive_potentials = next_c;
        if (pot->bt.wme_->grounds_tc != thisAgent->grounds_tc) { /* add pot to grounds */
          pot->bt.wme_->grounds_tc = thisAgent->grounds_tc;
          c->rest = thisAgent->grounds; thisAgent->grounds = c;
          add_cond_to_tc (thisAgent, pot, tc, NIL, NIL);
          need_another_pass = TRUE;
        } else { /* pot was already in the grounds, do don't add it */
          free_cons (thisAgent, c);
        }
      } else {
        prev_c = c;
      }
    } /* end of for c */
  } /* end of while need_another_pass */

  if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM]) {
    gSKI_MakeAgentCallbackXML(thisAgent, kFunctionEndTag, kTagGroundedPotentials);
  }
}

/* ---------------------------------------------------------------
                     Trace Ungrounded Potentials

   This routine backtraces through ungrounded potentials.  At entry,
   all potentials must be ungrounded.  This BT's through each
   potential that has some trace (at the right level) that we can
   BT through.  Other potentials are left alone.  TRUE is returned
   if anything was BT'd; FALSE if nothing changed.
--------------------------------------------------------------- */

Bool trace_ungrounded_potentials (agent* thisAgent, goal_stack_level grounds_level) {

  /* mvp 5-17-94 */
  cons *c, *next_c, *prev_c, *prohibits;
  cons *pots_to_bt;
  condition *potential;
  preference *bt_pref, *p;

  if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM]) {
    print_string (thisAgent, "\n\n*** Tracing Ungrounded Potentials ***\n");
    gSKI_MakeAgentCallbackXML(thisAgent, kFunctionBeginTag, kTagUngroundedPotentials);
  }
  
  /* --- scan through positive potentials, pick out the ones that have
     a preference we can backtrace through --- */
  pots_to_bt = NIL;
  prev_c = NIL;
  for (c=thisAgent->positive_potentials; c!=NIL; c=next_c) {
    next_c = c->rest;
    potential = static_cast<condition_struct *>(c->first);
    bt_pref = find_clone_for_level (potential->bt.trace, 
		                            (goal_stack_level)(grounds_level+1));
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
          gSKI_MakeAgentCallbackXML(thisAgent, kFunctionEndTag, kTagUngroundedPotentials);
      }
      return FALSE;
  }

  /* --- backtrace through each one --- */
  while (pots_to_bt) {
    c = pots_to_bt;
    pots_to_bt = pots_to_bt->rest;
    potential = static_cast<condition_struct *>(c->first);
    free_cons (thisAgent, c);
    if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM]) {
      print_string (thisAgent, "\nFor ungrounded potential ");
      gSKI_MakeAgentCallbackXML(thisAgent, kFunctionBeginTag, kTagUngroundedPotential);
      print_wme (thisAgent, potential->bt.wme_);
      print_string (thisAgent, " ");
    }
    bt_pref = find_clone_for_level (potential->bt.trace, 
		                            (goal_stack_level)(grounds_level+1));

    /* mvp 5-17-94 */
    backtrace_through_instantiation (thisAgent, bt_pref->inst, grounds_level,potential,0);
    if (potential->bt.prohibits) {
      for (prohibits=potential->bt.prohibits; prohibits!=NIL; prohibits=prohibits->rest) {
        p = static_cast<preference_struct *>(prohibits->first);
        if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM]) {
          print_string (thisAgent, "     For prohibit preference: ");
          gSKI_MakeAgentCallbackXML(thisAgent, kFunctionBeginTag, kTagProhibitPreference);
          print_preference (thisAgent, p);
        }
        backtrace_through_instantiation (thisAgent, p->inst, grounds_level, potential, 6);
        
        if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM]) {
            gSKI_MakeAgentCallbackXML(thisAgent, kFunctionEndTag, kTagProhibitPreference);
        }
      }
    }
    /* mvp done */
    if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM]) {
        gSKI_MakeAgentCallbackXML(thisAgent, kFunctionEndTag, kTagUngroundedPotential);
    }
  }

  if (thisAgent->sysparams[TRACE_BACKTRACING_SYSPARAM]) {
      gSKI_MakeAgentCallbackXML(thisAgent, kFunctionEndTag, kTagUngroundedPotentials);
  }

  return TRUE;
}
