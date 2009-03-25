/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/*************************************************************************
 *
 *  file:  reinforcement_learning.h
 *
 * =======================================================================
 */

#ifndef REINFORCEMENT_LEARNING_H
#define REINFORCEMENT_LEARNING_H

#include <map>
#include <string>
#include <list>

#include "production.h"
#include "soar_module.h"

using namespace soar_module;

//////////////////////////////////////////////////////////
// RL Constants
//////////////////////////////////////////////////////////

// more specific forms of no change impasse types
// made negative to never conflict with impasse constants
#define STATE_NO_CHANGE_IMPASSE_TYPE -1
#define OP_NO_CHANGE_IMPASSE_TYPE -2

//////////////////////////////////////////////////////////
// RL Parameters
//////////////////////////////////////////////////////////

class rl_param_container: public param_container
{
	public:
		enum learning_choices { sarsa, q };
		
		boolean_param *learning;
		decimal_param *discount_rate;
		decimal_param *learning_rate;
		constant_param<learning_choices> *learning_policy;
		decimal_param *et_decay_rate;
		decimal_param *et_tolerance;
		boolean_param *temporal_extension;
		boolean_param *hrl_discount;

		rl_param_container( agent *new_agent );
};

//////////////////////////////////////////////////////////
// RL Statistics
//////////////////////////////////////////////////////////

class rl_stat_container: public stat_container
{
	public:	
		decimal_stat *update_error;
		decimal_stat *total_reward;
		decimal_stat *global_reward;
				
		rl_stat_container( agent *new_agent );
};

//////////////////////////////////////////////////////////
// RL Types
//////////////////////////////////////////////////////////

template <class T> class SoarMemoryAllocator;
typedef std::map<production *, double, std::less<production *>, SoarMemoryAllocator<std::pair<production* const, double> > > rl_et_map;

typedef struct rl_data_struct {
 	rl_et_map *eligibility_traces;
	::list *prev_op_rl_rules;
	double previous_q;
	double reward;
	unsigned int reward_age;	// the number of steps since a cycle containing rl rules
	unsigned int num_prev_op_rl_rules;
} rl_data;

//
// These must go below types
//

#include "stl_support.h"

//////////////////////////////////////////////////////////
// Parameter Maintenance
//////////////////////////////////////////////////////////

// reinitialize Soar-RL data structures
extern void rl_reset_data( agent *my_agent );

// remove Soar-RL references to a production
extern void rl_remove_refs_for_prod( agent *my_agent, production *prod );

//////////////////////////////////////////////////////////
// Parameter Get/Set/Validate
//////////////////////////////////////////////////////////

// shortcut for determining if Soar-RL is enabled
extern bool rl_enabled( agent *my_agent );

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

//////////////////////////////////////////////////////////
// Updates
//////////////////////////////////////////////////////////

// Store and update data that will be needed later to perform a Bellman update for the current operator
extern void rl_store_data( agent *my_agent, Symbol *goal, preference *cand );

// update the value of Soar-RL rules
extern void rl_perform_update( agent *my_agent, double op_value, bool op_rl, Symbol *goal );

// clears eligibility traces in accordance with watkins
extern void rl_watkins_clear( agent *my_agent, Symbol *goal );

#endif
