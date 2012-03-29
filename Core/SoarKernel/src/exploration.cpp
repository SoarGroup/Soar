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
#include <list>
#include <limits>

#include "agent.h"
#include "soar_rand.h"
#include "xml.h"
#include "print.h"
#include "soar_TraceNames.h"
#include "gsysparam.h"
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
	return exploration_convert_policy( policy_name ) != 0;
}

bool exploration_valid_policy( const int policy )
{
	return policy > 0 && policy < USER_SELECT_INVALID;
}

/***************************************************************************
 * Function     : exploration_convert_policy
 **************************************************************************/
const int exploration_convert_policy( const char *policy_name )
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

	return 0;
}

const char *exploration_convert_policy( const int policy )
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
	const int policy = exploration_convert_policy( policy_name );

	if ( policy )
		return exploration_set_policy( my_agent, policy );

	return false;
}

bool exploration_set_policy( agent *my_agent, const int policy )
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
const int exploration_get_policy( agent *my_agent )
{
	return static_cast<int>(my_agent->sysparams[ USER_SELECT_MODE_SYSPARAM ]);
}

/***************************************************************************
 * Function     : exploration_add_parameter
 **************************************************************************/
exploration_parameter *exploration_add_parameter( double value, bool (*val_func)( double ), const char *name )
{
	// new parameter entry
	exploration_parameter * const newbie = new exploration_parameter;
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
const int exploration_convert_parameter( agent *my_agent, const char *name )
{
	for ( int i = 0; i < EXPLORATION_PARAMS; ++i )
		if ( !strcmp( name, my_agent->exploration_params[ i ]->name ) )
			return i;

	return EXPLORATION_PARAMS;
}

const char *exploration_convert_parameter( agent *my_agent, const int parameter )
{
	return ( parameter >= 0 && parameter < EXPLORATION_PARAMS ) ? my_agent->exploration_params[ parameter ]->name : NULL ;
}

/***************************************************************************
 * Function     : exploration_valid_parameter
 **************************************************************************/
const bool exploration_valid_parameter( agent *my_agent, const char *name )
{
	return exploration_convert_parameter( my_agent, name ) != EXPLORATION_PARAMS;
}

const bool exploration_valid_parameter( agent *my_agent, const int parameter )
{
	return exploration_convert_parameter( my_agent, parameter ) != NULL;
}

/***************************************************************************
 * Function     : exploration_get_parameter_value
 **************************************************************************/
double exploration_get_parameter_value( agent *my_agent, const char *parameter )
{	
	const int param = exploration_convert_parameter( my_agent, parameter );
	if ( param == EXPLORATION_PARAMS )
		return 0;

	return my_agent->exploration_params[ param ]->value;
}

double exploration_get_parameter_value( agent *my_agent, const int parameter )
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
	return value >= 0 && value <= 1;
}

/***************************************************************************
 * Function     : exploration_validate_temperature
 **************************************************************************/
bool exploration_validate_temperature( double value )
{
	return value > 0;
}

/***************************************************************************
 * Function     : exploration_valid_parameter_value
 **************************************************************************/
bool exploration_valid_parameter_value( agent *my_agent, const char *name, double value )
{
	const int param = exploration_convert_parameter( my_agent, name );
	if ( param == EXPLORATION_PARAMS )
		return false;

	return my_agent->exploration_params[ param ]->val_func( value );
}

bool exploration_valid_parameter_value( agent *my_agent, const int parameter, double value )
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
	const int param = exploration_convert_parameter( my_agent, name );
	if ( param == EXPLORATION_PARAMS )
		return false;

	my_agent->exploration_params[ param ]->value = value;

	return true;
}

bool exploration_set_parameter_value( agent *my_agent, const int parameter, double value )
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
	return my_agent->sysparams[ USER_SELECT_REDUCE_SYSPARAM ] != FALSE;
}

/***************************************************************************
 * Function     : exploration_set_auto_update
 **************************************************************************/
bool exploration_set_auto_update( agent *my_agent, bool setting )
{
	my_agent->sysparams[ USER_SELECT_REDUCE_SYSPARAM ] = setting ? TRUE : FALSE;

	return true;
}

/***************************************************************************
 * Function     : exploration_update_parameters
 **************************************************************************/
void exploration_update_parameters( agent *my_agent )
{	
	if ( exploration_get_auto_update( my_agent ) )
	{
		for ( int i = 0; i < EXPLORATION_PARAMS; ++i )
		{
			const int reduction_policy = exploration_get_reduction_policy( my_agent, i );
			const double reduction_rate = exploration_get_reduction_rate( my_agent, i, reduction_policy );

			if ( reduction_policy == EXPLORATION_REDUCTION_EXPONENTIAL )
			{
				if ( reduction_rate != 1 )
				{
					const double current_value = exploration_get_parameter_value( my_agent, i );

					exploration_set_parameter_value( my_agent, i, current_value * reduction_rate );
				}
			}
			else if ( reduction_policy == EXPLORATION_REDUCTION_LINEAR )
			{
				const double current_value = exploration_get_parameter_value( my_agent, i );

				if ( current_value > 0 && reduction_rate != 0.0 )
					exploration_set_parameter_value( my_agent, i, (current_value - reduction_rate > 0) ? (current_value - reduction_rate) : 0 );
			}
		}
	}
}

/***************************************************************************
 * Function     : exploration_convert_reduction_policy
 **************************************************************************/
const int exploration_convert_reduction_policy( const char *policy_name )
{
	if ( !strcmp( policy_name, "exponential" ) )
		return EXPLORATION_REDUCTION_EXPONENTIAL;
	if ( !strcmp( policy_name, "linear" ) )
		return EXPLORATION_REDUCTION_LINEAR;

	return EXPLORATION_REDUCTIONS;
}

const char *exploration_convert_reduction_policy( const int policy )
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
const int exploration_get_reduction_policy( agent *my_agent, const char *parameter )
{
	const int param = exploration_convert_parameter( my_agent, parameter );
	if ( param == EXPLORATION_PARAMS )
		return EXPLORATION_REDUCTIONS;

	return my_agent->exploration_params[ param ]->reduction_policy;
}

const int exploration_get_reduction_policy( agent *my_agent, const int parameter )
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
	return exploration_convert_reduction_policy( policy_name ) != EXPLORATION_REDUCTIONS;
}

bool exploration_valid_reduction_policy( agent * /*my_agent*/, const char * /*parameter*/, const int policy )
{	
	return exploration_convert_reduction_policy( policy ) != NULL;
}

bool exploration_valid_reduction_policy( agent * /*my_agent*/, const int /*parameter*/, const int policy )
{	
	return exploration_convert_reduction_policy( policy ) != NULL;
}

/***************************************************************************
 * Function     : exploration_set_reduction_policy
 **************************************************************************/
bool exploration_set_reduction_policy( agent *my_agent, const char *parameter, const char *policy_name )
{
	const int param = exploration_convert_parameter( my_agent, parameter );
	if ( param == EXPLORATION_PARAMS )
		return false;

	const int policy = exploration_convert_reduction_policy( policy_name );
	if ( policy == EXPLORATION_REDUCTIONS )
		return false;

	my_agent->exploration_params[ param ]->reduction_policy = policy;

	return true;
}

bool exploration_set_reduction_policy( agent *my_agent, const int parameter, const int policy )
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
	const int param = exploration_convert_parameter( my_agent, parameter );
	if ( param == EXPLORATION_PARAMS )
		return false;

	const int policy = exploration_convert_reduction_policy( policy_name );
	if ( policy == EXPLORATION_REDUCTIONS )
		return false;

	return exploration_valid_reduction_rate( my_agent, param, policy, reduction_rate );
}

bool exploration_valid_reduction_rate( agent *my_agent, const int parameter, const int policy, double reduction_rate )
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
	return reduction_rate >= 0 && reduction_rate <= 1;
}

/***************************************************************************
 * Function     : exploration_valid_linear
 **************************************************************************/
bool exploration_valid_linear( double reduction_rate )
{
	return reduction_rate >= 0;
}

/***************************************************************************
 * Function     : exploration_get_reduction_rate
 **************************************************************************/
double exploration_get_reduction_rate( agent *my_agent, const char *parameter, const char *policy_name )
{
	const int param = exploration_convert_parameter( my_agent, parameter );
	if ( param == EXPLORATION_PARAMS )
		return 0;

	const int policy = exploration_convert_reduction_policy( policy_name );
	if ( policy == EXPLORATION_REDUCTIONS )
		return 0;

	return exploration_get_reduction_rate( my_agent, param, policy );
}

double exploration_get_reduction_rate( agent *my_agent, const int parameter, const int policy )
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
	const int param = exploration_convert_parameter( my_agent, parameter );
	if ( param == EXPLORATION_PARAMS )
		return false;

	const int policy = exploration_convert_reduction_policy( policy_name );
	if ( policy == EXPLORATION_REDUCTIONS )
		return false;

	return exploration_set_reduction_rate( my_agent, param, policy, reduction_rate );
}

bool exploration_set_reduction_rate( agent *my_agent, const int parameter, const int policy, double reduction_rate )
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
	const int exploration_policy = exploration_get_policy( my_agent );
	preference *return_val = NULL;

	const bool my_rl_enabled = rl_enabled( my_agent );

	const rl_param_container::learning_choices my_learning_policy = my_rl_enabled ? my_agent->rl_params->learning_policy->get_value() : rl_param_container::q;

  exploration_compute_value_of_candidates(my_agent, candidates, s); ///< bazald

	double top_value = candidates->numeric_value;
	bool top_rl = candidates->rl_contribution;

	// should find highest valued candidate in q-learning
	if ( my_rl_enabled && my_learning_policy == rl_param_container::q )
	{
		for ( const preference * cand = candidates; cand; cand = cand->next_candidate )
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
			for ( return_val = candidates; return_val->next_candidate; return_val = return_val->next_candidate );
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
			rl_perform_update( my_agent, top_value, top_rl, s->id );

			if ( return_val->numeric_value != top_value )
				rl_watkins_clear( my_agent, s->id );
		}
	}

	return return_val;
}

/***************************************************************************
 * Function     : exploration_probability_according_to_policy, bazald
 **************************************************************************/
double exploration_probability_according_to_policy( agent *my_agent, slot *s, preference *candidates, preference *selection )
{ 
  const int exploration_policy = exploration_get_policy(my_agent);

  switch(exploration_policy)
  {
    case USER_SELECT_FIRST:
      return candidates == selection ? 1.0f : 0.0f;

    case USER_SELECT_LAST:
      return selection->next_candidate ? 0.0f: 1.0f;

    case USER_SELECT_RANDOM:
    {
      unsigned int cand_count = 0;
      for(const preference * cand = candidates; cand; cand = cand->next_candidate)
        ++cand_count;
      return 1.0 / cand_count;
    }

    case USER_SELECT_SOFTMAX:
    {
      unsigned int cand_count = 0;
      double total_probability = 0.0;
      for(const preference *cand = candidates; cand; cand = cand->next_candidate) {
        ++cand_count;
        if(cand->numeric_value > 0)
          total_probability += cand->numeric_value;
      }

      if(total_probability > 0) {
        if(selection->numeric_value > 0)
          return selection->numeric_value / total_probability;
        else
          return 0.0;
      }
      else
        return 1.0 / cand_count;
    }

    case USER_SELECT_E_GREEDY:
    {
      const double epsilon = exploration_get_parameter_value(my_agent, EXPLORATION_PARAM_EPSILON);

      double top_value = candidates->numeric_value;
      unsigned int top_count = 0;
      unsigned int cand_count = 0;
      for(const preference * cand = candidates; cand; cand = cand->next_candidate) {
        ++cand_count;
        if(cand->numeric_value > top_value) {
          top_value = cand->numeric_value;
          top_count = 1;
        }
        else if(cand->numeric_value == top_value)
          ++top_count;
      }

      double retval = epsilon / cand_count;
      if(selection->numeric_value == top_value)
        retval += (1.0 - epsilon) / top_count;
      return retval;
    }

    case USER_SELECT_BOLTZMANN:
    {
      const double t = exploration_get_parameter_value(my_agent, EXPLORATION_PARAM_TEMPERATURE);

      double maxq = candidates->numeric_value;
      for(preference *cand = candidates->next_candidate; cand; cand = cand->next_candidate) {
        if(maxq < cand->numeric_value)
          maxq = cand->numeric_value;
      }

      double exptotal = 0.0;
      double expselection = 0.0;
      for(preference *cand = candidates; cand; cand = cand->next_candidate) {
        // equivalent to exp((cand->numeric_value / t) - (maxq / t)) but safer against overflow
        double v = exp((cand->numeric_value - maxq) / t);
        exptotal += v;
        if(cand == selection)
          expselection = v;
      }

      return expselection / exptotal;
    }

    default:
      abort();
      return 0.0;
  }
}

/***************************************************************************
 * Function     : exploration_randomly_select
 **************************************************************************/
preference *exploration_randomly_select( preference *candidates )
{
	unsigned int cand_count = 0;
	for ( const preference * cand = candidates; cand; cand = cand->next_candidate )
		++cand_count;

	preference * cand = candidates;
	for( uint32_t chosen_num = SoarRandInt( cand_count - 1 ); chosen_num; --chosen_num )
		cand = cand->next_candidate;

	return cand;
}

/***************************************************************************
 * Function     : exploration_probabilistically_select
 **************************************************************************/
preference *exploration_probabilistically_select( preference *candidates )
{
	// IF THIS FUNCTION CHANGES, SEE soar_ecPrintPreferences

	// count up positive numbers
	double total_probability = 0.0;
	for ( const preference *cand = candidates; cand; cand = cand->next_candidate )
		if ( cand->numeric_value > 0 )
			total_probability += cand->numeric_value;

	// if nothing positive, resort to random
	if ( total_probability == 0.0 )
		return exploration_randomly_select( candidates );

	// choose a random preference within the distribution
	const double selected_probability = total_probability * SoarRand();

	// select the candidate based upon the chosen preference
	double current_sum = 0.0;
	for ( preference *cand = candidates; cand; cand = cand->next_candidate )
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

/*
 * Select a candidate whose Q-value is Q_i with probability
 *
 * e^(Q_i / t) / sum(j=1 to n, e^(Q_j / t)).
 * 
 * Since Q values can get very large or very small (negative values),
 * overflow and underflow problems can occur when calculating the
 * exponentials. This is avoided by subtracting a constant k from
 * all exponent values involved. This doesn't affect the actual
 * probabilities with which candidates are chosen, because subtracting
 * a constant from an exponent is equivalent to dividing by the base
 * raised to that constant, and the divisors cancel out during the
 * calculation.
 *
 * k is chosen to be Q_max / t. This means that the values of all
 * numerator exponentials are at most 1, and the value of the sum in the
 * denominator is between 1 and n. This gets rid of the overflow problem
 * completely, and in the cases where underflow will occur, the actual
 * probability of the action being considered will be so small (< 10^-300)
 * that it's negligible.
 */
preference *exploration_boltzmann_select( agent *my_agent, preference *candidates )
{
	double t = exploration_get_parameter_value( my_agent, EXPLORATION_PARAM_TEMPERATURE );
	double maxq;
	preference* c;
	
	maxq = candidates->numeric_value;
	for (c = candidates->next_candidate; c; c = c->next_candidate) {
		if (maxq < c->numeric_value)
			maxq = c->numeric_value;
	}
	
	double exptotal = 0.0;
	std::list<double> expvals;
	std::list<double>::iterator i;

	for (c = candidates; c; c = c->next_candidate) {
		// equivalent to exp((c->numeric_value / t) - (maxq / t)) but safer against overflow
		double v = exp((c->numeric_value - maxq) / t);
		expvals.push_back(v);
		exptotal += v;
	}
	
	// output trace information
	if ( my_agent->sysparams[ TRACE_INDIFFERENT_SYSPARAM ] )
	{
		for (c = candidates, i = expvals.begin(); c; c = c->next_candidate, i++)
		{
			double prob = *i / exptotal;
			print_with_symbols( my_agent, "\n Candidate %y:  ", c->value );
			print( my_agent, "Value (Sum) = %f, (Prob) = %f", c->numeric_value, prob );
			xml_begin_tag( my_agent, kTagCandidate );
			xml_att_val( my_agent, kCandidateName, c->value );
			xml_att_val( my_agent, kCandidateType, kCandidateTypeSum );
			xml_att_val( my_agent, kCandidateValue, c->numeric_value );
			xml_att_val( my_agent, kCandidateExpValue, prob );
			xml_end_tag( my_agent, kTagCandidate );
		}
	}

	double r = SoarRand(exptotal);
	double sum = 0.0;

	for (c = candidates, i = expvals.begin(); c; c = c->next_candidate, i++) {
		sum += *i;
		if (sum >= r)
			return c;
	}
	
	return NIL;
}

/***************************************************************************
 * Function     : exploration_epsilon_greedy_select
 **************************************************************************/
preference *exploration_epsilon_greedy_select( agent *my_agent, preference *candidates )
{
	const double epsilon = exploration_get_parameter_value( my_agent, EXPLORATION_PARAM_EPSILON );

	if ( my_agent->sysparams[ TRACE_INDIFFERENT_SYSPARAM ] )
	{
		for ( const preference *cand = candidates; cand; cand = cand->next_candidate )
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
	preference *top_cand = candidates;
	double top_value = candidates->numeric_value;
	int num_max_cand = 0;

	for ( preference * cand = candidates; cand; cand = cand->next_candidate )
	{
		if ( cand->numeric_value > top_value )
		{
			top_value = cand->numeric_value;
			top_cand = cand;
			num_max_cand = 1;
		} 
		else if ( cand->numeric_value == top_value )
			++num_max_cand;
	}

	if ( num_max_cand == 1 )
		return top_cand;
	else
	{
		preference *cand = candidates;
		while ( cand->numeric_value != top_value )
			cand = cand->next_candidate;

		// if operators tied for highest Q-value, select among tied set at random
		for ( uint32_t chosen_num = SoarRandInt( num_max_cand - 1 ); chosen_num; --chosen_num )
		{
			cand = cand->next_candidate;

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
	if ( !cand )
		return;

	// initialize candidate values
	cand->total_preferences_for_candidate = 0;
	cand->numeric_value = 0;
	cand->rl_contribution = false;

	// all numeric indifferents
	for ( preference *pref = s->preferences[ NUMERIC_INDIFFERENT_PREFERENCE_TYPE ]; pref; pref = pref->next)
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
	for ( preference *pref = s->preferences[ BINARY_INDIFFERENT_PREFERENCE_TYPE ]; pref; pref = pref->next )
	{
		if (cand->value == pref->value)
		{
			cand->total_preferences_for_candidate += 1;
			cand->numeric_value += get_number_from_symbol( pref->referent );
        	}
	}

	// if no contributors, provide default
	if ( !cand->total_preferences_for_candidate )
	{
		cand->numeric_value = default_value;
		cand->total_preferences_for_candidate = 1;
	}

	// accomodate average mode
	if ( my_agent->numeric_indifferent_mode == NUMERIC_INDIFFERENT_MODE_AVG )
		cand->numeric_value = cand->numeric_value / cand->total_preferences_for_candidate;
}

void exploration_compute_value_of_candidates(agent *my_agent, preference *candidates, slot *s) ///< bazald
{
  // get preference values for each candidate
  // see soar_ecPrintPreferences
  for(preference *cand = candidates; cand; cand = cand->next_candidate)
    exploration_compute_value_of_candidate(my_agent, cand, s);
}
