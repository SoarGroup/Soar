/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
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
#include "production_reorder.h"

#include "agent.h"
#include "condition.h"
#include "ebc_repair.h"
#include "explanation_memory.h"
#include "mem.h"
#include "output_manager.h"
#include "preference.h"
#include "print.h"
#include "production.h"
#include "rhs.h"
#include "run_soar.h"
#include "soar_TraceNames.h"
#include "symbol.h"
#include "symbol_manager.h"
#include "test.h"
#include "xml.h"

#include <stdlib.h>

/* *********************************************************************

                             Reordering

********************************************************************* */

/* =====================================================================

                  Name of production being reordered

   In case any errors are encountered during reordering, this variable
   holds the name of the production currently being reordered, so it
   can be printed with the error message.
===================================================================== */

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

  reorder_action_list() does the reordering.  Its parameter action_list
  is reordered in place (destructively modified).  It also requires at entry
  that the variables bound on the LHS are marked.  The function returns
  a ProdReorderFailureType indicating the issue if a legal ordering
  could not be produced.
===================================================================== */

bool legal_to_execute_action(action* a, tc_number tc);

bool isNewUngroundedElement(matched_symbol_list* ungrounded_syms, Symbol* pSym, uint64_t pInstIdentity)
{
    for (auto it = ungrounded_syms->begin(); it != ungrounded_syms->end(); it++)
    {
        if (((*it)->instantiated_sym == pSym) && ((*it)->inst_identity == pInstIdentity))
            return false;
    }
    return true;

}
ProdReorderFailureType reorder_action_list(agent* thisAgent, action** action_list,
                         tc_number lhs_tc, matched_symbol_list* ungrounded_syms,
                         bool add_ungrounded)
{
    cons* new_bound_vars;
    action* remaining_actions;
    action* first_action, *last_action;
    action* a, *prev_a;
    ProdReorderFailureType failure_type;

    new_bound_vars = NIL;
    remaining_actions = *action_list;
    first_action = NIL;
    last_action = NIL;

    while (remaining_actions)
    {
        /* --- scan through remaining_actions, look for one that's legal --- */
        prev_a = NIL;
        a = remaining_actions;
        while (true)
        {
            if (!a)
            {
                break;    /* looked at all candidates, but none were legal */
            }
            if (legal_to_execute_action(a, lhs_tc))
            {
                break;
            }
            prev_a = a;
            a = a->next;
        }
        if (!a)
        {
            break;
        }
        /* --- move action a from remaining_actions to reordered list --- */
        if (prev_a)
        {
            prev_a->next = a->next;
        }
        else
        {
            remaining_actions = a->next;
        }
        a->next = NIL;
        if (last_action)
        {
            last_action->next = a;
        }
        else
        {
            first_action = a;
        }
        last_action = a;
        /* --- add new variables from a to new_bound_vars --- */
        add_all_variables_in_action(thisAgent, a, lhs_tc, &new_bound_vars);
    }

    if (remaining_actions)
    {   /* --- there are remaining_actions but none can be legally added --- */

        std::string unSymString("");
        action* lAction;
        Symbol* lSym;
        thisAgent->outputManager->set_print_indents("        ");
        for (lAction = remaining_actions; lAction; lAction = lAction->next)
        {
            thisAgent->outputManager->sprinta_sf(thisAgent, unSymString, "%a\n", lAction);
            if (add_ungrounded && lAction->id && rhs_value_is_symbol(lAction->id) && !rhs_value_to_was_unbound_var(lAction->id))
            {
                lSym = rhs_value_to_symbol(lAction->id);
                Symbol* lVarSym, *lInstSym;
                uint64_t lNewID;

                lVarSym = lSym;
                if (lSym->is_sti())
                {
                    lInstSym = lSym;
                } else {
                    lInstSym = lSym->var->instantiated_sym;
                }
                lNewID = rhs_value_to_inst_identity(lAction->id);
                if (isNewUngroundedElement(ungrounded_syms, lInstSym,  lNewID))
                {
                    chunk_element* lNewUngroundedSym;
                    thisAgent->memoryManager->allocate_with_pool(MP_chunk_element, &lNewUngroundedSym);
                    lNewUngroundedSym->variable_sym = lVarSym;
                    lNewUngroundedSym->instantiated_sym = lInstSym;
                    lNewUngroundedSym->inst_identity = lNewID;
                    ungrounded_syms->push_back(lNewUngroundedSym);
                }
            }
        }
        failure_type = reorder_failed_reordering_rhs;
        thisAgent->outputManager->set_print_indents("");
        thisAgent->outputManager->display_reorder_error(thisAgent, reorder_failed_reordering_rhs, thisAgent->name_of_production_being_reordered, unSymString.c_str());
        /* --- reconstruct list of all actions --- */
        if (last_action)
        {
            last_action->next = remaining_actions;
        }
        else
        {
            first_action = remaining_actions;
        }
    }
    else
    {
        failure_type = reorder_success;
    }

    /* --- unmark variables that we just marked --- */
    unmark_variables_and_free_list(thisAgent, new_bound_vars);

    /* --- return final result --- */
    *action_list = first_action;
    return failure_type;
}

bool all_vars_in_rhs_value_bound(rhs_value rv, tc_number tc)
{
    cons* c;
    cons* fl;
    Symbol* sym;

    if (rhs_value_is_funcall(rv))
    {
        /* --- function calls --- */
        fl = rhs_value_to_funcall_list(rv);
        for (c = fl->rest; c != NIL; c = c->rest)
            if (! all_vars_in_rhs_value_bound(static_cast<char*>(c->first), tc))
            {
                return false;
            }
        return true;
    }
    else
    {
        /* --- ordinary (symbol) rhs values --- */
        sym = rhs_value_to_symbol(rv);
        if (sym->is_variable())
        {
            return (sym->tc_num == tc);
        }
        return true;
    }
}

bool legal_to_execute_action(action* a, tc_number tc)
{
    if (a->type == MAKE_ACTION)
    {
        if (! all_vars_in_rhs_value_bound(a->id, tc))
        {
            return false;
        }
        if (rhs_value_is_funcall(a->attr) && !all_vars_in_rhs_value_bound(a->attr, tc))
        {
            return false;
        }
        if (rhs_value_is_funcall(a->value) && !all_vars_in_rhs_value_bound(a->value, tc))
        {
            return false;
        }
        if (preference_is_binary(a->preference_type) &&
            rhs_value_is_funcall(a->referent) && !all_vars_in_rhs_value_bound(a->referent, tc))
        {
            return false;
        }
        return true;
    }
    /* --- otherwise it's a function call; make sure args are all bound  --- */
    return all_vars_in_rhs_value_bound(a->value, tc);
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

saved_test* simplify_test(agent* thisAgent, test* t, saved_test* old_sts)
{
    test ct, New, subtest;
    saved_test* saved;
    Symbol* var, *sym;
    cons* c, *prev_c, *next_c;
    uint64_t sym_identity = LITERAL_VALUE;
    test sym_identity_set_test;

    if (!(*t))
    {
        add_gensymmed_equality_test(thisAgent, t, 'd');
        return old_sts;
    }

    ct = *t;

    switch (ct->type)
    {
        case EQUALITY_TEST:
            return old_sts;
            break;
        case CONJUNCTIVE_TEST:
            /* -- set sym to symbol found in an last equality test in a conjunction list
             *
             *    Note: Could be a problem because we have more than one equality sym in
             *           a conjunction list. Which one do we want to index on? It's storing
             *           all the non-equality tests based on that symbol.  Couldn't this have
             *           occurred before with { <var> <var2> } where they match different
             *           symbols? --- */
            sym = ct->eq_test->data.referent;
            sym_identity = ct->eq_test->inst_identity;
            sym_identity_set_test = ct->eq_test;
            /* --- if no equality test was found, generate a dummy variable for it --- */
            if (!sym)
            {
                add_gensymmed_equality_test(thisAgent, &New, 'd');
                c->first = New;
                c->rest = ct->data.conjunct_list;
                ct->data.conjunct_list = c;
            }
            /* -- moves all tests except equality in this conjunction list to the saved test
             *    list passed in.
             *    - Use sym determined above to index the test
             *      - Does add refcount for sym that it uses for indexing.
             *      - Must make sure saved->var is getting cleaned up.
             *    - Destroys the cons list in conjunct_list but doesn't deallocate tests
             *      because it isn't copying the test, just "moving" it to the saved_test
             *      struct
             *
             *      Note: Could be a problem because an equality test with another symbol
             *            won't be saved anywhere.  It also won't be deallocated.-- */
            prev_c = NIL;
            c = ct->data.conjunct_list;
            while (c)
            {
                next_c = c->rest;
                subtest = static_cast<test>(c->first);
                if ((subtest->type != EQUALITY_TEST) && (subtest->type != SMEM_LINK_UNARY_TEST) && (subtest->type != SMEM_LINK_UNARY_NOT_TEST))
                {
                    /* -- create saved_test, splice this cons out of conjunct_list -- */
                    thisAgent->memoryManager->allocate_with_pool(MP_saved_test, &saved);
                    saved->next = old_sts;
                    old_sts = saved;
                    saved->var = sym;
                    thisAgent->symbolManager->symbol_add_ref(sym);
                    saved->inst_identity = sym_identity;
                    if (sym_identity_set_test->identity)
                        saved->identity = sym_identity_set_test->identity;
                    else
                        saved->identity = NULL;
                    saved->the_test = subtest;
                    if (prev_c)
                    {
                        prev_c->rest = next_c;
                    }
                    else
                    {
                        ct->data.conjunct_list = next_c;
                    }
                    free_cons(thisAgent, c);
                }
                else
                {
                    prev_c = c;
                }
                c = next_c;
            }
            /* Check if the conjunction is really just a single test after simplification */
            if (ct->data.conjunct_list->rest == NULL)
            {
                test tempTest = static_cast<test>(ct->data.conjunct_list->first);
                free_cons(thisAgent, ct->data.conjunct_list);
                ct->data.conjunct_list = NULL;
                /* Switch type to a goal_id test, so that deallocate won't try to deallocate anything extra */
                ct->type = GOAL_ID_TEST;
                deallocate_test(thisAgent, ct);
                *t = tempTest;
            }
            break;

        default:
            /* -- for goal/impasse, disjunction, and non-equality relational tests,
             *    add a dummy variable and use that as an index to a new entry in the
             *    saved_test list passed in.  Full test with original referent still
             *    saved.
             *    - Must make sure dummy variable also gets cleaned up-- */
            var = thisAgent->symbolManager->generate_new_variable("dummy-");
            New = make_test(thisAgent, var, EQUALITY_TEST);
            thisAgent->symbolManager->symbol_remove_ref(&var);
            thisAgent->memoryManager->allocate_with_pool(MP_saved_test, &saved);
            saved->next = old_sts;
            old_sts = saved;
            saved->var = var;
            saved->inst_identity = LITERAL_VALUE;
            saved->identity = NULL;
            saved->the_test = *t;
            *t = New;
            break;
    }
    return old_sts;
}

saved_test* simplify_condition_list(agent* thisAgent, condition* conds_list)
{
    condition* c;
    saved_test* sts;

    sts = NIL;

    for (c = conds_list; c != NIL; c = c->next)
    {
//#define CONSIDER_NEGATIVE 1
#ifdef CONSIDER_NEGATIVE
        if (c->type != CONJUNCTIVE_NEGATION_CONDITION)
        {
#else
        if (c->type == POSITIVE_CONDITION)
        {
#endif
            sts = simplify_test(thisAgent, &(c->data.tests.id_test), sts);
            sts = simplify_test(thisAgent, &(c->data.tests.attr_test), sts);
            sts = simplify_test(thisAgent, &(c->data.tests.value_test), sts);
        }
    }
    return sts;
}

TestType reverse_direction_of_relational_test(agent* thisAgent, byte type)
{
    switch (type)
    {
        case NOT_EQUAL_TEST:
        case SAME_TYPE_TEST:
        case SMEM_LINK_TEST:
        case SMEM_LINK_NOT_TEST:
            return static_cast<TestType>(type);
        case LESS_TEST:
            return GREATER_TEST;
        case GREATER_TEST:
            return LESS_TEST;
        case LESS_OR_EQUAL_TEST:
            return GREATER_OR_EQUAL_TEST;
        case GREATER_OR_EQUAL_TEST:
            return LESS_OR_EQUAL_TEST;
        default:
        {
            char msg[BUFFER_MSG_SIZE];
            strncpy(msg,
                    "Internal error: arg to reverse_direction_of_relational_test\n", BUFFER_MSG_SIZE);
            msg[BUFFER_MSG_SIZE - 1] = 0; /* ensure null termination */
            abort_with_fatal_error(thisAgent, msg);
            break;
        }
    }
    return NOT_EQUAL_TEST; /* unreachable, but without it, gcc -Wall warns here */
}

saved_test* restore_saved_tests_to_test(agent* thisAgent,
                                        test* t,
                                        bool is_id_field,
                                        tc_number bound_vars_tc_number,
                                        saved_test* tests_to_restore, bool neg)
{
    saved_test* st, *prev_st, *next_st;
    bool added_it;
    Symbol* referent;

    prev_st = NIL;
    st = tests_to_restore;
    while (st)
    {
        next_st = st->next;
        added_it = false;


        switch (st->the_test->type)
        {
            case GOAL_ID_TEST:
            case IMPASSE_ID_TEST:
                if (! is_id_field)
                {
                    /* goal/impasse tests only go in id fields */
                    break;
                }
            /* ... otherwise fall through to the next case below ... */
            case DISJUNCTION_TEST:
                if ((*t)->eq_test->data.referent == st->var)
                {
                    add_test_if_not_already_there(thisAgent, t, st->the_test, neg, true);
                    added_it = true;
                }
                break;
            default:  /* --- st->test is a relational test --- */
                referent = st->the_test->data.referent;
                if ((*t)->eq_test->data.referent == st->var)
                {
                    if (referent->is_constant_or_marked_variable(bound_vars_tc_number) ||
                            (st->var == referent))
                    {
                        add_test_if_not_already_there(thisAgent, t, st->the_test, neg);
                        added_it = true;
                    }
                }
                else if ((*t)->eq_test->data.referent == referent)
                {
                    if (st->var->is_constant_or_marked_variable(bound_vars_tc_number) || (st->var == referent))
                    {
                        st->the_test->type = reverse_direction_of_relational_test(thisAgent, st->the_test->type);
                        st->the_test->data.referent = st->var;
                        st->the_test->inst_identity = st->inst_identity;
                        st->the_test->identity = st->identity;
                        st->var = referent;
                        add_test_if_not_already_there(thisAgent, t, st->the_test, neg);
                        added_it = true;
                    }
                }
                break;
        } /* end of switch statement */
        if (added_it)
        {
            if (prev_st)
            {
                prev_st->next = next_st;
            }
            else
            {
                tests_to_restore = next_st;
            }
            thisAgent->symbolManager->symbol_remove_ref(&st->var);
            thisAgent->memoryManager->free_with_pool(MP_saved_test, st);
        }
        else
        {
            prev_st = st;
        }
        st = next_st;
    } /* end of while (st) */
    return tests_to_restore;
}

void restore_and_deallocate_saved_tests(agent* thisAgent,
                                        condition* conds_list,
                                        tc_number tc, /* tc number for vars bound outside */
                                        saved_test* tests_to_restore)
{
    condition* cond;
    cons* new_vars;

        new_vars = NIL;
    for (cond = conds_list; cond != NIL; cond = cond->next)
    {
        #ifdef CONSIDER_NEGATIVE
        if (cond->type == CONJUNCTIVE_NEGATION_CONDITION) continue;
        #else
        if (cond->type != POSITIVE_CONDITION) continue;
        #endif

        bool neg = cond->type == NEGATIVE_CONDITION;

        tests_to_restore = restore_saved_tests_to_test(thisAgent, (&cond->data.tests.id_test), true, tc, tests_to_restore, neg);
        add_bound_variables_in_test(thisAgent, cond->data.tests.id_test, tc, &new_vars);
        tests_to_restore = restore_saved_tests_to_test(thisAgent, (&cond->data.tests.attr_test), false, tc, tests_to_restore, neg);
        add_bound_variables_in_test(thisAgent, cond->data.tests.attr_test, tc, &new_vars);
        tests_to_restore = restore_saved_tests_to_test(thisAgent, (&cond->data.tests.value_test), false, tc, tests_to_restore, neg);
        add_bound_variables_in_test(thisAgent, cond->data.tests.value_test, tc, &new_vars);
    }

    if (tests_to_restore)
    {
        saved_test* next_st;

        while (tests_to_restore)
        {
            next_st = tests_to_restore->next;

            if (thisAgent->trace_settings[TRACE_CHUNKS_WARNINGS_SYSPARAM])
            {
                thisAgent->outputManager->printa_sf(thisAgent,  "\nWarning:  Ignoring test %t whose referent %y is unbound in production %s\n", tests_to_restore->the_test, tests_to_restore->var, thisAgent->name_of_production_being_reordered);
                // XML generation
                growable_string gs = make_blank_growable_string(thisAgent);
                add_to_growable_string(thisAgent, &gs, "Warning:  Ignoring test(s) whose referent is unbound in production  ");
                add_to_growable_string(thisAgent, &gs, thisAgent->name_of_production_being_reordered);
                xml_generate_warning(thisAgent, text_of_growable_string(gs));

                free_growable_string(thisAgent, gs);
            }
            thisAgent->symbolManager->symbol_remove_ref(&tests_to_restore->var);
            deallocate_test(thisAgent, tests_to_restore->the_test);
            thisAgent->memoryManager->free_with_pool(MP_saved_test, tests_to_restore);
            tests_to_restore = next_st;
        }
    }
    unmark_variables_and_free_list(thisAgent, new_vars);
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

cons* collect_vars_tested_by_test_that_are_bound(agent* thisAgent, test t,
        tc_number tc,
        cons* starting_list)
{
    cons* c;
    Symbol* referent;

    if (!t)
    {
        return starting_list;
    }

    switch (t->type)
    {
        case GOAL_ID_TEST:
        case IMPASSE_ID_TEST:
        case DISJUNCTION_TEST:
        case SMEM_LINK_UNARY_TEST:
        case SMEM_LINK_UNARY_NOT_TEST:
            break;
        case CONJUNCTIVE_TEST:
            for (c = t->data.conjunct_list; c != NIL; c = c->rest)
                starting_list = collect_vars_tested_by_test_that_are_bound
                                (thisAgent, static_cast<test>(c->first), tc, starting_list);
            break;
        default:
            /* --- relational tests and equality --- */
            referent = t->data.referent;
            if (referent->symbol_type == VARIABLE_SYMBOL_TYPE)
                if (referent->tc_num == tc)
                {
                    starting_list = add_if_not_member(thisAgent, referent, starting_list);
                }
            break;
    }
    return starting_list;
}

cons* collect_vars_tested_by_cond_that_are_bound(agent* thisAgent,
        condition* cond,
        tc_number tc,
        cons* starting_list)
{
    condition* c;

    if (cond->type == CONJUNCTIVE_NEGATION_CONDITION)
    {
        /* --- conjuctive negations --- */
        for (c = cond->data.ncc.top; c != NIL; c = c->next)
            starting_list = collect_vars_tested_by_cond_that_are_bound
                            (thisAgent, c, tc, starting_list);
    }
    else
    {
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

void fill_in_vars_requiring_bindings(agent* thisAgent, condition* cond_list, tc_number tc)
{
    cons* new_bound_vars;
    condition* c;

    /* --- add anything bound in a positive condition at this level --- */
    new_bound_vars = NIL;
    for (c = cond_list; c != NIL; c = c->next)
        if (c->type == POSITIVE_CONDITION)
        {
            add_bound_variables_in_condition(thisAgent, c, tc, &new_bound_vars);
        }

    /* --- scan through negated and NC cond's, fill in stuff --- */
    for (c = cond_list; c != NIL; c = c->next)
    {
        if (c->type != POSITIVE_CONDITION)
        {
            c->reorder.vars_requiring_bindings =  collect_vars_tested_by_cond_that_are_bound(thisAgent, c, tc, NIL);
        }
        if (c->type == CONJUNCTIVE_NEGATION_CONDITION)
        {
            fill_in_vars_requiring_bindings(thisAgent, c->data.ncc.top, tc);
        }
    }

    unmark_variables_and_free_list(thisAgent, new_bound_vars);
}

void remove_vars_requiring_bindings(agent* thisAgent,
                                    condition* cond_list)
{
    condition* c;

    /* --- scan through negated and NC cond's, remove lists from them --- */
    for (c = cond_list; c != NIL; c = c->next)
    {
        if (c->type != POSITIVE_CONDITION)
        {
            free_list(thisAgent, c->reorder.vars_requiring_bindings);
        }
        if (c->type == CONJUNCTIVE_NEGATION_CONDITION)
        {
            remove_vars_requiring_bindings(thisAgent, c->data.ncc.top);
        }
    }
}

/* =====================================================================

             Finding the Root Variables in a Condition List

   This routine finds the root variables in a given condition list.
   The caller should setup the current tc to be the set of variables
   bound outside the given condition list.  (This should normally be
   an empty TC, except when the condition list is the subconditions
   of an NCC.)
===================================================================== */

cons* collect_root_variables(agent* thisAgent,
                             condition* cond_list,
                             tc_number tc, /* for vars bound outside */
                             matched_symbol_list* ungrounded_syms,
                             bool add_ungrounded)
{

    matched_symbol_list* new_vars_from_value_slot = new matched_symbol_list();
    matched_symbol_list* new_vars_from_id_slot = new matched_symbol_list();
    condition* cond;
    bool found_goal_impasse_test;

    /* The following find alls soar identifiers in the identifier element
     * that aren't in a value element */

    Symbol* lSym, *lMatchedSym;
    uint64_t l_inst_identity;

    /* --- find everthing that's in the value slot of some condition --- */
    for (cond = cond_list; cond != NIL; cond = cond->next)
    {
        if (cond->type == POSITIVE_CONDITION)
        {
            lMatchedSym = NULL;
            if (cond->data.tests.value_test->eq_test->data.referent->is_variable())
            {
                lMatchedSym = cond->data.tests.value_test->eq_test->data.referent->var->instantiated_sym;
            }
            if (!lMatchedSym) lMatchedSym = cond->data.tests.value_test->eq_test->data.referent;
            lSym = cond->data.tests.value_test->eq_test->data.referent;
            l_inst_identity = cond->data.tests.value_test->eq_test->inst_identity;
            add_bound_variable_with_identity(thisAgent, lSym, lMatchedSym, l_inst_identity, tc, new_vars_from_value_slot);
        }
    }
    /* --- now see what else we can add by throwing in the id slot --- */
    for (cond = cond_list; cond != NIL; cond = cond->next)
    {
        if (cond->type == POSITIVE_CONDITION)
        {
            if (cond->data.tests.id_test->eq_test->data.referent->is_variable())
            {
                lMatchedSym = cond->data.tests.id_test->eq_test->data.referent->var->instantiated_sym;
            }
            if (!lMatchedSym) lMatchedSym = cond->data.tests.id_test->eq_test->data.referent;

            lSym = cond->data.tests.id_test->eq_test->data.referent;
            l_inst_identity = cond->data.tests.id_test->eq_test->inst_identity;
            add_bound_variable_with_identity(thisAgent, lSym, lMatchedSym, l_inst_identity, tc, new_vars_from_id_slot);
        }
    }

    /* --- unmark everything we just marked --- */
    delete_ungrounded_symbol_list(thisAgent, &new_vars_from_value_slot);
    for (auto it = new_vars_from_id_slot->begin(); it != new_vars_from_id_slot->end(); it++)
    {
        (*it)->variable_sym->tc_num = 0;
    }

    /* --- make sure each root var has some condition with goal/impasse --- */
    std::string errorStr;

    for (auto it = new_vars_from_id_slot->begin(); it != new_vars_from_id_slot->end(); it++)
    {
        chunk_element* lOldMatchedSym = (*it);
        found_goal_impasse_test = false;

        for (cond = cond_list; cond != NIL; cond = cond->next)
        {
        if (cond->type != POSITIVE_CONDITION) continue;
        if ((cond->data.tests.id_test->eq_test->data.referent == lOldMatchedSym->variable_sym) &&
            test_includes_goal_or_impasse_id_test(cond->data.tests.id_test, true, true))
            {
                    found_goal_impasse_test = true;
                    break;
                }
        }
        if (! found_goal_impasse_test)
        {
            if (add_ungrounded && isNewUngroundedElement(ungrounded_syms,  lOldMatchedSym->instantiated_sym,  lOldMatchedSym->inst_identity))
            {
                chunk_element* lNewUngroundedSym;
                thisAgent->memoryManager->allocate_with_pool(MP_chunk_element, &lNewUngroundedSym);
                lNewUngroundedSym->variable_sym = lOldMatchedSym->variable_sym;
                lNewUngroundedSym->instantiated_sym = lOldMatchedSym->instantiated_sym;
                lNewUngroundedSym->inst_identity = lOldMatchedSym->inst_identity;
                ungrounded_syms->push_back(lNewUngroundedSym);
            } else {
                // TODO: we should reject the rule entirely, not just print a warning. sp {hello-world (<s> ^results <any>)-->}
                if (thisAgent->outputManager->settings[OM_WARNINGS])
                {
                    thisAgent->outputManager->sprinta_sf(thisAgent, errorStr, "\nWarning: On the LHS of production %s, identifier %y is not connected to any goal or impasse.\n",
                            thisAgent->name_of_production_being_reordered, lOldMatchedSym->variable_sym);
                    thisAgent->outputManager->printa(thisAgent, errorStr.c_str());
                    xml_generate_warning(thisAgent, errorStr.c_str());
                }
            }
        }
    }

    cons* returnList = NULL;
    for (auto it = new_vars_from_id_slot->begin(); it != new_vars_from_id_slot->end(); it++)
    {
        push(thisAgent, (*it)->variable_sym, returnList);
    }
    delete_ungrounded_symbol_list(thisAgent, &new_vars_from_id_slot);
    return returnList;
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
   Return true iff the given test is covered by the previously
   bound variables.  The set of previously bound variables is
   given by the current TC, PLUS any variables in the list
   "extra_vars."
------------------------------------------------------------- */

bool test_covered_by_bound_vars(test t, tc_number tc, cons* extra_vars)
{
    Symbol* referent = t->eq_test->data.referent;
    if (referent->is_constant_or_marked_variable(tc))   return true;
    if (extra_vars)                                     return member_of_list(referent, extra_vars);
    return false;
}

/* -------------------------------------------------------------
   Returns the user set value of the expected match cost of the
   multi-attribute, or 1 if the input symbol isn't in the user
   set list.
------------------------------------------------------------- */

int64_t get_cost_of_possible_multi_attribute(agent* thisAgent, Symbol* sym)
{
    multi_attribute* m = thisAgent->multi_attributes;
    while (m)
    {
        if (m->symbol == sym) return m->value;
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

int64_t cost_of_adding_condition(agent* thisAgent, condition* cond, tc_number tc, cons* root_vars_not_bound_yet)
{
    cons* c;
    int64_t result;

    /* --- handle the common simple case quickly up front --- */
    if ((! root_vars_not_bound_yet) &&
            (cond->type == POSITIVE_CONDITION) &&
            (cond->data.tests.id_test) &&
            (cond->data.tests.attr_test) &&
            (cond->data.tests.value_test) &&
            (cond->data.tests.id_test->type == EQUALITY_TEST) &&
            (cond->data.tests.attr_test->type == EQUALITY_TEST) &&
            (cond->data.tests.value_test->type == EQUALITY_TEST))
    {

        if (!(cond->data.tests.id_test->data.referent->is_constant_or_marked_variable(tc)))
        {
            return MAX_COST;
        }
        if ((cond->data.tests.attr_test->data.referent->is_constant_or_marked_variable(tc)))
            result = get_cost_of_possible_multi_attribute
                     (thisAgent, cond->data.tests.attr_test->data.referent);
        else
        {
            result =  BF_FOR_ATTRIBUTES;
        }

        if (!(cond->data.tests.value_test->data.referent->is_constant_or_marked_variable(tc)))
        {
            if (cond->test_for_acceptable_preference)
            {
                result = result * BF_FOR_ACCEPTABLE_PREFS;
            }
            else
            {
                result = result * BF_FOR_VALUES;
            }
        }
        return result;
    } /* --- end of common simple case --- */

    if (cond->type == POSITIVE_CONDITION)
    {
        /* --- for pos cond's, check what's bound, etc. --- */
        if (! test_covered_by_bound_vars(cond->data.tests.id_test, tc,
                                         root_vars_not_bound_yet))
        {
            return MAX_COST;
        }
        if (test_covered_by_bound_vars(cond->data.tests.attr_test, tc,
                                       root_vars_not_bound_yet))
        {
            result = 1;
        }
        else
        {
            result =  BF_FOR_ATTRIBUTES;
        }
        if (! test_covered_by_bound_vars(cond->data.tests.value_test, tc,
                                         root_vars_not_bound_yet))
        {
            if (cond->test_for_acceptable_preference)
            {
                result = result * BF_FOR_ACCEPTABLE_PREFS;
            }
            else
            {
                result = result * BF_FOR_VALUES;
            }
        }
        return result;
    }
    /* --- negated or NC conditions:  just check whether all variables
       requiring bindings are actually bound.  If so, return 1, else
       return MAX_COST --- */
    for (c = cond->reorder.vars_requiring_bindings; c != NIL; c = c->rest)
    {
        if (static_cast<Symbol*>(c->first)->tc_num != tc)
        {
            return MAX_COST;
        }
    }
    return 1;
}

/* -------------------------------------------------------------
   Return an estimate of the "cost" of the lowest-cost condition
   that could be added next, IF the given "chosen" condition is
   added first.
------------------------------------------------------------- */

int64_t find_lowest_cost_lookahead(agent* thisAgent,
                                   condition* candidates,
                                   condition* chosen,
                                   tc_number tc,
                                   cons* root_vars_not_bound_yet)
{
    condition* c;
    int64_t min_cost, cost;
    cons* new_vars;

    new_vars = NIL;
    add_bound_variables_in_condition(thisAgent, chosen, tc, &new_vars);
    min_cost = MAX_COST + 1;
    for (c = candidates; c != NIL; c = c->next)
    {
        if (c == chosen)
        {
            continue;
        }
        cost = cost_of_adding_condition(thisAgent, c, tc, root_vars_not_bound_yet);
        if (cost < min_cost)
        {
            min_cost = cost;
            if (cost <= 1)
            {
                break;
            }
        }
    }
    unmark_variables_and_free_list(thisAgent, new_vars);
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

void reorder_condition_list(agent* thisAgent,
                            condition** top_of_conds,
                            cons* roots,
                            tc_number tc,
                            bool reorder_nccs);

void reorder_simplified_conditions(agent* thisAgent,
                                   condition** top_of_conds,
                                   cons* roots,
                                   tc_number bound_vars_tc_number,
                                   bool reorder_nccs)
{
    condition* remaining_conds;           /* header of dll */
    condition* first_cond, *last_cond;
    condition* cond, *next_cond;
    condition* min_cost_conds, *chosen;
    int64_t cost = 0;
    int64_t min_cost = 0;
    cons* new_vars;

    remaining_conds = *top_of_conds;
    first_cond = NIL;
    last_cond = NIL;
    new_vars = NIL;

    /* repeat:  scan through remaining_conds
                rate each one
                if tie, call lookahead routine
                add min-cost item to conds
    */

    while (remaining_conds)
    {
        /* --- find min-cost set --- */
        min_cost_conds = NIL;
        min_cost = 0;
        for (cond = remaining_conds; cond != NIL; cond = cond->next)
        {
            cost = cost_of_adding_condition(thisAgent, cond, bound_vars_tc_number, roots);
            if ((! min_cost_conds) || (cost < min_cost))
            {
                min_cost = cost;
                min_cost_conds = cond;
                cond->reorder.next_min_cost = NIL;
            }
            else if (cost == min_cost)
            {
                cond->reorder.next_min_cost = min_cost_conds;
                min_cost_conds = cond;
            }
            /* if (min_cost <= 1) break;  This optimization needs to be removed,
                                          otherwise the tie set is not created.
                                          Without the tie set we can't check the
                                          canonical order. */
        }

        /* --- if min_cost==MAX_COST, print error message --- */
        if ((min_cost == MAX_COST) &&
            (thisAgent->outputManager->settings[OM_WARNINGS]))
        {
            thisAgent->outputManager->printa_sf(thisAgent,  "Warning:  in production %s,\n",
                  thisAgent->name_of_production_being_reordered);
            thisAgent->outputManager->printa_sf(thisAgent,  "     The LHS conditions are not all connected.\n");
            /* BUGBUG I'm not sure whether this can ever happen. */

            // XML geneneration
            growable_string gs = make_blank_growable_string(thisAgent);
            add_to_growable_string(thisAgent, &gs, "Warning:  in production ");
            add_to_growable_string(thisAgent, &gs, thisAgent->name_of_production_being_reordered);
            add_to_growable_string(thisAgent, &gs, "\n     The LHS conditions are not all connected.");
            xml_generate_warning(thisAgent, text_of_growable_string(gs));
            free_growable_string(thisAgent, gs);

        }
        /* --- if more than one min-cost item, and cost>1, do lookahead --- */
        if ((min_cost > 1) && (min_cost_conds->reorder.next_min_cost))
        {
            min_cost = MAX_COST + 1;
            for (cond = min_cost_conds, next_cond = cond->reorder.next_min_cost;
                    cond != NIL;
                    cond = next_cond, next_cond = (cond ? cond->reorder.next_min_cost : NIL))
            {
                cost = find_lowest_cost_lookahead(thisAgent, remaining_conds, cond,
                                                  bound_vars_tc_number, roots);
                if (cost < min_cost)
                {
                    min_cost = cost;
                    min_cost_conds = cond;
                    cond->reorder.next_min_cost = NIL;
                }
                else
                {
                    /*******************************************************************
                     These code segments find the condition in the tie set with the smallest
                     value in the canonical order. This ensures that productions with the
                     same set of conditions are ordered the same. Except if the variables
                     are assigned differently.
                    *********************************************************************/
                    if (cost == min_cost && cond->type == POSITIVE_CONDITION)
                    {
                        if (canonical_cond_greater(min_cost_conds, cond))
                        {
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
        if (min_cost == 1 && (min_cost_conds->reorder.next_min_cost))
        {
            for (cond = min_cost_conds; cond != NIL; cond = cond->reorder.next_min_cost)
            {
                if (cond->type == POSITIVE_CONDITION &&
                        min_cost_conds->type == POSITIVE_CONDITION &&
                        canonical_cond_greater(min_cost_conds, cond))
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
        remove_from_dll(remaining_conds, chosen, next, prev);
        if (!first_cond)
        {
            first_cond = chosen;
        }
        /* Note: args look weird on the next line, because we're really
           inserting the chosen item at the *end* of the list */
        insert_at_head_of_dll(last_cond, chosen, prev, next);

        /* --- if a conjunctive negation, recursively reorder its conditions --- */
        if ((chosen->type == CONJUNCTIVE_NEGATION_CONDITION) && reorder_nccs)
        {
            cons* ncc_roots;
            ncc_roots = collect_root_variables(thisAgent, chosen->data.ncc.top, bound_vars_tc_number);
            reorder_condition_list(thisAgent, &(chosen->data.ncc.top), ncc_roots, bound_vars_tc_number, reorder_nccs);
            free_list(thisAgent, ncc_roots);
        }

        /* --- update set of bound variables for newly added condition --- */
        add_bound_variables_in_condition(thisAgent, chosen, bound_vars_tc_number, &new_vars);

        /* --- if all roots are bound, set roots=NIL: don't need 'em anymore --- */
        if (roots)
        {
            cons* c;
            for (c = roots; c != NIL; c = c->rest)
                if (static_cast<Symbol*>(c->first)->tc_num != bound_vars_tc_number)
                {
                    break;
                }
            if (!c)
            {
                roots = NIL;
            }
        }

    } /* end of while (remaining_conds) */

    unmark_variables_and_free_list(thisAgent, new_vars);
    *top_of_conds = first_cond;
}

void reorder_condition_list(agent* thisAgent,
                            condition** top_of_conds,
                            cons* roots,
                            tc_number tc, /* for vars bound outside */
                            bool reorder_nccs)
{
    saved_test* saved_tests;

    saved_tests = simplify_condition_list(thisAgent, *top_of_conds);
    reorder_simplified_conditions(thisAgent, top_of_conds, roots, tc, reorder_nccs);
    restore_and_deallocate_saved_tests(thisAgent, *top_of_conds, tc, saved_tests);
}

/* -------------------------------------------------------------
   Reorders the LHS.
------------------------------------------------------------- */

/* SBH/MVP 6-24-94 */

bool test_tests_for_root(test t, cons* roots)
{

    cons* c;
    Symbol* referent;

    /* Gather variables from test. */

    if (!t)
    {
        return false;
    }

    switch (t->type)
    {
        case GOAL_ID_TEST:
        case IMPASSE_ID_TEST:
        case SMEM_LINK_UNARY_TEST:
        case SMEM_LINK_UNARY_NOT_TEST:
        case DISJUNCTION_TEST:
            return false;
            break;

        case CONJUNCTIVE_TEST:
            for (c = t->data.conjunct_list; c != NIL; c = c->rest)
                if (test_tests_for_root(static_cast<test>(c->first), roots))
                {
                    return true;
                }
            return false;
            break;

        default:
            /* --- relational tests other than equality --- */
            referent = t->data.referent;
            if ((referent->symbol_type == VARIABLE_SYMBOL_TYPE) &&
                    member_of_list(referent, roots))
            {
                return true;
            }
            return false;
            break;
    }
    return false;
}

/* -------------------------------------------------------------
    check_unbound_negative_relational_test_referents
    check_negative_relational_test_bindings

    These two functions are for fixing bug 517. The bug stems
    from two different code paths being used to check the bound
    variables after reordering the left hand side; one for
    positive conditions and one for negated conditions.

    Specifically, the old system would let unbound referents of
    non-equality relational tests continue past the reordering
    until the production addition failed as the bad production
    was added to the rete.

    These two functions specifically check that all referents
    of non-equality relational tests are bound and return false
    if an unbound referent is discovered.

    There may be a faster way of checking for this inside of
    the existing calls to fill_in_vars_requiring_bindings and
    reorder_condition_list, but my last attempt at fixing it
    there failed.

    Example bad production:
    sp {test
        (state <s> ^superstate nil -^foo {<> <bar>})
    -->
    }
------------------------------------------------------------- */
bool check_unbound_negative_relational_test_referents(agent* thisAgent, test t, tc_number tc)
{
    cons* c;

    // we only care about relational tests other than equality
    if (!t)
    {
        return true;
    }

    switch (t->type)
    {
        case EQUALITY_TEST:
        case GOAL_ID_TEST:
        case IMPASSE_ID_TEST:
        case SMEM_LINK_UNARY_TEST:
        case SMEM_LINK_UNARY_NOT_TEST:
        case DISJUNCTION_TEST:
            break;

        case CONJUNCTIVE_TEST:
            // we do need to loop over conjunctive tests, however
            for (c = t->data.conjunct_list; c != NIL; c = c->rest)
                if (!check_unbound_negative_relational_test_referents(thisAgent, static_cast<test>(c->first), tc))
                {
                    return false;
                }
            break;

        default:
            /* --- relational tests other than equality --- */
            if (t->data.referent->symbol_type == VARIABLE_SYMBOL_TYPE)
            {
                if (t->data.referent->tc_num != tc)
                {
                    thisAgent->outputManager->printa_sf(thisAgent,
                          "Error: production %s has an unbound referent in negated relational test %t.\n",
                          thisAgent->name_of_production_being_reordered, t);
                    return false;
                }
            }
            break;
    }
    return true;
}

bool check_negative_relational_test_bindings(agent* thisAgent, condition* cond_list, tc_number tc)
{
    cons* bound_vars = NIL;   // this list necessary pop variables bound inside ncc's out of scope on return
    condition* c;
    bool ret = true;

    /* --- add anything bound in a positive condition at this level --- */
    /* --- recurse in to NCCs --- */
    for (c = cond_list; ret && c != NIL; c = c->next)
    {
        if (c->type == POSITIVE_CONDITION)
        {
            add_bound_variables_in_condition(thisAgent, c, tc, &bound_vars);
        }
        else if (c->type == CONJUNCTIVE_NEGATION_CONDITION)
        {
            ret = check_negative_relational_test_bindings(thisAgent, c->data.ncc.top, tc);
        }
    }

    /* --- find referents of non-equality tests in conjunctive tests in negated conditions ---*/
    for (c = cond_list; ret && c != NIL; c = c->next)
    {
        if (c->type == NEGATIVE_CONDITION)
        {
            ret = check_unbound_negative_relational_test_referents(thisAgent, c->data.tests.id_test, tc);
            ret = ret && check_unbound_negative_relational_test_referents(thisAgent, c->data.tests.attr_test, tc);
            ret = ret && check_unbound_negative_relational_test_referents(thisAgent, c->data.tests.value_test, tc);
        }
    }

    // unmark anything bound on this level
    unmark_variables_and_free_list(thisAgent, bound_vars);
    return ret;
}

void remove_isa_state_tests_for_non_roots(agent* thisAgent, condition** lhs_top, cons* roots)
{
    condition* cond;
    test temp;
    for (cond = *lhs_top; cond != NIL; cond = cond->next)
    {
        if ((cond->type == POSITIVE_CONDITION) &&
                (test_includes_goal_or_impasse_id_test(cond->data.tests.id_test, true, false)) &&
                (!test_tests_for_root(cond->data.tests.id_test, roots)))
        {
            temp = cond->data.tests.id_test;
            cond->data.tests.id_test =
                copy_test(thisAgent, temp, false, false, true);
            deallocate_test(thisAgent, temp);
        }
    }
}

ProdReorderFailureType reorder_lhs(agent* thisAgent, condition** lhs_top, bool reorder_nccs, matched_symbol_list* ungrounded_syms, bool add_ungrounded)
{
    tc_number tc;
    cons* roots;

    tc = get_new_tc_number(thisAgent);
    roots = collect_root_variables(thisAgent, *lhs_top, tc, ungrounded_syms, add_ungrounded);

    /* Collecting root variables will also detect ungrounded symbols on the LHS */
    if (add_ungrounded && ungrounded_syms->size() > 0)
    {
        std::string unSymString;
        for (auto it = ungrounded_syms->begin(); it != ungrounded_syms->end(); ) {
            unSymString += (*it)->variable_sym->to_string(true);
            if (++it != ungrounded_syms->end())
            {
                unSymString += ", ";
            }
        }

        auto error = reorder_failed_unconnected_conditions;
        thisAgent->outputManager->display_reorder_error(thisAgent, error, thisAgent->name_of_production_being_reordered, unSymString.c_str());
        return error;
    }

    if (!roots)
    {
        condition* cond;

        for (cond = *lhs_top; cond != NIL; cond = cond->next)
        {
            if ((cond->type == POSITIVE_CONDITION) && (test_includes_goal_or_impasse_id_test(cond->data.tests.id_test,  true, false)))
            {
                add_bound_variables_in_test(thisAgent, cond->data.tests.id_test, tc, &roots);
                if (roots) break;
            }
        }
    if (!roots)
    {
        auto error = reorder_failed_no_roots;
        thisAgent->outputManager->display_reorder_error(thisAgent, error, thisAgent->name_of_production_being_reordered);
        return error;
    }
    }

    remove_isa_state_tests_for_non_roots(thisAgent, lhs_top, roots);

    fill_in_vars_requiring_bindings(thisAgent, *lhs_top, tc);

    reorder_condition_list(thisAgent, lhs_top, roots, tc, reorder_nccs);
    remove_vars_requiring_bindings(thisAgent, *lhs_top);
    free_list(thisAgent, roots);

    if (!check_negative_relational_test_bindings(thisAgent, *lhs_top, get_new_tc_number(thisAgent)))
    {
        auto error = reorder_failed_negative_relational_test_bindings;
        thisAgent->outputManager->display_reorder_error(thisAgent, error, thisAgent->name_of_production_being_reordered);
        return error;
    }

    return reorder_success;
}

void init_reorderer(agent* thisAgent)     /* called from init_production_utilities() */
{
    thisAgent->memoryManager->init_memory_pool(MP_saved_test, sizeof(saved_test), "saved test");
}

