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

#include "production.h"
#include "gdatastructs.h"
#include "chunk.h"

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
#define RL_PARAM_ET_TOLERANCE				5
#define RL_PARAM_TEMPORAL_EXTENSION			6
#define RL_PARAMS							7 // must be 1+ last rl param

// names of stats
#define RL_STAT_UPDATE_ERROR				0
#define RL_STAT_TOTAL_REWARD				1
#define RL_STAT_GLOBAL_REWARD				2
#define RL_STATS							3 // must be 1+ last rl stat

// more specific forms of no change impasse types
// made negative to never conflict with impasse constants
#define STATE_NO_CHANGE_IMPASSE_TYPE -1
#define OP_NO_CHANGE_IMPASSE_TYPE -2


//
// These must go below constants
//

#include "stl_support.h"

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
typedef std::map<production *, double, std::less<production *>, SoarMemoryAllocator<std::pair<production* const, double> > > soar_rl_et_map;

typedef struct rl_data_struct {
 	soar_rl_et_map *eligibility_traces;
	list *prev_op_rl_rules;
	double previous_q;
	double reward;
	unsigned int reward_age;	// the number of steps since a cycle containing rl rules
	unsigned int num_prev_op_rl_rules;
	unsigned int step;			// the number of steps the current operator has been installed at the goal
	signed int impasse_type;	// if this goal is an impasse, what type
} rl_data;

//////////////////////////////////////////////////////////
// Parameter Maintenance
//////////////////////////////////////////////////////////

// memory clean
extern void clean_parameters( agent *my_agent );
extern void clean_stats( agent *my_agent );

// reinitialize Soar-RL data structures
extern void reset_rl_data( agent *my_agent );

// reinitialize Soar-RL statistics
extern void reset_rl_stats( agent *my_agent );

// remove Soar-RL references to a production
extern void remove_rl_refs_for_prod( agent *my_agent, production *prod );

//////////////////////////////////////////////////////////
// Parameter Get/Set/Validate
//////////////////////////////////////////////////////////

// add parameter
extern rl_parameter *add_rl_parameter( const char *name, double value, bool (*val_func)( double ) );
extern rl_parameter *add_rl_parameter( const char *name, const long value, bool (*val_func)( const long ), const char *(*to_str)( long ), const long (*from_str)( const char * ) );

// convert parameter
extern const char *convert_rl_parameter( agent *my_agent, const long param );
extern const long convert_rl_parameter( agent *my_agent, const char *name );

// validate parameter
extern bool valid_rl_parameter( agent *my_agent, const char *name );
extern bool valid_rl_parameter( agent *my_agent, const long param );

// parameter type
extern rl_param_type get_rl_parameter_type( agent *my_agent, const char *name );
extern rl_param_type get_rl_parameter_type( agent *my_agent, const long param );

// get parameter
extern const long get_rl_parameter( agent *my_agent, const char *name, const double test );
extern const char *get_rl_parameter( agent *my_agent, const char *name, const char *test );
extern double get_rl_parameter( agent *my_agent, const char *name );

extern const long get_rl_parameter( agent *my_agent, const long param, const double test );
extern const char *get_rl_parameter( agent *my_agent, const long param, const char *test );
extern double get_rl_parameter( agent *my_agent, const long param );

// validate parameter value
extern bool valid_rl_parameter_value( agent *my_agent, const char *name, double new_val );
extern bool valid_rl_parameter_value( agent *my_agent, const char *name, const char *new_val );
extern bool valid_rl_parameter_value( agent *my_agent, const char *name, const long new_val );

extern bool valid_rl_parameter_value( agent *my_agent, const long param, double new_val );
extern bool valid_rl_parameter_value( agent *my_agent, const long param, const char *new_val );
extern bool valid_rl_parameter_value( agent *my_agent, const long param, const long new_val );

// set parameter
extern bool set_rl_parameter( agent *my_agent, const char *name, double new_val );
extern bool set_rl_parameter( agent *my_agent, const char *name, const char *new_val );
extern bool set_rl_parameter( agent *my_agent, const char *name, const long new_val );

extern bool set_rl_parameter( agent *my_agent, const long param, double new_val );
extern bool set_rl_parameter( agent *my_agent, const long param, const char *new_val );
extern bool set_rl_parameter( agent *my_agent, const long param, const long new_val );

// learning
extern bool validate_rl_learning( const long new_val );
extern const char *convert_rl_learning( const long val );
extern const long convert_rl_learning( const char *val );

// discount rate
extern bool validate_rl_discount( const double new_val );

// learning rate
extern bool validate_rl_learning_rate( const double new_val );

// learning policy
extern bool validate_rl_learning_policy( const long new_val );
extern const char *convert_rl_learning_policy( const long val );
extern const long convert_rl_learning_policy( const char *val );

// trace decay rate
extern bool validate_rl_decay_rate( const double new_val );

// trace tolerance
extern bool validate_rl_trace_tolerance( const double new_val );

// temporal-extension
extern bool validate_te_enabled( const long new_val );
extern const char *convert_te_enabled( const long val );
extern const long convert_te_enabled( const char *val );

// shortcut for determining if Soar-RL is enabled
extern bool soar_rl_enabled( agent *my_agent );

//////////////////////////////////////////////////////////
// Stats
//////////////////////////////////////////////////////////

// add stat
extern rl_stat *add_rl_stat( const char *name );

// convert stat
extern const long convert_rl_stat( agent *my_agent, const char *name );
extern const char *convert_rl_stat( agent *my_agent, const long stat );

// valid stat
extern bool valid_rl_stat( agent *my_agent, const char *name );
extern bool valid_rl_stat( agent *my_agent, const long stat );

// get stat
extern double get_rl_stat( agent *my_agent, const char *name );
extern double get_rl_stat( agent *my_agent, const long stat );

// set stat
extern bool set_rl_stat( agent *my_agent, const char *name, double new_val );
extern bool set_rl_stat( agent *my_agent, const long stat, double new_val );

//////////////////////////////////////////////////////////
// Production Validation
//////////////////////////////////////////////////////////

// validate template
extern bool valid_rl_template( production *prod );

// validate rl rule
extern bool valid_rl_rule( production *prod );

// template instantiation
extern int get_template_id( const char *prod_name );

//////////////////////////////////////////////////////////
// Template Tracking
//////////////////////////////////////////////////////////

// initializes agent's tracking of template-originated rl-rules
extern void initialize_template_tracking( agent *my_agent );

// updates the agent's tracking of template-originated rl-rules
extern void update_template_tracking( agent *my_agent, const char *rule_name );

// get the next id for a template (increments internal counter)
extern int next_template_id( agent *my_agent );

// reverts internal counter
extern void revert_template_id( agent *my_agent );

//////////////////////////////////////////////////////////
// Template Behavior
//////////////////////////////////////////////////////////

// builds a new Soar-RL rule from a template instantiation
extern Symbol *build_template_instantiation( agent *my_agent, instantiation *my_template_instance, struct token_struct *tok, wme *w );

// creates an incredibly simple action
extern action *make_simple_action( agent *my_gent, Symbol *id_sym, Symbol *attr_sym, Symbol *val_sym, Symbol *ref_sym );

// adds a test to a condition list for goals or impasses contained within the condition list
extern void add_goal_or_impasse_tests_to_conds(agent *my_agent, condition *all_conds);

//////////////////////////////////////////////////////////
// Reward
//////////////////////////////////////////////////////////

// tabulation of a single goal's reward
extern void tabulate_reward_value_for_goal( agent *my_agent, Symbol *goal );

// tabulation of all agent goal reward
extern void tabulate_reward_values( agent *my_agent );

// shortcut function to discount a reward value based upon current discount mode
extern double discount_reward( agent *my_agent, double reward, unsigned int step );

//////////////////////////////////////////////////////////
// Updates
//////////////////////////////////////////////////////////

// Store and update data that will be needed later to perform a Bellman update for the current operator
extern void store_rl_data( agent *my_agent, Symbol *goal, preference *cand );

// update the value of Soar-RL rules
extern void perform_rl_update( agent *my_agent, double op_value, Symbol *goal );

// clears eligibility traces in accordance with watkins
extern void watkins_clear( agent *my_agent, Symbol *goal );

#endif
