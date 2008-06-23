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
bool wma_set_parameter( agent *my_agent, const char *name, double new_val )
{
	const long param = wma_convert_parameter( my_agent, name );
	if ( param == WMA_PARAMS )
		return false;
	
	if ( !wma_valid_parameter_value( my_agent, param, new_val ) )
		return false;
	
	my_agent->wma_params[ param ]->param->number_param.value = new_val;

	return true;
}

bool wma_set_parameter( agent *my_agent, const char *name, const char *new_val )
{	
	const long param = wma_convert_parameter( my_agent, name );
	if ( param == WMA_PARAMS )
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
		set_sysparam( my_agent, WMA_ENABLED, converted_val );
	
	my_agent->wma_params[ param ]->param->constant_param.value = converted_val;

	return true;
}

bool wma_set_parameter( agent *my_agent, const char *name, const long new_val )
{
	const long param = wma_convert_parameter( my_agent, name );
	if ( param == WMA_PARAMS )
		return false;
	
	if ( !wma_valid_parameter_value( my_agent, param, new_val ) )
		return false;

	// learning special case
	if ( param == WMA_PARAM_ACTIVATION )
		set_sysparam( my_agent, WMA_ENABLED, new_val );
	
	my_agent->wma_params[ param ]->param->constant_param.value = new_val;

	return true;
}

//

bool wma_set_parameter( agent *my_agent, const long param, double new_val )
{	
	if ( !wma_valid_parameter_value( my_agent, param, new_val ) )
		return false;
	
	my_agent->wma_params[ param ]->param->number_param.value = new_val;

	return true;
}

bool wma_set_parameter( agent *my_agent, const long param, const char *new_val )
{	
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
		set_sysparam( my_agent, WMA_ENABLED, converted_val );
	
	my_agent->wma_params[ param ]->param->constant_param.value = converted_val;

	return true;
}

bool wma_set_parameter( agent *my_agent, const long param, const long new_val )
{	
	if ( !wma_valid_parameter_value( my_agent, param, new_val ) )
		return false;

	// learning special case
	if ( param == WMA_PARAM_ACTIVATION )
		set_sysparam( my_agent, WMA_ENABLED, new_val );
	
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
	return ( ( new_val >= -1 ) && ( new_val <= 0 ) );
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