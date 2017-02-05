/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/*************************************************************************
 *
 *  file:  instantiation.cpp
 *
 * =======================================================================
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

#include "instantiation.h"

#include "agent.h"
#include "callback.h"
#include "condition.h"
#include "debug.h"
#include "debug_inventories.h"
#include "decide.h"
#include "dprint.h"
#include "ebc.h"
#include "instantiation.h"
#include "mem.h"
#include "misc.h"
#include "output_manager.h"
#include "preference.h"
#include "output_manager.h"
#include "print.h"
#include "production.h"
#include "reinforcement_learning.h"
#include "rete.h"
#include "rhs_functions.h"
#include "rhs.h"
#include "run_soar.h"
#include "semantic_memory.h"
#include "slot.h"
#include "soar_module.h"
#include "soar_TraceNames.h"
#include "symbol.h"
#include "test.h"
#include "working_memory_activation.h"
#include "working_memory.h"
#include "xml.h"

#include <assert.h>
#include <list>
#include <stdlib.h>
#include <string> // SBW 8/4/08

using namespace soar_TraceNames;

void init_instantiation_pool(agent* thisAgent)
{
    thisAgent->memoryManager->init_memory_pool(MP_instantiation, sizeof(instantiation), "instantiation");
}

/*--------------------------------------------------------------------
 Find Clone For Level

 This routines take a given preference and finds the clone of it whose
 match goal is at the given goal_stack_level.  (This is used to find the
 proper preference to backtrace through.)  If the given preference
 itself is at the right level, it is returned.  If there is no clone at
 the right level, NIL is returned.
 ----------------------------------------------------------------------- */

preference* find_clone_for_level(preference* p, goal_stack_level level)
{
    preference* clone;

    if (!p)
    {
        /* if the wme doesn't even have a preference on it, we can't backtrace
         at all (this happens with I/O and some architecture-created wmes */
        return NIL;
    }

    /* look at pref and all of its clones, find one at the right level */

    if (p->inst->match_goal_level == level)
    {
        return p;
    }

    for (clone = p->next_clone; clone != NIL; clone = clone->next_clone)
        if (clone->inst->match_goal_level == level)
        {
            return clone;
        }

    for (clone = p->prev_clone; clone != NIL; clone = clone->prev_clone)
        if (clone->inst->match_goal_level == level)
        {
            return clone;
        }

    /* if none was at the right level, we can't backtrace at all */
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

void find_match_goal(instantiation* inst)
{
    Symbol* lowest_goal_so_far;
    goal_stack_level lowest_level_so_far;
    condition* cond;
    Symbol* id;

    lowest_goal_so_far = NIL;
    lowest_level_so_far = -1;
    for (cond = inst->top_of_instantiated_conditions; cond != NIL;
            cond = cond->next)
        if (cond->type == POSITIVE_CONDITION)
        {
            id = cond->bt.wme_->id;
            if (id->id->isa_goal)
                if (cond->bt.level > lowest_level_so_far)
                {
                    lowest_goal_so_far = id;
                    lowest_level_so_far = cond->bt.level;
                }
        }

    inst->match_goal = lowest_goal_so_far;
    if (lowest_goal_so_far)
    {
        inst->match_goal_level = lowest_level_so_far;
    }
    else
    {
        inst->match_goal_level = ATTRIBUTE_IMPASSE_LEVEL;
    }
}

void set_bt_and_find_match_goal(instantiation* inst)
{
    Symbol* lowest_goal_so_far;
    goal_stack_level lowest_level_so_far;
    condition* cond;
    Symbol* id;

    lowest_goal_so_far = NIL;
    lowest_level_so_far = -1;
    for (cond = inst->top_of_instantiated_conditions; cond != NIL; cond = cond->next)
        if (cond->type == POSITIVE_CONDITION)
        {
            cond->bt.level = cond->bt.wme_->id->id->level;
            cond->bt.trace = cond->bt.wme_->preference;  // These are later changed to the correct clone for the level
            id = cond->bt.wme_->id;
            if (id->id->isa_goal)
                if (cond->bt.level > lowest_level_so_far)
                {
                    lowest_goal_so_far = id;
                    lowest_level_so_far = cond->bt.level;
                }
        }

    inst->match_goal = lowest_goal_so_far;
    if (lowest_goal_so_far)
    {
        inst->match_goal_level = lowest_level_so_far;
    }
    else
    {
        inst->match_goal_level = ATTRIBUTE_IMPASSE_LEVEL;
    }
}

goal_stack_level get_match_goal(condition* top_cond)
{
    goal_stack_level lowest_level_so_far;
    condition* cond;
    Symbol* id;

    lowest_level_so_far = -1;
    for (cond = top_cond; cond != NIL; cond = cond->next)
    {
        if (cond->type == POSITIVE_CONDITION)
        {
            id = cond->bt.wme_->id;
            if (id->id->isa_goal)
                if (cond->bt.level > lowest_level_so_far)
                {
                    lowest_level_so_far = cond->bt.level;
                }
        }
    }
    if (lowest_level_so_far != -1)
    {
        return lowest_level_so_far;
    }
    else
    {
        return ATTRIBUTE_IMPASSE_LEVEL;
    }
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

uint64_t get_rhs_function_first_arg_identity(agent* thisAgent, rhs_value rv)
{
    cons* fl;
    rhs_function* rf;
    rhs_value firstArg;

    fl = rhs_value_to_funcall_list(rv);
    rf = static_cast<rhs_function_struct*>(fl->first);
    firstArg =  static_cast<char*>(fl->rest->first);
    assert(rhs_value_is_symbol(firstArg));
    uint64_t returnVal = rhs_value_to_o_id(static_cast<char*>(firstArg));
    thisAgent->explanationBasedChunker->deep_copy_sym_expanded = rhs_value_to_symbol(static_cast<char*>(firstArg));
    return returnVal;
}

Symbol* instantiate_rhs_value(agent* thisAgent, rhs_value rv,
                              goal_stack_level new_id_level, char new_id_letter,
                              struct token_struct* tok, wme* w, bool& wasUnboundVar)
{
    cons* fl;
    cons* arglist;
    cons* c, *prev_c, *arg_cons;
    rhs_function* rf;
    Symbol* result;
    bool nil_arg_found;

    /* We pass back whether this was an unbound var via wasUnboundVar so that the re-orderer
     * will not consider it unconnected to the LHS if it's a child of an unconnected symbol */
    wasUnboundVar = false;
    if (rhs_value_is_symbol(rv))
    {

        result = rhs_value_to_symbol(rv);

        assert(!result->is_sti() || (result->id->level != NO_WME_LEVEL));
        thisAgent->symbolManager->symbol_add_ref(result);
        return result;
    }

    if (rhs_value_is_unboundvar(rv))
    {
        int64_t index;
        Symbol* sym;

        wasUnboundVar = true;
        index = static_cast<int64_t>(rhs_value_to_unboundvar(rv));
        if (thisAgent->firer_highest_rhs_unboundvar_index < index)
        {
            thisAgent->firer_highest_rhs_unboundvar_index = index;
        }
        sym = *(thisAgent->rhs_variable_bindings + index);

        if (!sym)
        {
            sym = thisAgent->symbolManager->make_new_identifier(new_id_letter, new_id_level);
            *(thisAgent->rhs_variable_bindings + index) = sym;
            return sym;
        }
        else if (sym->is_variable())
        {
            new_id_letter = *(sym->var->name + 1);
            sym = thisAgent->symbolManager->make_new_identifier(new_id_letter, new_id_level);
            *(thisAgent->rhs_variable_bindings + index) = sym;
            return sym;
        }
        else
        {
            thisAgent->symbolManager->symbol_add_ref(sym);
            return sym;
        }
    }

    if (rhs_value_is_reteloc(rv))
    {
        result = get_symbol_from_rete_loc(rhs_value_to_reteloc_levels_up(rv),
                                          rhs_value_to_reteloc_field_num(rv), tok, w);
        thisAgent->symbolManager->symbol_add_ref(result);
        return result;
    }

    fl = rhs_value_to_funcall_list(rv);
    rf = static_cast<rhs_function_struct*>(fl->first);

    /* build up a list of the argument values */
    prev_c = NIL;
    nil_arg_found = false;
    arglist = NIL; /* unnecessary, but gcc -Wall warns without it */
    for (arg_cons = fl->rest; arg_cons != NIL; arg_cons = arg_cons->rest)
    {
        allocate_cons(thisAgent, &c);
        bool dummy_was_unbound_var;

        c->first = instantiate_rhs_value(thisAgent, static_cast<char*>(arg_cons->first), new_id_level, new_id_letter, tok, w, dummy_was_unbound_var);
        if (!c->first)
        {
            nil_arg_found = true;
        }
        if (prev_c)
        {
            prev_c->rest = c;
        }
        else
        {
            arglist = c;
        }
        prev_c = c;
    }
    if (prev_c)
    {
        prev_c->rest = NIL;
    }
    else
    {
        arglist = NIL;
    }

    /* if all args were ok, call the function */

    if (!nil_arg_found)
    {
        // stop the kernel timer while doing RHS funcalls  KJC 11/04
        // the total_cpu timer needs to be updated in case RHS fun is statsCmd
#ifndef NO_TIMING_STUFF
        thisAgent->timers_kernel.stop();
        thisAgent->timers_cpu.stop();
        thisAgent->timers_total_kernel_time.update(thisAgent->timers_kernel);
        thisAgent->timers_total_cpu_time.update(thisAgent->timers_cpu);
        thisAgent->timers_cpu.start();
#endif

        result = (*(rf->f))(thisAgent, arglist, rf->user_data);

//        dprint(DT_DEBUG, "Called RHS function %y with args", rf->name);
//        Symbol* lSym2;
//        for (c = arglist; c != NIL; c = c->rest)
//            if (c->first)
//            {
//                lSym2 = static_cast<Symbol *>(c->first);
//                dprint_noprefix(DT_DEBUG, " %y", lSym2);
//            }
//        dprint_noprefix(DT_DEBUG, "\n");

        #ifndef NO_TIMING_STUFF  // restart the kernel timer
        thisAgent->timers_kernel.start();
#endif

    }
    else
    {
        result = NIL;
    }

    /* scan through arglist, dereference symbols and deallocate conses */
    Symbol* lSym;
    for (c = arglist; c != NIL; c = c->rest)
        if (c->first)
        {
            lSym = static_cast<Symbol *>(c->first);
            thisAgent->symbolManager->symbol_remove_ref(&lSym);
        }
    free_list(thisAgent, arglist);

    return result;
}

preference* execute_action(agent* thisAgent, action* a, struct token_struct* tok, wme* w, action* rule_action)
{
    Symbol* lId, *lAttr, *lValue, *lReferent;
    char first_letter;
    preference* newPref;

    bool_quadruple was_unbound_vars;

    if (a->type == FUNCALL_ACTION)
    {
        lValue = instantiate_rhs_value(thisAgent, a->value, -1, 'v', tok, w, was_unbound_vars.id);
        if (lValue)
        {
            thisAgent->symbolManager->symbol_remove_ref(&lValue);
        }
        return NIL;
    }

    lAttr = NIL;
    lValue = NIL;
    lReferent = NIL;
    uint64_t oid_id = 0, oid_attr = 0, oid_value = 0, oid_referent = 0;
    rhs_value f_id = 0, f_attr = 0, f_value = 0, f_referent = 0;

    lId = instantiate_rhs_value(thisAgent, a->id, -1, 's', tok, w, was_unbound_vars.id);
    if (!lId)
    {
        goto abort_execute_action;
    }
    if (!lId->is_sti())
    {
        thisAgent->outputManager->printa_sf(thisAgent, "Error: RHS of action %a makes a preference for %y (not an identifier)\n", a, lId);
        goto abort_execute_action;
    }

    lAttr = instantiate_rhs_value(thisAgent, a->attr, lId->id->level, 'a', tok, w, was_unbound_vars.attr);
    if (!lAttr)
    {
        goto abort_execute_action;
    }

    first_letter = first_letter_from_symbol(lAttr);

    lValue = instantiate_rhs_value(thisAgent, a->value, lId->id->level, first_letter, tok, w, was_unbound_vars.value);
    if (!lValue)
    {
        goto abort_execute_action;
    }

    if (preference_is_binary(a->preference_type))
    {
        lReferent = instantiate_rhs_value(thisAgent, a->referent, lId->id->level, first_letter, tok, w, was_unbound_vars.referent);
        if (!lReferent)
        {
            goto abort_execute_action;
        }
    }

    if (((a->preference_type != ACCEPTABLE_PREFERENCE_TYPE)
            && (a->preference_type != REJECT_PREFERENCE_TYPE))
            && (!(lId->id->isa_goal && (lAttr == thisAgent->symbolManager->soarSymbols.operator_symbol))))
    {
        thisAgent->outputManager->printa_sf(thisAgent, "\nError: attribute preference other than +/- for %y ^%y -- ignoring it.", lId, lAttr);
        goto abort_execute_action;
    }
    /* Populate identity and rhs_function stuff */
    if (rule_action)
    {
        if (rule_action->id)
        {
            if (rhs_value_is_funcall(rule_action->id))
            {
                f_id = rule_action->id;
                /* rule_action will get deallocated in create_instantiation, but we want it
                 * in the preference for learning, so we just steal this copy and set
                 * rule_action's to null */
                rule_action->id = NULL;
            } else {
                oid_id = rhs_value_to_o_id(rule_action->id);
            }
        }
        if (rule_action->attr)
        {
            if (rhs_value_is_funcall(rule_action->attr))
            {
                f_attr = rule_action->attr;
                rule_action->attr = NULL;
            } else {
                oid_attr = rhs_value_to_o_id(rule_action->attr);
            }
        }
        if (rule_action->value)
        {
            if (rhs_value_is_funcall(rule_action->value))
            {
                if (!thisAgent->WM->glbDeepCopyWMEs.empty())
                {
                    // Extract identity from first argument of rule_action->value
                    oid_value = get_rhs_function_first_arg_identity(thisAgent, rule_action->value);
                    deallocate_rhs_value(thisAgent, rule_action->value);
                    rule_action->value = NULL;
                } else {
                    f_value = rule_action->value;
                }
            } else {
                oid_value = rhs_value_to_o_id(rule_action->value);
            }
        }
        if (rule_action->referent)
        {
            if (rhs_value_is_funcall(rule_action->referent))
            {
                f_referent = rule_action->referent;
                rule_action->referent = NULL;
            } else {
                oid_referent = rhs_value_to_o_id(rule_action->referent);
            }
        }
    }
    newPref = make_preference(thisAgent, a->preference_type, lId, lAttr, lValue, lReferent, identity_quadruple(oid_id, oid_attr, oid_value, oid_referent), false, was_unbound_vars);
    newPref->rhs_funcs.id = copy_rhs_value(thisAgent, f_id);
    newPref->rhs_funcs.attr = copy_rhs_value(thisAgent, f_attr);
    newPref->rhs_funcs.value = copy_rhs_value(thisAgent, f_value);
    newPref->rhs_funcs.referent = copy_rhs_value(thisAgent, f_referent);
    newPref->parent_action = a;
    return newPref;

abort_execute_action: /* control comes here when some error occurred */
    if (lId)
    {
        thisAgent->symbolManager->symbol_remove_ref(&lId);
    }
    if (lAttr)
    {
        thisAgent->symbolManager->symbol_remove_ref(&lAttr);
    }
    if (lValue)
    {
        thisAgent->symbolManager->symbol_remove_ref(&lValue);
    }
    if (lReferent)
    {
        thisAgent->symbolManager->symbol_remove_ref(&lReferent);
    }
    return NIL;
}

/* -----------------------------------------------------------------------
                    Run-Time O-Support Calculation

   This routine calculates o-support for each preference for the given
   instantiation, filling in pref->o_supported (true or false) on each one.

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
void calculate_support_for_instantiation_preferences(agent* thisAgent, instantiation* inst, instantiation* original_inst)
{
    preference* pref;
    wme* w;
    condition* c;
    action*    act;
    bool      o_support, op_elab;
    bool      operator_proposal;
    int       pass;
    wme*       lowest_goal_wme;

    o_support = false;
    op_elab = false;

    if (inst->prod->declared_support == DECLARED_O_SUPPORT)
    {
        o_support = true;
    }
    else if (inst->prod->declared_support == DECLARED_I_SUPPORT)
    {
        o_support = false;
    }
    else if (inst->prod->declared_support == UNDECLARED_SUPPORT)
    {
        /* check if the instantiation is proposing an operator.  if it is, then this instantiation is i-supported.  */

        operator_proposal = false;
        instantiation* non_variabilized_inst = original_inst ? original_inst : inst;

        if (non_variabilized_inst->rete_wme)
        {
            for (act = non_variabilized_inst->prod->action_list; act != NIL ; act = act->next)
            {
                if ((act->type == MAKE_ACTION)  &&
                        (rhs_value_is_symbol(act->attr)) &&
                        (rhs_value_to_rhs_symbol(act->attr)->referent == thisAgent->symbolManager->soarSymbols.operator_symbol) &&
                        (act->preference_type == ACCEPTABLE_PREFERENCE_TYPE))
                {
                    if (rhs_value_is_reteloc(act->id) &&
                        get_symbol_from_rete_loc(
                            rhs_value_to_reteloc_levels_up(act->id),
                            rhs_value_to_reteloc_field_num(act->id),
                            non_variabilized_inst->rete_token,
                            non_variabilized_inst->rete_wme)->is_state())
                    {
                        operator_proposal = true;
                        o_support = false;
                        break;
                    }
                    else if (rhs_value_is_symbol(act->id))
                    {
                        Symbol* lSym = rhs_value_to_symbol(act->id);
                        /* -- Not sure rhs id can even be a symbol at this point.  Temporary warning here. -- */
                        thisAgent->outputManager->printa_sf(thisAgent, "ERROR!  Unexpected symbol %y in calculate_support_for_instantiation_preferences(). Please report"
                              " to Soar Umich group.\n", lSym);
                        if (lSym->is_state())
                        {
                            operator_proposal = true;
                            o_support = false;
                            break;
                        }
                    }
                }
            }
        }

        if (operator_proposal == false)
        {
            /* An operator wasn't being proposed, so now we need to test if
            the operator is being tested on the LHS.

            i'll need to make two passes over the wmes that pertain to
            this instantiation.  the first pass looks for the lowest goal
            identifier.  the second pass looks for a wme of the form:

            (<lowest-goal-id> ^operator ...)

            if such a wme is found, then this o-support = true; false otherwise.

            this code is essentially identical to that in
            p_node_left_addition() in rete.c. */

            lowest_goal_wme = NIL;

            for (pass = 0; pass != 2; pass++)
            {

                for (c = inst->top_of_instantiated_conditions; c != NIL; c = c->next)
                {
                    if (c->type == POSITIVE_CONDITION)
                    {
                        w = c->bt.wme_;

                        if (pass == 0)
                        {

                            if (w->id->id->isa_goal == true)
                            {

                                if (lowest_goal_wme == NIL)
                                {
                                    lowest_goal_wme = w;
                                }

                                else
                                {
                                    if (w->id->id->level > lowest_goal_wme->id->id->level)
                                    {
                                        lowest_goal_wme = w;
                                    }
                                }
                            }

                        }

                        else
                        {
                            if ((w->attr == thisAgent->symbolManager->soarSymbols.operator_symbol) &&
                                (w->acceptable == false) &&
                                (w->id == lowest_goal_wme->id))
                            {
                                /* iff RHS has only operator elaborations
                                    then it's IE_PROD, otherwise PE_PROD, so
                                    look for non-op-elabs in the actions  KJC 1/00 */
                                for (act = inst->prod->action_list;
                                    act != NIL ; act = act->next)
                                {
                                    if (act->type == MAKE_ACTION)
                                    {
                                        if ((rhs_value_is_symbol(act->id)) &&
                                            (rhs_value_to_symbol(act->id) == w->value))
                                        {
                                            op_elab = true;
                                        }
                                        else if (rhs_value_is_reteloc(act->id) &&
                                            w->value == get_symbol_from_rete_loc(rhs_value_to_reteloc_levels_up(act->id), rhs_value_to_reteloc_field_num(act->id), inst->rete_token, w))
                                        {
                                            op_elab = true;
                                        }
                                        else
                                        {
                                            /* this is not an operator elaboration */
                                            o_support = true;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if (o_support == true)
    {

        if (op_elab == true)
        {
            thisAgent->outputManager->printa_sf(thisAgent, "\nWARNING:  Operator elaborations mixed with operator applications\nAssigning i_support to prod %y", inst->prod_name);

            growable_string gs = make_blank_growable_string(thisAgent);
            add_to_growable_string(thisAgent, &gs, "WARNING:  Operator elaborations mixed with operator applications\nAssigning i_support to prod ");
            add_to_growable_string(thisAgent, &gs, inst->prod_name->to_string(true));
            xml_generate_warning(thisAgent, text_of_growable_string(gs));
            free_growable_string(thisAgent, gs);

            o_support = false;
        }
    }

    for (pref = inst->preferences_generated; pref != NIL; pref = pref->inst_next)
    {
        pref->o_supported = o_support;
    }
}

/* -----------------------------------------------------------------------
 Finalize New Instantiation Stuff

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
 - if "need_to_do_support_calculations" is true, calculates o-support
 for preferences_generated;
 ----------------------------------------------------------------------- */

void finalize_instantiation(agent* thisAgent, instantiation* inst, bool need_to_do_support_calculations, instantiation*  original_inst, bool addToGoal)
{
    condition* cond;
    preference* p;

    /* We don't add a prod refcount for justifications so that they will be
     * excised when they no longer match or no longer have preferences asserted */
    if (inst->prod && (inst->prod->type != JUSTIFICATION_PRODUCTION_TYPE))
    {
        production_add_ref(inst->prod);
    }

    for (cond = inst->top_of_instantiated_conditions; cond != NIL; cond = cond->next)
    {
        if (cond->type == POSITIVE_CONDITION)
        {
            wme_add_ref(cond->bt.wme_);

            /* if trace is for a lower level, find one for this level */
            if (cond->bt.trace)
            {
                if (cond->bt.trace->inst->match_goal_level > inst->match_goal_level)
                {
                    cond->bt.trace =  find_clone_for_level(cond->bt.trace, inst->match_goal_level);
                }
                if (cond->bt.trace)
                {
                    preference_add_ref(cond->bt.trace);
                }
            }
        }
        cond->inst = inst;
    }

    if (addToGoal)
    {
        assert(inst->match_goal);
        for (p = inst->preferences_generated; p != NIL; p = p->inst_next)
        {
            insert_at_head_of_dll(inst->match_goal->id->preferences_from_goal, p, all_of_goal_next, all_of_goal_prev);
            p->on_goal_list = true;
        }
    }
    if (need_to_do_support_calculations)
    {
        calculate_support_for_instantiation_preferences(thisAgent, inst, original_inst);
    }
}

void add_pref_to_inst(agent* thisAgent, preference* pref, instantiation* inst)
{
    pref->inst = inst;
    pref->level = inst->match_goal_level;

    /* The parser cannot determine if a rhs preference (<s> ^operator <o> = <x>) is a relative
     * operator indifferent preference or numeric indifferent preference until it matches, so we
     * set it here. */
    if ((pref->type == BINARY_INDIFFERENT_PREFERENCE_TYPE) && (pref->referent->is_float() || pref->referent->is_int()))
    {
        pref->type = NUMERIC_INDIFFERENT_PREFERENCE_TYPE;
    }

    if (inst->prod)
    {
      if  (inst->prod->declared_support == DECLARED_O_SUPPORT)
      {
          pref->o_supported = true;
      } else if (inst->prod->declared_support == DECLARED_I_SUPPORT)
      {
          pref->o_supported = false;
      }
      else
      {
          pref->o_supported = (thisAgent->FIRING_TYPE == PE_PRODS) ? true : false;
      }
    }
    else
    {
        pref->o_supported = (thisAgent->FIRING_TYPE == PE_PRODS) ? true : false;
    }
    insert_at_head_of_dll(inst->preferences_generated, pref, inst_next,  inst_prev);
}

void add_cond_to_arch_inst(agent* thisAgent, condition* &prev_cond, instantiation* inst, wme* pWME, bool addBTPref = true)
{
    condition * cond;

    cond = make_condition(thisAgent,
        make_test(thisAgent, pWME->id , EQUALITY_TEST),
        make_test(thisAgent, pWME->attr, EQUALITY_TEST),
        make_test(thisAgent, pWME->value, EQUALITY_TEST));
    thisAgent->SMem->add_identity_to_iSTI_test(cond->data.tests.id_test, inst->i_id);
    if (cond->data.tests.attr_test->data.referent->is_sti())
        thisAgent->SMem->add_identity_to_iSTI_test(cond->data.tests.attr_test, inst->i_id);
    if (cond->data.tests.value_test->data.referent->is_sti())
        thisAgent->SMem->add_identity_to_iSTI_test(cond->data.tests.value_test, inst->i_id);
    cond->prev = prev_cond;
    cond->next = NULL;
    if (prev_cond != NULL)
    {
        prev_cond->next = cond;
        inst->bottom_of_instantiated_conditions = cond;
    }
    else
    {
        inst->top_of_instantiated_conditions = cond;
        inst->bottom_of_instantiated_conditions = cond;
    }
    cond->test_for_acceptable_preference = pWME->acceptable;
    cond->bt.wme_ = pWME;
    cond->inst = inst;

    cond->bt.level = pWME->id->id->level;

    if (addBTPref && pWME->preference)
    {
        cond->bt.trace = pWME->preference;
    }
//            preference_add_ref(cond->bt.trace);

    prev_cond = cond;
}

void add_pref_to_arch_inst(agent* thisAgent, instantiation* inst, Symbol* pID, Symbol* pAttr, Symbol* pValue)
{
    preference* pref;

    pref = make_preference(thisAgent, ACCEPTABLE_PREFERENCE_TYPE, pID, pAttr, pValue,  NIL);
//    pref->o_supported = true;
    thisAgent->symbolManager->symbol_add_ref(pref->id);
    thisAgent->symbolManager->symbol_add_ref(pref->attr);
    thisAgent->symbolManager->symbol_add_ref(pref->value);
    pref->identities.id = thisAgent->SMem->get_identity_for_iSTI(pref->id, inst->i_id);
    if (pref->attr->is_sti())
        pref->identities.attr = thisAgent->SMem->get_identity_for_iSTI(pref->attr, inst->i_id);
    if (pref->value->is_sti())
    pref->identities.value = thisAgent->SMem->get_identity_for_iSTI(pref->value, inst->i_id);

    add_pref_to_inst(thisAgent, pref, inst);
    /* MToDo | If smem retrievals leak, try moving this to make_arch_for_impasse_item */
    preference_add_ref(inst->preferences_generated);

}

void add_deep_copy_prefs_to_inst(agent* thisAgent, preference* pref, instantiation* inst)
{

    deep_copy_wme* lNewDC_WME;
    goal_stack_level glbDeepCopyWMELevel = 0;
    condition* prev_cond = NULL;
    preference* lPref;

    glbDeepCopyWMELevel = pref->id->id->level;

    for (auto it = thisAgent->WM->glbDeepCopyWMEs.begin();  it != thisAgent->WM->glbDeepCopyWMEs.end(); it++)
    {
        lNewDC_WME = *it;

        /* Set the id levels.  Root and any re-visited STIs will already have it set */
        if (lNewDC_WME->id->id->level == NO_WME_LEVEL)
        {
            lNewDC_WME->id->id->level = glbDeepCopyWMELevel;
        }
        if (lNewDC_WME->attr->is_sti() && lNewDC_WME->attr->id->level == NO_WME_LEVEL)
        {
            lNewDC_WME->attr->id->level = glbDeepCopyWMELevel;
        }
        if (lNewDC_WME->value->is_sti() && lNewDC_WME->value->id->level == NO_WME_LEVEL)
        {
            lNewDC_WME->value->id->level = glbDeepCopyWMELevel;
        }

        /* Add a condition that tests the WME that the deep copied preference is based on */
        add_cond_to_arch_inst(thisAgent, inst->bottom_of_instantiated_conditions, inst, lNewDC_WME->deep_copied_wme);

        /* Add the copied preference */
        /* We may need to add refcounts for this pref?  I think they may already be adjusted when copied in the rhs function */
        lPref = make_preference(thisAgent, ACCEPTABLE_PREFERENCE_TYPE, lNewDC_WME->id, lNewDC_WME->attr, lNewDC_WME->value, NULL);

        /* We set the identities of the preferences so that they are dependent on the explanation behind
         * the working memory elements that they were copied from */
        lPref->identities.id = thisAgent->SMem->get_identity_for_iSTI(lNewDC_WME->deep_copied_wme->id, inst->i_id);
        lPref->identities.attr = thisAgent->SMem->get_identity_for_iSTI(lNewDC_WME->deep_copied_wme->attr, inst->i_id);
        lPref->identities.value = thisAgent->SMem->get_identity_for_iSTI(lNewDC_WME->deep_copied_wme->value, inst->i_id);

        /* Now add a preferences that will create the deep-copied WME */
        add_pref_to_inst(thisAgent, lPref, inst);

        /* Clean up the deep copy struct.  We didn't increase refcounts on their symbols. */
        delete lNewDC_WME;
    }

    thisAgent->WM->glbDeepCopyWMEs.clear();

}

void init_instantiation(agent* thisAgent, instantiation* &inst, Symbol* backup_name, production* prod, struct token_struct* tok, wme* w)
{
    thisAgent->memoryManager->allocate_with_pool(MP_instantiation, &inst);
    inst->i_id = thisAgent->explanationBasedChunker->get_new_inst_id();
    inst->next = inst->prev = NULL;
    inst->prod = prod;
    inst->rete_token = tok;
    inst->rete_wme = w;

    inst->backtrace_number = 0;
    inst->tested_quiescence = false;
    inst->tested_local_negation = false;
    inst->creates_deep_copy = false;
    inst->tested_LTM = false;

    inst->in_ms = false;
    inst->in_newly_created = false;
    inst->in_newly_deleted = false;
    inst->GDS_evaluated_already = false;

    inst->top_of_instantiated_conditions = NULL;
    inst->bottom_of_instantiated_conditions = NULL;
    inst->preferences_generated = NULL;
    inst->preferences_cached = NULL;
    inst->OSK_prefs = NULL;
    inst->OSK_proposal_prefs = NULL;
    inst->OSK_proposal_slot = NULL;

    inst->explain_status = explain_unrecorded;
    inst->explain_depth = 0;
    inst->explain_tc_num = 0;

    inst->prod_name = prod ? prod->name : backup_name ? backup_name : NULL;
    if (inst->prod_name)
    {
        thisAgent->symbolManager->symbol_add_ref(inst->prod_name);
        IDI_add(thisAgent, inst);
    }
}

inline bool trace_firings_of_inst(agent* thisAgent, instantiation* inst)
{
    return ((inst)->prod && (thisAgent->trace_settings[TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM + (inst)->prod->type] || ((inst)->prod->trace_firings)));
}


/* -----------------------------------------------------------------------
 Create Instantiation

 This builds the instantiation for a new match, and adds it to
 newly_created_instantiations.  It also calls chunk_instantiation() to
 do any necessary chunk or justification building.
 ----------------------------------------------------------------------- */
void create_instantiation(agent* thisAgent, production* prod, struct token_struct* tok, wme* w)
{
    instantiation* inst;
    condition* cond;
    preference* pref;
    action* a, *a2, *rhs_vars = NULL;
    cons* c;
    bool trace_it;
    int64_t index;
    Symbol** cell;

    #ifdef BUG_139_WORKAROUND  /* This is now checked for before we call this function */
    assert(prod->type != JUSTIFICATION_PRODUCTION_TYPE);
    #endif

    debug_refcount_change_start(thisAgent, true);
    init_instantiation(thisAgent, inst, thisAgent->symbolManager->soarSymbols.architecture_inst_symbol, prod, tok, w);
    inst->next = thisAgent->newly_created_instantiations;
    thisAgent->newly_created_instantiations = inst;
    inst->in_newly_created = true;
    inst->in_ms = true;

    dprint_header(DT_MILESTONES, PrintBefore, "Allocating instantiation for instance of %u (%y) begun.\n", inst->i_id, inst->prod_name);
    dprint(DT_DEALLOCATE_INST, "Allocating instantiation for instance of %u (%y) begun.\n", inst->i_id, inst->prod_name);

    if (thisAgent->trace_settings[TRACE_ASSERTIONS_SYSPARAM])
    {
        std::string lStr;
        thisAgent->outputManager->sprinta_sf(thisAgent, lStr, "\nNew match of %y.  Creating instantiation.\n", inst->prod_name);
        thisAgent->outputManager->printa(thisAgent, lStr.c_str());
        xml_generate_verbose(thisAgent, lStr.c_str());
    }

    thisAgent->production_being_fired = inst->prod;
    prod->firing_count++;
    thisAgent->production_firing_count++;

    AddAdditionalTestsMode additional_test_mode = (prod->type == TEMPLATE_PRODUCTION_TYPE) ? JUST_INEQUALITIES: ALL_ORIGINALS;

    /* build the instantiated conditions, and bind LHS variables */
    p_node_to_conditions_and_rhs(thisAgent, prod->p_node, tok, w,
        &(inst->top_of_instantiated_conditions),
        &(inst->bottom_of_instantiated_conditions), &(rhs_vars),
        inst->i_id, additional_test_mode);

    /* record the level of each of the wmes that was positively tested */
    for (cond = inst->top_of_instantiated_conditions; cond != NIL; cond = cond->next)
    {
        cond->inst = inst;
        if (cond->type == POSITIVE_CONDITION)
        {
            cond->bt.level = cond->bt.wme_->id->id->level;
            cond->bt.trace = cond->bt.wme_->preference;  // These are later changed to the correct clone for the level
        }
    }
    set_bt_and_find_match_goal(inst);

    /* print trace info */
    trace_it = trace_firings_of_inst(thisAgent, inst);
    if (trace_it)
    {
        thisAgent->outputManager->start_fresh_line(thisAgent);
        thisAgent->outputManager->printa(thisAgent,  "Firing ");
        print_instantiation_with_wmes(thisAgent, inst, static_cast<wme_trace_type>(thisAgent->trace_settings[TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM]), 0);
    }

    /* initialize rhs_variable_bindings array with names of variables
     (if there are any stored on the production -- for chunks there won't be any) */
    index = 0;
    cell = thisAgent->rhs_variable_bindings;
    for (c = prod->rhs_unbound_variables; c != NIL; c = c->rest)
    {
        *(cell++) = static_cast<symbol_struct*>(c->first);
        index++;
    }
    thisAgent->firer_highest_rhs_unboundvar_index = index - 1;

    /* Before executing the RHS actions, tell the user that the -- */
    /* phase has changed to output by printing the arrow */
    if (trace_it)
    {
        thisAgent->outputManager->printa(thisAgent,  " -->\n");
        xml_object(thisAgent, kTagActionSideMarker);
    }

    /* execute the RHS actions, collect the results */
    a2 = rhs_vars;

    for (a = prod->action_list; a != NIL; a = a->next)
    {
        if (prod->type != TEMPLATE_PRODUCTION_TYPE)
        {
            dprint(DT_RL_VARIABLIZATION, "Executing action for non-template production.\n");
            if (a2)
            {
                pref = execute_action(thisAgent, a, tok, w, a2);
            } else {
                pref = execute_action(thisAgent, a, tok, w, NULL);
            }
        }
        else
        {
            dprint(DT_RL_VARIABLIZATION, "Executing action for template production.  (building template instantiation)\n");
            pref = NIL;
            rl_build_template_instantiation(thisAgent, inst, tok, w, a2);

        }
        if (pref)
        {
            add_pref_to_inst(thisAgent, pref, inst);
            if (!thisAgent->WM->glbDeepCopyWMEs.empty())
            {
                inst->creates_deep_copy = true;
                thisAgent->SMem->force_add_identity_for_STI(pref->value, pref->identities.value);
                thisAgent->SMem->force_add_identity_for_STI(thisAgent->explanationBasedChunker->deep_copy_sym_expanded, pref->identities.value);
                thisAgent->explanationBasedChunker->deep_copy_sym_expanded = NULL;
                add_deep_copy_prefs_to_inst(thisAgent, pref, inst);
            }
        }

        if (a2)  a2 = a2->next;
    }

    /* reset rhs_variable_bindings array to all zeros */
    index = 0;
    cell = thisAgent->rhs_variable_bindings;
    while (index++ <= thisAgent->firer_highest_rhs_unboundvar_index)
    {
        *(cell++) = NIL;
    }

    /* fill in lots of other stuff */
    finalize_instantiation(thisAgent, inst, false, NIL, true);

    /* print trace info: printing preferences */
    /* Note: can't move this up, since fill_in_new_instantiation_stuff gives
     the o-support info for the preferences we're about to print */
    if (trace_it || thisAgent->trace_settings[TRACE_FIRINGS_PREFERENCES_SYSPARAM])
    {
        for (pref = inst->preferences_generated; pref != NIL; pref = pref->inst_next)
        {
            if (thisAgent->trace_settings[TRACE_FIRINGS_PREFERENCES_SYSPARAM])
                thisAgent->outputManager->printa_sf(thisAgent,  "%e+ ");
            else
                thisAgent->outputManager->printa(thisAgent,  " ");
            print_preference(thisAgent, pref);
        }
    }

    thisAgent->explanationBasedChunker->set_learning_for_instantiation(inst);

    /* Copy any operator selection knowledge preferences for conditions of this instantiation */
    thisAgent->explanationBasedChunker->copy_OSK(inst);

    thisAgent->production_being_fired = NIL;

    dprint(DT_PRINT_INSTANTIATIONS,  "Created instantiation for match of %y (%u) in %y (%d) : \n%5", inst->prod_name, inst->i_id, inst->match_goal, static_cast<long long>(inst->match_goal_level), inst->top_of_instantiated_conditions, inst->preferences_generated);

    /* Clean up variable to identity mappings for this instantiation */
    thisAgent->explanationBasedChunker->cleanup_after_instantiation_creation();

    /* build chunks/justifications if necessary */

    thisAgent->explanationBasedChunker->learn_EBC_rule(inst, &(thisAgent->newly_created_instantiations));

    deallocate_action_list(thisAgent, rhs_vars);
    debug_refcount_change_end(thisAgent, (std::string(inst->prod_name->sc->name) + std::string(" instantiation creation")).c_str(), true);

    dprint_header(DT_MILESTONES, PrintAfter, "Created instantiation for match of %y (%u) finished in state %y(%d).\n", inst->prod_name, inst->i_id, inst->match_goal, static_cast<long long>(inst->match_goal_level));

    if (!thisAgent->system_halted)
    {
        /* invoke callback function */
        soar_invoke_callbacks(thisAgent, FIRING_CALLBACK, static_cast<soar_call_data>(inst));

    }
}

/* -----------------------------------------------------------------------
 Deallocate Instantiation

 This deallocates the given instantiation.  This should only be invoked
 from possibly_deallocate_instantiation().
 ----------------------------------------------------------------------- */

void deallocate_instantiation(agent* thisAgent, instantiation*& inst)
{
    dprint(DT_DEALLOCATE_INST, "Deallocating instantiation called for %u (%y)\n", inst->i_id, inst->prod_name);
    if (inst->in_newly_created)
    {
        dprint(DT_DEALLOCATE_INST, "Skipping deallocation of instantiation %u (%y) because it is still on the newly created instantiation list.\n", inst->i_id, inst->prod_name);
        if (!inst->in_newly_deleted)
        {
            inst->in_newly_deleted = true;
            thisAgent->newly_deleted_instantiations.push_back(inst);
        }
        return;
    }

    condition_list cond_stack;
    inst_list l_instantiation_list;
    instantiation* lInst;

    l_instantiation_list.push_back(inst);
    inst_list::iterator next_iter = l_instantiation_list.begin();

    while (next_iter != l_instantiation_list.end())
    {
        lInst = *next_iter;
        assert(lInst);
        ++next_iter;
        dprint(DT_DEALLOCATE_INST, "Deallocating instantiation: Stage 1 for %u (%y)\n", lInst->i_id, lInst->prod_name);
        debug_refcount_change_start(thisAgent, false);

        for (condition* cond = lInst->top_of_instantiated_conditions; cond != NIL; cond = cond->next)
        {
            if (cond->type == POSITIVE_CONDITION)
            {

                /*  voigtjr, nlderbin:
                 We flattened out the following recursive loop in order to prevent a stack
                 overflow that happens when the chain of backtrace instantiations is very long:

                 retract_instantiation
                 possibly_deallocate_instantiation
                 loop start:
                 deallocate_instantiation (here)
                 preference_remove_ref
                 possibly_deallocate_preferences_and_clones
                 deallocate_preference
                 possibly_deallocate_instantiation
                 goto loop start

                 Note:  If one of those functions that are flattened out here changes, this code may
                        need updating. - Mazin */

                wme_remove_ref(thisAgent, cond->bt.wme_);

                if (cond->bt.trace)
                {
                    preference* lPref = cond->bt.trace;
                    /* -----------------------
                     * preference_remove_ref()
                     * ----------------------- */
                    #ifndef DO_TOP_LEVEL_PREF_REF_CTS
                    assert(lPref->level);
                    if (lPref->level > TOP_GOAL_LEVEL)
                    #endif
                    {
                        if (lPref->reference_count != 0)
                            lPref->reference_count--;
                        if (lPref->reference_count == 0)
                        {
                            /* possibly_deallocate_preference_and_clones() */
                            preference* clone;
                            preference* next;
                            bool has_active_clones = false;

                            dprint(DT_DEALLOCATE_PREF, "Possibly deallocating preference %p and clones...\n", lPref);

                            for (clone = lPref->next_clone; clone != NIL; clone = clone->next_clone)
                                if (clone->reference_count) has_active_clones = true;
                            if (has_active_clones) continue;
                            for (clone = lPref->prev_clone; clone != NIL; clone = clone->prev_clone)
                                if (clone->reference_count) has_active_clones = true;
                            if (has_active_clones) continue;

                            dprint(DT_DEALLOCATE_PREF, "Deallocating clones of %p...\n", lPref);
                            clone = lPref->next_clone;
                            while (clone)
                            {
                                next = clone->next_clone;
                                deallocate_preference(thisAgent, clone);
                                clone = next;
                            }
                            clone = lPref->prev_clone;
                            while (clone)
                            {
                                next = clone->prev_clone;
                                deallocate_preference(thisAgent, clone);
                                clone = next;
                            }

                            /* ------------------------------
                             * deallocate_preference() part 1
                             * ------------------------------ */
                            if (lPref->on_goal_list)
                            {
                                remove_from_dll(lPref->inst->match_goal->id->preferences_from_goal, lPref, all_of_goal_next, all_of_goal_prev);
                                lPref->on_goal_list = false;
                            }

                            remove_from_dll(lPref->inst->preferences_generated, lPref, inst_next, inst_prev);

                            /* -----------------------------------
                             * possibly_deallocate_instantiation()
                             * ----------------------------------- */
                            if ((!lPref->inst->preferences_generated) && (!lPref->inst->in_ms))
                            {
                                next_iter = l_instantiation_list.insert(next_iter, lPref->inst);
                            }

                            cond_stack.push_back(cond);
                        }
                    }
                }
            }
        }
        debug_refcount_change_end(thisAgent, (std::string(lInst->prod_name->sc->name) + std::string(" instantiation deallocation - wme and clones")).c_str(), false);
    }

    /* --------------------------------------------------
     * deallocate_preference() part 2 (cleans up bt pref)
     * -------------------------------------------------- */
    debug_refcount_change_start(thisAgent, false);
    while (!cond_stack.empty())
    {
        condition* temp = cond_stack.back();
        cond_stack.pop_back();

        cache_preference_if_necessary(thisAgent, temp->bt.trace);
        PDI_remove(thisAgent, temp->bt.trace);

        /* dereference component symbols */
        thisAgent->symbolManager->symbol_remove_ref(&temp->bt.trace->id);
        thisAgent->symbolManager->symbol_remove_ref(&temp->bt.trace->attr);
        thisAgent->symbolManager->symbol_remove_ref(&temp->bt.trace->value);
        if (preference_is_binary(temp->bt.trace->type))
        {
            thisAgent->symbolManager->symbol_remove_ref(&temp->bt.trace->referent);
        }

        if (temp->bt.trace->wma_o_set)
        {
            wma_remove_pref_o_set(thisAgent, temp->bt.trace);
        }
        if (temp->bt.trace->rhs_funcs.id) deallocate_rhs_value(thisAgent, temp->bt.trace->rhs_funcs.id);
        if (temp->bt.trace->rhs_funcs.attr) deallocate_rhs_value(thisAgent, temp->bt.trace->rhs_funcs.attr);
        if (temp->bt.trace->rhs_funcs.value) deallocate_rhs_value(thisAgent, temp->bt.trace->rhs_funcs.value);
        if (temp->bt.trace->rhs_funcs.referent) deallocate_rhs_value(thisAgent, temp->bt.trace->rhs_funcs.referent);
        if (temp->bt.trace->cloned_rhs_funcs.id) deallocate_rhs_value(thisAgent, temp->bt.trace->cloned_rhs_funcs.id);
        if (temp->bt.trace->cloned_rhs_funcs.attr) deallocate_rhs_value(thisAgent, temp->bt.trace->cloned_rhs_funcs.attr);
        if (temp->bt.trace->cloned_rhs_funcs.value) deallocate_rhs_value(thisAgent, temp->bt.trace->cloned_rhs_funcs.value);
        if (temp->bt.trace->cloned_rhs_funcs.referent) deallocate_rhs_value(thisAgent, temp->bt.trace->cloned_rhs_funcs.referent);

        /* free the memory */
        thisAgent->memoryManager->free_with_pool(MP_preference, temp->bt.trace);
    }
    debug_refcount_change_end(thisAgent, (std::string(inst->prod_name->sc->name) + std::string(" instantiation deallocation: cumulative bt pref cleanup")).c_str(), false);

    // free instantiations in the reverse order

    dprint(DT_DEALLOCATE_INST, "Freeing instantiation dealloc list built from %u (%y)\n", inst->i_id, inst->prod_name);
    for (inst_list::reverse_iterator riter = l_instantiation_list.rbegin(); riter != l_instantiation_list.rend(); ++riter)
    {
        instantiation* lDelInst = *riter;

        debug_refcount_change_start(thisAgent, false);
        dprint(DT_MILESTONES, "Deallocating instantiation: Stage 2 for %u (%y)\n", lDelInst->i_id, lDelInst->prod_name);
        dprint(DT_DEALLOCATE_INST, "Deallocating instantiation: Stage 2 for %u (%y)\n", lDelInst->i_id, lDelInst->prod_name);

        thisAgent->symbolManager->symbol_remove_ref(&lDelInst->prod_name);

        deallocate_condition_list(thisAgent, lDelInst->top_of_instantiated_conditions);

        /* Clean up operator selection knowledge */
        if (lDelInst->OSK_prefs)
        {
            dprint(DT_OSK, "Cleaning up OSK preference contained in inst %u %y\n", lDelInst->i_id, lDelInst->prod_name);
            clear_preference_list(thisAgent, lDelInst->OSK_prefs);
        }
        if (lDelInst->OSK_proposal_prefs)
        {
            dprint(DT_OSK, "Cleaning up OSK proposal preference contained in inst %u %y\n", lDelInst->i_id, lDelInst->prod_name);
            /* These prefs did not have their refcounts increased, so we don't want to call clear_preference_list */
            free_list(thisAgent, lDelInst->OSK_proposal_prefs);
            lDelInst->OSK_proposal_prefs = NULL;
        }
        if (lDelInst->OSK_proposal_slot)
        {
            assert(lDelInst->OSK_proposal_slot->instantiation_with_temp_OSK = lDelInst);
            lDelInst->OSK_proposal_slot->instantiation_with_temp_OSK = NULL;
        }

        /* Clean up preferences cached for possible explanations */
        preference* next_pref;
        while (lDelInst->preferences_cached)
        {
            next_pref = lDelInst->preferences_cached->inst_next;
            dprint(DT_EXPLAIN_CACHE, "Deallocating cached preference for instantiation %u (match of %y): %p\n", lDelInst->i_id, lDelInst->prod_name, lDelInst->preferences_cached);
            deallocate_preference(thisAgent, lDelInst->preferences_cached);
            lDelInst->preferences_cached = next_pref;
        }

        if (lDelInst->prod)
        {
            dprint(DT_DEALLOCATE_PROD, "  Removing production reference for i %u (%y = %d).\n", lDelInst->i_id, lDelInst->prod->name, lDelInst->prod->reference_count);
            if ((lDelInst->prod->type == JUSTIFICATION_PRODUCTION_TYPE) && (lDelInst->prod->reference_count == 1))
            {
                /* We are about to remove a justification that has not been excised from the rete.
                 * Normally, justifications are excised as soon as they don't have any matches in
                 * rete.cpp.  But if removing the preference will remove the instantiation, we
                 * need to excise it now so that the rete doesn't try to later */
                excise_production(thisAgent, lDelInst->prod, false, true);
            } else {
                production_remove_ref(thisAgent, lDelInst->prod);
            }
        }

        debug_refcount_change_end(thisAgent, (std::string(lDelInst->prod_name->sc->name) + std::string(" instantiation deallocation 3 of")).c_str(), false);
        IDI_remove(thisAgent, lDelInst->i_id);
        thisAgent->memoryManager->free_with_pool(MP_instantiation, lDelInst);
    }
    dprint(DT_DEALLOCATE_INST, "Deallocate instantiation complete.\n");
    inst = NULL;
}

/* -----------------------------------------------------------------------
 Retract Instantiation

 This retracts the given instantiation.
 ----------------------------------------------------------------------- */

void retract_instantiation(agent* thisAgent, instantiation* inst)
{
    preference* pref, *next;
    bool retracted_a_preference;
    bool trace_it;

    /* invoke callback function */
    soar_invoke_callbacks(thisAgent, RETRACTION_CALLBACK,
                          static_cast<soar_call_data>(inst));

    retracted_a_preference = false;

    trace_it = trace_firings_of_inst(thisAgent, inst);

    /* retract any preferences that are in TM and aren't o-supported */
    pref = inst->preferences_generated;

    debug_refcount_change_start(thisAgent, true);
    while (pref != NIL)
    {
        next = pref->inst_next;
        if (pref->in_tm && (!pref->o_supported))
        {

            if (trace_it)
            {
                if (!retracted_a_preference)
                {
                    thisAgent->outputManager->start_fresh_line(thisAgent);
                    thisAgent->outputManager->printa(thisAgent,  "Retracting ");
                    print_instantiation_with_wmes(thisAgent, inst, static_cast<wme_trace_type>(thisAgent->trace_settings[TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM]), 1);
                    thisAgent->outputManager->printa(thisAgent,  " -->\n");
                    xml_object(thisAgent, kTagActionSideMarker);
                }
            }
            if (trace_it || thisAgent->trace_settings[TRACE_FIRINGS_PREFERENCES_SYSPARAM])
            {
                if (thisAgent->trace_settings[TRACE_FIRINGS_PREFERENCES_SYSPARAM])
                    thisAgent->outputManager->printa_sf(thisAgent,  "%e- ");
                else
                    thisAgent->outputManager->printa(thisAgent,  " ");
                print_preference(thisAgent, pref);
            }
            remove_preference_from_tm(thisAgent, pref);
            retracted_a_preference = true;
        }
        pref = next;
    }
    debug_refcount_change_end(thisAgent, (std::string(inst->prod_name->sc->name) + std::string(" instantiation retraction of")).c_str(), true);

    /* prod may not exist if rule was manually excised */
    if (inst->prod)
    {
        /* remove inst from list of instantiations of this production */
        remove_from_dll(inst->prod->instantiations, inst, next, prev);

        production* prod = inst->prod;

        if (prod->type == CHUNK_PRODUCTION_TYPE)
        {
            rl_param_container::apoptosis_choices apoptosis = thisAgent->RL->rl_params->apoptosis->get_value();

            /* We care about production history of chunks if we have either
             * - A non-RL rule and all chunks are subject to apoptosis OR
             * - An RL rule that has not been updated by RL AND is not in line to be updated by RL
             */
            if (apoptosis != rl_param_container::apoptosis_none)
            {
                if (  (!prod->rl_rule && (apoptosis == rl_param_container::apoptosis_chunks))
                    || (prod->rl_rule && (static_cast<int64_t>(prod->rl_update_count) == 0)
                                      && (prod->rl_ref_count == 0)))
                {
                    thisAgent->RL->rl_prods->reference_object(prod, 1);
                }
            }
        }
    }

    /* mark as no longer in MS, and possibly deallocate  */
    inst->in_ms = false;
    dprint(DT_DEALLOCATE_INST, "Possibly deallocating instantiation %u (match of %y) for retraction.\n", inst->i_id, inst->prod_name);
    possibly_deallocate_instantiation(thisAgent, inst);
}

instantiation* make_architectural_instantiation(agent* thisAgent, Symbol* pState, wme_set* pConds, symbol_triple_list* pActions)
{
    dprint_header(DT_MILESTONES, PrintBoth, "make_architectural_instantiation() called.\n");

    instantiation* inst;

    init_instantiation(thisAgent, inst, thisAgent->symbolManager->soarSymbols.fake_instantiation_symbol);
    inst->match_goal = pState;
    inst->match_goal_level = pState->id->level;
    inst->tested_LTM = true;

    thisAgent->SMem->clear_instance_mappings();
    dprint(DT_DEALLOCATE_INST, "Allocating architectural instantiation %u (match of %y)\n", inst->i_id, inst->prod_name);

    // create LHS
    {
        condition* prev_cond = NULL;

        add_cond_to_arch_inst(thisAgent, prev_cond, inst, pState->id->smem_info->smem_link_wme, false);
        add_cond_to_arch_inst(thisAgent, prev_cond, inst, pState->id->smem_info->cmd_wme, false);
        add_cond_to_arch_inst(thisAgent, prev_cond, inst, pState->id->smem_info->result_wme, false);

        for (wme_set::iterator c_it = pConds->begin(); c_it != pConds->end(); c_it++)
        {
            add_cond_to_arch_inst(thisAgent, prev_cond, inst, (*c_it));
        }
    }

//    find_match_goal(inst);

    // create RHS
    {
        for (symbol_triple_list::iterator a_it = pActions->begin(); a_it != pActions->end(); a_it++)
        {
            add_pref_to_arch_inst(thisAgent, inst, (*a_it)->id, (*a_it)->attr, (*a_it)->value);
        }
    }

    /* Clean up symbol to identity mappings for this instantiation*/
    thisAgent->explanationBasedChunker->cleanup_after_instantiation_creation();

    /* Initialize levels, add refcounts to prefs/wmes, sets on_goal_list flag, o-support calculation if needed */
    finalize_instantiation(thisAgent, inst, false, NULL, false);

    dprint(DT_PRINT_INSTANTIATIONS,  "Created architectural instantiation %u:\n%5", inst->i_id, inst->top_of_instantiated_conditions, inst->preferences_generated);


    return inst;
}

/* ------------------------------------------------------------------
            Fake Preferences for Goal ^Item Augmentations

   When we backtrace through a (goal ^item) augmentation, we want
   to backtrace to the acceptable preference wme in the supercontext
   corresponding to that ^item.  A slick way to do this automagically
   is to set the backtracing preference pointer on the (goal ^item)
   wme to be a "fake" preference for a "fake" instantiation.  The
   instantiation has as its LHS a list of one condition, which matched
   the acceptable preference wme in the supercontext.

   make_fake_instantiation_for_impasse_item() builds such a fake preference
   and instantiation, given a pointer to the supergoal and the
   acceptable/require preference for the value, and returns a pointer
   to the fake preference.  *** for Soar 8.3, we changed the fake
   preference to be ACCEPTABLE instead of REQUIRE.  This could
   potentially break some code, but it avoids the BUGBUG condition
   that can occur when you have a REQUIRE lower in the stack than an
   ACCEPTABLE but the goal stack gets popped while the WME backtrace
   still points to the REQUIRE, instead of the higher ACCEPTABLE.
   See the section above on Preference Semantics.  It also allows
   the GDS to backtrace through ^items properly.

------------------------------------------------------------------ */

//preference* make_architectural_instantiation_for_impasse_item2(agent* thisAgent, Symbol* goal, preference* cand)
//{
//    slot* s;
//    wme* ap_wme, *ss_link_wme;
//    instantiation* inst;
//    preference* pref;
//    condition* cond, *last_cond;
//    uint64_t state_sym_identity, superop_sym_identity;
//
//    /* find the acceptable preference wme we want to backtrace to */
//    s = cand->slot;
//    for (ap_wme = s->acceptable_preference_wmes; ap_wme != NIL; ap_wme = ap_wme->next)
//        if (ap_wme->value == cand->value)
//        {
//            break;
//        }
//    for (ss_link_wme = goal->id->impasse_wmes; ss_link_wme != NIL; ss_link_wme = ss_link_wme->next)
//    {
//        if (ss_link_wme->attr == thisAgent->symbolManager->soarSymbols.superstate_symbol)
//        {
//            break;
//        }
//    }
//
//    if (!ap_wme || !ss_link_wme)
//    {
//        char msg[BUFFER_MSG_SIZE];
//        strncpy(msg,
//            "decide.c: Internal error: Couldn't find WMEs for architectural instantiation for impasse ^item!\n", BUFFER_MSG_SIZE);
//        msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
//        abort_with_fatal_error(thisAgent, msg);
//    }
//
//    /* make the fake instantiation */
//    init_instantiation(thisAgent, inst, thisAgent->symbolManager->soarSymbols.fake_instantiation_symbol);
//
//    /* make the fake conditions */
//    cond = make_condition(thisAgent);
//    cond->data.tests.id_test = make_test(thisAgent, ap_wme->id, EQUALITY_TEST);
//    cond->data.tests.id_test->identity = thisAgent->explanationBasedChunker->get_or_create_identity(thisAgent->symbolManager->soarSymbols.ss_context_variable, inst->i_id);
//    cond->data.tests.attr_test = make_test(thisAgent, ap_wme->attr, EQUALITY_TEST);
//    cond->data.tests.value_test = make_test(thisAgent, ap_wme->value, EQUALITY_TEST);
//    cond->data.tests.value_test->identity = thisAgent->explanationBasedChunker->get_or_create_identity(thisAgent->symbolManager->soarSymbols.o_context_variable, inst->i_id);
//    superop_sym_identity = cond->data.tests.value_test->identity;
//    /* -- Fill in fake condition info -- */
//    cond->type = POSITIVE_CONDITION;
//    cond->test_for_acceptable_preference = true;
//    cond->bt.wme_ = ap_wme;
//    cond->inst = inst;
//
//    wme_add_ref(ap_wme);
//
//    cond->bt.level = ap_wme->id->id->level;
//
//    last_cond = cond;
//    cond = make_condition(thisAgent);
//    cond->data.tests.id_test = make_test(thisAgent, goal, EQUALITY_TEST);
//    cond->data.tests.id_test->identity = thisAgent->explanationBasedChunker->get_or_create_identity(thisAgent->symbolManager->soarSymbols.s_context_variable, inst->i_id);
//    state_sym_identity = cond->data.tests.id_test->identity;
//    cond->data.tests.attr_test = make_test(thisAgent, thisAgent->symbolManager->soarSymbols.superstate_symbol, EQUALITY_TEST);
//    cond->data.tests.value_test = make_test(thisAgent, ap_wme->id, EQUALITY_TEST);
//    cond->data.tests.value_test->identity = last_cond->data.tests.id_test->identity;
//    cond->next = last_cond;
//    /* -- Fill in fake condition info -- */
//    cond->type = POSITIVE_CONDITION;
//    cond->test_for_acceptable_preference = true;
//    cond->bt.wme_ = ss_link_wme;
//    cond->inst = inst;
//
//    wme_add_ref(ss_link_wme);
//
//    cond->bt.level = goal->id->id->level;
//
//    pref = make_preference(thisAgent, ACCEPTABLE_PREFERENCE_TYPE, goal, thisAgent->symbolManager->soarSymbols.item_symbol,
//        cand->value, NIL, identity_quadruple(state_sym_identity, 0, superop_sym_identity));
//    thisAgent->symbolManager->symbol_add_ref(pref->id);
//    thisAgent->symbolManager->symbol_add_ref(pref->attr);
//    thisAgent->symbolManager->symbol_add_ref(pref->value);
//    insert_at_head_of_dll(goal->id->preferences_from_goal, pref, all_of_goal_next, all_of_goal_prev);
//    pref->on_goal_list = true;
//    preference_add_ref(pref);
//
//    pref->inst = inst;
//    pref->inst_next = pref->inst_prev = NIL;
//
//    /* -- Fill in more instantiation info -- */
//    inst->match_goal = goal;
//    inst->match_goal_level = goal->id->level;
//    inst->top_of_instantiated_conditions = cond;
//    inst->bottom_of_instantiated_conditions = cond;
//    inst->preferences_generated = pref;
//
//    dprint(DT_DEALLOCATE_INST, "Allocating instantiation %u (match of %y)  Architectural item instantiation.\n", inst->i_id, inst->prod_name);
//
//    /* Clean up symbol to identity mappings for this instantiation */
//    thisAgent->explanationBasedChunker->cleanup_after_instantiation_creation();
//
//    return pref;
//}
preference* make_architectural_instantiation_for_impasse_item(agent* thisAgent, Symbol* goal, preference* cand)
{
    slot*           s;
    wme*            ap_wme, *ss_link_wme;
    instantiation*  inst;
    condition*      prev_cond = NULL;

    /* Find the acceptable preference wme we want to backtrace to and the superstate
     * wme to link rule to dependent knowledge in superstate.  Now needed for EBC. */
    s = cand->slot;
    for (ap_wme = s->acceptable_preference_wmes; ap_wme != NIL; ap_wme = ap_wme->next)
        if (ap_wme->value == cand->value) break;
    for (ss_link_wme = goal->id->impasse_wmes; ss_link_wme != NIL; ss_link_wme = ss_link_wme->next)
        if (ss_link_wme->attr == thisAgent->symbolManager->soarSymbols.superstate_symbol) break;

    assert(ap_wme && ss_link_wme);

    thisAgent->SMem->clear_instance_mappings();
    init_instantiation(thisAgent, inst, thisAgent->symbolManager->soarSymbols.fake_instantiation_symbol);

    dprint(DT_DEALLOCATE_INST, "Allocating architectural instantiation for impasse item %u (match of %y)\n", inst->i_id, inst->prod_name);

    /* We already know the match goal, so we can just set it */
    inst->match_goal = goal;
    inst->match_goal_level = goal->id->level;

    // Create the LHS
    add_cond_to_arch_inst(thisAgent, prev_cond, inst, ap_wme, false);
    add_cond_to_arch_inst(thisAgent, prev_cond, inst, ss_link_wme, false);

    // Create the RHS
    add_pref_to_arch_inst(thisAgent, inst, goal, thisAgent->symbolManager->soarSymbols.item_symbol, cand->value);
    inst->preferences_generated->o_supported = false;

    /* Initialize levels, add refcounts to prefs/wmes, sets on_goal_list flag, o-support calculation if needed */
    finalize_instantiation(thisAgent, inst, false, NULL, true);

    /* MToDo | I don't think we actually need this here since we're not using that map to create identities.
     *         Also need to move the smem instance mappings somewhere general */

    /* Clean up symbol to identity mappings for this instantiation*/
    thisAgent->explanationBasedChunker->cleanup_after_instantiation_creation();

    dprint(DT_PRINT_INSTANTIATIONS,  "Created architectural instantiation for impasse item %u:\n%5", inst->i_id, inst->top_of_instantiated_conditions, inst->preferences_generated);

    return inst->preferences_generated;

}
