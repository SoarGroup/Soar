/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/*************************************************************************
 *
 *  file:  episodic_memory.h
 *
 * =======================================================================
 */

#ifndef EPISODIC_MEMORY_H
#define EPISODIC_MEMORY_H

#include <portability.h>

#include <map>
#include <list>
#include <stack>
#include <set>
#include <queue>

#include "sqlite3.h"

#include "soar_module.h"

using namespace soar_module;

typedef union symbol_union Symbol;
typedef struct wme_struct wme;

//////////////////////////////////////////////////////////
// EpMem Capacity
//
// There's an inherent problem in the storage capacity
// difference between the EpMem code, the Soar kernel
// (mainly Symbols), the STL, and SQLite.  Integer
// symbols are currently 32-bit. STL depends upon
// compiler options.  SQLite can go either way.
//
// portability.h defines EPMEM_64 (because epmem types
// are scattered throughout the kernel).  Here we use it
// to define the appropriate sqlite function names.
//
//////////////////////////////////////////////////////////

#ifdef EPMEM_64

#define EPMEM_SQLITE_BIND_INT sqlite3_bind_int64
#define EPMEM_SQLITE_COLUMN_INT sqlite3_column_int64

#else

#define EPMEM_SQLITE_BIND_INT sqlite3_bind_int
#define EPMEM_SQLITE_COLUMN_INT sqlite3_column_int

#endif

//////////////////////////////////////////////////////////
// EpMem Experimentation
//
// If defined, we hijack the main EpMem function
// for tight-loop experimentation/timing.
//
//////////////////////////////////////////////////////////

//#define EPMEM_EXPERIMENT


//////////////////////////////////////////////////////////
// EpMem Constants
//////////////////////////////////////////////////////////

// statements
// 0 - 19 => common
// 20 - ( EPMEM_MAX_STATEMENTS - 1 ) => mode
#define EPMEM_STMT_BEGIN							0
#define EPMEM_STMT_COMMIT							1
#define EPMEM_STMT_ROLLBACK							2
#define EPMEM_STMT_VAR_GET							3
#define EPMEM_STMT_VAR_SET							4

#define EPMEM_STMT_RIT_ADD_LEFT						5
#define EPMEM_STMT_RIT_TRUNCATE_LEFT				6
#define EPMEM_STMT_RIT_ADD_RIGHT					7
#define EPMEM_STMT_RIT_TRUNCATE_RIGHT				8

#define EPMEM_STMT_GET_HASH							9
#define EPMEM_STMT_ADD_HASH							10

#define EPMEM_STMT_ONE_ADD_TIME						20
#define EPMEM_STMT_ONE_ADD_NODE_RANGE				21
#define EPMEM_STMT_ONE_ADD_NODE_UNIQUE				22
#define EPMEM_STMT_ONE_FIND_NODE_UNIQUE				23
#define EPMEM_STMT_ONE_FIND_IDENTIFIER				24
#define EPMEM_STMT_ONE_VALID_EPISODE				25
#define EPMEM_STMT_ONE_NEXT_EPISODE					26
#define EPMEM_STMT_ONE_PREV_EPISODE					27
#define EPMEM_STMT_ONE_GET_EPISODE					28
#define EPMEM_STMT_ONE_ADD_NODE_NOW					29
#define EPMEM_STMT_ONE_DELETE_NODE_NOW				30
#define EPMEM_STMT_ONE_ADD_NODE_POINT				31

#define EPMEM_STMT_THREE_ADD_TIME					20
#define EPMEM_STMT_THREE_ADD_NODE_NOW				21
#define EPMEM_STMT_THREE_DELETE_NODE_NOW			22
#define EPMEM_STMT_THREE_ADD_EDGE_NOW				23
#define EPMEM_STMT_THREE_DELETE_EDGE_NOW			24
#define EPMEM_STMT_THREE_ADD_NODE_POINT				25
#define EPMEM_STMT_THREE_ADD_EDGE_POINT				26
#define EPMEM_STMT_THREE_ADD_NODE_RANGE				27
#define EPMEM_STMT_THREE_ADD_EDGE_RANGE				28
#define EPMEM_STMT_THREE_FIND_NODE_UNIQUE			29
#define EPMEM_STMT_THREE_ADD_NODE_UNIQUE			30
#define EPMEM_STMT_THREE_FIND_EDGE_UNIQUE			31
#define EPMEM_STMT_THREE_FIND_EDGE_UNIQUE_SHARED	32
#define EPMEM_STMT_THREE_ADD_EDGE_UNIQUE			33
#define EPMEM_STMT_THREE_VALID_EPISODE				34
#define EPMEM_STMT_THREE_NEXT_EPISODE				35
#define EPMEM_STMT_THREE_PREV_EPISODE				36
#define EPMEM_STMT_THREE_GET_NODES					37
#define EPMEM_STMT_THREE_GET_EDGES					38

#define EPMEM_MAX_STATEMENTS 						40 // must be at least 1+ largest of any STMT constant

// variables (rit vars must be same as stat versions)
enum epmem_variable_key
{
	var_rit_offset_1, var_rit_leftroot_1, var_rit_rightroot_1, var_rit_minstep_1,
	var_rit_offset_2, var_rit_leftroot_2, var_rit_rightroot_2, var_rit_minstep_2,
	var_mode, var_next_id
};

// algorithm constants
#define EPMEM_DB_CLOSED								-1 // initialize db_status to this (sqlite error codes are positive)
#define EPMEM_DB_PREP_STR_MAX						-1 // non-zero nByte param indicates to read to zero terminator

#define EPMEM_MEMID_NONE							-1
#define EPMEM_NODEID_ROOT							0

#define EPMEM_NODE_POS								0
#define EPMEM_NODE_NEG								1
#define EPMEM_RANGE_START							0
#define EPMEM_RANGE_END								1
#define EPMEM_RANGE_EP								0
#define EPMEM_RANGE_NOW								1
#define EPMEM_RANGE_POINT							2

#define EPMEM_RIT_ROOT								0
#define EPMEM_RIT_OFFSET_INIT						-1
#define EPMEM_LN_2									0.693147180559945

#define EPMEM_DNF									2

#define EPMEM_RIT_STATE_NODE						0
#define EPMEM_RIT_STATE_EDGE						1


//////////////////////////////////////////////////////////
// EpMem Parameters
//////////////////////////////////////////////////////////

class epmem_path_param;
class epmem_graph_match_param;
class epmem_mode_param;

class epmem_param_container: public param_container
{
	public:
		enum db_choices { memory, file };
		enum mode_choices { tree, graph };
		enum phase_choices { phase_output, phase_selection };
		enum trigger_choices { none, output, dc };
		enum force_choices { remember, ignore, force_off };

		boolean_param *learning;
		constant_param<db_choices> *database;
		epmem_path_param *path;
		integer_param *commit;

		epmem_mode_param *mode;
		epmem_graph_match_param *graph_match;

		constant_param<phase_choices> *phase;
		constant_param<trigger_choices> *trigger;
		constant_param<force_choices> *force;
		decimal_param *balance;
		set_param *exclusions;
		constant_param<timer::timer_level> *timers;

		epmem_param_container( agent *new_agent );
};

class epmem_path_param: public string_param
{
	protected:
		agent *my_agent;

	public:
		epmem_path_param( const char *new_name, const char *new_value, predicate<const char *> *new_val_pred, predicate<const char *> *new_prot_pred, agent *new_agent );
		virtual void set_value( const char *new_value );
};

class epmem_graph_match_param: public boolean_param
{
	protected:
		agent *my_agent;

	public:
		epmem_graph_match_param( const char *new_name, boolean new_value, predicate<boolean> *new_prot_pred, agent *new_agent );
		virtual bool validate_string( const char *new_string );
};

class epmem_mode_param: public constant_param<epmem_param_container::mode_choices>
{
	protected:
		agent *my_agent;

	public:
		epmem_mode_param( const char *new_name, epmem_param_container::mode_choices new_value, predicate<epmem_param_container::mode_choices> *new_prot_pred, agent *new_agent );
		virtual void set_value( epmem_param_container::mode_choices new_value );
};

template <typename T>
class epmem_db_predicate: public agent_predicate<T>
{
	public:
		epmem_db_predicate( agent *new_agent );
		bool operator() ( T val );
};


//////////////////////////////////////////////////////////
// EpMem Statistics
//////////////////////////////////////////////////////////

class epmem_mem_usage_stat;
class epmem_mem_high_stat;

class epmem_stat_container: public stat_container
{
	public:
		integer_stat *time;
		epmem_mem_usage_stat *mem_usage;
		epmem_mem_high_stat *mem_high;
		integer_stat *ncb_wmes;

		integer_stat *qry_pos;
		integer_stat *qry_neg;
		integer_stat *qry_ret;
		integer_stat *qry_card;
		integer_stat *qry_lits;

		integer_stat *next_id;

		integer_stat *rit_offset_1;
		integer_stat *rit_left_root_1;
		integer_stat *rit_right_root_1;
		integer_stat *rit_min_step_1;

		integer_stat *rit_offset_2;
		integer_stat *rit_left_root_2;
		integer_stat *rit_right_root_2;
		integer_stat *rit_min_step_2;

		epmem_stat_container( agent *my_agent );
};

class epmem_mem_usage_stat: public integer_stat
{
	public:
		epmem_mem_usage_stat( const char *new_name, long new_value, predicate<long> *new_prot_pred );
		long get_value();
};

//

class epmem_mem_high_stat: public integer_stat
{
	public:
		epmem_mem_high_stat( const char *new_name, long new_value, predicate<long> *new_prot_pred );
		long get_value();
};


//////////////////////////////////////////////////////////
// EpMem Timers
//////////////////////////////////////////////////////////

class epmem_timer_container: public timer_container
{
	public:
		timer *total;
		timer *storage;
		timer *ncb_retrieval;
		timer *query;
		timer *api;
		timer *trigger;
		timer *init;
		timer *next;
		timer *prev;

		timer *ncb_edge;
		timer *ncb_edge_rit;
		timer *ncb_node;
		timer *ncb_node_rit;

		timer *query_dnf;
		timer *query_graph_match;
		timer *query_pos_start_ep;
		timer *query_pos_start_now;
		timer *query_pos_start_point;
		timer *query_pos_end_ep;
		timer *query_pos_end_now;
		timer *query_pos_end_point;
		timer *query_neg_start_ep;
		timer *query_neg_start_now;
		timer *query_neg_start_point;
		timer *query_neg_end_ep;
		timer *query_neg_end_now;
		timer *query_neg_end_point;

		epmem_timer_container( agent *my_agent );
};

class epmem_timer_level_predicate: public agent_predicate<timer::timer_level>
{
	public:
		epmem_timer_level_predicate( agent *new_agent );
		bool operator() ( timer::timer_level val );
};

class epmem_timer: public timer
{
	public:
		epmem_timer( const char *new_name, agent *new_agent, timer_level new_level );
};


//

void epmem_start_timer( agent *my_agent, long timer );
void epmem_stop_timer( agent *my_agent, long timer );


//////////////////////////////////////////////////////////
// Common Types
//////////////////////////////////////////////////////////

// represents a unique node identifier in the episodic store
typedef unsigned long epmem_node_id;

// represents a unique episode identifier in the episodic store
typedef long epmem_time_id;

// represents a vector of times
typedef std::vector<epmem_time_id> epmem_time_list;

// keeping state for multiple RIT's
typedef struct epmem_rit_state_param_struct
{
	soar_module::integer_stat *stat;
	epmem_variable_key var_key;
} epmem_rit_state_param;

typedef struct epmem_rit_state_struct
{
	epmem_rit_state_param offset;
	epmem_rit_state_param leftroot;
	epmem_rit_state_param rightroot;
	epmem_rit_state_param minstep;

	long add_query;
	soar_module::timer *timer;
} epmem_rit_state;

//////////////////////////////////////////////////////////
// Soar Integration Types
//////////////////////////////////////////////////////////

// data associated with each state
typedef struct epmem_data_struct
{
	unsigned long last_ol_time;		// last update to output-link
	unsigned long last_ol_count;	// last count of output-link

	unsigned long last_cmd_time;	// last update to epmem.command
	unsigned long last_cmd_count;	// last update to epmem.command

	epmem_time_id last_memory;		// last retrieved memory

	std::set<wme *> *cue_wmes;		// wmes in last cue
	std::stack<wme *> *epmem_wmes;	// wmes in last epmem
} epmem_data;


//////////////////////////////////////////////////////////
// Mode "one" Types (i.e. Working Memory Tree)
//////////////////////////////////////////////////////////

// represents a leaf wme
typedef struct epmem_leaf_node_struct
{
	double leaf_weight;						// wma value
	epmem_node_id leaf_id;					// node id
} epmem_leaf_node;

// maintains state within sqlite b-trees
typedef struct epmem_range_query_struct
{
	sqlite3_stmt *stmt;						// sqlite query
	epmem_time_id val;						// current b-tree leaf value

	double weight;							// wma value
	long ct;								// cardinality w.r.t. positive/negative query

	soar_module::timer *timer;				// timer to update upon executing the query
} epmem_range_query;

// functor to maintain a priority cue of b-tree pointers
// based upon their current value
struct epmem_compare_range_queries
{
	bool operator() ( const epmem_range_query *a, const epmem_range_query *b ) const
	{
		return ( a->val < b->val );
	}
};
typedef std::priority_queue<epmem_range_query *, std::vector<epmem_range_query *>, epmem_compare_range_queries> epmem_range_query_list;


//////////////////////////////////////////////////////////
// Mode "three" Types (i.e. Working Memory Graph)
//////////////////////////////////////////////////////////

// see below
typedef struct epmem_shared_literal_struct epmem_shared_literal;
typedef std::vector<epmem_shared_literal *> epmem_shared_literal_list;
typedef std::list<epmem_shared_literal_list::size_type> epmem_shared_wme_list;

// lookup tables to facilitate shared identifiers
typedef std::map<epmem_node_id, Symbol *> epmem_id_mapping;
typedef std::map<epmem_node_id, epmem_shared_literal *> epmem_literal_mapping;

// lookup table to propagate constrained identifiers during
// full graph-match
typedef std::map<Symbol *, epmem_node_id> epmem_constraint_list;
typedef std::map<epmem_node_id, Symbol *> epmem_reverse_constraint_list;

// types/structures to facilitate re-use of identifiers
typedef std::map<epmem_node_id, epmem_node_id> epmem_id_pool;
typedef std::map<long, epmem_id_pool *> epmem_hashed_id_pool;
typedef std::map<epmem_node_id, epmem_hashed_id_pool *> epmem_parent_id_pool;
typedef std::map<epmem_node_id, epmem_id_pool *> epmem_return_id_pool;
typedef struct epmem_id_reservation_struct
{
	epmem_node_id my_id;
	long my_hash;
	epmem_id_pool *my_pool;
} epmem_id_reservation;

// represents a graph edge (i.e. identifier)
// follows cs theory notation of finite automata: q1 = d( q0, w )
typedef struct epmem_edge_struct
{
	epmem_node_id q0;						// id
	Symbol *w;								// attr
	epmem_node_id q1;						// value
} epmem_edge;

// represents cached children of an identifier in working memory
typedef struct epmem_wme_cache_element_struct
{
	wme **wmes;								// child wmes
	unsigned long len;			// number of children
	unsigned long parents;		// number of parents

	epmem_literal_mapping *lits;			// child literals
} epmem_wme_cache_element;

// represents state of a leaf wme
// at a particular episode
typedef struct epmem_shared_match_struct
{
	double value_weight;					// wma value
	long value_ct;				// cardinality w.r.t. positive/negative query

	unsigned long ct;				// number of contributing literals that are "on"
} epmem_shared_match;

// represents a list of literals grouped
// sequentially by cue wme
typedef struct epmem_shared_literal_group_struct
{
	epmem_shared_literal_list *literals;	// vector of sequentially grouped literals
	epmem_shared_wme_list *wmes;			// list of indexes to the start of wmes

	wme *c_wme;								// current wme (used during building and using groups)
} epmem_shared_literal_group;

// represents state of one historical
// identity of a cue wme at a particular
// episode
struct epmem_shared_literal_struct
{
	epmem_node_id shared_id;				// shared q1, if identifier

	unsigned long ct;				// number of contributing literals that are "on"
	unsigned long max;			// number of contributing literals that *need* to be on

	struct wme_struct *wme;					// associated cue wme
	bool wme_kids;							// does the cue wme have children (indicative of leaf wme status)

	epmem_shared_match *match;				// associated match, if leaf wme
	epmem_shared_literal_group *children;	// grouped child literals, if not leaf wme
};

// maintains state within sqlite b-trees
typedef struct epmem_shared_query_struct
{
	sqlite3_stmt *stmt;						// associated sqlite query
	epmem_time_id val;						// current b-tree leaf value
	soar_module::timer *timer;				// timer to update upon executing the query

	epmem_shared_literal_list *triggers;	// literals to update when stepping this b-tree
} epmem_shared_query;

// functor to maintain a priority cue of b-tree pointers
// based upon their current value
struct epmem_compare_shared_queries
{
	bool operator() ( const epmem_shared_query *a, const epmem_shared_query *b ) const
	{
		return ( a->val < b->val );
	}
};
typedef std::priority_queue<epmem_shared_query *, std::vector<epmem_shared_query *>, epmem_compare_shared_queries> epmem_shared_query_list;

//
// These must go below types
//

#include "stl_support.h"

//////////////////////////////////////////////////////////
// Parameter Functions (see cpp for comments)
//////////////////////////////////////////////////////////

// shortcut for determining if EpMem is enabled
extern bool epmem_enabled( agent *my_agent );


//////////////////////////////////////////////////////////
// Soar Functions (see cpp for comments)
//////////////////////////////////////////////////////////

// init, end
extern void epmem_reset( agent *my_agent, Symbol *state = NULL );
extern void epmem_close( agent *my_agent );

// perform epmem actions
extern void epmem_go( agent *my_agent );

#endif
