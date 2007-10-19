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
#include <math.h>
#include <float.h>

#include "soar_rand.h"
#include "agent.h"
#include "print.h"
#include "gski_event_system_functions.h"
#include "xmlTraceNames.h"

#include "exploration.h"
#include "misc.h"
#include "reinforcement_learning.h"

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
	if ( !strcmp( policy_name, "softmax" ) )
		return USER_SELECT_SOFTMAX;
	
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
	if ( policy == USER_SELECT_SOFTMAX )
		return "softmax";
	
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
	newbie->rates[ EXPLORATION_EXPONENTIAL ] = 1;
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
 * Function     : update_exploration_parameters
 **************************************************************************/
void update_exploration_parameters( agent *my_agent )
{	
	std::vector<std::string> *parameters = get_parameter_names( my_agent );

	for ( std::vector<std::string>::size_type i=0; i<parameters->size(); i++ )
	{
		const char *parameter_name = (*parameters)[ i ].c_str();
		const long reduction_policy = get_reduction_policy( my_agent, parameter_name );
		double reduction_rate = get_reduction_rate( my_agent, parameter_name, reduction_policy );
		double current_value = get_parameter_value( my_agent, parameter_name );

		if ( reduction_policy == EXPLORATION_EXPONENTIAL )
		{
			if ( reduction_rate != 1 )
				set_parameter_value( my_agent, parameter_name, ( current_value * reduction_rate ) );
		}
		else if ( reduction_policy == EXPLORATION_LINEAR )
		{
			if ( ( current_value > 0 ) && ( reduction_rate != 0 ) )
				set_parameter_value( my_agent, parameter_name, ( ( ( current_value - reduction_rate ) > 0 )?( current_value - reduction_rate ):( 0 ) ) );
		}
	}

	delete parameters;
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

/***************************************************************************
 * Function     : choose_according_to_exploration_mode
 **************************************************************************/
preference *choose_according_to_exploration_mode( agent *my_agent, slot *s, preference *candidates )
{
	float top_value = candidates->numeric_value;
	const long exploration_policy = get_exploration_policy( my_agent );
	preference *return_val;

	// get preference values for each candidate
	for ( preference *cand = candidates; cand != NIL; cand = cand->next_candidate )
		compute_value_of_candidate( my_agent, cand, s );

	// should find highest valued candidate in q-learning
	if ( soar_rl_enabled( my_agent ) && ( get_rl_parameter( my_agent, "learning-policy", RL_RETURN_LONG ) == RL_LEARNING_Q ) )
		for ( preference *cand=candidates; cand!=NIL; cand=cand->next_candidate )
			if ( cand->numeric_value > top_value )
				top_value = cand->numeric_value;
	
	switch ( exploration_policy )
	{
		case USER_SELECT_FIRST:
			return_val = candidates;
			break;
		
		case USER_SELECT_LAST:
			for (return_val = candidates; return_val->next_candidate != NIL; return_val = return_val->next_candidate);
			break;

		case USER_SELECT_RANDOM:
			return_val = randomly_select( candidates );
			break;

		case USER_SELECT_SOFTMAX:
			return_val = probabilistically_select( candidates );
			break;

		case USER_SELECT_E_GREEDY:
			return_val = epsilon_greedy_select( my_agent, candidates );
			break;

		case USER_SELECT_BOLTZMANN:
			return_val = boltzmann_select( my_agent, candidates );
			break;
	}

	// should perform update here for chosen candidate in sarsa
	if ( soar_rl_enabled( my_agent ) && ( get_rl_parameter( my_agent, "learning-policy", RL_RETURN_LONG ) == RL_LEARNING_SARSA ) )
		perform_rl_update( my_agent, return_val->numeric_value, s->id );
	else if ( soar_rl_enabled( my_agent ) && ( get_rl_parameter( my_agent, "learning-policy", RL_RETURN_LONG ) == RL_LEARNING_Q ) )
	{
		if ( return_val->numeric_value != top_value )
			watkins_clear( my_agent, s->id );

		perform_rl_update( my_agent, top_value, s->id );
	}
	
	return return_val;
}

/***************************************************************************
 * Function     : randomly_select
 **************************************************************************/
preference *randomly_select( preference *candidates )
{
	unsigned int cand_count = 0;
	unsigned int chosen_num = 0;
	preference *cand;
	
	// select at random 
	for ( cand = candidates; cand != NIL; cand = cand->next_candidate )
		cand_count++;

	chosen_num = SoarRandInt( cand_count - 1 );

	cand = candidates;
	while ( chosen_num ) 
	{ 
		cand = cand->next_candidate; 
		chosen_num--; 
	}

	return cand;
}

/***************************************************************************
 * Function     : probabilistically_select
 **************************************************************************/
preference *probabilistically_select( preference *candidates )
{	
	preference *cand = 0;
	double total_probability = 0;
	double selected_probability = 0;
	double current_sum = 0;
	double rn = 0;

	bool good_prefs = true;

	// shouldn't support non-positive numbers
	for ( cand = candidates; cand != NIL; cand = cand->next_candidate )
	{
		if ( cand->numeric_value <= 0 )
		{
			good_prefs = false;
			break;
		}
		else
			total_probability += cand->numeric_value;
	}
	
	if ( !good_prefs )
		return randomly_select( candidates );
	
	rn = SoarRand();
	selected_probability = rn * total_probability;
	current_sum = 0;

	for ( cand = candidates; cand != NIL; cand = cand->next_candidate ) 
	{
		current_sum += cand->numeric_value;
		if ( selected_probability <= current_sum )
			return cand;
	}

	return NIL;
}

/***************************************************************************
 * Function     : boltzmann_select
 **************************************************************************/
preference *boltzmann_select( agent *my_agent, preference *candidates )
{
	preference *cand = 0;
	double total_probability = 0;
	double selected_probability = 0;
	double rn = 0;
	double current_sum = 0;

	double temp = get_parameter_value( my_agent, "temperature" );
	
	// output trace information
	if ( my_agent->sysparams[ TRACE_INDIFFERENT_SYSPARAM ] )
	{
		for ( cand = candidates; cand != NIL; cand = cand->next_candidate )
		{
			print_with_symbols( my_agent, "\n Candidate %y:  ", cand->value );
			print( my_agent, "Value (Sum) = %f, (Exp) = %f", cand->numeric_value, exp( cand->numeric_value / temp ) );
			gSKI_MakeAgentCallbackXML( my_agent, kFunctionBeginTag, kTagCandidate );
			gSKI_MakeAgentCallbackXML( my_agent, kFunctionAddAttribute, kCandidateName, symbol_to_string( my_agent, cand->value, true, 0, 0 ) );
			gSKI_MakeAgentCallbackXML( my_agent, kFunctionAddAttribute, kCandidateType, kCandidateTypeSum );
			gSKI_MakeAgentCallbackXML( my_agent, kFunctionAddAttribute, kCandidateValue, cand->numeric_value );
			gSKI_MakeAgentCallbackXML( my_agent, kFunctionAddAttribute, kCandidateExpValue, exp( cand->numeric_value / temp ) );
			gSKI_MakeAgentCallbackXML( my_agent, kFunctionEndTag, kTagCandidate );
		}
	}

	/**
	 * Since we can't guarantee any combination of temperature/q-values, could be useful
	 * to notify the user if double limit has been breached.
	 */
	double exp_max = log( DBL_MAX );
	double q_max = exp_max * temp;

	/*
	 * method to increase usable range of boltzmann with double
	 * - find the highest/lowest q-values
	 * - take half the difference
	 * - subtract this value from all q-values when making calculations
	 * 
	 * this maintains relative probabilities of selection, while reducing greatly the exponential extremes of calculations
	 */
	float q_diff = 0;
	if ( candidates->next_candidate != NIL ) 
	{
		float q_high = candidates->numeric_value;
		float q_low = candidates->numeric_value;
		
		for ( cand = candidates->next_candidate; cand != NIL; cand = cand->next_candidate ) 
		{
			if ( cand->numeric_value > q_high )
				q_high = cand->numeric_value;
			if ( cand->numeric_value < q_low )
				q_low = cand->numeric_value;
		}

		q_diff = ( q_high - q_low ) / 2;
	} 
	else 
	{
		q_diff = candidates->numeric_value;
	}

	for (cand = candidates; cand != NIL; cand = cand->next_candidate) 
	{

		/*  Total Probability represents the range of values, we expect
		 *  the use of negative valued preferences, so its possible the
		 *  sum is negative, here that means a fractional probability
		 */
		float q_val = ( cand->numeric_value - q_diff );
		total_probability += exp( (double) (  q_val / temp ) );
		
		/**
 		 * Let user know if adjusted q-value will overflow
		 */
		if ( q_val > q_max )
		{
			print( my_agent, "WARNING: Boltzmann update overflow! %g > %g", q_val, q_max );
			
			char buf[256];
       		SNPRINTF( buf, 254, "WARNING: Boltzmann update overflow! %g > %g", q_val, q_max );
       		gSKI_MakeAgentCallbackXML( my_agent, kFunctionBeginTag, kTagWarning );
       		gSKI_MakeAgentCallbackXML( my_agent, kFunctionAddAttribute, kTypeString, buf );
       		gSKI_MakeAgentCallbackXML( my_agent, kFunctionEndTag, kTagWarning );
		}
	}

	rn = SoarRand(); // generates a number in [0,1]
	selected_probability = rn * total_probability;

	for (cand = candidates; cand != NIL; cand = cand->next_candidate) 
	{
		current_sum += exp( (double) ( ( cand->numeric_value - q_diff ) / temp ) );
		
		if ( selected_probability <= current_sum )
			return cand;
	}
	
	return NIL;
}

/***************************************************************************
 * Function     : epsilon_greedy_select
 **************************************************************************/
preference *epsilon_greedy_select( agent *my_agent, preference *candidates )
{
	preference *cand = 0;

	double epsilon = get_parameter_value( my_agent, "epsilon" );

	if ( my_agent->sysparams[ TRACE_INDIFFERENT_SYSPARAM ] )
	{
		for ( cand = candidates; cand != NIL; cand = cand->next_candidate )
		{
			print_with_symbols( my_agent, "\n Candidate %y:  ", cand->value );
			print( my_agent, "Value (Sum) = %f", cand->numeric_value );
			gSKI_MakeAgentCallbackXML( my_agent, kFunctionBeginTag, kTagCandidate );
			gSKI_MakeAgentCallbackXML( my_agent, kFunctionAddAttribute, kCandidateName, symbol_to_string( my_agent, cand->value, true, 0, 0 ) );
			gSKI_MakeAgentCallbackXML( my_agent, kFunctionAddAttribute, kCandidateType, kCandidateTypeSum );
			gSKI_MakeAgentCallbackXML( my_agent, kFunctionAddAttribute, kCandidateValue, cand->numeric_value );
			gSKI_MakeAgentCallbackXML( my_agent, kFunctionEndTag, kTagCandidate );
		}
	}

	if ( SoarRand() < epsilon )	
		return randomly_select( candidates );
	else
		return get_highest_q_value_pref( candidates );
	
	return NIL;
}

/***************************************************************************
 * Function     : get_highest_q_value_pref
 **************************************************************************/
preference *get_highest_q_value_pref( preference *candidates )
{
	preference *cand;
	preference *top_cand = candidates;
	float top_value = candidates->numeric_value;
	int num_max_cand = 0;

	for ( cand=candidates; cand!=NIL; cand=cand->next_candidate )
	{
		if ( cand->numeric_value > top_value ) 
		{
			top_value = cand->numeric_value;
			top_cand = cand;
			num_max_cand = 1;
		} 
		else if ( cand->numeric_value == top_value ) 
			num_max_cand++;
	}

	if ( num_max_cand == 1 )	
		return top_cand;
	else 
	{
		// if operators tied for highest Q-value, select among tied set at random
		unsigned int chosen_num = SoarRandInt( num_max_cand - 1 );
		
		cand = candidates;
		while ( cand->numeric_value != top_value ) 
			cand = cand->next_candidate;
		
		while ( chosen_num ) 
		{
			cand = cand->next_candidate;
			chosen_num--;
			
			while ( cand->numeric_value != top_value ) 
				cand = cand->next_candidate;
		}
		
		return cand;
	}
}

/***************************************************************************
 * Function     : compute_value_of_candidate
 **************************************************************************/
void compute_value_of_candidate( agent *my_agent, preference *cand, slot *s, float default_value )
{
	if ( !cand ) return;

	// temporary runner
	preference *pref;

	// initialize candidate values
	cand->total_preferences_for_candidate = 0;
	cand->numeric_value = 0;
	
	// all numeric indifferents
	for ( pref = s->preferences[ NUMERIC_INDIFFERENT_PREFERENCE_TYPE ]; pref != NIL; pref = pref->next) 
	{
		if ( cand->value == pref->value )
		{
			cand->total_preferences_for_candidate += 1;
			cand->numeric_value += get_number_from_symbol( pref->referent );
		}
	}

	// all binary indifferents
	for ( pref = s->preferences[ BINARY_INDIFFERENT_PREFERENCE_TYPE ]; pref != NIL; pref = pref->next ) 
	{
		if (cand->value == pref->value)
		{
			cand->total_preferences_for_candidate += 1;
			cand->numeric_value += get_number_from_symbol( pref->referent );
        }
	}
	
	// if no contributors, provide default
	if ( cand->total_preferences_for_candidate == 0 ) 
	{
		cand->numeric_value = default_value;
		cand->total_preferences_for_candidate = 1;
	}
	
	// accomodate average mode
	if ( my_agent->numeric_indifferent_mode == NUMERIC_INDIFFERENT_MODE_AVG )
		cand->numeric_value = cand->numeric_value / cand->total_preferences_for_candidate;
}
