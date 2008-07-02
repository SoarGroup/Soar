/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/*************************************************************************
 *
 *  file:  reinforcement_learing.h
 *
 * =======================================================================
 */

#ifndef REINFORCEMENT_LEARNING_H
#define REINFORCEMENT_LEARNING_H

#include <map>
#include <string>
#include <list>
#include <set>

#include "production.h"

// choosing the q value confidence method
#define HOEFFDING_BOUNDING  0
#define INTERVAL_ESTIMATION 1
#define BAYESIAN_ESTIMATION 2

#define Q_CONFIDENCE_METHOD INTERVAL_ESTIMATION
//#define Q_CONFIDENCE_METHOD INTERVAL_ESTIMATION

//////////////////////////////////////////////////////////
// RL Constants
//////////////////////////////////////////////////////////
#define RL_RETURN_LONG 0.1
#define RL_RETURN_STRING ""

#define RL_LEARNING_ON 1
#define RL_LEARNING_OFF 2

#define RL_LEARNING_SARSA 1
#define RL_LEARNING_Q 2

#define RL_TE_ON 1
#define RL_TE_OFF 2

// names of params
#define RL_PARAM_LEARNING					0
#define RL_PARAM_DISCOUNT_RATE				1
#define RL_PARAM_LEARNING_RATE				2
#define RL_PARAM_LEARNING_POLICY			3
#define RL_PARAM_ET_DECAY_RATE				4
#define RL_PARAM_ET_TOLERANCE         5
#define RL_PARAM_TEMPORAL_EXTENSION   6

#if Q_CONFIDENCE_METHOD == HOEFFDING_BOUNDING || Q_CONFIDENCE_METHOD == INTERVAL_ESTIMATION
#define RL_PARAM_BOUND_CONFIDENCE     7
#if Q_CONFIDENCE_METHOD == HOEFFDING_BOUNDING
#define RL_PARAM_SA_SPACE_SIZE        8
#define RL_PARAM_R_MAX                9
#define RL_PARAM_V_MAX                10
#define RL_PARAMS							       11 // must be 1+ last rl param

#elif Q_CONFIDENCE_METHOD == INTERVAL_ESTIMATION
#define RL_PARAM_IE_WINSIZE           8
#define RL_PARAM_IE_LOWER_INDEX       9
#define RL_PARAM_IE_UPPER_INDEX       10
#define RL_PARAMS                     11

#endif
#endif

// names of stats
#define RL_STAT_UPDATE_ERROR				0
#define RL_STAT_TOTAL_REWARD				1
#define RL_STAT_GLOBAL_REWARD				2
#define RL_STATS							3 // must be 1+ last rl stat

// more specific forms of no change impasse types
// made negative to never conflict with impasse constants
#define STATE_NO_CHANGE_IMPASSE_TYPE -1
#define OP_NO_CHANGE_IMPASSE_TYPE -2

//////////////////////////////////////////////////////////
// RL Types
//////////////////////////////////////////////////////////
enum rl_param_type { rl_param_string = 1, rl_param_number = 2, rl_param_invalid = 3 };

typedef struct rl_string_parameter_struct  
{
	long value;
	bool (*val_func)( const long );
	const char *(*to_str)( const long );
	const long (*from_str)( const char * );
} rl_string_parameter;

typedef struct rl_number_parameter_struct  
{
	double value;
	bool (*val_func)( double );
} rl_number_parameter;

typedef union rl_parameter_union_class
{
	rl_string_parameter string_param;
	rl_number_parameter number_param;
} rl_parameter_union;

typedef struct rl_parameter_struct
{
	rl_parameter_union *param;
	rl_param_type type;
	const char *name;
} rl_parameter;

typedef struct rl_stat_struct
{
	double value;
	const char *name;
} rl_stat;

template <class T> class SoarMemoryAllocator;
typedef std::map<production *, double, std::less<production *>, SoarMemoryAllocator<std::pair<production* const, double> > > rl_et_map;

typedef struct rl_data_struct {
 	rl_et_map *eligibility_traces;
	list *prev_op_rl_rules;
	double previous_q;
	double reward;
	unsigned int reward_age;	// the number of steps since a cycle containing rl rules
	unsigned int num_prev_op_rl_rules;
	unsigned int step;			// the number of steps the current operator has been installed at the goal
	signed int impasse_type;	// if this goal is an impasse, what type
} rl_data;

typedef struct rl_qconf_data_struct {
#if Q_CONFIDENCE_METHOD == INTERVAL_ESTIMATION   
  double q_min;
  double q_max;
  // previous n sample window sorted by value
  std::multiset<double> win_by_val;
  // previous n sample window sorted by time
  std::list<double> win_by_time;

#elif Q_CONFIDENCE_METHOD == HOEFFDING_BOUNDING
  double q_min;
  double q_max;
  int num_updates;
#endif
} rl_qconf_data;


//
// These must go below types
//

#include "stl_support.h"

//////////////////////////////////////////////////////////
// Parameter Maintenance
//////////////////////////////////////////////////////////

// memory clean
extern void rl_clean_parameters( agent *my_agent );
extern void rl_clean_stats( agent *my_agent );

// reinitialize Soar-RL data structures
extern void rl_reset_data( agent *my_agent );

// reinitialize Soar-RL statistics
extern void rl_reset_stats( agent *my_agent );

// remove Soar-RL references to a production
extern void rl_remove_refs_for_prod( agent *my_agent, production *prod );

//////////////////////////////////////////////////////////
// Parameter Get/Set/Validate
//////////////////////////////////////////////////////////

// add parameter
extern rl_parameter *rl_add_parameter( const char *name, double value, bool (*val_func)( double ) );
extern rl_parameter *rl_add_parameter( const char *name, const long value, bool (*val_func)( const long ), const char *(*to_str)( long ), const long (*from_str)( const char * ) );

// convert parameter
extern const char *rl_convert_parameter( agent *my_agent, const long param );
extern const long rl_convert_parameter( agent *my_agent, const char *name );

// validate parameter
extern bool rl_valid_parameter( agent *my_agent, const char *name );
extern bool rl_valid_parameter( agent *my_agent, const long param );

// parameter type
extern rl_param_type rl_get_parameter_type( agent *my_agent, const char *name );
extern rl_param_type rl_get_parameter_type( agent *my_agent, const long param );

// get parameter
extern const long rl_get_parameter( agent *my_agent, const char *name, const double test );
extern const char *rl_get_parameter( agent *my_agent, const char *name, const char *test );
extern double rl_get_parameter( agent *my_agent, const char *name );

extern const long rl_get_parameter( agent *my_agent, const long param, const double test );
extern const char *rl_get_parameter( agent *my_agent, const long param, const char *test );
extern double rl_get_parameter( agent *my_agent, const long param );

// validate parameter value
extern bool rl_valid_parameter_value( agent *my_agent, const char *name, double new_val );
extern bool rl_valid_parameter_value( agent *my_agent, const char *name, const char *new_val );
extern bool rl_valid_parameter_value( agent *my_agent, const char *name, const long new_val );

extern bool rl_valid_parameter_value( agent *my_agent, const long param, double new_val );
extern bool rl_valid_parameter_value( agent *my_agent, const long param, const char *new_val );
extern bool rl_valid_parameter_value( agent *my_agent, const long param, const long new_val );

// set parameter
extern bool rl_set_parameter( agent *my_agent, const char *name, double new_val );
extern bool rl_set_parameter( agent *my_agent, const char *name, const char *new_val );
extern bool rl_set_parameter( agent *my_agent, const char *name, const long new_val );

extern bool rl_set_parameter( agent *my_agent, const long param, double new_val );
extern bool rl_set_parameter( agent *my_agent, const long param, const char *new_val );
extern bool rl_set_parameter( agent *my_agent, const long param, const long new_val );

// learning
extern bool rl_validate_learning( const long new_val );
extern const char *rl_convert_learning( const long val );
extern const long rl_convert_learning( const char *val );

// discount rate
extern bool rl_validate_discount( const double new_val );

// learning rate
extern bool rl_validate_learning_rate( const double new_val );

// learning policy
extern bool rl_validate_learning_policy( const long new_val );
extern const char *rl_convert_learning_policy( const long val );
extern const long rl_convert_learning_policy( const char *val );

// trace decay rate
extern bool rl_validate_decay_rate( const double new_val );

// trace tolerance
extern bool rl_validate_trace_tolerance( const double new_val );

// temporal-extension
extern bool rl_validate_te_enabled( const long new_val );
extern const char *rl_convert_te_enabled( const long val );
extern const long rl_convert_te_enabled( const char *val );

extern bool validate_nonnegative(const double new_val);
extern bool validate_probability(const double new_val);

// shortcut for determining if Soar-RL is enabled
extern bool rl_enabled( agent *my_agent );

//////////////////////////////////////////////////////////
// Stats
//////////////////////////////////////////////////////////

// add stat
extern rl_stat *rl_add_stat( const char *name );

// convert stat
extern const long rl_convert_stat( agent *my_agent, const char *name );
extern const char *rl_convert_stat( agent *my_agent, const long stat );

// valid stat
extern bool rl_valid_stat( agent *my_agent, const char *name );
extern bool rl_valid_stat( agent *my_agent, const long stat );

// get stat
extern double rl_get_stat( agent *my_agent, const char *name );
extern double rl_get_stat( agent *my_agent, const long stat );

// set stat
extern bool rl_set_stat( agent *my_agent, const char *name, double new_val );
extern bool rl_set_stat( agent *my_agent, const long stat, double new_val );

//////////////////////////////////////////////////////////
// Production Validation
//////////////////////////////////////////////////////////

// validate template
extern bool rl_valid_template( production *prod );

// validate rl rule
extern bool rl_valid_rule( production *prod );

// template instantiation
extern int rl_get_template_id( const char *prod_name );

//////////////////////////////////////////////////////////
// Template Tracking
//////////////////////////////////////////////////////////

// initializes agent's tracking of template-originated rl-rules
extern void rl_initialize_template_tracking( agent *my_agent );

// updates the agent's tracking of template-originated rl-rules
extern void rl_update_template_tracking( agent *my_agent, const char *rule_name );

// get the next id for a template (increments internal counter)
extern int rl_next_template_id( agent *my_agent );

// reverts internal counter
extern void rl_revert_template_id( agent *my_agent );

//////////////////////////////////////////////////////////
// Template Behavior
//////////////////////////////////////////////////////////

// builds a new Soar-RL rule from a template instantiation
extern Symbol *rl_build_template_instantiation( agent *my_agent, instantiation *my_template_instance, struct token_struct *tok, wme *w );

// creates an incredibly simple action
extern action *rl_make_simple_action( agent *my_gent, Symbol *id_sym, Symbol *attr_sym, Symbol *val_sym, Symbol *ref_sym );

// adds a test to a condition list for goals or impasses contained within the condition list
extern void rl_add_goal_or_impasse_tests_to_conds(agent *my_agent, condition *all_conds);

//////////////////////////////////////////////////////////
// Reward
//////////////////////////////////////////////////////////

// tabulation of a single goal's reward
extern void rl_tabulate_reward_value_for_goal( agent *my_agent, Symbol *goal );

// tabulation of all agent goal reward
extern void rl_tabulate_reward_values( agent *my_agent );

// shortcut function to discount a reward value based upon current discount mode
extern double rl_discount_reward( agent *my_agent, double reward, unsigned int step );

//////////////////////////////////////////////////////////
// Updates
//////////////////////////////////////////////////////////

// Store and update data that will be needed later to perform a Bellman update for the current operator
extern void rl_store_data( agent *my_agent, Symbol *goal, preference *cand );

// update the value of Soar-RL rules
extern void rl_perform_update( agent *my_agent, double op_value, double Vminb, double Vmaxb, Symbol *goal );

// initialize confidence info for a production
extern void initialize_qconf(agent* my_agent, production* p);

// clears eligibility traces in accordance with watkins
extern void rl_watkins_clear( agent *my_agent, Symbol *goal );

#endif
