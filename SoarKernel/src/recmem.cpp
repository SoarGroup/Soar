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
 *  file:  recmem.cpp
 *
 * =======================================================================
 *  
 *             Recognition Memory (Firer and Chunker) Routines
 *                 (Does not include the Rete net)
 *
 * Init_firer() and init_chunker() should be called at startup time, to
 * do initialization.
 *
 * Do_preference_phase() runs the entire preference phase.  This is called
 * from the top-level control in main.c.
 *
 * Possibly_deallocate_instantiation() checks whether an instantiation
 * can be deallocated yet, and does so if possible.  This is used whenever
 * the (implicit) reference count on the instantiation decreases.
 * =======================================================================
 */

#include "gdatastructs.h"
#include "instantiations.h"
#include "kernel.h"
#include "mem.h"
#include "symtab.h"
#include "agent.h"
#include "prefmem.h"
#include "rhsfun.h"
#include "rete.h"
#include "print.h"
#include "production.h"
#include "wmem.h"
#include "osupport.h"
#include "recmem.h"
#include "tempmem.h"

/* JC ADDED: for gSKI events */
#include "gski_event_system_functions.h"

using namespace xmlTraceNames;

/* Uncomment the following line to get instantiation printouts */
/* #define DEBUG_INSTANTIATIONS */

/* TEMPORARY HACK (Ideally this should be doable through
   the external kernel interface but for now using a 
   couple of global STL lists to get this information
   from the rhs function to this prefference adding code)*/
wme* glbDeepCopyWMEs = NULL;   

/* mvp 5-17-94 */
/* --------------------------------------------------------------------------
            Build Prohibit Preference List for Backtracing
--------------------------------------------------------------------------*/

void build_prohibits_list (agent* thisAgent, instantiation *inst) {
  condition *cond;
  preference *pref, *new_pref;

  for (cond=inst->top_of_instantiated_conditions; cond!=NIL; cond=cond->next) {
    cond->bt.prohibits = NIL;
    if (cond->type==POSITIVE_CONDITION && cond->bt.trace) {
      if (cond->bt.trace->slot) {
        pref = cond->bt.trace->slot->preferences[PROHIBIT_PREFERENCE_TYPE];
        while (pref) {
          new_pref = NIL;
          if (pref->inst->match_goal_level == inst->match_goal_level && pref->in_tm) {
            push (thisAgent, pref, cond->bt.prohibits);
            preference_add_ref (pref);
          } else {
            new_pref = find_clone_for_level (pref, inst->match_goal_level);
            if (new_pref) {
              if (new_pref->in_tm) {
                push (thisAgent, new_pref, cond->bt.prohibits);
                preference_add_ref (new_pref);
              }
            }
          }
          pref = pref->next;
        }
      }
    }
  }
}

/* -----------------------------------------------------------------------
                         Find Clone For Level

   This routines take a given preference and finds the clone of it whose
   match goal is at the given goal_stack_level.  (This is used to find the
   proper preference to backtrace through.)  If the given preference
   itself is at the right level, it is returned.  If there is no clone at
   the right level, NIL is returned.
----------------------------------------------------------------------- */

preference *find_clone_for_level (preference *p, goal_stack_level level) {
  preference *clone;

  if (! p) {
    /* --- if the wme doesn't even have a preference on it, we can't backtrace
       at all (this happens with I/O and some architecture-created wmes --- */
    return NIL;
  }

  /* --- look at pref and all of its clones, find one at the right level --- */

  if (p->inst->match_goal_level == level) return p;

  for (clone=p->next_clone; clone!=NIL; clone=clone->next_clone)
    if (clone->inst->match_goal_level==level) return clone;

  for (clone=p->prev_clone; clone!=NIL; clone=clone->prev_clone)
    if (clone->inst->match_goal_level==level) return clone;

  /* --- if none was at the right level, we can't backtrace at all --- */
  return NIL;
}
  
/* =======================================================================

                           Firer Utilities

======================================================================= */

/* -----------------------------------------------------------------------
                             Find Match Goal

   Given an instantiation, this routines looks at the instantiated
   conditions to find its match goal.  It fills in inst->match_goal and
   inst->match_goal_level.  If there is a match goal, match_goal is set
   to point to the goal identifier.  If no goal was matched, match_goal
   is set to NIL and match_goal_level is set to ATTRIBUTE_IMPASSE_LEVEL.
----------------------------------------------------------------------- */

void find_match_goal (instantiation *inst) {
  Symbol *lowest_goal_so_far;
  goal_stack_level lowest_level_so_far;
  condition *cond;
  Symbol *id;
  
  lowest_goal_so_far = NIL;
  lowest_level_so_far = -1;
  for (cond=inst->top_of_instantiated_conditions; cond!=NIL; cond=cond->next)
    if (cond->type==POSITIVE_CONDITION) {
      id = cond->bt.wme_->id;
      if (id->id.isa_goal)
        if (cond->bt.level > lowest_level_so_far) {
          lowest_goal_so_far = id;
          lowest_level_so_far = cond->bt.level;
        }
    }
  
  inst->match_goal = lowest_goal_so_far;
  if (lowest_goal_so_far)
    inst->match_goal_level = lowest_level_so_far;
  else
    inst->match_goal_level = ATTRIBUTE_IMPASSE_LEVEL;
}

/* -----------------------------------------------------------------------

               Executing the RHS Actions of an Instantiation

   Execute_action() executes a given RHS action.  For MAKE_ACTION's, it
   returns the created preference structure, or NIL if an error occurs.
   For FUNCALL_ACTION's, it returns NIL.

   Instantiate_rhs_value() returns the (symbol) instantiation of an
   rhs_value, or NIL if an error occurs.  It takes a new_id_level
   argument indicating what goal_stack_level a new id is to be created
   at, in case a gensym is needed for the instantiation of a variable.
   (although I'm not sure this is really needed.)

   As rhs unbound variables are encountered, they are instantiated with
   new gensyms.  These gensyms are then stored in the rhs_variable_bindings
   array, so if the same unbound variable is encountered a second time
   it will be instantiated with the same gensym.
----------------------------------------------------------------------- */


Symbol *instantiate_rhs_value (agent* thisAgent, rhs_value rv, 
			       goal_stack_level new_id_level,
                               char new_id_letter,
                               struct token_struct *tok, wme *w) {
  list *fl;
  list *arglist;
  cons *c, *prev_c, *arg_cons;
  rhs_function *rf;
  Symbol *result;
  Bool nil_arg_found;
  
  if (rhs_value_is_symbol(rv)) {
    result = rhs_value_to_symbol(rv);
    symbol_add_ref (result);
    return result;
  }

  if (rhs_value_is_unboundvar(rv)) {
    long index;
    Symbol *sym;

    index = rhs_value_to_unboundvar(rv);
    if (thisAgent->firer_highest_rhs_unboundvar_index < index)
      thisAgent->firer_highest_rhs_unboundvar_index = index;
    sym = *(thisAgent->rhs_variable_bindings+index);

    if (!sym) {
      sym = make_new_identifier (thisAgent, new_id_letter, new_id_level);
      *(thisAgent->rhs_variable_bindings+index) = sym;
      return sym;
    } else if (sym->common.symbol_type==VARIABLE_SYMBOL_TYPE) {
      new_id_letter = *(sym->var.name + 1);
      sym = make_new_identifier (thisAgent, new_id_letter, new_id_level);
      *(thisAgent->rhs_variable_bindings+index) = sym;
      return sym;
    } else {
      symbol_add_ref (sym);
      return sym;
    }
  }

  if (rhs_value_is_reteloc(rv)) {
    result = get_symbol_from_rete_loc ((unsigned short) rhs_value_to_reteloc_levels_up(rv),
                                       (byte)rhs_value_to_reteloc_field_num(rv),
                                       tok, w);
    symbol_add_ref (result);
    return result;
  }

  fl = rhs_value_to_funcall_list(rv);
  rf = static_cast<rhs_function_struct *>(fl->first);

  /* --- build up a list of the argument values --- */
  prev_c = NIL;
  nil_arg_found = FALSE;
  arglist = NIL; /* unnecessary, but gcc -Wall warns without it */
  for (arg_cons=fl->rest; arg_cons!=NIL; arg_cons=arg_cons->rest) {
    allocate_cons (thisAgent, &c);
    c->first = instantiate_rhs_value (thisAgent, 
									  static_cast<char *>(arg_cons->first), 
									  new_id_level, new_id_letter, tok, w);
    if (! c->first) nil_arg_found = TRUE;
    if (prev_c) prev_c->rest = c; else arglist = c;
    prev_c = c;
  }
  if (prev_c) prev_c->rest = NIL; else arglist = NIL;

  /* --- if all args were ok, call the function --- */

  if (!nil_arg_found) {
    // stop the kernel timer while doing RHS funcalls  KJC 11/04
    // the total_cpu timer needs to be updated in case RHS fun is statsCmd
    #ifndef NO_TIMING_STUFF
    stop_timer (thisAgent, &thisAgent->start_kernel_tv,
		&thisAgent->total_kernel_time);
    stop_timer (thisAgent, &thisAgent->start_total_tv,
		    &thisAgent->total_cpu_time);
    start_timer (thisAgent, &thisAgent->start_total_tv);
    #endif

    result = (*(rf->f))(thisAgent, arglist, rf->user_data);

    #ifndef NO_TIMING_STUFF  // restart the kernel timer
    start_timer (thisAgent, &thisAgent->start_kernel_tv);
    #endif

  } else
    result = NIL;
  
  /* --- scan through arglist, dereference symbols and deallocate conses --- */
  for (c=arglist; c!=NIL; c=c->rest)
    if (c->first) symbol_remove_ref (thisAgent, (Symbol *)(c->first));
  free_list (thisAgent, arglist);

  return result;
}

preference *execute_action (agent* thisAgent, action *a, struct token_struct *tok, wme *w) {
  Symbol *id, *attr, *value, *referent;
  char first_letter;
  
  if (a->type==FUNCALL_ACTION) {
    value = instantiate_rhs_value (thisAgent, a->value, -1, 'v', tok, w);
    if (value) symbol_remove_ref (thisAgent, value);
    return NIL;
  }

  attr = NIL;
  value = NIL;
  referent = NIL;
  
  id = instantiate_rhs_value (thisAgent, a->id, -1, 's', tok, w);
  if (!id) goto abort_execute_action;
  if (id->common.symbol_type!=IDENTIFIER_SYMBOL_TYPE) {
    print_with_symbols (thisAgent, "Error: RHS makes a preference for %y (not an identifier)\n", id);
    goto abort_execute_action;
  }
  
  attr = instantiate_rhs_value (thisAgent, a->attr, id->id.level, 'a', tok, w);
  if (!attr) goto abort_execute_action;

  first_letter = first_letter_from_symbol (attr);
  
  value = instantiate_rhs_value (thisAgent, a->value, id->id.level,
                                 first_letter, tok, w);
  if (!value) goto abort_execute_action;

  if (preference_is_binary(a->preference_type)) {
    referent = instantiate_rhs_value (thisAgent, a->referent, id->id.level,
                                      first_letter, tok, w);
    if (!referent) goto abort_execute_action;
  }

  /* --- RBD 4/17/95 added stuff to handle attribute_preferences_mode --- */
  if (((a->preference_type != ACCEPTABLE_PREFERENCE_TYPE) &&
     (a->preference_type != REJECT_PREFERENCE_TYPE)) &&
     (! (id->id.isa_goal && (attr==thisAgent->operator_symbol)) )) {
     if ((thisAgent->attribute_preferences_mode==2) ||
        (thisAgent->operand2_mode ==TRUE) ) {
        print_with_symbols (thisAgent, "\nError: attribute preference other than +/- for %y ^%y -- ignoring it.", id, attr);
        goto abort_execute_action;
     } else if (thisAgent->attribute_preferences_mode==1) {
        print_with_symbols (thisAgent, "\nWarning: attribute preference other than +/- for %y ^%y.", id, attr);

		growable_string gs = make_blank_growable_string(thisAgent);
		add_to_growable_string(thisAgent, &gs, "Warning: attribute preference other than +/- for ");
		add_to_growable_string(thisAgent, &gs, symbol_to_string(thisAgent, id, true, 0, 0));
		add_to_growable_string(thisAgent, &gs, " ^");
		add_to_growable_string(thisAgent, &gs, symbol_to_string(thisAgent, attr, true, 0, 0));
		add_to_growable_string(thisAgent, &gs, ".");
		GenerateWarningXML(thisAgent, text_of_growable_string(gs));
		free_growable_string(thisAgent, gs);

     }  
  }

  return make_preference (thisAgent, a->preference_type, id, attr, value, referent);

  abort_execute_action:   /* control comes here when some error occurred */
  if (id) symbol_remove_ref (thisAgent, id);  
  if (attr) symbol_remove_ref (thisAgent, attr);  
  if (value) symbol_remove_ref (thisAgent, value);  
  if (referent) symbol_remove_ref (thisAgent, referent);
  return NIL;
}

/* -----------------------------------------------------------------------
                    Fill In New Instantiation Stuff

   This routine fills in a newly created instantiation structure with
   various information.   At input, the instantiation should have:
     - preferences_generated filled in; 
     - instantiated conditions filled in;
     - top-level positive conditions should have bt.wme_, bt.level, and
       bt.trace filled in, but bt.wme_ and bt.trace shouldn't have their
       reference counts incremented yet.

   This routine does the following:
     - increments reference count on production;
     - fills in match_goal and match_goal_level;
     - for each top-level positive cond:
         replaces bt.trace with the preference for the correct level,
         updates reference counts on bt.pref and bt.wmetraces and wmes
     - for each preference_generated, adds that pref to the list of all
       pref's for the match goal
     - fills in backtrace_number;   
     - if "need_to_do_support_calculations" is TRUE, calculates o-support
       for preferences_generated;
----------------------------------------------------------------------- */

void fill_in_new_instantiation_stuff (agent* thisAgent, instantiation *inst,
                                      Bool need_to_do_support_calculations) {
  condition *cond;
  preference *p;
  goal_stack_level level;

  production_add_ref (inst->prod);
  
  find_match_goal (inst);

  level = inst->match_goal_level;

  /* Note: since we'll never backtrace through instantiations at the top
     level, it might make sense to not increment the reference counts
     on the wmes and preferences here if the instantiation is at the top
     level.  As it stands now, we could gradually accumulate garbage at
     the top level if we have a never-ending sequence of production
     firings at the top level that chain on each other's results.  (E.g.,
     incrementing a counter on every decision cycle.)  I'm leaving it this
     way for now, because if we go to S-Support, we'll (I think) need to
     save these around (maybe). */

  /* KJC 6/00:  maintaining all the top level ref cts does have a big
     impact on memory pool usage and also performance (due to malloc).
	 (See tests done by Scott Wallace Fall 99.)	 Therefore added 
	 preprocessor macro so that by unsetting macro the top level ref cts are not 
	 incremented.  It's possible that in some systems, these ref cts may 
	 be desireable: they can be added by defining DO_TOP_LEVEL_REF_CTS
	 */

  for (cond=inst->top_of_instantiated_conditions; cond!=NIL; cond=cond->next)
    if (cond->type==POSITIVE_CONDITION) {		
        #ifdef DO_TOP_LEVEL_REF_CTS
		wme_add_ref (cond->bt.wme_);
        #else
		if (level > TOP_GOAL_LEVEL) wme_add_ref (cond->bt.wme_);
        #endif
 		/* --- if trace is for a lower level, find one for this level --- */
      if (cond->bt.trace) {
        if (cond->bt.trace->inst->match_goal_level > level) { 
          cond->bt.trace = find_clone_for_level (cond->bt.trace, level);
		} 
        #ifdef DO_TOP_LEVEL_REF_CTS
		if (cond->bt.trace) preference_add_ref (cond->bt.trace);
        #else
		if ((cond->bt.trace) && (level > TOP_GOAL_LEVEL)) 
			preference_add_ref (cond->bt.trace);
        #endif
      }
    }



  if (inst->match_goal) {
    for (p=inst->preferences_generated; p!=NIL; p=p->inst_next) {
      insert_at_head_of_dll (inst->match_goal->id.preferences_from_goal, p,
                             all_of_goal_next, all_of_goal_prev);
      p->on_goal_list = TRUE;
    }
  }
  inst->backtrace_number = 0;

  if ((thisAgent->o_support_calculation_type == 0) ||
	  (thisAgent->o_support_calculation_type == 3) ||
	  (thisAgent->o_support_calculation_type == 4))  {
    /* --- do calc's the normal Soar 6 way --- */  
    if (need_to_do_support_calculations)
      calculate_support_for_instantiation_preferences (thisAgent, inst);
  } else if (thisAgent->o_support_calculation_type == 1) {
    if (need_to_do_support_calculations)
      calculate_support_for_instantiation_preferences (thisAgent, inst);
    /* --- do calc's both ways, warn on differences --- */
    if ((inst->prod->declared_support!=DECLARED_O_SUPPORT) &&
        (inst->prod->declared_support!=DECLARED_I_SUPPORT)) {
      /* --- At this point, we've done them the normal way.  To look for
             differences, save o-support flags on a list, then do Doug's
             calculations, then compare and restore saved flags. --- */
      list *saved_flags;
      preference *pref;
      Bool difference_found;
      saved_flags = NIL;
      for (pref=inst->preferences_generated; pref!=NIL; pref=pref->inst_next)
        push (thisAgent, (pref->o_supported ? pref : NIL), saved_flags);
      saved_flags = destructively_reverse_list (saved_flags);
      dougs_calculate_support_for_instantiation_preferences (thisAgent, inst);
      difference_found = FALSE;
      for (pref=inst->preferences_generated; pref!=NIL; pref=pref->inst_next){
        cons *c; Bool b;
        c = saved_flags; saved_flags = c->rest;
        b = (c->first ? TRUE : FALSE); free_cons (thisAgent, c);
        if (pref->o_supported != b) difference_found = TRUE;
        pref->o_supported = b;
      }
      if (difference_found) {
        print_with_symbols(thisAgent, "\n*** O-support difference found in production %y",
                           inst->prod->name);
      }
    }
  }
  else {
    /* --- do calc's Doug's way --- */
    if ((inst->prod->declared_support!=DECLARED_O_SUPPORT) &&
        (inst->prod->declared_support!=DECLARED_I_SUPPORT)) {
      dougs_calculate_support_for_instantiation_preferences (thisAgent, inst);
    }
  }
}

/* =======================================================================

                          Main Firer Routines

   Init_firer() should be called at startup time.  Do_preference_phase()
   is called from the top level to run the whole preference phase.

   Preference phase follows this sequence:

   (1) Productions are fired for new matches.  As productions are fired,
   their instantiations are stored on the list newly_created_instantiations,
   linked via the "next" fields in the instantiation structure.  No
   preferences are actually asserted yet.
   
   (2) Instantiations are retracted; their preferences are retracted.

   (3) Preferences (except o-rejects) from newly_created_instantiations
   are asserted, and these instantiations are removed from the 
   newly_created_instantiations list and moved over to the per-production
   lists of instantiations of that production.

   (4) Finally, o-rejects are processed.

   Note: Using the O_REJECTS_FIRST flag, step (4) becomes step (2b)
======================================================================= */

void init_firer (agent* thisAgent) {
  init_memory_pool (thisAgent, &thisAgent->instantiation_pool, sizeof(instantiation),
                    "instantiation");
}

/* --- Macro returning TRUE iff we're supposed to trace firings for the
   given instantiation, which should have the "prod" field filled in. --- */
#ifdef USE_MACROS
#define trace_firings_of_inst(thisAgent, inst) \
  ((inst)->prod && \
   (thisAgent->sysparams[TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM+(inst)->prod->type] || \
    ((inst)->prod->trace_firings)))
#else
inline Bool trace_firings_of_inst(agent* thisAgent, instantiation * inst)
{
  return ((inst)->prod &&
    (thisAgent->sysparams[TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM+(inst)->prod->type] ||
    ((inst)->prod->trace_firings)));
}
#endif

/* -----------------------------------------------------------------------
                         Create Instantiation

   This builds the instantiation for a new match, and adds it to
   newly_created_instantiations.  It also calls chunk_instantiation() to
   do any necessary chunk or justification building.
----------------------------------------------------------------------- */

void create_instantiation (agent* thisAgent, production *prod,
                           struct token_struct *tok,
                           wme *w) {
   instantiation *inst;
   condition *cond;
   preference *pref;
   action *a;
   cons *c;
   Bool need_to_do_support_calculations;
   Bool trace_it;
   long index;
   Symbol **cell;

#ifdef BUG_139_WORKAROUND
    /* RPM workaround for bug #139: don't fire justifications */
    if (prod->type == JUSTIFICATION_PRODUCTION_TYPE) {
        return;
    }
#endif

   allocate_with_pool (thisAgent, &thisAgent->instantiation_pool, &inst);
   inst->next = thisAgent->newly_created_instantiations;
   thisAgent->newly_created_instantiations = inst;
   inst->prod = prod;
   inst->rete_token = tok;
   inst->rete_wme = w;
   inst->okay_to_variablize = TRUE;
   inst->in_ms = TRUE;


   /* REW: begin   09.15.96 */
   /*  We want to initialize the GDS_evaluated_already flag
    *  when a new instantiation is created.
    */

   inst->GDS_evaluated_already = FALSE;

   if (thisAgent->operand2_mode == TRUE) {
       if (thisAgent->soar_verbose_flag == TRUE) {
         print_with_symbols(thisAgent, "\n   in create_instantiation: %y",
               inst->prod->name);
         char buf[256];
         snprintf(buf, 254, "in create_instantiation: %s", symbol_to_string(thisAgent, inst->prod->name, true, 0, 0));
         GenerateVerboseXML(thisAgent, buf);
       }
   }
   /* REW: end   09.15.96 */


   thisAgent->production_being_fired = inst->prod;
   prod->firing_count++;
   thisAgent->production_firing_count++;

   /* --- build the instantiated conditions, and bind LHS variables --- */
   p_node_to_conditions_and_nots (thisAgent, prod->p_node, tok, w,
         &(inst->top_of_instantiated_conditions),
         &(inst->bottom_of_instantiated_conditions),
         &(inst->nots), NIL);

   /* --- record the level of each of the wmes that was positively tested --- */
   for (cond=inst->top_of_instantiated_conditions; cond!=NIL; cond=cond->next) {
      if (cond->type==POSITIVE_CONDITION) {
         cond->bt.level = cond->bt.wme_->id->id.level;
         cond->bt.trace = cond->bt.wme_->preference;
      }
   }

   /* --- print trace info --- */
   trace_it = trace_firings_of_inst (thisAgent, inst);
   if (trace_it) {
      if (get_printer_output_column(thisAgent)!=1) print (thisAgent, "\n");  /* AGR 617/634 */
      print (thisAgent, "Firing ");
      print_instantiation_with_wmes
         (thisAgent, inst, 
		 (wme_trace_type)(thisAgent->sysparams[TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM]), 0);
   }

   /* --- initialize rhs_variable_bindings array with names of variables
      (if there are any stored on the production -- for chunks there won't
      be any) --- */
   index = 0;
   cell = thisAgent->rhs_variable_bindings;
   for (c=prod->rhs_unbound_variables; c!=NIL; c=c->rest) {
      *(cell++) = static_cast<symbol_union *>(c->first);
      index++;
   }
   thisAgent->firer_highest_rhs_unboundvar_index = index - 1;

   /* 7.1/8 merge: Not sure about this.  This code in 704, but not in either 7.1 or 703/soar8 */
   /* --- Before executing the RHS actions, tell the user that the -- */
   /* --- phase has changed to output by printing the arrow --- */
   if (trace_it && thisAgent->sysparams[TRACE_FIRINGS_PREFERENCES_SYSPARAM]) {
      print (thisAgent, " -->\n");
	  gSKI_MakeAgentCallbackXML(thisAgent, kFunctionBeginTag, kTagActionSideMarker);
	  gSKI_MakeAgentCallbackXML(thisAgent, kFunctionEndTag, kTagActionSideMarker);
   }

   /* --- execute the RHS actions, collect the results --- */
   inst->preferences_generated = NIL;
   need_to_do_support_calculations = FALSE;
   for (a=prod->action_list; a!=NIL; a=a->next) {
      pref = execute_action (thisAgent, a, tok, w);
	  /* SoarTech changed from an IF stmt to a WHILE loop to support GlobalDeepCpy */
      while (pref) {   
         pref->inst = inst;
         insert_at_head_of_dll (inst->preferences_generated, pref,
               inst_next, inst_prev);
         if (inst->prod->declared_support==DECLARED_O_SUPPORT)
            pref->o_supported = TRUE;
         else if (inst->prod->declared_support==DECLARED_I_SUPPORT)
         {  
            pref->o_supported = FALSE;
         }
         else {

            if (thisAgent->operand2_mode == TRUE) {
               pref->o_supported =
                  (thisAgent->FIRING_TYPE == PE_PRODS) ? TRUE : FALSE;
            }
            /* REW: end   09.15.96 */

            else {
               if (a->support==O_SUPPORT) pref->o_supported = TRUE;
               else if (a->support==I_SUPPORT) pref->o_supported = FALSE;
               else {
                  need_to_do_support_calculations = TRUE;
                  if (thisAgent->soar_verbose_flag == TRUE) {
                     printf("\n\nin create_instantiation():  need_to_do_support_calculations == TRUE!!!\n\n");
                     GenerateVerboseXML(thisAgent, "in create_instantiation():  need_to_do_support_calculations == TRUE!!!");
                  }
               }

            }

         }

         /* TEMPORARY HACK (Ideally this should be doable through
            the external kernel interface but for now using a 
            couple of global STL lists to get this information
            from the rhs function to this prefference adding code)

            Getting the next pref from the set of possible prefs 
            added by the deep copy rhs function */
           if ( glbDeepCopyWMEs != 0 ) {
            wme* tempwme = glbDeepCopyWMEs;
            pref = make_preference(thisAgent, 
                                   a->preference_type, 
                                   tempwme->id, 
                                   tempwme->attr, 
                                   tempwme->value, 0);
            glbDeepCopyWMEs = tempwme->next;
            deallocate_wme(thisAgent, tempwme);
         } else {
            pref = 0;
         }
        }
   }

   /* --- reset rhs_variable_bindings array to all zeros --- */
   index = 0;
   cell = thisAgent->rhs_variable_bindings;
   while (index++ <= thisAgent->firer_highest_rhs_unboundvar_index) *(cell++) = NIL;

   /* --- fill in lots of other stuff --- */
   fill_in_new_instantiation_stuff (thisAgent, inst, need_to_do_support_calculations);

   /* --- print trace info: printing preferences --- */
   /* Note: can't move this up, since fill_in_new_instantiation_stuff gives
      the o-support info for the preferences we're about to print */
   if (trace_it && thisAgent->sysparams[TRACE_FIRINGS_PREFERENCES_SYSPARAM]) {
      for (pref=inst->preferences_generated; pref!=NIL; pref=pref->inst_next) {
         print (thisAgent, " ");
         print_preference (thisAgent, pref);
      }
   }

   /* mvp 5-17-94 */
   build_prohibits_list (thisAgent, inst);

   thisAgent->production_being_fired = NIL;

   /* --- build chunks/justifications if necessary --- */
   chunk_instantiation (thisAgent, inst, thisAgent->sysparams[LEARNING_ON_SYSPARAM] != 0);

    /* MVP 6-8-94 */
   if (!thisAgent->system_halted) {
      /* --- invoke callback function --- */
      soar_invoke_callbacks(thisAgent, thisAgent, 
            FIRING_CALLBACK,
            (soar_call_data) inst);

   }
 

   /* JC ADDED: Need to tell gSKI that a production was fired */
   gSKI_MakeAgentCallback(gSKI_K_EVENT_PRODUCTION_FIRED, 1, thisAgent, static_cast<void*>(inst));
}

/* -----------------------------------------------------------------------
                        Deallocate Instantiation

   This deallocates the given instantiation.  This should only be invoked
   via the possibly_deallocate_instantiation() macro.
----------------------------------------------------------------------- */

void deallocate_instantiation (agent* thisAgent, instantiation *inst) {
  condition *cond;

  /* mvp 5-17-94 */
  list *c, *c_old;
  preference *pref;
  goal_stack_level level;
  
  level = inst->match_goal_level;
 
#ifdef DEBUG_INSTANTIATIONS
  if (inst->prod)
    print_with_symbols (thisAgent, "\nDeallocate instantiation of %y",inst->prod->name);
#endif

  for (cond=inst->top_of_instantiated_conditions; cond!=NIL; cond=cond->next)
    if (cond->type==POSITIVE_CONDITION) {

      /* mvp 6-22-94, modified 94.01.17 by AGR with lotsa help from GAP */
     if (cond->bt.prohibits) {
       c_old = c = cond->bt.prohibits;
       cond->bt.prohibits = NIL;
       for (; c!=NIL; c=c->rest) {
 		   pref = (preference *) c->first;
           #ifdef DO_TOP_LEVEL_REF_CTS
		   preference_remove_ref (thisAgent, pref);
           #else
	       if (level > TOP_GOAL_LEVEL)  preference_remove_ref (thisAgent, pref);
           #endif
       }
       free_list (thisAgent, c_old);
     }
     /* mvp done */  

     #ifdef DO_TOP_LEVEL_REF_CTS
       wme_remove_ref (thisAgent, cond->bt.wme_);
       if (cond->bt.trace) preference_remove_ref (thisAgent, cond->bt.trace);
     #else
	   if (level > TOP_GOAL_LEVEL) {
		   wme_remove_ref (thisAgent, cond->bt.wme_);
           if (cond->bt.trace) preference_remove_ref (thisAgent, cond->bt.trace);
	   }
     #endif
   }

  deallocate_condition_list (thisAgent, inst->top_of_instantiated_conditions);
  deallocate_list_of_nots (thisAgent, inst->nots);
  if (inst->prod) production_remove_ref (thisAgent, inst->prod);
  free_with_pool (&thisAgent->instantiation_pool, inst);
}

/* -----------------------------------------------------------------------
                         Retract Instantiation

   This retracts the given instantiation.
----------------------------------------------------------------------- */

void retract_instantiation (agent* thisAgent, instantiation *inst) {
  preference *pref, *next;
  Bool retracted_a_preference;
  Bool trace_it;

  /* --- invoke callback function --- */
  soar_invoke_callbacks(thisAgent, thisAgent, 
			RETRACTION_CALLBACK,
			(soar_call_data) inst);
   
  /* JC ADDED: tell gSKI that we've retracted a production instantiation */
  gSKI_MakeAgentCallback(gSKI_K_EVENT_PRODUCTION_RETRACTED, 0, thisAgent, static_cast<void*>(inst));


  retracted_a_preference = FALSE;
  
  trace_it = trace_firings_of_inst (thisAgent, inst);

  /* --- retract any preferences that are in TM and aren't o-supported --- */
  pref = inst->preferences_generated;

  while (pref!=NIL) {
    next = pref->inst_next;
    if (pref->in_tm && (! pref->o_supported)) {

      if (trace_it) {
        if (!retracted_a_preference) {
			if (get_printer_output_column(thisAgent)!=1) print (thisAgent, "\n");  /* AGR 617/634 */
			print (thisAgent, "Retracting ");
            print_instantiation_with_wmes (thisAgent, inst, 
				(wme_trace_type)thisAgent->sysparams[TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM],1);
			if (thisAgent->sysparams[TRACE_FIRINGS_PREFERENCES_SYSPARAM]) {
				print (thisAgent, " -->\n");
				gSKI_MakeAgentCallbackXML(thisAgent, kFunctionBeginTag, kTagActionSideMarker);
				gSKI_MakeAgentCallbackXML(thisAgent, kFunctionEndTag, kTagActionSideMarker);
			}
		}
        if (thisAgent->sysparams[TRACE_FIRINGS_PREFERENCES_SYSPARAM]) {
          print (thisAgent, " ");
          print_preference (thisAgent, pref);
        }
      }

      remove_preference_from_tm (thisAgent, pref);
      retracted_a_preference = TRUE;
    }
    pref = next;
  }

  /* --- remove inst from list of instantiations of this production --- */
  remove_from_dll (inst->prod->instantiations, inst, next, prev);

  /* --- if retracting a justification, excise it --- */
  /*
   * if the reference_count on the production is 1 (or less) then the
   * only thing supporting this justification is the instantiation, hence
   * it has already been excised, and doing it again is wrong.
   */
  if (inst->prod->type==JUSTIFICATION_PRODUCTION_TYPE &&
      inst->prod->reference_count > 1)
    excise_production (thisAgent, inst->prod, FALSE);
  
  /* --- mark as no longer in MS, and possibly deallocate  --- */
  inst->in_ms = FALSE;
  possibly_deallocate_instantiation (thisAgent, inst);
}

/* -----------------------------------------------------------------------
                         Assert New Preferences

   This routine scans through newly_created_instantiations, asserting
   each preference generated except for o-rejects.  It also removes
   each instantiation from newly_created_instantiations, linking each
   onto the list of instantiations for that particular production.
   O-rejects are bufferred and handled after everything else.

   Note that some instantiations on newly_created_instantiations are not
   in the match set--for the initial instantiations of chunks/justifications,
   if they don't match WM, we have to assert the o-supported preferences
   and throw away the rest.
----------------------------------------------------------------------- */

void assert_new_preferences (agent* thisAgent) 
{
   instantiation *inst, *next_inst;
   preference *pref, *next_pref;
   preference *o_rejects;
   
   o_rejects = NIL;  
   
   
   /* REW: begin 09.15.96 */
   if ((thisAgent->operand2_mode == TRUE) &&
       (thisAgent->soar_verbose_flag == TRUE)) {
           printf("\n   in assert_new_preferences:");
           GenerateVerboseXML(thisAgent, "in assert_new_preferences:");
       }
   /* REW: end   09.15.96 */
   
#ifdef O_REJECTS_FIRST
    {

        slot *s;
        preference *p, *next_p;

        /* Do an initial loop to process o-rejects, then re-loop
           to process normal preferences.  No buffering should be needed.
         */
        for (inst = thisAgent->newly_created_instantiations; inst != NIL; inst = next_inst) {
            next_inst = inst->next;

            for (pref = inst->preferences_generated; pref != NIL; pref = next_pref) {
                next_pref = pref->inst_next;
                if ((pref->type == REJECT_PREFERENCE_TYPE) && (pref->o_supported)) {
                    /* --- o-reject: just put it in the buffer for later --- */

                    s = find_slot(pref->id, pref->attr);
                    if (s) {
                        /* --- remove all pref's in the slot that have the same value --- */
                        p = s->all_preferences;
                        while (p) {
                            next_p = p->all_of_slot_next;
                            if (p->value == pref->value)
                                remove_preference_from_tm(thisAgent, p);
                            p = next_p;
                        }
                    }
                }
            }
        }
    }
#endif
   
   for (inst=thisAgent->newly_created_instantiations;
        inst!=NIL;
        inst=next_inst) 
   {
      next_inst = inst->next;
      if (inst->in_ms)
         insert_at_head_of_dll (inst->prod->instantiations, inst, next, prev);
      
      /* REW: begin 09.15.96 */
      if (thisAgent->operand2_mode == TRUE) 
      {
          if (thisAgent->soar_verbose_flag == TRUE) {
            print_with_symbols(thisAgent, "\n      asserting instantiation: %y\n",
            inst->prod->name);
            char buf[256];
            snprintf(buf, 254, "asserting instantiation: %s", symbol_to_string(thisAgent, inst->prod->name, true, 0, 0));
            GenerateVerboseXML(thisAgent, buf);
          }
      }
      /* REW: end   09.15.96 */
      
      for (pref=inst->preferences_generated; pref!=NIL; pref=next_pref) 
      {
         next_pref = pref->inst_next;
         if ((pref->type==REJECT_PREFERENCE_TYPE) && (pref->o_supported)) 
         {
#ifndef O_REJECTS_FIRST
            /* --- o-reject: just put it in the buffer for later --- */
            pref->next = o_rejects;
            o_rejects = pref;
#endif            
            
            /* REW: begin 09.15.96 */
            /* No knowledge retrieval necessary in Operand2 */
            /* REW: end   09.15.96 */
            
         } 
         else if (inst->in_ms || pref->o_supported) 
         {
            /* --- normal case --- */
            add_preference_to_tm (thisAgent, pref);
            
            
            /* REW: begin 09.15.96 */
            /* No knowledge retrieval necessary in Operand2 */
            /* REW: end   09.15.96 */
            
            
         } 
         else 
         {
         /* --- inst. is refracted chunk, and pref. is not o-supported:
            remove the preference --- */
            
            /* --- first splice it out of the clones list--otherwise we might
            accidentally deallocate some clone that happens to have refcount==0
            just because it hasn't been asserted yet --- */
            
            if (pref->next_clone) 
               pref->next_clone->prev_clone = pref->prev_clone;
            if (pref->prev_clone) 
               pref->prev_clone->next_clone = pref->next_clone;
            pref->next_clone = pref->prev_clone = NIL;
            
            /* --- now add then remove ref--this should result in deallocation */
            preference_add_ref (pref);
            preference_remove_ref (thisAgent, pref);
         }
      }
   }
   
#ifndef O_REJECTS_FIRST
   if (o_rejects) 
      process_o_rejects_and_deallocate_them (thisAgent, o_rejects);
#endif
}

/* -----------------------------------------------------------------------
                          Do Preference Phase

   This routine is called from the top level to run the preference phase.
----------------------------------------------------------------------- */

void do_preference_phase (agent* thisAgent) {
  production *prod;
  struct token_struct *tok;
  wme *w;
  instantiation *inst;


/* AGR 617/634:  These are 2 bug reports that report the same problem,
   namely that when 2 chunk firings happen in succession, there is an
   extra newline printed out.  The simple fix is to monitor
   get_printer_output_column and see if it's at the beginning of a line
   or not when we're ready to print a newline.  94.11.14 */


  if (thisAgent->sysparams[TRACE_PHASES_SYSPARAM]) {
	  if (thisAgent->operand2_mode == TRUE) {
		  if (thisAgent->current_phase == APPLY_PHASE) {  /* it's always IE for PROPOSE */
			  gSKI_MakeAgentCallbackXML(thisAgent, kFunctionBeginTag, kTagSubphase);
			  gSKI_MakeAgentCallbackXML(thisAgent, kFunctionAddAttribute, kPhase_Name, kSubphaseName_FiringProductions);
			  switch (thisAgent->FIRING_TYPE) {
					case PE_PRODS:
						print (thisAgent, "\t--- Firing Productions (PE) ---\n",0);
						gSKI_MakeAgentCallbackXML(thisAgent, kFunctionAddAttribute, kPhase_FiringType, kPhaseFiringType_PE);
						break;
					case IE_PRODS:
						print (thisAgent, "\t--- Firing Productions (IE) ---\n",0);
						gSKI_MakeAgentCallbackXML(thisAgent, kFunctionAddAttribute, kPhase_FiringType, kPhaseFiringType_IE);
						break;
			  }
			  gSKI_MakeAgentCallbackXML(thisAgent, kFunctionEndTag, kTagSubphase);
		  }
	  }
	  else
		  // the XML for this is generated in this function
		  print_phase (thisAgent, "\n--- Preference Phase ---\n",0);
  }


  thisAgent->newly_created_instantiations = NIL;

  /* MVP 6-8-94 */
  while (get_next_assertion (thisAgent, &prod, &tok, &w)) {
     if (thisAgent->max_chunks_reached) {
       thisAgent->system_halted = TRUE;
	   	  soar_invoke_callbacks(thisAgent, thisAgent, 
		  AFTER_HALT_SOAR_CALLBACK,
		  (soar_call_data) NULL);
       return;
     }
     create_instantiation (thisAgent, prod, tok, w);
   }

  assert_new_preferences (thisAgent);

  while (get_next_retraction (thisAgent, &inst))
    retract_instantiation (thisAgent, inst);

/* REW: begin 08.20.97 */

  /*  In Waterfall, if there are nil goal retractions, then we want to 
      retract them as well, even though they are not associated with any
      particular goal (because their goal has been deleted). The 
      functionality of this separate routine could have been easily 
      combined in get_next_retraction but I wanted to highlight the 
      distinction between regualr retractions (those that can be 
      mapped onto a goal) and nil goal retractions that require a
      special data strucutre (because they don't appear on any goal) 
      REW.  */

  if (thisAgent->operand2_mode && thisAgent->nil_goal_retractions) {
    while (get_next_nil_goal_retraction (thisAgent, &inst))
      retract_instantiation (thisAgent, inst);
  }

/* REW: end   08.20.97 */

  if (thisAgent->sysparams[TRACE_PHASES_SYSPARAM]) {
     if (! thisAgent->operand2_mode) {
 	  print_phase (thisAgent, "\n--- END Preference Phase ---\n",1);
	 }
  }

}

