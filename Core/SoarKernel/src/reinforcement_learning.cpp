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
#include <iostream>
#include <map>
#include <vector>
#include <string>

#include "agent.h"
#include "production.h"
#include "gdatastructs.h"
#include "rhsfun.h"

#include "reinforcement_learning.h"
#include "misc.h"

/***************************************************************************
 * Function     : add_rl_parameter
 **************************************************************************/
rl_parameter *add_rl_parameter( double value, bool (*val_func)( double ) )
{
	// new parameter entry
	rl_parameter *newbie = new rl_parameter;
	newbie->param = new rl_parameter_union;
	newbie->param->number_param.value = value;
	newbie->param->number_param.val_func = val_func;
	newbie->type = rl_param_number;
	
	return newbie;
}

rl_parameter *add_rl_parameter( const char *value, bool (*val_func)( const char * ), const char *(*to_str)( long ), const long (*from_str)( const char * ) )
{
	// new parameter entry
	rl_parameter *newbie = new rl_parameter;
	newbie->param = new rl_parameter_union;
	newbie->param->string_param.val_func = val_func;
	newbie->param->string_param.to_str = to_str;
	newbie->param->string_param.from_str = from_str;
	newbie->param->string_param.value = from_str( value );
	newbie->type = rl_param_string;
	
	return newbie;
}

/***************************************************************************
 * Function     : valid_rl_parameter
 **************************************************************************/
bool valid_rl_parameter( agent *my_agent, const char *name )
{
	return is_set( my_agent->rl_params, new std::string( name ) );
}

/***************************************************************************
 * Function     : get_rl_parameter_type
 **************************************************************************/
rl_param_type get_rl_parameter_type( agent *my_agent, const char *name )
{
	if ( !valid_rl_parameter( my_agent, name ) )
		return rl_param_invalid;
	
	return (*my_agent->rl_params)[ name ].type;
}

/***************************************************************************
 * Function     : get_rl_parameter
 **************************************************************************/
const long get_rl_parameter( agent *my_agent, const char *name, const double test )
{
	if ( get_rl_parameter_type( my_agent, name ) != rl_param_string )
		return NULL;
	
	return (*my_agent->rl_params)[ name ].param->string_param.value;
}

const char *get_rl_parameter( agent *my_agent, const char *name, const char *test )
{
	if ( get_rl_parameter_type( my_agent, name ) != rl_param_string )
		return NULL;
	
	return (*my_agent->rl_params)[ name ].param->string_param.to_str( (*my_agent->rl_params)[ name ].param->string_param.value );
}

double get_rl_parameter( agent *my_agent, const char *name )
{
	if ( get_rl_parameter_type( my_agent, name ) != rl_param_number )
		return NULL;
	
	return (*my_agent->rl_params)[ name ].param->number_param.value;
}

/***************************************************************************
 * Function     : valid_rl_parameter_value
 **************************************************************************/
bool valid_rl_parameter_value( agent *my_agent, const char *name, double new_val )
{
	if ( get_rl_parameter_type( my_agent, name ) != rl_param_number )
		return false;
	
	if ( !(*my_agent->rl_params)[ name ].param->number_param.val_func( new_val ) )
		return false;
	
	return true;
}

bool valid_rl_parameter_value( agent *my_agent, const char *name, const char *new_val )
{
	if ( get_rl_parameter_type( my_agent, name ) != rl_param_string )
		return false;
	
	if ( !(*my_agent->rl_params)[ name ].param->string_param.val_func( new_val ) )
		return false;
	
	return true;
}

bool valid_rl_parameter_value( agent *my_agent, const char *name, const long new_val )
{
	if ( get_rl_parameter_type( my_agent, name ) != rl_param_string )
		return false;
	
	const char *new_val_str = (*my_agent->rl_params)[ name ].param->string_param.to_str( new_val );
	if ( !(*my_agent->rl_params)[ name ].param->string_param.val_func( new_val_str ) )
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
	
	(*my_agent->rl_params)[ name ].param->number_param.value = new_val;
	return true;
}

bool set_rl_parameter( agent *my_agent, const char *name, const char *new_val )
{
	if ( !valid_rl_parameter_value( my_agent, name, new_val ) )
		return false;
	
	(*my_agent->rl_params)[ name ].param->string_param.value = (*my_agent->rl_params)[ name ].param->string_param.from_str( new_val );
	return true;
}

bool set_rl_parameter( agent *my_agent, const char *name, const long new_val )
{
	if ( !valid_rl_parameter_value( my_agent, name, new_val ) )
		return false;
	
	(*my_agent->rl_params)[ name ].param->string_param.value = new_val;
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
	return is_set( my_agent->rl_stats, new std::string( name ) );
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

/***************************************************************************
 * Function     : valid_rl_template
 **************************************************************************/
bool valid_rl_template( production *prod )
{
	bool numeric_pref = false;
	bool var_pref = false;
	int num_actions = 0;

	for ( action *a = prod->action_list; a; a = a->next ) 
	{
		num_actions++;
		if ( ( a->type == MAKE_ACTION ) )
		{
			if ( a->preference_type == NUMERIC_INDIFFERENT_PREFERENCE_TYPE )
			{
				numeric_pref = true;
			}
			else if ( a->preference_type == BINARY_INDIFFERENT_PREFERENCE_TYPE )
			{	
				if ( rhs_value_is_symbol( a->referent ) && ( rhs_value_to_symbol( a->referent )->id.common_symbol_info.symbol_type == VARIABLE_SYMBOL_TYPE ) )
					var_pref = true;
			}
		}
	}

	return ( ( num_actions == 1 ) && ( numeric_pref || var_pref ) );
}

/***************************************************************************
 * Function     : valid_rl_rule
 **************************************************************************/
bool valid_rl_rule( production *prod )
{
	bool numeric_pref = false;
	int num_actions = 0;

	for ( action *a = prod->action_list; a; a = a->next ) 
	{
		num_actions++;
		if ( ( a->type == MAKE_ACTION ) )
		{
			if ( a->preference_type == NUMERIC_INDIFFERENT_PREFERENCE_TYPE )
				numeric_pref = true;
		}
	}

	return ( numeric_pref && ( num_actions == 1 ) );
}

/***************************************************************************
 * Function     : get_template_base
 **************************************************************************/
template_instantiation *get_template_base( const char *prod_name )
{
	std::string temp = prod_name;
	template_instantiation *return_val = new template_instantiation;
	
	// has to be at least "rl*#*a" (where a is a single letter/number/etc)
	if ( temp.length() < 6 )
		return NULL;
	
	// check first three letters are "rl*"
	if ( temp.compare( 0, 3, "rl*" ) )
		return NULL;
	
	// find second * to isolate id
	std::string::size_type second_star = temp.find_first_of( '*', 3 );
	if ( second_star == std::string::npos )
		return NULL;
	
	// make sure there's something left after second_star
	if ( second_star == ( temp.length() - 1 ) )
		return NULL;
	
	// make sure id is a valid natural number
	std::string id_str = temp.substr( 3, ( second_star - 3 ) );
	if ( !is_natural_number( &id_str ) )
		return NULL;
	
	// convert id
	int id;
	from_string( id, id_str );
	
	// return info
	return_val->template_base = temp.substr( second_star + 1 );
	return_val->id = id;
	return return_val;
}

/***************************************************************************
 * Function     : initialize_template_tracking
 **************************************************************************/
void initialize_template_tracking( agent *my_agent )
{
	std::vector<std::string> *my_keys = map_keys( my_agent->rl_template_count );
	
	for ( size_t i=0; i<my_keys->size(); i++ )
		(*my_agent->rl_template_count)[ (*my_keys)[i] ] = 0;
}

/***************************************************************************
 * Function     : update_template_tracking
 **************************************************************************/
void update_template_tracking( agent *my_agent, const char *rule_name )
{
	template_instantiation *origin = get_template_base( rule_name );
	if ( origin != NULL )
	{
		if ( is_set( my_agent->rl_template_count, &(origin->template_base) ) )
		{
			if ( (*my_agent->rl_template_count)[ origin->template_base ] < origin->id )
				(*my_agent->rl_template_count)[ origin->template_base ] = origin->id;
		}
		else
		{
			(*my_agent->rl_template_count)[ origin->template_base ] = origin->id;
		}
	}
}

/***************************************************************************
 * Function     : revert_template_tracking
 **************************************************************************/
void revert_template_tracking( agent *my_agent, const char *rule_name )
{
	template_instantiation *origin = get_template_base( rule_name );
	if ( ( origin != NULL ) && ( (*my_agent->rl_template_count)[ origin->template_base ] == origin->id ) )
	{
		int temp_id = 0;
		template_instantiation *temp_origin;
		
		for ( int i=0; i<NUM_PRODUCTION_TYPES; i++ )
		{
			for ( production *prod=my_agent->all_productions_of_type[i]; prod != NIL; prod = prod->next )
			{
				temp_origin = get_template_base( prod->name->sc.name );
				if ( temp_origin != NULL )
				{
					if ( !origin->template_base.compare( temp_origin->template_base ) )
					{
						if ( ( temp_origin->id != origin->id ) && ( temp_origin->id > temp_id ) )
							temp_id = temp_origin->id;
					}
				}
			}
		}
		
		(*my_agent->rl_template_count)[ origin->template_base ] = temp_id;
	}
}

/***************************************************************************
 * Function     : next_template_id
 **************************************************************************/
int next_template_id( agent *my_agent, const char *template_name )
{
	std::string *temp = new std::string( template_name );
	int return_val;
	
	if ( !is_set( my_agent->rl_template_count, temp ) )
	{
		// first instantiation is 1
		return_val = 1;
	}
	else
	{
		// get current value + 1
		return_val = (*my_agent->rl_template_count)[ *temp ];
		return_val++;
	}
	
	// increment counter
	(*my_agent->rl_template_count)[ *temp ] = return_val;
	
	return return_val;
}
