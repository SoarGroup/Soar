#include "soarkernel.h"
 

float compute_Q_value();
Symbol *equality_test_for_symbol_in_test(test t);
condition *make_simple_condition(Symbol *id_sym, Symbol *attr_sym, Symbol *val_sym, bool acceptable);
action *make_simple_action(Symbol *id_sym, Symbol *attr_sym, Symbol *val_sym, Symbol *ref_sym);
production *build_RL_production(condition *top_cond, condition *bottom_cond, not *nots, preference *pref, wme *w);
void learn_RL_productions(int level);
void record_for_RL();
void variablize_condition_list(condition * cond);
void variablize_nots_and_insert_into_conditions(not * nots, condition * conds);
action *copy_and_variablize_result_list(preference * pref);
void variablize_symbol(Symbol ** sym);
void SAN_add_goal_or_impasse_tests(condition * all_conds);
bool symbol_is_in_tc(Symbol * sym, tc_number tc);
production *specify_production(instantiation *ist);
void trace_to_prod(wme * w, tc_number tc_prod, condition ** cond);
bool trace_to_prod_depth(wme * w, tc_number tc_prod, tc_number tc_seen, condition ** cond, int depth);


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
// For each preference, store a pointer to the production that generated it. Later, each production
// on this list will have its RL-value updated.
// Also, if an RL-production needs to be specified, build a new production and place a pointer to it
// on this list.

void record_for_RL()
{
	wme *w;
	slot *s;
	instantiation *ist;
	production *prod, *new_prod;
	preference *pref;
 	RL_record *record;
 
  // SAN - catch operator ID here
  s = current_agent(bottom_goal)->id.operator_slot;
  w = s->wmes;
  if (w){

	  // print_wme(chosenOp);

	  record = current_agent(records);
   	  record->op = w->value;
	  symbol_add_ref(record->op);       // SAN ??
	  record->previous_Q = record->next_Q;

	  for (pref = s->preferences[NUMERIC_INDIFFERENT_PREFERENCE_TYPE]; pref ; pref = pref->next){
		  if (record->op == pref->value){
			  ist = pref->inst;
			  prod = ist->prod;
			  record->num_prod++;
			  prod->times_applied++;
			  push(prod, record->pointer_list);
			  // Potentially build new RL-production
			  if ((prod->times_applied > 50) && prod->increasing){
				  new_prod = specify_production(ist);
				  if (new_prod){
					  prod->times_applied = 0;
					  push(new_prod, record->pointer_list);
					  record->num_prod++;
				  }
			 
			  }
		  }
	  }
  }
}

production *specify_production(instantiation *ist){
	tc_number tc_num = get_new_tc_number(); 
	Symbol *sym;
	condition *cond, *c;
	wme *w;
	int i;
	production *prod;
	decay_timelist_element *decay_list;
	int decay_pos;
	wme_decay_element *decay_element;
    list *identifiers = 0;
    cons *cons;

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

	  for (cons = identifiers; cons ; cons=cons->rest){
			sym = (Symbol *) cons->first;
			add_symbol_to_tc(sym, tc_num, NIL, NIL);
	  } 

	 /* for (cond = ist->top_of_instantiated_conditions; cond ; cond = cond->next){
		if (cond->type == POSITIVE_CONDITION){
			  sym = equality_test_for_symbol_in_test(cond->data.tests.id_test);
			  if (sym) add_symbol_to_tc(sym, tc_num, NIL, NIL);
			  sym = equality_test_for_symbol_in_test(cond->data.tests.attr_test);
			  if (sym) add_symbol_to_tc(sym, tc_num, NIL, NIL);
			  sym = equality_test_for_symbol_in_test(cond->data.tests.value_test);
			  if (sym) add_symbol_to_tc(sym, tc_num, NIL, NIL);
		}
	 }*/
	
	 decay_list = current_agent(decay_timelist);
	 decay_pos = current_agent(current_decay_timelist_element)->position;
	 
		// get wmes in order of activation
	 for (i = 0; i < DECAY_ARRAY_SIZE ; i++){
		 decay_pos = decay_pos > 0 ? decay_pos - 1 : MAX_DECAY - 1;
		 if (decay_list[decay_pos].first_decay_element != NULL)
		 {
			 decay_element = decay_list[decay_pos].first_decay_element;
			 while (decay_element != NULL)
			 {
				 condition *cond_top = NIL, *cond_bottom, *new_top, *new_bottom;
				 w = decay_element->this_wme;
							 
		 /*num = rand() % current_agent(num_wmes_in_rete);
		w = current_agent(all_wmes_in_rete);
		for (i = 0 ; i < num; i++)
			w = w->rete_next;*/
				 cond = make_simple_condition(w->id, w->attr, w->value, w->acceptable);
				 // Check that the condition made from this wme is not already
				 // in the condition list of the production.
				 for (c = ist->top_of_instantiated_conditions ; c ; c = c->next){
					 if (conditions_are_equal(c, cond)){
					 free_with_pool(&current_agent(condition_pool), cond);
					 cond = NIL;
					 break;
					 }
				 }
				 if (!cond){
					 decay_element = decay_element->next;
					 continue;
				 }
				 free_with_pool(&current_agent(condition_pool), cond);
				 trace_to_prod(w, tc_num, &cond_top);
				 for (cond_bottom = cond_top ; cond_bottom->next; cond_bottom = cond_bottom->next);
				 copy_condition_list(ist->top_of_instantiated_conditions, &new_top, &new_bottom);
				 cond_top->prev = new_bottom;
				 new_bottom->next = cond_top;
				 prod = build_RL_production(new_top, cond_bottom, ist->nots, ist->preferences_generated, w);
				 deallocate_condition_list(new_top);
				 if (!prod){
					 tc_num = get_new_tc_number();
					 for (cons = identifiers; cons ; cons=cons->rest){
						 sym = (Symbol *) cons->first;
						 add_symbol_to_tc(sym, tc_num, NIL, NIL);
					 }
					 decay_element = decay_element->next;
					 continue;
					 // return NIL; // a perhaps tenporary solution to the tc_nums screwed up by build_RL_production
				 }
				 return prod;
			 }
		 }
	 }
	 return NIL;
}

// Update the value on RL productions from last cycle
void learn_RL_productions(int level){
	RL_record *record;
	production *prod;
	float Q, temp;
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
			temp = prod->avg_update;
			prod->avg_update = fabs(Q) + prod->avg_update*current_agent(epsilon);
			prod->increasing = (prod->avg_update > temp);
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
		   current_agent(RL_count)--;
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
                                 Symbol *val_sym,
								 bool acceptable)
{
    condition *newcond;

    allocate_with_pool (&current_agent(condition_pool),  &newcond);
    newcond->type = POSITIVE_CONDITION;
    newcond->next = NULL;
    newcond->prev = NULL;
	newcond->test_for_acceptable_preference = acceptable;


	/*
    if (strcmp(attr_sym->sc.name, "operator") == 0)
    {
        newcond->test_for_acceptable_preference = TRUE;
    }
    else
    {
        newcond->test_for_acceptable_preference = FALSE;
    }*/

    newcond->data.tests.id_test = make_equality_test_without_adding_reference(id_sym);
    newcond->data.tests.attr_test = make_equality_test_without_adding_reference(attr_sym);

    /*if (val_sym->common.symbol_type == IDENTIFIER_SYMBOL_TYPE)
    {
        char buf[100];
        buf[0] = val_sym->id.name_letter;
        buf[1] = ''\0'';
        val_sym = generate_new_variable(buf);
    }*/
    newcond->data.tests.value_test = make_equality_test_without_adding_reference(val_sym);

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

// Given a wme and the identifiers in the instantiated condition-list of a production
// return a condition list linking wme->id to one in the list of identifiers
// For now, these conditions will be positive conditions, using only equality tests.
void trace_to_prod(wme * w, tc_number tc_prod, condition ** cond){
	tc_number tc_seen = get_new_tc_number();
	int i = 1;
	condition *c;

	while(!trace_to_prod_depth(w, tc_prod, tc_seen, cond, i)){
		i++;
		tc_seen = get_new_tc_number();
	}

	for (c = *cond ; c ; c = c->next){
          symbol_add_ref(referent_of_equality_test(c->data.tests.id_test));
          symbol_add_ref(referent_of_equality_test(c->data.tests.attr_test));
          symbol_add_ref(referent_of_equality_test(c->data.tests.value_test));

    }
}

// Given-
// wme w
// tc_prod: the identifiers in the instantiated condition-list of a production
// depth d
// cond: a condition list.
// Find a list of wmes of length d that links w to an identifier in tc_prod.
// If such a list exists, return TRUE and add the list to cond.
// Else, return FALSE and cond should be unchanged.
// In either case, no reference_counts should be changed.
bool trace_to_prod_depth(wme * w, tc_number tc_prod, tc_number tc_seen, condition ** cond, int depth){
	Symbol * id = w->id;
	Symbol * attr = w->attr;
	Symbol * value = w->value;
	condition *new_cond;
	wme * z;
	dl_cons *dc;

	if (depth == 0) return FALSE;
	// if(symbol_is_in_tc(id, tc_seen)) return FALSE;
	new_cond = make_simple_condition(id, attr, value, w->acceptable);
//	if (new_cond->test_for_acceptable_preference == w->acceptable)
	insert_at_head_of_dll(*cond, new_cond, next, prev);
	if (symbol_is_in_tc(id, tc_prod)){
		return TRUE;
	} else {
		// bug - add attr test
		// add_symbol_to_tc(value, tc_seen, NIL, NIL);
		for (dc = id->id.parents ; dc ; dc = dc->next){
			z = dc->item;
			if (trace_to_prod_depth(z, tc_prod, tc_seen, cond, depth - 1))
				return TRUE;
		}
		remove_from_dll(*cond,*cond,next,prev);
		return FALSE;
	}
}

	
