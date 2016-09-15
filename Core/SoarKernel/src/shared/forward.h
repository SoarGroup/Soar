/*
 * forward.h
 *
 *  Created on: Dec 29, 2015
 *      Author: mazzin
 */

#ifndef CORE_SOARKERNEL_SRC_SHARED_FORWARD_H_
#define CORE_SOARKERNEL_SRC_SHARED_FORWARD_H_

typedef struct action_struct action;
typedef struct agent_struct agent;
typedef struct backtrace_struct backtrace_str;
typedef unsigned char byte;
typedef struct chunk_cond_struct chunk_cond;
typedef struct condition_struct condition;
typedef struct cons_struct cons;
typedef cons list;
typedef struct dl_cons_struct dl_cons;
typedef struct exploration_parameter_struct exploration_parameter;
typedef signed short goal_stack_level;
typedef struct hash_table_struct hash_table;
typedef struct instantiation_struct instantiation;
typedef struct io_wme_struct io_wme;
typedef struct memory_pool_struct memory_pool;
typedef struct ms_change_struct ms_change;
typedef byte ms_trace_type;
typedef struct multi_attributes_struct multi_attribute;
typedef struct node_varnames_struct node_varnames;
typedef struct pi_struct parent_inst;
typedef struct preference_struct preference;
typedef struct production_struct production;
typedef struct rete_node_struct rete_node;
typedef unsigned short rete_node_level;
typedef struct rete_test_struct rete_test;
typedef struct rhs_function_struct rhs_function;
typedef char* rhs_value;
typedef struct saved_test_struct saved_test;
typedef struct select_info_struct select_info;
typedef struct slot_struct slot;
typedef struct symbol_struct Symbol;
typedef uint64_t tc_number;
typedef struct test_struct test_info;
typedef test_info* test;
typedef struct trace_mode_info_struct trace_mode_info;
typedef char varnames;
typedef uint64_t wma_d_cycle;
typedef struct wma_decay_element_struct wma_decay_element;
typedef uint64_t wma_reference;
typedef struct wme_struct wme;
typedef byte wme_trace_type;
typedef struct symbol_with_match_struct symbol_with_match;

typedef int64_t epmem_node_id;  // represents a unique node identifier in the episodic store
typedef uint64_t epmem_hash_id; // represents a unique temporal hash in the episodic store
typedef uint64_t epmem_time_id; // represents a unique episode identifier in the episodic store
typedef uint64_t smem_lti_id;   // represents the unique identification of a long-term identifier
typedef uint64_t smem_hash_id;  // represents a temporal hash

typedef struct smem_weighted_cue_element_struct smem_weighted_cue_element;
typedef struct smem_chunk_struct smem_chunk;
struct smem_compare_weighted_cue_elements;
struct smem_compare_activated_lti;
struct smem_chunk_value_constant;
struct smem_chunk_value_lti;
typedef union smem_chunk_value_union smem_chunk_value;

class Output_Manager;
class Symbol_Manager;
class WM_Manager;
class EpMem_Manager;
class SMem_Manager;
class MathQuery;
class RL_Manager;
class Explanation_Based_Chunker;
class Memory_Manager;
class Explanation_Memory;
class Soar_Instance;
class AgentOutput_Info;
class GraphViz_Visualizer;
class debug_param_container;
class OM_Parameters;
class OM_DB;
class LTI_Promotion_Set;
class wma_param_container;
class wma_stat_container;
class wma_timer_container;
class rl_param_container;
class rl_stat_container;
class smem_timer_container;
class smem_stat_container;
class smem_param_container;
class smem_statement_container;
class epmem_param_container;
class epmem_stat_container;
class epmem_timer_container;
class epmem_common_statement_container;
class epmem_graph_statement_container;
class ebc_param_container;
class chunk_record;
class instantiation_record;
class chunk_record;
class condition_record;
class action_record;
class production_record;
class identity_record;

namespace sml
{
    class Kernel;
    class Agent;
    class AgentSML;
}

namespace soar_module {
    class sqlite_database;
    class sqlite_statement;
    class timer;
}
namespace soar {
    class Lexer;
    class Lexeme;
}

namespace cli
{
    class CommandLineInterface;

}

#endif /* CORE_SOARKERNEL_SRC_SHARED_FORWARD_H_ */
