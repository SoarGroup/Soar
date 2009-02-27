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
#define EPMEM_RETURN_LONG 0.1
#define EPMEM_RETURN_STRING ""

// parameters
// - protected are [ DB, MODE ]
#define EPMEM_PARAM_LEARNING						0
#define EPMEM_PARAM_DB								1
#define EPMEM_PARAM_COMMIT							2
#define EPMEM_PARAM_PATH							3
#define EPMEM_PARAM_MODE							4
#define EPMEM_PARAM_GRAPH_MATCH						5
#define EPMEM_PARAM_PHASE							6
#define EPMEM_PARAM_TRIGGER							7
#define EPMEM_PARAM_FORCE							8
#define EPMEM_PARAM_BALANCE							9
#define EPMEM_PARAM_EXCLUSIONS						10
#define EPMEM_PARAM_TIMERS							11
#define EPMEM_PARAMS								12 // must be 1+ last epmem param

// parameter settings
#define EPMEM_LEARNING_ON 1
#define EPMEM_LEARNING_OFF 2

#define EPMEM_DB_MEM 1
#define EPMEM_DB_FILE 2

#define EPMEM_MODE_ONE 1   // tree
#define EPMEM_MODE_THREE 3 // graph

#define EPMEM_GRAPH_MATCH_OFF 1
#define EPMEM_GRAPH_MATCH_ON 2

#define EPMEM_TRIGGER_NONE 1
#define EPMEM_TRIGGER_OUTPUT 2
#define EPMEM_TRIGGER_DC 3

#define EPMEM_PHASE_OUTPUT 1
#define EPMEM_PHASE_SELECTION 2

#define EPMEM_FORCE_REMEMBER 1
#define EPMEM_FORCE_IGNORE 2
#define EPMEM_FORCE_OFF 3

#define EPMEM_TIMERS_OFF 1
#define EPMEM_TIMERS_ONE 2
#define EPMEM_TIMERS_TWO 3
#define EPMEM_TIMERS_THREE 4

// statistics
// * = protected
#define EPMEM_STAT_TIME								0 // *
#define EPMEM_STAT_MEM_USAGE						1
#define EPMEM_STAT_MEM_HIGH							2
#define EPMEM_STAT_NCB_WMES							3
#define EPMEM_STAT_QRY_POS							4
#define EPMEM_STAT_QRY_NEG							5
#define EPMEM_STAT_QRY_RET							6
#define EPMEM_STAT_QRY_CARD							7
#define EPMEM_STAT_QRY_LITS							8

#define EPMEM_STAT_NEXT_ID							9

#define EPMEM_STAT_RIT_OFFSET_1						10 // *
#define EPMEM_STAT_RIT_LEFTROOT_1					11 // *
#define EPMEM_STAT_RIT_RIGHTROOT_1					12 // *
#define EPMEM_STAT_RIT_MINSTEP_1					13 // *

#define EPMEM_STAT_RIT_OFFSET_2						14 // *
#define EPMEM_STAT_RIT_LEFTROOT_2					15 // *
#define EPMEM_STAT_RIT_RIGHTROOT_2					16 // *
#define EPMEM_STAT_RIT_MINSTEP_2					17 // *

#define EPMEM_STATS									18 // must be 1+ last epmem stat

// timers
#define EPMEM_TIMER_TOTAL							0
#define EPMEM_TIMER_STORAGE							1
#define EPMEM_TIMER_NCB_RETRIEVAL					2
#define EPMEM_TIMER_QUERY							3
#define EPMEM_TIMER_API								4
#define EPMEM_TIMER_TRIGGER							5
#define EPMEM_TIMER_INIT							6
#define EPMEM_TIMER_NEXT							7
#define EPMEM_TIMER_PREV							8
#define EPMEM_TIMER_NCB_EDGE						9
#define EPMEM_TIMER_NCB_EDGE_RIT					10
#define EPMEM_TIMER_NCB_NODE						11
#define EPMEM_TIMER_NCB_NODE_RIT					12
#define EPMEM_TIMER_QUERY_DNF						13
#define EPMEM_TIMER_QUERY_GRAPH_MATCH				14
#define EPMEM_TIMER_QUERY_POS_START_EP				15
#define EPMEM_TIMER_QUERY_POS_START_NOW				16
#define EPMEM_TIMER_QUERY_POS_START_POINT			17
#define EPMEM_TIMER_QUERY_POS_END_EP				18
#define EPMEM_TIMER_QUERY_POS_END_NOW				19
#define EPMEM_TIMER_QUERY_POS_END_POINT				20
#define EPMEM_TIMER_QUERY_NEG_START_EP				21
#define EPMEM_TIMER_QUERY_NEG_START_NOW				22
#define EPMEM_TIMER_QUERY_NEG_START_POINT			23
#define EPMEM_TIMER_QUERY_NEG_END_EP				24
#define EPMEM_TIMER_QUERY_NEG_END_NOW				25
#define EPMEM_TIMER_QUERY_NEG_END_POINT				26

#define EPMEM_TIMERS								27 // must be 1+ last epmem timer

// statements
// 0 - 9 => common
// 10 - ( EPMEM_MAX_STATEMENTS - 1 ) => mode
#define EPMEM_STMT_BEGIN							0
#define EPMEM_STMT_COMMIT							1
#define EPMEM_STMT_ROLLBACK							2
#define EPMEM_STMT_VAR_GET							3
#define EPMEM_STMT_VAR_SET							4

#define EPMEM_STMT_RIT_ADD_LEFT						5
#define EPMEM_STMT_RIT_TRUNCATE_LEFT				6
#define EPMEM_STMT_RIT_ADD_RIGHT					7
#define EPMEM_STMT_RIT_TRUNCATE_RIGHT				8

#define EPMEM_STMT_ONE_ADD_TIME						10
#define EPMEM_STMT_ONE_ADD_NODE_RANGE				11
#define EPMEM_STMT_ONE_ADD_NODE_UNIQUE				12
#define EPMEM_STMT_ONE_FIND_NODE_UNIQUE				13
#define EPMEM_STMT_ONE_FIND_IDENTIFIER				14
#define EPMEM_STMT_ONE_VALID_EPISODE				15
#define EPMEM_STMT_ONE_NEXT_EPISODE					16
#define EPMEM_STMT_ONE_PREV_EPISODE					17
#define EPMEM_STMT_ONE_GET_EPISODE					18
#define EPMEM_STMT_ONE_ADD_NODE_NOW					19
#define EPMEM_STMT_ONE_DELETE_NODE_NOW				20
#define EPMEM_STMT_ONE_ADD_NODE_POINT				21

#define EPMEM_STMT_THREE_ADD_TIME					10
#define EPMEM_STMT_THREE_ADD_NODE_NOW				11
#define EPMEM_STMT_THREE_DELETE_NODE_NOW			12
#define EPMEM_STMT_THREE_ADD_EDGE_NOW				13
#define EPMEM_STMT_THREE_DELETE_EDGE_NOW			14
#define EPMEM_STMT_THREE_ADD_NODE_POINT				15
#define EPMEM_STMT_THREE_ADD_EDGE_POINT				16
#define EPMEM_STMT_THREE_ADD_NODE_RANGE				17
#define EPMEM_STMT_THREE_ADD_EDGE_RANGE				18
#define EPMEM_STMT_THREE_FIND_NODE_UNIQUE			19
#define EPMEM_STMT_THREE_ADD_NODE_UNIQUE			20
#define EPMEM_STMT_THREE_FIND_EDGE_UNIQUE			21
#define EPMEM_STMT_THREE_FIND_EDGE_UNIQUE_SHARED	22
#define EPMEM_STMT_THREE_ADD_EDGE_UNIQUE			23
#define EPMEM_STMT_THREE_VALID_EPISODE				24
#define EPMEM_STMT_THREE_NEXT_EPISODE				25
#define EPMEM_STMT_THREE_PREV_EPISODE				26
#define EPMEM_STMT_THREE_GET_NODES					27
#define EPMEM_STMT_THREE_GET_EDGES					28

#define EPMEM_MAX_STATEMENTS 						40 // must be at least 1+ largest of any STMT constant

// variables (rit vars must be same as stat versions)
#define EPMEM_VAR_RIT_OFFSET_1						EPMEM_STAT_RIT_OFFSET_1
#define EPMEM_VAR_RIT_LEFTROOT_1					EPMEM_STAT_RIT_LEFTROOT_1
#define EPMEM_VAR_RIT_RIGHTROOT_1					EPMEM_STAT_RIT_RIGHTROOT_1
#define EPMEM_VAR_RIT_MINSTEP_1						EPMEM_STAT_RIT_MINSTEP_1

#define EPMEM_VAR_RIT_OFFSET_2						EPMEM_STAT_RIT_OFFSET_2
#define EPMEM_VAR_RIT_LEFTROOT_2					EPMEM_STAT_RIT_LEFTROOT_2
#define EPMEM_VAR_RIT_RIGHTROOT_2					EPMEM_STAT_RIT_RIGHTROOT_2
#define EPMEM_VAR_RIT_MINSTEP_2						EPMEM_STAT_RIT_MINSTEP_2

#define EPMEM_VAR_MODE								EPMEM_VAR_RIT_MINSTEP_2 + 1
#define EPMEM_VAR_NEXT_ID							EPMEM_VAR_MODE + 1

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

// keeping state for multiple RIT's
#define EPMEM_RIT_STATE_OFFSET						0
#define EPMEM_RIT_STATE_LEFTROOT					1
#define EPMEM_RIT_STATE_RIGHTROOT					2
#define EPMEM_RIT_STATE_MINSTEP						3
#define EPMEM_RIT_STATE_ADD							4
#define EPMEM_RIT_STATE_TIMER						5

#define EPMEM_RIT_STATE_NODE						0
#define EPMEM_RIT_STATE_EDGE						1

//////////////////////////////////////////////////////////
// Parameter Types
//////////////////////////////////////////////////////////

enum epmem_param_type { epmem_param_constant = 1, epmem_param_number = 2, epmem_param_string = 3, epmem_param_invalid = 4 };

typedef struct epmem_constant_parameter_struct
{
	long value;
	bool (*val_func)( const long );
	const char *(*to_str)( const long );
	const long (*from_str)( const char * );
} epmem_constant_parameter;

typedef struct epmem_number_parameter_struct
{
	double value;
	bool (*val_func)( double );
} epmem_number_parameter;

typedef struct epmem_string_parameter_struct
{
	std::string *value;
	bool (*val_func)( const char * );
} epmem_string_parameter;

typedef union epmem_parameter_union_class
{
	epmem_constant_parameter constant_param;
	epmem_number_parameter number_param;
	epmem_string_parameter string_param;
} epmem_parameter_union;

typedef struct epmem_parameter_struct
{
	epmem_parameter_union *param;
	epmem_param_type type;
	const char *name;
} epmem_parameter;


//////////////////////////////////////////////////////////
// Stat Types
//////////////////////////////////////////////////////////

typedef struct epmem_stat_struct
{
	EPMEM_TYPE_INT value;
	const char *name;
} epmem_stat;


//////////////////////////////////////////////////////////
// Timer Types
//////////////////////////////////////////////////////////

typedef struct epmem_timer_struct
{
	struct timeval start_timer;
	struct timeval total_timer;
	const char *name;

	long level;
} epmem_timer;


//////////////////////////////////////////////////////////
// Common Types
//////////////////////////////////////////////////////////

// represents a unique node identifier in the episodic store
typedef unsigned EPMEM_TYPE_INT epmem_node_id;

// represents a unique episode identifier in the episodic store
typedef EPMEM_TYPE_INT epmem_time_id;

// represents a vector of times
typedef std::vector<epmem_time_id> epmem_time_list;

//////////////////////////////////////////////////////////
// Soar Integration Types
//////////////////////////////////////////////////////////

// data associated with each state
typedef struct epmem_data_struct
{
	unsigned EPMEM_TYPE_INT last_ol_time;	// last update to output-link
	unsigned EPMEM_TYPE_INT last_ol_count;	// last count of output-link

	unsigned EPMEM_TYPE_INT last_cmd_time;	// last update to epmem.command
	unsigned EPMEM_TYPE_INT last_cmd_count;	// last update to epmem.command

	epmem_time_id last_memory;				// last retrieved memory

	wme *ss_wme;

	std::set<wme *> *cue_wmes;				// wmes in last cue
	std::stack<wme *> *epmem_wmes;			// wmes in last epmem
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
	EPMEM_TYPE_INT ct;						// cardinality w.r.t. positive/negative query

	long timer;								// timer to update upon executing the query
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
	unsigned EPMEM_TYPE_INT len;			// number of children
	unsigned EPMEM_TYPE_INT parents;		// number of parents

	epmem_literal_mapping *lits;			// child literals
} epmem_wme_cache_element;

// represents state of a leaf wme
// at a particular episode
typedef struct epmem_shared_match_struct
{
	double value_weight;					// wma value
	EPMEM_TYPE_INT value_ct;				// cardinality w.r.t. positive/negative query

	unsigned EPMEM_TYPE_INT ct;				// number of contributing literals that are "on"
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

	unsigned EPMEM_TYPE_INT ct;				// number of contributing literals that are "on"
	unsigned EPMEM_TYPE_INT max;			// number of contributing literals that *need* to be on

	struct wme_struct *wme;					// associated cue wme
	unsigned EPMEM_TYPE_INT wme_kids;		// number of children the cue wme has

	epmem_shared_match *match;				// associated match, if leaf wme
	epmem_shared_literal_group *children;	// grouped child literals, if not leaf wme
};

// maintains state within sqlite b-trees
typedef struct epmem_shared_query_struct
{
	sqlite3_stmt *stmt;						// associated sqlite query
	epmem_time_id val;						// current b-tree leaf value
	long timer;								// timer to update upon executing the query

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

// clean memory
extern void epmem_clean_parameters( agent *my_agent );

// add parameter
extern epmem_parameter *epmem_new_parameter( const char *name, double value, bool (*val_func)( double ) );
extern epmem_parameter *epmem_new_parameter( const char *name, const long value, bool (*val_func)( const long ), const char *(*to_str)( long ), const long (*from_str)( const char * ) );
extern epmem_parameter *epmem_new_parameter( const char *name, const char *value, bool (*val_func)( const char * ) );

// convert parameter
extern const char *epmem_convert_parameter( agent *my_agent, const long param );
extern const long epmem_convert_parameter( agent *my_agent, const char *name );

// validate parameter
extern bool epmem_valid_parameter( agent *my_agent, const char *name );
extern bool epmem_valid_parameter( agent *my_agent, const long param );

// parameter type
extern epmem_param_type epmem_get_parameter_type( agent *my_agent, const char *name );
extern epmem_param_type epmem_get_parameter_type( agent *my_agent, const long param );

// get parameter
extern const long epmem_get_parameter( agent *my_agent, const char *name, const double test );
extern const char *epmem_get_parameter( agent *my_agent, const char *name, const char *test );
extern double epmem_get_parameter( agent *my_agent, const char *name );

extern const long epmem_get_parameter( agent *my_agent, const long param, const double test );
extern const char *epmem_get_parameter( agent *my_agent, const long param, const char *test );
extern double epmem_get_parameter( agent *my_agent, const long param );

// validate parameter value
extern bool epmem_valid_parameter_value( agent *my_agent, const char *name, double new_val );
extern bool epmem_valid_parameter_value( agent *my_agent, const char *name, const char *new_val );
extern bool epmem_valid_parameter_value( agent *my_agent, const char *name, const long new_val );

extern bool epmem_valid_parameter_value( agent *my_agent, const long param, double new_val );
extern bool epmem_valid_parameter_value( agent *my_agent, const long param, const char *new_val );
extern bool epmem_valid_parameter_value( agent *my_agent, const long param, const long new_val );

// set parameter
extern bool epmem_set_parameter( agent *my_agent, const char *name, double new_val );
extern bool epmem_set_parameter( agent *my_agent, const char *name, const char *new_val );
extern bool epmem_set_parameter( agent *my_agent, const char *name, const long new_val );

extern bool epmem_set_parameter( agent *my_agent, const long param, double new_val );
extern bool epmem_set_parameter( agent *my_agent, const long param, const char *new_val );
extern bool epmem_set_parameter( agent *my_agent, const long param, const long new_val );

// learning
extern bool epmem_validate_learning( const long new_val );
extern const char *epmem_convert_learning( const long val );
extern const long epmem_convert_learning( const char *val );

// database
extern bool epmem_validate_database( const long new_val );
extern const char *epmem_convert_database( const long val );
extern const long epmem_convert_database( const char *val );

// path
extern bool epmem_validate_path( const char *new_val );

// mode
extern bool epmem_validate_mode( const long new_val );
extern const char *epmem_convert_mode( const long val );
extern const long epmem_convert_mode( const char *val );

// graph match
extern bool epmem_validate_graph_match( const long new_val );
extern const char *epmem_convert_graph_match( const long val );
extern const long epmem_convert_graph_match( const char *val );

// phase
extern bool epmem_validate_phase( const long new_val );
extern const char *epmem_convert_phase( const long val );
extern const long epmem_convert_phase( const char *val );

// trigger
extern bool epmem_validate_trigger( const long new_val );
extern const char *epmem_convert_trigger( const long val );
extern const long epmem_convert_trigger( const char *val );

// force
extern bool epmem_validate_force( const long new_val );
extern const char *epmem_convert_force( const long val );
extern const long epmem_convert_force( const char *val );

// balance
extern bool epmem_validate_balance( const double new_val );

// exclusions
extern bool epmem_validate_exclusions( const char *new_val );

// commit
extern bool epmem_validate_commit( const double new_val );

// timers
extern bool epmem_validate_ext_timers( const long new_val );
extern const char *epmem_convert_ext_timers( const long val );
extern const long epmem_convert_ext_timers( const char *val );

// shortcut for determining if EpMem is enabled
extern bool epmem_enabled( agent *my_agent );

//////////////////////////////////////////////////////////
// Stat Functions (see cpp for comments)
//////////////////////////////////////////////////////////

// memory clean
extern void epmem_clean_stats( agent *my_agent );
extern void epmem_reset_stats( agent *my_agent );

// add stat
extern epmem_stat *epmem_new_stat( const char *name );

// convert stat
extern const long epmem_convert_stat( agent *my_agent, const char *name );
extern const char *epmem_convert_stat( agent *my_agent, const long stat );

// valid stat
extern bool epmem_valid_stat( agent *my_agent, const char *name );
extern bool epmem_valid_stat( agent *my_agent, const long stat );

// get stat
extern EPMEM_TYPE_INT epmem_get_stat( agent *my_agent, const char *name );
extern EPMEM_TYPE_INT epmem_get_stat( agent *my_agent, const long stat );

// set stat
extern bool epmem_set_stat( agent *my_agent, const char *name, EPMEM_TYPE_INT new_val );
extern bool epmem_set_stat( agent *my_agent, const long stat, EPMEM_TYPE_INT new_val );


//////////////////////////////////////////////////////////
// Timer Functions (see cpp for comments)
//////////////////////////////////////////////////////////

// memory clean
extern void epmem_clean_timers( agent *my_agent );
extern void epmem_reset_timers( agent *my_agent );

// add timer
extern epmem_timer *epmem_new_timer( const char *name, long timer );

// convert timer
extern const long epmem_convert_timer( agent *my_agent, const char *name );
extern const char *epmem_convert_timer( agent *my_agent, const long timer );

// valid timer
extern bool epmem_valid_timer( agent *my_agent, const char *name );
extern bool epmem_valid_timer( agent *my_agent, const long timer );

// get timer
extern double epmem_get_timer_value( agent *my_agent, const char *name );
extern double epmem_get_timer_value( agent *my_agent, const long timer );

// get timer name
extern const char *epmem_get_timer_name( agent *my_agent, const char *name );
extern const char *epmem_get_timer_name( agent *my_agent, const long timer );

// timer functions
extern void epmem_start_timer( agent *my_agent, const long timer );
extern void epmem_stop_timer( agent *my_agent, const long timer );


//////////////////////////////////////////////////////////
// Soar Functions (see cpp for comments)
//////////////////////////////////////////////////////////

// init, end
extern void epmem_reset( agent *my_agent, Symbol *state = NULL );
extern void epmem_close( agent *my_agent );

// perform epmem actions
extern void epmem_go( agent *my_agent );

// Called to create/remove a fake preference for an epmem wme
extern preference *epmem_make_fake_preference( agent *my_agent, Symbol *state, wme *w );

// Called to get a specific symbol augmentation
extern wme *epmem_get_aug_of_id( agent *my_agent, Symbol *sym, char *attr_name, char *value_name );

#endif
