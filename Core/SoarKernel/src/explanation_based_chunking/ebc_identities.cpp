/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/
#include "ebc.h"

#include "agent.h"
#include "dprint.h"
#include "preference.h"
#include "rhs.h"
#include "test.h"

uint64_t        get_superjoin_id(identity_set* pIDSet)  { if (pIDSet) return pIDSet->super_join->identity; else return NULL_IDENTITY_SET; }

/* Methods for generating variable identities during instantiation creation */

uint64_t Explanation_Based_Chunker::get_or_create_identity_for_sym(Symbol* pSym)
{
    int64_t existing_o_id = 0;

    auto iter_sym = instantiation_identities->find(pSym);
    if (iter_sym != instantiation_identities->end())
    {
        existing_o_id = iter_sym->second;
    }

    if (!existing_o_id)
    {
        increment_counter(variablization_identity_counter);
        (*instantiation_identities)[pSym] = variablization_identity_counter;
        return variablization_identity_counter;
    }
    return existing_o_id;
}

void Explanation_Based_Chunker::force_add_identity(Symbol* pSym, uint64_t pID)
{
    if (pSym->is_sti()) (*instantiation_identities)[pSym] = pID;
}

void Explanation_Based_Chunker::add_identity_to_test(test pTest)
{
    if (!pTest->identity) pTest->identity = get_or_create_identity_for_sym(pTest->data.referent);
}

/* Methods for assigning/propagating identity sets during instantiation creation */

identity_set* Explanation_Based_Chunker::get_floating_identity_set()
{
    increment_counter(variablization_identity_counter);
    dprint(DT_PROPAGATE_ID_SETS, "Creating floating identity join set for singleton: %u\n", variablization_identity_counter);
    return make_identity_set(variablization_identity_counter);
}

identity_set* Explanation_Based_Chunker::get_id_set_for_identity(uint64_t pID)
{
    auto iter = (*identities_to_id_sets).find(pID);
    if (iter != (*identities_to_id_sets).end()) return iter->second;
    else return NULL;
}

identity_set* Explanation_Based_Chunker::get_or_add_id_set_for_identity(uint64_t pID, identity_set* pIDSet, bool* pOwnsIdentitySet)
{
    auto iter = (*identities_to_id_sets).find(pID);
    if (iter != (*identities_to_id_sets).end())
    {
        dprint(DT_PROPAGATE_ID_SETS, "Assigning identity set for variable identity %u with identity set %u already used in rule.  Increased refcount of %u\n", pID, iter->second->identity);
        (*pOwnsIdentitySet) = false;
        return iter->second;
    }
    if (pIDSet)
    {
        (*identities_to_id_sets)[pID] = pIDSet;
        dprint(DT_PROPAGATE_ID_SETS, "Propagating identity set for variable identity %u with parent identity set %u\n", pID, pIDSet->identity);
        (*pOwnsIdentitySet) = false;
        return pIDSet;
    } else {
        identity_set* newJoinSet = make_identity_set(pID);
        (*identities_to_id_sets)[pID] = newJoinSet;
        (*pOwnsIdentitySet) = true;
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
                    identity_set* updated_id_set = get_id_set_for_identity(t->identity);
                    if (t->identity_set && t->owns_identity_set && (t->identity_set != updated_id_set))
                    {
                        deallocate_identity_set(t->identity_set);
                        /* We no longer own our identity set.  A previous test in the rule or a singleton wme own it */
                        t->owns_identity_set = false;
                    }
                    t->identity_set = updated_id_set;
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
    identity_set* updated_id_set;
    bool lOwnedIdentitySet;

    if (lPref->identities.id)
    {
        lOwnedIdentitySet = lPref->owns_identity_set.id;
        identity_set* updated_id_set = get_or_add_id_set_for_identity(lPref->identities.id, NULL, &(lPref->owns_identity_set.id));
        if (lPref->identity_sets.id && lOwnedIdentitySet && (lPref->identity_sets.id != updated_id_set)) deallocate_identity_set(lPref->identity_sets.id);
        lPref->identity_sets.id = updated_id_set;
    }
    if (lPref->identities.attr)
    {
        lOwnedIdentitySet = lPref->owns_identity_set.attr;
        identity_set* updated_id_set = get_or_add_id_set_for_identity(lPref->identities.attr, NULL, &(lPref->owns_identity_set.attr));
        if (lPref->identity_sets.attr && lOwnedIdentitySet && (lPref->identity_sets.attr != updated_id_set)) deallocate_identity_set(lPref->identity_sets.attr);
        lPref->identity_sets.attr = updated_id_set;
    }
    if (lPref->identities.value)
    {
        lOwnedIdentitySet = lPref->owns_identity_set.value;
        identity_set* updated_id_set = get_or_add_id_set_for_identity(lPref->identities.value, NULL, &(lPref->owns_identity_set.value));
        if (lPref->identity_sets.value && lOwnedIdentitySet && (lPref->identity_sets.value != updated_id_set)) deallocate_identity_set(lPref->identity_sets.value);
        lPref->identity_sets.value = updated_id_set;
    }
    if (lPref->identities.referent)
    {
        lOwnedIdentitySet = lPref->owns_identity_set.referent;
        identity_set* updated_id_set = get_or_add_id_set_for_identity(lPref->identities.referent, NULL, &(lPref->owns_identity_set.referent));
        if (lPref->identity_sets.referent && lOwnedIdentitySet && (lPref->identity_sets.referent != updated_id_set)) deallocate_identity_set(lPref->identity_sets.referent);
        lPref->identity_sets.referent = updated_id_set;
    }

    /* Note:  We don't deallocate the existing rhs_funcs before replacing them because they are created in
     *        execute_action which doesn't have ownership of these rhs functions and previously made copies
     *        for the preference that it's creating. We now just moved it here so that it can be done after
     *        identity sets have been fully unified. */
    if (lPref->rhs_funcs.id) lPref->rhs_funcs.id = copy_rhs_value(thisAgent, lPref->rhs_funcs.id, true);
    if (lPref->rhs_funcs.attr) lPref->rhs_funcs.attr = copy_rhs_value(thisAgent, lPref->rhs_funcs.attr, true);
    if (lPref->rhs_funcs.value) lPref->rhs_funcs.value = copy_rhs_value(thisAgent, lPref->rhs_funcs.value, true);
    if (lPref->rhs_funcs.referent) lPref->rhs_funcs.referent = copy_rhs_value(thisAgent, lPref->rhs_funcs.referent, true);

}

