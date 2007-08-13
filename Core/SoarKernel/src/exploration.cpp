#include <portability.h>

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/*************************************************************************
 *
 *  file:  exploration.cpp
 *
 * =======================================================================
 * Description  :  Various functions for exploration
 * =======================================================================
 */

#include <stdlib.h>

#include "agent.h"
#include "exploration.h"
#include "misc.h"

/***************************************************************************
 * Function     : convert_exploration_policy
 **************************************************************************/
const long convert_exploration_policy( const char *policy_name )
{
	if ( !strcmp( policy_name, "boltzmann" ) )
		return USER_SELECT_BOLTZMANN;
	if ( !strcmp( policy_name, "epsilon-greedy" ) )
		return USER_SELECT_E_GREEDY;
	if ( !strcmp( policy_name, "first" ) )
		return USER_SELECT_FIRST;
	if ( !strcmp( policy_name, "last" ) )
		return USER_SELECT_LAST;
	if ( !strcmp( policy_name, "random-uniform" ) )
		return USER_SELECT_RANDOM;
	
	return NULL;
}

const char *convert_exploration_policy( const long policy )
{
	if ( policy == USER_SELECT_BOLTZMANN )
		return "boltzmann";
	if ( policy == USER_SELECT_E_GREEDY )
		return "epsilon-greedy";
	if ( policy == USER_SELECT_FIRST )
		return "first";
	if ( policy == USER_SELECT_LAST )
		return "last";
	if ( policy == USER_SELECT_RANDOM )
		return "random-uniform";
	
	return NULL;
}

/***************************************************************************
 * Function     : set_exploration_policy
 **************************************************************************/
bool set_exploration_policy( agent *my_agent, const char *policy_name )
{	
	const long policy = convert_exploration_policy( policy_name );
	
	if ( policy != NULL )
		return set_exploration_policy( my_agent, policy );
	
	return false;
}

bool set_exploration_policy( agent *my_agent, const long policy )
{	
	const char *policy_name = convert_exploration_policy( policy );
	if ( policy_name != NULL )
		set_sysparam( my_agent, USER_SELECT_MODE_SYSPARAM, policy );
	
	return false;
}

/***************************************************************************
 * Function     : get_exploration_policy
 **************************************************************************/
const long get_exploration_policy( agent *my_agent )
{
	return my_agent->sysparams[ USER_SELECT_MODE_SYSPARAM ];
}

/***************************************************************************
 * Function     : add_exploration_parameter
 **************************************************************************/
exploration_parameter *add_exploration_parameter( double value, bool (*val_func)( double ) )
{
	// new parameter entry
	exploration_parameter *newbie = new exploration_parameter;
	newbie->value = value;
	newbie->reduction_policy = EXPLORATION_EXPONENTIAL;
	newbie->val_func = val_func;
	newbie->rates[ EXPLORATION_EXPONENTIAL ] = 0;
	newbie->rates[ EXPLORATION_LINEAR ] = 0;
	
	return newbie;
}

/***************************************************************************
 * Function     : valid_parameter
 **************************************************************************/
bool valid_parameter( agent *my_agent, const char *name )
{	
	std::string *temp = new std::string( name );
	bool return_val = is_set( my_agent->exploration_params, temp );
	delete temp;
	
	return return_val;
}

/***************************************************************************
 * Function     : get_parameter_value
 **************************************************************************/
double get_parameter_value( agent *my_agent, const char *parameter )
{
	if ( !valid_parameter( my_agent, parameter ) )
		return 0;
	
	return (*my_agent->exploration_params)[ parameter ].value;
}

/***************************************************************************
 * Function     : validate_epsilon
 **************************************************************************/
bool validate_epsilon( double value )
{
	return ( ( value >= 0 ) && ( value <= 1 ) );
}

/***************************************************************************
 * Function     : validate_temperature
 **************************************************************************/
bool validate_temperature( double value )
{
	return ( value > 0 );
}

/***************************************************************************
 * Function     : valid_parameter_value
 **************************************************************************/
bool valid_parameter_value( agent *my_agent, const char *name, double value )
{
	if ( !valid_parameter( my_agent, name ) )
		return false;
	
	return (*my_agent->exploration_params)[ name ].val_func( value );
}

/***************************************************************************
 * Function     : set_parameter_value
 **************************************************************************/
bool set_parameter_value( agent *my_agent, const char *name, double value )
{
	if ( !valid_parameter_value( my_agent, name, value ) )
		return false;
	
	(*my_agent->exploration_params)[ name ].value = value;
	
	return true;
}

/***************************************************************************
 * Function     : get_parameter_names
 **************************************************************************/
std::vector<std::string> *get_parameter_names( agent *my_agent )
{
	return map_keys( &(*my_agent->exploration_params) );
}

/***************************************************************************
 * Function     : convert_reduction_policy
 **************************************************************************/
const long convert_reduction_policy( const char *policy_name )
{
	if ( !strcmp( policy_name, "exponential" ) )
		return EXPLORATION_EXPONENTIAL;
	if ( !strcmp( policy_name, "linear" ) )
		return EXPLORATION_LINEAR;
	
	return NULL;
}

const char *convert_reduction_policy( const long policy )
{
	if ( policy == EXPLORATION_EXPONENTIAL )
		return "exponential";
	if ( policy == EXPLORATION_LINEAR )
		return "linear";
	
	return NULL;
}

/***************************************************************************
 * Function     : get_reduction_policy
 **************************************************************************/
const long get_reduction_policy( agent *my_agent, const char *parameter )
{
	if ( !valid_parameter( my_agent, parameter ) )
		return false;
	
	return (*my_agent->exploration_params)[ parameter ].reduction_policy;
}

/***************************************************************************
 * Function     : valid_reduction_policy
 **************************************************************************/
bool valid_reduction_policy( agent *my_agent, const char *parameter, const char *policy_name )
{	
	const long policy = convert_reduction_policy( policy_name );
	if ( policy == NULL )
		return false;
	
	return valid_reduction_policy( my_agent, parameter, policy );
}

bool valid_reduction_policy( agent *my_agent, const char *parameter, const long policy )
{	
	if ( !valid_parameter( my_agent, parameter ) )
		return false;
	
	long *temp = new long;
	*temp = policy;
	bool return_val = is_set( (&(*my_agent->exploration_params)[ parameter ].rates), temp );
	delete temp;

	return return_val;
}

/***************************************************************************
 * Function     : set_reduction_policy
 **************************************************************************/
bool set_reduction_policy( agent *my_agent, const char *parameter, const char *policy_name )
{
	const long policy = convert_reduction_policy( policy_name );
	if ( policy == NULL )
		return false;
	
	if ( !valid_reduction_policy( my_agent, parameter, policy ) )
		return false;
	
	(*my_agent->exploration_params)[ parameter ].reduction_policy = policy;
	
	return true;
}

/***************************************************************************
 * Function     : get_reduction_policies
 **************************************************************************/
std::vector<const char *> *get_reduction_policies( agent *my_agent, const char *parameter )
{
	std::vector<const char *> *return_val = new std::vector<const char *>();
	
	if ( valid_reduction_policy( my_agent, parameter, "exponential" ) )
		return_val->push_back( "exponential" );
	
	if ( valid_reduction_policy( my_agent, parameter, "linear" ) )
		return_val->push_back( "linear" );
	
	return return_val;
}

/***************************************************************************
 * Function     : valid_reduction_rate
 **************************************************************************/
bool valid_reduction_rate( agent *my_agent, const char *parameter, const char *policy_name, double reduction_rate )
{
	const long policy = convert_reduction_policy( policy_name );
	if ( policy == NULL )
		return false;
	
	return valid_reduction_rate( my_agent, parameter, policy, reduction_rate );
}

bool valid_reduction_rate( agent *my_agent, const char *parameter, const long policy, double reduction_rate )
{
	if ( !valid_reduction_policy( my_agent, parameter, policy ) )
		return false;
	
	switch ( policy )
	{
		case EXPLORATION_EXPONENTIAL:
			return valid_exponential( reduction_rate );
			break;
			
		case EXPLORATION_LINEAR:
			return valid_linear( reduction_rate );
			break;
			
		default:
			return false;
			break;
	}
}

/***************************************************************************
 * Function     : valid_exponential
 **************************************************************************/
bool valid_exponential( double reduction_rate )
{
	return ( ( reduction_rate >= 0 ) && ( reduction_rate <= 1 ) );
}

/***************************************************************************
 * Function     : valid_linear
 **************************************************************************/
bool valid_linear( double reduction_rate )
{
	return ( reduction_rate >= 0 );
}

/***************************************************************************
 * Function     : get_reduction_rate
 **************************************************************************/
double get_reduction_rate( agent *my_agent, const char *parameter, const char *policy_name )
{
	const long policy = convert_reduction_policy( policy_name );
	if ( policy == NULL )
		return 0;
	
	return get_reduction_rate( my_agent, parameter, policy );
}

double get_reduction_rate( agent *my_agent, const char *parameter, const long policy )
{
	if ( !valid_parameter( my_agent, parameter ) )
		return 0;
	
	if ( !valid_reduction_policy( my_agent, parameter, policy ) )
		return 0;
	
	return (*my_agent->exploration_params)[ parameter ].rates[ policy ];
}

/***************************************************************************
 * Function     : set_reduction_rate
 **************************************************************************/
bool set_reduction_rate( agent *my_agent, const char *parameter, const char *policy_name, double reduction_rate )
{
	const long policy = convert_reduction_policy( policy_name );
	if ( policy == NULL )
		return false;
	
	return set_reduction_rate( my_agent, parameter, policy, reduction_rate );
}

bool set_reduction_rate( agent *my_agent, const char *parameter, const long policy, double reduction_rate )
{
	if ( !valid_parameter( my_agent, parameter ) )
		return false;
	
	if ( !valid_reduction_policy( my_agent, parameter, policy ) )
		return false;
	
	if ( !valid_reduction_rate( my_agent, parameter, policy, reduction_rate ) )
		return false;
	
	(*my_agent->exploration_params)[ parameter ].rates[ policy ] = reduction_rate;
	
	return true;
}
