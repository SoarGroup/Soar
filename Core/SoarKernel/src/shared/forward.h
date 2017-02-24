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
typedef struct chunk_cond_struct chunk_cond;
typedef struct condition_struct condition;
typedef struct cons_struct cons;
typedef struct constraint_struct constraint;
typedef struct dl_cons_struct dl_cons;
typedef struct exploration_parameter_struct exploration_parameter;
typedef struct gds_struct goal_dependency_set;
typedef struct hash_table_struct hash_table;
typedef struct identity_join_struct identity_join;
typedef struct instantiation_struct instantiation;
typedef struct io_wme_struct io_wme;
typedef struct memory_pool_struct memory_pool;
typedef struct ms_change_struct ms_change;
typedef struct multi_attributes_struct multi_attribute;
typedef struct node_varnames_struct node_varnames;
typedef struct pi_struct parent_inst;
typedef struct preference_struct preference;
typedef struct production_struct production;
typedef struct rete_node_struct rete_node;
typedef struct rete_test_struct rete_test;
typedef struct rhs_function_struct rhs_function;
typedef struct saved_test_struct saved_test;
typedef struct select_info_struct select_info;
typedef struct slot_struct slot;
typedef struct symbol_struct Symbol;
typedef struct chunk_element_struct chunk_element;
typedef struct test_struct test_info;
typedef struct trace_mode_info_struct trace_mode_info;

typedef unsigned char byte;
typedef byte ms_trace_type;
typedef byte wme_trace_type;
typedef char varnames;
typedef char* rhs_value;
typedef cons cons;
typedef signed short goal_stack_level;
typedef test_info* test;
typedef uint64_t tc_number;
typedef unsigned short rete_node_level;

class Soar_Instance;
class Memory_Manager;
class Symbol_Manager;

class SoarDecider;
class WM_Manager;
class wma_param_container;
class wma_stat_container;
class wma_timer_container;
typedef uint64_t wma_d_cycle;
typedef uint64_t wma_reference;
typedef struct wma_decay_element_struct wma_decay_element;
typedef struct wme_struct wme;

class Output_Manager;
class GraphViz_Visualizer;
class Viz_Parameters;
class OM_Parameters;
class OM_DB;
class AgentOutput_Info;

class cli_command_params;
class decide_param_container;
class load_param_container;
class save_param_container;
class memory_param_container;
class production_param_container;
class wm_param_container;

class Explanation_Based_Chunker;
class ebc_timer_container;
class Repair_Path;
class Explanation_Memory;
class action_record;
class chunk_record;
class chunk_record;
class condition_record;
class ebc_param_container;
class identity_record;
class instantiation_record;
class production_record;

class EpMem_Manager;
class epmem_common_statement_container;
class epmem_graph_statement_container;
class epmem_param_container;
class epmem_stat_container;
class epmem_timer_container;
typedef int64_t epmem_node_id;  // represents a unique node identifier in the episodic store
typedef uint64_t epmem_hash_id; // represents a unique temporal hash in the episodic store
typedef uint64_t epmem_time_id; // represents a unique episode identifier in the episodic store

class RL_Manager;
class rl_param_container;
class rl_stat_container;

class SMem_Manager;
class MathQuery;
class smem_db_lib_version_stat;
class smem_mem_high_stat;
class smem_mem_usage_stat;
class smem_param_container;
class smem_path_param;
class smem_stat_container;
class smem_statement_container;
class smem_timer_container;
class smem_timer_level_predicate;
struct smem_compare_weighted_cue_elements;
struct smem_compare_activated_lti;
struct ltm_value_const;
struct ltm_value_lti;
typedef struct ltm_object_struct ltm_object;
typedef union ltm_value_union ltm_value;
typedef uint64_t smem_hash_id;  // represents a temporal hash
typedef struct smem_weighted_cue_element_struct smem_weighted_cue_element;

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

/* Three utility functions to help break when a soar symbol or wme is encountered.  If you set
 * your debugger to always break on these, you can just add them to stop Soar at the point where
 * problem structures are being processed.  Since we can throw these in anywhere, we're forward
 * declaring them here so that we don't need to include debug.h everywhere.*/
extern bool break_if_wme_matches_string(wme *w, const char* match_id, const char* match_attr, const char* match_value);
extern bool break_if_pref_matches_string(preference *w, const char* match_id, const char* match_attr, const char* match_value);
extern bool break_if_symbol_matches_string(Symbol* sym, const char* match);
extern bool break_if_id_matches(uint64_t lID, uint64_t lID_to_match);
extern bool break_if_test_symbol_matches_string(test t, const char* match);
extern void debug_refcount_change_start(agent* thisAgent, const char* symString, bool twoPart);
extern void debug_refcount_change_end(agent* thisAgent, const char* symString, const char* callerString, bool twoPart);

#endif /* CORE_SOARKERNEL_SRC_SHARED_FORWARD_H_ */
