#include "instantiation.h"

#include "agent.h"
#include "callback.h"
#include "condition.h"
#include "decide.h"
#include "ebc.h"
#include "ebc_identity.h"
#include "ebc_timers.h"
#include "episodic_memory.h"
#include "explanation_memory.h"
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

#include <cassert>
#include <list>
#include <stdlib.h>
#include <string>

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

 REQUIRES: a pref that is not at the target level.  Caller must check.
 ----------------------------------------------------------------------- */

preference* find_clone_for_level(preference* p, goal_stack_level level)
{
    preference* clone;

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

void find_match_goal(agent* thisAgent, instantiation* inst)
{
    Symbol* lowest_goal_so_far;
    goal_stack_level lowest_level_so_far, lowest_id_level_so_far;
    condition* cond;
    Symbol* id;

    lowest_goal_so_far = NIL;
    lowest_level_so_far = -1;
    lowest_id_level_so_far = -1;
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
            if (id->id->level > lowest_id_level_so_far)
            {
                lowest_id_level_so_far = cond->bt.level;
            }
        }

    inst->match_goal = lowest_goal_so_far;
    if (lowest_goal_so_far)
    {
        inst->match_goal_level = lowest_level_so_far;
    }
    else
    {
        inst->match_goal = find_goal_at_goal_stack_level(thisAgent, lowest_id_level_so_far);
        inst->match_goal_level = lowest_id_level_so_far;
//        inst->match_goal_level = ATTRIBUTE_IMPASSE_LEVEL;
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
    uint64_t returnVal = rhs_value_to_inst_identity(static_cast<char*>(firstArg));
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
                oid_id = rhs_value_to_inst_identity(rule_action->id);
            }
        }
        if (rule_action->attr)
        {
            if (rhs_value_is_funcall(rule_action->attr))
            {
                f_attr = rule_action->attr;
                rule_action->attr = NULL;
            } else {
                oid_attr = rhs_value_to_inst_identity(rule_action->attr);
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
                } else {
                    f_value = rule_action->value;
                }
                rule_action->value = NULL;
            } else {
                oid_value = rhs_value_to_inst_identity(rule_action->value);
            }
        }
        if (rule_action->referent)
        {
            if (rhs_value_is_funcall(rule_action->referent))
            {
                f_referent = rule_action->referent;
                rule_action->referent = NULL;
            } else {
                oid_referent = rhs_value_to_inst_identity(rule_action->referent);
            }
        }
    }
    newPref = make_preference(thisAgent, a->preference_type, lId, lAttr, lValue, lReferent, identity_quadruple(oid_id, oid_attr, oid_value, oid_referent), was_unbound_vars);

    /* We don't copy these rhs functions because update_preference_identities needs to copy them to update identity set information later */
    newPref->rhs_func_inst_identities.id = f_id;
    newPref->rhs_func_inst_identities.attr = f_attr;
    newPref->rhs_func_inst_identities.value = f_value;
    newPref->rhs_func_inst_identities.referent = f_referent;
    newPref->parent_action = a;
    return newPref;

abort_execute_action: /* control comes here when some error occurred */
    if (lId) thisAgent->symbolManager->symbol_remove_ref(&lId);
    if (lAttr) thisAgent->symbolManager->symbol_remove_ref(&lAttr);
    if (lValue) thisAgent->symbolManager->symbol_remove_ref(&lValue);
    if (lReferent)  thisAgent->symbolManager->symbol_remove_ref(&lReferent);
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
                if ((act->type == MAKE_ACTION)  && (rhs_value_is_symbol(act->attr)) &&
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
                                if (lowest_goal_wme == NIL) lowest_goal_wme = w;
                                else
                                    if (w->id->id->level > lowest_goal_wme->id->id->level) lowest_goal_wme = w;
                            }

                        }

                        else
                        {
                            if ((w->attr == thisAgent->symbolManager->soarSymbols.operator_symbol) && (w->acceptable == false) && (w->id == lowest_goal_wme->id))
                            {
                                /* iff RHS has only operator elaborations then it's IE_PROD, otherwise PE_PROD, so look for non-op-elabs in the actions  */
                                for (act = inst->prod->action_list;
                                    act != NIL ; act = act->next)
                                {
                                    if (act->type == MAKE_ACTION)
                                    {
                                        if ((rhs_value_is_symbol(act->id)) && (rhs_value_to_symbol(act->id) == w->value))
                                        {
                                            op_elab = true;
                                        }
                                        else if (rhs_value_is_reteloc(act->id) && w->value == get_symbol_from_rete_loc(rhs_value_to_reteloc_levels_up(act->id), rhs_value_to_reteloc_field_num(act->id), inst->rete_token, w))
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
void finalize_instantiation(agent* thisAgent, instantiation* inst, bool need_to_do_support_calculations, instantiation*  original_inst, bool addToGoal, bool is_chunk_inst)
{
    condition* cond;
    preference* p;

    bool isSubGoalMatch = (inst->match_goal_level > TOP_GOAL_LEVEL);
    bool lDoIdentities = (isSubGoalMatch && thisAgent->explanationBasedChunker->ebc_settings[SETTING_EBC_LEARNING_ON]);

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
            #ifndef DO_TOP_LEVEL_COND_REF_CTS
            if (isSubGoalMatch)
            #endif
            {
                wme_add_ref(cond->bt.wme_);
            }
            if (isSubGoalMatch && cond->bt.trace)
            {
                if (cond->bt.trace->level > inst->match_goal_level)
                {
                    cond->bt.trace =  find_clone_for_level(cond->bt.trace, inst->match_goal_level);
                }
                if (cond->bt.trace)
                {
                    preference_add_ref(cond->bt.trace);
                    if (lDoIdentities)
                    {
                        test lTest_id = cond->data.tests.id_test->eq_test;
                        test lTest_attr = cond->data.tests.attr_test->eq_test;
                        test lTest_value = cond->data.tests.value_test->eq_test;

                        if (cond->bt.trace->level == inst->match_goal_level)
                        {
                            if (lTest_id->inst_identity)
                                set_test_identity(thisAgent, lTest_id, thisAgent->explanationBasedChunker->get_or_add_identity(lTest_id->inst_identity, cond->bt.trace->identities.id, inst->match_goal));
                            if (lTest_attr->inst_identity)
                                set_test_identity(thisAgent, lTest_attr, thisAgent->explanationBasedChunker->get_or_add_identity(lTest_attr->inst_identity, cond->bt.trace->identities.attr, inst->match_goal));
                            if (lTest_value->inst_identity)
                                set_test_identity(thisAgent, lTest_value, thisAgent->explanationBasedChunker->get_or_add_identity(lTest_value->inst_identity, cond->bt.trace->identities.value, inst->match_goal));
                        } else if  (inst->match_goal_level > TOP_GOAL_LEVEL) {
                            if (lTest_id->inst_identity)
                                set_test_identity(thisAgent, lTest_id, thisAgent->explanationBasedChunker->get_or_add_identity(lTest_id->inst_identity, NULL_IDENTITY_SET, inst->match_goal));
                            if (lTest_attr->inst_identity)
                                set_test_identity(thisAgent, lTest_attr, thisAgent->explanationBasedChunker->get_or_add_identity(lTest_attr->inst_identity, NULL_IDENTITY_SET, inst->match_goal));
                            if (lTest_value->inst_identity)
                                set_test_identity(thisAgent, lTest_value, thisAgent->explanationBasedChunker->get_or_add_identity(lTest_value->inst_identity, NULL_IDENTITY_SET, inst->match_goal));
                        }
                    }
                }
            }
            /* Check for local singletons */
            if (cond->bt.wme_->local_singleton_value_identity_set && lDoIdentities && (cond->bt.wme_->id == inst->match_goal))
            {
                thisAgent->explanationBasedChunker->force_id_to_identity_mapping(cond->data.tests.id_test->eq_test->inst_identity, cond->bt.wme_->local_singleton_id_identity_set);
                thisAgent->explanationBasedChunker->force_id_to_identity_mapping(cond->data.tests.value_test->eq_test->inst_identity, cond->bt.wme_->local_singleton_value_identity_set);
                set_test_identity(thisAgent, cond->data.tests.id_test->eq_test, cond->bt.wme_->local_singleton_id_identity_set);
                set_test_identity(thisAgent, cond->data.tests.value_test->eq_test, cond->bt.wme_->local_singleton_value_identity_set);
                thisAgent->explanationMemory->increment_stat_identity_propagations();
            }
            if (lDoIdentities)
            {
                /* Architectural WME or we couldn't find a pref at the current level.  Start a new identity set */
                if (!cond->data.tests.id_test->eq_test->identity && cond->data.tests.id_test->eq_test->inst_identity)
                {
                    set_test_identity(thisAgent, cond->data.tests.id_test->eq_test, thisAgent->explanationBasedChunker->get_or_add_identity(cond->data.tests.id_test->eq_test->inst_identity, NULL_IDENTITY_SET, inst->match_goal));
                }
                if (!cond->data.tests.attr_test->eq_test->identity && cond->data.tests.attr_test->eq_test->inst_identity)
                {
                    set_test_identity(thisAgent, cond->data.tests.attr_test->eq_test, thisAgent->explanationBasedChunker->get_or_add_identity(cond->data.tests.attr_test->eq_test->inst_identity, NULL_IDENTITY_SET, inst->match_goal));
                }
                if (!cond->data.tests.value_test->eq_test->identity && cond->data.tests.value_test->eq_test->inst_identity)
                {
                    set_test_identity(thisAgent, cond->data.tests.value_test->eq_test, thisAgent->explanationBasedChunker->get_or_add_identity(cond->data.tests.value_test->eq_test->inst_identity, NULL_IDENTITY_SET, inst->match_goal));
                }
            }
        }
        cond->inst = inst;
    }

    if (lDoIdentities) thisAgent->explanationBasedChunker->update_identities_in_condlist(cond = inst->top_of_instantiated_conditions, inst);

    for (p = inst->preferences_generated; p != NIL; p = p->inst_next)
    {
        if (lDoIdentities)
        {
            thisAgent->explanationBasedChunker->update_identities_in_preferences(p, inst->match_goal, is_chunk_inst);
        }
        if (addToGoal)
        {
            insert_at_head_of_dll(inst->match_goal->id->preferences_from_goal, p, all_of_goal_next, all_of_goal_prev);
            p->on_goal_list = true;
        }
    }
    if (need_to_do_support_calculations) calculate_support_for_instantiation_preferences(thisAgent, inst, original_inst);
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

    /* Add contents */
    cond = make_condition(thisAgent, make_test(thisAgent, pWME->id , EQUALITY_TEST), make_test(thisAgent, pWME->attr, EQUALITY_TEST), make_test(thisAgent, pWME->value, EQUALITY_TEST));
    cond->inst = inst;
    cond->bt.wme_ = pWME;
    cond->bt.level = pWME->id->id->level;
    cond->test_for_acceptable_preference = pWME->acceptable;
    if (addBTPref && pWME->preference) cond->bt.trace = pWME->preference;

    /* Add identity information */
    if (thisAgent->explanationBasedChunker->ebc_settings[SETTING_EBC_LEARNING_ON])
    {
        thisAgent->explanationBasedChunker->add_inst_identity_to_test(cond->data.tests.id_test);
        if (cond->data.tests.attr_test->data.referent->is_sti())
            thisAgent->explanationBasedChunker->add_inst_identity_to_test(cond->data.tests.attr_test);
        if (cond->data.tests.value_test->data.referent->is_sti())
            thisAgent->explanationBasedChunker->add_inst_identity_to_test(cond->data.tests.value_test);
    }
    /* Set up links*/
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
    prev_cond = cond;

}

void add_pref_to_arch_inst(agent* thisAgent, instantiation* inst, Symbol* pID, Symbol* pAttr, Symbol* pValue)
{
    preference* pref;

    pref = make_preference(thisAgent, ACCEPTABLE_PREFERENCE_TYPE, pID, pAttr, pValue);
    thisAgent->symbolManager->symbol_add_ref(pref->id);
    thisAgent->symbolManager->symbol_add_ref(pref->attr);
    thisAgent->symbolManager->symbol_add_ref(pref->value);
    if (thisAgent->explanationBasedChunker->ebc_settings[SETTING_EBC_LEARNING_ON])
    {
        pref->inst_identities.id = thisAgent->explanationBasedChunker->get_or_create_inst_identity_for_sym(pref->id);
        if (pref->attr->is_sti())
            pref->inst_identities.attr = thisAgent->explanationBasedChunker->get_or_create_inst_identity_for_sym(pref->attr);
        if (pref->value->is_sti())
            pref->inst_identities.value = thisAgent->explanationBasedChunker->get_or_create_inst_identity_for_sym(pref->value);
    }
    add_pref_to_inst(thisAgent, pref, inst);
}

void add_deep_copy_prefs_to_inst(agent* thisAgent, preference* pref, instantiation* inst)
{

    deep_copy_wme* lNewDC_WME;
    goal_stack_level glbDeepCopyWMELevel = 0;
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
        lPref = make_preference(thisAgent, ACCEPTABLE_PREFERENCE_TYPE, lNewDC_WME->id, lNewDC_WME->attr, lNewDC_WME->value);

        if (thisAgent->explanationBasedChunker->ebc_settings[SETTING_EBC_LEARNING_ON])
        {
            /* We set the identities of the preferences so that they are dependent on the explanation behind
             * the working memory elements that they were copied from */
            lPref->inst_identities.id = thisAgent->explanationBasedChunker->get_or_create_inst_identity_for_sym(lNewDC_WME->deep_copied_wme->id);
            lPref->inst_identities.attr = thisAgent->explanationBasedChunker->get_or_create_inst_identity_for_sym(lNewDC_WME->deep_copied_wme->attr);
            lPref->inst_identities.value = thisAgent->explanationBasedChunker->get_or_create_inst_identity_for_sym(lNewDC_WME->deep_copied_wme->value);
        }

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
    inst->match_goal = NULL;
    inst->match_goal_level = 0;

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

    if (prod)
    {
        inst->prod_name = prod->name;
        inst->prod_naming_depth = prod->naming_depth;
    } else {
        inst->prod_name = backup_name;
        inst->prod_naming_depth = 0;
    }

    if (inst->prod_name)
    {
        thisAgent->symbolManager->symbol_add_ref(inst->prod_name);
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

    init_instantiation(thisAgent, inst, thisAgent->symbolManager->soarSymbols.architecture_inst_symbol, prod, tok, w);
    inst->next = thisAgent->newly_created_instantiations;
    thisAgent->newly_created_instantiations = inst;
    inst->in_newly_created = true;
    inst->in_ms = true;
    inst->match_goal_level = thisAgent->active_level;
    inst->match_goal = thisAgent->active_goal;


    if (thisAgent->trace_settings[TRACE_ASSERTIONS_SYSPARAM])
    {
        std::string lStr;
        thisAgent->outputManager->sprinta_sf(thisAgent, lStr, "\nNew match of %y in state %y (level %d).  Creating instantiation.\n", inst->prod_name, inst->match_goal, static_cast<int64_t>(inst->match_goal_level));
        thisAgent->outputManager->printa(thisAgent, lStr.c_str());
        xml_generate_verbose(thisAgent, lStr.c_str());
    }

    thisAgent->production_being_fired = inst->prod;
    prod->firing_count++;
    thisAgent->production_firing_count++;

    ExplainTraceType ebcTraceType = WM_Trace;
    if (prod->type == TEMPLATE_PRODUCTION_TYPE)
    {
        ebcTraceType = WM_Trace_w_Inequalities;
    } else if ((inst->match_goal_level > TOP_GOAL_LEVEL) && thisAgent->explanationBasedChunker->ebc_settings[SETTING_EBC_LEARNING_ON])
    {
        ebcTraceType = Explanation_Trace;
    }

    /* build the instantiated conditions, and bind LHS variables */
    p_node_to_conditions_and_rhs(thisAgent, prod->p_node, tok, w, &(inst->top_of_instantiated_conditions), &(inst->bottom_of_instantiated_conditions), (ebcTraceType != WM_Trace) ? &(rhs_vars) : NULL , ebcTraceType);

    /* record the level of each of the wmes that was positively tested */
    for (cond = inst->top_of_instantiated_conditions; cond != NIL; cond = cond->next)
        if (cond->type == POSITIVE_CONDITION)
        {
            cond->bt.level = cond->bt.wme_->id->id->level;
            cond->bt.trace = cond->bt.wme_->preference;  // These are later changed to the correct clone for the level
        }

    bool isSubGoalMatch = (inst->match_goal_level > TOP_GOAL_LEVEL);
    if (!isSubGoalMatch && (prod->type != TEMPLATE_PRODUCTION_TYPE))
    {
        /* We don't need identity information or the original vars on the top level, so we clean up now */
        thisAgent->explanationBasedChunker->clear_symbol_identity_map();
        deallocate_action_list(thisAgent, rhs_vars);
        rhs_vars = NULL;
    }

    /* Print rule firing trace info if enabled*/
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

    /* Print rule firing trace info's arrow*/
    if (trace_it && (thisAgent->trace_settings[TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM] != NONE_WME_TRACE))
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
            if (a2 && isSubGoalMatch)
            {
                pref = execute_action(thisAgent, a, tok, w, a2);
            } else {
                pref = execute_action(thisAgent, a, tok, w, NULL);
            }
        }
        else
        {
            pref = NIL;
            rl_build_template_instantiation(thisAgent, inst, tok, w, a2);
        }
        if (pref)
        {
            add_pref_to_inst(thisAgent, pref, inst);
            if (!thisAgent->WM->glbDeepCopyWMEs.empty())
            {
                inst->creates_deep_copy = true;
                if (isSubGoalMatch && thisAgent->explanationBasedChunker->ebc_settings[SETTING_EBC_LEARNING_ON])
                {
                    thisAgent->explanationBasedChunker->force_add_inst_identity(pref->value, pref->inst_identities.value);
                    thisAgent->explanationBasedChunker->force_add_inst_identity(thisAgent->explanationBasedChunker->deep_copy_sym_expanded, pref->inst_identities.value);
                }
                thisAgent->explanationBasedChunker->deep_copy_sym_expanded = NULL;
                add_deep_copy_prefs_to_inst(thisAgent, pref, inst);
            }
        }
        if (a2)  a2 = a2->next;
    }

    /* reset rhs_variable_bindings array to all zeros */
    index = 0;
    cell = thisAgent->rhs_variable_bindings;
    while (index++ <= thisAgent->firer_highest_rhs_unboundvar_index) *(cell++) = NIL;

    /* fill in lots of other stuff */
    finalize_instantiation(thisAgent, inst, false, NIL, true);

    /* Print rule firing trace info RHS (must be after fill_in_new_instantiation) */
    if (trace_it && (thisAgent->trace_settings[TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM] != NONE_WME_TRACE))
    {
        for (pref = inst->preferences_generated; pref != NIL; pref = pref->inst_next)
        {
            thisAgent->outputManager->printa_sf(thisAgent,  "%e ");
            print_preference(thisAgent, pref, true);
        }
    }

    thisAgent->production_being_fired = NIL;
    if (rhs_vars) deallocate_action_list(thisAgent, rhs_vars);

    if (isSubGoalMatch && thisAgent->explanationBasedChunker->ebc_settings[SETTING_EBC_LEARNING_ON])
    {
        thisAgent->explanationBasedChunker->clear_id_to_identity_map();
    }

    if (isSubGoalMatch) thisAgent->explanationBasedChunker->clear_symbol_identity_map();

    if (isSubGoalMatch)
    {
        /* Copy any operator selection knowledge preferences for conditions of this instantiation */
        thisAgent->explanationBasedChunker->copy_OSK(inst);

        /* build chunks/justifications if necessary */
        thisAgent->explanationBasedChunker->set_learning_for_instantiation(inst);
        thisAgent->explanationBasedChunker->learn_rule_from_instance(inst, &(thisAgent->newly_created_instantiations));
    }

    if (!thisAgent->system_halted) soar_invoke_callbacks(thisAgent, FIRING_CALLBACK, static_cast<soar_call_data>(inst));
}

/* -----------------------------------------------------------------------
 Deallocate Instantiation

 This deallocates the given instantiation.  This should only be invoked
 from possibly_deallocate_instantiation().
 ----------------------------------------------------------------------- */

void deallocate_instantiation(agent* thisAgent, instantiation*& inst)
{
    if (inst->in_newly_created)
    {
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
    char* lProdName;

    l_instantiation_list.push_back(inst);
    inst_list::iterator next_iter = l_instantiation_list.begin();

    while (next_iter != l_instantiation_list.end())
    {
        lInst = *next_iter;
        ++next_iter;
        lProdName = lInst->prod_name ? lInst->prod_name->sc->name : NULL;

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

                #ifndef DO_TOP_LEVEL_COND_REF_CTS
                if (lInst->match_goal_level > TOP_GOAL_LEVEL)
                #endif
                {
                    wme_remove_ref(thisAgent, cond->bt.wme_);
                }

                if (cond->bt.trace)
                {
                    preference* lPref = cond->bt.trace;
                    /* -----------------------
                     * preference_remove_ref()
                     * ----------------------- */
                    #ifndef DO_TOP_LEVEL_COND_REF_CTS
                    if (lInst->match_goal_level > TOP_GOAL_LEVEL)
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

                            for (clone = lPref->next_clone; clone != NIL; clone = clone->next_clone)
                                if (clone->reference_count) has_active_clones = true;
                            if (has_active_clones) continue;
                            for (clone = lPref->prev_clone; clone != NIL; clone = clone->prev_clone)
                                if (clone->reference_count) has_active_clones = true;
                            if (has_active_clones)
                            {
                                continue;
                            }
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
                            if (lPref->in_tm)
                            {
                                remove_preference_from_tm(thisAgent, lPref);
                            }
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
    }

    while (!cond_stack.empty())
    {
        condition* temp = cond_stack.back();
        cond_stack.pop_back();

        deallocate_preference_contents(thisAgent, temp->bt.trace, true);
    }

    // free instantiations in the reverse order

    for (inst_list::reverse_iterator riter = l_instantiation_list.rbegin(); riter != l_instantiation_list.rend(); ++riter)
    {
        instantiation* lDelInst = *riter;
        lProdName = lDelInst->prod_name ? lDelInst->prod_name->sc->name : NULL;

        deallocate_condition_list(thisAgent, lDelInst->top_of_instantiated_conditions);

        /* Clean up operator selection knowledge */
        if (lDelInst->OSK_prefs)
        {
            clear_preference_list(thisAgent, lDelInst->OSK_prefs);
        }
        if (lDelInst->OSK_proposal_prefs)
        {
            /* These prefs did not have their refcounts increased, so we don't want to call clear_preference_list */
            free_list(thisAgent, lDelInst->OSK_proposal_prefs);
            lDelInst->OSK_proposal_prefs = NULL;
        }
        if (lDelInst->OSK_proposal_slot)
        {
            lDelInst->OSK_proposal_slot->instantiation_with_temp_OSK = NULL;
        }

        /* Clean up preferences cached for possible explanations */
        preference* next_pref;
        while (lDelInst->preferences_cached)
        {
            next_pref = lDelInst->preferences_cached->inst_next;
            deallocate_preference(thisAgent, lDelInst->preferences_cached, true);
            lDelInst->preferences_cached = next_pref;
        }

        thisAgent->symbolManager->symbol_remove_ref(&lDelInst->prod_name);

        if (lDelInst->prod)
        {
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

        thisAgent->memoryManager->free_with_pool(MP_instantiation, lDelInst);
    }

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
    soar_invoke_callbacks(thisAgent, RETRACTION_CALLBACK, static_cast<soar_call_data>(inst));

    retracted_a_preference = false;

    trace_it = trace_firings_of_inst(thisAgent, inst);

    /* retract any preferences that are in TM and aren't o-supported */
    pref = inst->preferences_generated;

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
                    if (thisAgent->trace_settings[TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM] != NONE_WME_TRACE)
                    {
                        thisAgent->outputManager->printa(thisAgent,  " -->\n");
                    }
                    xml_object(thisAgent, kTagActionSideMarker);
                }
            }
            if (trace_it && (thisAgent->trace_settings[TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM] != NONE_WME_TRACE))
            {
                    thisAgent->outputManager->printa_sf(thisAgent,  "%e ");
                    print_preference(thisAgent, pref, true);
            }
            remove_preference_from_tm(thisAgent, pref);
            retracted_a_preference = true;
        }
        pref = next;
    }
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
    possibly_deallocate_instantiation(thisAgent, inst);
}

instantiation* make_architectural_instantiation_for_memory_system(agent* thisAgent, Symbol* pState, wme_set* pConds, symbol_triple_list* pActions, bool forSMem)
{
    instantiation* inst;

    init_instantiation(thisAgent, inst, thisAgent->symbolManager->soarSymbols.fake_instantiation_symbol);
    inst->match_goal = pState;
    inst->match_goal_level = pState->id->level;
    inst->tested_LTM = true;

    // create LHS
    {
        condition* prev_cond = NULL;

        if (forSMem)
        {
            add_cond_to_arch_inst(thisAgent, prev_cond, inst, pState->id->smem_info->smem_link_wme, false);
            add_cond_to_arch_inst(thisAgent, prev_cond, inst, pState->id->smem_info->cmd_wme, false);
            add_cond_to_arch_inst(thisAgent, prev_cond, inst, pState->id->smem_info->result_wme, false);
        } else
        {
            add_cond_to_arch_inst(thisAgent, prev_cond, inst, pState->id->epmem_info->epmem_link_wme, false);
            add_cond_to_arch_inst(thisAgent, prev_cond, inst, pState->id->epmem_info->cmd_wme, false);
            add_cond_to_arch_inst(thisAgent, prev_cond, inst, pState->id->epmem_info->result_wme, false);
        }
        for (wme_set::iterator c_it = pConds->begin(); c_it != pConds->end(); c_it++)
        {
            add_cond_to_arch_inst(thisAgent, prev_cond, inst, (*c_it));
        }
    }

    // create RHS
    {
        for (symbol_triple_list::iterator a_it = pActions->begin(); a_it != pActions->end(); a_it++)
        {
            add_pref_to_arch_inst(thisAgent, inst, (*a_it)->id, (*a_it)->attr, (*a_it)->value);
        }
    }

    /* Clean up symbol to identity mappings for this instantiation*/
    thisAgent->explanationBasedChunker->clear_symbol_identity_map();

    /* Initialize levels, add refcounts to prefs/wmes, sets on_goal_list flag, o-support calculation if needed */
    finalize_instantiation(thisAgent, inst, false, NULL, false);

    return inst;
}

/* ------------------------------------------------------------------
            make_architectural_instantiation_for_impasse_item

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

    init_instantiation(thisAgent, inst, thisAgent->symbolManager->soarSymbols.fake_instantiation_symbol);

    /* We already know the match goal, so we can just set it */
    inst->match_goal = goal;
    inst->match_goal_level = goal->id->level;

    // Create the LHS
    add_cond_to_arch_inst(thisAgent, prev_cond, inst, ap_wme, false);
    add_cond_to_arch_inst(thisAgent, prev_cond, inst, ss_link_wme, false);

    // Create the RHS
    add_pref_to_arch_inst(thisAgent, inst, goal, thisAgent->symbolManager->soarSymbols.item_symbol, cand->value);
    inst->preferences_generated->o_supported = false;
    preference_add_ref(inst->preferences_generated);

    /* Initialize levels, add refcounts to prefs/wmes, sets on_goal_list flag, o-support calculation if needed */
    finalize_instantiation(thisAgent, inst, false, NULL, true);

    /* Clean up symbol to identity mappings for this instantiation*/
    thisAgent->explanationBasedChunker->clear_symbol_identity_map();

    return inst->preferences_generated;

}
