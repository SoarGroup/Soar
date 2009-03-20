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

#include <string>

//////////////////////////////////////////////////////////
// SMem Constants
//////////////////////////////////////////////////////////
#define SMEM_RETURN_LONG 0.1
#define SMEM_RETURN_STRING ""

// parameters
// - protected are [ DB, PATH ]
#define SMEM_PARAM_LEARNING							0
#define SMEM_PARAM_DB								1
#define SMEM_PARAM_COMMIT							2
#define SMEM_PARAM_PATH								3
#define SMEM_PARAM_TIMERS							4
#define SMEM_PARAMS									5 // must be 1+ last smem param

// parameter settings
#define SMEM_LEARNING_ON 1
#define SMEM_LEARNING_OFF 2

#define SMEM_DB_MEM 1
#define SMEM_DB_FILE 2

#define SMEM_TIMERS_OFF 1
#define SMEM_TIMERS_ONE 2
#define SMEM_TIMERS_TWO 3
#define SMEM_TIMERS_THREE 4


// statistics
// * = protected
#define SMEM_STAT_MEM_USAGE							0
#define SMEM_STAT_MEM_HIGH							1
#define SMEM_STAT_NEXT_ID							2 // *

#define SMEM_STATS									3 // must be 1+ last smem stat


// timers
#define SMEM_TIMER_TOTAL							0
#define SMEM_TIMER_STORAGE							1
#define SMEM_TIMER_NCB_RETRIEVAL					2
#define SMEM_TIMER_QUERY							3
#define SMEM_TIMER_API								4
#define SMEM_TIMER_INIT								5

#define SMEM_TIMERS									6 // must be 1+ last smem timer


// statements
// 0 - 19 => common
// 20 - ( SMEM_MAX_STATEMENTS - 1 ) => mode
#define SMEM_STMT_BEGIN								0
#define SMEM_STMT_COMMIT							1
#define SMEM_STMT_ROLLBACK							2
#define SMEM_STMT_VAR_GET							3
#define SMEM_STMT_VAR_SET							4

#define SMEM_STMT_GET_HASH							9
#define SMEM_STMT_ADD_HASH							10

#define SMEM_MAX_STATEMENTS 						40 // must be at least 1+ largest of any STMT constant


// algorithm constants
#define SMEM_DB_CLOSED								-1 // initialize db_status to this (sqlite error codes are positive)
#define SMEM_DB_PREP_STR_MAX						-1 // non-zero nByte param indicates to read to zero terminator


//////////////////////////////////////////////////////////
// Parameter Types
//////////////////////////////////////////////////////////

enum smem_param_type { smem_param_constant = 1, smem_param_number = 2, smem_param_string = 3, smem_param_invalid = 4 };

typedef struct smem_constant_parameter_struct
{
	long value;
	bool (*val_func)( const long );
	const char *(*to_str)( const long );
	const long (*from_str)( const char * );
} smem_constant_parameter;

typedef struct smem_number_parameter_struct
{
	double value;
	bool (*val_func)( double );
} smem_number_parameter;

typedef struct smem_string_parameter_struct
{
	std::string *value;
	bool (*val_func)( const char * );
} smem_string_parameter;

typedef union smem_parameter_union_class
{
	smem_constant_parameter constant_param;
	smem_number_parameter number_param;
	smem_string_parameter string_param;
} smem_parameter_union;

typedef struct smem_parameter_struct
{
	smem_parameter_union *param;
	smem_param_type type;
	const char *name;
} smem_parameter;


//////////////////////////////////////////////////////////
// Stat Types
//////////////////////////////////////////////////////////

typedef struct smem_stat_struct
{
	EPMEM_TYPE_INT value;
	const char *name;
} smem_stat;


//////////////////////////////////////////////////////////
// Timer Types
//////////////////////////////////////////////////////////

typedef struct smem_timer_struct
{
	struct timeval start_timer;
	struct timeval total_timer;
	const char *name;

	long level;
} smem_timer;


//////////////////////////////////////////////////////////
// Soar Integration Types
//////////////////////////////////////////////////////////

// data associated with each state
typedef struct smem_data_struct
{
	unsigned EPMEM_TYPE_INT last_cmd_time;	// last update to epmem.command
	unsigned EPMEM_TYPE_INT last_cmd_count;	// last update to epmem.command	

	wme *ss_wme;

	std::set<wme *> *cue_wmes;				// wmes in last cue
	std::stack<wme *> *smem_wmes;			// wmes in last epmem
} smem_data;


//
// These must go below types
//

#include "stl_support.h"


//////////////////////////////////////////////////////////
// Parameter Functions (see cpp for comments)
//////////////////////////////////////////////////////////

// clean memory
extern void smem_clean_parameters( agent *my_agent );

// add parameter
extern smem_parameter *smem_new_parameter( const char *name, double value, bool (*val_func)( double ) );
extern smem_parameter *smem_new_parameter( const char *name, const long value, bool (*val_func)( const long ), const char *(*to_str)( long ), const long (*from_str)( const char * ) );
extern smem_parameter *smem_new_parameter( const char *name, const char *value, bool (*val_func)( const char * ) );

// convert parameter
extern const char *smem_convert_parameter( agent *my_agent, const long param );
extern const long smem_convert_parameter( agent *my_agent, const char *name );

// validate parameter
extern bool smem_valid_parameter( agent *my_agent, const char *name );
extern bool smem_valid_parameter( agent *my_agent, const long param );

// parameter type
extern smem_param_type smem_get_parameter_type( agent *my_agent, const char *name );
extern smem_param_type smem_get_parameter_type( agent *my_agent, const long param );

// get parameter
extern const long smem_get_parameter( agent *my_agent, const char *name, const double test );
extern const char *smem_get_parameter( agent *my_agent, const char *name, const char *test );
extern double smem_get_parameter( agent *my_agent, const char *name );

extern const long smem_get_parameter( agent *my_agent, const long param, const double test );
extern const char *smem_get_parameter( agent *my_agent, const long param, const char *test );
extern double smem_get_parameter( agent *my_agent, const long param );

// validate parameter value
extern bool smem_valid_parameter_value( agent *my_agent, const char *name, double new_val );
extern bool smem_valid_parameter_value( agent *my_agent, const char *name, const char *new_val );
extern bool smem_valid_parameter_value( agent *my_agent, const char *name, const long new_val );

extern bool smem_valid_parameter_value( agent *my_agent, const long param, double new_val );
extern bool smem_valid_parameter_value( agent *my_agent, const long param, const char *new_val );
extern bool smem_valid_parameter_value( agent *my_agent, const long param, const long new_val );

// set parameter
extern bool smem_set_parameter( agent *my_agent, const char *name, double new_val );
extern bool smem_set_parameter( agent *my_agent, const char *name, const char *new_val );
extern bool smem_set_parameter( agent *my_agent, const char *name, const long new_val );

extern bool smem_set_parameter( agent *my_agent, const long param, double new_val );
extern bool smem_set_parameter( agent *my_agent, const long param, const char *new_val );
extern bool smem_set_parameter( agent *my_agent, const long param, const long new_val );

// learning
extern bool smem_validate_learning( const long new_val );
extern const char *smem_convert_learning( const long val );
extern const long smem_convert_learning( const char *val );

// database
extern bool smem_validate_database( const long new_val );
extern const char *smem_convert_database( const long val );
extern const long smem_convert_database( const char *val );

// path
extern bool smem_validate_path( const char *new_val );

// commit
extern bool smem_validate_commit( const double new_val );

// timers
extern bool smem_validate_ext_timers( const long new_val );
extern const char *smem_convert_ext_timers( const long val );
extern const long smem_convert_ext_timers( const char *val );

// shortcut for determining if EpMem is enabled
extern bool smem_enabled( agent *my_agent );


//////////////////////////////////////////////////////////
// Stat Functions (see cpp for comments)
//////////////////////////////////////////////////////////

// memory clean
extern void smem_clean_stats( agent *my_agent );
extern void smem_reset_stats( agent *my_agent );

// add stat
extern smem_stat *smem_new_stat( const char *name );

// convert stat
extern const long smem_convert_stat( agent *my_agent, const char *name );
extern const char *smem_convert_stat( agent *my_agent, const long stat );

// valid stat
extern bool smem_valid_stat( agent *my_agent, const char *name );
extern bool smem_valid_stat( agent *my_agent, const long stat );

// get stat
extern EPMEM_TYPE_INT smem_get_stat( agent *my_agent, const char *name );
extern EPMEM_TYPE_INT smem_get_stat( agent *my_agent, const long stat );

// set stat
extern bool smem_set_stat( agent *my_agent, const char *name, EPMEM_TYPE_INT new_val );
extern bool smem_set_stat( agent *my_agent, const long stat, EPMEM_TYPE_INT new_val );


//////////////////////////////////////////////////////////
// Timer Functions (see cpp for comments)
//////////////////////////////////////////////////////////

// memory clean
extern void smem_clean_timers( agent *my_agent );
extern void smem_reset_timers( agent *my_agent );

// add timer
extern smem_timer *smem_new_timer( const char *name, long timer );

// convert timer
extern const long smem_convert_timer( agent *my_agent, const char *name );
extern const char *smem_convert_timer( agent *my_agent, const long timer );

// valid timer
extern bool smem_valid_timer( agent *my_agent, const char *name );
extern bool smem_valid_timer( agent *my_agent, const long timer );

// get timer
extern double smem_get_timer_value( agent *my_agent, const char *name );
extern double smem_get_timer_value( agent *my_agent, const long timer );

// get timer name
extern const char *smem_get_timer_name( agent *my_agent, const char *name );
extern const char *smem_get_timer_name( agent *my_agent, const long timer );

// timer functions
extern void smem_start_timer( agent *my_agent, const long timer );
extern void smem_stop_timer( agent *my_agent, const long timer );


//////////////////////////////////////////////////////////
// Soar Functions (see cpp for comments)
//////////////////////////////////////////////////////////

// init, end
extern void smem_reset( agent *my_agent, Symbol *state = NULL );
extern void smem_close( agent *my_agent );

// perform epmem actions
//extern void epmem_go( agent *my_agent );

// Called to create/remove a fake preference for an epmem wme
extern preference *smem_make_fake_preference( agent *my_agent, Symbol *state, wme *w );

// Called to get a specific symbol augmentation
extern wme *smem_get_aug_of_id( agent *my_agent, Symbol *sym, char *attr_name, char *value_name );


#endif