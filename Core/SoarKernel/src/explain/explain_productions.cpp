#include "explain.h"
//#include "agent.h"
#include "condition.h"
//#include "debug.h"
//#include "instantiation.h"
//#include "preference.h"
#include "production.h"
#include "rhs.h"
#include "rete.h"
//#include "symbol.h"
//#include "test.h"
//#include "output_manager.h"
//#include "working_memory.h"

production_record::production_record(agent* pAgent, production* pProd)
{
    assert(pProd->p_node);
    condition* bottom;
    thisAgent = pAgent;
    p_node_to_conditions_and_rhs(thisAgent, pProd->p_node, NIL, NIL, &lhs_conds, &bottom, &rhs_actions);
}

production_record::~production_record()
{
    deallocate_condition_list(thisAgent, lhs_conds);
    deallocate_action_list(thisAgent, rhs_actions);

}
