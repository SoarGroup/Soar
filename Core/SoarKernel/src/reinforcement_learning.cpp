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

#include <cstdlib>
#include <cmath>
#include <vector>
#include <fstream>
#include <sstream>

#include "agent.h"
#include "reinforcement_learning.h"
#include "production.h"
#include "rhsfun.h"
#include "instantiations.h"
#include "rete.h"
#include "wmem.h"
#include "tempmem.h"
#include "print.h"
#include "xml.h"
#include "utilities.h"
#include "recmem.h"

#include <iostream> ///< bazald

extern Symbol *instantiate_rhs_value (agent* thisAgent, rhs_value rv, goal_stack_level new_id_level, char new_id_letter, struct token_struct *tok, wme *w);
extern void variablize_symbol (agent* thisAgent, Symbol **sym);
extern void variablize_nots_and_insert_into_conditions (agent* thisAgent, not_struct *nots, condition *conds);
extern void variablize_condition_list (agent* thisAgent, condition *cond);

/////////////////////////////////////////////////////
// Parameters
/////////////////////////////////////////////////////

const std::vector<std::pair<std::string, param_accessor<double> *> > &rl_param_container::get_documentation_params() {
    static std::vector<std::pair<std::string, param_accessor<double> *> > documentation_params;
    static bool initted = false;
    if (!initted) {
        initted = true;
        // Is it okay to use new here, because this is a static variable anyway,
        // so it's not going to happen more than once and shouldn't ever be cleaned up?
        documentation_params.push_back(std::make_pair("rl-updates", new rl_updates_accessor()));
        documentation_params.push_back(std::make_pair("delta-bar-delta-h", new rl_dbd_h_accessor()));
    }
    return documentation_params;
}

rl_param_container::rl_param_container( agent *new_agent ): soar_module::param_container( new_agent )
{
    // learning
    learning = new rl_learning_param( "learning", soar_module::off, new soar_module::f_predicate<soar_module::boolean>(), new_agent );
    add( learning );

    // meta-learning-rate
    meta_learning_rate = new soar_module::decimal_param( "meta-learning-rate", 0.1, new soar_module::btw_predicate<double>( 0, 1, true ), new soar_module::f_predicate<double>() );
    add( meta_learning_rate );

    // update-log-path
    update_log_path = new soar_module::string_param( "update-log-path", "", new soar_module::predicate<const char *>(), new soar_module::f_predicate<const char *>() );
    add( update_log_path );

	// discount-rate
	discount_rate = new soar_module::decimal_param( "discount-rate", 0.9, new soar_module::btw_predicate<double>( 0, 1, true ), new soar_module::f_predicate<double>() );
	add( discount_rate );

  // influence discount-rate
  influence_discount_rate = new soar_module::decimal_param( "influence-discount-rate", 0.5, new soar_module::btw_predicate<double>( 0, 1, true ), new soar_module::f_predicate<double>() ); ///< bazald
  add( influence_discount_rate );

	// learning-rate
	learning_rate = new soar_module::decimal_param( "learning-rate", 0.3, new soar_module::btw_predicate<double>( 0, 1, true ), new soar_module::f_predicate<double>() );
	add( learning_rate );

	// learning-policy
	learning_policy = new soar_module::constant_param<learning_choices>( "learning-policy", sarsa, new soar_module::f_predicate<learning_choices>() );
	learning_policy->add_mapping( sarsa, "sarsa" );
	learning_policy->add_mapping( q, "q-learning" );
	add( learning_policy );

    // decay-mode
    decay_mode = new soar_module::constant_param<decay_choices>( "decay-mode", normal_decay, new soar_module::f_predicate<decay_choices>() );
    decay_mode->add_mapping( normal_decay, "normal" );
    decay_mode->add_mapping( exponential_decay, "exp" );
    decay_mode->add_mapping( logarithmic_decay, "log" );
    decay_mode->add_mapping( delta_bar_delta_decay, "delta-bar-delta" );
    add( decay_mode );

	// eligibility-trace-decay-rate
	et_decay_rate = new soar_module::decimal_param( "eligibility-trace-decay-rate", 0, new soar_module::btw_predicate<double>( 0, 1, true ), new soar_module::f_predicate<double>() );
	add( et_decay_rate );

	// eligibility-trace-tolerance
	et_tolerance = new soar_module::decimal_param( "eligibility-trace-tolerance", 0.001, new soar_module::gt_predicate<double>( 0, false ), new soar_module::f_predicate<double>() );
	add( et_tolerance );

  // rl-impasse
  rl_impasse = new soar_module::boolean_param( "rl-impasse", soar_module::off, new soar_module::f_predicate<soar_module::boolean>() ); ///< bazald
  add( rl_impasse );

  // fc-credit
  credit_assignment = new rl_credit_assignment_param( "credit-assignment", credit_even, new soar_module::f_predicate<credit_assignment_choices>(), my_agent );
  credit_assignment->add_mapping( credit_even, "even" );
  credit_assignment->add_mapping( credit_fc, "fc" );
  credit_assignment->add_mapping( credit_rl, "rl" );
  credit_assignment->add_mapping( credit_logrl, "log-rl" );
  add( credit_assignment );
  
	// temporal-extension
	temporal_extension = new soar_module::boolean_param( "temporal-extension", soar_module::on, new soar_module::f_predicate<soar_module::boolean>() );
	add( temporal_extension );

	// hrl-discount
	hrl_discount = new soar_module::boolean_param( "hrl-discount", soar_module::off, new soar_module::f_predicate<soar_module::boolean>() );
	add( hrl_discount );

	// temporal-discount
	temporal_discount = new soar_module::boolean_param( "temporal-discount", soar_module::on, new soar_module::f_predicate<soar_module::boolean>() );
	add( temporal_discount );

	// chunk-stop
	chunk_stop = new soar_module::boolean_param( "chunk-stop", soar_module::on, new soar_module::f_predicate<soar_module::boolean>() );
	add( chunk_stop );

	// meta
	meta = new soar_module::boolean_param( "meta", soar_module::off, new soar_module::f_predicate<soar_module::boolean>() );
	add( meta );

	// apoptosis
	apoptosis = new rl_apoptosis_param( "apoptosis", apoptosis_none, new soar_module::f_predicate<apoptosis_choices>(), my_agent );
	apoptosis->add_mapping( apoptosis_none, "none" );
	apoptosis->add_mapping( apoptosis_chunks, "chunks" );
	apoptosis->add_mapping( apoptosis_rl, "rl-chunks" );
	add( apoptosis );

	// apoptosis-decay
	apoptosis_decay = new soar_module::decimal_param( "apoptosis-decay", 0.5, new soar_module::btw_predicate<double>( 0, 1, true ), new rl_apoptosis_predicate<double>( my_agent ) );
	add( apoptosis_decay );

	// apoptosis-thresh
	apoptosis_thresh = new rl_apoptosis_thresh_param( "apoptosis-thresh", -2.0, new soar_module::gt_predicate<double>( 0, false ), new rl_apoptosis_predicate<double>( my_agent ) );
	add( apoptosis_thresh );
};

//

void rl_reset_data( agent* );

rl_learning_param::rl_learning_param( const char *new_name, soar_module::boolean new_value, soar_module::predicate<soar_module::boolean> *new_prot_pred, agent *new_agent ): soar_module::boolean_param( new_name, new_value, new_prot_pred ), my_agent( new_agent ) {}

void rl_learning_param::set_value( soar_module::boolean new_value )
{
	if ( new_value != value )
	{
		if ( new_value == soar_module::off )
		{
			rl_reset_data( my_agent );
		}

		value = new_value;
	}
}

//

rl_apoptosis_param::rl_apoptosis_param( const char *new_name, rl_param_container::apoptosis_choices new_value, soar_module::predicate<rl_param_container::apoptosis_choices> *new_prot_pred, agent *new_agent ): soar_module::constant_param<rl_param_container::apoptosis_choices>( new_name, new_value, new_prot_pred ), my_agent( new_agent ) {}

void rl_apoptosis_param::set_value( rl_param_container::apoptosis_choices new_value )
{
  if ( value != new_value )
  {
    // from off to on (doesn't matter which)
    if ( value == rl_param_container::apoptosis_none )
    {
      my_agent->rl_prods->set_decay_rate( my_agent->rl_params->apoptosis_decay->get_value() );
      my_agent->rl_prods->set_decay_thresh( my_agent->rl_params->apoptosis_thresh->get_value() );
      my_agent->rl_prods->initialize();
    }
    // from on to off
    else if ( new_value == rl_param_container::apoptosis_none )
    {
      my_agent->rl_prods->teardown();
    }

    value = new_value;
  }
}

//

rl_apoptosis_thresh_param::rl_apoptosis_thresh_param( const char* new_name, double new_value, soar_module::predicate<double>* new_val_pred, soar_module::predicate<double>* new_prot_pred ): soar_module::decimal_param( new_name, new_value, new_val_pred, new_prot_pred ) {}

void rl_apoptosis_thresh_param::set_value( double new_value ) { value = -new_value; }

//

template <typename T>
rl_apoptosis_predicate<T>::rl_apoptosis_predicate( agent *new_agent ): soar_module::agent_predicate<T>( new_agent ) {}

template <typename T>
bool rl_apoptosis_predicate<T>::operator() ( T /*val*/ ) { return ( this->my_agent->rl_params->apoptosis->get_value() != rl_param_container::apoptosis_none ); }

rl_credit_assignment_param::rl_credit_assignment_param( const char *new_name, rl_param_container::credit_assignment_choices new_value, soar_module::predicate<rl_param_container::credit_assignment_choices> *new_prot_pred, agent *new_agent ): soar_module::constant_param<rl_param_container::credit_assignment_choices>( new_name, new_value, new_prot_pred ), my_agent( new_agent ) {}

void rl_credit_assignment_param::set_value( rl_param_container::credit_assignment_choices new_value )
{
  value = new_value;
}


/////////////////////////////////////////////////////
/////////////////////////////////////////////////////

/////////////////////////////////////////////////////
// Stats
/////////////////////////////////////////////////////

rl_stat_container::rl_stat_container( agent *new_agent ): stat_container( new_agent )
{
	// update-error
	update_error = new soar_module::decimal_stat( "update-error", 0, new soar_module::f_predicate<double>() );
	add( update_error );

	// total-reward
	total_reward = new soar_module::decimal_stat( "total-reward", 0, new soar_module::f_predicate<double>() );
	add( total_reward );

	// global-reward
	global_reward = new soar_module::decimal_stat( "global-reward", 0, new soar_module::f_predicate<double>() );
	add( global_reward );
};


/////////////////////////////////////////////////////
/////////////////////////////////////////////////////

// quick shortcut to determine if rl is enabled
bool rl_enabled( agent *my_agent )
{
	return ( my_agent->rl_params->learning->get_value() == soar_module::on );
}

/////////////////////////////////////////////////////
/////////////////////////////////////////////////////

inline void rl_add_ref( Symbol* goal, production* prod )
{
	goal->id.rl_info->prev_op_rl_rules->push_back( prod );
	prod->rl_ref_count++;
}

inline void rl_remove_ref( Symbol* goal, production* prod )
{
	rl_rule_list* rules = goal->id.rl_info->prev_op_rl_rules;
	
	for ( rl_rule_list::iterator p=rules->begin(); p!=rules->end(); p++ )
	{
		if ( *p == prod )
		{
			prod->rl_ref_count--;
		}
	}

	rules->remove( prod );
}

void rl_clear_refs( Symbol* goal )
{
	rl_rule_list* rules = goal->id.rl_info->prev_op_rl_rules;
	
	for ( rl_rule_list::iterator p=rules->begin(); p!=rules->end(); p++ )
	{
		(*p)->rl_ref_count--;
	}

	rules->clear();
}

/////////////////////////////////////////////////////
/////////////////////////////////////////////////////

// resets rl data structures
void rl_reset_data( agent *my_agent )
{
	Symbol *goal = my_agent->top_goal;
	while( goal )
	{
		rl_data *data = goal->id.rl_info;

		data->eligibility_traces->clear();
		rl_clear_refs( goal );

		data->previous_q = 0;
		data->reward = 0;

		data->gap_age = 0;
		data->hrl_age = 0;
		
		goal = goal->id.lower_goal;
	}
}

// removes rl references to a production (used for excise)
void rl_remove_refs_for_prod( agent *my_agent, production *prod )
{
	for ( Symbol* state = my_agent->top_state; state; state = state->id.lower_goal )
	{
		state->id.rl_info->eligibility_traces->erase( prod );
		rl_remove_ref( state, prod );
	}
}


/////////////////////////////////////////////////////
/////////////////////////////////////////////////////

// returns true if a template is valid
bool rl_valid_template( production *prod )
{
	bool numeric_pref = false;
	bool var_pref = false;
	int num_actions = 0;

	for ( action *a = prod->action_list; a; a = a->next ) 
	{
		num_actions++;
		if ( a->type == MAKE_ACTION )
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

// returns true if an rl rule is valid
bool rl_valid_rule( production *prod )
{
	bool numeric_pref = false;
	int num_actions = 0;

	for ( action *a = prod->action_list; a; a = a->next ) 
	{
		num_actions++;
		if ( a->type == MAKE_ACTION )
		{
			if ( a->preference_type == NUMERIC_INDIFFERENT_PREFERENCE_TYPE )
				numeric_pref = true;
		}
	}

	return ( numeric_pref && ( num_actions == 1 ) );
}

// sets rl meta-data from a production documentation string
void rl_rule_meta( agent* my_agent, production* prod )
{
	if ( prod->documentation && ( my_agent->rl_params->meta->get_value() == soar_module::on ) )
	{
		std::string doc( prod->documentation );

        const std::vector<std::pair<std::string, param_accessor<double> *> > &documentation_params = my_agent->rl_params->get_documentation_params();
        for (std::vector<std::pair<std::string, param_accessor<double> *> >::const_iterator doc_params_it = documentation_params.begin();
                doc_params_it != documentation_params.end(); ++doc_params_it) {
            const std::string &param_name = doc_params_it->first;
            param_accessor<double> *accessor = doc_params_it->second;
            std::stringstream param_name_ss;
            param_name_ss << param_name << "=";
            std::string search_term = param_name_ss.str();
            size_t begin_index = doc.find(search_term);
            if (begin_index == std::string::npos) continue;
            begin_index += search_term.size();
            size_t end_index = doc.find(";", begin_index);
            if (end_index == std::string::npos) continue;
            std::string param_value_str = doc.substr(begin_index, end_index);
            accessor->set_param(prod, param_value_str);
        }

        /*
		std::string search( "rlupdates=" );

		if ( doc.length() > search.length() )
		{
			if ( doc.substr( 0, search.length() ).compare( search ) == 0 )
			{
				uint64_t val;
				from_string( val, doc.substr( search.length() ) );

				prod->rl_update_count = static_cast< double >( val );
			}
		}
        */
	}
}


/////////////////////////////////////////////////////
/////////////////////////////////////////////////////

// gets the auto-assigned id of a template instantiation
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
	if ( !is_whole_number( id_str ) )
		return -1;
	
	// convert id
	int id;
	from_string( id, id_str );
	return id;
}

// initializes the max rl template counter
void rl_initialize_template_tracking( agent *my_agent )
{
	my_agent->rl_template_count = 1;
}

// updates rl template counter for a rule
void rl_update_template_tracking( agent *my_agent, const char *rule_name )
{
	int new_id = rl_get_template_id( rule_name );

	if ( ( new_id != -1 ) && ( new_id > my_agent->rl_template_count ) )
		my_agent->rl_template_count = ( new_id + 1 );
}

// gets the next template-assigned id
int rl_next_template_id( agent *my_agent )
{
	return (my_agent->rl_template_count++);
}

// gives back a template-assigned id (on auto-retract)
void rl_revert_template_id( agent *my_agent )
{
	my_agent->rl_template_count--;
}

inline void rl_get_symbol_constant( Symbol* p_sym, Symbol* i_sym, rl_symbol_map* constants )
{
	if ( ( p_sym->common.symbol_type == VARIABLE_SYMBOL_TYPE ) && ( ( i_sym->common.symbol_type != IDENTIFIER_SYMBOL_TYPE ) || ( i_sym->id.smem_lti != NIL ) ) )
	{
		constants->insert( std::make_pair< Symbol*, Symbol* >( p_sym, i_sym ) );
	}
}

void rl_get_test_constant( test* p_test, test* i_test, rl_symbol_map* constants )
{
	if ( test_is_blank_test( *p_test ) )
	{
		return;
	}
	
	if ( test_is_blank_or_equality_test( *p_test ) )
	{		
		rl_get_symbol_constant( *(reinterpret_cast<Symbol**>( p_test )), *(reinterpret_cast<Symbol**>( i_test )), constants );

		return;
	}
	
	
	// complex test stuff
	// NLD: If the code below is uncommented, it accesses bad memory on the first
	//      id test and segfaults.  I'm honestly unsure why (perhaps something
	//      about state test?).  Most of this code was copied/adapted from
	//      the variablize_test code in production.cpp.
	/*
	{
		complex_test* p_ct = complex_test_from_test( *p_test );
		complex_test* i_ct = complex_test_from_test( *i_test );	

		if ( ( p_ct->type == GOAL_ID_TEST ) || ( p_ct->type == IMPASSE_ID_TEST ) || ( p_ct->type == DISJUNCTION_TEST ) )
		{
			return;
		}
		else if ( p_ct->type == CONJUNCTIVE_TEST )
		{
			cons* p_c=p_ct->data.conjunct_list;
			cons* i_c=i_ct->data.conjunct_list;

			while ( p_c )
			{
				rl_get_test_constant( reinterpret_cast<test*>( &( p_c->first ) ), reinterpret_cast<test*>( &( i_c->first ) ), constants );
				
				p_c = p_c->rest;
				i_c = i_c->rest;
			}

			return;
		}
		else
		{
			rl_get_symbol_constant( p_ct->data.referent, i_ct->data.referent, constants );

			return;
		}
	}
	*/
}

void rl_get_template_constants( condition* p_conds, condition* i_conds, rl_symbol_map* constants )
{
	condition* p_cond = p_conds;
	condition* i_cond = i_conds;

	while ( p_cond )
	{
		if ( ( p_cond->type == POSITIVE_CONDITION ) || ( p_cond->type == NEGATIVE_CONDITION ) )
		{
			rl_get_test_constant( &( p_cond->data.tests.id_test ), &( i_cond->data.tests.id_test ), constants );
			rl_get_test_constant( &( p_cond->data.tests.attr_test ), &( i_cond->data.tests.attr_test ), constants );
			rl_get_test_constant( &( p_cond->data.tests.value_test ), &( i_cond->data.tests.value_test ), constants );
		}
		else if ( p_cond->type == CONJUNCTIVE_NEGATION_CONDITION )
		{
			rl_get_template_constants( p_cond->data.ncc.top, i_cond->data.ncc.top, constants );
		}
		
		p_cond = p_cond->next;
		i_cond = i_cond->next;
	}
}

// builds a template instantiation
 Symbol *rl_build_template_instantiation( agent *my_agent, instantiation *my_template_instance, struct token_struct *tok, wme *w )
{	
	Symbol* return_val = NULL;
	
	// initialize production conditions
	if ( my_template_instance->prod->rl_template_conds == NIL )
	{
		not_struct* nots;
		condition* c_top;
		condition* c_bottom;

		p_node_to_conditions_and_nots( my_agent, my_template_instance->prod->p_node, NIL, NIL, &( c_top ), &( c_bottom ), &( nots ), NIL );

		my_template_instance->prod->rl_template_conds = c_top;
	}

	// initialize production instantiation set
	if ( my_template_instance->prod->rl_template_instantiations == NIL )
	{
		my_template_instance->prod->rl_template_instantiations = new rl_symbol_map_set;
	}

	// get constants
	rl_symbol_map constant_map;
	{	
		rl_get_template_constants( my_template_instance->prod->rl_template_conds, my_template_instance->top_of_instantiated_conditions, &( constant_map ) );		
	}

	// try to insert into instantiation set
	//if ( !constant_map.empty() )
	{
		std::pair< rl_symbol_map_set::iterator, bool > ins_result = my_template_instance->prod->rl_template_instantiations->insert( constant_map );
		if ( ins_result.second )
		{
			Symbol *id, *attr, *value, *referent;
			production *my_template = my_template_instance->prod;
			action *my_action = my_template->action_list;
			char first_letter;
			double init_value = 0;
			condition *cond_top, *cond_bottom;

			// make unique production name
			Symbol *new_name_symbol;
			std::string new_name = "";
			std::string empty_string = "";
			std::string temp_id;
			int new_id;
			do
			{
				new_id = rl_next_template_id( my_agent );
				to_string( new_id, temp_id );
				new_name = ( "rl*" + empty_string + my_template->name->sc.name + "*" + temp_id );
			} while ( find_sym_constant( my_agent, new_name.c_str() ) != NIL );
			new_name_symbol = make_sym_constant( my_agent, new_name.c_str() );
			
			// prep conditions
			copy_condition_list( my_agent, my_template_instance->top_of_instantiated_conditions, &cond_top, &cond_bottom );
			rl_add_goal_or_impasse_tests_to_conds( my_agent, cond_top );
			reset_variable_generator( my_agent, cond_top, NIL );
			my_agent->variablization_tc = get_new_tc_number( my_agent );
			variablize_condition_list( my_agent, cond_top );
			variablize_nots_and_insert_into_conditions( my_agent, my_template_instance->nots, cond_top );

			// get the preference value
			id = instantiate_rhs_value( my_agent, my_action->id, -1, 's', tok, w );
			attr = instantiate_rhs_value( my_agent, my_action->attr, id->id.level, 'a', tok, w );
			first_letter = first_letter_from_symbol( attr );
			value = instantiate_rhs_value( my_agent, my_action->value, id->id.level, first_letter, tok, w );
			referent = instantiate_rhs_value( my_agent, my_action->referent, id->id.level, first_letter, tok, w );

			// clean up after yourself :)
			symbol_remove_ref( my_agent, id );
			symbol_remove_ref( my_agent, attr );
			symbol_remove_ref( my_agent, value );
			symbol_remove_ref( my_agent, referent );

			// make new action list
			action *new_action = rl_make_simple_action( my_agent, id, attr, value, referent );
			new_action->preference_type = NUMERIC_INDIFFERENT_PREFERENCE_TYPE;

			// make new production
			production *new_production = make_production( my_agent, USER_PRODUCTION_TYPE, new_name_symbol, &cond_top, &cond_bottom, &new_action, false );

			// set initial expected reward values
			{
				if ( referent->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE )
				{
					init_value = static_cast< double >( referent->ic.value );
				}
				else if ( referent->common.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE )
				{
					init_value = referent->fc.value;
				}

				new_production->rl_ecr = 0.0;
				new_production->rl_efr = init_value;
        new_production->rl_mean2 = 0.0; ///< bazald
        new_production->rl_sample_variance = 0.0; ///< bazald
        new_production->rl_partial_variance = 0.0; ///< bazald
        new_production->rl_total_variance = 0.0; ///< bazald
        new_production->rl_tolerable_variance = 0.002; ///< bazald
        new_production->rl_sample_influence_cycle = 0; ///< bazald
        new_production->rl_sample_influence_updates = 0; ///< bazald
        new_production->rl_sample_influence_p = 0; ///< bazald
        new_production->rl_sample_influence_rest = 0; ///< bazald
        new_production->rl_sample_influence_input = 0; ///< bazald
			}

			// attempt to add to rete, remove if duplicate
			if ( add_production_to_rete( my_agent, new_production, cond_top, NULL, FALSE, TRUE ) == DUPLICATE_PRODUCTION )
			{
				excise_production( my_agent, new_production, false );
				rl_revert_template_id( my_agent );

				new_name_symbol = NULL;
			}
			deallocate_condition_list( my_agent, cond_top );

			return_val = new_name_symbol;
		}
	}

	return return_val;
}

// creates an action for a template instantiation
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
			ct->type = static_cast<byte>( ( id->id.isa_goal )?( GOAL_ID_TEST ):( IMPASSE_ID_TEST ) );
			t = make_test_from_complex_test( ct );
			add_new_test_to_test( my_agent, &( cond->data.tests.id_test ), t );
			id->id.tc_num = tc;
		}
	}
}


/////////////////////////////////////////////////////
/////////////////////////////////////////////////////

// gathers discounted reward for a state
void rl_tabulate_reward_value_for_goal( agent *my_agent, Symbol *goal )
{
	rl_data *data = goal->id.rl_info;	
	
	if ( !data->prev_op_rl_rules->empty() )
	{
		slot *s = find_slot( goal->id.reward_header, my_agent->rl_sym_reward );
		slot *t;
		wme *w, *x;
		
		double reward = 0.0;
		double discount_rate = my_agent->rl_params->discount_rate->get_value();

		if ( s )
		{			
			for ( w=s->wmes; w; w=w->next )
			{
				if ( w->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE )
				{
					t = find_slot( w->value, my_agent->rl_sym_value );
					if ( t )
					{
						for ( x=t->wmes; x; x=x->next )
						{
							if ( ( x->value->common.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE ) || ( x->value->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE ) )
							{
								reward += get_number_from_symbol( x->value );
							}
						}
					}
				}
			}
			
			// if temporal_discount is off, don't discount for gaps
			unsigned int effective_age = data->hrl_age;
			if (my_agent->rl_params->temporal_discount->get_value() == soar_module::on) {
				effective_age += data->gap_age;
			}

			data->reward += ( reward * pow( discount_rate, static_cast< double >( effective_age ) ) );
		}

		// update stats
		double global_reward = my_agent->rl_stats->global_reward->get_value();
		my_agent->rl_stats->total_reward->set_value( reward );
		my_agent->rl_stats->global_reward->set_value( global_reward + reward );

		if ( ( goal != my_agent->bottom_goal ) && ( my_agent->rl_params->hrl_discount->get_value() == soar_module::on ) )
		{
			data->hrl_age++;
		}
	}
}

// gathers reward for all states
void rl_tabulate_reward_values( agent *my_agent )
{
	Symbol *goal = my_agent->top_goal;

	while( goal )
	{
		rl_tabulate_reward_value_for_goal( my_agent, goal );
	    goal = goal->id.lower_goal;
	}
}

// stores rl info for a state w.r.t. a selected operator
void rl_store_data( agent *my_agent, Symbol *goal, preference *cand )
{
	rl_data *data = goal->id.rl_info;
	Symbol *op = cand->value;    

	bool using_gaps = ( my_agent->rl_params->temporal_extension->get_value() == soar_module::on );
	
	// Make list of just-fired prods
	unsigned int just_fired = 0;
	for ( preference *pref = goal->id.operator_slot->preferences[ NUMERIC_INDIFFERENT_PREFERENCE_TYPE ]; pref; pref = pref->next )
	{
		if ( ( op == pref->value ) && pref->inst->prod->rl_rule )
		{			
			if ( ( just_fired == 0 ) && !data->prev_op_rl_rules->empty() )
			{
				rl_clear_refs( goal );
			}
			
			rl_add_ref( goal, pref->inst->prod );
			just_fired++;			
		}
	}

	if ( just_fired )
	{		
		data->previous_q = cand->numeric_value;
	}
	else
	{
		if ( my_agent->sysparams[ TRACE_RL_SYSPARAM ] && using_gaps &&
			( data->gap_age == 0 ) && !data->prev_op_rl_rules->empty() )
		{			
			char buf[256];
			SNPRINTF( buf, 254, "gap started (%c%llu)", goal->id.name_letter, static_cast<long long unsigned>(goal->id.name_number) );
			
			print( my_agent, buf );
			xml_generate_warning( my_agent, buf );
		}
		
		if ( !using_gaps )
		{
			if ( !data->prev_op_rl_rules->empty() )
			{
				rl_clear_refs( goal );
			}			
			
			data->previous_q = cand->numeric_value;
		}
		else
		{		
			if ( !data->prev_op_rl_rules->empty() )
			{
				data->gap_age++;
			}
		}
	}
}

// performs the rl update at a state
void rl_perform_update( agent *my_agent, preference *cand, bool op_rl, Symbol *goal, bool update_efr ) ///< bazald
{
  double op_value = cand ? cand->numeric_value : 0.0;
	bool using_gaps = ( my_agent->rl_params->temporal_extension->get_value() == soar_module::on );

	if ( !using_gaps || op_rl )
	{		
		rl_data *data = goal->id.rl_info;
		
		if ( !data->prev_op_rl_rules->empty() )
		{			
			rl_et_map::iterator iter;			
			double alpha = my_agent->rl_params->learning_rate->get_value();
			double lambda = my_agent->rl_params->et_decay_rate->get_value();
			double gamma = my_agent->rl_params->discount_rate->get_value();
			double tolerance = my_agent->rl_params->et_tolerance->get_value();
            double theta = my_agent->rl_params->meta_learning_rate->get_value();

			// if temporal_discount is off, don't discount for gaps
			unsigned int effective_age = data->hrl_age + 1;
			if (my_agent->rl_params->temporal_discount->get_value() == soar_module::on) {
				effective_age += data->gap_age;
			}
 
			double discount = pow( gamma, static_cast< double >( effective_age ) );

			// notify of gap closure
			if ( data->gap_age && using_gaps && my_agent->sysparams[ TRACE_RL_SYSPARAM ] )
			{
				char buf[256];
				SNPRINTF( buf, 254, "gap ended (%c%llu)", goal->id.name_letter, static_cast<long long unsigned>(goal->id.name_number) );

				print( my_agent, buf );
				xml_generate_warning( my_agent, buf );
			}			

			// Iterate through eligibility_traces, decay traces. If less than TOLERANCE, remove from map.
			if ( lambda == 0 )
			{
				if ( !data->eligibility_traces->empty() )
				{
					data->eligibility_traces->clear();
				}
			}
			else
			{
				for ( iter = data->eligibility_traces->begin(); iter != data->eligibility_traces->end(); )
				{
					iter->second *= lambda;
					iter->second *= discount;
					if ( iter->second < tolerance ) 
					{
						data->eligibility_traces->erase( iter++ );
					}
					else 
					{
						++iter;
					}
				}
			}

      const double num_rules = double(data->prev_op_rl_rules->size()); ///< bazald

      /// Assign credit to different RL rules according to
      ///   even: previously only method, still the default - simply split credit evenly between RL rules
      ///   fc: firing counts - split by the inverse of how frequently each RL rule has fired
      ///   rl: RL update counts - split by the inverse of how frequently each Q-value (RL rule) has been updated
      ///   logrl: the same as 'rl', but the inverse of the log of the frequency -- should be sort of between rl and even
      std::map<production *, double> credit; ///< bazald
      if(my_agent->rl_params->credit_assignment->get_value() == rl_param_container::credit_logrl) {
        double total_credit = 0.0;
        for(rl_rule_list::iterator rt = data->prev_op_rl_rules->begin(), rend = data->prev_op_rl_rules->end(); rt != rend; ++rt)
          total_credit += 1.0 / (log((*rt)->rl_update_count + 1.0) + 1.0); ///< hasn't updated yet
        for(rl_rule_list::iterator rt = data->prev_op_rl_rules->begin(), rend = data->prev_op_rl_rules->end(); rt != rend; ++rt)
          credit[*rt] = (1.0 / (log((*rt)->rl_update_count + 1.0) + 1.0)) / total_credit;
      }
      else if(my_agent->rl_params->credit_assignment->get_value() == rl_param_container::credit_rl) {
        double total_credit = 0.0;
        for(rl_rule_list::iterator rt = data->prev_op_rl_rules->begin(), rend = data->prev_op_rl_rules->end(); rt != rend; ++rt)
          total_credit += 1.0 / ((*rt)->rl_update_count + 1.0); ///< hasn't updated yet
        for(rl_rule_list::iterator rt = data->prev_op_rl_rules->begin(), rend = data->prev_op_rl_rules->end(); rt != rend; ++rt)
          credit[*rt] = (1.0 / ((*rt)->rl_update_count + 1.0)) / total_credit;
      }
      else if(my_agent->rl_params->credit_assignment->get_value() == rl_param_container::credit_fc) {
        double total_credit = 0.0;
        for(rl_rule_list::iterator rt = data->prev_op_rl_rules->begin(), rend = data->prev_op_rl_rules->end(); rt != rend; ++rt)
          total_credit += (1.0 / (*rt)->firing_count); ///< has fired already
        for(rl_rule_list::iterator rt = data->prev_op_rl_rules->begin(), rend = data->prev_op_rl_rules->end(); rt != rend; ++rt)
          credit[*rt] = (1.0 / (*rt)->firing_count) / total_credit;
      }
      else if(my_agent->rl_params->credit_assignment->get_value() == rl_param_container::credit_even){
        const double value = 1.0 / num_rules;
        for(rl_rule_list::iterator rt = data->prev_op_rl_rules->begin(), rend = data->prev_op_rl_rules->end(); rt != rend; ++rt)
          credit[*rt] = value;
      }
      else
        abort();
      
			// Update trace for just fired prods
			double sum_old_ecr = 0.0;
			double sum_old_efr = 0.0;
			if ( !data->prev_op_rl_rules->empty() )
			{
				rl_rule_list::iterator p;
				
				for ( p=data->prev_op_rl_rules->begin(); p!=data->prev_op_rl_rules->end(); p++ )
				{
					sum_old_ecr += (*p)->rl_ecr;
					sum_old_efr += (*p)->rl_efr;
					
					iter = data->eligibility_traces->find( (*p) );
					
					if ( iter != data->eligibility_traces->end() ) 
					{
						iter->second += credit[*p]; ///< bazald
					}
					else 
					{
						(*data->eligibility_traces)[*p] = credit[*p]; ///< bazald
					}
				}
			}
      const double sum_old_combined = sum_old_ecr + sum_old_efr; ///< bazald
			
			// For each prod with a trace, perform update
			{
				double old_ecr, old_efr;
				double delta_ecr, delta_efr;
				double new_combined, new_ecr, new_efr;
                double delta_t = (data->reward + discount * op_value) - (sum_old_ecr + sum_old_efr);

        double rl_total_variance_next = 0.0; ///< bazald
        if(cand && cand->inst && cand->inst->prod) ///< bazald
        {
          if(cand->rl_contribution) {
            const production * const &prod2 = pref->inst->prod;
            rl_total_variance_next += prod2->rl_total_variance;
          }
        }

        double rl_total_variance = 0.0; ///< bazald
				for ( iter = data->eligibility_traces->begin(); iter != data->eligibility_traces->end(); iter++ )
				{	
					production *prod = iter->first;

					// get old vals
					old_ecr = prod->rl_ecr;
					old_efr = prod->rl_efr;
          const double old_combined = old_ecr + old_efr; ///< bazald

                    // Adjust alpha based on decay policy
                    // Miller 11/14/2011
                    double adjusted_alpha;
                    double adjusted_alpha_pp; ///< bazald
                    switch (my_agent->rl_params->decay_mode->get_value())
                    {
                        case rl_param_container::exponential_decay:
                            adjusted_alpha = 1.0 / (prod->rl_update_count + 1.0);
                            adjusted_alpha_pp = 1.0 / (prod->rl_update_count + 2.0); ///< bazald
                            break;
                        case rl_param_container::logarithmic_decay:
                            adjusted_alpha = 1.0 / (log(prod->rl_update_count + 1.0) + 1.0);
                            adjusted_alpha_pp = 1.0 / (log(prod->rl_update_count + 2.0) + 1.0); ///< bazald
                            break;
                        case rl_param_container::delta_bar_delta_decay:
                            {
                                // Note that in this case, x_i = 1.0 for all productions that are being updated.
                                // Those values have been included here for consistency with the algorithm as described in the delta bar delta paper.
                                prod->rl_delta_bar_delta_beta = prod->rl_delta_bar_delta_beta + theta * delta_t * 1.0 * prod->rl_delta_bar_delta_h;
                                adjusted_alpha = exp(prod->rl_delta_bar_delta_beta);
                                adjusted_alpha_pp = exp(prod->rl_delta_bar_delta_beta); ///< bazald TODO
                                double decay_term = 1.0 - adjusted_alpha * 1.0 * 1.0;
                                if (decay_term < 0.0) decay_term = 0.0;
                                prod->rl_delta_bar_delta_h = prod->rl_delta_bar_delta_h * decay_term + adjusted_alpha * delta_t * 1.0;
                                break;
                            }
                        case rl_param_container::normal_decay:
                        default:
                            adjusted_alpha = alpha;
                            adjusted_alpha_pp = 1.0 / ((1.0 / alpha) + 1.0); ///< bazald
                            break;
                    }

                    // calculate updates
                    delta_ecr = ( adjusted_alpha * iter->second * ( data->reward - sum_old_ecr ) );

                    if ( update_efr )
                    {
                        delta_efr = ( adjusted_alpha * iter->second * ( ( discount * op_value ) - sum_old_efr ) );
                    }
                    else
					{
						delta_efr = 0.0;
					}					
					const double delta_combined = delta_ecr + delta_efr; ///< bazald

					// calculate new vals
					new_ecr = ( old_ecr + delta_ecr );
					new_efr = ( old_efr + delta_efr );
					new_combined = ( new_ecr + new_efr );
					
					// print as necessary
					if ( my_agent->sysparams[ TRACE_RL_SYSPARAM ] ) 
					{
						std::ostringstream ss;						
						ss << "RL update " << prod->name->sc.name << " "
						   << old_ecr << " " << old_efr << " " << old_ecr + old_efr << " -> "
						   << new_ecr << " " << new_efr << " " << new_combined ;

						std::string temp_str( ss.str() );						
						print( my_agent, "%s\n", temp_str.c_str() );
						xml_generate_message( my_agent, temp_str.c_str() );

                        // Log update to file if the log file has been set
                        std::string log_path = my_agent->rl_params->update_log_path->get_value();
                        if (!log_path.empty()) {
                            std::ofstream file(log_path.c_str(), std::ios_base::app);
                            file << ss.str() << std::endl;
                            file.close();
                        }
                    }

                    // Change value of rule
                    symbol_remove_ref( my_agent, rhs_value_to_symbol( prod->action_list->referent ) );
                    prod->action_list->referent = symbol_to_rhs_value( make_float_constant( my_agent, new_combined ) );
                    prod->rl_update_count += 1;
                    prod->rl_ecr = new_ecr;
                    prod->rl_efr = new_efr;

          { /// bazald: Variance calculation
            // def online_variance(data): Thanks to Welford, Knuth's Art of Computer Programming
            //     n = 0
            //     mean = 0
            //     M2 = 0
            //  
            //     for x in data:
            //         n = n + 1
            //         delta = x - mean
            //         mean = mean + delta/n
            //         if n > 1:
            //             M2 = M2 + delta*(x - mean)
            //  
            //     variance_n = M2/n
            //     variance = M2/(n - 1)
            //     return (variance, variance_n)

            const double old_mean2 = prod->rl_mean2;
            const double old_sample_variance = prod->rl_sample_variance;

            if(prod->rl_update_count > 1) {
              prod->rl_mean2 += adjusted_alpha * iter->second * delta_ecr * (data->reward - prod->rl_ecr);
              prod->rl_sample_variance = prod->rl_mean2 / (prod->rl_update_count - 1);

              assert(adjusted_alpha * iter->second <= 1.0);

              prod->rl_partial_variance += adjusted_alpha * iter->second * (discount * rl_total_variance_next - prod->rl_partial_variance);
              rl_total_variance += prod->rl_sample_variance + prod->rl_partial_variance;
            }

//             std::cerr << " V   " << prod->name->sc.name << " = " << prod->rl_sample_variance << '|' << prod->rl_variance_nonincrease_count << " from (" << prod->rl_update_count << ", " << old_combined << ", " << new_combined << ", " << old_sample_variance << ')' << std::endl;
          }

                    // change documentation
                    if ( my_agent->rl_params->meta->get_value() == soar_module::on )
                    {
                        if ( prod->documentation )
                        {
                            free_memory_block_for_string( my_agent, prod->documentation );
                        }
                        std::stringstream doc_ss;
                        const std::vector<std::pair<std::string, param_accessor<double> *> > &documentation_params = my_agent->rl_params->get_documentation_params();
                        for (std::vector<std::pair<std::string, param_accessor<double> *> >::const_iterator doc_params_it = documentation_params.begin();
                                doc_params_it != documentation_params.end(); ++doc_params_it) {
                            doc_ss << doc_params_it->first << "=" << doc_params_it->second->get_param(prod) << ";";
                        }
                        prod->documentation = make_memory_block_for_string(my_agent, doc_ss.str().c_str());

                        /*
						std::string rlupdates( "rlupdates=" );
						std::string val;
						to_string( static_cast< uint64_t >( prod->rl_update_count ), val );
						rlupdates.append( val );

						prod->documentation = make_memory_block_for_string( my_agent, rlupdates.c_str() );
                        */
					}

					// Change value of preferences generated by current instantiations of this rule
					if ( prod->instantiations )
					{
						for ( instantiation *inst = prod->instantiations; inst; inst = inst->next )
						{
							for ( preference *pref = inst->preferences_generated; pref; pref = pref->inst_next )
							{
								symbol_remove_ref( my_agent, pref->referent );
								pref->referent = make_float_constant( my_agent, new_combined );
							}
						}
					}	
				}

        for(iter = data->eligibility_traces->begin(); iter != data->eligibility_traces->end(); iter++) ///< bazald
        { 
          production *prod = iter->first;

          // Adjust alpha based on decay policy
          // Miller 11/14/2011
          double adjusted_alpha;
          switch (my_agent->rl_params->decay_mode->get_value())
          {
              case rl_param_container::exponential_decay:
                  adjusted_alpha = 1.0 / (prod->rl_update_count + 1.0);
                  break;
              case rl_param_container::logarithmic_decay:
                  adjusted_alpha = 1.0 / (log(prod->rl_update_count + 1.0) + 1.0);
                  break;
              case rl_param_container::normal_decay:
              default:
                  adjusted_alpha = alpha;
                  break;
          }

          prod->rl_total_variance += adjusted_alpha * iter->second * (rl_total_variance / num_rules - prod->rl_total_variance);
//           std::cerr << "     " << prod->name->sc.name << " takes " << prod->rl_total_variance << " from rl_total_variance" << std::endl;
        }
			}
		}

		data->gap_age = 0;
		data->hrl_age = 0;
		data->reward = 0.0;
	}
}

// clears eligibility traces 
void rl_watkins_clear( agent * /*my_agent*/, Symbol *goal )
{
	goal->id.rl_info->eligibility_traces->clear();
}
