#ifdef HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#include <math.h>

#include "portability.h"
#include "reinforcement_learning.h"
#include "rhsfun.h"
#include "production.h"
#include "print.h"
#include "instantiations.h"
#include "prefmem.h"
#include "wmem.h"
#include "chunk.h"
#include "rete.h"


production *build_RL_production(agent *thisAgent, condition *top_cond, condition *bottom_cond, not_struct *nots, preference *pref);
void add_goal_or_impasse_tests_to_conds(agent *thisAgent, condition * all_conds);
action *make_simple_action(agent *thisAgent, Symbol *id_sym, Symbol *attr_sym, Symbol *val_sym, Symbol *ref_sym);
extern void variablize_condition_list (agent* thisAgent, condition *cond);
extern void variablize_nots_and_insert_into_conditions (agent* thisAgent, not_struct *nots, condition *conds);
extern void variablize_symbol (agent* thisAgent, Symbol **sym);
extern void add_impasse_wme (agent* thisAgent, Symbol *id, Symbol *attr, Symbol *value, preference *p);


/**************************************************
	Tabulate reward value for goal

Given a goal id ID, sum together any rewards found under that goal's
reward link - more precisely look for numeric values R at ID.reward-link.attr.attr R.
'data->step' should hold the number of steps the current operator has 
been installed at the goal, so if there is an op no-change, data->step > 1
and the new reward is discounted and added to reward accumulated on 
previous decision cycles.

This function is called for a goal right before it is destroyed. It is also called once for each goal 
in the goal stack 
- at the beginning of the decision phase
- right after a 'halt' command.

**************************************************/
void tabulate_reward_value_for_goal(agent *thisAgent, Symbol *goal){
	RL_data *data = goal->id.RL_data;
	if (data->impasse_type == NONE_IMPASSE_TYPE) {} // Only count rewards at top state... 
	else if (data->impasse_type == OP_NO_CHANGE_IMPASSE_TYPE) {} // or for op no-change impasses. 
	else return;
	slot *s = goal->id.reward_header->id.slots;
	float reward = 0.0;
	if (s){
		for ( ; s ; s = s->next){
			for (wme *w = s->wmes ; w ; w = w->next){
				 if (w->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE){
					for (slot *t = w->value->id.slots ; t ; t = t->next){
						for (wme *x = t->wmes ; x ; x = x->next){
							reward = reward + get_number_from_symbol(x->value);
						}
					}
				}
			}
		}
	data->reward += (reward*pow(thisAgent->gamma, data->step));
	}
	data->step++;
}

/************************************************
        Tabulate reward values
Checks for reward at all goals in the goal stack.

Called at the beginning of the decision phase.
************************************************/
void tabulate_reward_values(agent *thisAgent){
	Symbol *goal = thisAgent->top_goal;

	while(goal){
		tabulate_reward_value_for_goal(thisAgent, goal);
	    goal = goal->id.lower_goal;
	}
}

/**************************************************
		Compute temp diff

Computes a TD update. Takes as input the current reward (r_t)
and estimate of the Q-value of the last operator selected (Q_t-1), 
[both stored in RL_data]
and the Q-value at the current time step (Q_t). [stored in op_value]
Discounting uses the gamma parameter and a count of the
time steps since the last operator was selected.
Returns r_t + gamma*Q_t - Q_t-1.
**************************************************/

float compute_temp_diff(agent *thisAgent, RL_data* r, float op_value){
	
	float delta_Q = r->reward;
	delta_Q += pow(thisAgent->gamma, r->step)*op_value;
	delta_Q -= r->previous_Q;
	return delta_Q;
}

/***************************************************
		Perform Bellman update

Compute a TD update delta, and update the value of
RL rules by delta*alpha*eligibility_trace.

If any of the rules changed are currently matching
and asserting their numeric preference, we need to
update the value stored in the current preference
as well as the value in the rule. 

Finally, we reset all the RL_data in preparation for
the next operator.
***************************************************/

void perform_Bellman_update(agent *thisAgent, float op_value, Symbol *goal){
	
	RL_data *data = goal->id.RL_data;
	SoarSTLETMap::iterator iter;

	/* Iterate through eligibility_traces, decay traces. If less than TOLERANCE, remove from map. */
	
	for (iter = data->eligibility_traces->begin() ; iter != data->eligibility_traces->end() ; ){
		iter->second *= thisAgent->lambda;			// iter->second is an eligibility trace
		iter->second *= pow(thisAgent->gamma, data->step);
		if (iter->second < LAMBDA_TOLERANCE) data->eligibility_traces->erase(iter++);
		else ++iter;
	}

	/* Update trace for just fired prods */

	int num_prev_fired_rules = 0;
	for (cons *c = data->prev_op_RL_rules ; c ; c = c->rest)
		if (c->first) num_prev_fired_rules++;

	if (num_prev_fired_rules){
		double trace_increment = 1.0 / num_prev_fired_rules;
		for (cons *c = data->prev_op_RL_rules ; c ; c = c->rest){
			if (c->first){
				iter = data->eligibility_traces->find((production *) c->first);
				if (iter!=data->eligibility_traces->end()) iter->second += trace_increment;
				else (*data->eligibility_traces)[(production *) c->first] = trace_increment;
			}
		}
	}

	free_list(thisAgent, data->prev_op_RL_rules);
	data->prev_op_RL_rules = NIL;

	
	float update = compute_temp_diff(thisAgent, data, op_value);
	
	/* For each prod in map, add alpha*delta*trace to value */
	
	for (iter = data->eligibility_traces->begin() ; iter != data->eligibility_traces->end() ; iter++){
		
		production *prod = iter->first;
		float temp = get_number_from_symbol(rhs_value_to_symbol(prod->action_list->referent));
		temp += update*thisAgent->alpha*iter->second;		// iter->second is prod's eligibility trace

		/* Change value of rule. */
		symbol_remove_ref(thisAgent, rhs_value_to_symbol(prod->action_list->referent));
		prod->action_list->referent = symbol_to_rhs_value(make_float_constant(thisAgent, temp));

		/* Change value of preferences generated by current instantiations of this rule. */
		if (prod->instantiations){
			for (instantiation *inst = prod->instantiations ; inst ; inst = inst->next){
				for (preference *pref = inst->preferences_generated ; pref ; pref = pref->inst_next){
					symbol_remove_ref(thisAgent, pref->referent);
					pref->referent = make_float_constant(thisAgent, temp);
					}
			}
		}	
	}
	data->reward = 0.0;
	data->step = 0;
	data->impasse_type = NONE_IMPASSE_TYPE;
}

/***************************************************************
		Store RL data

Store and update data that will be needed later to perform a Bellman update
for the current operator. 
- Store the estimate of the current operator's Q-value.
- Generate RL rules from instantiated template rules.
- Update eligibility traces.

This is called after running the decision procedure and only if
an operator has been selected.

***************************************************************/

void store_RL_data(agent *thisAgent, Symbol *goal, preference *cand)
{
	RL_data *data = goal->id.RL_data;
	Symbol *op = cand->value;
    data->previous_Q = cand->numeric_value;
//	SoarSTLETMap::iterator iter;
//	list *prods_to_update = NIL;
 
	
	/* Make list of just-fired prods */
	
	for (preference *pref = goal->id.operator_slot->preferences[NUMERIC_INDIFFERENT_PREFERENCE_TYPE]; pref ; pref = pref->next){
		  if (op == pref->value){
			  instantiation *ist = pref->inst;
			  production *prod = ist->prod;
			  if (prod->RL) {
				  push(thisAgent, prod, data->prev_op_RL_rules);
			  } else if (prod->type == TEMPLATE_PRODUCTION_TYPE){
				  production *new_prod = build_production(thisAgent, pref);
				  if (new_prod){
					  push(thisAgent, new_prod, data->prev_op_RL_rules);
				  }
			  }
		  }
	}
}
	

/*****************************************************************
	Reset RL

Called when reinforcement learning is turned off.
*****************************************************************/

void reset_RL(agent *thisAgent){
	Symbol *goal = thisAgent->top_goal;
	while(goal){
		  RL_data *data = goal->id.RL_data;
		
		  data->eligibility_traces->clear(); 

		  free_list(thisAgent, data->prev_op_RL_rules);
		  data->prev_op_RL_rules = NIL;
 
		  data->reward = 0;
		  data->step = 0;
		  data->previous_Q = 0;
		  data->impasse_type = NONE_IMPASSE_TYPE;
		  goal = goal->id.lower_goal;
	}
}

/*********************************************
	Build production

Used to build RL rules from template rules. 
Takes a numeric preference generated by a 
template rule, builds an RL rule with constants
taken from the instantiation that generated the
preference.
If RL rule built already exists in rete, returns NIL.

************************************************/

production *build_production(agent *thisAgent, preference *pref)
{

	action *a;
	Symbol *prod_name;
	production *prod;
	Bool chunk_var;
	instantiation *inst = pref->inst;


	if ((pref->referent->common.symbol_type != INT_CONSTANT_SYMBOL_TYPE) && (pref->referent->common.symbol_type != FLOAT_CONSTANT_SYMBOL_TYPE)) return NIL;

	condition *cond_top, *cond_bottom;
	copy_condition_list(thisAgent, inst->top_of_instantiated_conditions, &cond_top, &cond_bottom);
	
	add_goal_or_impasse_tests_to_conds(thisAgent, cond_top);

	// Variablize
	chunk_var = thisAgent->variablize_this_chunk;
	thisAgent->variablize_this_chunk = TRUE;
	reset_variable_generator(thisAgent, cond_top, NIL);
	thisAgent->variablization_tc = get_new_tc_number(thisAgent);
	variablize_condition_list(thisAgent, cond_top);
	variablize_nots_and_insert_into_conditions(thisAgent, inst->nots, cond_top);
	

	// Make action list
	a = make_simple_action(thisAgent, pref->id, pref->attr, pref->value, pref->referent);
	a->preference_type = NUMERIC_INDIFFERENT_PREFERENCE_TYPE;


	// Make production
	prod_name = generate_new_sym_constant(thisAgent, "RL-", &thisAgent->RL_count);
	prod = make_production(thisAgent, USER_PRODUCTION_TYPE, prod_name, &cond_top, &cond_bottom, &a, FALSE);
	thisAgent->variablize_this_chunk = chunk_var;
	if (add_production_to_rete(thisAgent, prod, cond_top, 0, FALSE) == DUPLICATE_PRODUCTION){
		   excise_production(thisAgent, prod, FALSE);
		   (thisAgent->RL_count)--;
		   prod = NIL;
	}
	deallocate_condition_list(thisAgent,cond_top);
	return prod;
}

/* Utility function for build_production */
void add_goal_or_impasse_tests_to_conds(agent *thisAgent, condition * all_conds)
{
    condition *cond;
    tc_number tc;               /* mark each id as we add a test for it, so we don't add
                                   a test for the same id in two different places */
    Symbol *id;
    test t;
    complex_test *ct;

    tc = get_new_tc_number(thisAgent);
    for (cond = all_conds; cond != NIL; cond = cond->next) {
        if (cond->type != POSITIVE_CONDITION)
            continue;
        id = referent_of_equality_test(cond->data.tests.id_test);
        if ((id->id.isa_goal || id->id.isa_impasse) && (id->id.tc_num != tc)) {
            allocate_with_pool(thisAgent, &thisAgent->complex_test_pool, &ct);
            ct->type = (char) ((id->id.isa_goal) ? GOAL_ID_TEST : IMPASSE_ID_TEST);
            t = make_test_from_complex_test(ct);
            add_new_test_to_test(thisAgent, &(cond->data.tests.id_test), t);
            id->id.tc_num = tc;
        }
    }
}

/* Utility function for build_production. */
action *make_simple_action(agent *thisAgent, Symbol *id_sym, Symbol *attr_sym, Symbol *val_sym, Symbol *ref_sym)
{
    action *rhs;
    Symbol *temp;

    allocate_with_pool (thisAgent, &thisAgent->action_pool,  &rhs);
    rhs->next = NIL;
    rhs->type = MAKE_ACTION;

    // id
	temp = id_sym;
	symbol_add_ref(temp);
	variablize_symbol(thisAgent, &temp);
	rhs->id = symbol_to_rhs_value(temp);


    // attribute
    temp = attr_sym;
	symbol_add_ref(temp);
	variablize_symbol(thisAgent, &temp);
	rhs->attr = symbol_to_rhs_value(temp);

	// value
	temp = val_sym;
	symbol_add_ref (temp);
	variablize_symbol (thisAgent, &temp);
	rhs->value = symbol_to_rhs_value (temp);

	// referent
	temp = ref_sym;
	symbol_add_ref(temp);
	variablize_symbol(thisAgent, &temp);
	rhs->referent = symbol_to_rhs_value(temp);

    return rhs;

}//make_simple_action


/***********************************************************
	Check prefs for RL

Checks production RHS to see if rule could be an RL rule.
Returns true if production has a single numeric indifferent preference.

*************************************************************/

bool check_prefs_for_RL(production *prod){
	bool numeric_prefs = FALSE;
	int num_actions = 0;

	for (action *a = prod->action_list ; a ; a = a->next) {
		num_actions++;
		if (a->type != MAKE_ACTION) continue;
		if (a->preference_type == NUMERIC_INDIFFERENT_PREFERENCE_TYPE) numeric_prefs = TRUE;
	}

    if (!numeric_prefs) return FALSE;	
	else if (num_actions > 1) return FALSE;
	else return TRUE;

}

/* Currently unused function */
Bool check_template_for_RL(production *prod){
	int num_actions = 0;

	for (action *a = prod->action_list ; a ; a = a->next) num_actions++;
	if (num_actions != 1) return FALSE;
	if (prod->action_list->type != MAKE_ACTION) return FALSE;
	if (!preference_is_binary(prod->action_list->preference_type)) return FALSE;

	return TRUE;
}
 
/********************************************************
		remove RL refs for prod

Pointers to RL rules that are scheduled to be updated are
kept in the eligibility traces map. When a rule is excised,
these pointers must be removed.
********************************************************/

void remove_RL_refs_for_prod(agent *thisAgent, production *prod){
	for (Symbol* state = thisAgent->top_state ; state ; state = state->id.lower_goal){
		state->id.RL_data->eligibility_traces->erase(prod);
		for (cons *c = state->id.RL_data->prev_op_RL_rules ; c ; c = c->rest)
			if (static_cast<production *>(c->first) == prod) c->first = NIL;
	}
}


 

/*
void modify_corresponding_template_pref(instantiation *RL_inst){
	production *template_prod = RL_inst->prod->template_parent;
	if (!template_prod) return;
	
	for (instantiation *template_inst = template_prod->instantiations ; template_inst ; template_inst = template_inst->next){
		// Does this inst of the template correspond to the RL rule?
		bool insts_same = true;
		condition *RL_cond = RL_inst->top_of_instantiated_conditions;
		condition *template_cond = template_inst->top_of_instantiated_conditions;

		while (RL_cond){
			if (conditions_are_equal(RL_cond, template_cond)){
				RL_cond = RL_cond->next;
				template_cond = template_cond->next;
			} else {
				insts_same = false;
				break;
			}
		}

		if (insts_same){
			template_inst->preferences_generated->type = TEMPLATE_PREFERENCE_TYPE;
			break;
		}
	}
}*/


float get_number_from_symbol(Symbol *sym){
	if (sym->common.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE){
			return sym->fc.value;
		} else if (sym->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE){
			return sym->ic.value;
		}  
	return 0;
}

