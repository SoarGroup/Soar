#include <portability.h>

/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
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

#include "reinforcement_learning.h"

#include <stdlib.h>
#include <math.h>
#include <vector>

#include "production.h"
#include "rhsfun.h"
#include "instantiations.h"
#include "rete.h"
#include "wmem.h"

#include "print.h"
#include "xml.h"

#include "misc.h"

extern Symbol *instantiate_rhs_value (agent* thisAgent, rhs_value rv, goal_stack_level new_id_level, char new_id_letter, struct token_struct *tok, wme *w);
extern void variablize_symbol (agent* thisAgent, Symbol **sym);
extern void variablize_nots_and_insert_into_conditions (agent* thisAgent, not_struct *nots, condition *conds);
extern void variablize_condition_list (agent* thisAgent, condition *cond);


/////////////////////////////////////////////////////
// Parameters
/////////////////////////////////////////////////////

rl_param_container::rl_param_container( agent *new_agent ): param_container( new_agent )
{
	// learning
	learning = new boolean_param( "learning", off, new f_predicate<boolean>() );
	add( learning );

	// discount-rate
	discount_rate = new decimal_param( "discount-rate", 0.9, new btw_predicate<double>( 0, 1, true ), new f_predicate<double>() );
	add( discount_rate );

	// learning-rate
	learning_rate = new decimal_param( "learning-rate", 0.3, new btw_predicate<double>( 0, 1, true ), new f_predicate<double>() );
	add( learning_rate );

	// learning-policy
	learning_policy = new constant_param<learning_choices>( "learning-policy", sarsa, new f_predicate<learning_choices>() );
	learning_policy->add_mapping( sarsa, "sarsa" );
	learning_policy->add_mapping( q, "q-learning" );
	add( learning_policy );

	// eligibility-trace-decay-rate
	et_decay_rate = new decimal_param( "eligibility-trace-decay-rate", 0, new btw_predicate<double>( 0, 1, true ), new f_predicate<double>() );
	add( et_decay_rate );

	// eligibility-trace-tolerance
	et_tolerance = new decimal_param( "eligibility-trace-tolerance", 0.001, new gt_predicate<double>( 0, false ), new f_predicate<double>() );
	add( et_tolerance );

	// temporal-extension
	temporal_extension = new boolean_param( "temporal-extension", on, new f_predicate<boolean>() );
	add( temporal_extension );

	// hrl-discount
	hrl_discount = new boolean_param( "hrl-discount", on, new f_predicate<boolean>() );
	add( hrl_discount );
};

/////////////////////////////////////////////////////
/////////////////////////////////////////////////////

/////////////////////////////////////////////////////
// Stats
/////////////////////////////////////////////////////

rl_stat_container::rl_stat_container( agent *new_agent ): stat_container( new_agent )
{
	// update-error
	update_error = new decimal_stat( "update-error", 0, new f_predicate<double>() );
	add( update_error );

	// total-reward
	total_reward = new decimal_stat( "total-reward", 0, new f_predicate<double>() );
	add( total_reward );

	// global-reward
	global_reward = new decimal_stat( "global-reward", 0, new f_predicate<double>() );
	add( global_reward );
};

/////////////////////////////////////////////////////
/////////////////////////////////////////////////////


/***************************************************************************
 * Function     : rl_enabled
 **************************************************************************/
inline bool rl_enabled( agent *my_agent )
{
	return ( my_agent->rl_params->learning->get_value() == soar_module::on );
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
		data->reward_age = 0;
		
		goal = goal->id.lower_goal;
	}
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
	my_agent->variablization_tc = (0u - 1);
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
	
	if ( data->num_prev_op_rl_rules )
	{
		slot *s = goal->id.reward_header->id.slots;
		slot *t;
		wme *w, *x;
		
		double reward = 0.0;
		double discount_rate = my_agent->rl_params->discount_rate->get_value();	

		if ( s )
		{
			for ( ; s; s = s->next )
				for ( w = s->wmes ; w; w = w->next)
					if ( w->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE )
						for ( t = w->value->id.slots; t; t = t->next )
							for ( x = t->wmes; x; x = x->next )
								if ( ( x->value->common.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE ) || ( x->value->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE ) )
									reward = reward + get_number_from_symbol( x->value );
			
			
			data->reward += ( reward * pow( discount_rate, (double) data->reward_age ) );
		}

		// update stats
		double global_reward = my_agent->rl_stats->global_reward->get_value();
		my_agent->rl_stats->total_reward->set_value( reward );
		my_agent->rl_stats->global_reward->set_value( global_reward + reward );
		
		if ( ( my_agent->rl_params->hrl_discount->get_value() == soar_module::on ) )
		{
			if ( goal != my_agent->bottom_goal )
				data->reward_age++;
		}
	}
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
 * Function     : rl_store_data
 **************************************************************************/
void rl_store_data( agent *my_agent, Symbol *goal, preference *cand )
{
	rl_data *data = goal->id.rl_info;
	Symbol *op = cand->value;    

	bool using_gaps = ( my_agent->rl_params->temporal_extension->get_value() == soar_module::on );
	
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
		data->num_prev_op_rl_rules = just_fired;
		data->previous_q = cand->numeric_value;
	}
	else
	{
		if ( my_agent->sysparams[ TRACE_RL_SYSPARAM ] && using_gaps &&
			( data->reward_age == 0 ) && data->num_prev_op_rl_rules )
		{			
			char buf[256];
			SNPRINTF( buf, 254, "gap started (%c%d)", goal->id.name_letter, goal->id.name_number );
			
			print( my_agent, buf );
			xml_generate_warning( my_agent, buf );
		}
		
		if ( !using_gaps )
		{
			data->prev_op_rl_rules = NIL;
			data->num_prev_op_rl_rules = 0;
			data->previous_q = cand->numeric_value;
		}
		else
		{		
			if ( data->num_prev_op_rl_rules )
				data->reward_age++;
		}
	}
}

/***************************************************************************
 * Function     : rl_perform_update
 **************************************************************************/
void rl_perform_update( agent *my_agent, double op_value, bool op_rl, Symbol *goal )
{
	bool using_gaps = ( my_agent->rl_params->temporal_extension->get_value() == soar_module::on );

	if ( !using_gaps || op_rl )
	{		
		rl_data *data = goal->id.rl_info;
		
		if ( data->num_prev_op_rl_rules )
		{
			rl_et_map::iterator iter;
			
			double alpha = my_agent->rl_params->learning_rate->get_value();
			double lambda = my_agent->rl_params->et_decay_rate->get_value();
			double gamma = my_agent->rl_params->discount_rate->get_value();
			double tolerance = my_agent->rl_params->et_tolerance->get_value();

			// compute TD update, set stat
			double update = data->reward;
			double discount = pow( gamma, (double) data->reward_age );

			if ( my_agent->sysparams[ TRACE_RL_SYSPARAM ] && using_gaps && data->reward_age )
			{
				char buf[256];
				SNPRINTF( buf, 254, "gap ended (%c%d)", goal->id.name_letter, goal->id.name_number );
				
				print( my_agent, buf );
				xml_generate_warning( my_agent, buf );			
			}			

			update += ( discount * op_value );
			update -= data->previous_q;
			my_agent->rl_stats->update_error->set_value( (double) ( -update ) );

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
					iter->second *= discount;
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

				double delta = (update * alpha * iter->second);

				// SBW 12/18/08
				if ( my_agent->sysparams[ TRACE_RL_SYSPARAM ] ) 
				{ 
					std::string* oldValString = to_string( temp );
					double newVal = temp + delta;
					std::string* newValString = to_string(newVal);
					std::string message = "updating RL rule " + std::string(prod->name->sc.name) + " from " + *oldValString + " to " + *newValString; 
					
					print( my_agent, const_cast<char *>( message.c_str() ) );
					xml_generate_message( my_agent, const_cast<char *>( message.c_str() ) );
					
					delete oldValString;
					delete newValString;
				}
			    
				temp += delta;

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
		}

		data->reward_age = 0;
		data->reward = 0.0;
	}
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
