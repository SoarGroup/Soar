#include <portability.h>

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/*************************************************************************
 *
 *  file:  wma.cpp
 *
 * =======================================================================
 * Description  :  Various functions for WMA
 * =======================================================================
 */

#include "wma.h"

#include <cmath>
#include <stdlib.h>

#include "wmem.h"
#include "instantiations.h"
#include "explain.h"
#include "rete.h"
#include "decide.h"
#include "prefmem.h"
#include "soar_rand.h"

#include "misc.h"

#include "print.h"

using namespace std;

/***************************************************************************
 * Function     : wma_clean_parameters
 **************************************************************************/
void wma_clean_parameters( agent *my_agent )
{
	for ( int i=0; i<WMA_PARAMS; i++ )
	{
		if ( my_agent->wma_params[ i ]->type == wma_param_string )
			delete my_agent->wma_params[ i ]->param->string_param.value;
		
		delete my_agent->wma_params[ i ]->param;
		delete my_agent->wma_params[ i ];
	}
}

/***************************************************************************
 * Function     : wma_add_parameter
 **************************************************************************/
wma_parameter *wma_add_parameter( const char *name, double value, bool (*val_func)( double ) )
{
	// new parameter entry
	wma_parameter *newbie = new wma_parameter;
	newbie->param = new wma_parameter_union;
	newbie->param->number_param.value = value;
	newbie->param->number_param.val_func = val_func;
	newbie->type = wma_param_number;
	newbie->name = name;
	
	return newbie;
}

wma_parameter *wma_add_parameter( const char *name, const long value, bool (*val_func)( const long ), const char *(*to_str)( long ), const long (*from_str)( const char * ) )
{
	// new parameter entry
	wma_parameter *newbie = new wma_parameter;
	newbie->param = new wma_parameter_union;
	newbie->param->constant_param.val_func = val_func;
	newbie->param->constant_param.to_str = to_str;
	newbie->param->constant_param.from_str = from_str;
	newbie->param->constant_param.value = value;
	newbie->type = wma_param_constant;
	newbie->name = name;
	
	return newbie;
}

wma_parameter *wma_add_parameter( const char *name, const char *value, bool (*val_func)( const char * ) )
{
	// new parameter entry
	wma_parameter *newbie = new wma_parameter;
	newbie->param = new wma_parameter_union;
	newbie->param->string_param.value = new string( value );
	newbie->param->string_param.val_func = val_func;
	newbie->type = wma_param_string;
	newbie->name = name;
	
	return newbie;
}

/***************************************************************************
 * Function     : wma_convert_parameter
 **************************************************************************/
const char *wma_convert_parameter( agent *my_agent, const long param )
{
	if ( ( param < 0 ) || ( param >= WMA_PARAMS ) )
		return NULL;

	return my_agent->wma_params[ param ]->name;
}

const long wma_convert_parameter( agent *my_agent, const char *name )
{
	for ( int i=0; i<WMA_PARAMS; i++ )
		if ( !strcmp( name, my_agent->wma_params[ i ]->name ) )
			return i;

	return WMA_PARAMS;
}

/***************************************************************************
 * Function     : wma_valid_parameter
 **************************************************************************/
bool wma_valid_parameter( agent *my_agent, const char *name )
{
	return ( wma_convert_parameter( my_agent, name ) != WMA_PARAMS );
}

bool wma_valid_parameter( agent *my_agent, const long param )
{
	return ( wma_convert_parameter( my_agent, param ) != NULL );
}

/***************************************************************************
 * Function     : wma_get_parameter_type
 **************************************************************************/
wma_param_type wma_get_parameter_type( agent *my_agent, const char *name )
{
	const long param = wma_convert_parameter( my_agent, name );
	if ( param == WMA_PARAMS )
		return wma_param_invalid;
	
	return my_agent->wma_params[ param ]->type;
}

wma_param_type wma_get_parameter_type( agent *my_agent, const long param )
{
	if ( !wma_valid_parameter( my_agent, param ) )
		return wma_param_invalid;

	return my_agent->wma_params[ param ]->type;
}

/***************************************************************************
 * Function     : wma_get_parameter
 **************************************************************************/
const long wma_get_parameter( agent *my_agent, const char *name, const double /*test*/ )
{
	const long param = wma_convert_parameter( my_agent, name );
	if ( param == WMA_PARAMS )
		return NULL;
	
	if ( wma_get_parameter_type( my_agent, param ) != wma_param_constant )
		return NULL;
	
	return my_agent->wma_params[ param ]->param->constant_param.value;
}

const char *wma_get_parameter( agent *my_agent, const char *name, const char * /*test*/ )
{
	const long param = wma_convert_parameter( my_agent, name );
	if ( param == WMA_PARAMS )
		return NULL;
	
	if ( wma_get_parameter_type( my_agent, param ) == wma_param_string )
		return my_agent->wma_params[ param ]->param->string_param.value->c_str();
	if ( wma_get_parameter_type( my_agent, param ) != wma_param_constant )
		return NULL;
	
	return my_agent->wma_params[ param ]->param->constant_param.to_str( my_agent->wma_params[ param ]->param->constant_param.value );
}

double wma_get_parameter( agent *my_agent, const char *name )
{
	const long param = wma_convert_parameter( my_agent, name );
	if ( param == WMA_PARAMS )
		return NULL;
	
	if ( wma_get_parameter_type( my_agent, param ) != wma_param_number )
		return NULL;
	
	return my_agent->wma_params[ param ]->param->number_param.value;
}

//

const long wma_get_parameter( agent *my_agent, const long param, const double /*test*/ )
{
	if ( !wma_valid_parameter( my_agent, param ) )
		return NULL;

	if ( wma_get_parameter_type( my_agent, param ) != wma_param_constant )
		return NULL;
	
	return my_agent->wma_params[ param ]->param->constant_param.value;
}

const char *wma_get_parameter( agent *my_agent, const long param, const char * /*test*/ )
{
	if ( !wma_valid_parameter( my_agent, param ) )
		return NULL;
	
	if ( wma_get_parameter_type( my_agent, param ) == wma_param_string )
		return my_agent->wma_params[ param ]->param->string_param.value->c_str();
	if ( wma_get_parameter_type( my_agent, param ) != wma_param_constant )
		return NULL;
	
	return my_agent->wma_params[ param ]->param->constant_param.to_str( my_agent->wma_params[ param ]->param->constant_param.value );
}

double wma_get_parameter( agent *my_agent, const long param )
{
	if ( !wma_valid_parameter( my_agent, param ) )
		return NULL;
	
	if ( wma_get_parameter_type( my_agent, param ) != wma_param_number )
		return NULL;
	
	return my_agent->wma_params[ param ]->param->number_param.value;
}

/***************************************************************************
 * Function     : wma_valid_parameter_value
 **************************************************************************/
bool wma_valid_parameter_value( agent *my_agent, const char *name, double new_val )
{
	const long param = wma_convert_parameter( my_agent, name );
	if ( param == WMA_PARAMS )
		return false;
	
	if ( wma_get_parameter_type( my_agent, param ) != wma_param_number )
		return false;
	
	return my_agent->wma_params[ param ]->param->number_param.val_func( new_val );
}

bool wma_valid_parameter_value( agent *my_agent, const char *name, const char *new_val )
{
	const long param = wma_convert_parameter( my_agent, name );
	if ( param == WMA_PARAMS )
		return false;
	
	if ( wma_get_parameter_type( my_agent, param ) == wma_param_string )
		return my_agent->wma_params[ param ]->param->string_param.val_func( new_val );
	if ( wma_get_parameter_type( my_agent, param ) != wma_param_constant )
		return false;
	
	return my_agent->wma_params[ param ]->param->constant_param.val_func( my_agent->wma_params[ param ]->param->constant_param.from_str( new_val ) );
}

bool wma_valid_parameter_value( agent *my_agent, const char *name, const long new_val )
{
	const long param = wma_convert_parameter( my_agent, name );
	if ( param == WMA_PARAMS )
		return false;
	
	if ( wma_get_parameter_type( my_agent, param ) != wma_param_constant )
		return false;
	
	return my_agent->wma_params[ param ]->param->constant_param.val_func( new_val );
}

//

bool wma_valid_parameter_value( agent *my_agent, const long param, double new_val )
{
	if ( !wma_valid_parameter( my_agent, param ) )
		return false;
	
	if ( wma_get_parameter_type( my_agent, param ) != wma_param_number )
		return false;
	
	return my_agent->wma_params[ param ]->param->number_param.val_func( new_val );
}

bool wma_valid_parameter_value( agent *my_agent, const long param, const char *new_val )
{
	if ( !wma_valid_parameter( my_agent, param ) )
		return false;
	
	if ( wma_get_parameter_type( my_agent, param ) == wma_param_string )
		return my_agent->wma_params[ param ]->param->string_param.val_func( new_val );
	if ( wma_get_parameter_type( my_agent, param ) != wma_param_constant )
		return false;
	
	return my_agent->wma_params[ param ]->param->constant_param.val_func( my_agent->wma_params[ param ]->param->constant_param.from_str( new_val ) );
}

bool wma_valid_parameter_value( agent *my_agent, const long param, const long new_val )
{
	if ( !wma_valid_parameter( my_agent, param ) )
		return false;
	
	if ( wma_get_parameter_type( my_agent, param ) != wma_param_constant )
		return false;
	
	return my_agent->wma_params[ param ]->param->constant_param.val_func( new_val );
}

/***************************************************************************
 * Function     : wma_set_parameter
 **************************************************************************/
bool wma_parameter_protected( agent *my_agent, const long param )
{
	return ( ( my_agent->wma_initialized ) && ( param > WMA_PARAM_ACTIVATION ) && ( param < WMA_PARAMS ) );
}

bool wma_set_parameter( agent *my_agent, const char *name, double new_val )
{
	const long param = wma_convert_parameter( my_agent, name );
	if ( param == WMA_PARAMS )
		return false;

	if ( wma_parameter_protected( my_agent, param ) )
		return false;
	
	if ( !wma_valid_parameter_value( my_agent, param, new_val ) )
		return false;

	// special case of decay rate needing negative numbers
	if ( param == WMA_PARAM_DECAY_RATE )
		my_agent->wma_params[ param ]->param->number_param.value = -new_val;
	else
		my_agent->wma_params[ param ]->param->number_param.value = new_val;

	return true;
}

bool wma_set_parameter( agent *my_agent, const char *name, const char *new_val )
{	
	const long param = wma_convert_parameter( my_agent, name );
	if ( param == WMA_PARAMS )
		return false;

	if ( wma_parameter_protected( my_agent, param ) )
		return false;
	
	if ( !wma_valid_parameter_value( my_agent, param, new_val ) )
		return false;

	if ( wma_get_parameter_type( my_agent, param ) == wma_param_string )
	{
		(*my_agent->wma_params[ param ]->param->string_param.value) = new_val;
		return true;
	}
	
	const long converted_val = my_agent->wma_params[ param ]->param->constant_param.from_str( new_val );

	// learning special case
	if ( param == WMA_PARAM_ACTIVATION )
	{
		set_sysparam( my_agent, WMA_ENABLED, converted_val );

		if ( converted_val == WMA_ACTIVATION_ON )
			wma_init( my_agent );
		else
			wma_deinit( my_agent );
	}
	
	my_agent->wma_params[ param ]->param->constant_param.value = converted_val;

	return true;
}

bool wma_set_parameter( agent *my_agent, const char *name, const long new_val )
{
	const long param = wma_convert_parameter( my_agent, name );
	if ( param == WMA_PARAMS )
		return false;

	if ( wma_parameter_protected( my_agent, param ) )
		return false;
	
	if ( !wma_valid_parameter_value( my_agent, param, new_val ) )
		return false;

	// learning special case
	if ( param == WMA_PARAM_ACTIVATION )
	{
		set_sysparam( my_agent, WMA_ENABLED, new_val );

		if ( new_val == WMA_ACTIVATION_ON )
			wma_init( my_agent );
		else
			wma_deinit( my_agent );
	}
	
	my_agent->wma_params[ param ]->param->constant_param.value = new_val;

	return true;
}

//

bool wma_set_parameter( agent *my_agent, const long param, double new_val )
{	
	if ( !wma_valid_parameter_value( my_agent, param, new_val ) )
		return false;
	
	// special case of decay rate needing negative numbers
	if ( param == WMA_PARAM_DECAY_RATE )
		my_agent->wma_params[ param ]->param->number_param.value = -new_val;
	else
		my_agent->wma_params[ param ]->param->number_param.value = new_val;

	return true;
}

bool wma_set_parameter( agent *my_agent, const long param, const char *new_val )
{	
	if ( wma_parameter_protected( my_agent, param ) )
		return false;
	
	if ( !wma_valid_parameter_value( my_agent, param, new_val ) )
		return false;

	if ( wma_get_parameter_type( my_agent, param ) == wma_param_string )
	{
		(*my_agent->wma_params[ param ]->param->string_param.value) = new_val;
		return true;
	}
	
	const long converted_val = my_agent->wma_params[ param ]->param->constant_param.from_str( new_val );

	// learning special case
	if ( param == WMA_PARAM_ACTIVATION )
	{
		set_sysparam( my_agent, WMA_ENABLED, converted_val );

		if ( converted_val == WMA_ACTIVATION_ON )
			wma_init( my_agent );
		else
			wma_deinit( my_agent );
	}
	
	my_agent->wma_params[ param ]->param->constant_param.value = converted_val;

	return true;
}

bool wma_set_parameter( agent *my_agent, const long param, const long new_val )
{	
	if ( wma_parameter_protected( my_agent, param ) )
		return false;
	
	if ( !wma_valid_parameter_value( my_agent, param, new_val ) )
		return false;

	// learning special case
	if ( param == WMA_PARAM_ACTIVATION )
	{
		set_sysparam( my_agent, WMA_ENABLED, new_val );

		if ( new_val == WMA_ACTIVATION_ON )
			wma_init( my_agent );
		else
			wma_deinit( my_agent );
	}
	
	my_agent->wma_params[ param ]->param->constant_param.value = new_val;

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// activation
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : wma_validate_activation
 **************************************************************************/
bool wma_validate_activation( const long new_val )
{
	return ( ( new_val == WMA_ACTIVATION_ON ) || ( new_val == WMA_ACTIVATION_OFF ) );
}

/***************************************************************************
 * Function     : wma_convert_activation
 **************************************************************************/
const char *wma_convert_activation( const long val )
{
	const char *return_val = NULL;
	
	switch ( val )
	{
		case WMA_ACTIVATION_ON:
			return_val = "on";
			break;
			
		case WMA_ACTIVATION_OFF:
			return_val = "off";
			break;
	}
	
	return return_val;
}

const long wma_convert_activation( const char *val )
{
	long return_val = NULL;
	
	if ( !strcmp( val, "on" ) )
		return_val = WMA_ACTIVATION_ON;
	else if ( !strcmp( val, "off" ) )
		return_val = WMA_ACTIVATION_OFF;
	
	return return_val;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// decay
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : wma_validate_decay
 **************************************************************************/
bool wma_validate_decay( const double new_val )
{
	return ( ( new_val >= 0 ) && ( new_val <= 1 ) );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// criteria
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : wma_validate_criteria
 **************************************************************************/
bool wma_validate_criteria( const long new_val )
{
	return ( ( new_val >= WMA_CRITERIA_O_AGENT ) && ( new_val <= WMA_CRITERIA_ALL ) );
}

/***************************************************************************
 * Function     : wma_convert_criteria
 **************************************************************************/
const char *wma_convert_criteria( const long val )
{
	const char *return_val = NULL;
	
	switch ( val )
	{
		case WMA_CRITERIA_O_AGENT:
			return_val = "o-agent";
			break;
			
		case WMA_CRITERIA_O_AGENT_ARCH:
			return_val = "o-agent-arch";
			break;

		case WMA_CRITERIA_ALL:
			return_val = "all";
			break;
	}
	
	return return_val;
}

const long wma_convert_criteria( const char *val )
{
	long return_val = NULL;
	
	if ( !strcmp( val, "o-agent" ) )
		return_val = WMA_CRITERIA_O_AGENT;
	else if ( !strcmp( val, "0-agent-arch" ) )
		return_val = WMA_CRITERIA_O_AGENT_ARCH;
	else if ( !strcmp( val, "all" ) )
		return_val = WMA_CRITERIA_ALL;
	
	return return_val;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// forgetting
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : wma_validate_forgetting
 **************************************************************************/
bool wma_validate_forgetting( const long new_val )
{
	return ( ( new_val == WMA_FORGETTING_ON ) || ( new_val == WMA_FORGETTING_OFF ) );
}

/***************************************************************************
 * Function     : wma_convert_forgetting
 **************************************************************************/
const char *wma_convert_forgetting( const long val )
{
	const char *return_val = NULL;
	
	switch ( val )
	{
		case WMA_FORGETTING_ON:
			return_val = "on";
			break;
			
		case WMA_FORGETTING_OFF:
			return_val = "off";
			break;
	}
	
	return return_val;
}

const long wma_convert_forgetting( const char *val )
{
	long return_val = NULL;
	
	if ( !strcmp( val, "on" ) )
		return_val = WMA_FORGETTING_ON;
	else if ( !strcmp( val, "off" ) )
		return_val = WMA_FORGETTING_OFF;
	
	return return_val;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// i_support
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : wma_validate_i_support
 **************************************************************************/
bool wma_validate_i_support( const long new_val )
{
	return ( ( new_val >= WMA_I_NONE ) && ( new_val <= WMA_I_UNIFORM ) );
}

/***************************************************************************
 * Function     : wma_convert_i_support
 **************************************************************************/
const char *wma_convert_i_support( const long val )
{
	const char *return_val = NULL;
	
	switch ( val )
	{
		case WMA_I_NONE:
			return_val = "none";
			break;
			
		case WMA_I_NO_CREATE:
			return_val = "no-create";
			break;

		case WMA_I_UNIFORM:
			return_val = "uniform";
			break;
	}
	
	return return_val;
}

const long wma_convert_i_support( const char *val )
{
	long return_val = NULL;
	
	if ( !strcmp( val, "none" ) )
		return_val = WMA_I_NONE;
	else if ( !strcmp( val, "no-create" ) )
		return_val = WMA_I_NO_CREATE;
	else if ( !strcmp( val, "uniform" ) )
		return_val = WMA_I_UNIFORM;
	
	return return_val;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// persistence
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : wma_validate_persistence
 **************************************************************************/
bool wma_validate_persistence( const long new_val )
{
	return ( ( new_val == WMA_PERSISTENCE_ON ) || ( new_val == WMA_PERSISTENCE_OFF ) );
}

/***************************************************************************
 * Function     : wma_convert_persistence
 **************************************************************************/
const char *wma_convert_persistence( const long val )
{
	const char *return_val = NULL;
	
	switch ( val )
	{
		case WMA_PERSISTENCE_ON:
			return_val = "on";
			break;
			
		case WMA_PERSISTENCE_OFF:
			return_val = "off";
			break;
	}
	
	return return_val;
}

const long wma_convert_persistence( const char *val )
{
	long return_val = NULL;
	
	if ( !strcmp( val, "on" ) )
		return_val = WMA_PERSISTENCE_ON;
	else if ( !strcmp( val, "off" ) )
		return_val = WMA_PERSISTENCE_OFF;
	
	return return_val;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// precision
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : wma_validate_precision
 **************************************************************************/
bool wma_validate_precision( const long new_val )
{
	return ( ( new_val == WMA_PRECISION_LOW ) || ( new_val == WMA_PRECISION_HIGH ) );
}

/***************************************************************************
 * Function     : wma_convert_precision
 **************************************************************************/
const char *wma_convert_precision( const long val )
{
	const char *return_val = NULL;
	
	switch ( val )
	{
		case WMA_PRECISION_LOW:
			return_val = "low";
			break;
			
		case WMA_PRECISION_HIGH:
			return_val = "high";
			break;
	}
	
	return return_val;
}

const long wma_convert_precision( const char *val )
{
	long return_val = NULL;
	
	if ( !strcmp( val, "low" ) )
		return_val = WMA_PRECISION_LOW;
	else if ( !strcmp( val, "high" ) )
		return_val = WMA_PRECISION_HIGH;
	
	return return_val;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : wma_enabled
 **************************************************************************/
bool wma_enabled( agent *my_agent )
{
	return ( my_agent->sysparams[ WMA_ENABLED ] == WMA_ACTIVATION_ON );
}

/***************************************************************************
 * Function     : wma_clean_stats
 **************************************************************************/
void wma_clean_stats( agent *my_agent )
{
	for ( int i=0; i<WMA_STATS; i++ )
	  delete my_agent->wma_stats[ i ];
}

/***************************************************************************
 * Function     : wma_reset_stats
 **************************************************************************/
void wma_reset_stats( agent *my_agent )
{
	for ( int i=0; i<WMA_STATS; i++ )
		my_agent->wma_stats[ i ]->value = 0;
}

/***************************************************************************
 * Function     : wma_add_stat
 **************************************************************************/
wma_stat *wma_add_stat( const char *name )
{
	// new stat entry
	wma_stat *newbie = new wma_stat;
	newbie->name = name;
	newbie->value = 0;
	
	return newbie;
}

/***************************************************************************
 * Function     : wma_convert_stat
 **************************************************************************/
const long wma_convert_stat( agent *my_agent, const char *name )
{
	for ( int i=0; i<WMA_STATS; i++ )
		if ( !strcmp( name, my_agent->wma_stats[ i ]->name ) )
			return i;

	return WMA_STATS;
}

const char *wma_convert_stat( agent *my_agent, const long stat )
{
	if ( ( stat < 0 ) || ( stat >= WMA_STATS ) )
		return NULL;

	return my_agent->wma_stats[ stat ]->name;
}

/***************************************************************************
 * Function     : wma_valid_stat
 **************************************************************************/
bool wma_valid_stat( agent *my_agent, const char *name )
{
	return ( wma_convert_stat( my_agent, name ) != WMA_STATS );
}

bool wma_valid_stat( agent *my_agent, const long stat )
{
	return ( wma_convert_stat( my_agent, stat ) != NULL );
}

/***************************************************************************
 * Function     : wma_get_stat
 **************************************************************************/
double wma_get_stat( agent *my_agent, const char *name )
{
	const long stat = wma_convert_stat( my_agent, name );
	if ( stat == WMA_STATS )
		return 0;	

	return my_agent->wma_stats[ stat ]->value;
}

double wma_get_stat( agent *my_agent, const long stat )
{
	if ( !wma_valid_stat( my_agent, stat ) )
		return 0;

	return my_agent->wma_stats[ stat ]->value;
}

/***************************************************************************
 * Function     : wma_set_stat
 **************************************************************************/
bool wma_set_stat( agent *my_agent, const char *name, double new_val )
{
	const long stat = wma_convert_stat( my_agent, name );
	if ( stat == WMA_STATS )
		return false;
	
	my_agent->wma_stats[ stat ]->value = new_val;
	
	return true;
}

bool wma_set_stat( agent *my_agent, const long stat, double new_val )
{
	if ( !wma_valid_stat( my_agent, stat ) )
		return false;	
		
	my_agent->wma_stats[ stat ]->value = new_val;
	
	return true;
}

//

/***************************************************************************
 * Function     : wma_init
 * Author		: Andy Nuxoll?
 * Notes		: wma_init will set up the memory pool which holds the 
 *                decay_elements (which are the elements of the linked 
 *                lists at each decay_timelist position).  It also sets up 
 *                the timelist for all positions, and sets the current 
 *                pointer to the first of those.
 *
 *                Subsequent calls will free the memory pool.
 **************************************************************************/
void wma_init( agent *my_agent )
{
	if ( my_agent->wma_initialized )
		return;
	
	wma_timelist_element *temp_timelist = NULL;
	unsigned long current_time = my_agent->d_cycle_count;
	int i;

	// initialize memory pool
	if ( my_agent->wma_first )	
	{
		my_agent->wma_first = false;
		init_memory_pool( my_agent, &( my_agent->wma_decay_element_pool ), sizeof( wma_decay_element_t ), "wma_decay" );
	}

	// set up the timelist
	for( i=0; i<=WMA_MAX_TIMELIST; i++ )
	{
		temp_timelist = &( my_agent->wma_timelist[ i ] );
		temp_timelist->position = i;
		temp_timelist->time = current_time + i;
		temp_timelist->first_decay_element = NULL;
	}

	// init the current pointer
	my_agent->wma_timelist_current = my_agent->wma_timelist;

	// init the tc
	my_agent->wma_tc_counter = 2;

	// Pre-compute the integer powers of the decay exponent in order to avoid
	// repeated calls to pow() at runtime
	double decay_rate = wma_get_parameter( my_agent, WMA_PARAM_DECAY_RATE );
	for( i=0; i<WMA_POWER_SIZE; i++ )
		my_agent->wma_power_array[ i ] = pow( (double) i, decay_rate );

	// Pre-compute low precision decay values based upon number of references
	// and activation history
	my_agent->wma_quick_boost[0] = 0;
	{
		// number of times to simulate
		const unsigned long num_iterations = 1000;		
		
		wma_decay_element_t el;
		double activation_level;
		double sum;		
		long i;
		long n;

		unsigned long history_iter;
		unsigned long test_iter;
		unsigned long time_iter;
		unsigned long avg;

		// Loop over all possible history counts
		for( el.history_count=1; el.history_count<=WMA_DECAY_HISTORY; el.history_count++ )
		{
			avg = 0;

			// Perform multiple tests and find the average boost
			for( test_iter=0; test_iter<num_iterations; test_iter++ )
			{
				// Create a random history of the required size
				for( i=1; i<el.history_count; i++ )
					el.boost_history[i] = (unsigned long) floor( SoarRand( WMA_DECAY_HISTORY - 1 ) + 1 );

				// At least one of the references will be from the current cycle
				el.boost_history[0] = WMA_DECAY_HISTORY;

				for( i = el.history_count; i<WMA_DECAY_HISTORY; i++ )
					el.boost_history[i] = WMA_MAX_TIMELIST*2;

				//Count the number of references from this cycle
				el.num_references = 0;
				for( i=0; i<el.history_count; i++ )
					if ( el.boost_history[i] == WMA_DECAY_HISTORY )
						el.num_references++;

				// The boost routine expects the history to be in numerical order
				qsort( (void *) el.boost_history, (size_t) WMA_DECAY_HISTORY, sizeof( unsigned long ), compare_num );

				// Calculate the amount of boost received
				time_iter = WMA_DECAY_HISTORY;

				do
				{
					sum = 0;

					// Existing WME
					for ( history_iter = 0; history_iter<el.history_count; history_iter++ ) 
					{
						n = time_iter - el.boost_history[ history_iter ] + 1;
						if ( n < WMA_POWER_SIZE )
						{
							sum += my_agent->wma_power_array[ n ];
						}
					}

					activation_level = log( sum );

					time_iter++;

				} while( activation_level > WMA_ACTIVATION_CUTOFF );

				// Why -1?  because the number of references will be added to the
				// boost (num_refs*2 at creation time) in order to simulate the extra
				// boost you get for recent references.
				avg += ( time_iter - WMA_DECAY_HISTORY ) - 1;
			}

			avg /= num_iterations;
			my_agent->wma_quick_boost[ el.history_count ] = avg;
		}
	}

	// note initialization
	my_agent->wma_initialized = true;
}

/***************************************************************************
 * Function     : wma_deinit
 * Author		: Andy Nuxoll?
 * Notes		: wma_deinit will set the decay_elements for all of the 
 *                wmes in the decay timelist to NIL, otherwise there will 
 *                be dangling pointers if decay is turned back on.
 **************************************************************************/
void wma_deinit( agent *my_agent )
{
	if ( !my_agent->wma_initialized )
		return;
	
	{
		long first_spot, last_spot, i;
		wma_decay_element_t *remove_this;

		first_spot = my_agent->wma_timelist_current->position;
		last_spot = ( first_spot - 1 + ( WMA_MAX_TIMELIST + 1 ) ) % ( WMA_MAX_TIMELIST + 1 );

		for ( long i=first_spot; i!=last_spot; i=( ( i + 1 ) % ( WMA_MAX_TIMELIST + 1 ) ) )
		{
			remove_this = my_agent->wma_timelist[ i ].first_decay_element;
			while ( remove_this != NIL )
			{
				remove_this->this_wme->wma_decay_element = NIL;
				remove_this->this_wme->wma_has_decay_element = false;
				remove_this = remove_this->next;
			}
		}
	}

	my_agent->wma_initialized = false;
}

/***************************************************************************
 * Function     : wma_decay_helper
 * Author		: Andy Nuxoll?
 * Notes		: This function recursively discovers the activated WMEs 
 *                that led to the creation of a given WME.  When found, 
 *                their boost histories are added to the history in the 
 *                given decay element.
 *
 *                This function returns the number of supporting WMEs 
 *                found.
 **************************************************************************/
unsigned long wma_decay_helper( wme *w, wma_decay_element_t *el, unsigned long tc_value )
{
	unsigned long num_cond_wmes = 0;
	preference *pref = w->preference;
	instantiation *inst;
	condition *cond;
	wme *cond_wme;
	long i, j;

	if ( pref == NIL ) 
		return 0;

	inst = pref->inst;
	cond = inst->top_of_instantiated_conditions;
	while( cond != NIL )
	{
		if ( ( cond->type == POSITIVE_CONDITION ) && ( cond->bt.wme_->wma_tc_value != tc_value ) )
		{
			cond_wme = cond->bt.wme_;
			cond_wme->wma_tc_value = tc_value;

			if ( cond_wme->wma_has_decay_element )
			{
				if ( !cond_wme->wma_decay_element->just_created )
				{
					i = WMA_DECAY_HISTORY - 1;
					for( j=( cond_wme->wma_decay_element->history_count - 1); j>=0; j-- )
					{
						el->boost_history[ i ] += cond_wme->wma_decay_element->boost_history[ j ];
						i--;
					}

					num_cond_wmes++;
				}
			}
			else
			{
				num_cond_wmes += wma_decay_helper( cond_wme, el, tc_value );
			}
		}

		// Repeat for next condition
		cond = cond->next;
	}

	return num_cond_wmes;
}

/***************************************************************************
 * Function     : wma_calculate_average_history
 * Author		: Andy Nuxoll?
 * Notes		: This function examines the production instantiation that 
 *                led to the creation of a WME.  The decay history of each 
 *                WME in that instantiation is examined and averaged.  This 
 *                new average boost history is inserted into the given decay 
 *                element.
 *
 *                If the WME is not o-supported or no conditions are found 
 *                that meet the criteria (positive conditions on wmes that 
 *                have a decay history) then an empty history is assigned.
 **************************************************************************/
void wma_calculate_average_history( agent* my_agent, wme *w, wma_decay_element_t *el )
{
	preference *pref = w->preference;
	int i;
	int num_cond_wmes = 0;

	el->history_count = 0;
	if ( pref == NIL )
		return;

	for( i=0; i<WMA_DECAY_HISTORY; i++ )
		el->boost_history[ i ] = 0;

	num_cond_wmes = wma_decay_helper( w, el, ++my_agent->wma_tc_counter );

	if ( num_cond_wmes > 0 )
	{		
		// Calculate the average
		for( i = 0; i<WMA_DECAY_HISTORY; i++ )
			el->boost_history[ i ] /= num_cond_wmes;

		// Determine the actual length of the history
		for( i=( WMA_DECAY_HISTORY - 1 ); i>=0; i-- )
			if ( el->boost_history[ i ] > 0 ) 
				el->history_count++;

		// Compress the array values into the left hand side of the array
		for( i=0; i<( el->history_count ); i++ )			
			el->boost_history[i] = el->boost_history[ i + ( WMA_DECAY_HISTORY - el->history_count ) ];
	}
}

/***************************************************************************
 * Function     : wma_decay_reference_wme
 * Author		: Andy Nuxoll?
 * Notes		: Given a WME, this function increments its reference count.  
 *                If the WME is not activated (because it has i-support) 
 *                then this function traces the given WME's preference tree 
 *                to find the set of all activated WMEs that must exist in 
 *                order for the given WME to exist.  Each of these activated 
 *                WMEs is given a reference.
 **************************************************************************/
void wma_decay_reference_wme( agent *my_agent, wme *w, int depth = 0 )
{
	preference *pref = w->preference;
	instantiation *inst;
	condition *c;

	// Avoid stack overflow
	if ( depth > 10 ) 
		return;

	// Step 1:  Check for cases where referencing the WME is easy.  This should
	//          happen the majority of the time.

	// If the WME has a decay element we can just bump the reference
	// count and return
	if ( w->wma_has_decay_element )
	{
		w->wma_decay_element->num_references++;
		return;
	}
	// Architectural WMEs without decay elements are ignored
	else if ( ( pref == NIL ) || ( pref->reference_count == 0 ) )
	{
		return;
	}
	else if ( pref->o_supported == TRUE )
	{
		/*
		It's possible that there is an o-supported wme out there which does
		not have a decay_element.  Possible causes for this are:
		1.  The wme was i-supported and is just now being o-supported
		2.  Decay was turned off, and turned back on, so the wme never got a
		chance to make the decay_element
		3.  Forgetting has been disabled so the WME's decay element was
		removed but the WME was not removed from working memory.

		Regardless of how this happened, we need to create a new decay element
		for this WME here.
		*/

		// Add a decay element to this WME
		// MRJ: The 1 in the next call is not always right. It's a
		//      rare case, but maybe look at it...
		wma_update_new_wme( my_agent, w, 1 );
		return;
	}
	// If i-support mode is 'none' then we can stop here
	else if ( wma_get_parameter( my_agent, WMA_PARAM_I_SUPPORT, WMA_RETURN_LONG ) == WMA_I_NONE )
	{
		return;
	}

	
	// Step 2:  In this case we have an i-supported WME that has been
	//          referenced.  We need to find the supporting o-supported WMEs and
	//          reference them instead.

	inst = pref->inst;
	c = inst->top_of_instantiated_conditions;
	while( c != NIL )
	{
		// BUGBUG: How to handle negative conditions?  Ignore for now.
		if ( c->type == POSITIVE_CONDITION )
		{			
			wma_decay_reference_wme( my_agent, c->bt.wme_, depth + 1 ); 
		}
		c = c->next;
	}

}

/***************************************************************************
 * Function     : wma_update_new_wme
 * Author		: Andy Nuxoll?
 * Notes		: This function adds a decay element to an existing WME.  
 *                It is called whenever a wme is discovered that does not 
 *                have a decay element (usually this is at wme creation 
 *                time.)
 **************************************************************************/
void wma_update_new_wme( agent *my_agent, wme *w, int num_refs )
{
	wma_decay_element_t *temp_el;
	
	// should this be an activated wme?
	bool good_wme = true;

	// Step 1: Verify that this WME meets the criteria for being an activated wme
	const long criteria = wma_get_parameter( my_agent, WMA_PARAM_CRITERIA, WMA_RETURN_LONG );
	switch( criteria )
	{
		case WMA_CRITERIA_ALL:
			break;

		case WMA_CRITERIA_O_AGENT_ARCH:
			if ( ( w->preference != NIL ) && ( w->preference->o_supported != TRUE ) )
				good_wme = false;
			break;

		case WMA_CRITERIA_O_AGENT:
			if ( ( w->preference == NIL ) || ( w->preference->o_supported != TRUE ) )
				good_wme = false;
			break;
	}

	if ( !good_wme )
	{
		// However, the creation of an i-supported WME may activate the WMEs that
		// led to its creation.
		if ( ( w->preference != NIL ) && ( w->preference->o_supported != TRUE ) )
		{
			const long i_support = wma_get_parameter( my_agent, WMA_PARAM_I_SUPPORT, WMA_RETURN_LONG );
			
			switch( i_support )
			{
				case WMA_I_NONE:
				case WMA_I_NO_CREATE:
				break;
			
				case WMA_I_UNIFORM:
					wma_decay_reference_wme( my_agent, w );
				break;
			}
		}

		return;
	}

	// If the wme already has a decay element return.
	if ( w->wma_decay_element != NIL )
		return;

	// Step 2:  Allocate and initialize a new decay element for the WME
	allocate_with_pool( my_agent, &( my_agent->wma_decay_element_pool ), &temp_el );
	temp_el->just_created = true;
	temp_el->just_removed = false;
	temp_el->history_count = 0;
	temp_el->this_wme = w;
	temp_el->num_references = num_refs;

	// Give the WME an initial history based upon the WMEs that were tested to create it.
	wma_calculate_average_history( my_agent, w, temp_el );	

	// Insert at the current position in the decay timelist
	// It will be boosted out of this position at the next update
	temp_el->next = my_agent->wma_timelist_current->first_decay_element;
	temp_el->previous = NIL;
	temp_el->time_spot = my_agent->wma_timelist_current;
	if( temp_el->next != NIL )
		temp_el->next->previous = temp_el;
	my_agent->wma_timelist_current->first_decay_element = temp_el;

	//Attach it to the wme
	w->wma_decay_element = temp_el;
	w->wma_has_decay_element = true;
}

/***************************************************************************
 * Function     : wma_deactivate_element
 * Author		: Andy Nuxoll?
 * Notes		: This routine marks a decay element as being attached to a 
 *                wme struct that has been removed from working memory.  
 *                When the wme struct is actually deallocated then the 
 *                wma_remove_decay_element() routine is called.
 **************************************************************************/
void wma_deactivate_element( agent * /*my_agent*/, wme *w )
{
	// Make sure this wme has an element and that element has not already been
	// deactivated
	if ( !w->wma_has_decay_element || w->wma_decay_element->just_removed )
		return;	

	// Remove the decay element from the decay timelist
	if ( w->wma_decay_element->previous == NIL )
	{
		// if it is the first in a list, set that to the next
		w->wma_decay_element->time_spot->first_decay_element = w->wma_decay_element->next;
	}
	else
	{
		// otherwise remove the decay_element from the list
		w->wma_decay_element->previous->next = w->wma_decay_element->next;
	}

	if ( w->wma_decay_element->next != NIL )
	{
		// if the element has a next update prev -> next
		w->wma_decay_element->next->previous = w->wma_decay_element->previous;		
	}

	w->wma_decay_element->next = NIL;
	w->wma_decay_element->previous = NIL;
	w->wma_decay_element->just_removed = true;
}

/***************************************************************************
 * Function     : wma_remove_decay_element
 * Author		: Andy Nuxoll?
 * Notes		: This routine deallocates the decay element attached to a 
 *                given WME.
 **************************************************************************/
void wma_remove_decay_element( agent *my_agent, wme *w )
{
	if ( !w->wma_has_decay_element )
		return;

	// Deactivate the wme first
	if ( !w->wma_decay_element->just_removed )
		wma_deactivate_element( my_agent, w );

	free_with_pool( &( my_agent->wma_decay_element_pool ), w->wma_decay_element );

	w->wma_has_decay_element = false;
	w->wma_decay_element = NIL;
}

/***************************************************************************
 * Function     : wma_activate_wmes_in_pref
 * Author		: Andy Nuxoll?
 * Notes		: This routine boosts the activation of all WMEs in a given 
 *                preference
 **************************************************************************/
void wma_activate_wmes_in_pref( agent *my_agent, preference *pref )
{
	wme *w;

	// I have the recreated code here instead of a seperate function so that
	// all newly_created_insts are picked up.
	if ( ( pref->type != REJECT_PREFERENCE_TYPE ) && 
		 ( pref->type != PROHIBIT_PREFERENCE_TYPE ) &&
		 ( pref->slot != NIL ) )
	{
		w = pref->slot->wmes;
		while ( w )
		{
			// id and attr should already match so just compare the value
			if ( w->value == pref->value )			
				wma_decay_reference_wme( my_agent, w );
			
			w = w->next;
		}
	}
}

/***************************************************************************
 * Function     : wma_activate_wmes_in_inst
 * Author		: Andy Nuxoll?
 * Notes		: This routine boosts the activation of all WMEs in a given 
 *                production instantiation.
 **************************************************************************/
void wma_activate_wmes_in_inst( agent *my_agent, instantiation *inst )
{
	for ( condition *cond=inst->top_of_instantiated_conditions; cond!=NIL; cond=cond->next )
		if ( cond->type==POSITIVE_CONDITION )
			wma_decay_reference_wme( my_agent, cond->bt.wme_ ); 
}

/***************************************************************************
 * Function     : wma_update_wmes_in_retracted_inst
 * Author		: Andy Nuxoll?
 * Notes		: This code is detecting production retractions that affect 
 *                activated WMEs and decrementing their reference counts.
 **************************************************************************/
void wma_update_wmes_in_retracted_inst( agent *my_agent, instantiation *inst )
{
	wme *w;
	preference *pref, *next;

	if ( wma_get_parameter( my_agent, WMA_PARAM_PERSISTENCE, WMA_RETURN_LONG ) == WMA_PERSISTENCE_ON )
	{
		for ( pref=inst->preferences_generated; pref!=NIL; pref=next )
		{
			next = pref->inst_next;

			if ( ( pref->type != REJECT_PREFERENCE_TYPE ) && 
				 ( pref->type != PROHIBIT_PREFERENCE_TYPE ) &&
				 ( pref->o_supported ) &&
				 ( pref->slot != NIL ) )
			{
				// found an o-supported pref
				w = pref->slot->wmes;

				while ( w )
				{
					// id and attr should already match...
					if( w->value == pref->value )
					{
						// we got a match with an existing wme
						if ( w->wma_decay_element != NIL )
						{							
							w->wma_decay_element->num_references--;
						}
					}

					w = w->next;
				} 
			}
		}
	}
}

/***************************************************************************
 * Function     : wma_update_wmes_in_prods
 * Author		: Andy Nuxoll?
 * Notes		: This function scans all the match set changes and updates 
 *                the reference count for affected WMEs.
 **************************************************************************/
void wma_update_wmes_tested_in_prods( agent *my_agent )
{
	ms_change *msc;
	token temp_token, *t;
	instantiation *inst;
	condition *cond;

	for ( msc=my_agent->ms_o_assertions; msc!=NIL; msc=msc->next )
	{
		temp_token.parent = msc->tok;
		temp_token.w = msc->w;
		t = &temp_token;

		while ( t != my_agent->dummy_top_token )
		{
			if (t->w != NIL)
				wma_decay_reference_wme( my_agent, t->w );

			t = t->parent;
		}
	}

	for ( msc=my_agent->ms_i_assertions; msc!=NIL; msc=msc->next )
	{
		temp_token.parent = msc->tok;
		temp_token.w = msc->w;
		t = &temp_token;

		while ( t != my_agent->dummy_top_token )
		{
			if ( t->w != NIL )		
				wma_decay_reference_wme( my_agent, t->w );

			t = t->parent;
		}
	}

	// If instantiations do not persistently activate WMEs then there is no need
	// to decrement the reference count for retractions. :AMN: 12 Aug 2003
	if ( wma_get_parameter( my_agent, WMA_PARAM_PERSISTENCE, WMA_RETURN_LONG ) == WMA_PERSISTENCE_ON )
	{
		for ( msc=my_agent->ms_retractions; msc!=NIL; msc=msc->next )
		{
			inst = msc->inst;
			temp_token.w = msc->w;
			t = &temp_token;

			for ( cond=inst->top_of_instantiated_conditions; cond!=NIL; cond=cond->next )
			{
				// If a wme's existence caused an instance to cease to match (due to
				// a negative condition in that instance) then we don't want to
				// decrement the reference count on the WME because the WME's
				// reference count was never incremented.
				if ( cond->type==POSITIVE_CONDITION )
				{
					if ( cond->bt.wme_->wma_decay_element != NIL )
						cond->bt.wme_->wma_decay_element->num_references--;
				}
			}
		}
	}
}

/***************************************************************************
 * Function     : wma_get_wme_activation_level
 * Author		: Andy Nuxoll?
 * Notes		: This function is provided for external use.  Given a WME, 
 *                this it calculates an approximate activation level of that 
 *                WME as an integer between 0 and WMA_MAX_TIMELIST.  The 
 *                higher the number the more activated the WME is.
 *                Calculating a real valued activation level is expensive
 *                and usually unnecessary.  This function provides a nice
 *                compromise.
 *
 *                If the given WME does not have a decay element, this 
 *                function returns WMA_ACTIVATION_NONE_INT.
 **************************************************************************/
long wma_get_wme_activation_level( agent *my_agent, wme *w )
{
	if ( !w->wma_has_decay_element )
		return (long) WMA_ACTIVATION_NONE_INT;
	
	{
		long wme_pos = w->wma_decay_element->time_spot->position;
		long curr_pos = my_agent->wma_timelist_current->position;

		return ( ( wme_pos >= curr_pos )?( wme_pos - curr_pos ):( ( WMA_MAX_TIMELIST + 1 ) - curr_pos + wme_pos ) );
	}
}

/***************************************************************************
 * Function     : wma_get_wme_activation
 * Author		: Andy Nuxoll?
 * Notes		: This function is provided for external use.  Given a WME, 
 *                this it calculates an EXACT activation level of that WME 
 *                as a floating point number.  The higher the number the 
 *                more activated the WME is.
 *
 *                If the given WME does not have a decay element, this 
 *                function returns WMA_ACTIVATION_NONE_DOUBLE
 **************************************************************************/
double wma_get_wme_activation( agent *my_agent, wme *w )
{
	if ( !w->wma_has_decay_element )
		return (double) WMA_ACTIVATION_NONE_DOUBLE;

	double sum = 0.0;
	{
		// Calculate and print the activation level
		long n;
		for ( long i=0; i<w->wma_decay_element->history_count; i++ )
		{
			n = w->wma_decay_element->time_spot->time - w->wma_decay_element->boost_history[ i ] + 1;
			
			if ( n < WMA_POWER_SIZE )
				sum += my_agent->wma_power_array[ n ];
		}
	}

	return sum;
}

/***************************************************************************
 * Function     : wma_add_refs_to_history
 * Author		: Andy Nuxoll?
 * Notes		: This function adds N refs at the previous cycle to the 
 *                boost history of a given decay element.
 *
 *                CAVEAT: Why previous cycle instead of current?  It's a 
 *                kludge. This function is only called by wma_boost_wme() 
 *                which is called at the end of the cycle but after the 
 *                cycle count has been incremented.  So I compensate here.
 **************************************************************************/
void wma_add_refs_to_history( agent *my_agent, wma_decay_element_t *el, long num_refs )
{
	long i;
	long move_by;

	if ( num_refs > WMA_DECAY_HISTORY )
		num_refs = WMA_DECAY_HISTORY;

	if ( ( el->history_count + num_refs ) > WMA_DECAY_HISTORY )
	{
		// Shift some references out of the array to make room for the new ones.
		move_by = ( el->history_count + num_refs - WMA_DECAY_HISTORY );

		for ( i=0; i<( WMA_DECAY_HISTORY - num_refs ); i++ )
			el->boost_history[ i ] = el->boost_history[ i + move_by ];

		for( i=( WMA_DECAY_HISTORY - num_refs ); i<WMA_DECAY_HISTORY; i++)
			el->boost_history[ i ] = ( my_agent->d_cycle_count - 1 );

		el->history_count = WMA_DECAY_HISTORY;
	}
	else
	{
		for( i=el->history_count; i<( el->history_count + num_refs ); i++ )
			el->boost_history[ i ] = ( my_agent->d_cycle_count - 1 );

		el->history_count += num_refs;
	}
}

/***************************************************************************
 * Function     : wma_boost_wme
 * Author		: Andy Nuxoll?
 * Notes		: This function calculates the new position in the decay 
 *                timelist for a decay element when its associated WME is 
 *                referenced.
 **************************************************************************/
long wma_boost_wme( agent *my_agent, wma_decay_element_t *cur_decay_el )
{
	long decay_spot;
	long time_iter;
	double sum, activation_level;
	int history_iter;
	
	// Step 1: Update the boost history

	// For new WMEs, we need to make sure the ref count is nonzero
	if ( ( cur_decay_el->just_created ) && ( cur_decay_el->num_references < 1 ) )
		cur_decay_el->num_references = 1;

	wma_add_refs_to_history( my_agent, cur_decay_el, cur_decay_el->num_references );

	
	// Step 2:  Calculate the new position in the decay timelist
	
	// start at the current time_spot for this wme, because the
	// new time spot can't be lower than the current.
	time_iter = cur_decay_el->time_spot->time; 

	if ( wma_get_parameter( my_agent, WMA_PARAM_PRECISION, WMA_RETURN_LONG ) == WMA_PRECISION_LOW )
	{
		time_iter += my_agent->wma_quick_boost[ cur_decay_el->history_count ];
		time_iter += cur_decay_el->num_references;
	}
	else
	{
		long n;
		
		do
		{
			sum = 0;

			for ( history_iter=0; history_iter<=( cur_decay_el->history_count - 1 ); history_iter++ )
			{
				n = ( time_iter - cur_decay_el->boost_history[ history_iter ] + 1 );
				if ( n < WMA_POWER_SIZE )
					sum += my_agent->wma_power_array[ n ];
			}

			activation_level = log( sum );
			time_iter++;

		} while( activation_level > WMA_ACTIVATION_CUTOFF );
	}

	// time_iter is the cycle when the wme will be below the
	// decay threshold so remove the wme at the end of the cycle
	// just before that
	decay_spot = time_iter - 1;

	// calculate what position it should go to
	if( decay_spot > ( my_agent->wma_timelist_current->position + WMA_MAX_TIMELIST ) )
	{
		decay_spot = ( my_agent->wma_timelist_current->position + WMA_MAX_TIMELIST ) % ( WMA_MAX_TIMELIST + 1 );
	}
	else
	{
		decay_spot = decay_spot % ( WMA_MAX_TIMELIST + 1 );
	}

	
	// Step 0 revisited:  More special handling for just created WMEs
	if ( cur_decay_el->just_created )
	{
		cur_decay_el->just_created = false;

		// AMN: 25 May 2003 WMEs that are architectural (e.g., input-link,
		// ^superstate nil) need to have their num_references value reset.
		// Otherwise the system will behave as if the WMEs have been referenced
		// at every cycle.
		if ( cur_decay_el->this_wme->preference == NIL )
			cur_decay_el->num_references = 0;
	}

	return decay_spot;
}

/***************************************************************************
 * Function     : wma_reposition_wme
 * Author		: Andy Nuxoll?
 * Notes		: This function repositions a decay element in the decay 
 *                timelist.
 **************************************************************************/
void wma_reposition_wme( agent *my_agent, wma_decay_element_t *cur_decay_el, long decay_spot )
{
	// remove the current decay element whose decay spot we've just
	// calculated from the decay timelist in preparation for moving
	// it to its new location (see next code block)
	if( cur_decay_el->previous == NIL )
	{
		// it is first in the decay list so set the first element to
		// the next one (or NIL)
		cur_decay_el->time_spot->first_decay_element = cur_decay_el->next;

		// if there is a next element (now the first), set it's previous to NIL
		if( cur_decay_el->next != NIL )
			cur_decay_el->next->previous = NIL;
	}
	else
	{
		// it is not first (so will have a previous for sure)
		cur_decay_el->previous->next = cur_decay_el->next;
		
		if( cur_decay_el->next != NIL )
			cur_decay_el->next->previous = cur_decay_el->previous;
	}

	// Insert the current decay element in its new location
	cur_decay_el->next = my_agent->wma_timelist[ decay_spot ].first_decay_element;
	cur_decay_el->previous = NIL;
	cur_decay_el->time_spot = &( my_agent->wma_timelist[ decay_spot ] );

	// set up next element's previous pointer
	if( cur_decay_el->next != NIL )
		cur_decay_el->next->previous = cur_decay_el;

	// set up the first_decay_element for this time slot
	// (just insert it first since order doesn't matter)
	my_agent->wma_timelist[ decay_spot ].first_decay_element = cur_decay_el;
}

/***************************************************************************
 * Function     : wma_forget_wme
 * Author		: Andy Nuxoll?
 * Notes		: This routine removes an activated WME from working memory 
 *                and performs all necessary cleanup related to that removal.
 **************************************************************************/
void wma_forget_wme( agent *my_agent, wme *w )
{
	wme *w2;
	slot *s;
	Symbol *id;
	preference *p;
	
	id = w->id;

	// what lists will w be on?  acceptable preferences??
	for ( s=id->id.slots; s!=NIL; s=s->next )
	{
		for ( w2=s->wmes; w2!=NIL; w2=w2->next )
			if ( w == w2 ) break;

		if ( w2 )
			remove_from_dll( s->wmes, w, next, prev );

		for ( w2=s->acceptable_preference_wmes; w2!=NIL; w2=w2->next )
		{
			if ( w == w2 )
			{
				remove_from_dll( s->acceptable_preference_wmes, w, next, prev ); 
				break;
			}
		}

		// %%%once a preference is made into a wme, is it removed from the
		// preference lists in that slot, or kept there?  If kept, it must
		// be removed here. -MRJ

		// %%%do we need to deallocate this pref, or is that taken care of
		// automatically? -MRJ

		for( p=s->all_preferences; p!=NIL; p=p->all_of_slot_next )
		{
			// if p matches this wme, remove p...
			if( ( p->id == w->id )
				&& ( p->attr == w->attr )
				&& ( p->value == w->value ) )
			{
				remove_preference_from_tm( my_agent, p );
			}
		}
	}

	/* REW: begin 09.15.96 */
#ifndef SOAR_8_ONLY
	if ( ( my_agent->operand2_mode ) )
	{
#endif
		if ( w->gds )
		{
			if ( w->gds->goal != NIL )
			{
				gds_invalid_so_remove_goal( my_agent, w ); 
			}
		}
#ifndef SOAR_8_ONLY
	}
#endif

	remove_wme_from_wm( my_agent, w );      
}

/***************************************************************************
 * Function     : wma_move_and_remove_wmes
 * Author		: Andy Nuxoll?
 * Notes		: This function is called at the end of each cycle to boost 
 *                WMEs that have been referenced this cycle and remove WMEs 
 *                that have been forgotten due to decay.
 *
 *                NOTE:  The bulk of the work done by the decay system is 
 *                done in this routine or one of its subroutines).
 **************************************************************************/
void wma_move_and_remove_wmes( agent *my_agent )
{
	wma_decay_element_t *cur_decay_el, *next;
	long array_iter, array_position, start_position;
	
	// New position for the wme in the decay timelist
	long decay_spot;

	
	// Step 1:  Reposition all WMEs have have been created or referenced this
	//          cycle

	// need to start at the last time in the decay timelist and work backwards...
	start_position = my_agent->wma_timelist_current->position;

	// loop over all positions in the decay timelist
	for ( array_iter=0; array_iter<=WMA_MAX_TIMELIST; array_iter++ )
	{
		// Find position in the array for this iteration (starting 1 back from
		// current) 
		array_position = ( start_position - array_iter + WMA_MAX_TIMELIST ) % ( WMA_MAX_TIMELIST + 1 );
		cur_decay_el = my_agent->wma_timelist[ array_position ].first_decay_element;		

		// loop over all decay elemnts at this position in the timelist
		while ( cur_decay_el != NIL )
		{
			next = cur_decay_el->next;

			if ( ( cur_decay_el->num_references > 0 ) || ( cur_decay_el->just_created ) )
			{
				decay_spot = wma_boost_wme( my_agent, cur_decay_el );
				wma_reposition_wme( my_agent, cur_decay_el, decay_spot );

				// If we are *not* using persistent activation then the reference
				// count should be set to zero here to prevent subsequent
				// activation. :AMN: 12 Aug 2003
				if ( wma_get_parameter( my_agent, WMA_PARAM_PERSISTENCE, WMA_RETURN_LONG ) != WMA_PERSISTENCE_ON )
					cur_decay_el->num_references = 0;
			}

			cur_decay_el = next;
		}
	}

	// Step 2: Removes all the WMEs that are still at the current spot in the
	//         decay timelist.  This is the actual forgetting mechanism
	//         associated with decay.
	bool forgetting = ( wma_get_parameter( my_agent, WMA_PARAM_FORGETTING, WMA_RETURN_LONG ) == WMA_FORGETTING_ON );
	cur_decay_el = my_agent->wma_timelist_current->first_decay_element;
	while( cur_decay_el != NIL )
	{
		// save this pointer before it is potentially deallocated
		next = cur_decay_el->next;

		if ( forgetting )
			wma_forget_wme( my_agent, cur_decay_el->this_wme );
		else
			wma_remove_decay_element( my_agent, cur_decay_el->this_wme );

		cur_decay_el = next;
	}

	// Update working memory with all the WME removals that were done in the
	// loop above.  This has to be done before changing first_decay_element at
	// current time, otherwise it will cause pointer problems.
	if ( forgetting )
		do_buffered_wm_and_ownership_changes( my_agent ); 

	// update position that just had removals
	my_agent->wma_timelist_current->time += ( WMA_MAX_TIMELIST + 1 );

	// Mark the current position in the array as empty.  Note that if
	// remove_wme_from_wm() doesn't work right, this next line could cause
	// memory leaks
	my_agent->wma_timelist_current->first_decay_element = NIL;

	// Update current position in the array
	my_agent->wma_timelist_current = &( my_agent->wma_timelist[ ( my_agent->wma_timelist_current->position + 1 ) % ( WMA_MAX_TIMELIST + 1 ) ] );
}

/***************************************************************************
 * Function     : wma_print_activated_wmes
 * Author		: Andy Nuxoll?
 * Notes		: Quick utility to print the n-most activated level of wmes
 **************************************************************************/
void wma_print_activated_wmes( agent *my_agent, long n )
{
	wma_timelist_element *decay_list;
	wma_decay_element_t *decay_element;
	long decay_pos, power_pos;   
	double sum = 0;
	int history_iter;
	char act_buf[512];

	print( my_agent, "\nBEGIN ACTIVATED WME LIST\n" );

	decay_list = my_agent->wma_timelist;
	decay_pos = my_agent->wma_timelist_current->position;

	// Traverse the decay array backwards in order to get the most
	// activated wmes first
	for ( long i=0; i<n; i++ )
	{
		char buf[32];
		sprintf( buf, "%.2d     ", i );

		decay_pos = ( ( decay_pos > 0 ) ? ( decay_pos - 1 ) : ( WMA_MAX_TIMELIST - 1 ) );

		if ( decay_list[ decay_pos ].first_decay_element != NULL )
		{
			decay_element = decay_list[ decay_pos ].first_decay_element;
			while ( decay_element != NULL )
			{
				print( my_agent, buf );

				sprintf( act_buf, "(%d: ", decay_element->this_wme->timetag );
				print( my_agent, act_buf );
				print_with_symbols( my_agent, "%y ^%y %y", decay_element->this_wme->id, decay_element->this_wme->attr, decay_element->this_wme->value );
				sprintf( act_buf, ")      num refs: %d      activation: ", decay_element->num_references);
				print( my_agent, act_buf );

				// Calculate and print the activation level
				sum = 0;
				for ( history_iter=0; history_iter<=( decay_element->history_count - 1 ); history_iter++ )
				{
					power_pos = ( decay_element->time_spot->time - decay_element->boost_history[ history_iter ] + 1 );
					if ( power_pos < WMA_POWER_SIZE )
						sum += my_agent->wma_power_array[ power_pos ];
				}
				sprintf( act_buf, "%f\n", sum );
				print( my_agent, act_buf );

				decay_element = decay_element->next;
			}
		}

		printf( "\n" );
	}

	print( my_agent, "END OF ACTIVATED WME LIST" );
}
