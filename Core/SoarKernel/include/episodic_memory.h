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

#include <list>
#include <stack>
#include <set>
#include <queue>

#include "sqlite3.h"

typedef struct wme_struct wme;

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
#define EPMEM_PARAM_TRIGGER							6
#define EPMEM_PARAM_FORCE							7
#define EPMEM_PARAM_BALANCE							8
#define EPMEM_PARAM_EXCLUSIONS						9
#define EPMEM_PARAM_TIMERS							10
#define EPMEM_PARAMS								11 // must be 1+ last epmem param

// parameter settings
#define EPMEM_LEARNING_ON 1
#define EPMEM_LEARNING_OFF 2

#define EPMEM_DB_MEM 1
#define EPMEM_DB_FILE 2

#define EPMEM_MODE_ONE 1   // wm tree
#define EPMEM_MODE_THREE 3 // mva/shared wme

#define EPMEM_GRAPH_MATCH_OFF 1
#define EPMEM_GRAPH_MATCH_PATHS 2
#define EPMEM_GRAPH_MATCH_WMES 3

#define EPMEM_TRIGGER_NONE 1
#define EPMEM_TRIGGER_OUTPUT 2
#define EPMEM_TRIGGER_DC 3

#define EPMEM_FORCE_REMEMBER 1
#define EPMEM_FORCE_IGNORE 2
#define EPMEM_FORCE_OFF 3

#define EPMEM_TIMERS_OFF 0
#define EPMEM_TIMERS_ONE 1
#define EPMEM_TIMERS_TWO 2
#define EPMEM_TIMERS_THREE 3

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

#define EPMEM_STAT_NEXT_ID							8

#define EPMEM_STAT_RIT_OFFSET_1						9 // *
#define EPMEM_STAT_RIT_LEFTROOT_1					10 // *
#define EPMEM_STAT_RIT_RIGHTROOT_1					11 // *
#define EPMEM_STAT_RIT_MINSTEP_1					12 // *

#define EPMEM_STAT_RIT_OFFSET_2						13 // *
#define EPMEM_STAT_RIT_LEFTROOT_2					14 // *
#define EPMEM_STAT_RIT_RIGHTROOT_2					15 // *
#define EPMEM_STAT_RIT_MINSTEP_2					16 // *

#define EPMEM_STATS									17 // must be 1+ last epmem stat

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
#define EPMEM_TIMER_QUERY_POS_START_EP				9
#define EPMEM_TIMER_QUERY_POS_START_NOW				10
#define EPMEM_TIMER_QUERY_POS_START_POINT			11
#define EPMEM_TIMER_QUERY_POS_END_EP				12
#define EPMEM_TIMER_QUERY_POS_END_NOW				13
#define EPMEM_TIMER_QUERY_POS_END_POINT				14
#define EPMEM_TIMER_QUERY_NEG_START_EP				15
#define EPMEM_TIMER_QUERY_NEG_START_NOW				16
#define EPMEM_TIMER_QUERY_NEG_START_POINT			17
#define EPMEM_TIMER_QUERY_NEG_END_EP				18
#define EPMEM_TIMER_QUERY_NEG_END_NOW				19
#define EPMEM_TIMER_QUERY_NEG_END_POINT				20

#define EPMEM_TIMERS								21 // must be 1+ last epmem timer

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
#define EPMEM_STMT_THREE_ADD_EDGE_UNIQUE			22
#define EPMEM_STMT_THREE_VALID_EPISODE				23
#define EPMEM_STMT_THREE_NEXT_EPISODE				24
#define EPMEM_STMT_THREE_PREV_EPISODE				25
#define EPMEM_STMT_THREE_GET_NODES					26
#define EPMEM_STMT_THREE_GET_EDGES					27

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
#define EPMEM_MEMID_NONE							-1
#define EPMEM_MEMID_ROOT							0

#define EPMEM_NODE_POS								0
#define EPMEM_NODE_NEG								1
#define EPMEM_RANGE_START							0
#define EPMEM_RANGE_END								1
#define EPMEM_RANGE_EP								0
#define EPMEM_RANGE_NOW								1
#define EPMEM_RANGE_POINT							2

#define EPMEM_RIT_ROOT								0
#define EPMEM_LN_2									0.693147180559945

#define EPMEM_DNF									2

// keeping state for multiple RIT's
#define EPMEM_RIT_STATE_OFFSET						0
#define EPMEM_RIT_STATE_LEFTROOT					1
#define EPMEM_RIT_STATE_RIGHTROOT					2
#define EPMEM_RIT_STATE_MINSTEP						3
#define EPMEM_RIT_STATE_ADD							4

#define EPMEM_RIT_STATE_NODE						0
#define EPMEM_RIT_STATE_EDGE						1

//////////////////////////////////////////////////////////
// EpMem Types
//////////////////////////////////////////////////////////

// parameters
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

// statistics
typedef struct epmem_stat_struct
{
	long long value;
	const char *name;
} epmem_stat;

// timers
typedef struct epmem_timer_struct
{
	struct timeval start_timer;
	struct timeval total_timer;
	const char *name;

	long level;
} epmem_timer;

// common
typedef unsigned long long int epmem_node_id;
typedef long long int epmem_time_id;

// soar
typedef struct epmem_data_struct
{
	unsigned long last_ol_time;		// last update to output-link
	unsigned long last_ol_count;	// last count of output-link

	unsigned long last_cmd_time;	// last update to epmem.command
	unsigned long last_cmd_count;	// last update to epmem.command

	epmem_time_id last_memory;		// last retrieved memory

	wme *ss_wme;

	std::set<wme *> *cue_wmes;		// wmes in last cue
	std::stack<wme *> *epmem_wmes;	// wmes in last epmem
} epmem_data;

// mode: one
typedef struct epmem_leaf_node_struct
{
	double leaf_weight;
	epmem_node_id leaf_id;
} epmem_leaf_node;

typedef struct epmem_range_query_struct
{
	sqlite3_stmt *stmt;
	epmem_time_id val;

	double weight;
	long long ct;

	long timer;
} epmem_range_query;

struct epmem_compare_range_queries
{
	bool operator() ( const epmem_range_query *a, const epmem_range_query *b ) const
	{
		return ( a->val < b->val );
	}
};

typedef std::priority_queue<epmem_range_query *, std::vector<epmem_range_query *>, epmem_compare_range_queries> epmem_range_query_list;

// mode: three
typedef struct epmem_edge_struct
{
	epmem_node_id q0;
	Symbol *w;
	epmem_node_id q1;
} epmem_edge;

typedef struct epmem_wme_cache_element_struct
{
	wme **wmes;
	int len;
} epmem_wme_cache_element;

typedef struct epmem_shared_literal_struct epmem_shared_literal;
typedef std::list<epmem_shared_literal *> epmem_shared_trigger_list;

typedef struct epmem_shared_match_struct
{
	double value_weight;
	long long value_ct;

	unsigned long long ct;
} epmem_shared_match;

struct epmem_shared_literal_struct
{
	epmem_node_id shared_id;
	
	unsigned long long ct;

	struct wme_struct *wme;
	unsigned long long wme_kids;

	epmem_shared_match *match;
	epmem_shared_trigger_list *children;
};

typedef struct epmem_shared_query_struct
{
	sqlite3_stmt *stmt;
	epmem_time_id val;
	long timer;

	epmem_shared_trigger_list *triggers;
} epmem_shared_query;

struct epmem_compare_shared_queries
{
	bool operator() ( const epmem_shared_query *a, const epmem_shared_query *b ) const
	{
		return ( a->val < b->val );
	}
};

typedef std::priority_queue<epmem_shared_query *, std::vector<epmem_shared_query *>, epmem_compare_shared_queries> epmem_shared_query_list;

typedef std::map<Symbol *, epmem_node_id> epmem_constraint_list;

//
// These must go below types
//

#include "stl_support.h"

//////////////////////////////////////////////////////////
// Parameter Functions
//////////////////////////////////////////////////////////

// clean memory
extern void epmem_clean_parameters( agent *my_agent );

// add parameter
extern epmem_parameter *epmem_add_parameter( const char *name, double value, bool (*val_func)( double ) );
extern epmem_parameter *epmem_add_parameter( const char *name, const long value, bool (*val_func)( const long ), const char *(*to_str)( long ), const long (*from_str)( const char * ) );
extern epmem_parameter *epmem_add_parameter( const char *name, const char *value, bool (*val_func)( const char * ) );

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
// Stat Functions
//////////////////////////////////////////////////////////

// memory clean
extern void epmem_clean_stats( agent *my_agent );
extern void epmem_reset_stats( agent *my_agent );

// add stat
extern epmem_stat *epmem_add_stat( const char *name );

// convert stat
extern const long epmem_convert_stat( agent *my_agent, const char *name );
extern const char *epmem_convert_stat( agent *my_agent, const long stat );

// valid stat
extern bool epmem_valid_stat( agent *my_agent, const char *name );
extern bool epmem_valid_stat( agent *my_agent, const long stat );

// get stat
extern long long epmem_get_stat( agent *my_agent, const char *name );
extern long long epmem_get_stat( agent *my_agent, const long stat );

// set stat
extern bool epmem_set_stat( agent *my_agent, const char *name, long long new_val );
extern bool epmem_set_stat( agent *my_agent, const long stat, long long new_val );


//////////////////////////////////////////////////////////
// Timer Functions
//////////////////////////////////////////////////////////

// memory clean
extern void epmem_clean_timers( agent *my_agent );
extern void epmem_reset_timers( agent *my_agent );

// add timer
extern epmem_timer *epmem_add_timer( const char *name, long timer );

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
// Soar Functions
//////////////////////////////////////////////////////////

// init, end
extern void epmem_reset( agent *my_agent, Symbol *state = NULL );
extern void epmem_end( agent *my_agent );

// perform epmem actions
extern void epmem_go( agent *my_agent );

// Called to create/remove a fake preference for an epmem wme
extern preference *epmem_make_fake_preference( agent *my_agent, Symbol *state, wme *w );

// Called to get a specific symbol augmentation
extern wme *epmem_get_aug_of_id( agent *my_agent, Symbol *sym, char *attr_name, char *value_name );


#endif
