/*************************************************************************
 *
 *  file:  recmem.c
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
 *
 * Copyright 1995-2004 Carnegie Mellon University,
 *										 University of Michigan,
 *										 University of Southern California/Information
 *										 Sciences Institute. All rights reserved.
 *										
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1.	Redistributions of source code must retain the above copyright notice,
 *		this list of conditions and the following disclaimer. 
 * 2.	Redistributions in binary form must reproduce the above copyright notice,
 *		this list of conditions and the following disclaimer in the documentation
 *		and/or other materials provided with the distribution. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE SOAR CONSORTIUM ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE SOAR CONSORTIUM  OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * The views and conclusions contained in the software and documentation are
 * those of the authors and should not be interpreted as representing official
 * policies, either expressed or implied, of Carnegie Mellon University, the
 * University of Michigan, the University of Southern California/Information
 * Sciences Institute, or the Soar consortium.
 * =======================================================================
 */

#include "soarkernel.h"
#include <ctype.h>

#ifdef __hpux
#ifndef _INCLUDE_POSIX_SOURCE
#define _INCLUDE_POSIX_SOURCE
#endif
#define _INCLUDE_XOPEN_SOURCE
#define _INCLUDE_HPUX_SOURCE
#include <sys/types.h>
#undef  _INCLUDE_POSIX_SOURCE
#undef  _INCLUDE_XOPEN_SOURCE
#endif                          /* __hpux */
#if !defined(__SC__) && !defined(THINK_C) && !defined(WIN32) && !defined(MACINTOSH)
#include <sys/time.h>
#endif                          /* !__SC__ && !THINK_C && !WIN32 */
#ifdef __hpux
#undef _INCLUDE_HPUX_SOURCE
#endif                          /* __hpux */

/* Uncomment the following line to get instantiation printouts */
/* #define DEBUG_INSTANTIATIONS */

#ifdef NO_TOP_JUST
void remove_top_level_justifications(instantiation * inst);
#endif

/* mvp 5-17-94 */
/* --------------------------------------------------------------------------
            Build Prohibit Preference List for Backtracing
--------------------------------------------------------------------------*/

void build_prohibits_list(instantiation * inst)
{
    condition *cond;
    preference *pref, *new_pref;

    for (cond = inst->top_of_instantiated_conditions; cond != NIL; cond = cond->next) {
        cond->bt.prohibits = NIL;
        if (cond->type == POSITIVE_CONDITION && cond->bt.trace) {
            if (cond->bt.trace->slot) {
                pref = cond->bt.trace->slot->preferences[PROHIBIT_PREFERENCE_TYPE];
                while (pref) {
                    new_pref = NIL;
                    if (pref->inst->match_goal_level == inst->match_goal_level && pref->in_tm) {
                        push(pref, cond->bt.prohibits);
                        preference_add_ref(pref);
                    } else {
                        new_pref = find_clone_for_level(pref, inst->match_goal_level);
                        if (new_pref) {
                            if (new_pref->in_tm) {
                                push(new_pref, cond->bt.prohibits);
                                preference_add_ref(new_pref);
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

preference *find_clone_for_level(preference * p, goal_stack_level level)
{
    preference *clone;

    if (!p) {
        /* --- if the wme doesn't even have a preference on it, we can't backtrace
           at all (this happens with I/O and some architecture-created wmes --- */
        return NIL;
    }

    /* --- look at pref and all of its clones, find one at the right level --- */

#ifdef NO_TOP_JUST
    if (p->match_goal_level == level)
        return p;

    for (clone = p->next_clone; clone != NIL; clone = clone->next_clone)
        if (clone->match_goal_level == level)
            return clone;

    for (clone = p->prev_clone; clone != NIL; clone = clone->prev_clone)
        if (clone->match_goal_level == level)
            return clone;
#else

    if (p->inst->match_goal_level == level)
        return p;

    for (clone = p->next_clone; clone != NIL; clone = clone->next_clone)
        if (clone->inst->match_goal_level == level)
            return clone;

    for (clone = p->prev_clone; clone != NIL; clone = clone->prev_clone)
        if (clone->inst->match_goal_level == level)
            return clone;

#endif

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

void find_match_goal(instantiation * inst)
{
    Symbol *lowest_goal_so_far;
    goal_stack_level lowest_level_so_far;
    condition *cond;
    Symbol *id;

    lowest_goal_so_far = NIL;
    lowest_level_so_far = -1;
    for (cond = inst->top_of_instantiated_conditions; cond != NIL; cond = cond->next)
        if (cond->type == POSITIVE_CONDITION) {
            id = cond->bt.wme->id;
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
   (BUGBUG I'm not sure this is really needed.)

   As rhs unbound variables are encountered, they are instantiated with
   new gensyms.  These gensyms are then stored in the rhs_variable_bindings
   array, so if the same unbound variable is encountered a second time
   it will be instantiated with the same gensym.
----------------------------------------------------------------------- */

long firer_highest_rhs_unboundvar_index;

Symbol *instantiate_rhs_value(rhs_value rv, goal_stack_level new_id_level,
                              char new_id_letter, struct token_struct *tok, wme * w)
{
    list *fl;
    list *arglist;
    cons *c, *prev_c, *arg_cons;
    rhs_function *rf;
    Symbol *result;
    bool nil_arg_found;

    if (rhs_value_is_symbol(rv)) {
        result = rhs_value_to_symbol(rv);
        symbol_add_ref(result);
        return result;
    }

    if (rhs_value_is_unboundvar(rv)) {
        long index;
        Symbol *sym;

        index = rhs_value_to_unboundvar(rv);
        if (firer_highest_rhs_unboundvar_index < index)
            firer_highest_rhs_unboundvar_index = index;
        sym = *(current_agent(rhs_variable_bindings) + index);

        if (!sym) {
            sym = make_new_identifier(new_id_letter, new_id_level);
            *(current_agent(rhs_variable_bindings) + index) = sym;
            return sym;
        } else if (sym->common.symbol_type == VARIABLE_SYMBOL_TYPE) {
            new_id_letter = *(sym->var.name + 1);
            sym = make_new_identifier(new_id_letter, new_id_level);
            *(current_agent(rhs_variable_bindings) + index) = sym;
            return sym;
        } else {
            symbol_add_ref(sym);
            return sym;
        }
    }

    if (rhs_value_is_reteloc(rv)) {
        result = get_symbol_from_rete_loc((unsigned short) rhs_value_to_reteloc_levels_up(rv),
                                          (byte) rhs_value_to_reteloc_field_num(rv), tok, w);
        symbol_add_ref(result);
        return result;
    }

    fl = rhs_value_to_funcall_list(rv);
    rf = fl->first;

    /* --- build up a list of the argument values --- */
    prev_c = NIL;
    nil_arg_found = FALSE;
    arglist = NIL;              /* unnecessary, but gcc -Wall warns without it */
    for (arg_cons = fl->rest; arg_cons != NIL; arg_cons = arg_cons->rest) {
        allocate_cons(&c);
        c->first = instantiate_rhs_value(arg_cons->first, new_id_level, new_id_letter, tok, w);
        if (!c->first)
            nil_arg_found = TRUE;
        if (prev_c)
            prev_c->rest = c;
        else
            arglist = c;
        prev_c = c;
    }
    if (prev_c)
        prev_c->rest = NIL;
    else
        arglist = NIL;

    /* --- if all args were ok, call the function --- */
    if (!nil_arg_found)
        result = (*(rf->f)) (arglist);
    else
        result = NIL;

    /* --- scan through arglist, dereference symbols and deallocate conses --- */
    for (c = arglist; c != NIL; c = c->rest)
        if (c->first)
            symbol_remove_ref((Symbol *) (c->first));
    free_list(arglist);

    return result;
}

preference *execute_action(action * a, struct token_struct * tok, wme * w)
{
    Symbol *id, *attr, *value, *referent;
    char first_letter;

    if (a->type == FUNCALL_ACTION) {
        value = instantiate_rhs_value(a->value, -1, 'v', tok, w);
        if (value)
            symbol_remove_ref(value);
        return NIL;
    }

    attr = NIL;
    value = NIL;
    referent = NIL;

    id = instantiate_rhs_value(a->id, -1, 's', tok, w);
    if (!id)
        goto abort_execute_action;
    if (id->common.symbol_type != IDENTIFIER_SYMBOL_TYPE) {
        print_with_symbols("Error: RHS makes a preference for %y (not an identifier)\n", id);
        goto abort_execute_action;
    }

    attr = instantiate_rhs_value(a->attr, id->id.level, 'a', tok, w);
    if (!attr)
        goto abort_execute_action;

    first_letter = first_letter_from_symbol(attr);

    value = instantiate_rhs_value(a->value, id->id.level, first_letter, tok, w);
    if (!value)
        goto abort_execute_action;

    if (preference_is_binary(a->preference_type)) {
        referent = instantiate_rhs_value(a->referent, id->id.level, first_letter, tok, w);
        if (!referent)
            goto abort_execute_action;
    }

    /* --- RBD 4/17/95 added stuff to handle attribute_preferences_mode --- */
    if (((a->preference_type != ACCEPTABLE_PREFERENCE_TYPE) &&
         (a->preference_type != REJECT_PREFERENCE_TYPE)) &&
        (!(id->id.isa_goal && (attr == current_agent(operator_symbol))))) {

#ifndef SOAR_8_ONLY
        if ((current_agent(attribute_preferences_mode) == 2) || (current_agent(operand2_mode) == TRUE)) {
#endif
            print_with_symbols("\nError: attribute preference other than +/- for %y ^%y -- ignoring it.", id, attr);
            goto abort_execute_action;

#ifndef SOAR_8_ONLY
        } else if (current_agent(attribute_preferences_mode) == 1) {
            print_with_symbols("\nWarning: attribute preference other than +/- for %y ^%y.", id, attr);
        }
#endif

    }

    return make_preference(a->preference_type, id, attr, value, referent);

  abort_execute_action:        /* control comes here when some error occurred */
    if (id)
        symbol_remove_ref(id);
    if (attr)
        symbol_remove_ref(attr);
    if (value)
        symbol_remove_ref(value);
    if (referent)
        symbol_remove_ref(referent);
    return NIL;
}

/* -----------------------------------------------------------------------
                    Fill In New Instantiation Stuff

   This routine fills in a newly created instantiation structure with
   various information.   At input, the instantiation should have:
     - preferences_generated filled in; 
     - instantiated conditions filled in;
     - top-level positive conditions should have bt.wme, bt.level, and
       bt.trace filled in, but bt.wme and bt.trace shouldn't have their
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

void fill_in_new_instantiation_stuff(instantiation * inst, bool need_to_do_support_calculations)
{
    condition *cond;
    preference *p;
    goal_stack_level level;

#if defined(OPTIMIZE_TOP_LEVEL_RESULTS) || ( defined(THIN_JUSTIFICATIONS) && !defined(MAKE_PRODUCTION_FOR_THIN_JUSTS))
    if (inst->prod)
#endif
        production_add_ref(inst->prod);

    find_match_goal(inst);

    level = inst->match_goal_level;

#ifdef NO_TOP_JUST
    /* Record goal information as we may discard pref->inst later      */
    /* This list of preferences will catch the productions results and */
    /* clone preferences (I believe).                                  */

    for (p = inst->preferences_generated; p != NIL; p = p->inst_next) {
        p->match_goal = inst->match_goal;       /* NIL if from attribute impasse */
        p->match_goal_level = inst->match_goal_level;
    }

#endif                          /* NO_TOP_JUST */

    /* 

       Note: since we'll never backtrace through instantiations at the top
       level, it might make sense to not increment the reference counts
       on the wmes and preferences here if the instantiation is at the top
       level.  As it stands now, we could gradually accumulate garbage at
       the top level if we have a never-ending sequence of production
       firings at the top level that chain on each other's results.  (E.g.,
       incrementing a counter on every decision cycle.)  I'm leaving it this
       way for now, because if we go to S-Support, we'll (I think) need to
       save these around (maybe). 

     */

    for (cond = inst->top_of_instantiated_conditions; cond != NIL; cond = cond->next)
        if (cond->type == POSITIVE_CONDITION) {

            /* begin SW 7.7.99 */

#ifdef NO_TOP_LEVEL_REFS
            if (level > 1) {
                wme_add_ref(cond->bt.wme);
            }
#else
            wme_add_ref(cond->bt.wme);
#endif

            /* --- if trace is for a lower level, find one for this level --- */
            if (cond->bt.trace) {

#ifdef NO_TOP_JUST
                if (cond->bt.trace->match_goal_level > level)
                    cond->bt.trace = find_clone_for_level(cond->bt.trace, level);
#else

                if (cond->bt.trace->inst->match_goal_level > level)
                    cond->bt.trace = find_clone_for_level(cond->bt.trace, level);
#endif

                /* begin SW 7.7.99 */
#ifdef NO_TOP_LEVEL_REFS
                if ((cond->bt.trace) && (level > 1)) {
                    preference_add_ref(cond->bt.trace);
                }
#else
                if (cond->bt.trace)
                    preference_add_ref(cond->bt.trace);
#endif
            }

        }

    /* endif SW 7.7.99 */

    if (inst->match_goal) {
        for (p = inst->preferences_generated; p != NIL; p = p->inst_next) {
            insert_at_head_of_dll(inst->match_goal->id.preferences_from_goal, p, all_of_goal_next, all_of_goal_prev);
            p->on_goal_list = TRUE;
        }
    }
    inst->backtrace_number = 0;

    if (current_agent(o_support_calculation_type) == 0 ||
        current_agent(o_support_calculation_type) == 3 || current_agent(o_support_calculation_type) == 4) {
        /* --- do calc's the normal Soar 6 way --- */
        if (need_to_do_support_calculations)
            calculate_support_for_instantiation_preferences(inst);
    } else if (current_agent(o_support_calculation_type) == 1) {
        if (need_to_do_support_calculations)
            calculate_support_for_instantiation_preferences(inst);
        /* --- do calc's both ways, warn on differences --- */
        if ((inst->prod->declared_support != DECLARED_O_SUPPORT) &&
            (inst->prod->declared_support != DECLARED_I_SUPPORT)) {
            /* --- At this point, we've done them the normal way.  To look for
               differences, save o-support flags on a list, then do Doug's
               calculations, then compare and restore saved flags. --- */
            list *saved_flags;
            preference *pref;
            bool difference_found;
            saved_flags = NIL;
            for (pref = inst->preferences_generated; pref != NIL; pref = pref->inst_next)
                push((pref->o_supported ? pref : NIL), saved_flags);
            saved_flags = destructively_reverse_list(saved_flags);
            dougs_calculate_support_for_instantiation_preferences(inst);
            difference_found = FALSE;
            for (pref = inst->preferences_generated; pref != NIL; pref = pref->inst_next) {
                cons *c;
                bool b;
                c = saved_flags;
                saved_flags = c->rest;
                b = (bool) (c->first ? TRUE : FALSE);
                free_cons(c);
                if (pref->o_supported != b)
                    difference_found = TRUE;
                pref->o_supported = b;
            }
            if (difference_found) {
                print_with_symbols("\n*** O-support difference found in production %y", inst->prod->name);
            }
        }
    } else {
        /* --- do calc's Doug's way --- */
        if ((inst->prod->declared_support != DECLARED_O_SUPPORT) &&
            (inst->prod->declared_support != DECLARED_I_SUPPORT)) {
            dougs_calculate_support_for_instantiation_preferences(inst);
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

void init_firer(void)
{
    init_memory_pool(&current_agent(instantiation_pool), sizeof(instantiation), "instantiation");
}

/* --- Macro returning TRUE iff we're supposed to trace firings for the
   given instantiation, which should have the "prod" field filled in. --- */

#ifndef TRACE_CONTEXT_DECISIONS_ONLY

#define trace_firings_of_inst(inst) \
  ((inst)->prod && \
   (current_agent(sysparams)[TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM+(inst)->prod->type] || \
    ((inst)->prod->trace_firings)))

#endif

/* -----------------------------------------------------------------------
                         Create Instantiation

   This builds the instantiation for a new match, and adds it to
   newly_created_instantiations.  It also calls chunk_instantiation() to
   do any necessary chunk or justification building.
----------------------------------------------------------------------- */

void create_instantiation(production * prod, struct token_struct *tok, wme * w)
{
    instantiation *inst;
    condition *cond;
    preference *pref;
    action *a;
    cons *c;
    bool need_to_do_support_calculations;
    bool trace_it;
    long index;
    Symbol **cell;

#ifdef BUG_139_WORKAROUND
    /* RPM workaround for bug #139: don't fire justifications */
    if (prod->type == JUSTIFICATION_PRODUCTION_TYPE) {
        return;
    }
#endif

    allocate_with_pool(&current_agent(instantiation_pool), &inst);
    inst->next = current_agent(newly_created_instantiations);
    current_agent(newly_created_instantiations) = inst;
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

#ifndef SOAR_8_ONLY
    if (current_agent(operand2_mode) == TRUE) {
#endif
        if (current_agent(soar_verbose_flag) == TRUE)
            print_with_symbols("\n   in create_instantiation: %y", inst->prod->name);
#ifndef SOAR_8_ONLY
    }
#endif
    /* REW: end   09.15.96 */

    current_agent(production_being_fired) = inst->prod;
    prod->firing_count++;
    current_agent(production_firing_count)++;

    /* --- build the instantiated conditions, and bind LHS variables --- */
    p_node_to_conditions_and_nots(prod->p_node, tok, w,
                                  &(inst->top_of_instantiated_conditions),
                                  &(inst->bottom_of_instantiated_conditions), &(inst->nots), NIL);

    /* --- record the level of each of the wmes that was positively tested --- */
    for (cond = inst->top_of_instantiated_conditions; cond != NIL; cond = cond->next) {
        if (cond->type == POSITIVE_CONDITION) {
            cond->bt.level = cond->bt.wme->id->id.level;
            cond->bt.trace = cond->bt.wme->preference;

        }
    }

#ifndef TRACE_CONTEXT_DECISIONS_ONLY

    /* --- print trace info --- */
    trace_it = (bool) trace_firings_of_inst(inst);
    if (trace_it) {
        if (get_printer_output_column() != 1)
            print("\n");        /* AGR 617/634 */
        print("Firing ");
        print_instantiation_with_wmes
            (inst, (wme_trace_type) current_agent(sysparams)[TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM]);
    }
#endif

    /* --- initialize rhs_variable_bindings array with names of variables
       (if there are any stored on the production -- for chunks there won't
       be any) --- */
    index = 0;
    cell = current_agent(rhs_variable_bindings);
    for (c = prod->rhs_unbound_variables; c != NIL; c = c->rest) {
        *(cell++) = c->first;
        index++;
    }
    firer_highest_rhs_unboundvar_index = index - 1;

    /* 7.1/8 merge: Not sure about this.  This code in 704, but not in either 7.1 or 703/soar8 */
    /* --- Before executing the RHS actions, tell the user that the -- */
    /* --- phase has changed to output by printing the arrow --- */

#ifndef TRACE_CONTEXT_DECISIONS_ONLY

    if (trace_it && current_agent(sysparams)[TRACE_FIRINGS_PREFERENCES_SYSPARAM]) {
        print(" -->\n");
    }
#endif

    /* --- execute the RHS actions, collect the results --- */
    inst->preferences_generated = NIL;
    need_to_do_support_calculations = FALSE;
    for (a = prod->action_list; a != NIL; a = a->next) {
        pref = execute_action(a, tok, w);
        if (pref) {
            pref->inst = inst;
            insert_at_head_of_dll(inst->preferences_generated, pref, inst_next, inst_prev);
            if (inst->prod->declared_support == DECLARED_O_SUPPORT)
                pref->o_supported = TRUE;
            else if (inst->prod->declared_support == DECLARED_I_SUPPORT)
                pref->o_supported = FALSE;

            else {

#ifndef SOAR_8_ONLY
                if (current_agent(operand2_mode) == TRUE) {
#endif
                    pref->o_supported = (bool) ((current_agent(FIRING_TYPE) == PE_PRODS) ? TRUE : FALSE);

#ifndef SOAR_8_ONLY
                }

                /* REW: end   09.15.96 */

                else {
                    if (a->support == O_SUPPORT)
                        pref->o_supported = TRUE;
                    else if (a->support == I_SUPPORT)
                        pref->o_supported = FALSE;
                    else {
                        need_to_do_support_calculations = TRUE;
                        if (current_agent(soar_verbose_flag) == TRUE)
                            print("\n\nin create_instantiation():  need_to_do_support_calculations == TRUE!!!\n\n");
                    }

                }
#endif

            }
        }
    }

    /* --- reset rhs_variable_bindings array to all zeros --- */
    index = 0;
    cell = current_agent(rhs_variable_bindings);
    while (index++ <= firer_highest_rhs_unboundvar_index)
        *(cell++) = NIL;

    /* --- fill in lots of other stuff --- */
    fill_in_new_instantiation_stuff(inst, need_to_do_support_calculations);

#ifndef TRACE_CONTEXT_DECISIONS_ONLY

    /* --- print trace info: printing preferences --- */
    /* Note: can't move this up, since fill_in_new_instantiation_stuff gives
       the o-support info for the preferences we're about to print */
    if (trace_it && current_agent(sysparams)[TRACE_FIRINGS_PREFERENCES_SYSPARAM]) {
        for (pref = inst->preferences_generated; pref != NIL; pref = pref->inst_next) {
            print(" ");
            print_preference(pref);
        }
    }
#endif

    /* mvp 5-17-94 */
    build_prohibits_list(inst);

    current_agent(production_being_fired) = NIL;

    /* --- build chunks/justifications if necessary --- */
    chunk_instantiation(inst, (bool) current_agent(sysparams)[LEARNING_ON_SYSPARAM]);

    /* MVP 6-8-94 */
    if (!current_agent(system_halted)) {

#ifndef FEW_CALLBACKS
        /* --- invoke callback function --- */
        soar_invoke_callbacks(soar_agent, FIRING_CALLBACK, (soar_call_data) inst);
#endif
    }
}

/* -----------------------------------------------------------------------
                        Deallocate Instantiation

   This deallocates the given instantiation.  This should only be invoked
   via the possibly_deallocate_instantiation() macro.
----------------------------------------------------------------------- */

void deallocate_instantiation(instantiation * inst)
{
    condition *cond;

    /* mvp 5-17-94 */
    list *c, *c_old;
    preference *pref;
    goal_stack_level level;

    level = inst->match_goal_level;

#ifdef DEBUG_INSTANTIATIONS
    if (inst->prod)
        print_with_symbols("\nDeallocate instantiation of %y", inst->prod->name);
#endif

#ifdef WATCH_SSCI_INSTS
    if (inst->isa_ssci_inst == TRUE) {
        if (inst->prod) {
            print_with_symbols("\nDeallocating an SSCI instantiation: %y", inst->prod->name);
            print("Production has %d references\n", inst->prod->reference_count);
        } else
            print_with_symbols("\nDeallocating an SSCI instantiation whose production has already been excised\n");

    }
#endif

    for (cond = inst->top_of_instantiated_conditions; cond != NIL; cond = cond->next)
        if (cond->type == POSITIVE_CONDITION) {

            /* mvp 6-22-94, modified 94.01.17 by AGR with lotsa help from GAP */
            if (cond->bt.prohibits) {
                c_old = c = cond->bt.prohibits;
                cond->bt.prohibits = NIL;
                for (; c != NIL; c = c->rest) {
                    pref = (preference *) c->first;

#ifdef NO_TOP_LEVEL_REFS
                    /* Note, we can probably speed this up with a variable */
                    if (level > 1)
#endif
                        preference_remove_ref(pref);
                }
                free_list(c_old);
            }
            /* mvp done */

#ifdef NO_TOP_LEVEL_REFS
            if (level > 1) {
                wme_remove_ref(cond->bt.wme);
                if (cond->bt.trace)
                    preference_remove_ref(cond->bt.trace);
            }
#else
            wme_remove_ref(cond->bt.wme);
            if (cond->bt.trace)
                preference_remove_ref(cond->bt.trace);

#endif

        }

    deallocate_condition_list(inst->top_of_instantiated_conditions);
    deallocate_list_of_nots(inst->nots);

    if (inst->prod)
        production_remove_ref(inst->prod);

    free_with_pool(&current_agent(instantiation_pool), inst);
}

/* -----------------------------------------------------------------------
                         Retract Instantiation

   This retracts the given instantiation.
----------------------------------------------------------------------- */

void retract_instantiation(instantiation * inst)
{
    preference *pref, *next;
    bool retracted_a_preference;

#ifndef TRACE_CONTEXT_DECISIONS_ONLY
    bool trace_it;
#endif

#ifndef FEW_CALLBACKS
    /* --- invoke callback function --- */
    soar_invoke_callbacks(soar_agent, RETRACTION_CALLBACK, (soar_call_data) inst);
#endif

    retracted_a_preference = FALSE;

#ifndef TRACE_CONTEXT_DECISIONS_ONLY

    trace_it = (bool) trace_firings_of_inst(inst);
#endif

    /* --- retract any preferences that are in TM and aren't o-supported --- */
    pref = inst->preferences_generated;
    while (pref != NIL) {
        next = pref->inst_next;
        if (pref->in_tm && (!pref->o_supported)) {

#ifndef TRACE_CONTEXT_DECISIONS_ONLY

            if (trace_it) {
                if (!retracted_a_preference) {
                    if (get_printer_output_column() != 1)
                        print("\n");    /* AGR 617/634 */
                    print("Retracting ");
                    print_instantiation_with_wmes
                        (inst, (wme_trace_type) current_agent(sysparams)[TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM]);
                    if (current_agent(sysparams)[TRACE_FIRINGS_PREFERENCES_SYSPARAM])
                        print(" -->");
                }
                if (current_agent(sysparams)[TRACE_FIRINGS_PREFERENCES_SYSPARAM]) {
                    print(" ");
                    print_preference(pref);
                }
            }
#endif

            remove_preference_from_tm(pref);
            retracted_a_preference = TRUE;
        }
        pref = next;
    }

    /* --- remove inst from list of instantiations of this production --- */
#if defined(OPTIMIZE_TOP_LEVEL_RESULTS) || (defined(THIN_JUSTIFICATIONS) && !defined(MAKE_PRODUCTION_FOR_THIN_JUSTS))
    if (inst->prod)
#endif
        remove_from_dll(inst->prod->instantiations, inst, next, prev);

    /* --- if retracting a justification, excise it --- */
    /*
     * if the reference_count on the production is 1 (or less) then the
     * only thing supporting this justification is the instantiation, hence
     * it has already been excised, and doing it again is wrong.
     */

#ifdef WATCH_SSCI_INSTS
    if (inst->isa_ssci_inst) {
        print("Retracting SSCI instantiation...");
        if (inst->prod)
            print(" prod ref cound = %d\n", inst->prod->reference_count);
        else
            print("\n");
    }
#endif

#if defined(OPTIMIZE_TOP_LEVEL_RESULTS) || (defined(THIN_JUSTIFICATIONS) && !defined(MAKE_PRODUCTION_FOR_THIN_JUSTS))
    if (inst->prod && inst->prod->type == JUSTIFICATION_PRODUCTION_TYPE && inst->prod->reference_count > 1) {
#else

    if (inst->prod->type == JUSTIFICATION_PRODUCTION_TYPE && inst->prod->reference_count > 1) {
#endif

        excise_production(inst->prod, FALSE);
        /*inst->prod = NIL; */
    }

    /* --- mark as no longer in MS, and possibly deallocate  --- */
    inst->in_ms = FALSE;
    possibly_deallocate_instantiation(inst);
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

	 Note also that this ordering is different if the compile-time
	 flag 'O_REJECTS_FIRST' is defined.  In this situation, o-rejects are 
	 processed before other prefrences.
----------------------------------------------------------------------- */

void assert_new_preferences(void)
{
    instantiation *inst, *next_inst;
    preference *pref, *next_pref;
    preference *o_rejects;
#if defined(WATCH_INSTS_WITH_O_PREFS) || defined(REMOVE_INSTS_WITH_O_PREFS) || defined(OPTIMIZE_TOP_LEVEL_RESULTS) || defined(THIN_JUSTIFICATIONS)
    bool is_fully_o_supported;

#endif

    o_rejects = NIL;

    /* REW: begin 09.15.96 */
#ifndef SOAR_8_ONLY
    if ((current_agent(operand2_mode) == TRUE) &&
#else
    if (
#endif
           (current_agent(soar_verbose_flag) == TRUE))
        print("\n   in assert_new_preferences:");
    /* REW: end   09.15.96 */

#ifdef O_REJECTS_FIRST
    {

        slot *s;
        preference *p, *next_p;

        /* Do an initial loop to process o-rejects, then re-loop
           to process normal preferences.  No buffering should be needed.
         */
        for (inst = current_agent(newly_created_instantiations); inst != NIL; inst = next_inst) {
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
                                remove_preference_from_tm(p);
                            p = next_p;
                        }
                    }
                }
            }
        }
    }
#endif

    for (inst = current_agent(newly_created_instantiations); inst != NIL; inst = next_inst) {
        next_inst = inst->next;

#if defined(OPTIMIZE_TOP_LEVEL_RESULTS) || (defined(THIN_JUSTIFICATIONS) && !defined(MAKE_PRODUCTION_FOR_THIN_JUSTS))
        if (inst->in_ms && inst->prod)
#else
        if (inst->in_ms)
#endif
            insert_at_head_of_dll(inst->prod->instantiations, inst, next, prev);

        /* REW: begin 09.15.96 */
#ifndef SOAR_8_ONLY
        if (current_agent(operand2_mode) == TRUE) {
#endif
            if (current_agent(soar_verbose_flag) == TRUE) {
                if (inst->prod)
                    print_with_symbols("\n      asserting instantiation: %y\n", inst->prod->name);
                else
                    print("\n     asserting a Thin Instantiaion.\n");
            }
#ifndef SOAR_8_ONLY
        }
#endif
        /* REW: end   09.15.96 */

        for (pref = inst->preferences_generated; pref != NIL; pref = next_pref) {
            next_pref = pref->inst_next;

            if ((pref->type == REJECT_PREFERENCE_TYPE) && (pref->o_supported)) {
                /* --- o-reject: just put it in the buffer for later --- */
#ifndef O_REJECTS_FIRST
                pref->next = o_rejects;
                o_rejects = pref;
#endif

                /* REW: begin 09.15.96 */
                /* No knowledge retrieval necessary in Operand2 */
                /* REW: end   09.15.96 */

            } else if (inst->in_ms || pref->o_supported) {
                /* --- normal case --- */
                add_preference_to_tm(pref);

                /* REW: begin 09.15.96 */
                /* No knowledge retrieval necessary in Operand2 */
                /* REW: end   09.15.96 */

            } else {
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
                preference_add_ref(pref);
                preference_remove_ref(pref);
            }
        }

#if defined(WATCH_INSTS_WITH_O_PREFS) || defined(REMOVE_INSTS_WITH_O_PREFS) || defined(THIN_JUSTIFICATIONS) || defined(WATCH_SSCI_INSTS) || defined(OPTIMIZE_TOP_LEVEL_RESULTS)
        is_fully_o_supported = TRUE;

        /*
           print( "Checking o-support on rhs of inst\n" );
           print_instantiation_with_wmes( inst, FULL_WME_TRACE );
         */

        for (pref = inst->preferences_generated; pref != NIL; pref = pref->inst_next) {
            if (pref->o_supported == FALSE) {

                /*
                   if ( inst->isa_ssci_inst ) {
                   print( "This preference is not osupported\n" );
                   watchful_print_preference( pref );
                   }
                 */

                is_fully_o_supported = FALSE;
                break;
            }
        }

#ifdef WATCH_SSCI_INSTS
        if (inst->isa_ssci_inst == TRUE) {
            if (inst->prod) {
                print_with_symbols("\nThe SSCI instantiation of %y\n", inst->prod->name);
                print_instantiation_with_wmes(inst, FULL_WME_TRACE);

            } else {
                print("\nAn SSCI inst with a nil production\n");
            }
            if (is_fully_o_supported) {
                print("at %p has FULLY o-supported results\n", inst);
            } else {
                print("at %p has i-supported results\n", inst);
                for (pref = inst->preferences_generated; pref != NIL; pref = pref->inst_next) {
                    watchful_print_preference(pref);
                }
            }
        }
#endif

        /* 
         * If an instantiation produces all o-supported preferences,
         * and we're not going to do backtracing (via learning or otherwise)
         * then we don't need to keep the instantiation around.
         * At this point, I think all of the preferences should have been
         * asserted, so hopefully we can just remove it...
         * There are a number of possbile build options defining what gets
         * removed these all interact at this point in the code, making 
         * everything extrememly tricky: 
         * REMOVE_INSTS_WITH_O_PREFS removes everything
         * 
         *
         */
#ifdef REMOVE_INSTS_WITH_O_PREFS
        if (is_fully_o_supported)
            retract_instantiation(inst);

#else                           /* !REMOVE_INSTS_WITH_O_PREFS */
#if defined(OPTIMIZE_TOP_LEVEL_RESULTS)

        /* 
         *Remove everything with !inst->prod.  
         * This includes everyting made during OPTIMIZE_TOP_LEVEL_RESULT
         * AND everything made in THIN_JUSTIFICATIONS as long as
         * MAKE_PROD_FOR_THIN_JUSTS is not defined.
         */
        if (is_fully_o_supported && !inst->prod)
            retract_instantiation(inst);

#if defined(THIN_JUSTIFICATIONS) && defined(MAKE_PRODUCTION_FOR_THIN_JUSTS)
        /* 
         * The only way to determine what has been made in THIN_JUSTIFICATIONS
         * is by checking isa_ssci_inst. Because isa_ssci_inst is not
         * set for those made during OPTIMIZE_TOP_LEVEL_RESULTS, we
         * dont have to worry about removing these guys twice.
         */
        if (is_fully_o_supported && inst->isa_ssci_inst == TRUE)
            retract_instantiation(inst);
#endif

#else                           /* !OPTIMIZE_TOP_LEVEL_RESULTS */
        /* 
         * If we dont OPTIMIZE_TOP_LEVEL_RESULTS, then we only need to deal 
         * with things made as a result of THIN_JUSTIFICATIONS.  Of course, 
         * this still means 2 situations.
         */
#if defined(THIN_JUSTIFICATIONS) && defined(MAKE_PRODUCTION_FOR_THIN_JUSTS)
        if (is_fully_o_supported && inst->isa_ssci_inst == TRUE)
            retract_instantiation(inst);

#elif defined(THIN_JUSTIFICATIONS) && !defined(MAKE_PRODUCTION_FOR_THIN_JUSTS)
        if (is_fully_o_supported && !inst->prod) {
            retract_instantiation(inst);
        }
#endif

#endif                          /* OPTIMIZE_TOP_LEVEL_RESULTS */

#endif                          /* REMOVE_INSTS_WITH_O_PREFS */

#endif

#ifdef NO_TOP_JUST

#ifdef REMOVE_INSTS_WITH_O_PREFS
        if (inst && inst->prod)
#else
        if (inst->prod)
#endif                          /* REMOVE_INSTS_WITH_O_PREFS */
            if (inst->prod->type == JUSTIFICATION_PRODUCTION_TYPE)
                remove_top_level_justifications(inst);

#endif                          /* NO_TOP_JUST */

    }
#ifndef O_REJECTS_FIRST
    if (o_rejects)
        process_o_rejects_and_deallocate_them(o_rejects);
#endif
}

/* -----------------------------------------------------------------------
                          Do Preference Phase

   This routine is called from the top level to run the preference phase.
----------------------------------------------------------------------- */

void do_preference_phase(void)
{
    production *prod;
    struct token_struct *tok;
    wme *w;
    instantiation *inst;

#ifndef TRACE_CONTEXT_DECISIONS_ONLY

/* AGR 617/634:  These are 2 bug reports that report the same problem,
   namely that when 2 chunk firings happen in succession, there is an
   extra newline printed out.  The simple fix is to monitor
   get_printer_output_column and see if it's at the beginning of a line
   or not when we're ready to print a newline.  94.11.14 */

    if (current_agent(sysparams)[TRACE_PHASES_SYSPARAM]) {
#ifndef SOAR_8_ONLY
        if (current_agent(operand2_mode) == TRUE) {
#endif
            switch (current_agent(FIRING_TYPE)) {
            case PE_PRODS:
                print("\t--- Firing Productions (PE) ---\n");
                break;
            case IE_PRODS:
                print("\t--- Firing Productions (IE) ---\n");
                break;
            }
#ifndef SOAR_8_ONLY
        }

        else
            print("\n--- Preference Phase ---\n");
#endif
    }

#endif

    current_agent(newly_created_instantiations) = NIL;

    /* MVP 6-8-94 */
    while (get_next_assertion(&prod, &tok, &w)) {
        if (current_agent(max_chunks_reached)) {
            current_agent(system_halted) = TRUE;
            return;
        }

        create_instantiation(prod, tok, w);
    }

    assert_new_preferences();

    while (get_next_retraction(&inst))
        retract_instantiation(inst);

/* REW: begin 08.20.97 */

    /*  In Waterfall, if there are nil goal retractions, then we want to 
       retract them as well, even though they are not associated with any
       particular goal (because their goal has been deleted). The 
       functionality of this separte routine could have been easily 
       combined in get_next_retraction but I wanted to highlight the 
       distinction between regualr retractions (those that can be 
       mapped onto a goal) and nil goal retractions that require a
       special data strucutre (because they don't appear on any goal) 
       REW.  */

#ifndef SOAR_8_ONLY
    if (current_agent(operand2_mode) && current_agent(nil_goal_retractions)) {
#else
    if (current_agent(nil_goal_retractions)) {
#endif
        while (get_next_nil_goal_retraction(&inst))
            retract_instantiation(inst);
    }

/* REW: end   08.20.97 */

}

#ifdef NO_TOP_JUST

/* --------------------------------------------------------------------

                        Remove Top Level Justifications

   Go through the newly created instantiations and delete any that are
   top level justifications for o-supported preferences.

   The tricky part about this is the timing:
   1. We need to wait till after assert_new_preferences or the preferences
      from the justifications won't show up at all.
      Unfortunately, assert_new_preferences clears the list
      "newly_created_instantiations" as it works.
   2. We want to excise, retract and deallocate the instantiation.
      Excise production causes the rete to mark the instantiation for retraction.
      When it retracts, if its "preferences generated" list is empty and
      if the ref count on the instantiation is just 1, it'll be deallocated.

   This means, we have to slide this removal process into the assert_new_preferences
   loop.  That way the preferences get added, we excise the justification,
   mark all the preferences it created as having no instantiations supporting
   them and set the inst->preferences_generated list to nil.  The rete
   adds the justification to the list of retractions which are processed
   right after assert_new_preferences.  If everything goes to plan, the
   justification is pulled out and de-allocated and bingo we're done. DJP 5/8/96.

-------------------------------------------------------------------- */

void remove_top_level_justifications(instantiation * inst)
{
    preference *pref;
    bool all_o_supported, remove_just;
#ifdef NO_JUSTS_BELOW_OSUPPORT
    bool masked;
    slot *s;
    preference *p2;
#endif

    /* Top level justifications of o-supported preferences can be safely  */
    /* removed as they serve no purpose.                                  */

    /* It *may* also be safe to remove justifications that are lower than      */
    /* an o-supported preference in the goal stack.  This *may* also cause     */
    /* problems for the chunker in some (hopefully rare) cases of backtracing. */
    /* If it does, we should turn off the "NO_JUSTS_BELOW_OSUPPORT flag.       */

    /* First thing to check is whether the justification was already excised  */
    /* because it failed to match.  I think we can do this by checking the    */
    /* production pointer in the name's symbol.  Anyway, if it's been excised */
    /* we don't need to try to excise it again :)                             */

    if (!inst->prod->name->sc.production)
        return;

    /* This routine is only called for justifications */
    /* First, we'll see if the justification is top level and only produced */
    /* o-supported prefs : */

    remove_just = FALSE;
    if (inst->match_goal_level <= TOP_GOAL_LEVEL) {
        all_o_supported = TRUE;
        for (pref = inst->preferences_generated; (pref != NIL && all_o_supported); pref = pref->inst_next)
            if (!pref->o_supported)
                all_o_supported = FALSE;
        if (all_o_supported)
            remove_just = TRUE;
    }

    /* If we have a justification and for each of its preferences, there          */
    /* is an identical o-supported preference at a higher level in the goal stack */
    /* then we can remove this justification.                                     */

#ifdef NO_JUSTS_BELOW_OSUPPORT
    if (inst->match_goal_level > TOP_GOAL_LEVEL) {
        remove_just = TRUE;
        for (pref = inst->preferences_generated; (pref != NIL && remove_just); pref = pref->inst_next) {
            s = pref->slot;
            masked = FALSE;
            if (s) {
                /* Is there an o-supported pref for this same value higher in the stack ? */
                for (p2 = s->preferences[pref->type]; (p2 != NIL && !masked); p2 = p2->next)
                    if (p2->match_goal_level < pref->match_goal_level && p2->o_supported == TRUE && p2->value == pref->value)   /* Can we compare these pointers ? */
                        masked = TRUE;
                if (!masked)
                    remove_just = FALSE;
            }
        }
    }
#endif

    if (remove_just) {

#ifndef TRACE_CONTEXT_DECISIONS_ONLY
        if (current_agent(sysparams)[TRACE_JUSTIFICATIONS_SYSPARAM])
            print_with_symbols("\nRemoving %y", inst->prod->name);
#endif

        excise_production(inst->prod, FALSE);

        /* Excising the production causes the rete to line it up for retracting */
        /* This will happen after assert_new_preferences has finished its work. */

        /* Now remove all trace of this justification by reseting the pointers */
        /* for each preference.  This also means a lot of little changes all   */
        /* over Soar to cover the situation where pref->inst is nil.           */

        for (pref = inst->preferences_generated; pref != NIL; pref = pref->inst_next)
            pref->inst = NIL;

        /* Finally, reset the preferences generated list.               */
        /* This way, when the justification retracts, Soar will reclaim */
        /* the memory.                                                  */
        inst->preferences_generated = NIL;
        inst->in_ms = FALSE;    /* This should always be correct, right? */
    }
}

#endif
