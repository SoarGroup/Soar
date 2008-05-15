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

#include "symtab.h"
#include "wmem.h"

//////////////////////////////////////////////////////////
// EpMem Constants
//////////////////////////////////////////////////////////
#define EPMEM_RETURN_LONG 0.1
#define EPMEM_RETURN_STRING ""

#define EPMEM_LEARNING_ON 1
#define EPMEM_LEARNING_OFF 2

// names of params
#define EPMEM_PARAM_LEARNING					0
#define EPMEM_PARAMS							1 // must be 1+ last epmem param

// names of stats
#define EPMEM_STAT_DUMMY					0
#define EPMEM_STATS							1 // must be 1+ last epmem stat

//
// These must go below constants
//

#include "stl_support.h"

//////////////////////////////////////////////////////////
// EpMem Types
//////////////////////////////////////////////////////////
enum epmem_param_type { epmem_param_string = 1, epmem_param_number = 2, epmem_param_invalid = 3 };

typedef struct epmem_string_parameter_struct  
{
	long value;
	bool (*val_func)( const long );
	const char *(*to_str)( const long );
	const long (*from_str)( const char * );
} epmem_string_parameter;

typedef struct epmem_number_parameter_struct  
{
	double value;
	bool (*val_func)( double );
} epmem_number_parameter;

typedef union epmem_parameter_union_class
{
	epmem_string_parameter string_param;
	epmem_number_parameter number_param;
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

// Grand Central Station of EpMem
extern void epmem_update( agent *my_agent );

#endif
