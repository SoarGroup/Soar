/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/
#include "ebc.h"
#include "ebc_identity_set.h"
#include "ebc_timers.h"

#include "agent.h"
#include "condition.h"
#include "debug_inventories.h"
#include "dprint.h"
#include "symbol_manager.h"
#include "preference.h"
#include "rhs.h"
#include "test.h"

IdentitySet* Explanation_Based_Chunker::make_identity_set(uint64_t pIdentity)
{
    IdentitySet* lID_Set;
    thisAgent->memoryManager->allocate_with_pool(MP_identity_sets, &lID_Set);
    lID_Set->init(thisAgent);

    ISI_add(thisAgent, lID_Set->idset_id);
    dprint(DT_DEALLOCATE_ID_SETS, "Created identity set %u for variable identity %u\n", lID_Set->idset_id, pIdentity);
    return lID_Set;
}

void IdentitySet_remove_ref(agent* thisAgent, IdentitySet* &pID_Set)
{
    if (pID_Set->remove_ref())
    {
        dprint(DT_IDSET_REFCOUNTS, "Dellocating identity set %u\n", pID_Set->get_sub_identity());
        pID_Set->clean_up();
        thisAgent->memoryManager->free_with_pool(MP_identity_sets, pID_Set);
        pID_Set = NULL;
    }
}

void set_pref_identity_set(agent* thisAgent, preference* pPref, WME_Field pField, IdentitySet* pID_Set)
{
    if ((pField == ID_ELEMENT) && pPref->identity_sets.id)
    {
        if (pPref->identity_sets.id == pID_Set) return;
        IdentitySet_remove_ref(thisAgent, pPref->identity_sets.id);
    }
    else if ((pField == ATTR_ELEMENT) && pPref->identity_sets.attr)
    {
        if (pPref->identity_sets.attr == pID_Set) return;
        IdentitySet_remove_ref(thisAgent, pPref->identity_sets.attr);
    }
    else if ((pField == VALUE_ELEMENT) && pPref->identity_sets.value)
    {
        if (pPref->identity_sets.value == pID_Set) return;
        IdentitySet_remove_ref(thisAgent, pPref->identity_sets.value);
    }
    else if ((pField == REFERENT_ELEMENT) && pPref->identity_sets.referent)
    {
        if (pPref->identity_sets.referent == pID_Set) return;
        IdentitySet_remove_ref(thisAgent, pPref->identity_sets.referent);
    }

    if (pID_Set) pID_Set->add_ref();

    if (pField == ID_ELEMENT)               { pPref->identity_sets.id = pID_Set; }
    else if (pField == ATTR_ELEMENT)        { pPref->identity_sets.attr = pID_Set; }
    else if (pField == VALUE_ELEMENT)       { pPref->identity_sets.value = pID_Set; }
    else if (pField == REFERENT_ELEMENT)    { pPref->identity_sets.referent = pID_Set; }
}

void set_test_identity_set(agent* thisAgent, test pTest, IdentitySet* pID_Set)
{
    if (pTest->identity_set == pID_Set) return;
    if (pTest->identity_set) IdentitySet_remove_ref(thisAgent, pTest->identity_set);
    if (pID_Set) pID_Set->add_ref();
    pTest->identity_set = pID_Set;
}

void clear_test_identity_set(agent* thisAgent, test pTest)
{
    if (pTest->identity_set) IdentitySet_remove_ref(thisAgent, pTest->identity_set);
    pTest->identity_set = NULL;
}

IdentitySet* Explanation_Based_Chunker::get_floating_identity_set()
{
    dprint(DT_PROPAGATE_ID_SETS, "Creating floating identity join set for singleton\n");
    IdentitySet* lID_Set = make_identity_set(0);
    lID_Set->add_ref();
    return lID_Set;
}

IdentitySet* Explanation_Based_Chunker::get_id_set_for_identity(uint64_t pID)
{
    auto iter = (*identities_to_id_sets).find(pID);
    if (iter != (*identities_to_id_sets).end()) return iter->second;
    else return NULL_IDENTITY_SET;
}

IdentitySet* Explanation_Based_Chunker::get_or_add_id_set(uint64_t pID, IdentitySet* pIDSet)
{
    auto iter = (*identities_to_id_sets).find(pID);
    if (iter != (*identities_to_id_sets).end())
    {
        IdentitySet* lID_Set = iter->second;
        dprint(DT_PROPAGATE_ID_SETS, "--> Assigning identity set for variable identity %u with identity set %u already used in rule.\n", pID, lID_Set->get_identity());
        return lID_Set;
    }

    #ifndef DONT_PROPAGATE_ID_SETS
    if (pIDSet)
    {
        (*identities_to_id_sets)[pID] = pIDSet;
        dprint(DT_PROPAGATE_ID_SETS, "--> Copying identity set for variable identity %u with propagated parent identity set %u\n", pID, pIDSet->get_identity());
        return pIDSet;
    } else
    #endif
    {
        IdentitySet* newIdentitySet = make_identity_set(pID);
        (*identities_to_id_sets)[pID] = newIdentitySet;
        dprint(DT_PROPAGATE_ID_SETS, "--> No parent identity set.  Creating new identity join set %u for %u\n", newIdentitySet->get_identity(), pID);
        return newIdentitySet;
    }
}

void Explanation_Based_Chunker::clean_up_identity_sets()
{
    IdentitySet* lJoin_set;
    dprint(DT_DEALLOCATE_ID_SETS, "Cleaning up transient data in all %d identity sets in clean-up list\n", identity_sets_to_clean_up.size());
    for (auto it = identity_sets_to_clean_up.begin(); it != identity_sets_to_clean_up.end(); it++)
    {
        lJoin_set = (*it);
        if (lJoin_set)
        {
            assert(lJoin_set->dirty);
            if (lJoin_set->dirty) lJoin_set->clean_up_transient();
        }
    }
    identity_sets_to_clean_up.clear();
}

void Explanation_Based_Chunker::join_identity_sets(IdentitySet* lFromJoinSet, IdentitySet* lToJoinSet)
{
    lFromJoinSet = lFromJoinSet->super_join;
    lToJoinSet = lToJoinSet->super_join;

    if (lFromJoinSet == lToJoinSet) return;

    ebc_timers->variablization_rhs->start();
    ebc_timers->variablization_rhs->stop();
    ebc_timers->identity_unification->start();

    lFromJoinSet->touch();
    lToJoinSet->touch();

    dprint(DT_UNIFY_IDENTITY_SETS, "Combining two join sets for %u and %u...\n", lFromJoinSet->super_join->idset_id, lToJoinSet->super_join->idset_id);

    /* Check size and swap if necessary to favor growing the bigger join set */
    uint64_t lFromSize = lFromJoinSet->identity_sets ? lFromJoinSet->identity_sets->size() : 0;
    uint64_t lToSize = lToJoinSet->identity_sets ? lToJoinSet->identity_sets->size() : 0;
    if (lFromSize > lToSize)
    {
        dprint(DT_UNIFY_IDENTITY_SETS, "Swapping join sets so that %u is target and not %u\n", lFromJoinSet->super_join->idset_id, lToJoinSet->super_join->idset_id);
        IdentitySet* tempJoin = lFromJoinSet;
        lFromJoinSet = lToJoinSet;
        lToJoinSet = tempJoin;
    }

    if (!lToJoinSet->identity_sets)
    {
        lToJoinSet->identity_sets = new identity_set_list();
    }

    /* Iterate through identity sets in lFromJoinSet and set their super join set point to lToJoinSet */
    IdentitySet* lPreviouslyJoinedIdentity;
    if (lFromJoinSet->identity_sets)
    {
        for (auto it = lFromJoinSet->identity_sets->begin(); it != lFromJoinSet->identity_sets->end(); it++)
        {
            lPreviouslyJoinedIdentity = *it;
            dprint(DT_UNIFY_IDENTITY_SETS, "Changing previous join set mapping of %u to %u\n", lPreviouslyJoinedIdentity->idset_id, lFromJoinSet->super_join->idset_id);
            lPreviouslyJoinedIdentity->super_join = lToJoinSet;
            if (lPreviouslyJoinedIdentity->literalized()) lToJoinSet->literalize();
        }
        lToJoinSet->identity_sets->splice(lToJoinSet->identity_sets->begin(), (*lFromJoinSet->identity_sets));
        /* For use when we try using a vector with a new memory allocator that can handle variable size allocations */
        //lToJoinSet->identity_sets->insert(
        //    lToJoinSet->identity_sets->end(),
        //    std::make_move_iterator(lFromJoinSet->identity_sets->begin()),
        //    std::make_move_iterator(lFromJoinSet->identity_sets->end())
        //  );
        delete lFromJoinSet->identity_sets;
        lFromJoinSet->identity_sets = NULL;
    }
    /* The identity set being joined is not on its child identity_sets list, so we add it to other identity set here*/
//    dprint(DT_UNIFY_IDENTITY_SETS, "Changing join set mapping of %u -> %u to %u -> %u\n", lFromJoinSet->idset_id, lFromJoinSet->super_join->idset_id, lFromJoinSet->idset_id, lToJoinSet->idset_id);
    lToJoinSet->identity_sets->push_back(lFromJoinSet);

    /* Propagate literalization and constraint info */
    if (lFromJoinSet->literalized()) lToJoinSet->literalize();

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
                    set_test_identity_set(thisAgent, t, get_id_set_for_identity(t->identity));
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

void Explanation_Based_Chunker::update_identity_sets_in_preferences(preference* lPref, bool is_chunk_inst)
{

    if (lPref->identities.id) set_pref_identity_set(thisAgent, lPref, ID_ELEMENT, get_or_add_id_set(lPref->identities.id, NULL));
    if (lPref->identities.attr) set_pref_identity_set(thisAgent, lPref, ATTR_ELEMENT, get_or_add_id_set(lPref->identities.attr, NULL));
    if (lPref->identities.value) set_pref_identity_set(thisAgent, lPref, VALUE_ELEMENT, get_or_add_id_set(lPref->identities.value, NULL));
    if (lPref->identities.referent) set_pref_identity_set(thisAgent, lPref, REFERENT_ELEMENT, get_or_add_id_set(lPref->identities.referent, NULL));

    if (is_chunk_inst)
    {
        if (lPref->rhs_funcs.id)        update_identities_in_rhs_value(lPref->rhs_funcs.id);
        if (lPref->rhs_funcs.attr)      update_identities_in_rhs_value(lPref->rhs_funcs.attr);
        if (lPref->rhs_funcs.value)     update_identities_in_rhs_value(lPref->rhs_funcs.value);
        if (lPref->rhs_funcs.referent)  update_identities_in_rhs_value(lPref->rhs_funcs.referent);
    } else {
        /* For instantiations we don't deallocate the existing rhs_funcs before replacing them because
         * are created in execute_action() which doesn't have ownership of these rhs functions and
         * previously made copies for the preference that it's creating. We now just moved it here so
         * that it can be done after identity sets have been determined. */
        if (lPref->rhs_funcs.id) lPref->rhs_funcs.id = copy_rhs_value(thisAgent, lPref->rhs_funcs.id, true);
        if (lPref->rhs_funcs.attr) lPref->rhs_funcs.attr = copy_rhs_value(thisAgent, lPref->rhs_funcs.attr, true);
        if (lPref->rhs_funcs.value) lPref->rhs_funcs.value = copy_rhs_value(thisAgent, lPref->rhs_funcs.value, true);
        if (lPref->rhs_funcs.referent) lPref->rhs_funcs.referent = copy_rhs_value(thisAgent, lPref->rhs_funcs.referent, true);
    }
}

