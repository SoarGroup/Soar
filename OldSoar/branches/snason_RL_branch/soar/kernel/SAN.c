#include "soarkernel.h"
 

float compute_Q_value();
Symbol *equality_test_for_symbol_in_test(test t);


// The following three functions manage the stack of RL_records.
// Each level in the stack corresponds to a level in Soar's subgoaling hierarchy.
void push_record(RL_record **r, Symbol *level_sym){
	RL_record *new_record;

	new_record = malloc(sizeof(RL_record));
	if (new_record != NULL) {
		new_record->pointer_list = NIL;
		new_record->num_prod = 0;
		new_record->previous_Q = 0;
		new_record->next_Q = 0;
		new_record->reward = 0;
		new_record->step = 0;
	    new_record->op = NIL;
		new_record->goal_level = level_sym;
		symbol_add_ref(new_record->goal_level);
		new_record->level = level_sym->id.level;
	 	new_record->next = *r;
		*r = new_record;
	}
}

void pop_record(RL_record **r){
	RL_record *new_record;
 	
	new_record = *r;
    free_list(new_record->pointer_list);
	symbol_remove_ref(new_record->goal_level);
	if (new_record->op)
		symbol_remove_ref(new_record->op);
	*r = new_record->next;
	free(new_record);
}

void reset_RL(){

	while(current_agent(records))
 		pop_record(&current_agent(records));

	// current_agent(next_Q) = 0.0;
}


float compute_Q_value(RL_record* r){
	float Q;

	// print_with_symbols("\n Q value for %y\n", r->op);

	Q = r->reward;

    // print("\n Q after reward is %f\n" , Q);
	Q += pow(current_agent(gamma), r->step)*(r->next_Q);
	// print("Q after next_Q update is %f\n", Q);
	// print("\n alpha is %f\n", current_agent(alpha));
	Q -= r->previous_Q;
	// print("Q after previous %f\n", Q);
	Q *= current_agent(alpha);
	// print("Q after alpha %f\n", Q);

    if (r->num_prod > 0)
		Q = Q / r->num_prod;

	return Q;

}

float tabulate_reward_value(){
	float reward = 0.0;
	slot *s;
	wme *w;

	s = current_agent(reward_header)->id.slots;
	if (s){
		for ( ; s ; s = s->next){
			for (w = s->wmes ; w ; w = w->next){
				if (w->value->common.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE)
					reward = reward + w->value->fc.value;
				else if (w->value->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE)
					reward = reward + w->value->ic.value;
			}
		}
	}

	for (w = current_agent(reward_header)->id.input_wmes ; w ; w = w->next){
	 	if (w->value->common.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE)
				reward = reward + w->value->fc.value;
		else if (w->value->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE)
				reward = reward + w->value->ic.value;

	}
	return reward;
}




// record_for_RL
// If an operator has been selected, go through binary indifferent preferences that fired for it
// For each preference, store a pointer to the production that generated it.

void record_for_RL(){
	wme *chosenOp, *w;
	slot *s, *t;
	instantiation *ist;
	production *prod;
	preference *pref;
	condition *cond;
	RL_record *record;
	Symbol *sym;

  // SAN - catch operator ID here
  s = current_agent(bottom_goal)->id.operator_slot;
  chosenOp = s->wmes;
  if (chosenOp){

	  // print_wme(chosenOp);

	  record = current_agent(records);
   	  record->op = chosenOp->value;
	  symbol_add_ref(record->op);       // SAN ??
	  record->previous_Q = record->next_Q;

	  for (pref = s->preferences[NUMERIC_INDIFFERENT_PREFERENCE_TYPE]; pref ; pref = pref->next){
		  if (record->op == pref->value){
			  ist = pref->inst;
			  prod = ist->prod;
			  record->num_prod++;
			  prod->times_applied++;
			  push(prod, record->pointer_list);
			  if (prod->times_applied > 10){
				  list *identifiers = 0;
				  cons *c;
				  prod->times_applied = 0;
				  for (cond = ist->top_of_instantiated_conditions; cond ; cond = cond->next){
					  if (cond->type == POSITIVE_CONDITION){
						  sym = equality_test_for_symbol_in_test(cond->data.tests.id_test);
						  if (sym) identifiers = add_if_not_member(sym, identifiers);
						  sym = equality_test_for_symbol_in_test(cond->data.tests.attr_test);
						  if (sym) identifiers = add_if_not_member(sym, identifiers);
						  sym = equality_test_for_symbol_in_test(cond->data.tests.value_test);
						  if (sym) identifiers = add_if_not_member(sym, identifiers);
					  }
				  }
				  for (c = identifiers; c ; c=c->rest){
					  sym = (Symbol *) c->first;
					  for (t = sym->id.slots ; t ; t=t->next){
						  for (w = t->wmes ; w ; w=w->next){
						  }
					  }
				  }
				  free_list(identifiers);
			  }
		  }
	  }
  }
}


// Update the value on RL productions from last cycle
void learn_RL_productions(int level){
	RL_record *record;
	production *prod;
	float Q;
	cons *c;
	float increment;
	
	record = current_agent(records);

	do{

	if (record->level < level)
		return;

	if (record->op){

		Q = compute_Q_value(record);
		c = record->pointer_list;
		
		while(c){


			prod = (production *) c->first;
			c = c->rest;
 			prod->type = RL_PRODUCTION_TYPE;

			increment = rhs_value_to_symbol(prod->action_list->referent)->fc.value;
			increment += Q;
			
			prod->action_list->referent = symbol_to_rhs_value(make_float_constant(increment));
			// a->preference_type = NUMERIC_INDIFFERENT_PREFERENCE_TYPE;
			// a->referent = symbol_to_rhs_value(make_float_constant(Q));
			prod->avg_update = fabs(Q);
		//	prod->times_applied++;
		}

 			symbol_remove_ref(record->op);
		 	record->op = NIL;
			record->reward = 0.0;
			record->step = 0;
			record->num_prod = 0;
			record->previous_Q = 0;
			free_list(record->pointer_list);
			record->pointer_list = NIL;
	}

	if (record->level > level){
		pop_record(&current_agent(records));
		record = current_agent(records);
	} else
		record = record->next;


	} while(record);
}

// Modified from function test_includes_equality_test_for_symbol
// Returns a symbol in a test
Symbol *equality_test_for_symbol_in_test(test t)
{
    cons *c;
    complex_test *ct;
	Symbol *sym;

    if (test_is_blank_test(t))
        return 0;

    if (test_is_blank_or_equality_test(t)){
		sym = referent_of_equality_test(t);
		if (sym->common.symbol_type == IDENTIFIER_SYMBOL_TYPE)
			return sym;
		else return 0;
	}

    ct = complex_test_from_test(t);

    if (ct->type == CONJUNCTIVE_TEST) {
        for (c = ct->data.conjunct_list; c != NIL; c = c->rest){
            sym = equality_test_for_symbol_in_test(c->first);
			if (sym->common.symbol_type == IDENTIFIER_SYMBOL_TYPE) return sym;
		}
    }
    return 0;
}
