/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/* =================================================================
 *
 *                               rhs.cpp
 *
 *               Utility Routines for Actions and RHS Values
 *
 * ================================================================= */
#include "portability.h"
#include <stdlib.h>

#include "rhs.h"
#include "kernel.h"
#include "print.h"
#include "agent.h"
#include "production.h"
#include "variablization_manager.h"
#include "test.h"
#include "debug.h"

test var_test_bound_in_reconstructed_conds(
    agent* thisAgent,
    condition* cond,
    byte where_field_num,
    rete_node_level where_levels_up);

/* ----------------------------------------------------------------
   Deallocates the given rhs_value.
---------------------------------------------------------------- */

void deallocate_rhs_value(agent* thisAgent, rhs_value rv)
{
    cons* c;
    list* fl;

    if (rv == NULL) return;

    dprint(DT_DEALLOCATES, "Deallocating rhs value ");

    if (rhs_value_is_reteloc(rv))
    {
        dprint_noprefix(DT_DEALLOCATES, "reteloc. Ignoring.\n");
        return;
    }
    if (rhs_value_is_unboundvar(rv))
    {
        dprint_noprefix(DT_DEALLOCATES, "unboundvar.  Ignoring.\n");
        return;
    }
    if (rhs_value_is_funcall(rv))
    {
        dprint_noprefix(DT_DEALLOCATES, "which is a function call.\n");
       fl = rhs_value_to_funcall_list(rv);
        for (c = fl->rest; c != NIL; c = c->rest)
        {
            deallocate_rhs_value(thisAgent, static_cast<char*>(c->first));
        }
        free_list(thisAgent, fl);
    }
    else
    {
        rhs_symbol r = rhs_value_to_rhs_symbol(rv);
        dprint_noprefix(DT_DEALLOCATES, "%y(o%u)\n", r->referent, r->o_id);
        if (r->referent)
        {
            symbol_remove_ref(thisAgent, r->referent);
        }
        thisAgent->memoryManager->free_with_pool(MP_rhs_symbol, r);
    }
}

/* ----------------------------------------------------------------
   Returns a new copy of the given rhs_value.
---------------------------------------------------------------- */

rhs_value copy_rhs_value(agent* thisAgent, rhs_value rv)
{
    cons* c, *new_c, *prev_new_c;
    list* fl, *new_fl;

    if (rhs_value_is_reteloc(rv))
    {
        return rv;
    }
    if (rhs_value_is_unboundvar(rv))
    {
        return rv;
    }
    if (rhs_value_is_funcall(rv))
    {
        fl = rhs_value_to_funcall_list(rv);
        allocate_cons(thisAgent, &new_fl);
        new_fl->first = fl->first;
        prev_new_c = new_fl;
        for (c = fl->rest; c != NIL; c = c->rest)
        {
            allocate_cons(thisAgent, &new_c);
            new_c->first = copy_rhs_value(thisAgent, static_cast<char*>(c->first));
            prev_new_c->rest = new_c;
            prev_new_c = new_c;
        }
        prev_new_c->rest = NIL;
        return funcall_list_to_rhs_value(new_fl);
    }
    else
    {
        rhs_symbol r = rhs_value_to_rhs_symbol(rv);
        return allocate_rhs_value_for_symbol(thisAgent, r->referent, r->o_id);
    }
}


/* ----------------------------------------------------------------
   Deallocates the given action (singly-linked) list.
---------------------------------------------------------------- */

void deallocate_action_list(agent* thisAgent, action* actions)
{
    action* a;

    while (actions)
    {
        a = actions;
        actions = actions->next;
        if (a->type == FUNCALL_ACTION)
        {
            deallocate_rhs_value(thisAgent, a->value);
        }
        else
        {
            /* --- make actions --- */
            deallocate_rhs_value(thisAgent, a->id);
            deallocate_rhs_value(thisAgent, a->attr);
            deallocate_rhs_value(thisAgent, a->value);
            if (preference_is_binary(a->preference_type))
            {
                deallocate_rhs_value(thisAgent, a->referent);
            }
        }
        thisAgent->memoryManager->free_with_pool(MP_action, a);
    }
}

/* -----------------------------------------------------------------
   Find first letter of rhs_value, or '*' if nothing appropriate.
   (See comments on first_letter_from_symbol for more explanation.)
----------------------------------------------------------------- */

char first_letter_from_rhs_value(rhs_value rv)
{
    if (rhs_value_is_symbol(rv))
    {
        return first_letter_from_symbol(rhs_value_to_symbol(rv));
    }
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

void add_all_variables_in_rhs_value(agent* thisAgent,
                                    rhs_value rv, tc_number tc,
                                    list** var_list)
{
    list* fl;
    cons* c;
    Symbol* sym;

    if (rhs_value_is_symbol(rv))
    {
        /* --- ordinary values (i.e., symbols) --- */
        sym = rhs_value_to_symbol(rv);
        if (sym->symbol_type == VARIABLE_SYMBOL_TYPE)
        {
            sym->mark_if_unmarked(thisAgent, tc, var_list);
        }
    }
    else
    {
        /* --- function calls --- */
        fl = rhs_value_to_funcall_list(rv);
        for (c = fl->rest; c != NIL; c = c->rest)
        {
            add_all_variables_in_rhs_value(thisAgent, static_cast<char*>(c->first), tc, var_list);
        }
    }
}

void add_all_variables_in_action(agent* thisAgent, action* a,
                                 tc_number tc, list** var_list)
{
    Symbol* id;

    if (a->type == MAKE_ACTION)
    {
        /* --- ordinary make actions --- */
        id = rhs_value_to_symbol(a->id);
        if (id->is_variable())
        {
            id->mark_if_unmarked(thisAgent, tc, var_list);
        }
        add_all_variables_in_rhs_value(thisAgent, a->attr, tc, var_list);
        add_all_variables_in_rhs_value(thisAgent, a->value, tc, var_list);
        if (preference_is_binary(a->preference_type))
        {
            add_all_variables_in_rhs_value(thisAgent, a->referent, tc, var_list);
        }
    }
    else
    {
        /* --- function call actions --- */
        add_all_variables_in_rhs_value(thisAgent, a->value, tc, var_list);
    }
}

void add_all_variables_in_action_list(agent* thisAgent, action* actions, tc_number tc,
                                      list** var_list)
{
    action* a;

    for (a = actions; a != NIL; a = a->next)
    {
        add_all_variables_in_action(thisAgent, a, tc, var_list);
    }
}

/* =====================================================================

   Finding the variables bound in tests, conditions, and condition lists

   These routines collect the variables that are bound in equality tests.
   Their "var_list" arguments should either be NIL or else should point
   to the header of the list of marked variables being constructed.

===================================================================== */

void add_bound_variables_in_rhs_value(agent* thisAgent,
                                      rhs_value rv, tc_number tc,
                                      list** var_list)
{
    list* fl;
    cons* c;
    Symbol* sym;

    if (rhs_value_is_symbol(rv))
    {
        /* --- ordinary values (i.e., symbols) --- */
        sym = rhs_value_to_symbol(rv);
        if (sym->symbol_type == VARIABLE_SYMBOL_TYPE)
        {
            sym->mark_if_unmarked(thisAgent, tc, var_list);
        }
    }
    else
    {
        /* --- function calls --- */
        fl = rhs_value_to_funcall_list(rv);
        for (c = fl->rest; c != NIL; c = c->rest)
        {
            add_bound_variables_in_rhs_value(thisAgent, static_cast<char*>(c->first), tc, var_list);
        }
    }
}

void add_bound_variables_in_action(agent* thisAgent, action* a,
                                   tc_number tc, list** var_list)
{
    Symbol* id;

    if (a->type == MAKE_ACTION)
    {
        /* --- ordinary make actions --- */
        id = rhs_value_to_symbol(a->id);
        add_bound_variables_in_rhs_value(thisAgent, a->id, tc, var_list);
        add_bound_variables_in_rhs_value(thisAgent, a->attr, tc, var_list);
        add_bound_variables_in_rhs_value(thisAgent, a->value, tc, var_list);
        if (preference_is_binary(a->preference_type))
        {
            add_bound_variables_in_rhs_value(thisAgent, a->referent, tc, var_list);
        }
    }
    else
    {
        /* --- function call actions --- */
        add_bound_variables_in_rhs_value(thisAgent, a->value, tc, var_list);
    }
}

void add_bound_variables_in_action_list(agent* thisAgent, action* actions, tc_number tc,
                                        list** var_list)
{
    action* a;

    for (a = actions; a != NIL; a = a->next)
    {
        add_bound_variables_in_action(thisAgent, a, tc, var_list);
    }
}

action* make_action(agent* thisAgent)
{
    action* new_action;
    thisAgent->memoryManager->allocate_with_pool(MP_action,  &new_action);
    new_action->next = NIL;
    new_action->id = NIL;
    new_action->attr = NIL;
    new_action->value = NIL;
    new_action->referent = NIL;
    return new_action;
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

rhs_value create_RHS_value(agent* thisAgent,
                           rhs_value rv,
                           condition* cond,
                           char first_letter,
                           uint64_t pI_id,
                           AddAdditionalTestsMode add_original_vars)
{
    cons* c, *new_c, *prev_new_c;
    list* fl, *new_fl;
    Symbol* sym;
    int64_t index;
    char prefix[2];
    test t, original_t;
    uint64_t lO_id = 0;


    /* -- (1) Reteloc's seemed to be used for identifiers or constants originally bound to
     *    variables
     *    (2) unbound_vars for unbound vars of course
     *    (3) rhs_symbols for literal constants, including those in RHS functions -- */

    if (rhs_value_is_reteloc(rv))
    {
        /* -- rv is a symbol pointed to by a rete location
         *    This case seems to only be for identifiers or constants originally bound to variables -- */

        t = var_test_bound_in_reconstructed_conds(thisAgent, cond,
                rhs_value_to_reteloc_field_num(rv),
                rhs_value_to_reteloc_levels_up(rv));
        return allocate_rhs_value_for_symbol(thisAgent, t->data.referent, t->identity);
    }

    if (rhs_value_is_unboundvar(rv))
    {
        /* -- rv is a an unbound variable -- */
        index = static_cast<int64_t>(rhs_value_to_unboundvar(rv));
        if (! *(thisAgent->rhs_variable_bindings + index))
        {
            prefix[0] = first_letter;
            prefix[1] = 0;

            sym = generate_new_variable(thisAgent, prefix);
            *(thisAgent->rhs_variable_bindings + index) = sym;

            if (thisAgent->highest_rhs_unboundvar_index < index)
            {
                thisAgent->highest_rhs_unboundvar_index = index;
            }
            /* -- generate will increment the refcount on the new variable,
             *    so don't need to do it here. -- */
            if (add_original_vars && pI_id)
            {
                lO_id = thisAgent->variablizationManager->get_or_create_o_id(sym, pI_id);
            }
            return allocate_rhs_value_for_symbol_no_refcount(thisAgent, sym, lO_id);
        }
        else
        {
            /* -- unbound variable was already created in previous rhs action -- */
            sym = *(thisAgent->rhs_variable_bindings + index);
        }
        if (add_original_vars && pI_id)
        {
            lO_id = thisAgent->variablizationManager->get_existing_o_id(sym, pI_id);
        }

        return allocate_rhs_value_for_symbol(thisAgent, sym, lO_id);
    }

    if (rhs_value_is_funcall(rv))
    {
        /* -- rv is a rhs function -- */
        fl = rhs_value_to_funcall_list(rv);
        allocate_cons(thisAgent, &new_fl);
        new_fl->first = fl->first;
        prev_new_c = new_fl;
        for (c = fl->rest; c != NIL; c = c->rest)
        {
            allocate_cons(thisAgent, &new_c);
            new_c->first = create_RHS_value(thisAgent,
                                            static_cast<char*>(c->first),
                                            cond,
                                            first_letter,
                                            pI_id,
                                            add_original_vars);
//            uint64_t test_oid = rhs_value_to_o_id(static_cast<char*>(new_c->first));
//            rhs_symbol test_rsym = rhs_value_to_rhs_symbol(static_cast<char*>(new_c->first));
//            test_oid  = test_rsym->o_id;
//            test_oid = rhs_value_to_o_id(static_cast<char*>(new_c->first));
            prev_new_c->rest = new_c;
            prev_new_c = new_c;
//            dprint(DT_RHS_VARIABLIZATION, "%fRHS Value1 is %r\n", static_cast<char*>(c->first));
        }
        prev_new_c->rest = NIL;
        return funcall_list_to_rhs_value(new_fl);
    }
    else
    {
        /* -- rv is a rhs_symbol
         *    This case seems to only be for literal values including those in function calls. -- */

        rhs_symbol rs = rhs_value_to_rhs_symbol(rv);
        uint64_t lO_id = (add_original_vars != DONT_ADD_TESTS) ? rs->o_id : 0;
        return allocate_rhs_value_for_symbol(thisAgent, rs->referent, lO_id);
    }
}

action* create_RHS_action_list(agent* thisAgent,
                               action* actions,
                               condition* cond,
                               uint64_t pI_id,
                               AddAdditionalTestsMode add_original_vars)
{
    action* old, *New, *prev, *first;
    char first_letter;

    prev = NIL;
    first = NIL;
    old = actions;
    while (old)
    {
        New = make_action(thisAgent);
        if (prev)
        {
            prev->next = New;
        }
        else
        {
            first = New;
        }
        prev = New;
        New->type = old->type;
        New->preference_type = old->preference_type;
        New->support = old->support;
        if (old->type == FUNCALL_ACTION)
        {
            New->value = create_RHS_value(thisAgent, old->value, cond, 'v', pI_id, add_original_vars);
        }
        else
        {
            New->id = create_RHS_value(thisAgent, old->id, cond, 's', pI_id, add_original_vars);
            New->attr = create_RHS_value(thisAgent, old->attr, cond, 'a', pI_id, add_original_vars);
            first_letter = first_letter_from_rhs_value(New->attr);
            New->value = create_RHS_value(thisAgent, old->value, cond,
                                          first_letter, pI_id, add_original_vars);
            if (preference_is_binary(old->preference_type))
            {
                New->referent = create_RHS_value(thisAgent, old->referent,
                                                 cond, first_letter, pI_id, add_original_vars);
            }
        }
        old = old->next;
    }
    if (prev)
    {
        prev->next = NIL;
    }
    else
    {
        first = NIL;
    }
    return first;
}

rhs_value allocate_rhs_value_for_symbol_no_refcount(agent* thisAgent, Symbol* sym, uint64_t pO_ID)
{
    rhs_symbol new_rhs_symbol;

    if (!sym)
    {
        dprint(DT_RHS_VARIABLIZATION, "allocate_rhs_value_no_refcount called with nil.\n");
        return reinterpret_cast<rhs_value>(NIL);
    }
    thisAgent->memoryManager->allocate_with_pool(MP_rhs_symbol, &new_rhs_symbol);
    new_rhs_symbol->referent = sym;
    new_rhs_symbol->o_id = pO_ID;
//    dprint(DT_RHS_VARIABLIZATION, "Allocated new rhs_value for new rhs_symbol %y(o%u).\n", sym, pO_ID);

    return rhs_symbol_to_rhs_value(new_rhs_symbol);
}

rhs_value allocate_rhs_value_for_symbol(agent* thisAgent, Symbol* sym, uint64_t pO_ID)
{
    if (sym)
    {
        symbol_add_ref(thisAgent, sym);
    }
    return allocate_rhs_value_for_symbol_no_refcount(thisAgent, sym, pO_ID);
}

