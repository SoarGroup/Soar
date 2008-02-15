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
#include <math.h>

#include <iostream>
#include <map>
#include <vector>
#include <string>

#include "agent.h"
#include "production.h"
#include "gdatastructs.h"
#include "rhsfun.h"
#include "recmem.h"
#include "chunk.h"
#include "rete.h"
#include "wmem.h"

#include "xmlTraceNames.h"
#include "gski_event_system_functions.h"
#include "print.h"

#include "reinforcement_learning.h"
#include "misc.h"

extern Symbol *instantiate_rhs_value (agent* thisAgent, rhs_value rv, goal_stack_level new_id_level, char new_id_letter, struct token_struct *tok, wme *w);
extern void variablize_symbol (agent* thisAgent, Symbol **sym);
extern void variablize_nots_and_insert_into_conditions (agent* thisAgent, not_struct *nots, condition *conds);
extern void variablize_condition_list (agent* thisAgent, condition *cond);


/***************************************************************************
 * Function     : clean_parameters
 **************************************************************************/
void clean_parameters( agent *my_agent )
{
	for ( int i=0; i<RL_PARAMS; i++ )
	  delete my_agent->rl_params[ i ];
}

/***************************************************************************
 * Function     : clean_stats
 **************************************************************************/
void clean_stats( agent *my_agent )
{
	for ( int i=0; i<RL_STATS; i++ )
	  delete my_agent->rl_stats[ i ];
}

/***************************************************************************
 * Function     : reset_rl_data
 **************************************************************************/
void reset_rl_data( agent *my_agent )
{
	Symbol *goal = my_agent->top_goal;
	while( goal )
	{
		rl_data *data = goal->id.rl_info;

		data->eligibility_traces->clear(); 
		
		free_list( my_agent, data->prev_op_rl_rules );
		data->prev_op_rl_rules = NIL;
		data->num_prev_op_rl_rules = 0;

		data->previous_q = 0;
		data->reward = 0;
		data->step = 0;
		data->reward_age = 0;
		data->impasse_type = NONE_IMPASSE_TYPE;
		
		goal = goal->id.lower_goal;
	}
}

/***************************************************************************
 * Function     : reset_rl
 **************************************************************************/
void reset_rl_stats( agent *my_agent )
{
	for ( int i=0; i<RL_STATS; i++ )
		my_agent->rl_stats[ i ]->value = 0;
}

/***************************************************************************
 * Function     : remove_rl_refs_for_prod
 **************************************************************************/
void remove_rl_refs_for_prod( agent *my_agent, production *prod )
{
	for ( Symbol* state = my_agent->top_state; state; state = state->id.lower_goal )
	{
		state->id.rl_info->eligibility_traces->erase( prod );
		
		for ( cons *c = state->id.rl_info->prev_op_rl_rules; c; c = c->rest )
			if ( static_cast<production *>(c->first) == prod ) 
				c->first = NIL;
	}
}

/***************************************************************************
 * Function     : add_rl_parameter
 **************************************************************************/
rl_parameter *add_rl_parameter( const char *name, double value, bool (*val_func)( double ) )
{
	// new parameter entry
	rl_parameter *newbie = new rl_parameter;
	newbie->param = new rl_parameter_union;
	newbie->param->number_param.value = value;
	newbie->param->number_param.val_func = val_func;
	newbie->type = rl_param_number;
	newbie->name = name;
	
	return newbie;
}

rl_parameter *add_rl_parameter( const char *name, const long value, bool (*val_func)( const long ), const char *(*to_str)( long ), const long (*from_str)( const char * ) )
{
	// new parameter entry
	rl_parameter *newbie = new rl_parameter;
	newbie->param = new rl_parameter_union;
	newbie->param->string_param.val_func = val_func;
	newbie->param->string_param.to_str = to_str;
	newbie->param->string_param.from_str = from_str;
	newbie->param->string_param.value = value;
	newbie->type = rl_param_string;
	newbie->name = name;
	
	return newbie;
}

/***************************************************************************
 * Function     : convert_rl_parameter
 **************************************************************************/
const char *convert_rl_parameter( agent *my_agent, const long param )
{
	if ( ( param < 0 ) || ( param >= RL_PARAMS ) )
		return NULL;

	return my_agent->rl_params[ param ]->name;
}

const long convert_rl_parameter( agent *my_agent, const char *name )
{
	for ( int i=0; i<RL_PARAMS; i++ )
		if ( !strcmp( name, my_agent->rl_params[ i ]->name ) )
			return i;

	return RL_PARAMS;
}

/***************************************************************************
 * Function     : valid_rl_parameter
 **************************************************************************/
bool valid_rl_parameter( agent *my_agent, const char *name )
{
	return ( convert_rl_parameter( my_agent, name ) != RL_PARAMS );
}

bool valid_rl_parameter( agent *my_agent, const long param )
{
	return ( convert_rl_parameter( my_agent, param ) != NULL );
}

/***************************************************************************
 * Function     : get_rl_parameter_type
 **************************************************************************/
rl_param_type get_rl_parameter_type( agent *my_agent, const char *name )
{
	const long param = convert_rl_parameter( my_agent, name );
	if ( param == RL_PARAMS )
		return rl_param_invalid;
	
	return my_agent->rl_params[ param ]->type;
}

rl_param_type get_rl_parameter_type( agent *my_agent, const long param )
{
	if ( !valid_rl_parameter( my_agent, param ) )
		return rl_param_invalid;

	return my_agent->rl_params[ param ]->type;
}

/***************************************************************************
 * Function     : get_rl_parameter
 **************************************************************************/
const long get_rl_parameter( agent *my_agent, const char *name, const double test )
{
	const long param = convert_rl_parameter( my_agent, name );
	if ( param == RL_PARAMS )
		return NULL;
	
	if ( get_rl_parameter_type( my_agent, param ) != rl_param_string )
		return NULL;
	
	return my_agent->rl_params[ param ]->param->string_param.value;
}

const char *get_rl_parameter( agent *my_agent, const char *name, const char *test )
{
	const long param = convert_rl_parameter( my_agent, name );
	if ( param == RL_PARAMS )
		return NULL;
	
	if ( get_rl_parameter_type( my_agent, param ) != rl_param_string )
		return NULL;
	
	return my_agent->rl_params[ param ]->param->string_param.to_str( my_agent->rl_params[ param ]->param->string_param.value );
}

double get_rl_parameter( agent *my_agent, const char *name )
{
	const long param = convert_rl_parameter( my_agent, name );
	if ( param == RL_PARAMS )
		return NULL;
	
	if ( get_rl_parameter_type( my_agent, param ) != rl_param_number )
		return NULL;
	
	return my_agent->rl_params[ param ]->param->number_param.value;
}

//

const long get_rl_parameter( agent *my_agent, const long param, const double test )
{
	if ( !valid_rl_parameter( my_agent, param ) )
		return NULL;

	if ( get_rl_parameter_type( my_agent, param ) != rl_param_string )
		return NULL;
	
	return my_agent->rl_params[ param ]->param->string_param.value;
}

const char *get_rl_parameter( agent *my_agent, const long param, const char *test )
{
	if ( !valid_rl_parameter( my_agent, param ) )
		return NULL;
	
	if ( get_rl_parameter_type( my_agent, param ) != rl_param_string )
		return NULL;
	
	return my_agent->rl_params[ param ]->param->string_param.to_str( my_agent->rl_params[ param ]->param->string_param.value );
}

double get_rl_parameter( agent *my_agent, const long param )
{
	if ( !valid_rl_parameter( my_agent, param ) )
		return NULL;
	
	if ( get_rl_parameter_type( my_agent, param ) != rl_param_number )
		return NULL;
	
	return my_agent->rl_params[ param ]->param->number_param.value;
}

/***************************************************************************
 * Function     : valid_rl_parameter_value
 **************************************************************************/
bool valid_rl_parameter_value( agent *my_agent, const char *name, double new_val )
{
	const long param = convert_rl_parameter( my_agent, name );
	if ( param == RL_PARAMS )
		return false;
	
	if ( get_rl_parameter_type( my_agent, param ) != rl_param_number )
		return false;
	
	return my_agent->rl_params[ param ]->param->number_param.val_func( new_val );
}

bool valid_rl_parameter_value( agent *my_agent, const char *name, const char *new_val )
{
	const long param = convert_rl_parameter( my_agent, name );
	if ( param == RL_PARAMS )
		return false;
	
	if ( get_rl_parameter_type( my_agent, param ) != rl_param_string )
		return false;
	
	return my_agent->rl_params[ param ]->param->string_param.val_func( my_agent->rl_params[ param ]->param->string_param.from_str( new_val ) );
}

bool valid_rl_parameter_value( agent *my_agent, const char *name, const long new_val )
{
	const long param = convert_rl_parameter( my_agent, name );
	if ( param == RL_PARAMS )
		return false;
	
	if ( get_rl_parameter_type( my_agent, param ) != rl_param_string )
		return false;
	
	return my_agent->rl_params[ param ]->param->string_param.val_func( new_val );
}

//

bool valid_rl_parameter_value( agent *my_agent, const long param, double new_val )
{
	if ( !valid_rl_parameter( my_agent, param ) )
		return false;
	
	if ( get_rl_parameter_type( my_agent, param ) != rl_param_number )
		return false;
	
	return my_agent->rl_params[ param ]->param->number_param.val_func( new_val );
}

bool valid_rl_parameter_value( agent *my_agent, const long param, const char *new_val )
{
	if ( !valid_rl_parameter( my_agent, param ) )
		return false;
	
	if ( get_rl_parameter_type( my_agent, param ) != rl_param_string )
		return false;
	
	return my_agent->rl_params[ param ]->param->string_param.val_func( my_agent->rl_params[ param ]->param->string_param.from_str( new_val ) );
}

bool valid_rl_parameter_value( agent *my_agent, const long param, const long new_val )
{
	if ( !valid_rl_parameter( my_agent, param ) )
		return false;
	
	if ( get_rl_parameter_type( my_agent, param ) != rl_param_string )
		return false;
	
	return my_agent->rl_params[ param ]->param->string_param.val_func( new_val );
}

/***************************************************************************
 * Function     : set_rl_parameter
 **************************************************************************/
bool set_rl_parameter( agent *my_agent, const char *name, double new_val )
{
	const long param = convert_rl_parameter( my_agent, name );
	if ( param == RL_PARAMS )
		return false;
	
	if ( !valid_rl_parameter_value( my_agent, param, new_val ) )
		return false;
	
	my_agent->rl_params[ param ]->param->number_param.value = new_val;

	return true;
}

bool set_rl_parameter( agent *my_agent, const char *name, const char *new_val )
{
	const long param = convert_rl_parameter( my_agent, name );
	if ( param == RL_PARAMS )
		return false;
	
	if ( !valid_rl_parameter_value( my_agent, param, new_val ) )
		return false;

	const long converted_val = my_agent->rl_params[ param ]->param->string_param.from_str( new_val );

	// learning special case
	if ( param == RL_PARAM_LEARNING )
		set_sysparam( my_agent, RL_ENABLED, converted_val );
	
	my_agent->rl_params[ param ]->param->string_param.value = converted_val;

	return true;
}

bool set_rl_parameter( agent *my_agent, const char *name, const long new_val )
{
	const long param = convert_rl_parameter( my_agent, name );
	if ( param == RL_PARAMS )
		return false;
	
	if ( !valid_rl_parameter_value( my_agent, param, new_val ) )
		return false;

	// learning special case
	if ( param == RL_PARAM_LEARNING )
		set_sysparam( my_agent, RL_ENABLED, new_val );
	
	my_agent->rl_params[ param ]->param->string_param.value = new_val;

	return true;
}

//

bool set_rl_parameter( agent *my_agent, const long param, double new_val )
{
	if ( !valid_rl_parameter_value( my_agent, param, new_val ) )
		return false;
	
	my_agent->rl_params[ param ]->param->number_param.value = new_val;

	return true;
}

bool set_rl_parameter( agent *my_agent, const long param, const char *new_val )
{
	if ( !valid_rl_parameter_value( my_agent, param, new_val ) )
		return false;

	const long converted_val = my_agent->rl_params[ param ]->param->string_param.from_str( new_val );

	// learning special case
	if ( param == RL_PARAM_LEARNING )
		set_sysparam( my_agent, RL_ENABLED, converted_val );
	
	my_agent->rl_params[ param ]->param->string_param.value = converted_val;

	return true;
}

bool set_rl_parameter( agent *my_agent, const long param, const long new_val )
{	
	if ( !valid_rl_parameter_value( my_agent, param, new_val ) )
		return false;

	// learning special case
	if ( param == RL_PARAM_LEARNING )
		set_sysparam( my_agent, RL_ENABLED, new_val );
	
	my_agent->rl_params[ param ]->param->string_param.value = new_val;

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// learning
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : validate_rl_learning
 **************************************************************************/
bool validate_rl_learning( const long new_val )
{
	return ( ( new_val == RL_LEARNING_ON ) || ( new_val == RL_LEARNING_OFF ) );
}

/***************************************************************************
 * Function     : convert_rl_learning
 **************************************************************************/
const char *convert_rl_learning( const long val )
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
// discount rate
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : validate_rl_discount
 **************************************************************************/
bool validate_rl_discount( const double new_val )
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
bool validate_rl_learning_rate( const double new_val )
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
bool validate_rl_learning_policy( const long new_val )
{
	return ( ( new_val == RL_LEARNING_SARSA ) || ( new_val == RL_LEARNING_Q ) );
}

/***************************************************************************
 * Function     : convert_rl_learning_policy
 **************************************************************************/
const char *convert_rl_learning_policy( const long val )
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
bool validate_rl_decay_rate( const double new_val )
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
bool validate_rl_trace_tolerance( const double new_val )
{
	return ( new_val > 0 );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// temporal-extension
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : validate_te_enabled
 **************************************************************************/
bool validate_te_enabled( const long new_val )
{
	return ( ( new_val == RL_TE_ON ) || ( new_val == RL_TE_OFF ) );
}

/***************************************************************************
 * Function     : convert_te_enabled
 **************************************************************************/
const char *convert_te_enabled( const long val )
{
	const char *return_val = NULL;
	
	switch ( val )
	{
		case RL_TE_ON:
			return_val = "on";
			break;
			
		case RL_TE_OFF:
			return_val = "off";
			break;
	}
	
	return return_val;
}

const long convert_te_enabled( const char *val )
{
	long return_val = NULL;
	
	if ( !strcmp( val, "on" ) )
		return_val = RL_TE_ON;
	else if ( !strcmp( val, "off" ) )
		return_val = RL_TE_OFF;
	
	return return_val;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : soar_rl_enabled
 **************************************************************************/
bool soar_rl_enabled( agent *my_agent )
{
	return ( my_agent->sysparams[ RL_ENABLED ] == RL_LEARNING_ON );
}

/***************************************************************************
 * Function     : add_rl_stat
 **************************************************************************/
rl_stat *add_rl_stat( const char *name )
{
	// new stat entry
	rl_stat *newbie = new rl_stat;
	newbie->name = name;
	newbie->value = 0;
	
	return newbie;
}

/***************************************************************************
 * Function     : convert_rl_stat
 **************************************************************************/
const long convert_rl_stat( agent *my_agent, const char *name )
{
	for ( int i=0; i<RL_STATS; i++ )
		if ( !strcmp( name, my_agent->rl_stats[ i ]->name ) )
			return i;

	return RL_STATS;
}

const char *convert_rl_stat( agent *my_agent, const long stat )
{
	if ( ( stat < 0 ) || ( stat >= RL_STATS ) )
		return NULL;

	return my_agent->rl_stats[ stat ]->name;
}

/***************************************************************************
 * Function     : valid_rl_stat
 **************************************************************************/
bool valid_rl_stat( agent *my_agent, const char *name )
{
	return ( convert_rl_stat( my_agent, name ) != RL_STATS );
}

bool valid_rl_stat( agent *my_agent, const long stat )
{
	return ( convert_rl_stat( my_agent, stat ) != NULL );
}

/***************************************************************************
 * Function     : get_rl_stat
 **************************************************************************/
double get_rl_stat( agent *my_agent, const char *name )
{
	const long stat = convert_rl_stat( my_agent, name );
	if ( stat == RL_STATS )
		return 0;

	return my_agent->rl_stats[ stat ]->value;
}

double get_rl_stat( agent *my_agent, const long stat )
{
	if ( !valid_rl_stat( my_agent, stat ) )
		return 0;

	return my_agent->rl_stats[ stat ]->value;
}

/***************************************************************************
 * Function     : set_rl_stat
 **************************************************************************/
bool set_rl_stat( agent *my_agent, const char *name, double new_val )
{
	const long stat = convert_rl_stat( my_agent, name );
	if ( stat == RL_STATS )
		return false;
	
	my_agent->rl_stats[ stat ]->value = new_val;
	
	return true;
}

bool set_rl_stat( agent *my_agent, const long stat, double new_val )
{
	if ( !valid_rl_stat( my_agent, stat ) )
		return false;
	
	my_agent->rl_stats[ stat ]->value = new_val;
	
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
 * Function     : get_template_id
 **************************************************************************/
int get_template_id( const char *prod_name )
{
	std::string temp = prod_name;
	
	// has to be at least "rl*a*#" (where a is a single letter/number/etc)
	if ( temp.length() < 6 )
		return -1;
	
	// check first three letters are "rl*"
	if ( temp.compare( 0, 3, "rl*" ) )
		return -1;
	
	// find last * to isolate id
	std::string::size_type last_star = temp.find_last_of( '*' );
	if ( last_star == std::string::npos )
		return -1;
	
	// make sure there's something left after last_star
	if ( last_star == ( temp.length() - 1 ) )
		return -1;
	
	// make sure id is a valid natural number
	std::string id_str = temp.substr( last_star + 1 );
	if ( !is_natural_number( &id_str ) )
		return -1;
	
	// convert id
	int id;
	from_string( id, id_str );
	return id;
}

/***************************************************************************
 * Function     : initialize_template_tracking
 **************************************************************************/
void initialize_template_tracking( agent *my_agent )
{
	my_agent->rl_template_count = 1;
}

/***************************************************************************
 * Function     : update_template_tracking
 **************************************************************************/
void update_template_tracking( agent *my_agent, const char *rule_name )
{
	int new_id = get_template_id( rule_name );

	if ( ( new_id != -1 ) && ( new_id > my_agent->rl_template_count ) )
		my_agent->rl_template_count = ( new_id + 1 );
}

/***************************************************************************
 * Function     : next_template_id
 **************************************************************************/
int next_template_id( agent *my_agent )
{
	return (my_agent->rl_template_count++);
}

/***************************************************************************
 * Function     : revert_template_id
 **************************************************************************/
void revert_template_id( agent *my_agent )
{
	my_agent->rl_template_count--;
}

/***************************************************************************
 * Function     : build_template_instantiation
 **************************************************************************/
 Symbol *build_template_instantiation( agent *my_agent, instantiation *my_template_instance, struct token_struct *tok, wme *w )
{
	Symbol *id, *attr, *value, *referent;
	production *my_template = my_template_instance->prod;
	action *my_action = my_template->action_list;
	char first_letter;
	double init_value = 0;

	Bool chunk_var = my_agent->variablize_this_chunk;
	condition *cond_top, *cond_bottom;

	// get the preference value
	id = instantiate_rhs_value( my_agent, my_action->id, -1, 's', tok, w );
	attr = instantiate_rhs_value( my_agent, my_action->attr, id->id.level, 'a', tok, w );
	first_letter = first_letter_from_symbol( attr );
	value = instantiate_rhs_value( my_agent, my_action->value, id->id.level, first_letter, tok, w );
	referent = instantiate_rhs_value( my_agent, my_action->referent, id->id.level, first_letter, tok, w );

	if ( referent->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE )
		init_value = (double) referent->ic.value;
	else if ( referent->common.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE )
		init_value = referent->fc.value;

	// clean up after yourself :)
	symbol_remove_ref( my_agent, id );
	symbol_remove_ref( my_agent, attr );
	symbol_remove_ref( my_agent, value );
	symbol_remove_ref( my_agent, referent );

	// make new action list
	// small hack on variablization: the artificial tc gets dealt with later, just needs to be explicit non-zero
	my_agent->variablize_this_chunk = TRUE;
	my_agent->variablization_tc = -1;
	action *new_action = make_simple_action( my_agent, id, attr, value, referent );
	new_action->preference_type = NUMERIC_INDIFFERENT_PREFERENCE_TYPE;

	// make unique production name
	Symbol *new_name_symbol;
	std::string new_name = "";
	std::string empty_string = "";
	std::string *temp_id;
	int new_id;
	do
	{
		new_id = next_template_id( my_agent );
		temp_id = to_string( new_id );
		new_name = ( "rl*" + empty_string + my_template->name->sc.name + "*" + (*temp_id) );
		delete temp_id;
	} while ( find_sym_constant( my_agent, new_name.c_str() ) != NIL );
	new_name_symbol = make_sym_constant( my_agent, (char *) new_name.c_str() );
	
	// prep conditions
	copy_condition_list( my_agent, my_template_instance->top_of_instantiated_conditions, &cond_top, &cond_bottom );
	add_goal_or_impasse_tests_to_conds( my_agent, cond_top );
	reset_variable_generator( my_agent, cond_top, NIL );
	my_agent->variablization_tc = get_new_tc_number( my_agent );
	variablize_condition_list( my_agent, cond_top );
	variablize_nots_and_insert_into_conditions( my_agent, my_template_instance->nots, cond_top );

	// make new production
	production *new_production = make_production( my_agent, USER_PRODUCTION_TYPE, new_name_symbol, &cond_top, &cond_bottom, &new_action, false );
	my_agent->variablize_this_chunk = chunk_var;

	// attempt to add to rete, remove if duplicate
	if ( add_production_to_rete( my_agent, new_production, cond_top, NULL, FALSE, TRUE ) == DUPLICATE_PRODUCTION )
	{
		excise_production( my_agent, new_production, false );
		revert_template_id( my_agent );

		new_name_symbol = NULL;
	}
	deallocate_condition_list( my_agent, cond_top );

	return new_name_symbol;
}

/***************************************************************************
 * Function     : make_simple_action
 **************************************************************************/
action *make_simple_action( agent *my_agent, Symbol *id_sym, Symbol *attr_sym, Symbol *val_sym, Symbol *ref_sym )
{
    action *rhs;
    Symbol *temp;

    allocate_with_pool( my_agent, &my_agent->action_pool, &rhs );
    rhs->next = NIL;
    rhs->type = MAKE_ACTION;

    // id
	temp = id_sym;
	symbol_add_ref( temp );
	variablize_symbol( my_agent, &temp );
	rhs->id = symbol_to_rhs_value( temp );

    // attribute
    temp = attr_sym;
	symbol_add_ref( temp );
	variablize_symbol( my_agent, &temp );
	rhs->attr = symbol_to_rhs_value( temp );

	// value
	temp = val_sym;
	symbol_add_ref( temp );
	variablize_symbol( my_agent, &temp );
	rhs->value = symbol_to_rhs_value( temp );

	// referent
	temp = ref_sym;
	symbol_add_ref( temp );
	variablize_symbol( my_agent, &temp );
	rhs->referent = symbol_to_rhs_value( temp );

    return rhs;
}

/***************************************************************************
 * Function     : add_goal_or_impasse_tests_to_conds
 **************************************************************************/
void add_goal_or_impasse_tests_to_conds( agent *my_agent, condition *all_conds )
{
	// mark each id as we add a test for it, so we don't add a test for the same id in two different places
	Symbol *id;
	test t;
	complex_test *ct;
	tc_number tc = get_new_tc_number( my_agent );

	for ( condition *cond = all_conds; cond != NIL; cond = cond->next )
	{
		if ( cond->type != POSITIVE_CONDITION )
			continue;

		id = referent_of_equality_test( cond->data.tests.id_test );

		if ( ( id->id.isa_goal || id->id.isa_impasse ) && ( id->id.tc_num != tc ) ) 
		{
			allocate_with_pool( my_agent, &my_agent->complex_test_pool, &ct );
			ct->type = (char) ( ( id->id.isa_goal )?( GOAL_ID_TEST ):( IMPASSE_ID_TEST ) );
			t = make_test_from_complex_test( ct );
			add_new_test_to_test( my_agent, &( cond->data.tests.id_test ), t );
			id->id.tc_num = tc;
		}
	}
}

/***************************************************************************
 * Function     : tabulate_reward_value_for_goal
 **************************************************************************/
void tabulate_reward_value_for_goal( agent *my_agent, Symbol *goal )
{
	rl_data *data = goal->id.rl_info;

	// Only count rewards at top state... 
	// or for op no-change impasses.
	if ( ( data->impasse_type != NONE_IMPASSE_TYPE ) && ( data->impasse_type != OP_NO_CHANGE_IMPASSE_TYPE ) )  
		return;
	
	slot *s = goal->id.reward_header->id.slots;
	slot *t;
	wme *w, *x;
	double reward = 0.0;
	unsigned int reward_count = 0;

	if ( s )
	{
		for ( ; s; s = s->next )
			for ( w = s->wmes ; w; w = w->next)
				if ( w->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE )
					for ( t = w->value->id.slots; t; t = t->next )
						for ( x = t->wmes; x; x = x->next )
							if ( ( x->value->common.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE ) || ( x->value->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE ) )
							{
								reward = reward + get_number_from_symbol( x->value );
								reward_count++;
							}
		data->reward += discount_reward( my_agent, reward, data->step );
	}

	// update stats
	double global_reward = get_rl_stat( my_agent, RL_STAT_GLOBAL_REWARD );
	set_rl_stat( my_agent, RL_STAT_TOTAL_REWARD, reward );
	set_rl_stat( my_agent, RL_STAT_GLOBAL_REWARD, ( global_reward + reward ) );

	data->step++;
}

/***************************************************************************
 * Function     : tabulate_reward_values
 **************************************************************************/
void tabulate_reward_values( agent *my_agent )
{
	Symbol *goal = my_agent->top_goal;

	while( goal )
	{
		tabulate_reward_value_for_goal( my_agent, goal );
	    goal = goal->id.lower_goal;
	}
}

/***************************************************************************
 * Function     : discount_reward
 **************************************************************************/
double discount_reward( agent *my_agent, double reward, unsigned int step )
{
	double rate = get_rl_parameter( my_agent, RL_PARAM_DISCOUNT_RATE );

	return ( reward * pow( rate, (double) step ) );
}

/***************************************************************************
 * Function     : store_rl_data
 **************************************************************************/
void store_rl_data( agent *my_agent, Symbol *goal, preference *cand )
{
	rl_data *data = goal->id.rl_info;
	Symbol *op = cand->value;
    data->previous_q = cand->numeric_value;

	bool using_gaps = ( get_rl_parameter( my_agent, RL_PARAM_TEMPORAL_EXTENSION, RL_RETURN_LONG ) == RL_TE_ON );
	
	// Make list of just-fired prods
	unsigned int just_fired = 0;
	for ( preference *pref = goal->id.operator_slot->preferences[ NUMERIC_INDIFFERENT_PREFERENCE_TYPE ]; pref; pref = pref->next )
		if ( ( op == pref->value ) && pref->inst->prod->rl_rule )
			if ( pref->inst->prod->rl_rule ) 
			{
				if ( !just_fired )
				{
					free_list( my_agent, data->prev_op_rl_rules );
					data->prev_op_rl_rules = NIL;
				}
				
				push( my_agent, pref->inst->prod, data->prev_op_rl_rules );
				just_fired++;
			}

	if ( just_fired )
	{
		if ( my_agent->sysparams[ TRACE_RL_SYSPARAM ] )
		{
			if ( data->reward_age != 0 )
			{
				char buf[256];
				SNPRINTF( buf, 254, "WARNING: gap ended (%c%d)", goal->id.name_letter, goal->id.name_number );
				
				print( my_agent, buf );
				
       			gSKI_MakeAgentCallbackXML( my_agent, kFunctionBeginTag, kTagWarning );
       			gSKI_MakeAgentCallbackXML( my_agent, kFunctionAddAttribute, kTypeString, buf );
       			gSKI_MakeAgentCallbackXML( my_agent, kFunctionEndTag, kTagWarning );
			}
		}
		
		data->reward_age = 0;
		data->num_prev_op_rl_rules = just_fired;
	}
	else
	{
		if ( my_agent->sysparams[ TRACE_RL_SYSPARAM ] )
		{
			if ( data->reward_age == 0 )
			{
				char buf[256];
				SNPRINTF( buf, 254, "WARNING: gap started (%c%d)", goal->id.name_letter, goal->id.name_number );
				
				print( my_agent, buf );
				
       			gSKI_MakeAgentCallbackXML( my_agent, kFunctionBeginTag, kTagWarning );
       			gSKI_MakeAgentCallbackXML( my_agent, kFunctionAddAttribute, kTypeString, buf );
       			gSKI_MakeAgentCallbackXML( my_agent, kFunctionEndTag, kTagWarning );
			}
		}
		
		if ( !using_gaps )
		{
			data->prev_op_rl_rules = NIL;
			data->num_prev_op_rl_rules = 0;
		}
		
		data->reward_age++;
	}
}

/***************************************************************************
 * Function     : perform_rl_update
 **************************************************************************/
void perform_rl_update( agent *my_agent, double op_value, Symbol *goal )
{
	rl_data *data = goal->id.rl_info;
	soar_rl_et_map::iterator iter;

	bool using_gaps = ( get_rl_parameter( my_agent, RL_PARAM_TEMPORAL_EXTENSION, RL_RETURN_LONG ) == RL_TE_ON );

	double alpha = get_rl_parameter( my_agent, RL_PARAM_LEARNING_RATE );
	double lambda = get_rl_parameter( my_agent, RL_PARAM_ET_DECAY_RATE );
	double gamma = get_rl_parameter( my_agent, RL_PARAM_DISCOUNT_RATE );
	double tolerance = get_rl_parameter( my_agent, RL_PARAM_ET_TOLERANCE );

	// compute TD update, set stat
	double update = data->reward;

	if ( using_gaps )
		update *= pow( gamma, (double) data->reward_age );

	update += ( pow( gamma, (double) data->step ) * op_value );
	update -= data->previous_q;
	set_rl_stat( my_agent, (const long) RL_STAT_UPDATE_ERROR, (double) ( -update ) );

	// Iterate through eligibility_traces, decay traces. If less than TOLERANCE, remove from map.
	if ( lambda == 0 )
	{
		if ( !data->eligibility_traces->empty() )
			data->eligibility_traces->clear();
	}
	else
	{
		for ( iter = data->eligibility_traces->begin(); iter != data->eligibility_traces->end(); )
		{
			iter->second *= lambda;
			iter->second *= pow( gamma, (double) data->step );
			if ( iter->second < tolerance ) 
				data->eligibility_traces->erase( iter++ );
			else 
				++iter;
		}
	}
	
	// Update trace for just fired prods
	if ( data->num_prev_op_rl_rules )
	{
		double trace_increment = ( 1.0 / data->num_prev_op_rl_rules );
		
		for ( cons *c = data->prev_op_rl_rules; c; c = c->rest )
		{
			if ( c->first )
			{
				iter = data->eligibility_traces->find( (production *) c->first );
				if ( iter != data->eligibility_traces->end() ) 
					iter->second += trace_increment;
				else 
					(*data->eligibility_traces)[ (production *) c->first ] = trace_increment;
			}
		}
	}
	
	// For each prod in map, add alpha*delta*trace to value
	for ( iter = data->eligibility_traces->begin(); iter != data->eligibility_traces->end(); iter++ )
	{	
		production *prod = iter->first;
		double temp = get_number_from_symbol( rhs_value_to_symbol( prod->action_list->referent ) );

		// update is applied depending upon type of accumulation mode
		// sum: add the update to the existing value
		// avg: average the update with the existing value
		temp += ( update * alpha * iter->second );		

		// Change value of rule
		symbol_remove_ref( my_agent, rhs_value_to_symbol( prod->action_list->referent ) );
		prod->action_list->referent = symbol_to_rhs_value( make_float_constant( my_agent, temp ) );
		prod->rl_update_count += 1;

		// Change value of preferences generated by current instantiations of this rule
		if ( prod->instantiations )
		{
			for ( instantiation *inst = prod->instantiations; inst; inst = inst->next )
			{
				for ( preference *pref = inst->preferences_generated; pref; pref = pref->inst_next )
				{
					symbol_remove_ref( my_agent, pref->referent );
					pref->referent = make_float_constant( my_agent, temp );
				}
			}
		}	
	}

	data->reward = 0.0;
	data->step = 0;
	data->impasse_type = NONE_IMPASSE_TYPE;
}

/***************************************************************************
 * Function     : watkins_clear
 **************************************************************************/
void watkins_clear( agent *my_agent, Symbol *goal )
{
	rl_data *data = goal->id.rl_info;
	soar_rl_et_map::iterator iter;
	
	// Iterate through eligibility_traces, remove traces
	for ( iter = data->eligibility_traces->begin(); iter != data->eligibility_traces->end(); )
		data->eligibility_traces->erase( iter++ );
}
