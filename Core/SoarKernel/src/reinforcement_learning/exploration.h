/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/*************************************************************************
 *
 *  file:  exploration.h
 *
 * =======================================================================
 */

#ifndef EXPLORATION_H
#define EXPLORATION_H

#include "kernel.h"

//////////////////////////////////////////////////////////
// Exploration Types
//////////////////////////////////////////////////////////
typedef struct exploration_parameter_struct
{
    const char* name;
    double value;
    int reduction_policy;
    bool (*val_func)(double);
    double rates[ EXPLORATION_REDUCTIONS ];
} exploration_parameter;

//////////////////////////////////////////////////////////
// Exploration Policies
//////////////////////////////////////////////////////////

// validity
extern bool exploration_valid_policy(const char* policy_name);
extern bool exploration_valid_policy(const int policy);

// policy <=> name conversion
extern const int exploration_convert_policy(const char* policy_name);
extern const char* exploration_convert_policy(const int policy);

// sets exploration policy name
extern bool exploration_set_policy(agent* thisAgent, const char* policy_name);
extern bool exploration_set_policy(agent* thisAgent, const int policy);

// get exploration policy
extern const int exploration_get_policy(agent* thisAgent);

//////////////////////////////////////////////////////////
// Exploration Policy Parameters
//////////////////////////////////////////////////////////

// add parameter
extern exploration_parameter* exploration_add_parameter(double value, bool (*val_func)(double), const char* name);

// convert parameter name
extern const int exploration_convert_parameter(agent* thisAgent, const char* name);
extern const char* exploration_convert_parameter(agent* thisAgent, const int parameter);

// validate parameter name
extern const bool exploration_valid_parameter(agent* thisAgent, const char* name);
extern const bool exploration_valid_parameter(agent* thisAgent, const int parameter);

// get parameter value
extern double exploration_get_parameter_value(agent* thisAgent, const char* parameter);
extern double exploration_get_parameter_value(agent* thisAgent, const int parameter);

// validate parameter value
extern bool exploration_validate_epsilon(double value);
extern bool exploration_validate_temperature(double value);

// validate parameter value
extern bool exploration_valid_parameter_value(agent* thisAgent, const char* name, double value);
extern bool exploration_valid_parameter_value(agent* thisAgent, const int parameter, double value);

// set parameter value
extern bool exploration_set_parameter_value(agent* thisAgent, const char* name, double value);
extern bool exploration_set_parameter_value(agent* thisAgent, const int parameter, double value);

// update parameters according to their reduction policies/rates
extern void exploration_update_parameters(agent* thisAgent);

//////////////////////////////////////////////////////////
// Reduction Policies
//////////////////////////////////////////////////////////

// policy <=> name conversion
extern const int exploration_convert_reduction_policy(const char* policy_name);
extern const char* exploration_convert_reduction_policy(const int policy);

// get parameter reduction policy
extern const int exploration_get_reduction_policy(agent* thisAgent, const char* parameter);
extern const int exploration_get_reduction_policy(agent* thisAgent, const int parameter);

// validate reduction policy per parameter
extern bool exploration_valid_reduction_policy(agent* thisAgent, const char* parameter, const char* policy_name);
extern bool exploration_valid_reduction_policy(agent* thisAgent, const char* parameter, const int policy);
extern bool exploration_valid_reduction_policy(agent* thisAgent, const int parameter, const int policy);

// set parameter reduction policy
extern bool exploration_set_reduction_policy(agent* thisAgent, const char* parameter, const char* policy_name);
extern bool exploration_set_reduction_policy(agent* thisAgent, const int parameter, const int policy);

//////////////////////////////////////////////////////////
// Reduction Rates
//////////////////////////////////////////////////////////

// validate reduction rate
extern bool exploration_valid_reduction_rate(agent* thisAgent, const char* parameter, const char* policy_name, double reduction_rate);
extern bool exploration_valid_reduction_rate(agent* thisAgent, const int parameter, const int policy, double reduction_rate);
extern bool exploration_valid_exponential(double reduction_rate);
extern bool exploration_valid_linear(double reduction_rate);

// get reduction rate
extern double exploration_get_reduction_rate(agent* thisAgent, const char* parameter, const char* policy_name);
extern double exploration_get_reduction_rate(agent* thisAgent, const int parameter, const int policy);

// set reduction rate
extern bool exploration_set_reduction_rate(agent* thisAgent, const char* parameter, const char* policy_name, double reduction_rate);
extern bool exploration_set_reduction_rate(agent* thisAgent, const int parameter, const int policy, double reduction_rate);

//////////////////////////////////////////////////////////
// Decision Procedures
//////////////////////////////////////////////////////////

// selects a candidate based upon the current exploration mode
extern preference* exploration_choose_according_to_policy(agent* thisAgent, slot* s, preference* candidates);

// calculate the probability of a selection given the current exploration mode
extern double exploration_probability_according_to_policy(agent* thisAgent, slot* s, preference* candidates, preference* selection);

// selects a candidate in a random fashion
extern preference* exploration_randomly_select(preference* candidates, const bool &update_rho = true);

// selects a candidate in a softmax fashion
extern preference* exploration_probabilistically_select(preference* candidates);

// selects a candidate based on a boltzmann distribution
extern preference* exploration_boltzmann_select(agent* thisAgent, preference* candidates);

// selects a candidate based upon an epsilon-greedy distribution
extern preference* exploration_epsilon_greedy_select(agent* thisAgent, preference* candidates);

// returns candidate with highest q-value (random amongst ties), assumes computed values
extern preference* exploration_get_highest_q_value_pref(preference* candidates);

// computes total contribution for a candidate from each preference, as well as number of contributions
extern void exploration_compute_value_of_candidate(agent* thisAgent, preference* cand, slot* s, double default_value = 0);

#endif

