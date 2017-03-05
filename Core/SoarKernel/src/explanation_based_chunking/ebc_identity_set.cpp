#include "ebc_identity_set.h"
#include "ebc.h"

#include "agent.h"
#include "debug_inventories.h"
#include "symbol_manager.h"
#include "dprint.h"

IdentitySet::IdentitySet(agent* myAgent)
{
    thisAgent = myAgent;
    idset_id = thisAgent->explanationBasedChunker->get_new_identity_set_id();
    dirty = false;
    super_join = NULL;
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

IdentitySet::~IdentitySet()
{
    dprint(DT_DEALLOCATE_ID_SETS, "Deallocating identity set %u%s\n", idset_id, dirty ? " (dirty)." : " (not dirty)");
    if (dirty)
    {
        IdentitySetSharedPtr sharedThis = shared_from_this();
        if (super_join != sharedThis)
        {
            dprint(DT_DEALLOCATE_ID_SETS, "...removing identity set %u from super-join %u's identity_sets\n", idset_id, super_join->idset_id);
            super_join->identity_sets->remove(sharedThis);
        }
        if (identity_sets)
        {

            for (auto it = identity_sets->begin(); it != identity_sets->end(); it++)
            {
                IdentitySetWeakPtr lPreviouslyJoinedWeakIdentity(*it);
                IdentitySetSharedPtr lPreviouslyJoinedIdentity = lPreviouslyJoinedWeakIdentity.lock();
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
        dirty = true;
        IdentitySetWeakPtr lNewWP(shared_from_this());
        thisAgent->explanationBasedChunker->identity_sets_to_clean_up.insert(lNewWP);
    }
}

void IdentitySet::clean_up_transient()
{
    dprint(DT_DEALLOCATE_ID_SETS, "...cleaning up transient data in %s identity set %u (%s, var = %y, # id sets = %d, clone id = %u)\n",
        dirty ? "dirty" : "clean", idset_id, (super_join != NULL) ? "joined" : "not joined",
        new_var, identity_sets ? identity_sets->size() : 0, clone_identity);
    if (new_var) thisAgent->symbolManager->symbol_remove_ref(&new_var);
    if (identity_sets) delete identity_sets;
    dirty = false;
    super_join = NULL;
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

    if (super_join != NULL)
    {
        super_join->new_var = variable;
        super_join->clone_identity = thisAgent->explanationBasedChunker->get_new_var_identity_id();
    } else
    {
        new_var = variable;
        clone_identity = thisAgent->explanationBasedChunker->get_new_var_identity_id();
        touch();
    }

//        thisAgent->explanationMemory->add_identity_set_mapping(instantiation_being_built->i_id, IDS_base_instantiation, pIdentitySet, lVarInfo->identity);
}

void IdentitySet::update_clone_id()
{
    clone_identity = thisAgent->explanationBasedChunker->get_new_var_identity_id();
}

void IdentitySet::set_operational_cond(condition* pCond, WME_Field pField)
{
    if (super_join)
    {
        super_join->operational_cond = pCond;
        super_join->operational_field = pField;
    } else
    {
        operational_cond = pCond; touch();
        operational_field = pField;
        touch();
    }
}
