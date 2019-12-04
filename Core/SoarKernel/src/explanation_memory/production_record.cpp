#include "production_record.h"

#include "condition.h"
#include "production.h"
#include "rhs.h"
#include "rete.h"
#include "explanation_memory.h"
#include "explanation_memory.h"

void production_record::init(agent* pAgent, production* pProd)
{
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

void production_record::clean_up()
{
    if (lhs_conds)
    {
        deallocate_condition_list(thisAgent, lhs_conds);
        deallocate_action_list(thisAgent, rhs_actions);
    }
}
