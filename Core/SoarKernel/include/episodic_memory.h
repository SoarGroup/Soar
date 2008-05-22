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

#include <string>

#include "symtab.h"
#include "wmem.h"

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

#define EPMEM_PROVENANCE_ON 1
#define EPMEM_PROVENANCE_OFF 2

#define EPMEM_TRIGGER_OUTPUT 1

// statement storage
// 0 - 9 => common
// 10 - 19 => indexing
// 20 - 29 => storage
#define EPMEM_STMT_BEGIN						0
#define EPMEM_STMT_COMMIT						1
#define EPMEM_STMT_ROLLBACK						2
#define EPMEM_STMT_VAR_GET						3
#define EPMEM_STMT_VAR_SET						4

#define EPMEM_STMT_BIGTREE_I_ADD_EPISODE		10
#define EPMEM_STMT_BIGTREE_I_ADD_ID				11
#define EPMEM_STMT_BIGTREE_I_FIND_ID			12
#define EPMEM_STMT_BIGTREE_I_FIND_ID_NULL		13

#define EPMEM_STMT_BIGTREE_R_UPDATE_EPISODE		10
#define EPMEM_STMT_BIGTREE_R_ADD_EPISODE		11
#define EPMEM_STMT_BIGTREE_R_ADD_ID				12
#define EPMEM_STMT_BIGTREE_R_FIND_ID			13
#define EPMEM_STMT_BIGTREE_R_FIND_ID_NULL		14

#define EPMEM_MAX_STATEMENTS 					30 // must be at least 1+ largest of any STMT constant

// names of params
#define EPMEM_PARAM_LEARNING					0
#define EPMEM_PARAM_DB							1
#define EPMEM_PARAM_PATH						2
#define EPMEM_PARAM_INDEXING					3
#define EPMEM_PARAM_PROVENANCE					4
#define EPMEM_PARAM_TRIGGER						5
#define EPMEM_PARAMS							6 // must be 1+ last epmem param

// names of stats
#define EPMEM_STAT_DUMMY						0
#define EPMEM_STATS								1 // must be 1+ last epmem stat

//
// These must go below constants
//

#include "stl_support.h"

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
	unsigned long last_tag;		// last update to output-link
} epmem_data;

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

// provenance
extern bool epmem_validate_trigger( const long new_val );
extern const char *epmem_convert_trigger( const long val );
extern const long epmem_convert_trigger( const char *val );

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

// init
extern void epmem_reset( agent *my_agent );

// Called to add a new episode to the store
extern void epmem_new_episode( agent *my_agent );

// Called to consider adding new episodes to the store
extern void epmem_consider_new_episode( agent *my_agent );

#endif
