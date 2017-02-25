/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/
#include "ebc.h"

#include "agent.h"
#include "condition.h"
#include "dprint.h"
#include "instantiation.h"
#include "output_manager.h"
#include "preference.h"
#include "print.h"
#include "rete.h"
#include "rhs.h"
#include "symbol.h"
#include "symbol_manager.h"
#include "test.h"
#include "working_memory.h"

#include <assert.h>

/* Methods for generating variable identities during instantiation creation */

uint64_t Explanation_Based_Chunker::get_or_create_identity(Symbol* orig_var)
{
    int64_t existing_o_id = 0;

    auto iter_sym = instantiation_identities->find(orig_var);
    if (iter_sym != instantiation_identities->end())
    {
        existing_o_id = iter_sym->second;
    }

    if (!existing_o_id)
    {
        increment_counter(ovar_id_counter);
        (*instantiation_identities)[orig_var] = ovar_id_counter;
//        instantiation_being_built->bt_identity_set_mappings->insert({ovar_id_counter, 0});

        return ovar_id_counter;
    }
    return existing_o_id;
}

void Explanation_Based_Chunker::force_add_identity(Symbol* pSym, uint64_t pID)
{
    if (pSym->is_sti()) (*instantiation_identities)[pSym] = pID;
}

void Explanation_Based_Chunker::add_identity_to_test(test pTest)
{
    if (!pTest->identity) pTest->identity = get_or_create_identity(pTest->data.referent);
}

/* Methods for assigning/propagating identity sets during instantiation creation */

identity_set* Explanation_Based_Chunker::get_floating_identity_set()
{
    increment_counter(ovar_id_counter);
    dprint(DT_PROPAGATE_ID_SETS, "Creating floating identity join set for singleton: %u\n", ovar_id_counter);
    return make_join_set(ovar_id_counter);
}

identity_set* Explanation_Based_Chunker::get_id_set_for_identity(uint64_t pID)
{
    auto iter = (*identities_to_id_sets).find(pID);
    assert(iter != (*identities_to_id_sets).end());
    return iter->second;
}

identity_set* Explanation_Based_Chunker::get_or_add_id_set_for_identity(uint64_t pID, identity_set* pIDSet)
{
    auto iter = (*identities_to_id_sets).find(pID);
    if (iter != (*identities_to_id_sets).end())
    {
        join_set_add_ref(iter->second);
        dprint(DT_PROPAGATE_ID_SETS, "Propagating identity for test with identity %u with id set %u already used in rule.  Increased refcount of %u\n", pID, iter->second->identity, iter->second->refcount);
        return iter->second;
    }
    if (pIDSet)
    {
        (*identities_to_id_sets)[pID] = pIDSet;
        join_set_add_ref(pIDSet);
        dprint(DT_PROPAGATE_ID_SETS, "Propagating identity for test with identity %u with parent id set %u.  Increasing refcount of identity join to %u\n", pID, pIDSet->identity, pIDSet->refcount);
        return pIDSet;
    } else {
        identity_set* newJoinSet = make_join_set(pID);
        (*identities_to_id_sets)[pID] = newJoinSet;
        dprint(DT_PROPAGATE_ID_SETS, "No parent identity set.  Creating new identity join set %u for %u\n", newJoinSet->identity, pID);
        return newJoinSet;
    }
}

void Explanation_Based_Chunker::update_identity_sets_in_test(test t, instantiation* pInst)
{
    cons* c;
    switch (t->type)
        {
            case GOAL_ID_TEST:
            case IMPASSE_ID_TEST:
            case SMEM_LINK_UNARY_TEST:
            case SMEM_LINK_UNARY_NOT_TEST:
            case DISJUNCTION_TEST:
                break;
            case CONJUNCTIVE_TEST:
                for (c = t->data.conjunct_list; c != NIL; c = c->rest)
                {
                    update_identity_sets_in_test(static_cast<test>(c->first), pInst);
                }
                break;
            default:
                if (t->identity)
                {
//                    if (t->identity_set) identity_set_remove_ref(t->identity_set);
                    t->identity_set = get_id_set_for_identity(t->identity);
                }
                break;
        }
}

void Explanation_Based_Chunker::update_identity_sets_in_condlist(condition* pCondTop, instantiation* pInst)
{
    condition* pCond;

    for (pCond = pCondTop; pCond != NIL; pCond = pCond->next)
    {
        if (pCond->type != CONJUNCTIVE_NEGATION_CONDITION)
        {
            update_identity_sets_in_test(pCond->data.tests.id_test, pInst);
            update_identity_sets_in_test(pCond->data.tests.attr_test, pInst);
            update_identity_sets_in_test(pCond->data.tests.value_test, pInst);
        } else {
            update_identity_sets_in_condlist(pCond->data.ncc.top, pInst);
        }
    }
}

void Explanation_Based_Chunker::update_identity_sets_in_preferences(preference* lPref)
{
    if (lPref->identities.id) lPref->identity_sets.id = get_or_add_id_set_for_identity(lPref->identities.id);
    if (lPref->identities.attr) lPref->identity_sets.attr = get_or_add_id_set_for_identity(lPref->identities.attr);
    if (lPref->identities.value) lPref->identity_sets.value = get_or_add_id_set_for_identity(lPref->identities.value);
    if (lPref->identities.referent) lPref->identity_sets.referent = get_or_add_id_set_for_identity(lPref->identities.referent);

    /* We don't deallocate the old rhs_funcs because they are created in execute_action
     * which doesn't have ownership of these rhs functions and needs to make copies for the preference
     * that it's creating. */
    if (lPref->rhs_funcs.id)
    {
        lPref->rhs_funcs.id = copy_rhs_value(thisAgent, lPref->rhs_funcs.id, true);
    }
    if (lPref->rhs_funcs.attr)
    {
        lPref->rhs_funcs.attr = copy_rhs_value(thisAgent, lPref->rhs_funcs.attr, true);
    }
    if (lPref->rhs_funcs.value)
    {
        lPref->rhs_funcs.value = copy_rhs_value(thisAgent, lPref->rhs_funcs.value, true);
    }
    if (lPref->rhs_funcs.referent)
    {
        lPref->rhs_funcs.referent = copy_rhs_value(thisAgent, lPref->rhs_funcs.referent, true);
    }

}

