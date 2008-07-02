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

#include <vector>
#include <string>
#include <stack>
#include <list>

typedef struct wme_struct wme;

//////////////////////////////////////////////////////////
// EpMem Constants
//////////////////////////////////////////////////////////
#define EPMEM_RETURN_LONG 0.1
#define EPMEM_RETURN_STRING ""

#define EPMEM_LEARNING_ON 1
#define EPMEM_LEARNING_OFF 2

#define EPMEM_DB_MEM 1
#define EPMEM_DB_FILE 2

#define EPMEM_INDEXING_BIGTREE_INSTANCE 1
#define EPMEM_INDEXING_BIGTREE_RANGE	2
#define EPMEM_INDEXING_BIGTREE_RIT		3

#define EPMEM_PROVENANCE_ON 1
#define EPMEM_PROVENANCE_OFF 2

#define EPMEM_TRIGGER_OUTPUT 1
#define EPMEM_TRIGGER_DC 2

// statement storage
// 0 - 9 => common
// 10 - 39 => indexing
#define EPMEM_STMT_BEGIN							0
#define EPMEM_STMT_COMMIT							1
#define EPMEM_STMT_ROLLBACK							2
#define EPMEM_STMT_VAR_GET							3
#define EPMEM_STMT_VAR_SET							4

#define EPMEM_STMT_BIGTREE_I_ADD_EPISODE			10
#define EPMEM_STMT_BIGTREE_I_ADD_ID					11
#define EPMEM_STMT_BIGTREE_I_FIND_ID				12
#define EPMEM_STMT_BIGTREE_I_FIND_ID_NULL			13
#define EPMEM_STMT_BIGTREE_I_GET_EPISODE			14
#define EPMEM_STMT_BIGTREE_I_VALID_EPISODE			15
#define EPMEM_STMT_BIGTREE_I_NEXT_EPISODE			16
#define EPMEM_STMT_BIGTREE_I_PREV_EPISODE			17

#define EPMEM_STMT_BIGTREE_R_UPDATE_EPISODE			10
#define EPMEM_STMT_BIGTREE_R_ADD_EPISODE			11
#define EPMEM_STMT_BIGTREE_R_ADD_TIME				12
#define EPMEM_STMT_BIGTREE_R_ADD_ID					13
#define EPMEM_STMT_BIGTREE_R_FIND_ID				14
#define EPMEM_STMT_BIGTREE_R_FIND_ID_NULL			15
#define EPMEM_STMT_BIGTREE_R_GET_EPISODE			16
#define EPMEM_STMT_BIGTREE_R_VALID_EPISODE			17
#define EPMEM_STMT_BIGTREE_R_NEXT_EPISODE			18
#define EPMEM_STMT_BIGTREE_R_PREV_EPISODE			19
#define EPMEM_STMT_BIGTREE_R_ADD_WEIGHT				20
#define EPMEM_STMT_BIGTREE_R_TRUNCATE_WEIGHTS		21
#define EPMEM_STMT_BIGTREE_R_NULL_RANGES			22
#define EPMEM_STMT_BIGTREE_R_DEL_PROHIB				23
#define EPMEM_STMT_BIGTREE_R_DEL_PROHIB_LOW			24
#define EPMEM_STMT_BIGTREE_R_DEL_PROHIB_HIGH		25
#define EPMEM_STMT_BIGTREE_R_DEL_PROHIB_CONTAIN		26
#define EPMEM_STMT_BIGTREE_R_GET_LOW_RANGES			27
#define EPMEM_STMT_BIGTREE_R_GET_HIGH_RANGES		28
#define EPMEM_STMT_BIGTREE_R_TRUNCATE_RANGES		29
#define EPMEM_STMT_BIGTREE_R_RANGE_INDEX_1			30
#define EPMEM_STMT_BIGTREE_R_RANGE_INDEX_1R			31
#define EPMEM_STMT_BIGTREE_R_RANGE_INDEX_2			32
#define EPMEM_STMT_BIGTREE_R_RANGE_INDEX_2R			33

#define EPMEM_STMT_BIGTREE_RIT_ADD_TIME				10
#define EPMEM_STMT_BIGTREE_RIT_ADD_EPISODE			11
#define EPMEM_STMT_BIGTREE_RIT_DELETE_EPISODE		12
#define EPMEM_STMT_BIGTREE_RIT_ADD_ID				13
#define EPMEM_STMT_BIGTREE_RIT_FIND_ID				14
#define EPMEM_STMT_BIGTREE_RIT_FIND_ID_NULL			15
#define EPMEM_STMT_BIGTREE_RIT_VALID_EPISODE		16
#define EPMEM_STMT_BIGTREE_RIT_NEXT_EPISODE			17
#define EPMEM_STMT_BIGTREE_RIT_PREV_EPISODE			18
#define EPMEM_STMT_BIGTREE_RIT_ADD_WEIGHT			19
#define EPMEM_STMT_BIGTREE_RIT_TRUNCATE_WEIGHTS		20
#define EPMEM_STMT_BIGTREE_RIT_DEL_PROHIB			21
#define EPMEM_STMT_BIGTREE_RIT_DEL_PROHIB_LOW		22
#define EPMEM_STMT_BIGTREE_RIT_DEL_PROHIB_HIGH		23
#define EPMEM_STMT_BIGTREE_RIT_DEL_PROHIB_CONTAIN	24
#define EPMEM_STMT_BIGTREE_RIT_GET_LOW_RANGES		25
#define EPMEM_STMT_BIGTREE_RIT_GET_HIGH_RANGES		26
#define EPMEM_STMT_BIGTREE_RIT_TRUNCATE_RANGES		27
#define EPMEM_STMT_BIGTREE_RIT_ADD_LEFT				28
#define EPMEM_STMT_BIGTREE_RIT_TRUNCATE_LEFT		29
#define EPMEM_STMT_BIGTREE_RIT_ADD_RIGHT			30
#define EPMEM_STMT_BIGTREE_RIT_TRUNCATE_RIGHT		31
#define EPMEM_STMT_BIGTREE_RIT_GET_EPISODE			32
#define EPMEM_STMT_BIGTREE_RIT_ADD_NOW				33
#define EPMEM_STMT_BIGTREE_RIT_DELETE_NOW			34

#define EPMEM_MAX_STATEMENTS 						40 // must be at least 1+ largest of any STMT constant

#define EPMEM_MEMID_NONE							-1
#define EPMEM_MEMID_NOW								2147483647

#define EPMEM_PARENTID_ROOT							0

#define EPMEM_RIT_ROOT								0
#define EPMEM_LN_2									0.693147180559945

// names of params
#define EPMEM_PARAM_LEARNING						0
#define EPMEM_PARAM_DB								1
#define EPMEM_PARAM_PATH							2
#define EPMEM_PARAM_INDEXING						3
#define EPMEM_PARAM_PROVENANCE						4
#define EPMEM_PARAM_TRIGGER							5
#define EPMEM_PARAM_BALANCE							6
#define EPMEM_PARAMS								7 // must be 1+ last epmem param

// names of stats
#define EPMEM_STAT_TIME								0
#define EPMEM_STAT_MEM_USAGE						1
#define EPMEM_STAT_MEM_HIGH							2
#define EPMEM_STAT_RIT_OFFSET						3
#define EPMEM_STAT_RIT_LEFTROOT						4
#define EPMEM_STAT_RIT_RIGHTROOT					5
#define EPMEM_STAT_RIT_MINSTEP						6
#define EPMEM_STATS									7 // must be 1+ last epmem stat

//////////////////////////////////////////////////////////
// EpMem Types
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

typedef struct epmem_stat_struct
{
	double value;
	const char *name;
} epmem_stat;

typedef struct epmem_data_struct 
{
	unsigned long last_ol_time;		// last update to output-link
	unsigned long last_ol_count;	// last count of output-link

	unsigned long last_cmd_time;	// last update to epmem.command
	unsigned long last_cmd_count;	// last update to epmem.command

	int last_memory;				// last retrieved memory

	wme *ss_wme;

	std::list<wme *> *cue_wmes;		// wmes in last cue
	std::stack<wme *> *epmem_wmes;	// wmes in last epmem
} epmem_data;

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

// indexing
extern bool epmem_validate_indexing( const long new_val );
extern const char *epmem_convert_indexing( const long val );
extern const long epmem_convert_indexing( const char *val );

// provenance
extern bool epmem_validate_provenance( const long new_val );
extern const char *epmem_convert_provenance( const long val );
extern const long epmem_convert_provenance( const char *val );

// trigger
extern bool epmem_validate_trigger( const long new_val );
extern const char *epmem_convert_trigger( const long val );
extern const long epmem_convert_trigger( const char *val );

// balance
extern bool epmem_validate_balance( const double new_val );

// shortcut for determining if EpMem is enabled
extern bool epmem_enabled( agent *my_agent );

//////////////////////////////////////////////////////////
// Stats
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
extern double epmem_get_stat( agent *my_agent, const char *name );
extern double epmem_get_stat( agent *my_agent, const long stat );

// set stat
extern bool epmem_set_stat( agent *my_agent, const char *name, double new_val );
extern bool epmem_set_stat( agent *my_agent, const long stat, double new_val );

//////////////////////////////////////////////////////////
// Core Functions
//////////////////////////////////////////////////////////

// init, end
extern void epmem_reset( agent *my_agent, Symbol *state = NULL );
extern void epmem_end( agent *my_agent );

// Called to consider adding new episodes to the store
extern void epmem_consider_new_episode( agent *my_agent );

// Called to add a new episode to the store
extern void epmem_new_episode( agent *my_agent );

// Called to determine if a memory_id is valid
extern bool epmem_episode_exists( agent *my_agent, long memory_id );

// Called to determine the next episode id
extern long epmem_next_episode( agent *my_agent, long memory_id );

// Called to determine the previous episode id
extern long epmem_previous_episode( agent *my_agent, long memory_id );

// Called to react to commands
extern void epmem_respond_to_cmd( agent *my_agent );
extern void epmem_clear_result( agent *my_agent, Symbol *state );

// Called to install a particular memory into WM
extern void epmem_install_memory( agent *my_agent, Symbol *state, long memory_id );

// Called to get a particular wme augmentation
extern wme *epmem_get_aug_of_id( agent *my_agent, Symbol *sym, char *attr_name, char *value_name );

// Called to create/remove a fake preference for an epmem wme
extern preference *epmem_make_fake_preference( agent *my_agent, Symbol *state, wme *w );
extern void epmem_remove_fake_preference( agent *my_agent, wme *w );

#endif
