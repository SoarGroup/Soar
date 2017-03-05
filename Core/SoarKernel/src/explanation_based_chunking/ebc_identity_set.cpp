#include "ebc_identity_set.h"
#include "ebc.h"

#include "agent.h"
#include "debug_inventories.h"
#include "dprint.h"

IdentitySet::IdentitySet(agent* myAgent)
{
    thisAgent = myAgent;
    identity = thisAgent->explanationBasedChunker->get_new_identity_set_id();
    dirty = false;
    super_join = NULL;
    identity_sets = NULL;
    new_var = NULL;
    clone_identity = NULL_IDENTITY_SET;
    literalized = false;
    operational_cond = NULL;
    operational_field = NO_ELEMENT;
//    ISI_add(thisAgent, this);

//    break_if_id_matches(identity, 33);
    dprint(DT_DEALLOCATE_ID_SETS, "Created identity set %u.\n", identity);
}
