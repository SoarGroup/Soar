#include "ebc.h"
#include "ebc_identity.h"
#include "ebc_timers.h"

#include "agent.h"
#include "condition.h"
#include "debug_inventories.h"
#include "dprint.h"
#include "explanation_memory.h"
#include "symbol_manager.h"
#include "preference.h"
#include "rhs.h"
#include "test.h"

Identity* Explanation_Based_Chunker::create_new_identity(Symbol* pGoal)
{
    Identity* l_identity;
    thisAgent->memoryManager->allocate_with_pool(MP_identity_sets, &l_identity);
    l_identity->init(thisAgent);

    #ifdef EBC_DETAILED_STATISTICS
    thisAgent->explanationMemory->increment_stat_identities_created();
    #endif
    ISI_add(thisAgent, l_identity->idset_id);
    if (thisAgent->explanationMemory->is_any_enabled()) thisAgent->explanationMemory->add_identity(l_identity, pGoal);

    dprint(DT_DEALLOCATE_IDENTITIES, "Created identity set %u for variable identity %u\n", l_identity->idset_id);
    return l_identity;
}

void IdentitySet_remove_ref(agent* thisAgent, Identity* &pIdentity)
{
    if (pIdentity->internal_remove_ref())
    {
        dprint(DT_IDSET_REFCOUNTS, "Dellocating identity set %u\n", pIdentity->get_sub_identity());
        pIdentity->clean_up();
        thisAgent->memoryManager->free_with_pool(MP_identity_sets, pIdentity);
        pIdentity = NULL;
    }
}

void set_pref_identity(agent* thisAgent, preference* pPref, WME_Field pField, Identity* pIdentity)
{
    if ((pField == ID_ELEMENT) && pPref->identities.id)
    {
        if (pPref->identities.id == pIdentity) return;
        IdentitySet_remove_ref(thisAgent, pPref->identities.id);
    }
    else if ((pField == ATTR_ELEMENT) && pPref->identities.attr)
    {
        if (pPref->identities.attr == pIdentity) return;
        IdentitySet_remove_ref(thisAgent, pPref->identities.attr);
    }
    else if ((pField == VALUE_ELEMENT) && pPref->identities.value)
    {
        if (pPref->identities.value == pIdentity) return;
        IdentitySet_remove_ref(thisAgent, pPref->identities.value);
    }
    else if ((pField == REFERENT_ELEMENT) && pPref->identities.referent)
    {
        if (pPref->identities.referent == pIdentity) return;
        IdentitySet_remove_ref(thisAgent, pPref->identities.referent);
    }

    if (pIdentity) pIdentity->add_ref();

    if (pField == ID_ELEMENT)               { pPref->identities.id = pIdentity; }
    else if (pField == ATTR_ELEMENT)        { pPref->identities.attr = pIdentity; }
    else if (pField == VALUE_ELEMENT)       { pPref->identities.value = pIdentity; }
    else if (pField == REFERENT_ELEMENT)    { pPref->identities.referent = pIdentity; }
}

void set_test_identity(agent* thisAgent, test pTest, Identity* pIdentity)
{
    if (pTest->identity == pIdentity) return;
    if (pTest->identity) IdentitySet_remove_ref(thisAgent, pTest->identity);
    if (pIdentity) pIdentity->add_ref();
    pTest->identity = pIdentity;
}

void clear_test_identity(agent* thisAgent, test pTest)
{
    if (pTest->identity) IdentitySet_remove_ref(thisAgent, pTest->identity);
    pTest->identity = NULL;
}

Identity* Explanation_Based_Chunker::get_floating_identity(Symbol* pGoal)
{
    dprint(DT_PROPAGATE, "Creating floating identity join set for singleton\n");
    Identity* l_identity = create_new_identity(pGoal);
    l_identity->add_ref();
    return l_identity;
}

Identity* Explanation_Based_Chunker::get_identity_for_id(uint64_t pID)
{
    auto iter = (*inst_id_to_identity_map).find(pID);
    if (iter != (*inst_id_to_identity_map).end()) return iter->second;
    else return NULL_IDENTITY_SET;
}

Identity* Explanation_Based_Chunker::get_or_add_identity(uint64_t pID, Identity* pIdentity, Symbol* pGoal)
{
    auto iter = (*inst_id_to_identity_map).find(pID);
    if (iter != (*inst_id_to_identity_map).end())
    {
        Identity* l_identity = iter->second;

        #ifdef EBC_DETAILED_STATISTICS
        if (pIdentity) thisAgent->explanationMemory->increment_stat_identity_propagations_blocked();
        #endif

        dprint(DT_PROPAGATE, "--> Assigning identity set for variable identity %u with identity set %u already used in rule.\n", pID, l_identity->get_identity());
        return l_identity;
    }

    #ifndef EBC_DONT_PROPAGATE_IDENTITIES
    if (pIdentity)
    {
        (*inst_id_to_identity_map)[pID] = pIdentity;
        dprint(DT_PROPAGATE, "--> Copying identity set for variable identity %u with propagated parent identity set %u\n", pID, pIdentity->get_identity());
        return pIdentity;
    } else
    #endif
    {
        Identity* newIdentitySet = create_new_identity(pGoal);
        (*inst_id_to_identity_map)[pID] = newIdentitySet;

        #ifdef EBC_DETAILED_STATISTICS
        thisAgent->explanationMemory->increment_stat_identity_propagations();
        #endif
        dprint(DT_PROPAGATE, "--> No parent identity set.  Creating new identity join set %u for %u\n", newIdentitySet->get_identity(), pID);
        return newIdentitySet;
    }
}

void Explanation_Based_Chunker::clean_up_identities()
{
    Identity* lJoin_set;
    dprint(DT_DEALLOCATE_IDENTITIES, "Cleaning up transient data in all %d identity sets in clean-up list\n", identities_to_clean_up.size());
    for (auto it = identities_to_clean_up.begin(); it != identities_to_clean_up.end(); it++)
    {
        lJoin_set = (*it);
        if (lJoin_set) lJoin_set->clean_up_transient();
    }
    identities_to_clean_up.clear();
}

void Explanation_Based_Chunker::join_identities(Identity* lFromJoinSet, Identity* lToJoinSet)
{
    lFromJoinSet = lFromJoinSet->joined_identity;
    lToJoinSet = lToJoinSet->joined_identity;

    if (lFromJoinSet == lToJoinSet) return;

    #ifdef EBC_DETAILED_STATISTICS
    thisAgent->explanationMemory->increment_stat_identities_joined();
    #endif

    ebc_timers->variablization_rhs->start();
    ebc_timers->variablization_rhs->stop();
    ebc_timers->identity_unification->start();

    lFromJoinSet->touch();
    lToJoinSet->touch();

    dprint(DT_UNIFY_IDENTITY_SETS, "Combining two join sets for %u and %u...\n", lFromJoinSet->joined_identity->idset_id, lToJoinSet->joined_identity->idset_id);

    /* Check size and swap if necessary to favor growing the bigger join set */
    uint64_t lFromSize = lFromJoinSet->merged_identities ? lFromJoinSet->merged_identities->size() : 0;
    uint64_t lToSize = lToJoinSet->merged_identities ? lToJoinSet->merged_identities->size() : 0;
    if (lFromSize > lToSize)
    {
        dprint(DT_UNIFY_IDENTITY_SETS, "Swapping join sets so that %u is target and not %u\n", lFromJoinSet->joined_identity->idset_id, lToJoinSet->joined_identity->idset_id);
        Identity* tempJoin = lFromJoinSet;
        lFromJoinSet = lToJoinSet;
        lToJoinSet = tempJoin;
    }

    if (!lToJoinSet->merged_identities)
    {
        lToJoinSet->merged_identities = new identity_list();
    }

    /* Iterate through identity sets in lFromJoinSet and set their super join set point to lToJoinSet */
    Identity* lPreviouslyJoinedIdentity;
    if (lFromJoinSet->merged_identities)
    {
        for (auto it = lFromJoinSet->merged_identities->begin(); it != lFromJoinSet->merged_identities->end(); it++)
        {
            lPreviouslyJoinedIdentity = *it;
            dprint(DT_UNIFY_IDENTITY_SETS, "Changing previous join set mapping of %u to %u\n", lPreviouslyJoinedIdentity->idset_id, lFromJoinSet->joined_identity->idset_id);
            lPreviouslyJoinedIdentity->joined_identity = lToJoinSet;
            if (lPreviouslyJoinedIdentity->literalized()) lToJoinSet->literalize();
        }
        lToJoinSet->merged_identities->splice(lToJoinSet->merged_identities->begin(), (*lFromJoinSet->merged_identities));
        /* For use when we try using a vector with a new memory allocator that can handle variable size allocations */
        //lToJoinSet->identity_sets->insert(
        //    lToJoinSet->identity_sets->end(),
        //    std::make_move_iterator(lFromJoinSet->identity_sets->begin()),
        //    std::make_move_iterator(lFromJoinSet->identity_sets->end())
        //  );
        delete lFromJoinSet->merged_identities;
        lFromJoinSet->merged_identities = NULL;
    }
    /* The identity set being joined is not on its child identity_sets list, so we add it to other identity set here*/
//    dprint(DT_UNIFY_IDENTITY_SETS, "Changing join set mapping of %u -> %u to %u -> %u\n", lFromJoinSet->idset_id, lFromJoinSet->super_join->idset_id, lFromJoinSet->idset_id, lToJoinSet->idset_id);
    lToJoinSet->merged_identities->push_back(lFromJoinSet);

    /* Propagate literalization and constraint info */
    if (lFromJoinSet->literalized()) lToJoinSet->literalize();

    /* Point super_join to joined identity set */
    lFromJoinSet->joined_identity = lToJoinSet;

    ebc_timers->identity_unification->stop();
}

void Explanation_Based_Chunker::update_identities_in_test(test t, instantiation* pInst)
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
                    update_identities_in_test(static_cast<test>(c->first), pInst);
                }
                break;
            default:
                if (t->inst_identity)
                {
                    set_test_identity(thisAgent, t, get_identity_for_id(t->inst_identity));
                }
                break;
        }
}
void Explanation_Based_Chunker::update_identities_in_cond(condition* pCond, instantiation* pInst)
{
    update_identities_in_test(pCond->data.tests.id_test, pInst);
    update_identities_in_test(pCond->data.tests.attr_test, pInst);
    update_identities_in_test(pCond->data.tests.value_test, pInst);
}

void Explanation_Based_Chunker::update_identities_in_condlist(condition* pCondTop, instantiation* pInst)
{
    condition* pCond;

    for (pCond = pCondTop; pCond != NIL; pCond = pCond->next)
    {
        if (pCond->type != CONJUNCTIVE_NEGATION_CONDITION)
        {
            update_identities_in_cond(pCond, pInst);
        } else {
            update_identities_in_condlist(pCond->data.ncc.top, pInst);
        }
    }
}

void Explanation_Based_Chunker::update_identities_in_preferences(preference* lPref, Symbol* pGoal, bool is_chunk_inst)
{

    if (lPref->inst_identities.id) set_pref_identity(thisAgent, lPref, ID_ELEMENT, get_or_add_identity(lPref->inst_identities.id, NULL, pGoal));
    if (lPref->inst_identities.attr) set_pref_identity(thisAgent, lPref, ATTR_ELEMENT, get_or_add_identity(lPref->inst_identities.attr, NULL, pGoal));
    if (lPref->inst_identities.value) set_pref_identity(thisAgent, lPref, VALUE_ELEMENT, get_or_add_identity(lPref->inst_identities.value, NULL, pGoal));
    if (lPref->inst_identities.referent) set_pref_identity(thisAgent, lPref, REFERENT_ELEMENT, get_or_add_identity(lPref->inst_identities.referent, NULL, pGoal));

    if (is_chunk_inst)
    {
        if (lPref->rhs_func_inst_identities.id)        update_identities_in_rhs_value(lPref->rhs_func_inst_identities.id);
        if (lPref->rhs_func_inst_identities.attr)      update_identities_in_rhs_value(lPref->rhs_func_inst_identities.attr);
        if (lPref->rhs_func_inst_identities.value)     update_identities_in_rhs_value(lPref->rhs_func_inst_identities.value);
        if (lPref->rhs_func_inst_identities.referent)  update_identities_in_rhs_value(lPref->rhs_func_inst_identities.referent);
    } else {
        /* For instantiations we don't deallocate the existing rhs_funcs before replacing them because
         * are created in execute_action() which doesn't have ownership of these rhs functions and
         * previously made copies for the preference that it's creating. We now just moved it here so
         * that it can be done after identity sets have been determined. */
        rhs_value lNewRHSValue;
        if (lPref->rhs_func_inst_identities.id)
        {
            lNewRHSValue = copy_rhs_value(thisAgent, lPref->rhs_func_inst_identities.id, true);
            deallocate_rhs_value(thisAgent, lPref->rhs_func_inst_identities.id);
            lPref->rhs_func_inst_identities.id = lNewRHSValue;
        }
        if (lPref->rhs_func_inst_identities.attr)
        {
            lNewRHSValue = copy_rhs_value(thisAgent, lPref->rhs_func_inst_identities.attr, true);
            deallocate_rhs_value(thisAgent, lPref->rhs_func_inst_identities.attr);
            lPref->rhs_func_inst_identities.attr = lNewRHSValue;
        }
        if (lPref->rhs_func_inst_identities.value)
        {
            lNewRHSValue = copy_rhs_value(thisAgent, lPref->rhs_func_inst_identities.value, true);
            deallocate_rhs_value(thisAgent, lPref->rhs_func_inst_identities.value);
            lPref->rhs_func_inst_identities.value = lNewRHSValue;
        }
        if (lPref->rhs_func_inst_identities.referent)
        {
            lNewRHSValue = copy_rhs_value(thisAgent, lPref->rhs_func_inst_identities.referent, true);
            deallocate_rhs_value(thisAgent, lPref->rhs_func_inst_identities.referent);
            lPref->rhs_func_inst_identities.referent = lNewRHSValue;
        }
    }
}

