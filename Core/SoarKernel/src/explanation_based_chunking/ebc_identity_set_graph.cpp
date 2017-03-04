/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/
#include "ebc.h"
#include "ebc_timers.h"

#include "agent.h"
#include "condition.h"
#include "debug_inventories.h"
#include "dprint.h"
#include "symbol_manager.h"
#include "preference.h"
#include "rhs.h"
#include "test.h"

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

identity_set* Explanation_Based_Chunker::get_or_add_id_set(uint64_t pID, identity_set* pIDSet, bool* pOwnsIdentitySet)
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

identity_set* Explanation_Based_Chunker::make_identity_set(uint64_t pIdentity)
{
    identity_set* new_join_set;
    thisAgent->memoryManager->allocate_with_pool(MP_identity_sets, &new_join_set);
//    new_join_set->identity = pIdentity;
    new_join_set->identity = get_new_identity_set_id();
    new_join_set->dirty = false;
    new_join_set->super_join = new_join_set;
    new_join_set->identity_sets = NULL;
    new_join_set->new_var = NULL;
    new_join_set->clone_identity = NULL_IDENTITY_SET;
    new_join_set->literalized = false;
    new_join_set->operational_cond = NULL;
    new_join_set->operational_field = NO_ELEMENT;
    ISI_add(thisAgent, new_join_set);

    dprint(DT_DEALLOCATE_ID_SETS, "Created identity set %u for variable identity %u\n", new_join_set->identity, pIdentity);
    return new_join_set;
}

void Explanation_Based_Chunker::deallocate_identity_set(identity_set* &pIDSet, IDSet_Deallocation_Type pDeallocType)
{
    dprint(DT_DEALLOCATE_ID_SETS, "Deallocating identity set %u%s\n", pIDSet->identity, pIDSet->dirty ? " (dirty)." : " (not dirty)");
    if (pIDSet->dirty)
    {
        if (pIDSet->super_join != pIDSet)
        {
            dprint(DT_DEALLOCATE_ID_SETS, "...removing identity set %u from super-join %u's identity_sets\n", pIDSet->identity, pIDSet->super_join->identity);
            pIDSet->super_join->identity_sets->remove(pIDSet);
        }
        if (pIDSet->identity_sets)
        {
            identity_set* lPreviouslyJoinedIdentity;
            for (auto it = pIDSet->identity_sets->begin(); it != pIDSet->identity_sets->end(); it++)
            {
                lPreviouslyJoinedIdentity = *it;
                dprint(DT_DEALLOCATE_ID_SETS, "...resetting previous join set mapping of %u to %u\n", lPreviouslyJoinedIdentity->identity, pIDSet->identity);
                lPreviouslyJoinedIdentity->super_join = lPreviouslyJoinedIdentity;
            }
        }
        clean_up_identity_set_transient(pIDSet);
        identity_sets_to_clean_up.erase(pIDSet);
    }
    ISI_remove(thisAgent, pIDSet);
    thisAgent->memoryManager->free_with_pool(MP_identity_sets, pIDSet);
    pIDSet = NULL;
}

void Explanation_Based_Chunker::clean_up_identity_set_transient(identity_set* pIDSet)
{
    dprint(DT_DEALLOCATE_ID_SETS, "...cleaning up transient data in %s identity set %u (%s, var = %y, # id sets = %d, clone id = %u)\n",
        pIDSet->dirty ? "dirty" : "clean", pIDSet->identity, (pIDSet->super_join != pIDSet) ? "joined" : "not joined",
        pIDSet->new_var, pIDSet->identity_sets ? pIDSet->identity_sets->size() : 0, pIDSet->clone_identity);
    if (pIDSet->new_var) thisAgent->symbolManager->symbol_remove_ref(&pIDSet->new_var);
    if (pIDSet->identity_sets) delete pIDSet->identity_sets;
    pIDSet->dirty = false;
    pIDSet->super_join = pIDSet;
    pIDSet->identity_sets = NULL;
    pIDSet->new_var = NULL;
    pIDSet->clone_identity = NULL_IDENTITY_SET;
    pIDSet->literalized = false;
    pIDSet->operational_cond = NULL;
    pIDSet->operational_field = NO_ELEMENT;
}

void Explanation_Based_Chunker::queue_identity_set_deallocation(identity_set* pID_Set)
{
    dprint(DT_DEALLOCATE_ID_SETS, "Added identity set %u to global identity set deletion queue...\n", pID_Set->identity);
    identity_sets_to_delete.insert(pID_Set);
};

void Explanation_Based_Chunker::deallocate_queued_identity_sets(){
    identity_set* lID_Set;
    dprint(DT_DEALLOCATE_ID_SETS, "Deallocating global identity set deletion queue...\n");

    for (auto it = identity_sets_to_delete.begin(); it != identity_sets_to_delete.end(); it++)
    {
        lID_Set = (*it);
        deallocate_identity_set(lID_Set, IDS_test_dealloc);
    }
    identity_sets_to_delete.clear();
};

void Explanation_Based_Chunker::clean_up_identity_sets()
{
    dprint(DT_DEALLOCATE_ID_SETS, "Cleaning up transient data in all %d identity sets in clean-up list\n", identity_sets_to_clean_up.size());
    for (auto it = identity_sets_to_clean_up.begin(); it != identity_sets_to_clean_up.end(); it++)
    {
        identity_set* lJoin_set = *it;
        assert(lJoin_set->dirty);
        if (lJoin_set->dirty) clean_up_identity_set_transient(lJoin_set);
    }
    identity_sets_to_clean_up.clear();
}

void Explanation_Based_Chunker::store_variablization_in_identity_set(identity_set* pIdentitySet, Symbol* variable, Symbol* pMatched_sym)
{
    pIdentitySet->super_join->new_var = variable;
    variable->var->instantiated_sym = pMatched_sym;

    increment_counter(variablization_identity_counter);
    pIdentitySet->super_join->clone_identity = variablization_identity_counter;

    touch_identity_set(pIdentitySet);

//        thisAgent->explanationMemory->add_identity_set_mapping(instantiation_being_built->i_id, IDS_base_instantiation, pIdentitySet, lVarInfo->identity);
}

void Explanation_Based_Chunker::update_identity_set_clone_id(identity_set* pIdentitySet)
{
    increment_counter(variablization_identity_counter);
    pIdentitySet->super_join->clone_identity = variablization_identity_counter;
    touch_identity_set(pIdentitySet);
}

void Explanation_Based_Chunker::join_identity_sets(identity_set* lFromJoinSet, identity_set* lToJoinSet)
{
    lFromJoinSet = lFromJoinSet->super_join;
    lToJoinSet = lToJoinSet->super_join;

    ebc_timers->variablization_rhs->start();
    ebc_timers->variablization_rhs->stop();
    ebc_timers->identity_unification->start();

    touch_identity_set(lFromJoinSet);
    touch_identity_set(lToJoinSet);

    dprint(DT_UNIFY_IDENTITY_SETS, "Combining two join sets for %u and %u...\n", lFromJoinSet->super_join->identity, lToJoinSet->super_join->identity);

    /* Check size and swap if necessary to favor growing the bigger join set */
    uint64_t lFromSize = lFromJoinSet->identity_sets ? lFromJoinSet->identity_sets->size() : 0;
    uint64_t lToSize = lToJoinSet->identity_sets ? lToJoinSet->identity_sets->size() : 0;
    if (lFromSize > lToSize)
    {
        dprint(DT_UNIFY_IDENTITY_SETS, "Swapping join sets so that %u is target and not %u\n", lFromJoinSet->super_join->identity, lToJoinSet->super_join->identity);
        identity_set* tempJoin = lFromJoinSet;
        lFromJoinSet = lToJoinSet;
        lToJoinSet = tempJoin;
    }

    if (!lToJoinSet->identity_sets)
    {
        lToJoinSet->identity_sets = new identity_set_list();
    }

    /* Iterate through identity sets in lFromJoinSet and set their super join set point to lToJoinSet */
    identity_set* lPreviouslyJoinedIdentity;
    if (lFromJoinSet->identity_sets)
    {
        for (auto it = lFromJoinSet->identity_sets->begin(); it != lFromJoinSet->identity_sets->end(); it++)
        {
            lPreviouslyJoinedIdentity = *it;
            dprint(DT_UNIFY_IDENTITY_SETS, "Changing previous join set mapping of %u to %u\n", lPreviouslyJoinedIdentity->identity, lFromJoinSet->super_join->identity);
            lPreviouslyJoinedIdentity->super_join = lToJoinSet;
            if (lPreviouslyJoinedIdentity->literalized) lToJoinSet->literalized = true;
        }
        lToJoinSet->identity_sets->splice(lToJoinSet->identity_sets->begin(), (*lFromJoinSet->identity_sets));
        delete lFromJoinSet->identity_sets;
        lFromJoinSet->identity_sets = NULL;
    }
    /* The identity set being joined is not on its child identity_sets list, so we add it to other identity set here*/
    dprint(DT_UNIFY_IDENTITY_SETS, "Changing join set mapping of %u -> %u to %u -> %u\n", lFromJoinSet->identity, lFromJoinSet->super_join->identity, lFromJoinSet->identity, lToJoinSet->identity);
    lToJoinSet->identity_sets->push_back(lFromJoinSet);

    /* Propagate literalization and constraint info */
    if (lFromJoinSet->literalized) lToJoinSet->literalized = true;

    /* Point super_join to joined identity set */
    lFromJoinSet->super_join = lToJoinSet;

    ebc_timers->identity_unification->stop();
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
                        deallocate_identity_set(t->identity_set, IDS_update_test);
                        /* We no longer own our identity set.  A previous test in the rule or a singleton wme own it */
                        t->owns_identity_set = false;
                    }
                    t->identity_set = updated_id_set;
                }
                break;
        }
}
void Explanation_Based_Chunker::update_identity_sets_in_cond(condition* pCond, instantiation* pInst)
{
    update_identity_sets_in_test(pCond->data.tests.id_test, pInst);
    update_identity_sets_in_test(pCond->data.tests.attr_test, pInst);
    update_identity_sets_in_test(pCond->data.tests.value_test, pInst);
}

void Explanation_Based_Chunker::update_identity_sets_in_condlist(condition* pCondTop, instantiation* pInst)
{
    condition* pCond;

    for (pCond = pCondTop; pCond != NIL; pCond = pCond->next)
    {
        if (pCond->type != CONJUNCTIVE_NEGATION_CONDITION)
        {
            update_identity_sets_in_cond(pCond, pInst);
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
        identity_set* updated_id_set = get_or_add_id_set(lPref->identities.id, NULL, &(lPref->owns_identity_set.id));
        if (lPref->identity_sets.id && lOwnedIdentitySet && (lPref->identity_sets.id != updated_id_set)) deallocate_identity_set(lPref->identity_sets.id, IDS_update_pref);
        lPref->identity_sets.id = updated_id_set;
    }
    if (lPref->identities.attr)
    {
        lOwnedIdentitySet = lPref->owns_identity_set.attr;
        identity_set* updated_id_set = get_or_add_id_set(lPref->identities.attr, NULL, &(lPref->owns_identity_set.attr));
        if (lPref->identity_sets.attr && lOwnedIdentitySet && (lPref->identity_sets.attr != updated_id_set)) deallocate_identity_set(lPref->identity_sets.attr, IDS_update_pref);
        lPref->identity_sets.attr = updated_id_set;
    }
    if (lPref->identities.value)
    {
        lOwnedIdentitySet = lPref->owns_identity_set.value;
        identity_set* updated_id_set = get_or_add_id_set(lPref->identities.value, NULL, &(lPref->owns_identity_set.value));
        if (lPref->identity_sets.value && lOwnedIdentitySet && (lPref->identity_sets.value != updated_id_set)) deallocate_identity_set(lPref->identity_sets.value, IDS_update_pref);
        lPref->identity_sets.value = updated_id_set;
    }
    if (lPref->identities.referent)
    {
        lOwnedIdentitySet = lPref->owns_identity_set.referent;
        identity_set* updated_id_set = get_or_add_id_set(lPref->identities.referent, NULL, &(lPref->owns_identity_set.referent));
        if (lPref->identity_sets.referent && lOwnedIdentitySet && (lPref->identity_sets.referent != updated_id_set)) deallocate_identity_set(lPref->identity_sets.referent, IDS_update_pref);
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

