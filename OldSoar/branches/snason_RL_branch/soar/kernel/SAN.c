#include "soarkernel.h"
 

float compute_Q_value();
Symbol *equality_test_for_symbol_in_test(test t);
condition *make_simple_condition(Symbol *id_sym, Symbol *attr_sym, Symbol *val_sym);
action *make_simple_action(Symbol *id_sym, Symbol *attr_sym, Symbol *val_sym, Symbol *ref_sym);
production *build_RL_production(condition *top_cond, condition *bottom_cond, not *nots, preference *pref, wme *w);
void learn_RL_productions(int level);
void record_for_RL();
void variablize_condition_list(condition * cond);
void variablize_nots_and_insert_into_conditions(not * nots, condition * conds);
action *copy_and_variablize_result_list(preference * pref);
void variablize_symbol(Symbol ** sym);
void SAN_add_goal_or_impasse_tests(condition * all_conds);
void trace_to_state(wme * w, tc_number tc_num, condition ** cond);
bool symbol_is_in_tc(Symbol * sym, tc_number tc);


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

void record_for_RL()
{
	wme *chosenOp, *w;
	slot *s, *t;
	instantiation *ist;
	production *prod;
	preference *pref;
	condition *cond_top, *cond_bottom, *new_top, *new_bottom;
	RL_record *record;
	Symbol *sym;
	tc_number tc_num;

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
			  if (prod->times_applied > 5){
				  list *identifiers = 0;
				  cons *c;
				  prod->times_applied = 0;
				  tc_num = get_new_tc_number();
				  /*for (cond = ist->top_of_instantiated_conditions; cond ; cond = cond->next){
					  if (cond->type == POSITIVE_CONDITION){
						  sym = equality_test_for_symbol_in_test(cond->data.tests.id_test);
						  if (sym) identifiers = add_if_not_member(sym, identifiers);
						  sym = equality_test_for_symbol_in_test(cond->data.tests.attr_test);
						  if (sym) identifiers = add_if_not_member(sym, identifiers);
						  sym = equality_test_for_symbol_in_test(cond->data.tests.value_test);
						  if (sym) identifiers = add_if_not_member(sym, identifiers);
					  }
				  }*/
				  for (cond_top = ist->top_of_instantiated_conditions; cond_top ; cond_top = cond_top->next){
					  if (cond_top->type == POSITIVE_CONDITION){
						  sym = equality_test_for_symbol_in_test(cond_top->data.tests.id_test);
						  if (sym) add_symbol_to_tc(sym, tc_num, NIL, NIL);
						  sym = equality_test_for_symbol_in_test(cond_top->data.tests.attr_test);
						  if (sym) add_symbol_to_tc(sym, tc_num, NIL, NIL);
						  sym = equality_test_for_symbol_in_test(cond_top->data.tests.value_test);
						  if (sym) add_symbol_to_tc(sym, tc_num, NIL, NIL);
					}
				  }
				  prod = 0;
				  cond_top = 0;
				  /*for (c = identifiers; c ; c=c->rest){
					  if (prod) break;
					  sym = (Symbol *) c->first;
					  for (t = sym->id.slots ; t ; t=t->next){
						  if (prod) break;
						  for (w = t->wmes ; w ; w=w->next){
							  prod = build_RL_production(ist->top_of_instantiated_conditions, ist->nots, ist->preferences_generated, w);
							  if (prod) break;
						  }
					  }
				  }
				  */
				  for (w = current_agent(all_wmes_in_rete) ; w ; w = w->rete_next){
					  	condition *c, *new_cond = make_simple_condition(w->id, w->attr, w->value);
							for (c = ist->top_of_instantiated_conditions ; c ; c = c->next){
								if (conditions_are_equal(c, new_cond)){
									deallocate_condition_list(new_cond);
									new_cond = NIL;
									break;
								}
							}
						if (!new_cond) continue;	
						deallocate_condition_list(new_cond);
						trace_to_state(w, tc_num, &cond_top);
						break;
				  }
				  for (cond_bottom = cond_top ; cond_bottom->next; cond_bottom = cond_bottom->next);
				  copy_condition_list(ist->top_of_instantiated_conditions, &new_top, &new_bottom);
				  cond_top->prev = new_bottom;
				  new_bottom->next = cond_top;
				  prod = build_RL_production(new_top, cond_bottom, ist->nots, ist->preferences_generated, w);
				  if (prod){
					  push(prod, record->pointer_list);
					  record->num_prod++;
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
 //			prod->type = RL_PRODUCTION_TYPE;

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

production *build_RL_production(condition *top_cond, condition *bottom_cond, not *nots, preference *pref, wme *w)
{
 
	action *a;
	Symbol *prod_name;
	production *prod;
	bool chunk_var;

	
	// Make condition list
	/*new_cond = make_simple_condition(w->id, w->attr, w->value);
    for (c = top_cond ; c ; c = c->next){
		if (conditions_are_equal(c, new_cond)){
			deallocate_condition_list(new_cond);
			return NIL;
		}
	}
	copy_condition_list(top_cond, &new_top, &new_bottom);
	new_bottom->next = new_cond;
	new_cond->prev = new_bottom;
	*/
	SAN_add_goal_or_impasse_tests(top_cond);
 	

	// Variablize
	chunk_var = current_agent(variablize_this_chunk);
	current_agent(variablize_this_chunk) = TRUE;
	reset_variable_generator(top_cond, NIL);
	current_agent(variablization_tc) = get_new_tc_number();
	variablize_condition_list(top_cond);
	variablize_nots_and_insert_into_conditions(nots, top_cond);

	// Make action list
	a = make_simple_action(pref->id, pref->attr, pref->value, make_float_constant(0));
	a->preference_type = NUMERIC_INDIFFERENT_PREFERENCE_TYPE;
	

	// Make production
	prod_name = generate_new_sym_constant("RL-", &current_agent(RL_count));
	prod = make_production(RL_PRODUCTION_TYPE, prod_name, &top_cond, &bottom_cond, &a, FALSE);
	current_agent(variablize_this_chunk) = chunk_var;
	if (add_production_to_rete(prod, top_cond, 0, FALSE) == DUPLICATE_PRODUCTION){
		   excise_production(prod, FALSE);
		   prod = NIL;
	} 
	// deallocate_condition_list(new_top);
	return prod;
}


/*************/
action *make_simple_action(Symbol *id_sym, Symbol *attr_sym, Symbol *val_sym, Symbol *ref_sym)
{
    action *rhs;
    Symbol *temp;

    allocate_with_pool (&current_agent(action_pool),  &rhs);
    rhs->next = NIL;
    rhs->type = MAKE_ACTION;

    // id
	temp = id_sym;
	symbol_add_ref(temp);
	variablize_symbol(&temp);
	rhs->id = symbol_to_rhs_value(temp);


    // attribute
    temp = attr_sym;
	symbol_add_ref(temp);
	variablize_symbol(&temp);
	rhs->attr = symbol_to_rhs_value(temp);

	// value
	temp = val_sym;
	symbol_add_ref (temp);
	variablize_symbol (&temp);
	rhs->value = symbol_to_rhs_value (temp);

	// referent
	temp = ref_sym;
	symbol_add_ref(temp);
	variablize_symbol(&temp);
	rhs->referent = symbol_to_rhs_value(temp);

    return rhs;

}//make_simple_action


condition *make_simple_condition(Symbol *id_sym,
                                 Symbol *attr_sym,
                                 Symbol *val_sym)
{
    condition *newcond;

    allocate_with_pool (&current_agent(condition_pool),  &newcond);
    newcond->type = POSITIVE_CONDITION;
    newcond->next = NULL;
    newcond->prev = NULL;

    if (strcmp(attr_sym->sc.name, "operator") == 0)
    {
        newcond->test_for_acceptable_preference = TRUE;
    }
    else
    {
        newcond->test_for_acceptable_preference = FALSE;
    }

    newcond->data.tests.id_test = make_equality_test(id_sym);
    newcond->data.tests.attr_test = make_equality_test(attr_sym);

    /*if (val_sym->common.symbol_type == IDENTIFIER_SYMBOL_TYPE)
    {
        char buf[100];
        buf[0] = val_sym->id.name_letter;
        buf[1] = ''\0'';
        val_sym = generate_new_variable(buf);
    }*/
    newcond->data.tests.value_test = make_equality_test(val_sym);

    return newcond;

}//make_simple_condition

void SAN_add_goal_or_impasse_tests(condition * all_conds)
{
    condition *cond;
    tc_number tc;               /* mark each id as we add a test for it, so we don't add
                                   a test for the same id in two different places */
    Symbol *id;
    test t;
    complex_test *ct;

    tc = get_new_tc_number();
    for (cond = all_conds; cond != NIL; cond = cond->next) {
        if (cond->type != POSITIVE_CONDITION)
            continue;
        id = referent_of_equality_test(cond->data.tests.id_test);
        if ((id->id.isa_goal || id->id.isa_impasse) && (id->id.tc_num != tc)) {
            allocate_with_pool(&current_agent(complex_test_pool), &ct);
            ct->type = (char) ((id->id.isa_goal) ? GOAL_ID_TEST : IMPASSE_ID_TEST);
            t = make_test_from_complex_test(ct);
            add_new_test_to_test(&(cond->data.tests.id_test), t);
            id->id.tc_num = tc;
        }
    }
}

void trace_to_state(wme * w, tc_number tc_num, condition ** cond){
	Symbol * sym = w->id;
	condition *new_cond;
	wme * z;
	dl_cons *dc;

	new_cond = make_simple_condition(w->id, w->attr, w->value);
	if (new_cond->test_for_acceptable_preference == w->acceptable)
		insert_at_head_of_dll(*cond, new_cond, next, prev);
	if (!symbol_is_in_tc(sym, tc_num)){
		for (dc = sym->id.parents ; dc ; dc = dc->next){
			z = dc->item;
		 	trace_to_state(z, tc_num, cond);
		}
	}
}

	
