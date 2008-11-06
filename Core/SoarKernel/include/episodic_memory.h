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
#define EPMEM_PARAM_TRIGGER							5
#define EPMEM_PARAM_FORCE							6
#define EPMEM_PARAM_BALANCE							7
#define EPMEM_PARAM_EXCLUSIONS						8
#define EPMEM_PARAM_TIMERS							9
#define EPMEM_PARAMS								10 // must be 1+ last epmem param

// parameter settings
#define EPMEM_LEARNING_ON 1
#define EPMEM_LEARNING_OFF 2

#define EPMEM_DB_MEM 1
#define EPMEM_DB_FILE 2

#define EPMEM_MODE_ONE 1   // wm tree
#define EPMEM_MODE_TWO 2   // wm tree + mva/shared wme on retrieval
#define EPMEM_MODE_THREE 3 // wm tree + mva/shared wme on retrieval/query

#define EPMEM_TRIGGER_NONE 1
#define EPMEM_TRIGGER_OUTPUT 2
#define EPMEM_TRIGGER_DC 3

#define EPMEM_FORCE_REMEMBER 1
#define EPMEM_FORCE_IGNORE 2
#define EPMEM_FORCE_OFF 3

#define EPMEM_TIMERS_ON 1
#define EPMEM_TIMERS_OFF 2

// statistics
// * = protected
#define EPMEM_STAT_TIME								0 // *
#define EPMEM_STAT_MEM_USAGE						1
#define EPMEM_STAT_MEM_HIGH							2
#define EPMEM_STAT_QRY_POS							3
#define EPMEM_STAT_QRY_NEG							4
#define EPMEM_STAT_QRY_RET							5
#define EPMEM_STAT_QRY_CARD							6

#define EPMEM_STAT_RIT_OFFSET_1						7 // *
#define EPMEM_STAT_RIT_LEFTROOT_1					8 // *
#define EPMEM_STAT_RIT_RIGHTROOT_1					9 // *
#define EPMEM_STAT_RIT_MINSTEP_1					10 // *

#define EPMEM_STAT_RIT_OFFSET_2						11 // *
#define EPMEM_STAT_RIT_LEFTROOT_2					12 // *
#define EPMEM_STAT_RIT_RIGHTROOT_2					13 // *
#define EPMEM_STAT_RIT_MINSTEP_2					14 // *

#define EPMEM_STATS									15 // must be 1+ last epmem stat

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
// 10 - 29 => mode
#define EPMEM_STMT_BEGIN					0
#define EPMEM_STMT_COMMIT					1
#define EPMEM_STMT_ROLLBACK					2
#define EPMEM_STMT_VAR_GET					3
#define EPMEM_STMT_VAR_SET					4

#define EPMEM_STMT_RIT_ADD_LEFT				5
#define EPMEM_STMT_RIT_TRUNCATE_LEFT		6
#define EPMEM_STMT_RIT_ADD_RIGHT			7
#define EPMEM_STMT_RIT_TRUNCATE_RIGHT		8

#define EPMEM_STMT_ONE_ADD_TIME				10
#define EPMEM_STMT_ONE_ADD_EPISODE			11
#define EPMEM_STMT_ONE_ADD_ID				12
#define EPMEM_STMT_ONE_FIND_ID				13
#define EPMEM_STMT_ONE_FIND_ID_NULL			14
#define EPMEM_STMT_ONE_VALID_EPISODE		15
#define EPMEM_STMT_ONE_NEXT_EPISODE			16
#define EPMEM_STMT_ONE_PREV_EPISODE			17
#define EPMEM_STMT_ONE_GET_EPISODE			18
#define EPMEM_STMT_ONE_ADD_NOW				19
#define EPMEM_STMT_ONE_DELETE_NOW			20
#define EPMEM_STMT_ONE_ADD_POINT			21
#define EPMEM_STMT_ONE_MVA_ADD_ID			22
#define EPMEM_STMT_ONE_MVA_GET_EP			23

#define EPMEM_MAX_STATEMENTS 				30 // must be at least 1+ largest of any STMT constant

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

#define EPMEM_RIT_STATE_OFFSET						0
#define EPMEM_RIT_STATE_LEFTROOT					1
#define EPMEM_RIT_STATE_RIGHTROOT					2
#define EPMEM_RIT_STATE_MINSTEP						3
#define EPMEM_RIT_STATE_ADD							4

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
} epmem_timer;

//

typedef unsigned long long int epmem_node_id;
typedef long long int epmem_time_id;

typedef struct epmem_data_struct 
{
	unsigned long last_ol_time;		// last update to output-link
	unsigned long last_ol_count;	// last count of output-link

	unsigned long last_cmd_time;	// last update to epmem.command
	unsigned long last_cmd_count;	// last update to epmem.command

	epmem_time_id last_memory;		// last retrieved memory

	wme *ss_wme;

	std::list<wme *> *cue_wmes;		// wmes in last cue
	std::stack<wme *> *epmem_wmes;	// wmes in last epmem
} epmem_data;

typedef struct epmem_leaf_node_struct
{
	epmem_node_id leaf_id;
	double leaf_weight;
} epmem_leaf_node;

typedef struct epmem_range_query_struct
{
	sqlite3_stmt *stmt;
	epmem_time_id val;

	double weight;
	long long ct;

	long timer;	
} epmem_range_query;

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
extern epmem_timer *epmem_add_timer( const char *name );

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
