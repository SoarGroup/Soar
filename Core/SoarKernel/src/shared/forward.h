/*
 * forward.h
 *
 *  Created on: Dec 29, 2015
 *      Author: mazzin
 */

#ifndef CORE_SOARKERNEL_SRC_SHARED_FORWARD_H_
#define CORE_SOARKERNEL_SRC_SHARED_FORWARD_H_

typedef char* rhs_value;
typedef signed short goal_stack_level;
typedef unsigned short rete_node_level;
typedef uint64_t tc_number;
typedef struct action_struct action;
typedef struct agent_struct agent;
typedef struct chunk_cond_struct chunk_cond;
typedef struct condition_struct condition;
typedef struct cons_struct cons;
typedef cons list;
typedef struct instantiation_struct instantiation;
typedef struct ms_change_struct ms_change;
typedef struct node_varnames_struct node_varnames;
typedef struct preference_struct preference;
typedef struct production_struct production;
typedef struct rete_node_struct rete_node;
typedef struct slot_struct slot;
typedef struct symbol_struct Symbol;
typedef struct test_struct test_info;
typedef test_info* test;
typedef struct wme_struct wme;

class Output_Manager;
class Explanation_Logger;

namespace soar_module
{
    typedef struct symbol_triple_struct symbol_triple;
    typedef struct identity_triple_struct identity_triple;
    typedef struct rhs_triple_struct rhs_triple;
}
extern void print(agent* thisAgent, const char* format, ...);


#endif /* CORE_SOARKERNEL_SRC_SHARED_FORWARD_H_ */
