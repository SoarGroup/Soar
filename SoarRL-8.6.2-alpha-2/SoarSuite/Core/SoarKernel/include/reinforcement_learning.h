#ifndef REINFORCEMENT_H
#define REINFORCEMENT_H

#include <map>
#include <utility>
#include "agent.h"
#include "mem.h"
#include "gdatastructs.h"
#include "symtab.h"
#include "instantiations.h"
#include "production.h"
#include "stl_support.h"


 
const double LAMBDA_TOLERANCE = 0.001; // Determines what eligibility traces considered non-negligible.
typedef std::map<production *, double, std::less<production*>, SoarMemoryAllocator<std::pair<production*, double>>> SoarSTLETMap;
 

/*-------------------------------------------------------------------------
                          Reinforcement learning data

Stores data between decision phases that is needed to update numeric preference values. Each goal identifier
has an RL_data_struct.

Fields in an RL_data_struct:
	eligibility_traces: Contains pointers to RL rules, and the degree to which each rule's numeric value
						should be affected by the Bellman update on a subsequent decision cycle.
	prev_op_RL_rules: A list of RL rules that fired for the last selected operator.
    previous_Q: The Q-value computed for the previously selected operator.
 	reward: Reward accumulated for operator currently selected at this goal.
	step: The number of decision cycles an operator has been active. (will be > 1 only for op no-change)
	impasse_type: Type of impasse in subgoal or no impasse.
--------------------------------------------------------------------------*/
typedef struct RL_data_struct {
 	SoarSTLETMap *eligibility_traces;
	list *prev_op_RL_rules;
	float previous_Q;
	float reward;
	int step;
	byte impasse_type;
} RL_data;

extern float compute_temp_diff(agent *, RL_data *, float);
extern void perform_Bellman_update(agent *, float , Symbol *);
extern void tabulate_reward_values(agent *);
extern void store_RL_data(agent *, Symbol *, preference *);
extern void tabulate_reward_value_for_goal(agent *thisAgent, Symbol *goal);
extern void reset_RL(agent *thisAgent);
extern production *build_production(agent *thisAgent, preference *pref);
extern bool check_prefs_for_RL(production *prod);
extern Bool check_template_for_RL(production *prod);
// void modify_corresponding_template_pref(instantiation *RL_inst);
extern void remove_RL_refs_for_prod(agent *thisAgent, production *prod);
float get_number_from_symbol(Symbol *sym);

#endif