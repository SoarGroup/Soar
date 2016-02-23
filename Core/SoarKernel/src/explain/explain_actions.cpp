#include "explain.h"
#include "agent.h"
#include "condition.h"
#include "debug.h"
#include "instantiation.h"
#include "preference.h"
#include "production.h"
#include "rhs.h"
#include "symbol.h"
#include "test.h"
#include "output_manager.h"
#include "working_memory.h"

action_record::action_record(agent* myAgent, preference* pPref, action* pAction, uint64_t pActionID)
{
    thisAgent               = myAgent;
    actionID                = pActionID;
    instantiated_pref       = shallow_copy_preference(thisAgent, pPref);
    original_pref           = pPref;
    if (pAction)
    {
        variablized_action = copy_action(thisAgent, pAction);
    } else {
        variablized_action = NULL;
    }
    identities_used = NULL;
    dprint(DT_EXPLAIN_CONDS, "   Created action record a%u for pref %p (%r ^%r %r), [act %a]", pActionID, pPref, pPref->rhs_funcs.id, pPref->rhs_funcs.attr, pPref->rhs_funcs.value, pAction);
}

void action_record::update_action(preference* pPref)
{
    // Can this ever really change?  If not remove this function.
    assert(original_pref == pPref);
    original_pref           = pPref;
//    dprint(DT_EXPLAIN_CONDS, "   Updated action record a%u for pref %p (%r ^%r %r)", actionID, pPref, pPref->rhs_funcs.id, pPref->rhs_funcs.attr, pPref->rhs_funcs.value);
}

action_record::~action_record()
{
    dprint(DT_EXPLAIN_CONDS, "   Deleting action record a%u for: %p\n", actionID, instantiated_pref);
    deallocate_preference(thisAgent, instantiated_pref);
    deallocate_action_list(thisAgent, variablized_action);
    if (identities_used)
    {
        delete identities_used;
    }
}

void add_all_identities_in_rhs_value(agent* thisAgent, rhs_value rv, id_set* pIDSet)
{
    list* fl;
    cons* c;
    Symbol* sym;

    if (rhs_value_is_symbol(rv))
    {
        /* --- ordinary values (i.e., symbols) --- */
        sym = rhs_value_to_symbol(rv);
        if (sym->is_variable())
        {
            dprint(DT_EXPLAIN_PATHS, "Adding identity %u from rhs value to id set...\n", rhs_value_to_o_id(rv));
            pIDSet->insert(rhs_value_to_o_id(rv));
        }
    }
    else
    {
        /* --- function calls --- */
        fl = rhs_value_to_funcall_list(rv);
        for (c = fl->rest; c != NIL; c = c->rest)
        {
            add_all_identities_in_rhs_value(thisAgent, static_cast<char*>(c->first), pIDSet);
        }
    }
}

void add_all_identities_in_action(agent* thisAgent, action* a, id_set* pIDSet)
{
    Symbol* id;

    if (a->type == MAKE_ACTION)
    {
        /* --- ordinary make actions --- */
        id = rhs_value_to_symbol(a->id);
        if (id->is_variable())
        {
            dprint(DT_EXPLAIN_PATHS, "Adding identity %u from rhs action id to id set...\n", rhs_value_to_o_id(a->id));
            pIDSet->insert(rhs_value_to_o_id(a->id));
        }
        add_all_identities_in_rhs_value(thisAgent, a->attr, pIDSet);
        add_all_identities_in_rhs_value(thisAgent, a->value, pIDSet);
        if (preference_is_binary(a->preference_type))
        {
            add_all_identities_in_rhs_value(thisAgent, a->referent, pIDSet);
        }
    }
    else
    {
        /* --- function call actions --- */
        add_all_identities_in_rhs_value(thisAgent, a->value, pIDSet);
    }
}

id_set* action_record::get_identities()
{
    if (!identities_used)
    {
        identities_used = new id_set();
        add_all_identities_in_action(thisAgent, variablized_action, identities_used);
    }

    return identities_used;
}
