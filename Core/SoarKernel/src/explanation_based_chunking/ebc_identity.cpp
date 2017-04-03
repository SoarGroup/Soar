#include "ebc_identity.h"
#include "ebc.h"

#include "agent.h"
#include "debug_inventories.h"
#include "symbol_manager.h"
#include "dprint.h"

void Identity::init(agent* myAgent)
{
    thisAgent = myAgent;
    idset_id = thisAgent->explanationBasedChunker->get_new_identity_id();
    dirty = false;
    joined_identity = this;
    merged_identities = NULL;
    new_var = NULL;
    chunk_inst_identity = LITERAL_VALUE;
    m_literalized = false;
    operational_cond = NULL;
    operational_field = NO_ELEMENT;
    refcount = 0;
}

void Identity::clean_up()
{
    dprint(DT_DEALLOCATE_IDENTITIES, "Cleaning up identity set %u%s for deallocation...\n", idset_id, dirty ? " (dirty)." : " (not dirty)");
    if (dirty)
    {
        if (joined_identity != this)
        {
            dprint(DT_DEALLOCATE_IDENTITIES, "...removing identity set %u from super-join %u's identity_sets\n", idset_id, joined_identity->idset_id);
            joined_identity->merged_identities->remove(this);
            /* For use when we try using a vector with a new memory allocator that can handle variable size allocations */
            //identity_list::iterator newEnd = std::remove(joined_identity->merged_identities->begin(), joined_identity->merged_identities->end(), this);
            //joined_identity->identity_sets->erase(newEnd, joined_identity->merged_identities->end());
        }
        if (merged_identities)
        {

            for (auto it = merged_identities->begin(); it != merged_identities->end(); it++)
            {
                Identity* lPreviouslyJoinedIdentity = (*it);
                dprint(DT_DEALLOCATE_IDENTITIES, "...resetting previous join set mapping of %u to %u\n", lPreviouslyJoinedIdentity->idset_id, idset_id);
                lPreviouslyJoinedIdentity->joined_identity = lPreviouslyJoinedIdentity;
            }
        }
        clean_up_transient();
    }
    ISI_remove(thisAgent, idset_id);
}

void Identity::touch()
{
    if (!dirty)
    {
        dprint(DT_DEALLOCATE_IDENTITIES, "...marking identity set %u as dirty (superjoin = %u, %s)\n", idset_id, joined_identity->idset_id, joined_identity->dirty ? "dirty" : "clean");
        dirty = true;
        thisAgent->explanationBasedChunker->identities_to_clean_up.insert(this);
    }
}

void Identity::clean_up_transient()
{
    dprint(DT_DEALLOCATE_IDENTITIES, "...cleaning up transient data in %s identity set %u (%s, var = %y, # id sets = %d, clone id = %u)\n",
        dirty ? "dirty" : "clean", idset_id, (joined_identity != this) ? "joined" : "not joined",
        new_var, merged_identities ? merged_identities->size() : 0, chunk_inst_identity);
    if (new_var) thisAgent->symbolManager->symbol_remove_ref(&new_var);
    if (merged_identities) delete merged_identities;
    dirty = false;
    joined_identity = this;
    merged_identities = NULL;
    new_var = NULL;
    chunk_inst_identity = LITERAL_VALUE;
    m_literalized = false;
    operational_cond = NULL;
    operational_field = NO_ELEMENT;
}

void Identity::store_variablization(Symbol* variable, Symbol* pMatched_sym)
{
    variable->var->instantiated_sym = pMatched_sym;

    joined_identity->new_var = variable;
    joined_identity->chunk_inst_identity = thisAgent->explanationBasedChunker->get_new_inst_identity_id();
    joined_identity->touch();
}

uint64_t Identity::update_clone_id()
{
    if (!joined_identity->chunk_inst_identity)
    {
        joined_identity->chunk_inst_identity = thisAgent->explanationBasedChunker->get_new_inst_identity_id();
        joined_identity->touch();
    }
    return joined_identity->chunk_inst_identity;
}

void Identity::set_operational_cond(condition* pCond, WME_Field pField)
{
        joined_identity->operational_cond = pCond;
        joined_identity->operational_field = pField;
        joined_identity->touch();
    dprint(DT_CONSTRAINTS, "Setting operational condition for identity set %u to %s element of %l\n", get_identity(), field_to_string(pField), pCond);
}

uint64_t        get_joined_identity_id(Identity* pIdentity)        { if (!pIdentity) return NULL_IDENTITY_SET; else return pIdentity->get_identity(); }
Identity*    get_joined_identity(Identity* pIdentity)       { if (!pIdentity) return NULL; else return pIdentity->joined_identity;}
uint64_t        get_joined_identity_chunk_inst_id(Identity* pIdentity)  { if (!pIdentity) return NULL_IDENTITY_SET; else return pIdentity->get_clone_identity();}

