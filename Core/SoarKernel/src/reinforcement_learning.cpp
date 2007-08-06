#include <portability.h>

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/*************************************************************************
 *
 *  file:  reinforcement_learning.cpp
 *
 * =======================================================================
 * Description  :  Various functions for Soar-RL
 * =======================================================================
 */

#include <stdlib.h>

#include "agent.h"
#include "reinforcement_learning.h"


/***************************************************************************
 * Function     : add_tracking
 **************************************************************************/
rl_parameter_tracking *add_rl_tracking( const char *name, rl_param_type type )
{
	// new tracking entry
	rl_parameter_tracking *newbie = new rl_parameter_tracking;
	newbie->name = name;
	newbie->type = type;
	
	return newbie;
}

/***************************************************************************
 * Function     : add_rl_parameter
 **************************************************************************/
rl_parameter *add_rl_parameter( double value, bool (*val_func)( double ) )
{
	// new parameter entry
	rl_parameter *newbie = new rl_parameter;
	newbie->number_param.value = value;
	newbie->number_param.val_func = val_func;
	
	return newbie;
}

rl_parameter *add_rl_parameter( const char *value, bool (*val_func)( const char * ), const char *(*to_str)( long ), const long (*from_str)( const char * ) )
{
	// new parameter entry
	rl_parameter *newbie = new rl_parameter;
	newbie->string_param.val_func = val_func;
	newbie->string_param.to_str = to_str;
	newbie->string_param.from_str = from_str;
	newbie->string_param.value = from_str( value );
	
	return newbie;
}

/***************************************************************************
 * Function     : valid_rl_parameter
 **************************************************************************/
bool valid_rl_parameter( agent *my_agent, const char *name )
{
	return ( ( (*my_agent->rl_params).find( name ) != (*my_agent->rl_params).end() ) &&
			 ( (*my_agent->rl_param_tracking).find( name ) != (*my_agent->rl_param_tracking).end() ) );
}

/***************************************************************************
 * Function     : get_rl_parameter_type
 **************************************************************************/
rl_param_type get_rl_parameter_type( agent *my_agent, const char *name )
{
	if ( !valid_rl_parameter( my_agent, name ) )
		return rl_param_invalid;
	
	return (*my_agent->rl_param_tracking)[ name ].type;
}

/***************************************************************************
 * Function     : get_rl_parameter
 **************************************************************************/
const long get_rl_parameter( agent *my_agent, const char *name, const double test )
{
	if ( get_rl_parameter_type( my_agent, name ) != rl_param_string )
		return NULL;
	
	return (*my_agent->rl_params)[ name ].string_param.value;
}

const char *get_rl_parameter( agent *my_agent, const char *name, const char *test )
{
	if ( get_rl_parameter_type( my_agent, name ) != rl_param_string )
		return NULL;
	
	return (*my_agent->rl_params)[ name ].string_param.to_str( (*my_agent->rl_params)[ name ].string_param.value );
}

double get_rl_parameter( agent *my_agent, const char *name )
{
	if ( get_rl_parameter_type( my_agent, name ) != rl_param_number )
		return NULL;
	
	return (*my_agent->rl_params)[ name ].number_param.value;
}

/***************************************************************************
 * Function     : valid_rl_parameter_value
 **************************************************************************/
bool valid_rl_parameter_value( agent *my_agent, const char *name, double new_val )
{
	if ( get_rl_parameter_type( my_agent, name ) != rl_param_number )
		return false;
	
	if ( !(*my_agent->rl_params)[ name ].number_param.val_func( new_val ) )
		return false;
	
	return true;
}

bool valid_rl_parameter_value( agent *my_agent, const char *name, const char *new_val )
{
	if ( get_rl_parameter_type( my_agent, name ) != rl_param_string )
		return false;
	
	if ( !(*my_agent->rl_params)[ name ].string_param.val_func( new_val ) )
		return false;
	
	return true;
}

bool valid_rl_parameter_value( agent *my_agent, const char *name, const long new_val )
{
	if ( get_rl_parameter_type( my_agent, name ) != rl_param_string )
		return false;
	
	const char *new_val_str = (*my_agent->rl_params)[ name ].string_param.to_str( new_val );
	if ( !(*my_agent->rl_params)[ name ].string_param.val_func( new_val_str ) )
		return false;
	
	return true;
}

/***************************************************************************
 * Function     : set_rl_parameter
 **************************************************************************/
bool set_rl_parameter( agent *my_agent, const char *name, double new_val )
{
	if ( !valid_rl_parameter_value( my_agent, name, new_val ) )
		return false;
	
	(*my_agent->rl_params)[ name ].number_param.value = new_val;
	return true;
}

bool set_rl_parameter( agent *my_agent, const char *name, const char *new_val )
{
	if ( !valid_rl_parameter_value( my_agent, name, new_val ) )
		return false;
	
	(*my_agent->rl_params)[ name ].string_param.value = (*my_agent->rl_params)[ name ].string_param.from_str( new_val );
	return true;
}

bool set_rl_parameter( agent *my_agent, const char *name, const long new_val )
{
	if ( !valid_rl_parameter_value( my_agent, name, new_val ) )
		return false;
	
	(*my_agent->rl_params)[ name ].string_param.value = new_val;
	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////
// learning
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : validate_rl_learning
 **************************************************************************/
bool validate_rl_learning( const char *new_val )
{
	return ( !strcmp( new_val, "on" ) || !strcmp( new_val, "off" ) );
}

/***************************************************************************
 * Function     : convert_rl_learning
 **************************************************************************/
const char *convert_rl_learning( long val )
{
	const char *return_val = NULL;
	
	switch ( val )
	{
		case RL_LEARNING_ON:
			return_val = "on";
			break;
			
		case RL_LEARNING_OFF:
			return_val = "off";
			break;
	}
	
	return return_val;
}

const long convert_rl_learning( const char *val )
{
	long return_val = NULL;
	
	if ( !strcmp( val, "on" ) )
		return_val = RL_LEARNING_ON;
	else if ( !strcmp( val, "off" ) )
		return_val = RL_LEARNING_OFF;
	
	return return_val;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// accumulation mode
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : validate_rl_accumulation
 **************************************************************************/
bool validate_rl_accumulation( const char *new_val )
{
	return ( !strcmp( new_val, "sum" ) || !strcmp( new_val, "avg" ) );
}

/***************************************************************************
 * Function     : convert_rl_accumulation
 **************************************************************************/
const char *convert_rl_accumulation( long val )
{
	const char *return_val = NULL;
	
	switch ( val )
	{
		case RL_ACCUMULATION_SUM:
			return_val = "sum";
			break;
			
		case RL_ACCUMULATION_AVG:
			return_val = "avg";
			break;
	}
	
	return return_val;
}

const long convert_rl_accumulation( const char *val )
{
	long return_val = NULL;
	
	if ( !strcmp( val, "sum" ) )
		return_val = RL_ACCUMULATION_SUM;
	else if ( !strcmp( val, "avg" ) )
		return_val = RL_ACCUMULATION_AVG;
	
	return return_val;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// discount mode
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : validate_rl_discount
 **************************************************************************/
bool validate_rl_discount( const char *new_val )
{
	return ( !strcmp( new_val, "exponential" ) || !strcmp( new_val, "linear" ) );
}

/***************************************************************************
 * Function     : convert_rl_discount
 **************************************************************************/
const char *convert_rl_discount( long val )
{
	const char *return_val = NULL;
	
	switch ( val )
	{
		case RL_DISCOUNT_EXPONENTIAL:
			return_val = "exponential";
			break;
			
		case RL_DISCOUNT_LINEAR:
			return_val = "linear";
			break;
	}
	
	return return_val;
}

const long convert_rl_discount( const char *val )
{
	long return_val = NULL;
	
	if ( !strcmp( val, "exponential" ) )
		return_val = RL_DISCOUNT_EXPONENTIAL;
	else if ( !strcmp( val, "linear" ) )
		return_val = RL_DISCOUNT_LINEAR;
	
	return return_val;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// exponential discount rate
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : validate_rl_exp_discount
 **************************************************************************/
bool validate_rl_exp_discount( double new_val )
{
	return ( ( new_val >= 0 ) && ( new_val <= 1 ) );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// linear discount rate
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : validate_rl_lin_discount
 **************************************************************************/
bool validate_rl_lin_discount( double new_val )
{
	return ( ( new_val >= 0 ) && ( new_val <= 1 ) );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// learning rate
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : validate_rl_learning_rate
 **************************************************************************/
bool validate_rl_learning_rate( double new_val )
{
	return ( ( new_val >= 0 ) && ( new_val <= 1 ) );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// learning policy
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : validate_rl_learning_policy
 **************************************************************************/
bool validate_rl_learning_policy( const char *new_val )
{
	return ( !strcmp( new_val, "sarsa" ) || !strcmp( new_val, "q-learning" ) );
}

/***************************************************************************
 * Function     : convert_rl_learning_policy
 **************************************************************************/
const char *convert_rl_learning_policy( long val )
{
	const char *return_val = NULL;
	
	switch ( val )
	{
		case RL_LEARNING_SARSA:
			return_val = "sarsa";
			break;
			
		case RL_LEARNING_Q:
			return_val = "q-learning";
			break;
	}
	
	return return_val;
}

const long convert_rl_learning_policy( const char *val )
{
	long return_val = NULL;
	
	if ( !strcmp( val, "sarsa" ) )
		return_val = RL_LEARNING_SARSA;
	else if ( !strcmp( val, "q-learning" ) )
		return_val = RL_LEARNING_Q;
	
	return return_val;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// eligibility trace decay rate
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : validate_rl_decay_rate
 **************************************************************************/
bool validate_rl_decay_rate( double new_val )
{
	return ( ( new_val >= 0 ) && ( new_val <= 1 ) );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// eligibility trace tolerance
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : validate_rl_trace_tolerance
 **************************************************************************/
bool validate_rl_trace_tolerance( double new_val )
{
	return ( new_val > 0 );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : valid_rl_stat
 **************************************************************************/
bool valid_rl_stat( agent *my_agent, const char *name )
{
	return ( (*my_agent->rl_stats).find( name ) != (*my_agent->rl_stats).end() );
}

/***************************************************************************
 * Function     : get_rl_stat
 **************************************************************************/
double get_rl_stat( agent *my_agent, const char *name )
{
	if ( !valid_rl_stat( my_agent, name ) )
		return 0;
	
	return (*my_agent->rl_stats)[ name ];
}

/***************************************************************************
 * Function     : set_rl_stat
 **************************************************************************/
bool set_rl_stat( agent *my_agent, const char *name, double new_val )
{
	if ( !valid_rl_stat( my_agent, name ) )
		return false;
	
	(*my_agent->rl_stats)[ name ] = new_val;
	
	return true;
}
