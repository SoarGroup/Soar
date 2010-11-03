/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/*************************************************************************
 *
 *  file:  semantic_memory.h
 *
 * =======================================================================
 */

#ifndef SEMANTIC_MEMORY_H
#define SEMANTIC_MEMORY_H

#include <portability.h>

#include <stack>
#include <set>
#include <list>
#include <vector>
#include <queue>

#include "soar_module.h"
#include "soar_db.h"

//////////////////////////////////////////////////////////
// SMem Experimentation
//
// If defined, we hijack the main SMem function
// for tight-loop experimentation/timing.
//
//////////////////////////////////////////////////////////

//#define SMEM_EXPERIMENT


//////////////////////////////////////////////////////////
// SMem Parameters
//////////////////////////////////////////////////////////

class smem_path_param;

class smem_param_container: public soar_module::param_container
{
	public:
		enum db_choices { memory, file };
		enum cache_choices { cache_S, cache_M, cache_L };
		enum opt_choices { opt_safety, opt_speed };

		enum merge_choices { merge_none, merge_add };

		soar_module::boolean_param *learning;
		soar_module::constant_param<db_choices> *database;
		smem_path_param *path;
		soar_module::boolean_param *lazy_commit;

		soar_module::constant_param<soar_module::timer::timer_level> *timers;

		soar_module::constant_param<cache_choices> *cache;
		soar_module::constant_param<opt_choices> *opt;

		soar_module::integer_param *thresh;

		soar_module::constant_param<merge_choices>* merge;

		smem_param_container( agent *new_agent );
};

class smem_path_param: public soar_module::string_param
{
	protected:
		agent *my_agent;

	public:
		smem_path_param( const char *new_name, const char *new_value, soar_module::predicate<const char *> *new_val_pred, soar_module::predicate<const char *> *new_prot_pred, agent *new_agent );
		virtual void set_value( const char *new_value );
};

template <typename T>
class smem_db_predicate: public soar_module::agent_predicate<T>
{
	public:
		smem_db_predicate( agent *new_agent );
		bool operator() ( T val );
};


//////////////////////////////////////////////////////////
// SMem Statistics
//////////////////////////////////////////////////////////

class smem_mem_usage_stat;
class smem_mem_high_stat;

class smem_stat_container: public soar_module::stat_container
{
	public:
		smem_mem_usage_stat *mem_usage;
		smem_mem_high_stat *mem_high;

		soar_module::integer_stat *expansions;
		soar_module::integer_stat *cbr;
		soar_module::integer_stat *stores;

		soar_module::integer_stat *chunks;
		soar_module::integer_stat *slots;

		smem_stat_container( agent *my_agent );
};

class smem_mem_usage_stat: public soar_module::integer_stat
{
	protected:
		agent *my_agent;

	public:
		smem_mem_usage_stat( agent *new_agent, const char *new_name, int64_t new_value, soar_module::predicate<int64_t> *new_prot_pred );
		int64_t get_value();
};

//

class smem_mem_high_stat: public soar_module::integer_stat
{
	protected:
		agent *my_agent;

	public:
		smem_mem_high_stat( agent *new_agent, const char *new_name, int64_t new_value, soar_module::predicate<int64_t> *new_prot_pred );
		int64_t get_value();
};


//////////////////////////////////////////////////////////
// SMem Timers
//////////////////////////////////////////////////////////

class smem_timer_container: public soar_module::timer_container
{
	public:
		soar_module::timer *total;
		soar_module::timer *storage;
		soar_module::timer *ncb_retrieval;
		soar_module::timer *query;
		soar_module::timer *api;
		soar_module::timer *init;
		soar_module::timer *hash;
		soar_module::timer *act;

		smem_timer_container( agent *my_agent );
};

class smem_timer_level_predicate: public soar_module::agent_predicate<soar_module::timer::timer_level>
{
	public:
		smem_timer_level_predicate( agent *new_agent );
		bool operator() ( soar_module::timer::timer_level val );
};

class smem_timer: public soar_module::timer
{
	public:
		smem_timer( const char *new_name, agent *new_agent, soar_module::timer::timer_level new_level );
};


//////////////////////////////////////////////////////////
// SMem Statements
//////////////////////////////////////////////////////////

class smem_statement_container: public soar_module::sqlite_statement_container
{
	public:
		soar_module::sqlite_statement *begin;
		soar_module::sqlite_statement *commit;
		soar_module::sqlite_statement *rollback;

		soar_module::sqlite_statement *var_get;
		soar_module::sqlite_statement *var_set;
		soar_module::sqlite_statement *var_create;

		soar_module::sqlite_statement *hash_rev_int;
		soar_module::sqlite_statement *hash_rev_float;
		soar_module::sqlite_statement *hash_rev_str;
		soar_module::sqlite_statement *hash_get_int;
		soar_module::sqlite_statement *hash_get_float;
		soar_module::sqlite_statement *hash_get_str;
		soar_module::sqlite_statement *hash_add_type;
		soar_module::sqlite_statement *hash_add_int;
		soar_module::sqlite_statement *hash_add_float;
		soar_module::sqlite_statement *hash_add_str;

		soar_module::sqlite_statement *lti_add;
		soar_module::sqlite_statement *lti_get;
		soar_module::sqlite_statement *lti_letter_num;
		soar_module::sqlite_statement *lti_max;

		soar_module::sqlite_statement *web_add;
		soar_module::sqlite_statement *web_truncate;
		soar_module::sqlite_statement *web_expand;

		soar_module::sqlite_statement *web_attr_ct;
		soar_module::sqlite_statement *web_const_ct;
		soar_module::sqlite_statement *web_lti_ct;

		soar_module::sqlite_statement *web_attr_all;
		soar_module::sqlite_statement *web_const_all;
		soar_module::sqlite_statement *web_lti_all;

		soar_module::sqlite_statement *web_attr_child;
		soar_module::sqlite_statement *web_const_child;
		soar_module::sqlite_statement *web_lti_child;

		soar_module::sqlite_statement *ct_attr_check;
		soar_module::sqlite_statement *ct_const_check;
		soar_module::sqlite_statement *ct_lti_check;

		soar_module::sqlite_statement *ct_attr_add;
		soar_module::sqlite_statement *ct_const_add;
		soar_module::sqlite_statement *ct_lti_add;

		soar_module::sqlite_statement *ct_attr_update;
		soar_module::sqlite_statement *ct_const_update;
		soar_module::sqlite_statement *ct_lti_update;

		soar_module::sqlite_statement *ct_attr_get;
		soar_module::sqlite_statement *ct_const_get;
		soar_module::sqlite_statement *ct_lti_get;

		soar_module::sqlite_statement *act_set;
		soar_module::sqlite_statement *act_lti_child_ct_set;
		soar_module::sqlite_statement *act_lti_child_ct_get;
		soar_module::sqlite_statement *act_lti_set;
		soar_module::sqlite_statement *act_lti_get;

		soar_module::sqlite_statement *vis_lti;
		soar_module::sqlite_statement *vis_value_const;
		soar_module::sqlite_statement *vis_value_lti;

		smem_statement_container( agent *new_agent );
};


//////////////////////////////////////////////////////////
// Soar Constants
//////////////////////////////////////////////////////////

enum smem_variable_key
{
	var_max_cycle, var_num_nodes, var_num_edges, var_act_thresh
};

#define SMEM_ACT_MAX static_cast<uint64_t>( static_cast<uint64_t>( 0 - 1 ) / static_cast<uint64_t>(2) )

#define SMEM_LTI_UNKNOWN_LEVEL 0

#define SMEM_WEB_NULL 0
#define SMEM_WEB_NULL_STR "0"

// provides a distinct prefix to be used by all
// tables for two reasons:
// - distinguish from other modules
// - distinguish between smem versions
#define SMEM_SCHEMA "smem3_"

// empty table used to verify proper structure
#define SMEM_SIGNATURE SMEM_SCHEMA "signature"

//////////////////////////////////////////////////////////
// Soar Integration Types
//////////////////////////////////////////////////////////

// represents the unique identification of a
// long-term identifier
typedef uint64_t smem_lti_id;

// represents a temporal hash
typedef uint64_t smem_hash_id;

// represents a collection of long-term identifiers
typedef std::list<smem_lti_id> smem_lti_list;
typedef std::set<smem_lti_id> smem_lti_set;

// a list of symbols
typedef std::list<Symbol *> smem_sym_list;

// ways to store an identifier
enum smem_storage_type { store_level, store_recursive };

// represents a list of wmes
typedef std::list<wme *> smem_wme_list;

// data associated with each state
typedef struct smem_data_struct
{
	uint64_t last_cmd_time[2];			// last update to smem.command
	uint64_t last_cmd_count[2];		// last update to smem.command

	std::set<wme *> *cue_wmes;				// wmes in last cue
	std::stack<preference *> *smem_wmes;	// wmes in last smem
} smem_data;

//

enum smem_cue_element_type { attr_t, value_const_t, value_lti_t };

typedef struct smem_weighted_cue_element_struct
{
	uint64_t weight;

	struct wme_struct *cue_element;
	smem_hash_id attr_hash;
	smem_hash_id value_hash;
	smem_lti_id value_lti;

	smem_cue_element_type element_type;

} smem_weighted_cue_element;

struct smem_compare_weighted_cue_elements
{
	bool operator() ( const smem_weighted_cue_element *a, const smem_weighted_cue_element *b ) const
	{
		return ( a->weight > b->weight );
	}
};

typedef std::priority_queue<smem_weighted_cue_element *, std::vector<smem_weighted_cue_element *>, smem_compare_weighted_cue_elements> smem_prioritized_weighted_cue;
typedef std::list<smem_weighted_cue_element *> smem_weighted_cue_list;

typedef std::pair< int64_t, smem_lti_id > smem_activated_lti;

struct smem_compare_activated_lti
{
	bool operator() ( const smem_activated_lti a, const smem_activated_lti b ) const
	{
		return ( b.first > a.first );
	}
};

typedef std::priority_queue< smem_activated_lti, std::vector<smem_activated_lti>, smem_compare_activated_lti> smem_prioritized_activated_lti_queue;

//

typedef struct smem_chunk_struct smem_chunk;
typedef std::set<smem_chunk *> smem_chunk_set;
typedef union smem_chunk_value_union smem_chunk_value;
typedef std::list<smem_chunk_value *> smem_slot;
typedef std::map<Symbol *, smem_slot *> smem_slot_map;

struct smem_chunk_struct
{
	Symbol *soar_id;
	smem_lti_id lti_id;

	char lti_letter;
	uint64_t lti_number;

	smem_slot_map *slots;
};

struct smem_chunk_value_constant
{
	smem_cue_element_type val_type;
	Symbol *val_value;
};

struct smem_chunk_value_lti
{
	smem_cue_element_type val_type;
	smem_chunk *val_value;
};

union smem_chunk_value_union
{
	struct smem_chunk_value_constant val_const;
	struct smem_chunk_value_lti val_lti;
};

typedef std::map<std::string, smem_chunk *> smem_str_to_chunk_map;
typedef std::map<Symbol *, smem_chunk *> smem_sym_to_chunk_map;

//

typedef struct smem_vis_lti_struct
{
	public:
		smem_lti_id lti_id;
		std::string lti_name;
		unsigned int level;
} smem_vis_lti;

//

enum smem_query_levels { qry_search, qry_full };


//////////////////////////////////////////////////////////
// Soar Functions (see cpp for comments)
//////////////////////////////////////////////////////////

extern bool smem_enabled( agent *my_agent );
extern void smem_attach( agent *my_agent );

extern bool smem_parse_chunks( agent *my_agent, const char *chunks, std::string **err_msg );

extern void smem_visualize_store( agent *my_agent, std::string *return_val );
extern void smem_visualize_lti( agent *my_agent, smem_lti_id lti_id, unsigned int depth, std::string *return_val );

typedef struct condition_struct condition;
typedef struct action_struct action;

extern Bool smem_count_ltis( agent *my_agent, void *item, void *userdata );
extern bool smem_valid_production( condition *lhs_top, action *rhs_top );

extern smem_lti_id smem_lti_get_id( agent *my_agent, char name_letter, uint64_t name_number );
extern Symbol *smem_lti_soar_make( agent *my_agent, smem_lti_id lti, char name_letter, uint64_t name_number, goal_stack_level level );

extern void smem_reset( agent *my_agent, Symbol *state );
extern void smem_reset_id_counters( agent *my_agent );
extern void smem_close( agent *my_agent );

// perform smem actions
extern void smem_go( agent *my_agent, bool store_only );

#endif
