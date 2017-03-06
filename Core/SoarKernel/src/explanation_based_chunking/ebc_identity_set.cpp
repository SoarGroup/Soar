#include "ebc_identity_set.h"
#include "ebc.h"

#include "agent.h"
#include "debug_inventories.h"
#include "symbol_manager.h"
#include "dprint.h"

void IdentitySet::init(agent* myAgent)
{
    thisAgent = myAgent;
    idset_id = thisAgent->explanationBasedChunker->get_new_identity_set_id();
    dirty = false;
    super_join = this;
    identity_sets = NULL;
    new_var = NULL;
    clone_identity = LITERAL_VALUE;
    m_literalized = false;
    operational_cond = NULL;
    operational_field = NO_ELEMENT;
    ISI_add(thisAgent, idset_id);

//    break_if_id_matches(identity, 33);
    dprint(DT_DEALLOCATE_ID_SETS, "Created identity set %u.\n", idset_id);
}

void IdentitySet::clean_up()
{
    dprint(DT_DEALLOCATE_ID_SETS, "Cleaning up identity set %u%s for deallocation...\n", idset_id, dirty ? " (dirty)." : " (not dirty)");
    if (dirty)
    {
//        IdentitySetSharedPtr sharedThis = shared_from_this();
//        if (super_join != sharedThis)
        if (super_join != this)
        {
            dprint(DT_DEALLOCATE_ID_SETS, "...removing identity set %u from super-join %u's identity_sets\n", idset_id, super_join->idset_id);
//            super_join->identity_sets->remove(sharedThis);
            super_join->identity_sets->remove(this);
        }
        if (identity_sets)
        {

            for (auto it = identity_sets->begin(); it != identity_sets->end(); it++)
            {
//                IdentitySetWeakPtr lPreviouslyJoinedWeakIdentity(*it);
//                IdentitySetSharedPtr lPreviouslyJoinedIdentity = lPreviouslyJoinedWeakIdentity.lock();
                IdentitySetSharedPtr lPreviouslyJoinedIdentity = (*it);
                dprint(DT_DEALLOCATE_ID_SETS, "...resetting previous join set mapping of %u to %u\n", lPreviouslyJoinedIdentity->idset_id, idset_id);
                lPreviouslyJoinedIdentity->super_join = lPreviouslyJoinedIdentity;
            }
        }
        clean_up_transient();
    }
    ISI_remove(thisAgent, idset_id);
}

void IdentitySet::touch()
{
    if (!dirty)
    {
        dprint(DT_DEALLOCATE_ID_SETS, "...marking identity set %u as dirty (superjoin = %u, %s)\n", idset_id, super_join->idset_id, super_join->dirty ? "dirty" : "clean");
        dirty = true;
//        IdentitySetWeakPtr lNewWP(shared_from_this());
//        thisAgent->explanationBasedChunker->identity_sets_to_clean_up.insert(lNewWP);
        thisAgent->explanationBasedChunker->identity_sets_to_clean_up.insert(this);
    }
}

void IdentitySet::clean_up_transient()
{
    dprint(DT_DEALLOCATE_ID_SETS, "...cleaning up transient data in %s identity set %u (%s, var = %y, # id sets = %d, clone id = %u)\n",
        dirty ? "dirty" : "clean", idset_id, (super_join != this) ? "joined" : "not joined",
        new_var, identity_sets ? identity_sets->size() : 0, clone_identity);
    if (new_var) thisAgent->symbolManager->symbol_remove_ref(&new_var);
    if (identity_sets) delete identity_sets;
    dirty = false;
    super_join = this;
    identity_sets = NULL;
    new_var = NULL;
    clone_identity = LITERAL_VALUE;
    m_literalized = false;
    operational_cond = NULL;
    operational_field = NO_ELEMENT;
}

void IdentitySet::store_variablization(Symbol* variable, Symbol* pMatched_sym)
{
    variable->var->instantiated_sym = pMatched_sym;

    super_join->new_var = variable;
    super_join->clone_identity = thisAgent->explanationBasedChunker->get_new_var_identity_id();
    super_join->touch();
//        thisAgent->explanationMemory->add_identity_set_mapping(instantiation_being_built->i_id, IDS_base_instantiation, pIdentitySet, lVarInfo->identity);
}

void IdentitySet::update_clone_id()
{
    if (!super_join->clone_identity)
    {
        super_join->clone_identity = thisAgent->explanationBasedChunker->get_new_var_identity_id();
        super_join->touch();
    }
}

void IdentitySet::set_operational_cond(condition* pCond, WME_Field pField)
{
        super_join->operational_cond = pCond;
        super_join->operational_field = pField;
        super_join->touch();
    dprint(DT_CONSTRAINTS, "Setting operational condition for identity set %u to %s element of %l\n", get_identity(), field_to_string(pField), pCond);
}
uint64_t                get_joined_identity_id(IdentitySetSharedPtr pID_Set) { if (!pID_Set) return NULL_IDENTITY_SET; else return pID_Set->get_identity(); }
IdentitySetSharedPtr    get_joined_identity_set(IdentitySetSharedPtr pID_Set)    { if (!pID_Set) return NULL; else return pID_Set->super_join;}
uint64_t                get_joined_identity_clone_id(IdentitySetSharedPtr pID_Set)    { if (!pID_Set) return NULL_IDENTITY_SET; else return pID_Set->get_clone_identity();}

