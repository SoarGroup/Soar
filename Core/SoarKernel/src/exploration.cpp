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

/***************************************************************************
 * Function     : get_exploration_policy
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
 * Function     : set_exploration_policy
 **************************************************************************/
const long get_exploration_policy( agent *my_agent )
{
	return my_agent->sysparams[ USER_SELECT_MODE_SYSPARAM ];
}


/***************************************************************************
 * Function     : init_exploration_parameters
 **************************************************************************/
exploration_parameter *add_exploration_parameter( double value )
{
	exploration_parameter *newbie = new exploration_parameter;
	newbie->value = value;
	newbie->reduction_policy = EXPLORATION_EXPONENTIAL;
	newbie->rates[ EXPLORATION_EXPONENTIAL ] = 0;
	newbie->rates[ EXPLORATION_LINEAR ] = 0;
	
	return newbie;
}

/***************************************************************************
 * Function     : valid_parameter
 **************************************************************************/
bool valid_parameter( agent *my_agent, const char *name )
{
	return ( my_agent->exploration_params.find( name ) != my_agent->exploration_params.end() );
}

/***************************************************************************
 * Function     : get_parameter_value
 **************************************************************************/
double get_parameter_value( agent *my_agent, const char *parameter )
{
	if ( !valid_parameter( my_agent, parameter ) )
		return -1;
	
	return my_agent->exploration_params[ parameter ];
}
