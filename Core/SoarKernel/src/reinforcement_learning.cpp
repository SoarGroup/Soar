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
#include "recmem.h"
#include "chunk.h"
#include "rete.h"

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
	return is_set( my_agent->rl_params, temp );
	delete temp;
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

	Bool chunk_var;
	condition *cond_top, *cond_bottom;

	// build the instantiated conditions, and bind LHS variables
	p_node_to_conditions_and_nots( my_agent, my_template->p_node, tok, w, 
									&( my_template_instance->top_of_instantiated_conditions ), 
									&( my_template_instance->bottom_of_instantiated_conditions ), 
									&( my_template_instance->nots ), NIL );

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

	// make new action list
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
	chunk_var = my_agent->variablize_this_chunk;
	my_agent->variablize_this_chunk = TRUE;
	reset_variable_generator( my_agent, cond_top, NIL );
	my_agent->variablization_tc = get_new_tc_number( my_agent );
	variablize_condition_list( my_agent, cond_top );
	variablize_nots_and_insert_into_conditions( my_agent, my_template_instance->nots, cond_top );

	// make new production
	production *new_production = make_production( my_agent, USER_PRODUCTION_TYPE, new_name_symbol, &cond_top, &cond_bottom, &new_action, false );
	my_agent->variablize_this_chunk = chunk_var;

	// attempt to add to rete, remove if duplicate
	if ( add_production_to_rete( my_agent, new_production, cond_top, 0, false ) == DUPLICATE_PRODUCTION )
	{
		excise_production( my_agent, new_production, false );
		revert_template_tracking( my_agent, my_template->name->sc.name );

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

