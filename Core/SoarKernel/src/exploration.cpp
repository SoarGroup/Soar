#include <portability.h>

/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
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

#include "exploration.h"

#include <stdlib.h>
#include <math.h>
#include <float.h>

#include "soar_rand.h"
#include "xml.h"
#include "print.h"
#include "soar_TraceNames.h"

#include "reinforcement_learning.h"
#include "misc.h"
#include "utilities.h"
#include "instantiations.h"

using namespace soar_TraceNames;

/***************************************************************************
 * Function     : exploration_valid_policy
 **************************************************************************/

bool exploration_valid_policy( const char *policy_name )
{
	return ( exploration_convert_policy( policy_name ) != 0 );
}

bool exploration_valid_policy( const long policy )
{
	return ( ( policy > 0 ) && ( policy < USER_SELECT_INVALID ) );
}

/***************************************************************************
 * Function     : exploration_convert_policy
 **************************************************************************/
const long exploration_convert_policy( const char *policy_name )
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

const char *exploration_convert_policy( const long policy )
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
 * Function     : exploration_set_policy
 **************************************************************************/
bool exploration_set_policy( agent *my_agent, const char *policy_name )
{	
	const long policy = exploration_convert_policy( policy_name );
	
	if ( policy != 0 )
		return exploration_set_policy( my_agent, policy );
	
	return false;
}

bool exploration_set_policy( agent *my_agent, const long policy )
{	
	if ( exploration_valid_policy( policy ) )
	{
		set_sysparam( my_agent, USER_SELECT_MODE_SYSPARAM, policy );
		return true;
	}
	
	return false;
}

/***************************************************************************
 * Function     : exploration_get_policy
 **************************************************************************/
const long exploration_get_policy( agent *my_agent )
{
	return my_agent->sysparams[ USER_SELECT_MODE_SYSPARAM ];
}

/***************************************************************************
 * Function     : exploration_add_parameter
 **************************************************************************/
exploration_parameter *exploration_add_parameter( double value, bool (*val_func)( double ), const char *name )
{
	// new parameter entry
	exploration_parameter *newbie = new exploration_parameter;
	newbie->value = value;
	newbie->name = name;
	newbie->reduction_policy = EXPLORATION_REDUCTION_EXPONENTIAL;
	newbie->val_func = val_func;
	newbie->rates[ EXPLORATION_REDUCTION_EXPONENTIAL ] = 1;
	newbie->rates[ EXPLORATION_REDUCTION_LINEAR ] = 0;
	
	return newbie;
}

/***************************************************************************
 * Function     : exploration_convert_parameter
 **************************************************************************/
const long exploration_convert_parameter( agent *my_agent, const char *name )
{
	for ( int i=0; i<EXPLORATION_PARAMS; i++ )
		if ( !strcmp( name, my_agent->exploration_params[ i ]->name ) )
			return i;
	
	return EXPLORATION_PARAMS;
}

const char *exploration_convert_parameter( agent *my_agent, const long parameter )
{
	return ( ( ( parameter >= 0 ) && ( parameter < EXPLORATION_PARAMS ) )?( my_agent->exploration_params[ parameter ]->name ):( NULL ) );
}

/***************************************************************************
 * Function     : exploration_valid_parameter
 **************************************************************************/
const bool exploration_valid_parameter( agent *my_agent, const char *name )
{
	return ( exploration_convert_parameter( my_agent, name ) != EXPLORATION_PARAMS );
}

const bool exploration_valid_parameter( agent *my_agent, const long parameter )
{
	return ( exploration_convert_parameter( my_agent, parameter ) != NULL );
}

/***************************************************************************
 * Function     : exploration_get_parameter_value
 **************************************************************************/
double exploration_get_parameter_value( agent *my_agent, const char *parameter )
{	
	const long param = exploration_convert_parameter( my_agent, parameter );
	if ( param == EXPLORATION_PARAMS )
		return 0;
	
	return my_agent->exploration_params[ param ]->value;
}

double exploration_get_parameter_value( agent *my_agent, const long parameter )
{
	if ( exploration_valid_parameter( my_agent, parameter ) )
		return my_agent->exploration_params[ parameter ]->value;

	return 0;
}

/***************************************************************************
 * Function     : exploration_validate_epsilon
 **************************************************************************/
bool exploration_validate_epsilon( double value )
{
	return ( ( value >= 0 ) && ( value <= 1 ) );
}

/***************************************************************************
 * Function     : exploration_validate_temperature
 **************************************************************************/
bool exploration_validate_temperature( double value )
{
	return ( value > 0 );
}

/***************************************************************************
 * Function     : exploration_valid_parameter_value
 **************************************************************************/
bool exploration_valid_parameter_value( agent *my_agent, const char *name, double value )
{
	const long param = exploration_convert_parameter( my_agent, name );
	if ( param == EXPLORATION_PARAMS )
		return false;
	
	return my_agent->exploration_params[ param ]->val_func( value );
}

bool exploration_valid_parameter_value( agent *my_agent, const long parameter, double value )
{
	if ( exploration_valid_parameter( my_agent, parameter ) )
		return my_agent->exploration_params[ parameter ]->val_func( value );

	return false;
}

/***************************************************************************
 * Function     : exploration_set_parameter_value
 **************************************************************************/
bool exploration_set_parameter_value( agent *my_agent, const char *name, double value )
{
	const long param = exploration_convert_parameter( my_agent, name );
	if ( param == EXPLORATION_PARAMS )
		return false;
	
	my_agent->exploration_params[ param ]->value = value;
	
	return true;
}

bool exploration_set_parameter_value( agent *my_agent, const long parameter, double value )
{
	if ( exploration_valid_parameter( my_agent, parameter ) )
	{
		my_agent->exploration_params[ parameter ]->value = value;
		return true;
	}
	else
		return false;
}

/***************************************************************************
 * Function     : exploration_get_auto_update
 **************************************************************************/
bool exploration_get_auto_update( agent *my_agent )
{
	return ( my_agent->sysparams[ USER_SELECT_REDUCE_SYSPARAM ] != FALSE );
}

/***************************************************************************
 * Function     : exploration_set_auto_update
 **************************************************************************/
bool exploration_set_auto_update( agent *my_agent, bool setting )
{
	my_agent->sysparams[ USER_SELECT_REDUCE_SYSPARAM ] = ( ( setting )?( TRUE ):( FALSE ) );
	
	return true;
}

/***************************************************************************
 * Function     : exploration_update_parameters
 **************************************************************************/
void exploration_update_parameters( agent *my_agent )
{	
	if ( exploration_get_auto_update( my_agent ) )
	{			
		for ( int i=0; i<EXPLORATION_PARAMS; i++ )
		{
			const long reduction_policy = exploration_get_reduction_policy( my_agent, i );
			double reduction_rate = exploration_get_reduction_rate( my_agent, i, reduction_policy );			
	
			if ( reduction_policy == EXPLORATION_REDUCTION_EXPONENTIAL )
			{
				if ( reduction_rate != 1 )
				{
					double current_value = exploration_get_parameter_value( my_agent, i );

					exploration_set_parameter_value( my_agent, i, ( current_value * reduction_rate ) );
				}
			}
			else if ( reduction_policy == EXPLORATION_REDUCTION_LINEAR )
			{
				double current_value = exploration_get_parameter_value( my_agent, i );
				
				if ( ( current_value > 0 ) && ( reduction_rate != 0 ) )
					exploration_set_parameter_value( my_agent, i, ( ( ( current_value - reduction_rate ) > 0 )?( current_value - reduction_rate ):( 0 ) ) );
			}
		}
	}
}

/***************************************************************************
 * Function     : exploration_convert_reduction_policy
 **************************************************************************/
const long exploration_convert_reduction_policy( const char *policy_name )
{
	if ( !strcmp( policy_name, "exponential" ) )
		return EXPLORATION_REDUCTION_EXPONENTIAL;
	if ( !strcmp( policy_name, "linear" ) )
		return EXPLORATION_REDUCTION_LINEAR;
	
	return EXPLORATION_REDUCTIONS;
}

const char *exploration_convert_reduction_policy( const long policy )
{
	if ( policy == EXPLORATION_REDUCTION_EXPONENTIAL )
		return "exponential";
	if ( policy == EXPLORATION_REDUCTION_LINEAR )
		return "linear";
	
	return NULL;
}

/***************************************************************************
 * Function     : exploration_get_reduction_policy
 **************************************************************************/
const long exploration_get_reduction_policy( agent *my_agent, const char *parameter )
{
	const long param = exploration_convert_parameter( my_agent, parameter );
	if ( param == EXPLORATION_PARAMS )
		return EXPLORATION_REDUCTIONS;

	return my_agent->exploration_params[ param ]->reduction_policy;
}

const long exploration_get_reduction_policy( agent *my_agent, const long parameter )
{
	if ( exploration_valid_parameter( my_agent, parameter ) )
		return my_agent->exploration_params[ parameter ]->reduction_policy;
	else
		return EXPLORATION_REDUCTIONS;
}

/***************************************************************************
 * Function     : exploration_valid_reduction_policy
 **************************************************************************/
bool exploration_valid_reduction_policy( agent * /*my_agent*/, const char * /*parameter*/, const char *policy_name )
{	
	return ( exploration_convert_reduction_policy( policy_name ) != EXPLORATION_REDUCTIONS );
}

bool exploration_valid_reduction_policy( agent * /*my_agent*/, const char * /*parameter*/, const long policy )
{	
	return ( exploration_convert_reduction_policy( policy ) != NULL );
}

bool exploration_valid_reduction_policy( agent * /*my_agent*/, const long /*parameter*/, const long policy )
{	
	return ( exploration_convert_reduction_policy( policy ) != NULL );
}

/***************************************************************************
 * Function     : exploration_set_reduction_policy
 **************************************************************************/
bool exploration_set_reduction_policy( agent *my_agent, const char *parameter, const char *policy_name )
{
	const long param = exploration_convert_parameter( my_agent, parameter );
	if ( param == EXPLORATION_PARAMS )
		return false;
	
	const long policy = exploration_convert_reduction_policy( policy_name );
	if ( policy == EXPLORATION_REDUCTIONS )
		return false;		
	
	my_agent->exploration_params[ param ]->reduction_policy = policy;
	
	return true;
}

bool exploration_set_reduction_policy( agent *my_agent, const long parameter, const long policy )
{
	if ( exploration_valid_parameter( my_agent, parameter ) &&
		 exploration_valid_reduction_policy( my_agent, parameter, policy ) )
	{
		my_agent->exploration_params[ parameter ]->reduction_policy = policy;
		return true;
	}

	return false;
}

/***************************************************************************
 * Function     : exploration_valid_reduction_rate
 **************************************************************************/
bool exploration_valid_reduction_rate( agent *my_agent, const char *parameter, const char *policy_name, double reduction_rate )
{
	const long param = exploration_convert_parameter( my_agent, parameter );
	if ( param == EXPLORATION_PARAMS )
		return false;
	
	const long policy = exploration_convert_reduction_policy( policy_name );
	if ( policy == EXPLORATION_REDUCTIONS )
		return false;
	
	return exploration_valid_reduction_rate( my_agent, param, policy, reduction_rate );
}

bool exploration_valid_reduction_rate( agent *my_agent, const long parameter, const long policy, double reduction_rate )
{
	if ( !exploration_valid_reduction_policy( my_agent, parameter, policy ) )
		return false;
	
	switch ( policy )
	{
		case EXPLORATION_REDUCTION_EXPONENTIAL:
			return exploration_valid_exponential( reduction_rate );
			break;
			
		case EXPLORATION_REDUCTION_LINEAR:
			return exploration_valid_linear( reduction_rate );
			break;
			
		default:
			break;
	}
	return false;
}

/***************************************************************************
 * Function     : exploration_valid_exponential
 **************************************************************************/
bool exploration_valid_exponential( double reduction_rate )
{
	return ( ( reduction_rate >= 0 ) && ( reduction_rate <= 1 ) );
}

/***************************************************************************
 * Function     : exploration_valid_linear
 **************************************************************************/
bool exploration_valid_linear( double reduction_rate )
{
	return ( reduction_rate >= 0 );
}

/***************************************************************************
 * Function     : exploration_get_reduction_rate
 **************************************************************************/
double exploration_get_reduction_rate( agent *my_agent, const char *parameter, const char *policy_name )
{
	const long param = exploration_convert_parameter( my_agent, parameter );
	if ( param == EXPLORATION_PARAMS )
		return 0;
	
	const long policy = exploration_convert_reduction_policy( policy_name );
	if ( policy == EXPLORATION_REDUCTIONS )
		return 0;
	
	return exploration_get_reduction_rate( my_agent, param, policy );
}

double exploration_get_reduction_rate( agent *my_agent, const long parameter, const long policy )
{	
	if ( exploration_valid_parameter( my_agent, parameter ) &&
		 exploration_valid_reduction_policy( my_agent, parameter, policy ) )
		return my_agent->exploration_params[ parameter ]->rates[ policy ];
	
	return 0;
}

/***************************************************************************
 * Function     : exploration_set_reduction_rate
 **************************************************************************/
bool exploration_set_reduction_rate( agent *my_agent, const char *parameter, const char *policy_name, double reduction_rate )
{
	const long param = exploration_convert_parameter( my_agent, parameter );
	if ( param == EXPLORATION_PARAMS )
		return false;
	
	const long policy = exploration_convert_reduction_policy( policy_name );
	if ( policy == EXPLORATION_REDUCTIONS )
		return false;
	
	return exploration_set_reduction_rate( my_agent, param, policy, reduction_rate );
}

bool exploration_set_reduction_rate( agent *my_agent, const long parameter, const long policy, double reduction_rate )
{
	if ( exploration_valid_parameter( my_agent, parameter ) &&
		 exploration_valid_reduction_policy( my_agent, parameter, policy ) &&
		 exploration_valid_reduction_rate( my_agent, parameter, policy, reduction_rate ) )
	{
		my_agent->exploration_params[ parameter ]->rates[ policy ] = reduction_rate;
		return true;
	}
			
	return false;
}

/***************************************************************************
 * Function     : exploration_choose_according_to_policy
 **************************************************************************/
preference *exploration_choose_according_to_policy( agent *my_agent, slot *s, preference *candidates )
{	
	const long exploration_policy = exploration_get_policy( my_agent );
	preference *return_val = NULL;

	bool my_rl_enabled = rl_enabled( my_agent );

	/// Initialization to eliminate warning
	rl_param_container::learning_choices my_learning_policy = rl_param_container::q;
	if ( my_rl_enabled )
	{
		my_learning_policy = my_agent->rl_params->learning_policy->get_value();
	}

	// get preference values for each candidate
	for ( preference *cand = candidates; cand != NIL; cand = cand->next_candidate )
		exploration_compute_value_of_candidate( my_agent, cand, s );

	double top_value = candidates->numeric_value;
	bool top_rl = candidates->rl_contribution;

	// should find highest valued candidate in q-learning
	if ( my_rl_enabled && ( my_learning_policy == rl_param_container::q ) )
	{
		for ( preference *cand=candidates; cand!=NIL; cand=cand->next_candidate )
		{
			if ( cand->numeric_value > top_value )
			{
				top_value = cand->numeric_value;
				top_rl = cand->rl_contribution;
			}
		}
	}
	
	switch ( exploration_policy )
	{
		case USER_SELECT_FIRST:
			return_val = candidates;
			break;
		
		case USER_SELECT_LAST:
			for (return_val = candidates; return_val->next_candidate != NIL; return_val = return_val->next_candidate);
			break;

		case USER_SELECT_RANDOM:
			return_val = exploration_randomly_select( candidates );
			break;

		case USER_SELECT_SOFTMAX:
			return_val = exploration_probabilistically_select( candidates );
			break;

		case USER_SELECT_E_GREEDY:
			return_val = exploration_epsilon_greedy_select( my_agent, candidates );
			break;

		case USER_SELECT_BOLTZMANN:
			return_val = exploration_boltzmann_select( my_agent, candidates );
			break;
	}

	// should perform update here for chosen candidate in sarsa	
	if ( my_rl_enabled )
	{
		rl_tabulate_reward_values( my_agent );

		if ( my_learning_policy == rl_param_container::sarsa )
		{
			rl_perform_update( my_agent, return_val->numeric_value, return_val->rl_contribution, s->id );
		}
		else if ( my_learning_policy == rl_param_container::q )
		{
			if ( return_val->numeric_value != top_value )
			{
				rl_watkins_clear( my_agent, s->id );
			}

			rl_perform_update( my_agent, top_value, top_rl, s->id );
		}
	}
	
	return return_val;
}

/***************************************************************************
 * Function     : exploration_randomly_select
 **************************************************************************/
preference *exploration_randomly_select( preference *candidates )
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
 * Function     : exploration_probabilistically_select
 **************************************************************************/
preference *exploration_probabilistically_select( preference *candidates )
{	
	preference *cand = 0;
	double total_probability = 0;
	double selected_probability = 0;
	double current_sum = 0;
	double rn = 0;

	// count up positive numbers
	for ( cand = candidates; cand != NIL; cand = cand->next_candidate )
		if ( cand->numeric_value > 0 )
			total_probability += cand->numeric_value;
	
	// if nothing positive, resort to random
	if ( total_probability == 0 )
		return exploration_randomly_select( candidates );
	
	// choose a random preference within the distribution
	rn = SoarRand();
	selected_probability = rn * total_probability;
	current_sum = 0;

	// select the candidate based upon the chosen preference
	for ( cand = candidates; cand != NIL; cand = cand->next_candidate ) 
	{
		if ( cand->numeric_value > 0 )
		{
			current_sum += cand->numeric_value;
			if ( selected_probability <= current_sum )
				return cand;
		}
	}

	return NIL;
}

/***************************************************************************
 * Function     : exploration_boltzmann_select
 **************************************************************************/
preference *exploration_boltzmann_select( agent *my_agent, preference *candidates )
{
	preference *cand = 0;
	double total_probability = 0;
	double selected_probability = 0;
	double rn = 0;
	double current_sum = 0;

	double temp = exploration_get_parameter_value( my_agent, (const long) EXPLORATION_PARAM_TEMPERATURE );
	
	// output trace information
	if ( my_agent->sysparams[ TRACE_INDIFFERENT_SYSPARAM ] )
	{
		for ( cand = candidates; cand != NIL; cand = cand->next_candidate )
		{
			print_with_symbols( my_agent, "\n Candidate %y:  ", cand->value );
			print( my_agent, "Value (Sum) = %f, (Exp) = %f", cand->numeric_value, exp( cand->numeric_value / temp ) );
			xml_begin_tag( my_agent, kTagCandidate );
			xml_att_val( my_agent, kCandidateName, cand->value );
			xml_att_val( my_agent, kCandidateType, kCandidateTypeSum );
			xml_att_val( my_agent, kCandidateValue, cand->numeric_value );
			xml_att_val( my_agent, kCandidateExpValue, exp( cand->numeric_value / temp ) );
			xml_end_tag( my_agent, kTagCandidate );
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
	double q_diff = 0;
	if ( candidates->next_candidate != NIL ) 
	{
		double q_high = candidates->numeric_value;
		double q_low = candidates->numeric_value;
		
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
		double q_val = ( cand->numeric_value - q_diff );
		total_probability += exp( q_val / temp );
		
		/**
 		 * Let user know if adjusted q-value will overflow
		 */
		if ( q_val > q_max )
		{
			print( my_agent, "WARNING: Boltzmann update overflow! %g > %g", q_val, q_max );
			
			char buf[256];
       		SNPRINTF( buf, 254, "WARNING: Boltzmann update overflow! %g > %g", q_val, q_max );
       		xml_generate_warning( my_agent, buf );
		}
	}

	rn = SoarRand(); // generates a number in [0,1]
	selected_probability = rn * total_probability;

	for (cand = candidates; cand != NIL; cand = cand->next_candidate) 
	{
		current_sum += exp( ( cand->numeric_value - q_diff ) / temp );
		
		if ( selected_probability <= current_sum )
			return cand;
	}
	
	return NIL;
}

/***************************************************************************
 * Function     : exploration_epsilon_greedy_select
 **************************************************************************/
preference *exploration_epsilon_greedy_select( agent *my_agent, preference *candidates )
{
	preference *cand = 0;

	double epsilon = exploration_get_parameter_value( my_agent, static_cast< const long >( EXPLORATION_PARAM_EPSILON ) );

	if ( my_agent->sysparams[ TRACE_INDIFFERENT_SYSPARAM ] )
	{
		for ( cand = candidates; cand != NIL; cand = cand->next_candidate )
		{
			print_with_symbols( my_agent, "\n Candidate %y:  ", cand->value );
			print( my_agent, "Value (Sum) = %f", cand->numeric_value );
			xml_begin_tag( my_agent, kTagCandidate );
			xml_att_val( my_agent, kCandidateName, cand->value );
			xml_att_val( my_agent, kCandidateType, kCandidateTypeSum );
			xml_att_val( my_agent, kCandidateValue, cand->numeric_value );
			xml_end_tag( my_agent, kTagCandidate );
		}
	}

	if ( SoarRand() < epsilon )	
		return exploration_randomly_select( candidates );
	else
		return exploration_get_highest_q_value_pref( candidates );
}

/***************************************************************************
 * Function     : exploration_get_highest_q_value_pref
 **************************************************************************/
preference *exploration_get_highest_q_value_pref( preference *candidates )
{
	preference *cand;
	preference *top_cand = candidates;
	double top_value = candidates->numeric_value;
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
 * Function     : exploration_compute_value_of_candidate
 **************************************************************************/
void exploration_compute_value_of_candidate( agent *my_agent, preference *cand, slot *s, double default_value )
{
	if ( !cand ) return;

	// temporary runner
	preference *pref;

	// initialize candidate values
	cand->total_preferences_for_candidate = 0;
	cand->numeric_value = 0;
	cand->rl_contribution = false;
	
	// all numeric indifferents
	for ( pref = s->preferences[ NUMERIC_INDIFFERENT_PREFERENCE_TYPE ]; pref != NIL; pref = pref->next) 
	{
		if ( cand->value == pref->value )
		{
			cand->total_preferences_for_candidate += 1;
			cand->numeric_value += get_number_from_symbol( pref->referent );

			if ( pref->inst->prod->rl_rule )
			{
				cand->rl_contribution = true;
			}
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
	{
		cand->numeric_value = cand->numeric_value / cand->total_preferences_for_candidate;
	}
}
