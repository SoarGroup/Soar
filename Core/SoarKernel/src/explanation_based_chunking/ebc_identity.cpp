#include "ebc_identity.h"
#include "ebc.h"

#include "agent.h"
#include "symbol_manager.h"

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
    if (dirty)
    {
        if (joined_identity != this)
        {
            joined_identity->merged_identities->remove(this);
        }
        if (merged_identities)
        {

            for (auto it = merged_identities->begin(); it != merged_identities->end(); it++)
            {
                Identity* lPreviouslyJoinedIdentity = (*it);
                lPreviouslyJoinedIdentity->joined_identity = lPreviouslyJoinedIdentity;
            }
        }
        clean_up_transient();
    }
}

void Identity::touch()
{
    if (!dirty)
    {
        dirty = true;
        thisAgent->explanationBasedChunker->identities_to_clean_up.insert(this);
    }
}

void Identity::clean_up_transient()
{
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
}

uint64_t    get_joined_identity_id(Identity* pIdentity)             { if (!pIdentity) return NULL_IDENTIFIER_ID; else return pIdentity->get_identity(); }
Identity*   get_joined_identity(Identity* pIdentity)                { if (!pIdentity) return NULL_IDENTITY_SET; else return pIdentity->joined_identity;}
uint64_t    get_joined_identity_chunk_inst_id(Identity* pIdentity)  { if (!pIdentity) return NULL_IDENTIFIER_ID; else return pIdentity->get_clone_identity();}

