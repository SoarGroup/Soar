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
	std::vector<std::string> *my_keys = map_keys( my_agent->rl_params );
	rl_parameter *temp;

	for ( int i=0; i<my_keys->size(); i++ )
	{
		temp = &( (*my_agent->rl_params)[ (*my_keys)[ i ] ] );
		delete( temp->param );
	}

	my_keys->clear();
	delete my_keys;
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

		data->previous_q = 0;
		data->reward = 0;
		data->step = 0;
		data->impasse_type = NONE_IMPASSE_TYPE;
		
		goal = goal->id.lower_goal;
	}
}

/***************************************************************************
 * Function     : reset_rl
 **************************************************************************/
void reset_rl_stats( agent *my_agent )
{
	std::vector<std::string> *my_keys = map_keys( my_agent->rl_stats );
	
	for ( size_t i=0; i<my_keys->size(); i++ )
		(*my_agent->rl_stats)[ (*my_keys)[i] ] = 0;

	my_keys->clear();
	delete my_keys;
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
	std::string *temp = new std::string( name );
	bool return_val = is_set( my_agent->rl_params, temp );
	delete temp;

	return return_val;
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
	return ( new_val >= 0 );
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
// eligibility trace discount rate
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/***************************************************************************
 * Function     : validate_rl_trace_discount
 **************************************************************************/
bool validate_rl_trace_discount( double new_val )
{
	return ( ( new_val >= 0 ) && ( new_val <= 1 ) );
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
 * Function     : soar_rl_enabled
 **************************************************************************/
bool soar_rl_enabled( agent *my_agent )
{
	return ( get_rl_parameter( my_agent, "learning", RL_RETURN_LONG ) == RL_LEARNING_ON );
}

/***************************************************************************
 * Function     : valid_rl_stat
 **************************************************************************/
bool valid_rl_stat( agent *my_agent, const char *name )
{
	bool return_val;
	std::string *temp = new std::string( name );

	return_val = is_set( my_agent->rl_stats, temp );

	delete temp;
	return return_val;
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
	template_instantiation *return_val = new template_instantiation;
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

	my_keys->clear();
	delete my_keys;
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

	delete origin;
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

				delete temp_origin;
			}
		}
		
		(*my_agent->rl_template_count)[ origin->template_base ] = temp_id;
	}

	delete origin;
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
	
	delete temp;
	return return_val;
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
	float init_value = 0;

	Bool chunk_var = my_agent->variablize_this_chunk;
	condition *cond_top, *cond_bottom;

	// get the preference value
	id = instantiate_rhs_value( my_agent, my_action->id, -1, 's', tok, w );
	attr = instantiate_rhs_value( my_agent, my_action->attr, id->id.level, 'a', tok, w );
	first_letter = first_letter_from_symbol( attr );
	value = instantiate_rhs_value( my_agent, my_action->value, id->id.level, first_letter, tok, w );
	referent = instantiate_rhs_value( my_agent, my_action->referent, id->id.level, first_letter, tok, w );

	if ( referent->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE )
		init_value = (float) referent->ic.value;
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
	std::string *temp_id;
	int new_id;
	do
	{
		new_id = next_template_id( my_agent, my_template->name->sc.name );
		temp_id = to_string( new_id );
		new_name = ( "rl*" + (*temp_id) + "*" + my_template->name->sc.name );
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
		revert_template_tracking( my_agent, new_name.c_str() );

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
	float reward = 0.0;
	unsigned int reward_count = 0;

	if ( s )
	{
		for ( ; s; s = s->next )
			for ( wme *w = s->wmes ; w; w = w->next)
				if ( ( w->value->common.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE ) || ( w->value->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE ) )
				{
					reward = reward + get_number_from_symbol( w->value );
					reward_count++;
				}
		
		if ( reward_count && ( get_rl_parameter( my_agent, "accumulation-mode", RL_RETURN_LONG ) == RL_ACCUMULATION_AVG ) )
			reward = ( reward / ( (float) reward_count ) );

		data->reward += discount_reward( my_agent, reward, data->step );
	}

	// update stats
	double global_reward = get_rl_stat( my_agent, "global-reward" );
	set_rl_stat( my_agent, "total-reward", reward );
	set_rl_stat( my_agent, "global-reward", ( global_reward + reward ) );

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
float discount_reward( agent *my_agent, float reward, unsigned int step )
{
	double return_val = 0;
	const long mode = get_rl_parameter( my_agent, "discount-mode", RL_RETURN_LONG );
	double rate;

	if ( mode == RL_DISCOUNT_EXPONENTIAL )
	{
		rate = get_rl_parameter( my_agent, "exponential-discount-rate" );
		
		return_val = ( reward * pow( rate, (double) step ) );
	}
	else if ( mode == RL_DISCOUNT_LINEAR )
	{
		rate = get_rl_parameter( my_agent, "linear-discount-rate" );
		double stepped_rate = ( rate * (double) step );

		if ( reward > 0 )
		{
			return_val = ( ( reward > stepped_rate )?( reward - stepped_rate ):( 0 ) );
		}
		else if ( reward < 0 )
		{
			return_val = ( ( fabs( reward ) > stepped_rate )?( reward + stepped_rate ):( 0 ) );
		}
	}

	return return_val;
}

/***************************************************************************
 * Function     : store_rl_data
 **************************************************************************/
void store_rl_data( agent *my_agent, Symbol *goal, preference *cand )
{
	rl_data *data = goal->id.rl_info;
	Symbol *op = cand->value;
    data->previous_q = cand->numeric_value;
	
	// Make list of just-fired prods
	for ( preference *pref = goal->id.operator_slot->preferences[ NUMERIC_INDIFFERENT_PREFERENCE_TYPE ]; pref; pref = pref->next )
		if ( ( op == pref->value ) && pref->inst->prod->rl_rule )
			if ( pref->inst->prod->rl_rule ) 
				push( my_agent, pref->inst->prod, data->prev_op_rl_rules );
}

/***************************************************************************
 * Function     : perform_rl_update
 **************************************************************************/
void perform_rl_update( agent *my_agent, float op_value, Symbol *goal )
{	
	rl_data *data = goal->id.rl_info;
	soar_rl_et_map::iterator iter;

	double alpha = get_rl_parameter( my_agent, "learning-rate" );
	double lambda = get_rl_parameter( my_agent, "eligibility-trace-decay-rate" );
	double gamma = get_rl_parameter( my_agent, "eligibility-trace-discount-rate" );
	double tolerance = get_rl_parameter( my_agent, "eligibility-trace-tolerance" );

	// Iterate through eligibility_traces, decay traces. If less than TOLERANCE, remove from map.
	for ( iter = data->eligibility_traces->begin(); iter != data->eligibility_traces->end(); )
	{
		iter->second *= lambda;
		iter->second *= pow( gamma, (double) data->step );
		if ( iter->second < tolerance ) 
			data->eligibility_traces->erase( iter++ );
		else 
			++iter;
	}
	
	// Update trace for just fired prods
	int num_prev_fired_rules = 0;
	for ( cons *c = data->prev_op_rl_rules; c; c = c->rest )
		if (c->first) 
			num_prev_fired_rules++;

	if ( num_prev_fired_rules )
	{
		double trace_increment = 1.0 / num_prev_fired_rules;
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
	free_list( my_agent, data->prev_op_rl_rules );
	data->prev_op_rl_rules = NIL;

	// compute TD update, set stat
	float update = data->reward;
	update += ( pow( gamma, (double) data->step ) * op_value );
	update -= data->previous_q;
	set_rl_stat( my_agent, "update-error", (double) ( -update ) );
	
	// For each prod in map, add alpha*delta*trace to value
	for ( iter = data->eligibility_traces->begin(); iter != data->eligibility_traces->end(); iter++ )
	{	
		production *prod = iter->first;
		float temp = get_number_from_symbol( rhs_value_to_symbol( prod->action_list->referent ) );
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
