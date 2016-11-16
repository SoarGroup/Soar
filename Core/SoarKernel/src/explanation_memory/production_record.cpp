#include "production_record.h"

#include "condition.h"
#include "dprint.h"
#include "production.h"
#include "rhs.h"
#include "rete.h"
#include "explanation_memory.h"
#include "explanation_memory.h"

production_record::production_record(agent* pAgent, production* pProd)
{
    dprint(DT_EXPLAIN_CACHE, "Creating production record for %y\n.", pProd->name);
    thisAgent = pAgent;
    if(!pProd->p_node)
    {
        lhs_conds = NULL;
        rhs_actions = NULL;
        return;
    }
    condition* bottom;
    p_node_to_conditions_and_rhs(thisAgent, pProd->p_node, NIL, NIL, &lhs_conds, &bottom, &rhs_actions);
}

production_record::~production_record()
{
    if (lhs_conds)
    {
        dprint(DT_EXPLAIN_CACHE, "Deleting production record's conditions and actions.\n");
        deallocate_condition_list(thisAgent, lhs_conds);
        deallocate_action_list(thisAgent, rhs_actions);
    }
}
