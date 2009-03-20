#include <portability.h>

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/*************************************************************************
 *
 *  file:  semantic_memory.cpp
 *
 * =======================================================================
 * Description  :  Various functions for Soar-SMem
 * =======================================================================
 */

#include "semantic_memory.h"
#include "utilities.h"
#include "print.h"
#include "xml.h"

#include <cmath>

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Bookmark strings to help navigate the code
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

// parameters	 				smem::param
// stats 						smem::stats
// timers 						smem::timers

// wme-related					epmem::wmes
// preference-related 			epmem::prefs

// cleaning up					smem::clean


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Parameter Functions (smem::params)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : smem_clean_parameters
 * Author		: Nate Derbinsky
 * Notes		: Cleans up memory for parameters
 **************************************************************************/
void smem_clean_parameters( agent *my_agent )
{
	for ( int i=0; i<SMEM_PARAMS; i++ )
	{
		if ( my_agent->smem_params[ i ]->type == smem_param_string )
			delete my_agent->smem_params[ i ]->param->string_param.value;

		delete my_agent->smem_params[ i ]->param;
		delete my_agent->smem_params[ i ];
	}
}

/***************************************************************************
 * Function     : smem_new_parameter
 * Author		: Nate Derbinsky
 * Notes		: Creates a new parameter (of desired type)
 **************************************************************************/
smem_parameter *smem_new_parameter( const char *name, double value, bool (*val_func)( double ) )
{
	smem_parameter *newbie = new smem_parameter;
	newbie->param = new smem_parameter_union;
	newbie->param->number_param.value = value;
	newbie->param->number_param.val_func = val_func;
	newbie->type = smem_param_number;
	newbie->name = name;

	return newbie;
}

smem_parameter *smem_new_parameter( const char *name, const long value, bool (*val_func)( const long ), const char *(*to_str)( long ), const long (*from_str)( const char * ) )
{
	smem_parameter *newbie = new smem_parameter;
	newbie->param = new smem_parameter_union;
	newbie->param->constant_param.val_func = val_func;
	newbie->param->constant_param.to_str = to_str;
	newbie->param->constant_param.from_str = from_str;
	newbie->param->constant_param.value = value;
	newbie->type = smem_param_constant;
	newbie->name = name;

	return newbie;
}

smem_parameter *smem_new_parameter( const char *name, const char *value, bool (*val_func)( const char * ) )
{
	smem_parameter *newbie = new smem_parameter;
	newbie->param = new smem_parameter_union;
	newbie->param->string_param.value = new std::string( value );
	newbie->param->string_param.val_func = val_func;
	newbie->type = smem_param_string;
	newbie->name = name;

	return newbie;
}

/***************************************************************************
 * Function     : smem_convert_parameter
 * Author		: Nate Derbinsky
 * Notes		: Convert parameter name <=> constant
 **************************************************************************/
const char *smem_convert_parameter( agent *my_agent, const long param )
{
	if ( ( param < 0 ) || ( param >= SMEM_PARAMS ) )
		return NULL;

	return my_agent->smem_params[ param ]->name;
}

const long smem_convert_parameter( agent *my_agent, const char *name )
{
	for ( int i=0; i<SMEM_PARAMS; i++ )
		if ( !strcmp( name, my_agent->smem_params[ i ]->name ) )
			return i;

	return SMEM_PARAMS;
}

/***************************************************************************
 * Function     : smem_valid_parameter
 * Author		: Nate Derbinsky
 * Notes		: Determines if a parameter name/number is valid
 **************************************************************************/
bool smem_valid_parameter( agent *my_agent, const char *name )
{
	return ( smem_convert_parameter( my_agent, name ) != SMEM_PARAMS );
}

bool smem_valid_parameter( agent *my_agent, const long param )
{
	return ( smem_convert_parameter( my_agent, param ) != NULL );
}

/***************************************************************************
 * Function     : smem_get_parameter_type
 * Author		: Nate Derbinsky
 * Notes		: Returns the parameter type
 **************************************************************************/
smem_param_type smem_get_parameter_type( agent *my_agent, const char *name )
{
	const long param = smem_convert_parameter( my_agent, name );
	if ( param == SMEM_PARAMS )
		return smem_param_invalid;

	return my_agent->smem_params[ param ]->type;
}

smem_param_type smem_get_parameter_type( agent *my_agent, const long param )
{
	if ( !smem_valid_parameter( my_agent, param ) )
		return smem_param_invalid;

	return my_agent->smem_params[ param ]->type;
}

/***************************************************************************
 * Function     : smem_get_parameter
 * Author		: Nate Derbinsky
 * Notes		: Get the parameter value
 **************************************************************************/
const long smem_get_parameter( agent *my_agent, const char *name, const double /*test*/ )
{
	const long param = smem_convert_parameter( my_agent, name );
	if ( param == SMEM_PARAMS )
		return NULL;

	if ( smem_get_parameter_type( my_agent, param ) != smem_param_constant )
		return NULL;

	return my_agent->smem_params[ param ]->param->constant_param.value;
}

const char *smem_get_parameter( agent *my_agent, const char *name, const char * /*test*/ )
{
	const long param = smem_convert_parameter( my_agent, name );
	if ( param == SMEM_PARAMS )
		return NULL;

	if ( smem_get_parameter_type( my_agent, param ) == smem_param_string )
		return my_agent->smem_params[ param ]->param->string_param.value->c_str();
	if ( smem_get_parameter_type( my_agent, param ) != smem_param_constant )
		return NULL;

	return my_agent->smem_params[ param ]->param->constant_param.to_str( my_agent->smem_params[ param ]->param->constant_param.value );
}

double smem_get_parameter( agent *my_agent, const char *name )
{
	const long param = smem_convert_parameter( my_agent, name );
	if ( param == SMEM_PARAMS )
		return NULL;

	if ( smem_get_parameter_type( my_agent, param ) != smem_param_number )
		return NULL;

	return my_agent->smem_params[ param ]->param->number_param.value;
}

//

const long smem_get_parameter( agent *my_agent, const long param, const double /*test*/ )
{
	if ( !smem_valid_parameter( my_agent, param ) )
		return NULL;

	if ( smem_get_parameter_type( my_agent, param ) != smem_param_constant )
		return NULL;

	return my_agent->smem_params[ param ]->param->constant_param.value;
}

const char *smem_get_parameter( agent *my_agent, const long param, const char * /*test*/ )
{
	if ( !smem_valid_parameter( my_agent, param ) )
		return NULL;

	if ( smem_get_parameter_type( my_agent, param ) == smem_param_string )
		return my_agent->smem_params[ param ]->param->string_param.value->c_str();
	if ( smem_get_parameter_type( my_agent, param ) != smem_param_constant )
		return NULL;

	return my_agent->smem_params[ param ]->param->constant_param.to_str( my_agent->smem_params[ param ]->param->constant_param.value );
}

double smem_get_parameter( agent *my_agent, const long param )
{
	if ( !smem_valid_parameter( my_agent, param ) )
		return NULL;

	if ( smem_get_parameter_type( my_agent, param ) != smem_param_number )
		return NULL;

	return my_agent->smem_params[ param ]->param->number_param.value;
}

/***************************************************************************
 * Function     : smem_enabled
 * Author		: Nate Derbinsky
 * Notes		: Shortcut function to system parameter
 **************************************************************************/
bool smem_enabled( agent *my_agent )
{
	return ( my_agent->sysparams[ SMEM_ENABLED ] == SMEM_LEARNING_ON );
}

/***************************************************************************
 * Function     : smem_valid_parameter_value
 * Author		: Nate Derbinsky
 * Notes		: Returns if a value is valid for the parameter
 **************************************************************************/
bool smem_valid_parameter_value( agent *my_agent, const char *name, double new_val )
{
	const long param = smem_convert_parameter( my_agent, name );
	if ( param == SMEM_PARAMS )
		return false;

	if ( smem_get_parameter_type( my_agent, param ) != smem_param_number )
		return false;

	return my_agent->smem_params[ param ]->param->number_param.val_func( new_val );
}

bool smem_valid_parameter_value( agent *my_agent, const char *name, const char *new_val )
{
	const long param = smem_convert_parameter( my_agent, name );
	if ( param == SMEM_PARAMS )
		return false;

	if ( smem_get_parameter_type( my_agent, param ) == smem_param_string )
		return my_agent->smem_params[ param ]->param->string_param.val_func( new_val );
	if ( smem_get_parameter_type( my_agent, param ) != smem_param_constant )
		return false;

	return my_agent->smem_params[ param ]->param->constant_param.val_func( my_agent->smem_params[ param ]->param->constant_param.from_str( new_val ) );
}

bool smem_valid_parameter_value( agent *my_agent, const char *name, const long new_val )
{
	const long param = smem_convert_parameter( my_agent, name );
	if ( param == SMEM_PARAMS )
		return false;

	if ( smem_get_parameter_type( my_agent, param ) != smem_param_constant )
		return false;

	return my_agent->smem_params[ param ]->param->constant_param.val_func( new_val );
}

//

bool smem_valid_parameter_value( agent *my_agent, const long param, double new_val )
{
	if ( !smem_valid_parameter( my_agent, param ) )
		return false;

	if ( smem_get_parameter_type( my_agent, param ) != smem_param_number )
		return false;

	return my_agent->smem_params[ param ]->param->number_param.val_func( new_val );
}

bool smem_valid_parameter_value( agent *my_agent, const long param, const char *new_val )
{
	if ( !smem_valid_parameter( my_agent, param ) )
		return false;

	if ( smem_get_parameter_type( my_agent, param ) == smem_param_string )
		return my_agent->smem_params[ param ]->param->string_param.val_func( new_val );
	if ( smem_get_parameter_type( my_agent, param ) != smem_param_constant )
		return false;

	return my_agent->smem_params[ param ]->param->constant_param.val_func( my_agent->smem_params[ param ]->param->constant_param.from_str( new_val ) );
}

bool smem_valid_parameter_value( agent *my_agent, const long param, const long new_val )
{
	if ( !smem_valid_parameter( my_agent, param ) )
		return false;

	if ( smem_get_parameter_type( my_agent, param ) != smem_param_constant )
		return false;

	return my_agent->smem_params[ param ]->param->constant_param.val_func( new_val );
}

/***************************************************************************
 * Function     : smem_parameter_protected
 * Author		: Nate Derbinsky
 * Notes		: Returns true if a parameter is currently protected
 * 				  from modification
 **************************************************************************/
bool smem_parameter_protected( agent *my_agent, const long param )
{
	return ( ( my_agent->smem_db_status != SMEM_DB_CLOSED ) && ( param >= SMEM_PARAM_DB ) && ( param <= SMEM_PARAM_PATH ) );
}

/***************************************************************************
 * Function     : smem_set_parameter
 * Author		: Nate Derbinsky
 * Notes		: Set parameter value
 * 				  Special considerations for commit, exclusions
 * 				  learning, graph-match
 **************************************************************************/
bool smem_set_parameter( agent *my_agent, const char *name, double new_val )
{
	const long param = smem_convert_parameter( my_agent, name );
	if ( param == SMEM_PARAMS )
		return false;

	if ( smem_parameter_protected( my_agent, param ) )
		return false;

	// special case of commit needing conversion to int
	if ( param == SMEM_PARAM_COMMIT )
		new_val = floor( new_val );

	if ( !smem_valid_parameter_value( my_agent, param, new_val ) )
		return false;

	my_agent->smem_params[ param ]->param->number_param.value = new_val;

	return true;
}

bool smem_set_parameter( agent *my_agent, const char *name, const char *new_val )
{
	const long param = smem_convert_parameter( my_agent, name );
	if ( param == SMEM_PARAMS )
		return false;

	if ( smem_parameter_protected( my_agent, param ) )
		return false;

	if ( !smem_valid_parameter_value( my_agent, param, new_val ) )
		return false;	

	if ( smem_get_parameter_type( my_agent, param ) == smem_param_string )
	{
		// path special case
		if ( param == SMEM_PARAM_PATH )
		{
			if ( my_agent->smem_first_switch )
			{
				my_agent->smem_first_switch = false;
				my_agent->smem_params[ SMEM_PARAM_DB ]->param->constant_param.value = SMEM_DB_FILE;

				const char *msg = "Database set to file";
				print( my_agent, const_cast<char *>( msg ) );
				xml_generate_message( my_agent, const_cast<char *>( msg ) );
			}
		}

		(*my_agent->smem_params[ param ]->param->string_param.value) = new_val;
		return true;
	}

	const long converted_val = my_agent->smem_params[ param ]->param->constant_param.from_str( new_val );

	// learning special case
	if ( param == SMEM_PARAM_LEARNING )
	{
		set_sysparam( my_agent, SMEM_ENABLED, converted_val );
	}	

	my_agent->smem_params[ param ]->param->constant_param.value = converted_val;

	return true;
}

bool smem_set_parameter( agent *my_agent, const char *name, const long new_val )
{
	const long param = smem_convert_parameter( my_agent, name );
	if ( param == SMEM_PARAMS )
		return false;

	if ( smem_parameter_protected( my_agent, param ) )
		return false;

	if ( !smem_valid_parameter_value( my_agent, param, new_val ) )
		return false;

	// learning special case
	if ( param == SMEM_PARAM_LEARNING )
	{
		set_sysparam( my_agent, SMEM_ENABLED, new_val );
	}	

	my_agent->smem_params[ param ]->param->constant_param.value = new_val;

	return true;
}

//

bool smem_set_parameter( agent *my_agent, const long param, double new_val )
{
	if ( smem_parameter_protected( my_agent, param ) )
		return false;

	// special case of commit needing conversion to int
	if ( param == SMEM_PARAM_COMMIT )
		new_val = floor( new_val );

	if ( !smem_valid_parameter_value( my_agent, param, new_val ) )
		return false;

	my_agent->smem_params[ param ]->param->number_param.value = new_val;

	return true;
}

bool smem_set_parameter( agent *my_agent, const long param, const char *new_val )
{
	if ( smem_parameter_protected( my_agent, param ) )
		return false;

	if ( !smem_valid_parameter_value( my_agent, param, new_val ) )
		return false;


	if ( smem_get_parameter_type( my_agent, param ) == smem_param_string )
	{
		// path special case
		if ( param == SMEM_PARAM_PATH )
		{
			if ( my_agent->smem_first_switch )
			{
				my_agent->smem_first_switch = false;
				my_agent->smem_params[ SMEM_PARAM_DB ]->param->constant_param.value = SMEM_DB_FILE;

				const char *msg = "Database set to file";
				print( my_agent, const_cast<char *>( msg ) );
				xml_generate_message( my_agent, const_cast<char *>( msg ) );
			}
		}

		(*my_agent->smem_params[ param ]->param->string_param.value) = new_val;
		return true;
	}

	const long converted_val = my_agent->smem_params[ param ]->param->constant_param.from_str( new_val );

	// learning special case
	if ( param == SMEM_PARAM_LEARNING )
	{
		set_sysparam( my_agent, SMEM_ENABLED, converted_val );
	}	

	my_agent->smem_params[ param ]->param->constant_param.value = converted_val;

	return true;
}

bool smem_set_parameter( agent *my_agent, const long param, const long new_val )
{
	if ( smem_parameter_protected( my_agent, param ) )
		return false;

	if ( !smem_valid_parameter_value( my_agent, param, new_val ) )
		return false;

	// learning special case
	if ( param == SMEM_PARAM_LEARNING )
	{
		set_sysparam( my_agent, SMEM_ENABLED, new_val );
	}	

	my_agent->smem_params[ param ]->param->constant_param.value = new_val;

	return true;
}

//
// validation/conversion functions for parameters
//

// learning parameter
bool smem_validate_learning( const long new_val )
{
	return ( ( new_val == SMEM_LEARNING_ON ) || ( new_val == SMEM_LEARNING_OFF ) );
}

const char *smem_convert_learning( const long val )
{
	const char *return_val = NULL;

	switch ( val )
	{
		case SMEM_LEARNING_ON:
			return_val = "on";
			break;

		case SMEM_LEARNING_OFF:
			return_val = "off";
			break;
	}

	return return_val;
}

const long smem_convert_learning( const char *val )
{
	long return_val = NULL;

	if ( !strcmp( val, "on" ) )
		return_val = SMEM_LEARNING_ON;
	else if ( !strcmp( val, "off" ) )
		return_val = SMEM_LEARNING_OFF;

	return return_val;
}

// database parameter
bool smem_validate_database( const long new_val )
{
	return ( ( new_val == SMEM_DB_MEM ) || ( new_val == SMEM_DB_FILE ) );
}

const char *smem_convert_database( const long val )
{
	const char *return_val = NULL;

	switch ( val )
	{
		case SMEM_DB_MEM:
			return_val = "memory";
			break;

		case SMEM_DB_FILE:
			return_val = "file";
			break;
	}

	return return_val;
}

const long smem_convert_database( const char *val )
{
	long return_val = NULL;

	if ( !strcmp( val, "memory" ) )
		return_val = SMEM_DB_MEM;
	else if ( !strcmp( val, "file" ) )
		return_val = SMEM_DB_FILE;

	return return_val;
}

// path parameter
bool smem_validate_path( const char * /*new_val*/ )
{
	return true;
}

// commit parameter
bool smem_validate_commit( const double new_val )
{
	return ( new_val > 0 );
}

// timers parameter
bool smem_validate_ext_timers( const long new_val )
{
	return ( ( new_val >= SMEM_TIMERS_OFF ) && ( new_val <= SMEM_TIMERS_THREE ) );
}

const char *smem_convert_ext_timers( const long val )
{
	const char *return_val = NULL;

	switch ( val )
	{
		case SMEM_TIMERS_OFF:
			return_val = "off";
			break;

		case SMEM_TIMERS_ONE:
			return_val = "one";
			break;

		case SMEM_TIMERS_TWO:
			return_val = "two";
			break;

		case SMEM_TIMERS_THREE:
			return_val = "three";
			break;
	}

	return return_val;
}

const long smem_convert_ext_timers( const char *val )
{
	long return_val = NULL;

	if ( !strcmp( val, "off" ) )
		return_val = SMEM_TIMERS_OFF;
	else if ( !strcmp( val, "one" ) )
		return_val = SMEM_TIMERS_ONE;
	else if ( !strcmp( val, "two" ) )
		return_val = SMEM_TIMERS_TWO;
	else if ( !strcmp( val, "three" ) )
		return_val = SMEM_TIMERS_THREE;

	return return_val;
}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Statistic Functions (smem::stats)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : smem_clean_stats
 * Author		: Nate Derbinsky
 * Notes		: Cleans up memory for statistics
 **************************************************************************/
void smem_clean_stats( agent *my_agent )
{
	for ( int i=0; i<SMEM_STATS; i++ )
	  delete my_agent->smem_stats[ i ];
}

/***************************************************************************
 * Function     : smem_stat_protected
 * Author		: Nate Derbinsky
 * Notes		: Determines if a statistic is protected from resetting
 **************************************************************************/
bool smem_stat_protected( agent *my_agent, const long stat )
{
	return ( ( my_agent->smem_db_status != SMEM_DB_CLOSED ) &&		     
			 ( stat == SMEM_STAT_NEXT_ID ) );
}

/***************************************************************************
 * Function     : smem_reset_stats
 * Author		: Nate Derbinsky
 * Notes		: Reesets unprotected statistics
 **************************************************************************/
void smem_reset_stats( agent *my_agent )
{
	for ( int i=0; i<SMEM_STATS; i++ )
		if ( !smem_stat_protected( my_agent, i ) )
			my_agent->smem_stats[ i ]->value = 0;
}

/***************************************************************************
 * Function     : smem_new_stat
 * Author		: Nate Derbinsky
 * Notes		: Creates a new statistic
 **************************************************************************/
smem_stat *smem_new_stat( const char *name )
{
	smem_stat *newbie = new smem_stat;
	newbie->name = name;
	newbie->value = 0;

	return newbie;
}

/***************************************************************************
 * Function     : smem_convert_stat
 * Author		: Nate Derbinsky
 * Notes		: Convert between statistic name<=>number
 **************************************************************************/
const long smem_convert_stat( agent *my_agent, const char *name )
{
	for ( int i=0; i<SMEM_STATS; i++ )
		if ( !strcmp( name, my_agent->smem_stats[ i ]->name ) )
			return i;

	return SMEM_STATS;
}

const char *smem_convert_stat( agent *my_agent, const long stat )
{
	if ( ( stat < 0 ) || ( stat >= SMEM_STATS ) )
		return NULL;

	return my_agent->smem_stats[ stat ]->name;
}

/***************************************************************************
 * Function     : smem_valid_stat
 * Author		: Nate Derbinsky
 * Notes		: Return if a statistic name/number is valid
 **************************************************************************/
bool smem_valid_stat( agent *my_agent, const char *name )
{
	return ( smem_convert_stat( my_agent, name ) != SMEM_STATS );
}

bool smem_valid_stat( agent *my_agent, const long stat )
{
	return ( smem_convert_stat( my_agent, stat ) != NULL );
}

/***************************************************************************
 * Function     : smem_get_stat
 * Author		: Nate Derbinsky
 * Notes		: Get a statistic value
 * 				  Special consideration for sqlite info
 **************************************************************************/
EPMEM_TYPE_INT smem_get_stat( agent *my_agent, const char *name )
{
	const long stat = smem_convert_stat( my_agent, name );
	if ( stat == SMEM_STATS )
		return 0;

	if ( stat == SMEM_STAT_MEM_USAGE )
		return (EPMEM_TYPE_INT) sqlite3_memory_used();
	if ( stat == SMEM_STAT_MEM_HIGH )
		return (EPMEM_TYPE_INT) sqlite3_memory_highwater( false );

	return my_agent->smem_stats[ stat ]->value;
}

EPMEM_TYPE_INT smem_get_stat( agent *my_agent, const long stat )
{
	if ( !smem_valid_stat( my_agent, stat ) )
		return 0;

	if ( stat == SMEM_STAT_MEM_USAGE )
		return (EPMEM_TYPE_INT) sqlite3_memory_used();
	if ( stat == SMEM_STAT_MEM_HIGH )
		return (EPMEM_TYPE_INT) sqlite3_memory_highwater( false );

	return my_agent->smem_stats[ stat ]->value;
}

/***************************************************************************
 * Function     : smem_set_stat
 * Author		: Nate Derbinsky
 * Notes		: Set a statistic value
 * 				  Special consideration for sqlite info
 **************************************************************************/
bool smem_set_stat( agent *my_agent, const char *name, EPMEM_TYPE_INT new_val )
{
	const long stat = smem_convert_stat( my_agent, name );
	if ( ( stat == SMEM_STATS ) ||
		 ( stat == SMEM_STAT_MEM_USAGE ) ||
		 ( stat == SMEM_STAT_MEM_HIGH ) )
		return false;

	my_agent->smem_stats[ stat ]->value = new_val;

	return true;
}

bool smem_set_stat( agent *my_agent, const long stat, EPMEM_TYPE_INT new_val )
{
	if ( !smem_valid_stat( my_agent, stat ) )
		return false;

	if ( ( stat == SMEM_STAT_MEM_USAGE ) ||
		 ( stat == SMEM_STAT_MEM_HIGH ) )
		return false;

	my_agent->smem_stats[ stat ]->value = new_val;

	return true;
}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Timer Functions (smem::timers)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : smem_clean_timers
 * Author		: Nate Derbinsky
 * Notes		: Cleans up memory for timers
 **************************************************************************/
void smem_clean_timers( agent *my_agent )
{
	for ( int i=0; i<SMEM_TIMERS; i++ )
		delete my_agent->smem_timers[ i ];
}

/***************************************************************************
 * Function     : smem_reset_timers
 * Author		: Nate Derbinsky
 * Notes		: Resets all timers
 **************************************************************************/
void smem_reset_timers( agent *my_agent )
{
	for ( int i=0; i<SMEM_TIMERS; i++ )
	{
		reset_timer( &my_agent->smem_timers[ i ]->start_timer );
		reset_timer( &my_agent->smem_timers[ i ]->total_timer );
	}
}

/***************************************************************************
 * Function     : smem_new_timer
 * Author		: Nate Derbinsky
 * Notes		: Creates a new initialized timer
 **************************************************************************/
smem_timer *smem_new_timer( const char *name, long level )
{
	// new timer entry
	smem_timer *newbie = new smem_timer;
	newbie->name = name;
	newbie->level = level;

	reset_timer( &newbie->start_timer );
	reset_timer( &newbie->total_timer );

	return newbie;
}

/***************************************************************************
 * Function     : smem_convert_timer
 * Author		: Nate Derbinsky
 * Notes		: Convert a timer name<=>number
 **************************************************************************/
const long smem_convert_timer( agent *my_agent, const char *name )
{
	for ( int i=0; i<SMEM_TIMERS; i++ )
		if ( !strcmp( name, my_agent->smem_timers[ i ]->name ) )
			return i;

	return SMEM_TIMERS;
}

const char *smem_convert_timer( agent *my_agent, const long timer )
{
	if ( ( timer < 0 ) || ( timer >= SMEM_TIMERS ) )
		return NULL;

	return my_agent->smem_timers[ timer ]->name;
}

/***************************************************************************
 * Function     : smem_valid_timer
 * Author		: Nate Derbinsky
 * Notes		: Determines if the timer name/number is valid
 **************************************************************************/
bool smem_valid_timer( agent *my_agent, const char *name )
{
	return ( smem_convert_timer( my_agent, name ) != SMEM_TIMERS );
}

bool smem_valid_timer( agent *my_agent, const long timer )
{
	return ( smem_convert_timer( my_agent, timer ) != NULL );
}

/***************************************************************************
 * Function     : smem_get_timer_value
 * Author		: Nate Derbinsky
 * Notes		: Returns the current value of the timer
 **************************************************************************/
double smem_get_timer_value( agent *my_agent, const char *name )
{
	const long timer = smem_convert_timer( my_agent, name );
	if ( timer == SMEM_TIMERS )
		return 0.0;

	return timer_value( &my_agent->smem_timers[ timer ]->total_timer );
}

double smem_get_timer_value( agent *my_agent, const long timer )
{
	if ( !smem_valid_timer( my_agent, timer ) )
		return 0.0;

	return timer_value( &my_agent->smem_timers[ timer ]->total_timer );
}

/***************************************************************************
 * Function     : smem_get_timer_name
 * Author		: Nate Derbinsky
 * Notes		: Returns the timer name
 **************************************************************************/
const char *smem_get_timer_name( agent *my_agent, const char *name )
{
	const long timer = smem_convert_timer( my_agent, name );
	if ( timer == SMEM_TIMERS )
		return 0;

	return my_agent->smem_timers[ timer ]->name;
}

const char *smem_get_timer_name( agent *my_agent, const long timer )
{
	if ( !smem_valid_timer( my_agent, timer ) )
		return 0;

	return my_agent->smem_timers[ timer ]->name;
}

/***************************************************************************
 * Function     : smem_start_timer
 * Author		: Nate Derbinsky
 * Notes		: Starts a timer
 **************************************************************************/
void smem_start_timer( agent *my_agent, const long timer )
{
	if ( smem_valid_timer( my_agent, timer ) && ( smem_get_parameter( my_agent, SMEM_PARAM_TIMERS, SMEM_RETURN_LONG ) >= my_agent->smem_timers[ timer ]->level ) )
	{
		start_timer( my_agent, &my_agent->smem_timers[ timer ]->start_timer );
	}
}

/***************************************************************************
 * Function     : smem_stop_timer
 * Author		: Nate Derbinsky
 * Notes		: Stops a timer
 **************************************************************************/
void smem_stop_timer( agent *my_agent, const long timer )
{
	if ( smem_valid_timer( my_agent, timer ) && ( smem_get_parameter( my_agent, SMEM_PARAM_TIMERS, SMEM_RETURN_LONG ) >= my_agent->smem_timers[ timer ]->level ) )
	{
		stop_timer( my_agent, &my_agent->smem_timers[ timer ]->start_timer, &my_agent->smem_timers[ timer ]->total_timer );
	}
}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// WME Functions (smem::wmes)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : smem_get_augs_of_id
 * Author		: Andy Nuxoll
 * Notes		: This routine works just like the one defined in utilities.h.
 *				  Except this one does not use C++ templates because I have an
 *				  irrational dislike for them borne from the years when the STL
 *				  highly un-portable.  I'm told this is no longer true but I'm
 *				  still bitter.
 **************************************************************************/
wme **smem_get_augs_of_id( agent* my_agent, Symbol * id, tc_number tc, unsigned EPMEM_TYPE_INT *num_attr )
{
	slot *s;
	wme *w;
	wme **list;
	unsigned EPMEM_TYPE_INT list_position;
	unsigned EPMEM_TYPE_INT n = 0;

	// augs only exist for identifiers
	if ( id->common.symbol_type != IDENTIFIER_SYMBOL_TYPE )
		return NULL;

	// don't want to get into a loop
	if ( id->id.tc_num == tc )
		return NULL;
	id->id.tc_num = tc;

	// first count number of augs, required for later allocation
	for ( w = id->id.impasse_wmes; w != NIL; w = w->next )
		n++;
	for ( w = id->id.input_wmes; w != NIL; w = w->next )
		n++;
	for ( s = id->id.slots; s != NIL; s = s->next )
	{
		for ( w = s->wmes; w != NIL; w = w->next )
			n++;
		for ( w = s->acceptable_preference_wmes; w != NIL; w = w->next )
			n++;
	}

	// allocate the list, note the size
	list = static_cast<wme**>(allocate_memory( my_agent, n * sizeof(wme *), MISCELLANEOUS_MEM_USAGE));
	( *num_attr ) = n;

	list_position = 0;
	for ( w = id->id.impasse_wmes; w != NIL; w = w->next )
       list[ list_position++ ] = w;
	for ( w = id->id.input_wmes; w != NIL; w = w->next )
		list[ list_position++ ] = w;
	for ( s = id->id.slots; s != NIL; s = s->next )
	{
		for ( w = s->wmes; w != NIL; w = w->next )
           list[ list_position++ ] = w;
		for ( w = s->acceptable_preference_wmes; w != NIL; w = w->next )
			list[ list_position++ ] = w;
	}

	return list;
}

/***************************************************************************
 * Function     : smem_wme_has_value
 * Author		: Andy Nuxoll
 * Notes		: This routine returns TRUE if the given WMEs attribute
 *                and value are both symbols and have the names given.
 *                If either of the given names are NULL then they are
 *                assumed to be a match (i.e., a wildcard).  Thus passing
 *                NULL for both attr_name and value_name will always yield
 *                a TRUE result.
 **************************************************************************/
bool smem_wme_has_value( wme *w, char *attr_name, char *value_name )
{
	if ( w == NULL )
		return false;

	if ( attr_name != NULL )
	{
		if ( w->attr->common.symbol_type != SYM_CONSTANT_SYMBOL_TYPE )
			return false;

		if ( strcmp( w->attr->sc.name, attr_name ) != 0 )
			return false;
	}

    if ( value_name != NULL )
	{
		if ( w->value->common.symbol_type != SYM_CONSTANT_SYMBOL_TYPE )
			return false;

		if ( strcmp( w->attr->sc.name, value_name ) != 0 )
			return false;
	}

	return true;
}

/***************************************************************************
 * Function     : smem_get_aug_of_id
 * Author		: Andy Nuxoll
 * Notes		: This routine examines a symbol for an augmentation that
 *				  has the given attribute and value and returns it.  See
 *                epmem_wme_has_value() for info on how the correct wme is
 *                matched to the given strings.
 **************************************************************************/
wme *smem_get_aug_of_id( agent *my_agent, Symbol *sym, char *attr_name, char *value_name )
{
	wme **wmes;
	wme *return_val = NULL;

	unsigned EPMEM_TYPE_INT len = 0;
	unsigned EPMEM_TYPE_INT i;

	wmes = smem_get_augs_of_id( my_agent, sym, get_new_tc_number( my_agent ), &len );
	if ( wmes == NULL )
		return return_val;

	for ( i=0; i<len; i++ )
	{
		if ( smem_wme_has_value( wmes[ i ], attr_name, value_name ) )
		{
			return_val = wmes[ i ];
			break;
		}
	}

	free_memory( my_agent, wmes, MISCELLANEOUS_MEM_USAGE );

	return return_val;
}

/***************************************************************************
 * Function     : smem_symbol_to_string
 * Author		: Nate Derbinsky
 * Notes		: Converts any constant symbol type to a string
 **************************************************************************/
const char *smem_symbol_to_string( agent *my_agent, Symbol *sym )
{
	switch( sym->common.symbol_type )
	{
		case SYM_CONSTANT_SYMBOL_TYPE:
		case INT_CONSTANT_SYMBOL_TYPE:
		case FLOAT_CONSTANT_SYMBOL_TYPE:
			return symbol_to_string( my_agent, sym, false, NULL, NULL );
	}

	return "";
}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Preference Functions (smem::prefs)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : smem_make_fake_preference
 * Author		: Andy Nuxoll/Nate Derbinsky
 * Notes		: This function adds a fake preference to a WME so that
 *                it will not be added to the goal dependency set of the
 *                state it is attached to.  This is used to prevents the
 *                GDS from removing a state whenever a epmem is retrieved
 *                that is attached to it.
 *
 *                The conditions of the preference refer to the WMEs
 *                in the current cue.
 *
 *                (The bulk of the content of this function is taken from
 *                 make_fake_preference_for_goal_item() in decide.c)
 **************************************************************************/
preference *smem_make_fake_preference( agent *my_agent, Symbol *state, wme *w )
{
	// if we are on the top state, don't make the preference
	if ( state->id.smem_info->ss_wme == NULL )
		return NIL;

	// make fake preference
	preference *pref = make_preference( my_agent, ACCEPTABLE_PREFERENCE_TYPE, w->id, w->attr, w->value, NIL );
	pref->o_supported = TRUE;
	symbol_add_ref( pref->id );
	symbol_add_ref( pref->attr );
	symbol_add_ref( pref->value );

	// add preference to goal list
	insert_at_head_of_dll( state->id.preferences_from_goal, pref, all_of_goal_next, all_of_goal_prev );
	pref->on_goal_list = TRUE;

	// add reference
	preference_add_ref( pref );

	// make fake instantiation
	instantiation *inst;
	allocate_with_pool( my_agent, &( my_agent->instantiation_pool ), &inst );
	pref->inst = inst;
	pref->inst_next = pref->inst_prev = NULL;
	inst->preferences_generated = pref;
	inst->prod = NULL;
	inst->next = inst->prev = NULL;
	inst->rete_token = NULL;
	inst->rete_wme = NULL;
	inst->match_goal = state;
	inst->match_goal_level = state->id.level;
	inst->okay_to_variablize = TRUE;
	inst->backtrace_number = 0;
	inst->in_ms = FALSE;

	// create a condition for each cue WME (superstate if no cue)
	bool no_cue = state->id.smem_info->cue_wmes->empty();
	condition *cond = NULL;
	condition *prev_cond = NULL;
	if ( no_cue )
		state->id.smem_info->cue_wmes->insert( state->id.smem_info->ss_wme );
	{
		std::set<wme *>::iterator p = state->id.smem_info->cue_wmes->begin();

		while ( p != state->id.smem_info->cue_wmes->end() )
		{
			// construct the condition
			allocate_with_pool( my_agent, &( my_agent->condition_pool ), &cond );
			cond->type = POSITIVE_CONDITION;
			cond->prev = prev_cond;
			cond->next = NULL;
			if ( prev_cond != NULL )
			{
				prev_cond->next = cond;
			}
			else
			{
				inst->top_of_instantiated_conditions = cond;
				inst->bottom_of_instantiated_conditions = cond;
				inst->nots = NULL;
			}
			cond->data.tests.id_test = make_equality_test( (*p)->id );
			cond->data.tests.attr_test = make_equality_test( (*p)->attr );
			cond->data.tests.value_test = make_equality_test( (*p)->value );
			cond->test_for_acceptable_preference = TRUE;
			cond->bt.wme_ = (*p);
			wme_add_ref( (*p) );
			cond->bt.level = (*p)->id->id.level;
			cond->bt.trace = (*p)->preference;
			if ( cond->bt.trace )
				preference_add_ref( cond->bt.trace );
			cond->bt.prohibits = NULL;

			prev_cond = cond;

			p++;
		}
	}
	if ( no_cue )
		state->id.smem_info->cue_wmes->clear();

    return pref;
}

/***************************************************************************
 * Function     : smem_remove_fake_preference
 * Author		: Andy Nuxoll
 * Notes		: This function removes a fake preference on a WME
 *                created by epmem_make_fake_preference().  While it's
 *                a one-line function I thought it was important to
 *                create so it would be clear what's going on in this
 *                case.
 **************************************************************************/
void smem_remove_fake_preference( agent *my_agent, wme *w )
{
	if ( w->preference )
		preference_remove_ref( my_agent, w->preference );
}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Clean-Up Functions (smem::clean)
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : smem_close
 * Author		: Nate Derbinsky
 * Notes		: Performs cleanup operations when the database needs
 * 				  to be closed (end soar, manual close, etc)
 **************************************************************************/
void smem_close( agent *my_agent )
{
	if ( my_agent->smem_db_status != SMEM_DB_CLOSED )
	{
		// end any pending transactions
		/*
		if ( epmem_in_transaction( my_agent ) )
			epmem_transaction_end( my_agent, true );
		*/

		// deallocate query statements
		/*
		for ( int i=0; i<EPMEM_MAX_STATEMENTS; i++ )
		{
			if ( my_agent->epmem_statements[ i ] != NULL )
			{
				sqlite3_finalize( my_agent->epmem_statements[ i ] );
				my_agent->epmem_statements[ i ] = NULL;
			}
		}
		*/

		// close the database
		//sqlite3_close( my_agent->epmem_db );

		// initialize database status
		//my_agent->epmem_db = NULL;
		//my_agent->epmem_db_status = EPMEM_DB_CLOSED;
	}
}

/***************************************************************************
 * Function     : smem_clear_result
 * Author		: Nate Derbinsky
 * Notes		: Removes any WMEs produced by SMem resulting from
 * 				  a command
 **************************************************************************/
void smem_clear_result( agent *my_agent, Symbol *state )
{
	while ( !state->id.smem_info->smem_wmes->empty() )
	{
		smem_remove_fake_preference( my_agent, state->id.smem_info->smem_wmes->top() );
		remove_input_wme( my_agent, state->id.smem_info->smem_wmes->top() );
		state->id.smem_info->smem_wmes->pop();
	}
}

/***************************************************************************
 * Function     : smem_reset
 * Author		: Nate Derbinsky
 * Notes		: Performs cleanup when a state is removed
 **************************************************************************/
void smem_reset( agent *my_agent, Symbol *state )
{
	if ( state == NULL )
		state = my_agent->top_goal;

	while( state )
	{
		smem_data *data = state->id.smem_info;
		
		data->last_cmd_time = 0;
		data->last_cmd_count = 0;		

		data->cue_wmes->clear();

		// clear off any result stuff (takes care of smem_wmes)
		smem_clear_result( my_agent, state );

		// remove fake preferences
		smem_remove_fake_preference( my_agent, state->id.smem_wme );
		smem_remove_fake_preference( my_agent, state->id.smem_cmd_wme );
		smem_remove_fake_preference( my_agent, state->id.smem_result_wme );

		state = state->id.lower_goal;
	}
}

