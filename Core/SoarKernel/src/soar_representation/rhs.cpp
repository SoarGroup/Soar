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
#include "rhs.h"
#include "rhs_functions.h"

#include "agent.h"
#include "dprint.h"
#include "ebc.h"
#include "ebc_identity.h"
#include "preference.h"
#include "print.h"
#include "production.h"
#include "symbol_manager.h"
#include "symbol.h"
#include "test.h"

#include <stdlib.h>

test var_test_bound_in_reconstructed_conds(
    agent* thisAgent,
    condition* cond,
    byte where_field_num,
    rete_node_level where_levels_up);

/*--------------------------------------------------------------
   Deallocates the given rhs_value.
--------------------------------------------------------------*/

void deallocate_rhs_value(agent* thisAgent, rhs_value rv)
{
    cons* c;
    cons* fl;

    if (!rv || rhs_value_is_reteloc(rv) || rhs_value_is_unboundvar(rv)) return;

    dprint(DT_DEALLOCATE_RHS_VALUE, "Deallocating rhs value %r\n", rv);
    if (rhs_value_is_funcall(rv))
    {

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
        if (r->referent) thisAgent->symbolManager->symbol_remove_ref(&r->referent);
        thisAgent->memoryManager->free_with_pool(MP_rhs_symbol, r);
    }
}

/*--------------------------------------------------------------
   Returns a new copy of the given rhs_value.
--------------------------------------------------------------*/

rhs_value copy_rhs_value(agent* thisAgent, rhs_value rv, bool get_identity_set, bool get_cloned_identity)
{
    cons* c = NULL, *new_c = NULL, *prev_new_c = NULL;
    cons* fl=NULL, *new_fl=NULL;

    if (!rv) {
            return NULL;
    } else if ((rhs_value_is_reteloc(rv)) || (rhs_value_is_unboundvar(rv)))
    {
        return rv;
    } else if (rhs_value_is_funcall(rv))
    {
        fl = rhs_value_to_funcall_list(rv);
        allocate_cons(thisAgent, &new_fl);
        new_fl->first = fl->first;
        prev_new_c = new_fl;
        for (c = fl->rest; c != NIL; c = c->rest)
        {
            allocate_cons(thisAgent, &new_c);
            new_c->first = copy_rhs_value(thisAgent, static_cast<char*>(c->first), get_identity_set, get_cloned_identity);
            prev_new_c->rest = new_c;
            prev_new_c = new_c;
        }
        prev_new_c->rest = NIL;
        return funcall_list_to_rhs_value(new_fl);
    }
    else
    {
        rhs_symbol r = rhs_value_to_rhs_symbol(rv);
        uint64_t lID = r->inst_identity;
        Identity* l_identity = r->identity;
        if (get_identity_set)
        {
            if (l_identity)
            {
                if (l_identity->get_clone_identity())
                {
                    assert(false);
                    l_identity = thisAgent->explanationBasedChunker->get_identity_for_id(l_identity->get_clone_identity());
                }
                else
                {
                    l_identity = thisAgent->explanationBasedChunker->get_identity_for_id(l_identity->get_identity());
                }
            }
            else if (lID)
                l_identity = thisAgent->explanationBasedChunker->get_identity_for_id(lID);
        }
        if (l_identity && get_cloned_identity)
        {
            lID = l_identity->get_clone_identity();
            l_identity = NULL_IDENTITY_SET; // Will be filled in later based when finalizing
        }

        return allocate_rhs_value_for_symbol(thisAgent, r->referent, lID, l_identity, r->was_unbound_var);
    }
}

/*--------------------------------------------------------------
   Deallocates the given action (singly-linked) list.
--------------------------------------------------------------*/

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
            /*- make actions -*/
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

/*---------------------------------------------------------------
   Find first letter of rhs_value, or '*' if nothing appropriate.
   (See comments on first_letter_from_symbol for more explanation.)
---------------------------------------------------------------*/

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
                                    cons** var_list)
{
    cons* fl;
    cons* c;
    Symbol* sym;

    if (rhs_value_is_symbol(rv))
    {
        /*- ordinary values (i.e., symbols) -*/
        sym = rhs_value_to_symbol(rv);
        if (sym->is_variable())
        {
            sym->mark_if_unmarked(thisAgent, tc, var_list);
        }
    }
    else
    {
        /*- function calls -*/
        fl = rhs_value_to_funcall_list(rv);
        for (c = fl->rest; c != NIL; c = c->rest)
        {
            add_all_variables_in_rhs_value(thisAgent, static_cast<char*>(c->first), tc, var_list);
        }
    }
}

/* The add_LTI parameter is available so that when Soar is marking symbols for
 * action ordering based on whether the levels of the symbols would be known,
 * it also consider whether the LTIs level can be determined by being linked
 * to a LHS element or a RHS action that has already been executed */

void add_all_variables_in_action(agent* thisAgent, action* a,
                                 tc_number tc, cons** var_list)
{
    Symbol* id;

    if (a->type == MAKE_ACTION)
    {
        /*- ordinary make actions -*/
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
        /*- function call actions -*/
        add_all_variables_in_rhs_value(thisAgent, a->value, tc, var_list);
    }
}

void add_all_variables_in_action_list(agent* thisAgent, action* actions, tc_number tc,
                                      cons** var_list)
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
                                      cons** var_list)
{
    cons* fl;
    cons* c;
    Symbol* sym;

    if (rhs_value_is_symbol(rv))
    {
        /*- ordinary values (i.e., symbols) -*/
        sym = rhs_value_to_symbol(rv);
        if (sym->symbol_type == VARIABLE_SYMBOL_TYPE)
        {
            sym->mark_if_unmarked(thisAgent, tc, var_list);
        }
    }
    else
    {
        /*- function calls -*/
        fl = rhs_value_to_funcall_list(rv);
        for (c = fl->rest; c != NIL; c = c->rest)
        {
            add_bound_variables_in_rhs_value(thisAgent, static_cast<char*>(c->first), tc, var_list);
        }
    }
}

void add_bound_variables_in_action(agent* thisAgent, action* a,
                                   tc_number tc, cons** var_list)
{
    Symbol* id;

    if (a->type == MAKE_ACTION)
    {
        /*- ordinary make actions -*/
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
        /*- function call actions -*/
        add_bound_variables_in_rhs_value(thisAgent, a->value, tc, var_list);
    }
}

void add_bound_variables_in_action_list(agent* thisAgent, action* actions, tc_number tc,
                                        cons** var_list)
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

action* copy_action(agent* thisAgent, action* pAction)
{
    action* new_action = make_action(thisAgent);
    new_action->type = pAction->type;
    new_action->preference_type = pAction->preference_type;
    new_action->support = pAction->support;
    if (pAction->type == FUNCALL_ACTION)
    {
        new_action->value = copy_rhs_value(thisAgent, pAction->value);
    }
    else
    {
        new_action->id = copy_rhs_value(thisAgent, pAction->id);
        new_action->attr = copy_rhs_value(thisAgent, pAction->attr);
        new_action->value = copy_rhs_value(thisAgent, pAction->value);
        if (preference_is_binary(pAction->preference_type)) new_action->referent = copy_rhs_value(thisAgent, pAction->referent);
    }
    return new_action;
}
/*-----------------------------------------------------------------
             Reconstructing the RHS Actions of a Production

   When we print a production (but not when we fire one), we have to
   reconstruct the RHS actions.  This is because many of the variables
   in the RHS have been replaced by references to Rete locations (i.e.,
   rather than specifying <v>, we specify "value field 3 levels up"
   or "the 7th RHS unbound variable".  The routines below copy rhs_value's
   and actions, and substitute variable names for such references.
   For RHS unbound variables, we gensym new variable names.
-----------------------------------------------------------------*/

rhs_value create_RHS_value(agent* thisAgent,
                           rhs_value rv,
                           condition* cond,
                           char first_letter,
                           ExplainTraceType ebcTraceType)
{
    cons* c, *new_c, *prev_new_c;
    cons* fl, *new_fl;
    Symbol* sym;
    int64_t index;
    char prefix[2];
	test t;//, original_t;
    uint64_t lO_id = 0;


    /* (1) Reteloc identifiers or constants originally bound to variables
     * (2) unbound_vars for unbound vars of course
     * (3) rhs_symbols for literal constants, including those in RHS functions */

    if (rhs_value_is_reteloc(rv))
    {
        /* Symbol pointed to by a rete location
         * This case seems to only be for identifiers or constants originally bound to variables */

        t = var_test_bound_in_reconstructed_conds(thisAgent, cond,
                rhs_value_to_reteloc_field_num(rv),
                rhs_value_to_reteloc_levels_up(rv));
        dprint(DT_ALLOCATE_RHS_VALUE, "create_RHS_value: reteloc %y %us%u\n", t->data.referent, t->inst_identity, t->identity ? t->identity->get_identity() : 0);

        return allocate_rhs_value_for_symbol(thisAgent, t->data.referent, t->inst_identity, t->identity);
    }

    if (rhs_value_is_unboundvar(rv))
    {
        /* Unbound variable */
        index = static_cast<int64_t>(rhs_value_to_unboundvar(rv));
        if (! *(thisAgent->rhs_variable_bindings + index))
        {
            prefix[0] = first_letter;
            prefix[1] = 0;

            sym = thisAgent->symbolManager->generate_new_variable(prefix);
            *(thisAgent->rhs_variable_bindings + index) = sym;

            if (thisAgent->highest_rhs_unboundvar_index < index)
            {
                thisAgent->highest_rhs_unboundvar_index = index;
            }
            if (ebcTraceType == Explanation_Trace)
            {
                lO_id = thisAgent->explanationBasedChunker->get_or_create_inst_identity_for_sym(sym);
            }
            /* generate will increment the refcount on the new variable, so don't need to do it here. */
            dprint(DT_ALLOCATE_RHS_VALUE, "create_RHS_value: unbound %y %u\n", sym, lO_id);
            return allocate_rhs_value_for_symbol_no_refcount(thisAgent, sym, lO_id, NULL, true);
        }
        else
        {
            /* unbound variable was already created in previous rhs action */
            sym = *(thisAgent->rhs_variable_bindings + index);
        }
        if (ebcTraceType == Explanation_Trace)
        {
            lO_id = thisAgent->explanationBasedChunker->get_or_create_inst_identity_for_sym(sym);
        }

        dprint(DT_ALLOCATE_RHS_VALUE, "create_RHS_value: previous unbound %y [%u]\n", sym, lO_id);
        return allocate_rhs_value_for_symbol(thisAgent, sym, lO_id, NULL, true);
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
            new_c->first = create_RHS_value(thisAgent,
                                            static_cast<char*>(c->first),
                                            cond,
                                            first_letter,
                                            ebcTraceType);
            prev_new_c->rest = new_c;
            prev_new_c = new_c;
        }
        prev_new_c->rest = NIL;
        return funcall_list_to_rhs_value(new_fl);
    }
    else
    {
        /* Literal values including those in function calls. */
        rhs_symbol rs = rhs_value_to_rhs_symbol(rv);
        dprint(DT_ALLOCATE_RHS_VALUE, "create_RHS_value: rhs_symbol %y %u\n", rs->referent, rs->inst_identity);
        if (ebcTraceType == Explanation_Trace)
            return allocate_rhs_value_for_symbol(thisAgent, rs->referent, rs->inst_identity, rs->identity, rs->was_unbound_var);
        else
            return allocate_rhs_value_for_symbol(thisAgent, rs->referent, LITERAL_VALUE, NULL, rs->was_unbound_var);
    }
}

action* create_RHS_action_list(agent* thisAgent,
                               action* actions,
                               condition* cond,
                               ExplainTraceType ebcTraceType)
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
            New->value = create_RHS_value(thisAgent, old->value, cond, 'v', ebcTraceType);
        }
        else
        {
            New->id = create_RHS_value(thisAgent, old->id, cond, 's', ebcTraceType);
            New->attr = create_RHS_value(thisAgent, old->attr, cond, 'a', ebcTraceType);
            first_letter = first_letter_from_rhs_value(New->attr);
            New->value = create_RHS_value(thisAgent, old->value, cond, first_letter, ebcTraceType);
            if (preference_is_binary(old->preference_type))
            {
                New->referent = create_RHS_value(thisAgent, old->referent, cond, first_letter, ebcTraceType);
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

rhs_value allocate_rhs_value_for_symbol_no_refcount(agent* thisAgent, Symbol* sym, uint64_t pInstIdentity, Identity* pIdentity, bool pWasUnbound)
{
    rhs_symbol new_rhs_symbol;

    if (!sym) return NULL;
    thisAgent->memoryManager->allocate_with_pool(MP_rhs_symbol, &new_rhs_symbol);
    new_rhs_symbol->referent = sym;
    new_rhs_symbol->inst_identity = pInstIdentity;
    new_rhs_symbol->identity_id_unjoined = LITERAL_VALUE;
    new_rhs_symbol->identity = pIdentity;
    new_rhs_symbol->was_unbound_var = pWasUnbound;
    return rhs_symbol_to_rhs_value(new_rhs_symbol);
}

rhs_value allocate_rhs_value_for_symbol(agent* thisAgent, Symbol* sym, uint64_t pInstIdentity, Identity* pIdentity, bool pWasUnbound)
{
    rhs_symbol new_rhs_symbol;

    if (!sym)
    {
        return NULL;
    } else {
        thisAgent->symbolManager->symbol_add_ref(sym);
    }
    thisAgent->memoryManager->allocate_with_pool(MP_rhs_symbol, &new_rhs_symbol);
    new_rhs_symbol->referent = sym;
    new_rhs_symbol->inst_identity = pInstIdentity;
    new_rhs_symbol->identity_id_unjoined = LITERAL_VALUE;
    new_rhs_symbol->identity = pIdentity;
    new_rhs_symbol->was_unbound_var = pWasUnbound;
    return rhs_symbol_to_rhs_value(new_rhs_symbol);
}

bool rhs_value_is_literalizing_function(rhs_value rv)
{
    cons* fl;
    rhs_function* rf;
    if (rhs_value_is_funcall(rv))
    {
        fl = rhs_value_to_funcall_list(rv);
        rf = static_cast<rhs_function_struct*>(fl->first);
        return rf->literalize_arguments;
    }
    return false;
}


