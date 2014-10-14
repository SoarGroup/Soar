/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

#include "portability.h"
#include <stdlib.h>

#include "kernel.h"
#include "print.h"
#include "mem.h"
#include "symtab.h"
#include "init_soar.h"
#include "gsysparam.h"
#include "agent.h"
#include "production.h"
#include "rhs.h"
#include "rhs_functions.h"
#include "rhs_functions_math.h"
#include "io_soar.h"
#include "recmem.h"
#include "wmem.h"
#include "test.h"
#include "decide.h"

#include "xml.h"
#include "soar_TraceNames.h"

#include <map>
#include <string>
#include <time.h>

using namespace soar_TraceNames;

/* =================================================================

              Utility Routines for Actions and RHS Values

================================================================= */

/* ----------------------------------------------------------------
   Deallocates the given rhs_value.
---------------------------------------------------------------- */

void deallocate_rhs_value(agent* thisAgent, rhs_value rv)
{
    cons* c;
    list* fl;

    if (rhs_value_is_reteloc(rv))
    {
        return;
    }
    if (rhs_value_is_unboundvar(rv))
    {
        return;
    }
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
        symbol_remove_ref(thisAgent, rhs_value_to_symbol(rv));
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
        symbol_add_ref(thisAgent, rhs_value_to_symbol(rv));
        return rv;
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
        free_with_pool(&thisAgent->action_pool, a);
    }
}

/* -----------------------------------------------------------------
 * Looks through an rhs_value, returns appropriate first letter for
 * a dummy variable to follow it.  Returns '*' if none found.
 * (See comments on first_letter_from_symbol for more explanation.)
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
            mark_variable_if_unmarked(thisAgent, sym, tc, var_list);
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
            mark_variable_if_unmarked(thisAgent, id, tc, var_list);
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

