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
action *copy_and_variablize_pref_list (agent* thisAgent, preference *pref);

/**************************************************
	Tabulate reward value for goal

Given a goal id ID, sum together any rewards found under that goal's
reward link - more precisely look for numeric values R at ID.reward-link.attr.attr R.
'data->step' should hold the number of steps the current operator has 
been installed at the goal, so if there is an op no-change, data->step > 1
and the new reward is discounted and added to reward accumulated on 
previous decision cycles.

This function is called for a goal right before it is destroyed and is called
for the top state right before a 'halt'. It is also called once for each goal 
in the goal stack at the beginning of the decision phase.

**************************************************/
void tabulate_reward_value_for_goal(agent *thisAgent, Symbol *goal){
	RL_data *data = goal->id.RL_data;
	if (data->impasse_type == NONE_IMPASSE_TYPE) {} /* Only count rewards at top state or for op no-change impasses. */
	else if (data->impasse_type == OP_NO_CHANGE_IMPASSE_TYPE) {}
	else return;
	slot *s = goal->id.reward_header->id.slots;
	float reward = 0.0;
	if (s){
		for ( ; s ; s = s->next){
			for (wme *w = s->wmes ; w ; w = w->next){
				 if (w->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE){
					for (slot *t = w->value->id.slots ; t ; t = t->next){
						for (wme *x = t->wmes ; x ; x = x->next){
							if (x->value->common.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE){
								reward = reward + x->value->fc.value;
								if (x->preference->o_supported){
									wme *new_wme = make_wme(thisAgent, x->id, make_sym_constant(thisAgent, "status"), make_sym_constant(thisAgent, "read"), FALSE);
									insert_at_head_of_dll (x->id->id.input_wmes, new_wme, next, prev);
									add_wme_to_wm (thisAgent, new_wme);
								}
							} else if (x->value->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE){
								reward = reward + x->value->ic.value;
								if (x->preference->o_supported){
									wme *new_wme = make_wme(thisAgent, x->id, make_sym_constant(thisAgent, "status"), make_sym_constant(thisAgent,"read"), FALSE);
									insert_at_head_of_dll (x->id->id.input_wmes, new_wme, next, prev);
									add_wme_to_wm (thisAgent, new_wme);
								}
							}
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

Computes a TD update. Takes as input the current reward (r_t+1),
max Q at the current time step (Q_t+1) , and the estimate of the
Q-value of the last operator selected (Q_t). 
Discounting uses the gamma parameter and a count of the
time steps since the last operator was selected.
Returns r_t+1 + gamma*Q_t+1 - Q_t.
**************************************************/

float compute_temp_diff(agent *thisAgent, RL_data* r, float op_value){
	
	float delta_Q = r->reward;
	delta_Q += pow(thisAgent->gamma, r->step)*op_value;
	delta_Q -= r->previous_Q;
	return delta_Q;
}

/***************************************************
		Perform Bellman update

If we have RL rules to update for the last operator
selected, then compute a TD update r, and change the
numeric preference for each rule by 
(r/(number of RL rules to be updated))*learning rate.

If any of the rules changed are currently matching
and asserting their numeric preference, we need to
update the value stored in the current preference
as well as the value in the rule. In this case, this
function returns current_pref_changed=TRUE.

Finally, we reset all the RL_data in preparation for
the next operator.
***************************************************/

void perform_Bellman_update(agent *thisAgent, float op_value, Symbol *goal){
	
	cons *c;
	RL_data *data = goal->id.RL_data;
	eligibility_trace_element *trace = data->current_eligibility_element;
 //	bool current_pref_changed = FALSE;

/* Eligibility trace */
	if (trace){
    float sum = 0;
	/*for (c = trace->prods_to_update ; c ; c = c->rest) {            // Iterate over most recently fired RL rules 
		production *prod = (production *) c->first;
		if (!prod) continue;
		Symbol *s = rhs_value_to_symbol(prod->action_list->referent);
				if (s->common.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE) {
					sum += s->fc.value;
				} else if (s->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE) {
					sum += s->ic.value;
				} else {
					// this should never happen
					continue;
				}
			}
	data->previous_Q = sum;*/

	float update = compute_temp_diff(thisAgent, data, op_value);

	

	for (int i = 0 ; i<data->number_in_list ; i++){

		// int num_prods = 0;
		// for (c = trace->prods_to_update; c ; c = c->rest){
		//	if (c->first)
		//		num_prods++;
		//}
		
		if (trace->num_prods > 0){
			float increment = thisAgent->alpha*(update / trace->num_prods)*pow(thisAgent->lambda, i)*pow(thisAgent->gamma, i);
			c = trace->prods_to_update;
			while(c){
			
					production *prod = (production *) c->first;
					c = c->rest;
		
					if (!prod) continue;
	
					float temp;
					if (rhs_value_to_symbol(prod->action_list->referent)->common.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE){
						temp = rhs_value_to_symbol(prod->action_list->referent)->fc.value;
					} else if (rhs_value_to_symbol(prod->action_list->referent)->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE){
						temp = rhs_value_to_symbol(prod->action_list->referent)->ic.value;
					} else {
					// We should never get here. Need to return an error here.
					}
				
					temp += increment;
	
					/* Change value of rule. */
					symbol_remove_ref(thisAgent, rhs_value_to_symbol(prod->action_list->referent));
					prod->action_list->referent = symbol_to_rhs_value(make_float_constant(thisAgent, temp));
				
					/* Change value of preferences generated by current instantiations of this rule. */
					if (prod->instantiations){
						// current_pref_changed = TRUE;
						for (instantiation *inst = prod->instantiations ; inst ; inst = inst->next){
							for (preference *pref = inst->preferences_generated ; pref ; pref = pref->inst_next){
								symbol_remove_ref(thisAgent, pref->referent);
								pref->referent = make_float_constant(thisAgent, temp);
							}
						}
					}	 
			}
		}

		trace = trace->previous;
	}
}
	
	data->reward = 0.0;
	data->step = 0;
	data->impasse_type = NONE_IMPASSE_TYPE;


/* End Eligibility trace */


	



//	int num_prods = 0;
//	for (c = data->productions_to_be_updated; c ; c = c->rest){
//		if (c->first)
//			num_prods++;
//	}
// 	
//	
//	if (num_prods > 0){  // if there are productions to update
//		/* Compute Q-value of previous operator based on the current values of rules that fired for it. */
//		// if (data->impasse_type != NONE_IMPASSE_TYPE){
//			float sum = 0;
//			for (c = data->productions_to_be_updated ; c ; c = c->rest){
//				Symbol *s = rhs_value_to_symbol(((production *) c->first)->action_list->referent);
//				if (s->common.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE) {
//					sum += s->fc.value;
//				} else if (s->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE) {
//					sum += s->ic.value;
//				} else {
//					// this should never happen
//					continue;
//				}
//			}
//			data->previous_Q = sum;
//		// }
//
//		float update = compute_temp_diff(thisAgent, data, op_value);
//		float increment = thisAgent->alpha*(update / num_prods);
//		c = data->productions_to_be_updated;
//			while(c){
//		 
//				production *prod = (production *) c->first;
//				c = c->rest;
//	 
//				if (!prod) continue;
//
//				prod->copies_awaiting_updates--;
//	  		 		
//				float temp;
//				if (rhs_value_to_symbol(prod->action_list->referent)->common.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE){
//					temp = rhs_value_to_symbol(prod->action_list->referent)->fc.value;
//				} else if (rhs_value_to_symbol(prod->action_list->referent)->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE){
//					temp = rhs_value_to_symbol(prod->action_list->referent)->ic.value;
//				} else {
//				 // We should never get here. Need to return an error here.
//				}
//			 
//				temp += increment;
//
//				/* Change value of rule. */
//				symbol_remove_ref(thisAgent, rhs_value_to_symbol(prod->action_list->referent));
//				prod->action_list->referent = symbol_to_rhs_value(make_float_constant(thisAgent, temp));
//			 
//				/* Change value of preferences generated by current instantiations of this rule. */
//				if (prod->instantiations){
//					// current_pref_changed = TRUE;
//					for (instantiation *inst = prod->instantiations ; inst ; inst = inst->next){
//						for (preference *pref = inst->preferences_generated ; pref ; pref = pref->inst_next){
//							symbol_remove_ref(thisAgent, pref->referent);
//							pref->referent = make_float_constant(thisAgent, temp);
//						}
//					}
//				}	 
//		}
//	}
//
//	data->reward = 0.0;
//	data->step = 0;
//	// data->previous_Q = 0;
//	data->impasse_type = NONE_IMPASSE_TYPE;
//	free_list(thisAgent, data->productions_to_be_updated);
//	data->productions_to_be_updated = NIL;
//	
//	// return current_pref_changed;
//	return TRUE;
}

/*************************************************************
       RL update symbolically chosen

When an operator is chosen probabilistically, we perform a TD
update with max Q (since we do Q-learning). But when the operator
is chosen with symbolic preferences, we always update with the
value of the selected operator. That value is computed here and
used in a call to perform_Bellman_update.
*************************************************************/

void RL_update_symbolically_chosen(agent *thisAgent, slot *s, preference *candidates){
	if (!candidates) return;
	float temp_Q = 0;   // DEFAULT_INDIFFERENT_VALUE;
	
	for (preference *temp=s->preferences[NUMERIC_INDIFFERENT_PREFERENCE_TYPE]; temp!=NIL; temp=temp->next){
		if (candidates->value == temp->value){
			if (temp->referent->common.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE){
				temp_Q += temp->referent->fc.value;
			} else if (temp->referent->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE){
				temp_Q += temp->referent->ic.value;
			} else {
				/* This shhould never happen. */
			}
		}
	}
	candidates->numeric_value = temp_Q; // store estimate of Q-value to be used to update this operator on the next decision cycle
	perform_Bellman_update(thisAgent, temp_Q, s->id);
}

/***************************************************************
		Store RL data

Store data that will be needed later to perform a Bellman update
for the current operator. This includes the current estimate of
the operator's Q-value, and a list of pointers to RL rules that
fired proposing numeric preferences for the operator.

This is called after running the decision procedure and only if
an operator has been selected.

***************************************************************/

void store_RL_data(agent *thisAgent, Symbol *goal, preference *cand)
{
	RL_data *data = goal->id.RL_data;
	Symbol *op = cand->value;
    data->previous_Q = cand->numeric_value;

/* Eligibility trace */
	if (data->number_in_list == 0){ // Eligibility trace list is empty
		data->number_in_list++;
		eligibility_trace_element *ete = static_cast<eligibility_trace_element_struct *>(allocate_memory(thisAgent, sizeof(eligibility_trace_element_struct),
												   MISCELLANEOUS_MEM_USAGE));
		ete->prods_to_update = NIL;
		ete->num_prods = 0;
		// data->productions_to_be_updated = dc;
		// if (data->number_in_list == thisAgent->num_traces){
		ete->next = ete;
		ete->previous = ete;
		// } else dc->next = NIL;
		data->current_eligibility_element = ete;
	} else if (data->number_in_list < thisAgent->num_traces) {	// Add a new element to the end of productions_to_be_updated
		data->number_in_list++;
		eligibility_trace_element *ete = static_cast<eligibility_trace_element_struct *>(allocate_memory(thisAgent, sizeof(eligibility_trace_element_struct),
												   MISCELLANEOUS_MEM_USAGE));;
		ete->prods_to_update = NIL;
		ete->num_prods = 0;
    	ete->next = data->current_eligibility_element->next;
		data->current_eligibility_element->next = ete;
		ete->previous = data->current_eligibility_element;
		ete->next->previous = ete;
		data->current_eligibility_element = ete;
	} else {
		data->current_eligibility_element = data->current_eligibility_element->next;
		while (data->number_in_list > thisAgent->num_traces) {
			eligibility_trace_element *temp = data->current_eligibility_element;
			for (cons *c = temp->prods_to_update ; c ; c=c->rest){
				if (c->first)
				((production *) c->first)->copies_awaiting_updates--;
			}
			free_list(thisAgent, temp->prods_to_update);
			temp->previous->next = temp->next;
			temp->next->previous = temp->previous;
			data->current_eligibility_element = temp->next;
			free_memory(thisAgent, temp, MISCELLANEOUS_MEM_USAGE);
			data->number_in_list--;
		}
		for (cons *c = data->current_eligibility_element->prods_to_update ; c ; c=c->rest){
			if (c->first)
				((production *) c->first)->copies_awaiting_updates--;
		}
		free_list(thisAgent, data->current_eligibility_element->prods_to_update);
		data->current_eligibility_element->num_prods = 0;
		data->current_eligibility_element->prods_to_update = NIL;
	}

	for (preference *pref = goal->id.operator_slot->preferences[NUMERIC_INDIFFERENT_PREFERENCE_TYPE]; pref ; pref = pref->next){
			  if (op == pref->value){
				  instantiation *ist = pref->inst;
				  production *prod = ist->prod;
				  if (prod->RL) {
					  push(thisAgent, prod, data->current_eligibility_element->prods_to_update);
					  data->current_eligibility_element->num_prods++;
					  prod->copies_awaiting_updates++;
				  } else if (prod->type == TEMPLATE_PRODUCTION_TYPE){
					  production *new_prod = build_production(thisAgent, pref->inst->top_of_instantiated_conditions, pref->inst->nots, pref);
					  if (new_prod){
						push(thisAgent, new_prod, data->current_eligibility_element->prods_to_update);
						data->current_eligibility_element->num_prods++;
						new_prod->copies_awaiting_updates++;
					  }
				  }
			}
	}
		  
/*	for (preference *pref = goal->id.operator_slot->preferences[TEMPLATE_PREFERENCE_TYPE]; pref ; pref = pref->next){
		if (op == pref->value){
          production *prod = build_production(thisAgent, pref->inst->top_of_instantiated_conditions, pref->inst->nots, pref);
			if (prod){
				push(thisAgent, prod, data->current_eligibility_element->prods_to_update);
				data->current_eligibility_element->num_prods++;
				prod->copies_awaiting_updates++;
			}
		}
	}*/



//	for (preference *pref = goal->id.operator_slot->preferences[NUMERIC_INDIFFERENT_PREFERENCE_TYPE]; pref ; pref = pref->next){
//			  if (op == pref->value){
//				  instantiation *ist = pref->inst;
//				  production *prod = ist->prod;
//				  if (prod->RL) {
//					  push(thisAgent, prod, data->productions_to_be_updated);
//					  prod->copies_awaiting_updates++;
//				  } 
//			  }
//	}

//	for (preference *pref = goal->id.operator_slot->preferences[TEMPLATE_PREFERENCE_TYPE]; pref ; pref = pref->next){
//		if (op == pref->value){
//          production *prod = build_production(thisAgent, pref->inst->top_of_instantiated_conditions, pref->inst->nots, pref);
//			if (prod){
//				prod->copies_awaiting_updates++;
//				push(thisAgent, prod, data->productions_to_be_updated);
//			}
//		}
//	}
}

/*****************************************************************
	Reset RL

Called when reinforcement learning is turned off.
*****************************************************************/

void reset_RL(agent *thisAgent){
	Symbol *goal = thisAgent->top_goal;
	while(goal){
		  RL_data *data = goal->id.RL_data;
		//  free_list(thisAgent, data->productions_to_be_updated);
		  // data->productions_to_be_updated = NIL;
		  /* Eligibility trace */
			eligibility_trace_element *traces = data->current_eligibility_element;
		  	for (int i = 0 ; i<data->number_in_list ; i++){
				for (cons *c = traces->prods_to_update ; c ; c=c->rest){
					if (c->first)
						((production *) c->first)->copies_awaiting_updates--;
				}
				free_list(thisAgent, traces->prods_to_update);
				eligibility_trace_element *temp = traces;
				traces = traces->next;
				free_memory(thisAgent, temp, MISCELLANEOUS_MEM_USAGE);
			}
		  data->current_eligibility_element = NIL;
		  data->number_in_list = 0;
		  /* End Eligibility trace */
		  data->reward = 0;
		  data->step = 0;
		  data->previous_Q = 0;
		  data->impasse_type = NONE_IMPASSE_TYPE;
		  goal = goal->id.lower_goal;
	}
}

production *build_production(agent *thisAgent, condition *top_cond, not_struct *nots, preference *pref)
{

	action *a;
	Symbol *prod_name;
	production *prod;
	Bool chunk_var;


	if ((pref->referent->common.symbol_type != INT_CONSTANT_SYMBOL_TYPE) && (pref->referent->common.symbol_type != FLOAT_CONSTANT_SYMBOL_TYPE)) return NIL;

	condition *cond_top, *cond_bottom;
	copy_condition_list(thisAgent, top_cond, &cond_top, &cond_bottom);
	
	add_goal_or_impasse_tests_to_conds(thisAgent, cond_top);

	// Variablize
	chunk_var = thisAgent->variablize_this_chunk;
	thisAgent->variablize_this_chunk = TRUE;
	reset_variable_generator(thisAgent, cond_top, NIL);
	thisAgent->variablization_tc = get_new_tc_number(thisAgent);
	variablize_condition_list(thisAgent, cond_top);
	variablize_nots_and_insert_into_conditions(thisAgent, nots, cond_top);
	// a = copy_and_variablize_pref_list (thisAgent, pref);

	

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

void check_prefs_for_RL(production *prod){
	bool numeric_prefs = FALSE;
	int num_actions = 0;

	for (action *a = prod->action_list ; a ; a = a->next) {
		num_actions++;
		if (a->type != MAKE_ACTION) continue;
		if (a->preference_type == NUMERIC_INDIFFERENT_PREFERENCE_TYPE) numeric_prefs = TRUE;
	}

	if (!numeric_prefs) prod->RL = FALSE;
	else if (num_actions > 1) prod->RL = FALSE;
	else prod->RL = TRUE;

}

Bool check_template_for_RL(production *prod){
	int num_actions = 0;

	for (action *a = prod->action_list ; a ; a = a->next) num_actions++;
	if (num_actions != 1) return FALSE;
	if (prod->action_list->type != MAKE_ACTION) return FALSE;
	if (!preference_is_binary(prod->action_list->preference_type)) return FALSE;

	return TRUE;
}

action *copy_and_variablize_pref_list (agent* thisAgent, preference *pref) {
  action *a;
  Symbol *temp;
  
  if (!pref) return NIL;
  allocate_with_pool (thisAgent, &thisAgent->action_pool, &a);
  a->type = MAKE_ACTION;

  temp = pref->id;
  symbol_add_ref (temp);
  variablize_symbol (thisAgent, &temp);
  a->id = symbol_to_rhs_value (temp);

  temp = pref->attr;
  symbol_add_ref (temp);
  variablize_symbol (thisAgent, &temp);
  a->attr = symbol_to_rhs_value (temp);

  temp = pref->value;
  symbol_add_ref (temp);
  variablize_symbol (thisAgent, &temp);
  a->value = symbol_to_rhs_value (temp);

  a->preference_type = pref->type;

  if (preference_is_binary(pref->type)) {
    temp = pref->referent;
    symbol_add_ref (temp);
    variablize_symbol (thisAgent, &temp);
    a->referent = symbol_to_rhs_value (temp);
  }
  
  a->next = copy_and_variablize_pref_list (thisAgent, pref->inst_next);
  return a;  
}

void remove_update_refs_for_prod(agent *thisAgent, production *prod){
    int refs_remaining = prod->copies_awaiting_updates;
	for (Symbol* state = thisAgent->top_state ; state ; state = state->id.lower_goal){
		eligibility_trace_element *traces = state->id.RL_data->current_eligibility_element;
		for (int i = 0 ; i<state->id.RL_data->number_in_list ; i++){
			 for (cons* c = traces->prods_to_update ; c ; c = c->rest){
				 if ((production *) c->first == prod){
					 c->first = NIL;
					 traces->num_prods--;
					 refs_remaining--;
					 if (!refs_remaining) return;
				 }
			 }
			 traces = traces->next;
		}
	}}
