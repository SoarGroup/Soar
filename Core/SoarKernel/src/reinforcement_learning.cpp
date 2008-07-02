/* vim:set noexpandtab: */
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
#define DBG if(0)

#include "reinforcement_learning.h"

#include <stdlib.h>
#include <math.h>
#include <set>
#include <vector>

using std::multiset;

#include "agent.h"
#include "production.h"
#include "rhsfun.h"
#include "instantiations.h"
#include "rete.h"
#include "wmem.h"

#include "print.h"
#include "xml.h"

#include "misc.h"
#include "decide.h"

#define ZETA2 1.6449340668 // sum of the harmonic series 1/k^2

extern Symbol *instantiate_rhs_value (agent* thisAgent, rhs_value rv, goal_stack_level new_id_level, char new_id_letter, struct token_struct *tok, wme *w);
extern void variablize_symbol (agent* thisAgent, Symbol **sym);
extern void variablize_nots_and_insert_into_conditions (agent* thisAgent, not_struct *nots, condition *conds);
extern void variablize_condition_list (agent* thisAgent, condition *cond);


/***************************************************************************
 * Function     : rl_clean_parameters
 **************************************************************************/
void rl_clean_parameters( agent *my_agent )
{
	for ( int i=0; i<RL_PARAMS; i++ )
	{
		delete my_agent->rl_params[ i ]->param;
		delete my_agent->rl_params[ i ];
	}
}

/***************************************************************************
 * Function     : rl_clean_stats
 **************************************************************************/
void rl_clean_stats( agent *my_agent )
{
	for ( int i=0; i<RL_STATS; i++ )
	  delete my_agent->rl_stats[ i ];
}

/***************************************************************************
 * Function     : rl_reset_data
 **************************************************************************/
void rl_reset_data( agent *my_agent )
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
void rl_reset_stats( agent *my_agent )
{
	for ( int i=0; i<RL_STATS; i++ )
		my_agent->rl_stats[ i ]->value = 0;
}

/***************************************************************************
 * Function     : rl_remove_refs_for_prod
 **************************************************************************/
void rl_remove_refs_for_prod( agent *my_agent, production *prod )
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
 * Function     : rl_add_parameter
 **************************************************************************/
rl_parameter *rl_add_parameter( const char *name, double value, bool (*val_func)( double ) )
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

rl_parameter *rl_add_parameter( const char *name, const long value, bool (*val_func)( const long ), const char *(*to_str)( long ), const long (*from_str)( const char * ) )
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
 * Function     : rl_convert_parameter
 **************************************************************************/
const char *rl_convert_parameter( agent *my_agent, const long param )
{
	if ( ( param < 0 ) || ( param >= RL_PARAMS ) )
		return NULL;

	return my_agent->rl_params[ param ]->name;
}

const long rl_convert_parameter( agent *my_agent, const char *name )
{
	for ( int i=0; i<RL_PARAMS; i++ )
		if ( !strcmp( name, my_agent->rl_params[ i ]->name ) )
			return i;

	return RL_PARAMS;
}

/***************************************************************************
 * Function     : rl_valid_parameter
 **************************************************************************/
bool rl_valid_parameter( agent *my_agent, const char *name )
{
	return ( rl_convert_parameter( my_agent, name ) != RL_PARAMS );
}

bool rl_valid_parameter( agent *my_agent, const long param )
{
	return ( rl_convert_parameter( my_agent, param ) != NULL );
}

/***************************************************************************
 * Function     : rl_get_parameter_type
 **************************************************************************/
rl_param_type rl_get_parameter_type( agent *my_agent, const char *name )
{
	const long param = rl_convert_parameter( my_agent, name );
	if ( param == RL_PARAMS )
		return rl_param_invalid;
	
	return my_agent->rl_params[ param ]->type;
}

rl_param_type rl_get_parameter_type( agent *my_agent, const long param )
{
	if ( !rl_valid_parameter( my_agent, param ) )
		return rl_param_invalid;

	return my_agent->rl_params[ param ]->type;
}

/***************************************************************************
 * Function     : rl_get_parameter
 **************************************************************************/
const long rl_get_parameter( agent *my_agent, const char *name, const double /*test*/ )
{
	const long param = rl_convert_parameter( my_agent, name );
	if ( param == RL_PARAMS )
		return NULL;
	
	if ( rl_get_parameter_type( my_agent, param ) != rl_param_string )
		return NULL;
	
	return my_agent->rl_params[ param ]->param->string_param.value;
}

const char *rl_get_parameter( agent *my_agent, const char *name, const char * /*test*/ )
{
	const long param = rl_convert_parameter( my_agent, name );
	if ( param == RL_PARAMS )
		return NULL;
	
	if ( rl_get_parameter_type( my_agent, param ) != rl_param_string )
		return NULL;
	
	return my_agent->rl_params[ param ]->param->string_param.to_str( my_agent->rl_params[ param ]->param->string_param.value );
}

double rl_get_parameter( agent *my_agent, const char *name )
{
	const long param = rl_convert_parameter( my_agent, name );
	if ( param == RL_PARAMS )
		return NULL;
	
	if ( rl_get_parameter_type( my_agent, param ) != rl_param_number )
		return NULL;
	
	return my_agent->rl_params[ param ]->param->number_param.value;
}

//

const long rl_get_parameter( agent *my_agent, const long param, const double /*test*/ )
{
	if ( !rl_valid_parameter( my_agent, param ) )
		return NULL;

	if ( rl_get_parameter_type( my_agent, param ) != rl_param_string )
		return NULL;
	
	return my_agent->rl_params[ param ]->param->string_param.value;
}

const char *rl_get_parameter( agent *my_agent, const long param, const char * /*test*/ )
{
	if ( !rl_valid_parameter( my_agent, param ) )
		return NULL;
	
	if ( rl_get_parameter_type( my_agent, param ) != rl_param_string )
		return NULL;
	
	return my_agent->rl_params[ param ]->param->string_param.to_str( my_agent->rl_params[ param ]->param->string_param.value );
}

double rl_get_parameter( agent *my_agent, const long param )
{
	if ( !rl_valid_parameter( my_agent, param ) )
		return NULL;
	
	if ( rl_get_parameter_type( my_agent, param ) != rl_param_number )
		return NULL;
	
	return my_agent->rl_params[ param ]->param->number_param.value;
}

/***************************************************************************
 * Function     : rl_valid_parameter_value
 **************************************************************************/
bool rl_valid_parameter_value( agent *my_agent, const char *name, double new_val )
{
	const long param = rl_convert_parameter( my_agent, name );
	if ( param == RL_PARAMS )
		return false;
	
	if ( rl_get_parameter_type( my_agent, param ) != rl_param_number )
		return false;
	
	return my_agent->rl_params[ param ]->param->number_param.val_func( new_val );
}

bool rl_valid_parameter_value( agent *my_agent, const char *name, const char *new_val )
{
	const long param = rl_convert_parameter( my_agent, name );
	if ( param == RL_PARAMS )
		return false;
	
	if ( rl_get_parameter_type( my_agent, param ) != rl_param_string )
		return false;
	
	return my_agent->rl_params[ param ]->param->string_param.val_func( my_agent->rl_params[ param ]->param->string_param.from_str( new_val ) );
}

bool rl_valid_parameter_value( agent *my_agent, const char *name, const long new_val )
{
	const long param = rl_convert_parameter( my_agent, name );
	if ( param == RL_PARAMS )
		return false;
	
	if ( rl_get_parameter_type( my_agent, param ) != rl_param_string )
		return false;
	
	return my_agent->rl_params[ param ]->param->string_param.val_func( new_val );
}

//

bool rl_valid_parameter_value( agent *my_agent, const long param, double new_val )
{
	if ( !rl_valid_parameter( my_agent, param ) )
		return false;
	
	if ( rl_get_parameter_type( my_agent, param ) != rl_param_number )
		return false;
	
	return my_agent->rl_params[ param ]->param->number_param.val_func( new_val );
}

bool rl_valid_parameter_value( agent *my_agent, const long param, const char *new_val )
{
	if ( !rl_valid_parameter( my_agent, param ) )
		return false;
	
	if ( rl_get_parameter_type( my_agent, param ) != rl_param_string )
		return false;
	
	return my_agent->rl_params[ param ]->param->string_param.val_func( my_agent->rl_params[ param ]->param->string_param.from_str( new_val ) );
}

bool rl_valid_parameter_value( agent *my_agent, const long param, const long new_val )
{
	if ( !rl_valid_parameter( my_agent, param ) )
		return false;
	
	if ( rl_get_parameter_type( my_agent, param ) != rl_param_string )
		return false;
	
	return my_agent->rl_params[ param ]->param->string_param.val_func( new_val );
}

/***************************************************************************
 * Function     : rl_set_parameter
 **************************************************************************/
bool rl_set_parameter( agent *my_agent, const char *name, double new_val )
{
	const long param = rl_convert_parameter( my_agent, name );
	if ( param == RL_PARAMS )
		return false;
	
	if ( !rl_valid_parameter_value( my_agent, param, new_val ) )
		return false;
	
	my_agent->rl_params[ param ]->param->number_param.value = new_val;

	return true;
}

bool rl_set_parameter( agent *my_agent, const char *name, const char *new_val )
{
	const long param = rl_convert_parameter( my_agent, name );
	if ( param == RL_PARAMS )
		return false;
	
	if ( !rl_valid_parameter_value( my_agent, param, new_val ) )
		return false;

	const long converted_val = my_agent->rl_params[ param ]->param->string_param.from_str( new_val );

	// learning special case
	if ( param == RL_PARAM_LEARNING )
		set_sysparam( my_agent, RL_ENABLED, converted_val );
	
	my_agent->rl_params[ param ]->param->string_param.value = converted_val;

	return true;
}

bool rl_set_parameter( agent *my_agent, const char *name, const long new_val )
{
	const long param = rl_convert_parameter( my_agent, name );
	if ( param == RL_PARAMS )
		return false;
	
	if ( !rl_valid_parameter_value( my_agent, param, new_val ) )
		return false;

	// learning special case
	if ( param == RL_PARAM_LEARNING )
		set_sysparam( my_agent, RL_ENABLED, new_val );
	
	my_agent->rl_params[ param ]->param->string_param.value = new_val;

	return true;
}

//

bool rl_set_parameter( agent *my_agent, const long param, double new_val )
{
	if ( !rl_valid_parameter_value( my_agent, param, new_val ) )
		return false;
	
	my_agent->rl_params[ param ]->param->number_param.value = new_val;

	return true;
}

bool rl_set_parameter( agent *my_agent, const long param, const char *new_val )
{
	if ( !rl_valid_parameter_value( my_agent, param, new_val ) )
		return false;

	const long converted_val = my_agent->rl_params[ param ]->param->string_param.from_str( new_val );

	// learning special case
	if ( param == RL_PARAM_LEARNING )
		set_sysparam( my_agent, RL_ENABLED, converted_val );
	
	my_agent->rl_params[ param ]->param->string_param.value = converted_val;

	return true;
}

bool rl_set_parameter( agent *my_agent, const long param, const long new_val )
{	
	if ( !rl_valid_parameter_value( my_agent, param, new_val ) )
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
 * Function     : rl_validate_learning
 **************************************************************************/
bool rl_validate_learning( const long new_val )
{
	return ( ( new_val == RL_LEARNING_ON ) || ( new_val == RL_LEARNING_OFF ) );
}

/***************************************************************************
 * Function     : rl_convert_learning
 **************************************************************************/
const char *rl_convert_learning( const long val )
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

const long rl_convert_learning( const char *val )
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
 * Function     : rl_validate_discount
 **************************************************************************/
bool rl_validate_discount( const double new_val )
{
	return ( ( new_val >= 0 ) && ( new_val <= 1 ) );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// learning rate
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : rl_validate_learning_rate
 **************************************************************************/
bool rl_validate_learning_rate( const double new_val )
{
	return ( ( new_val >= 0 ) && ( new_val <= 1 ) );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// learning policy
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : rl_validate_learning_policy
 **************************************************************************/
bool rl_validate_learning_policy( const long new_val )
{
	return ( ( new_val == RL_LEARNING_SARSA ) || ( new_val == RL_LEARNING_Q ) );
}


bool validate_nonnegative(const double new_val) { return new_val >= 0.0; }
bool validate_probability(const double new_val) { return 0.0 <= new_val && new_val <= 1.0; }

/***************************************************************************
 * Function     : rl_convert_learning_policy
 **************************************************************************/
const char *rl_convert_learning_policy( const long val )
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

const long rl_convert_learning_policy( const char *val )
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
 * Function     : rl_validate_decay_rate
 **************************************************************************/
bool rl_validate_decay_rate( const double new_val )
{
	return ( ( new_val >= 0 ) && ( new_val <= 1 ) );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// eligibility trace tolerance
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : rl_validate_trace_tolerance
 **************************************************************************/
bool rl_validate_trace_tolerance( const double new_val )
{
	return ( new_val > 0 );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// temporal-extension
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : rl_validate_te_enabled
 **************************************************************************/
bool rl_validate_te_enabled( const long new_val )
{
	return ( ( new_val == RL_TE_ON ) || ( new_val == RL_TE_OFF ) );
}

/***************************************************************************
 * Function     : rl_convert_te_enabled
 **************************************************************************/
const char *rl_convert_te_enabled( const long val )
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

const long rl_convert_te_enabled( const char *val )
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
 * Function     : rl_enabled
 **************************************************************************/
bool rl_enabled( agent *my_agent )
{
	return ( my_agent->sysparams[ RL_ENABLED ] == RL_LEARNING_ON );
}

/***************************************************************************
 * Function     : rl_add_stat
 **************************************************************************/
rl_stat *rl_add_stat( const char *name )
{
	// new stat entry
	rl_stat *newbie = new rl_stat;
	newbie->name = name;
	newbie->value = 0;
	
	return newbie;
}

/***************************************************************************
 * Function     : rl_convert_stat
 **************************************************************************/
const long rl_convert_stat( agent *my_agent, const char *name )
{
	for ( int i=0; i<RL_STATS; i++ )
		if ( !strcmp( name, my_agent->rl_stats[ i ]->name ) )
			return i;

	return RL_STATS;
}

const char *rl_convert_stat( agent *my_agent, const long stat )
{
	if ( ( stat < 0 ) || ( stat >= RL_STATS ) )
		return NULL;

	return my_agent->rl_stats[ stat ]->name;
}

/***************************************************************************
 * Function     : rl_valid_stat
 **************************************************************************/
bool rl_valid_stat( agent *my_agent, const char *name )
{
	return ( rl_convert_stat( my_agent, name ) != RL_STATS );
}

bool rl_valid_stat( agent *my_agent, const long stat )
{
	return ( rl_convert_stat( my_agent, stat ) != NULL );
}

/***************************************************************************
 * Function     : rl_get_stat
 **************************************************************************/
double rl_get_stat( agent *my_agent, const char *name )
{
	const long stat = rl_convert_stat( my_agent, name );
	if ( stat == RL_STATS )
		return 0;

	return my_agent->rl_stats[ stat ]->value;
}

double rl_get_stat( agent *my_agent, const long stat )
{
	if ( !rl_valid_stat( my_agent, stat ) )
		return 0;

	return my_agent->rl_stats[ stat ]->value;
}

/***************************************************************************
 * Function     : rl_set_stat
 **************************************************************************/
bool rl_set_stat( agent *my_agent, const char *name, double new_val )
{
	const long stat = rl_convert_stat( my_agent, name );
	if ( stat == RL_STATS )
		return false;
	
	my_agent->rl_stats[ stat ]->value = new_val;
	
	return true;
}

bool rl_set_stat( agent *my_agent, const long stat, double new_val )
{
	if ( !rl_valid_stat( my_agent, stat ) )
		return false;
	
	my_agent->rl_stats[ stat ]->value = new_val;
	
	return true;
}

/***************************************************************************
 * Function     : rl_valid_template
 **************************************************************************/
bool rl_valid_template( production *prod )
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
 * Function     : rl_valid_rule
 **************************************************************************/
bool rl_valid_rule( production *prod )
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
 * Function     : rl_get_template_id
 **************************************************************************/
int rl_get_template_id( const char *prod_name )
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
 * Function     : rl_initialize_template_tracking
 **************************************************************************/
void rl_initialize_template_tracking( agent *my_agent )
{
	my_agent->rl_template_count = 1;
}

/***************************************************************************
 * Function     : rl_update_template_tracking
 **************************************************************************/
void rl_update_template_tracking( agent *my_agent, const char *rule_name )
{
	int new_id = rl_get_template_id( rule_name );

	if ( ( new_id != -1 ) && ( new_id > my_agent->rl_template_count ) )
		my_agent->rl_template_count = ( new_id + 1 );
}

/***************************************************************************
 * Function     : rl_next_template_id
 **************************************************************************/
int rl_next_template_id( agent *my_agent )
{
	return (my_agent->rl_template_count++);
}

/***************************************************************************
 * Function     : rl_revert_template_id
 **************************************************************************/
void rl_revert_template_id( agent *my_agent )
{
	my_agent->rl_template_count--;
}

/***************************************************************************
 * Function     : rl_build_template_instantiation
 **************************************************************************/
 Symbol *rl_build_template_instantiation( agent *my_agent, instantiation *my_template_instance, struct token_struct *tok, wme *w )
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
	action *new_action = rl_make_simple_action( my_agent, id, attr, value, referent );
	new_action->preference_type = NUMERIC_INDIFFERENT_PREFERENCE_TYPE;

	// make unique production name
	Symbol *new_name_symbol;
	std::string new_name = "";
	std::string empty_string = "";
	std::string *temp_id;
	int new_id;
	do
	{
		new_id = rl_next_template_id( my_agent );
		temp_id = to_string( new_id );
		new_name = ( "rl*" + empty_string + my_template->name->sc.name + "*" + (*temp_id) );
		delete temp_id;
	} while ( find_sym_constant( my_agent, new_name.c_str() ) != NIL );
	new_name_symbol = make_sym_constant( my_agent, (char *) new_name.c_str() );
	
	// prep conditions
	copy_condition_list( my_agent, my_template_instance->top_of_instantiated_conditions, &cond_top, &cond_bottom );
	rl_add_goal_or_impasse_tests_to_conds( my_agent, cond_top );
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
		rl_revert_template_id( my_agent );

		new_name_symbol = NULL;
	}
	deallocate_condition_list( my_agent, cond_top );

	/*
	if (my_agent->rl_qconf->find(new_production) == my_agent->rl_qconf->end()) {
		print(my_agent, "create %s\n", new_name.c_str());
		initialize_q_bounds(my_agent, new_production);
	}
	*/

	return new_name_symbol;
}

/***************************************************************************
 * Function     : rl_make_simple_action
 **************************************************************************/
action *rl_make_simple_action( agent *my_agent, Symbol *id_sym, Symbol *attr_sym, Symbol *val_sym, Symbol *ref_sym )
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
 * Function     : rl_add_goal_or_impasse_tests_to_conds
 **************************************************************************/
void rl_add_goal_or_impasse_tests_to_conds( agent *my_agent, condition *all_conds )
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
 * Function     : rl_tabulate_reward_value_for_goal
 **************************************************************************/
void rl_tabulate_reward_value_for_goal( agent *my_agent, Symbol *goal )
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
		data->reward += rl_discount_reward( my_agent, reward, data->step );
	}

	// update stats
	double global_reward = rl_get_stat( my_agent, RL_STAT_GLOBAL_REWARD );
	rl_set_stat( my_agent, RL_STAT_TOTAL_REWARD, reward );
	rl_set_stat( my_agent, RL_STAT_GLOBAL_REWARD, ( global_reward + reward ) );

	data->step++;
}

/***************************************************************************
 * Function     : rl_tabulate_reward_values
 **************************************************************************/
void rl_tabulate_reward_values( agent *my_agent )
{
	Symbol *goal = my_agent->top_goal;

	while( goal )
	{
		rl_tabulate_reward_value_for_goal( my_agent, goal );
	    goal = goal->id.lower_goal;
	}
}

/***************************************************************************
 * Function     : rl_discount_reward
 **************************************************************************/
double rl_discount_reward( agent *my_agent, double reward, unsigned int step )
{
	double rate = rl_get_parameter( my_agent, RL_PARAM_DISCOUNT_RATE );

	return ( reward * pow( rate, (double) step ) );
}

/***************************************************************************
 * Function     : rl_store_data
 **************************************************************************/
void rl_store_data( agent *my_agent, Symbol *goal, preference *cand )
{
	rl_data *data = goal->id.rl_info;
	Symbol *op = cand->value;
    data->previous_q = cand->numeric_value;

	bool using_gaps = ( rl_get_parameter( my_agent, RL_PARAM_TEMPORAL_EXTENSION, RL_RETURN_LONG ) == RL_TE_ON );
	
	// Make list of just-fired prods
	unsigned int just_fired = 0;
	for ( preference *pref = goal->id.operator_slot->preferences[ NUMERIC_INDIFFERENT_PREFERENCE_TYPE ]; pref; pref = pref->next )
		if ( ( op == pref->value ) && pref->inst->prod->rl_rule )
			// if ( pref->inst->prod->rl_rule ) jzxu 03/17/08: there seems to be no point to this second test, so I'm taking it out
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
				xml_generate_warning( my_agent, buf );
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
				xml_generate_warning( my_agent, buf );
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

void initialize_qconf(agent* my_agent, production* p) {
#if Q_CONFIDENCE_METHOD == HOEFFDING_BOUNDING
	// set initial upper and lower bounds for the q values
	double Vmax = rl_get_parameter(my_agent, RL_PARAM_V_MAX);
	if (Vmax == 0.0) {
		Vmax = rl_get_parameter(my_agent, RL_PARAM_R_MAX) / (1.0 - rl_get_parameter(my_agent, RL_PARAM_DISCOUNT_RATE));
	}
	double conf = rl_get_parameter(my_agent, RL_PARAM_OOB_PROB);

	/* I'm not sure what this should be yet, I'm just grabbing it out of even-dar
	 * as a place holder. jzxu 03/07/2008 */
	double qmax_0 = Vmax * log(ZETA2 * rl_get_parameter(my_agent, RL_PARAM_SA_SPACE_SIZE) / conf);
	(*my_agent->rl_qconf)[p].q_min = -qmax_0;
	(*my_agent->rl_qconf)[p].q_max = qmax_0;
	(*my_agent->rl_qconf)[p].num_updates = 0;

	//print(my_agent, "\n============ BOUNDS CREATE ============\n");
	//print_production(my_agent, p, 0);
	DBG print(my_agent, "\nbound_create %s Qmin: %f Qmax: %f N: %d\n", p->name->sc.name, -qmax_0, qmax_0, 0);
	//DBG_PRINT(my_agent, "!!!!!!!!!!!! BOUNDS CREATE !!!!!!!!!!!!\n");
#elif Q_CONFIDENCE_METHOD == INTERVAL_ESTIMATION
  // as a sentinel for not being able to determine bounds yet, set q_min >
  // q_max
  (*my_agent->rl_qconf)[p].q_min = 1;
  (*my_agent->rl_qconf)[p].q_max = 0;
#endif
}

/***************************************************************************
 * Function     : rl_perform_update
 *
 * Vminb and Vmaxb are used to update bound info
 *
 **************************************************************************/
void rl_perform_update( agent *my_agent, double op_value, double Vminb, double Vmaxb, Symbol *goal )
{
	rl_data *data = goal->id.rl_info;
	rl_et_map::iterator iter;

	bool using_gaps = ( rl_get_parameter( my_agent, RL_PARAM_TEMPORAL_EXTENSION, RL_RETURN_LONG ) == RL_TE_ON );

	double alpha = rl_get_parameter( my_agent, RL_PARAM_LEARNING_RATE );
	double lambda = rl_get_parameter( my_agent, RL_PARAM_ET_DECAY_RATE );
	double gamma = rl_get_parameter( my_agent, RL_PARAM_DISCOUNT_RATE );
	double tolerance = rl_get_parameter( my_agent, RL_PARAM_ET_TOLERANCE );

	// compute TD update, set stat
	double update = data->reward;

	if ( using_gaps )
		update *= pow( gamma, (double) data->reward_age );

	update += ( pow( gamma, (double) data->step ) * op_value );
	update -= data->previous_q;
	rl_set_stat( my_agent, (const long) RL_STAT_UPDATE_ERROR, (double) ( -update ) );

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

#if Q_CONFIDENCE_METHOD == HOEFFDING_BOUNDING
		/* we can only update q bounds for TD(0) for now */
		if (iter->second == 1.0) {
			if (my_agent->rl_qconf->find(prod) == my_agent->rl_qconf->end()) {
				initialize_qconf(my_agent, prod);
			}
			rl_qconf_data &bounds = (*my_agent->rl_qconf)[prod];
			int t = ++bounds.num_updates;

			double confidence = rl_get_parameter(my_agent, RL_PARAM_OOB_PROB);
			double kSA1 = t * t * rl_get_parameter( my_agent, RL_PARAM_SA_SPACE_SIZE );
			double kSA2 = (t-1) * (t-1) * rl_get_parameter( my_agent, RL_PARAM_SA_SPACE_SIZE );
			/* alternative calculations include
			 *
			 * double kSA = number of total TD updates ^ 2
			 * double kSA = (number of total TD updates / lowerbound on SA space size) ^ 2
			 *
			 * As long as sum of all 1 / (c * kSA) <= 1
			 */

			double log_term1 = log(ZETA2 * kSA1 / confidence);
			double log_term2 = log(ZETA2 * kSA2 / confidence);
			// pi was introduced just to make the equation simpler
			double pi = 1.0 - alpha;
			double Vmax = rl_get_parameter(my_agent, RL_PARAM_V_MAX);
			if (Vmax == 0.0) {
				Vmax = rl_get_parameter(my_agent, RL_PARAM_R_MAX) / (1.0 - gamma);
			}

			double sqrt_term1 =               sqrt(0.5 * ((1 - pow(pi, 2 *    t   )) / (1 - pi * pi)) * log_term1);
			double sqrt_term2 = (t == 1 ? 0 : sqrt(0.5 * ((1 - pow(pi, 2 * (t - 1))) / (1 - pi * pi)) * log_term2));

			double beta = Vmax * (sqrt_term1 - pi * sqrt_term2);


			bounds.q_min = (1-alpha) * bounds.q_min + alpha * (data->reward + gamma * Vminb - beta);
			bounds.q_max = (1-alpha) * bounds.q_max + alpha * (data->reward + gamma * Vmaxb + beta);
			//DBG print(my_agent, "update %s\n", prod->name);
			//DBG print(my_agent, "\n============ BOUNDS UPDATE ============\n");
			//DBG print_production(my_agent, prod, 0);
			//DBG print(my_agent, "sqrt_term1: %f sqrt_term2: %f\n", sqrt_term1, sqrt_term2);
			DBG print(my_agent, "\nbound_update %s Q: %f Qmin: %f Qmax: %f N: %d\n", prod->name->sc.name, temp, bounds.q_min, bounds.q_max, bounds.num_updates);
			//DBG print(my_agent, "!!!!!!!!!!!! BOUNDS UPDATE !!!!!!!!!!!!\n");
		}
#elif Q_CONFIDENCE_METHOD == INTERVAL_ESTIMATION
    // maybe int. est. can easily handle eligibility traces?
    if (iter->second == 1.0) {
			if (my_agent->rl_qconf->find(prod) == my_agent->rl_qconf->end()) {
				initialize_qconf(my_agent, prod);
			}
      int window_size = rl_get_parameter(my_agent, RL_PARAM_IE_WINSIZE);
      int r = rl_get_parameter(my_agent, RL_PARAM_IE_LOWER_INDEX);
      int s = rl_get_parameter(my_agent, RL_PARAM_IE_UPPER_INDEX);
      
      rl_qconf_data &conf_data = (*my_agent->rl_qconf)[prod];
      conf_data.win_by_val.insert(temp);
      conf_data.win_by_time.push_back(temp);
      if (conf_data.win_by_time.size() > window_size) {
        // keep a constant window size
        double stale = conf_data.win_by_time.front();
        // because multiset will erase all elements of the same value using
        // erase(val), I have to use find to get the position of one element
        // and erase that position
        multiset<double>::iterator stale_pos = conf_data.win_by_val.find(stale);
        conf_data.win_by_val.erase(stale_pos);
        conf_data.win_by_time.pop_front();
        
        // set the min and max to be the values with predetermined indexes
        // we only do this when the window has filled up
        std::multiset<double>::iterator i;
        int c;
        for(i = conf_data.win_by_val.begin(), c = 0;
            i != conf_data.win_by_val.end(); ++i, ++c)
        {
          if (c == r) {
            conf_data.q_min = *i;
          }
          else if (c == s) {
            conf_data.q_max = *i;
            break;
          }
        }
      }
      DBG print(my_agent, "\nbound_update %s Q: %f Qmin: %f Qmax: %f N: %d\n", prod->name->sc.name, temp, conf_data.q_min, conf_data.q_max, conf_data.win_by_val.size());
    }
#endif
	}

	data->reward = 0.0;
	data->step = 0;
	data->impasse_type = NONE_IMPASSE_TYPE;
}

/***************************************************************************
 * Function     : rl_watkins_clear
 **************************************************************************/
void rl_watkins_clear( agent * /*my_agent*/, Symbol *goal )
{
	rl_data *data = goal->id.rl_info;
	rl_et_map::iterator iter;
	
	// Iterate through eligibility_traces, remove traces
	for ( iter = data->eligibility_traces->begin(); iter != data->eligibility_traces->end(); )
		data->eligibility_traces->erase( iter++ );
}


/* don't need this yet
void print_q_bounds(agent* my_agent) {
	rl_qconf_map::iterator i;
	for (i = my_agent->rl_qconf.begin(); i != my_agent->rl_qconf.end(); ++i) {
		print(my_agent, "%s %d %f %f\n", i->first->name->sc.name, i->second.num_updates, i->second.q_min, i->second.q_max);
	}
}
*/

