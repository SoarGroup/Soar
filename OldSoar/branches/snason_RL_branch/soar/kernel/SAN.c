#include "soarkernel.h"


float compute_temp_diff(RL_record *, float);
Symbol *equality_test_for_symbol_in_test(test t);
condition *make_simple_condition(Symbol *id_sym, Symbol *attr_sym, Symbol *val_sym);
action *make_simple_action(Symbol *id_sym, Symbol *attr_sym, Symbol *val_sym, Symbol *ref_sym);
production *build_RL_production(condition *top_cond, condition *bottom_cond, not *nots, preference *pref);
void learn_RL_productions(int level, float);
void record_for_RL();
void variablize_condition_list(condition * cond);
void variablize_nots_and_insert_into_conditions(not * nots, condition * conds);
action *copy_and_variablize_result_list(preference * pref);
void variablize_symbol(Symbol ** sym);
void SAN_add_goal_or_impasse_tests(condition * all_conds);
bool symbol_is_in_tc(Symbol * sym, tc_number tc);
production *specify_production(stored_instantiation *ist);
void trace_to_prod(stored_wme * w, tc_number tc_prod, condition ** cond);
bool trace_to_prod_depth(stored_wme * w, tc_number tc_prod, tc_number tc_seen, condition ** cond, int depth);
int list_length(list *);
void STDDEV(double values[], int n, double *mean_pointer, double *std_dev_pointer);
void store_WM();
stored_wme *make_stored_wme(wme *);
stored_instantiation *make_stored_instantiation(instantiation *ist);
list *SAN_extract_list_elements(list **header, void *item);


// The following three functions manage the stack of RL_records.
// Each level in the stack corresponds to a level in Soar's subgoaling hierarchy.
void push_record(RL_record **r, Symbol *level_sym){
	RL_record *new_record;

	new_record = malloc(sizeof(RL_record));
	if (new_record != NULL) {
		new_record->pointer_list = NIL;
		new_record->stored_instantiations = NIL;
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
	stored_instantiation *temp;

	new_record = *r;
    free_list(new_record->pointer_list);
 	while(new_record->stored_instantiations){
		temp = new_record->stored_instantiations;
		new_record->stored_instantiations = new_record->stored_instantiations->next;
		deallocate_condition_list(temp->top_of_instantiated_conditions);
		deallocate_list_of_nots(temp->nots);
		free(temp);
	}
	symbol_remove_ref(new_record->goal_level);
	if (new_record->op)
		symbol_remove_ref(new_record->op);
	*r = new_record->next;
	free(new_record);
}

void reset_RL(){
	int i;

	while(current_agent(records))
 		pop_record(&current_agent(records));

	for (i = 0; i < 15; i++)
		current_agent(updates_list)[i] = 0;

	current_agent(updates_position) = 0;

	  {
	  stored_wme *swme;
	  while(current_agent(stored_WM_top)){
		  swme = current_agent(stored_WM_top);
		  current_agent(stored_WM_top) = current_agent(stored_WM_top)->next;
		  symbol_remove_ref(swme->id);
		  symbol_remove_ref(swme->attr);
		  symbol_remove_ref(swme->value);
		  free(swme);
	  }
	  current_agent(stored_WM_bottom) = NIL;
  }

	// current_agent(next_Q) = 0.0;
}


void copy_nots(not * top_not, not ** dest_not)
{
    not *new;

    *dest_not = NIL;
    while (top_not) {
		allocate_with_pool(&current_agent(not_pool), &new);
		new->s1 = top_not->s1;
   	    symbol_add_ref(new->s1);
    	new->s2 = top_not->s2;
		symbol_add_ref(new->s2);
		new->next = *dest_not;
		*dest_not = new;
        top_not = top_not->next;
    }
}


float compute_temp_diff(RL_record* r, float best_op_value){
	float Q;

	// print_with_symbols("\n Q value for %y\n", r->op);

	Q = r->reward;

    // print("\n Q after reward is %f\n" , Q);
	Q += pow(current_agent(gamma), r->step)*best_op_value;
	//Q += pow(current_agent(gamma), r->step)*(r->next_Q);
	// print("Q after next_Q update is %f\n", Q);
	// print("\n alpha is %f\n", current_agent(alpha));
	Q -= r->previous_Q;
	// print("Q after previous %f\n", Q);
	Q *= current_agent(alpha);
	// print("Q after alpha %f\n", Q);

    /*if (r->num_prod > 0)
		Q = Q / r->num_prod;*/

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
	stored_instantiation *stored_ist;
	production *prod, *new_prod;
	preference *pref;
 	RL_record *record;
	cons *c, *d;

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
			  if (record->op == pref->value)
				/* This is one scheme - where the format of an RL rule is never changed but a rule may be specialized. */
				 {
				  ist = pref->inst;
				  prod = ist->prod;
				  push(prod, record->pointer_list);
				  // Potentially build new RL-production

				  stored_ist = make_stored_instantiation(ist);
				  insert_at_head_of_dll(record->stored_instantiations,stored_ist, next, prev);

/*			   if (prod->times_updated < 0){
			//	  if ((prod->times_updated > 50) && !prod->conv_value){
 				 // if (prod->conv_mean && !prod->conv_value){
						  // excise_production(prod->child, TRUE);
						  // extract_list_element(record->pointer_list, prod->child); // bug - need to do for stack of records
						  // prod->child = NIL;
				//	  }
					  new_prod = specify_production(ist);
					  if (new_prod){
						  print_with_symbols("Specialize %y to %y\n", prod->name, new_prod->name);
						  prod->times_updated = 0;
//						  prod->conv_mean = FALSE;
						  prod->conv_value = TRUE;
						  // prod->increasing = 0;
						  // prod->child = new_prod;
						  push(new_prod, record->pointer_list);
					  }
				 }*/
			  }
		  }
	  for (c = record->pointer_list ; c ; c = c->rest){
		  prod = (production *) c->first;
		  for (d = prod->child_productions ; d ; d = d->rest){
			  new_prod = (production *) d->first;			  
			  if (!member_of_list(new_prod, record->pointer_list)){
				  list *to_be_deleted;
				  new_prod->promoted = TRUE;
				  to_be_deleted = SAN_extract_list_elements(&(prod->child_productions), new_prod);   // rewrite to use existing functions
				  free_list(to_be_deleted);
				  
			  }
		  }
	  }
	  if (!record->pointer_list){
			  condition *new_top, *new_bottom;
			  condition *new_cond;
			  not *new_not;
			  ist = w->preference->inst;
			  copy_condition_list(ist->top_of_instantiated_conditions, &new_top, &new_bottom);
			  copy_nots(ist->nots, &new_not);
			  for(pref = ist->preferences_generated; pref ; pref = pref->inst_next){
				  if (pref->type == ACCEPTABLE_PREFERENCE_TYPE){
					  new_cond = make_simple_condition(pref->id, pref->attr, pref->value);
					  symbol_add_ref(pref->id);
					  symbol_add_ref(pref->attr);
					  symbol_add_ref(pref->value);
					  insert_at_end_of_dll(new_bottom, new_cond, next, prev);
				  }
			  }
			  prod = build_RL_production(new_top, new_bottom, new_not, 
				  make_preference(NUMERIC_INDIFFERENT_PREFERENCE_TYPE, w->id, w->attr, w->value, make_float_constant(0)));
			  deallocate_condition_list(new_top);
			  deallocate_list_of_nots(new_not);
			  if (prod){
				  print_with_symbols("Add new rule %y\n", prod->name);
				  prod->promoted = TRUE;
				  push(prod, record->pointer_list);
			  }
		  }

  }
  {
	  stored_wme *swme;
	  while(current_agent(stored_WM_top)){
		  swme = current_agent(stored_WM_top);
		  current_agent(stored_WM_top) = current_agent(stored_WM_top)->next;
		  symbol_remove_ref(swme->id);
		  symbol_remove_ref(swme->attr);
		  symbol_remove_ref(swme->value);
		  free_list(swme->parents);
		  free(swme);
	  }
	  current_agent(stored_WM_bottom) = NIL;
  }

	  store_WM();
	  /* This is the other scheme - where new RL rules are made by filling out rule templates. */
		/*  {
			  ist = pref->inst;
			  prod = ist->prod;
			  if (prod->type == RL_PRODUCTION_TYPE) {
				  push(prod, record->pointer_list);
			  } else {
				  condition *cond_top, *cond_bottom;
				  not *nots;

				  copy_condition_list(ist->top_of_instantiated_conditions, &cond_top, &cond_bottom);
				  copy_nots(ist->nots, &nots);
				  new_prod = build_RL_production(cond_top, cond_bottom, nots, ist->preferences_generated);
				  deallocate_condition_list(cond_top);
				  deallocate_list_of_nots(nots);
				  if (new_prod) push(new_prod, record->pointer_list);
			  }
		  }
	  }*/

	  // Did any production fire while its child did not?
	  // for (c = record->pointer_list ; c ; c = c->rest){
	  // prod = (production *) c->first;
	//	  if (prod->child){
	//		  if (!member_of_list(prod->child, record->pointer_list))
	//		  		  prod->child = NIL;
	//	  }
	  //}
}

/* -----------------------------------------------------
store_WM:
A copy of WM is stored for one cycle. The purpose of this
is to allow the agent to make a decision whether to specialize
a rule based on what happens after applying the operator.
---------------------------------------------------------- */
void store_WM(){
	decay_timelist_element *decay_list;
	int decay_pos;
	wme_decay_element *decay_element;
	stored_wme *swme, *temp;
	int i;
	wme *w;
	dl_cons *dc, *new_dc;

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
				w = decay_element->this_wme;

				 swme = make_stored_wme(w);
				 if (!current_agent(stored_WM_top)) current_agent(stored_WM_top) = swme;
				 insert_at_end_of_dll(current_agent(stored_WM_bottom),swme, next, prev);

				 decay_element = decay_element->next;
			 }
		 }
	}

	for (w = current_agent(all_wmes_in_rete) ; w ; w = w->rete_next){
		if (!w->has_decay_element){
			swme = make_stored_wme(w);
			if (!current_agent(stored_WM_top)) current_agent(stored_WM_top) = swme;
			insert_at_end_of_dll(current_agent(stored_WM_bottom), swme, next, prev);
		}
	}


	for (swme = current_agent(stored_WM_top) ; swme ; swme = swme->next){
		for (dc = swme->id->id.parents ; dc ; dc = dc->next){
			w = dc->item;
			temp = w->stored_wme;
			allocate_with_pool(&current_agent(dl_cons_pool), &new_dc);
			new_dc->item = temp;
			insert_at_head_of_dll(swme->parents, new_dc, next, prev);
		}
	}


}





/* -----------------------------------------------------
specify_production:
This function builds a new RL-production with a LHS more specific
than the given RL-production. It does this by selecting a WME currently
in WM and trying to add it to the old production's condition list. The WME
is rejected if it is already matched in the production's instantiation.
Otherwise, the function traces through WM, from the id of the prospective
WME until it hits an identifier in the production's instantiation. All WMEs
in this trace are also added as conditions to the new production.
In this implementation, WMEs are tried in order of decreasing activation.
------------------------------------------------------------- */

production *specify_production(stored_instantiation *ist){
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
	stored_wme *swme;


	// Make a list of identifiers in the old production instantiation.
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

	// Mark the identifiers in the old production instantiation.
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

	 swme = current_agent(stored_WM_top);

	 while(swme){
	 
				 condition *cond_top = NIL, *cond_bottom, *new_top, *new_bottom;
				 not *nots = NIL, *nots_last, *new_not;
				 bool good_wme = TRUE;
				  
				 if (swme->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE){
					 swme = swme->next;
					 continue;
				 }

				 // Check that our new condition not already in production conditions
				 for (c = ist->top_of_instantiated_conditions ; c ; c = c->next){
					 if (c->type != POSITIVE_CONDITION)
						 continue;
					 if (!tests_are_equal(c->data.tests.id_test, make_equality_test_without_adding_reference(swme->id)))
						 continue;
					 if (!tests_are_equal(c->data.tests.attr_test, make_equality_test_without_adding_reference(swme->attr)))
						 continue;
					 if (!tests_are_equal(c->data.tests.value_test, make_equality_test_without_adding_reference(swme->value)))
						 continue;
					 good_wme = FALSE;
					 break;
				 }

				 // Check that the condition made from this wme is not already
				 // in the condition list of the production.
				 /*for (c = ist->top_of_instantiated_conditions ; c ; c = c->next){
					 if (conditions_are_equal(c, cond)){
					 free_with_pool(&current_agent(condition_pool), cond);
					 cond = NIL;
					 break;
					 }
				 }*/
				 if (!good_wme){
					 swme = swme->next;
					 continue;
				 }
				 // free_with_pool(&current_agent(condition_pool), cond);
				 /*if (new_not)
					 new_not->next = ist->nots;
				 */
				 // Find conditions linking new condition to old production instantiation
				 trace_to_prod(swme, tc_num, &cond_top);

				 // If one of the new WMEs is in the same slot as an WME in the instantiation,
				 // place a Not between their values.
				 for (c = cond_top ; c ; c = c->next){
					 for (cond = ist->top_of_instantiated_conditions; cond ; cond = cond->next){

					 if (c->type != POSITIVE_CONDITION)
						 continue;
					 if (!tests_are_equal(c->data.tests.id_test, cond->data.tests.id_test))
						 continue;
					 if (!tests_are_equal(c->data.tests.attr_test, cond->data.tests.attr_test))
						 continue;
					 if (!tests_are_equal(c->data.tests.value_test, cond->data.tests.value_test)){
						 allocate_with_pool(&current_agent(not_pool), &new_not);
						 new_not->s1 = referent_of_equality_test(c->data.tests.value_test);
						 symbol_add_ref(new_not->s1);
						 new_not->s2 = referent_of_equality_test(cond->data.tests.value_test);
						 symbol_add_ref(new_not->s2);
						 new_not->next = nots;
						 nots = new_not;
						}
					}
				 }

				 // Make new condition list, consisting of both new and old conditions.
				 copy_condition_list(ist->top_of_instantiated_conditions, &new_top, &new_bottom);
				 for (cond_bottom = cond_top ; cond_bottom->next; cond_bottom = cond_bottom->next);
				 cond_top->prev = new_bottom;
				 new_bottom->next = cond_top;
				 // Make new nots list, consisting of both new and old nots.
				 copy_nots(ist->nots, &new_not);
				 if (nots){
					 for (nots_last = nots ; nots_last->next ; nots_last = nots_last->next);
				 	 nots_last->next = new_not;
				 } else { nots = new_not; }

				 prod = build_RL_production(new_top, cond_bottom, nots , make_preference(NUMERIC_INDIFFERENT_PREFERENCE_TYPE, ist->state, make_sym_constant("operator"), ist->op, NIL));
				 deallocate_condition_list(new_top);
			 	 deallocate_list_of_nots(nots);
				 if (!prod){
					 tc_num = get_new_tc_number();
					 for (cons = identifiers; cons ; cons=cons->rest){
						 sym = (Symbol *) cons->first;
						 add_symbol_to_tc(sym, tc_num, NIL, NIL);
					 }
					 swme = swme->next;
					 continue;
					 // return NIL; // a perhaps tenporary solution to the tc_nums screwed up by build_RL_production
				 }
				 return prod;
			 }
			return NIL;		
 }
	 
	 


// Update the value on RL productions from last cycle
void learn_RL_productions(int level, float best_op_value){
	RL_record *record;
	production *prod;
	instantiation *inst;
	stored_instantiation *stored_inst;
	float update, temp, old_avg, old_avg_avg, old_var, old_avg_var;
	cons *c;
	float increment;
	preference *pref;
	int num_prods;
	wme *w;
	condition *cond;

	record = current_agent(records);

	do{

	if (record->level < level)
		return;

	if (record->op){

		update = compute_temp_diff(record, best_op_value);
 

		if (fabs(update) > 0.1*fabs(record->previous_Q)) {
			for (stored_inst = record->stored_instantiations ; stored_inst ; stored_inst = stored_inst->next){
				if (stored_inst->prod->promoted && (stored_inst->prod->updates_since_record > 5)){
					prod = specify_production(stored_inst);
					if (prod){
						print_with_symbols("Specialize %y to %y\n", stored_inst->prod->name, prod->name);
						push(prod, record->pointer_list);
						push(prod, stored_inst->prod->child_productions);
						stored_inst->prod->max = rhs_value_to_symbol(stored_inst->prod->action_list->referent)->fc.value;
						stored_inst->prod->min = stored_inst->prod->max;
						stored_inst->prod->updates_since_record = 0;
					}
				}
			}
		}
		while(record->stored_instantiations){
			stored_inst = record->stored_instantiations;
			record->stored_instantiations = record->stored_instantiations->next;
			deallocate_condition_list(stored_inst->top_of_instantiated_conditions);
			deallocate_list_of_nots(stored_inst->nots);
			free(stored_inst);
		}


		/*
		if ((current_agent(prev_diff) == 0) && (fabs(update) > 0)){
			// current_agent(stop_soar) = TRUE;
			// print("!!!!!!!!!!!!! Look to previous WM\n");
			build_RL_production(record->WM_record, , nil, record->pref_record);
		} else if (current_agent(prev_exploit) && (update < 0) && (fabs(update) > 0.1*current_agent(prev_diff))){
			// current_agent(stop_soar) = TRUE;
			// print("!!!!!!!!!!!!! Look to previous WM\n");
			build_RL_production(record->WM_record, , nil, record->pref_record);
		} else if (!current_agent(prev_exploit) && (update > 0) && (fabs(update) > 0.1*current_agent(prev_diff))) {
			// current_agent(stop_soar) = TRUE;
			// print("!!!!!!!!!!!!! Look to previous WM\n");
			build_RL_production(record->WM_record, , nil, record->pref_record);
		}

			deallocate_condition_list(record->WM_record);
			deallocate_preference(record->pref_record);
			record->WM_record = NIL;
			for (w = current_agent(all_wmes_in_rete) ; w ; w = w->rete_next){
				cond = make_simple_condition(w->id, w->attr, w->value, w->acceptable);
				cond->next = record->WM_record;
				record->WM_record = cond;
			}
			reset_variable_generator(nil,nil);
			current_agent(variablization_tc) = get_new_tc_num();
			variablize_condition_list(current_agent(WM_record));
			record->pref_record = make_preference(NUMERIC_PREFERENCE_TYPE, variablize_symbol(current_agent(top_state)), current_agent(operator_symbol), variablize_symbol(record->op), nil);
		    */

		//for (w = current_agent(all_wmes_in_rete) ; w ; w = w->rete_next)
		//			print_wme(w);

		/*temp = fabs(update - current_agent(updates_mean));

		if (current_agent(prev_exploit) && (temp > current_agent(updates_stddev))){
			current_agent(stop_soar) = TRUE;
			print("!!!!!!!!!!!!! Look to previous WM\n");
		} else if (!current_agent(prev_exploit) && (update > 0) && (temp > (0.5*current_agent(updates_stddev)))){
			current_agent(stop_soar) = TRUE;
			print("!!!!!!!!!!!!!! Look to previous WM\n");
		}




		current_agent(updates_list)[current_agent(updates_position)] = update;
		current_agent(updates_position) = (current_agent(updates_position) + 1) % 15;
		STDDEV(current_agent(updates_list), 15, &current_agent(updates_mean), &current_agent(updates_stddev));*/

		num_prods = list_length(record->pointer_list);
		if (num_prods > 0){

			increment = update / num_prods;


			c = record->pointer_list;

			while(c){


				prod = (production *) c->first;
				c = c->rest;
	//			prod->type = RL_PRODUCTION_TYPE;

				if (!prod) continue;

				temp = rhs_value_to_symbol(prod->action_list->referent)->fc.value;
				temp += increment;

				symbol_remove_ref(rhs_value_to_symbol(prod->action_list->referent));
				prod->action_list->referent = symbol_to_rhs_value(make_float_constant(temp));
				if (temp > prod->max){
					prod->max = temp;
					prod->updates_since_record = 0;
				} else if (temp < prod->min) {
					prod->min = temp;
					prod->updates_since_record = 0;
				} else {
					prod->updates_since_record++;
				}

				for (inst = prod->instantiations ; inst ; inst = inst->next){
					for (pref = inst->preferences_generated ; pref ; pref = pref->inst_next){
						symbol_remove_ref(pref->referent);
						pref->referent = symbol_to_rhs_value(make_float_constant(temp));
					}
				}


				// a->preference_type = NUMERIC_INDIFFERENT_PREFERENCE_TYPE;
				// a->referent = symbol_to_rhs_value(make_float_constant(Q));
				prod->times_updated++;
				// prod->value_list[prod->value_position] = temp;
				// prod->value_position = (prod->value_position + 1) % 15;

			/*	if (prod->times_updated > 15){
					STDDEV(prod, 15);
					// if (prod->std_dev > 0.01*fabs(record->previous_Q)){
					 // if ((prod->std_dev > 0.01*fabs(record->previous_Q)) || (prod->std_dev > 0.1*fabs(temp))){
					if (prod->std_dev > 0.0001){
					prod->conv_value = FALSE;
					} else { prod->conv_value = TRUE; }
				} */

				prod->update = update;
				// old_avg = prod->avg_update;
 				// old_avg = prod->avg_value;
				// prod->avg_update = ((prod->times_updated - 1)*old_avg + abs(update)) / prod->times_updated;
				// prod->avg_value = ((prod->times_updated - 1)*old_avg + temp) / prod->times_updated;


				// old_avg_avg = prod->avg_avg;
				// prod->avg_avg = ((prod->times_updated - 1)*old_avg_avg + prod->avg_value) / prod->times_updated;

				// old_var = prod->var;
				// prod->var = (((old_var + pow(old_avg,2))*(prod->times_updated - 1) + pow(temp,2)) / prod->times_updated) - pow(prod->avg_value,2);

				// old_avg_var = prod->avg_var;
				// prod->avg_var = (((old_avg_var + pow(old_avg_avg,2))*(prod->times_updated - 1) + pow(prod->avg_value,2)) / prod->times_updated) - pow(prod->avg_avg,2);

				/*if (fabs(old_avg - prod->avg_update) < 0.05 && fabs(prod->avg_update) < 0.05){
					prod->conv_mean = TRUE;
				} else { prod->conv_mean = FALSE; }
				old_abs = prod->decay_abs_update;
				prod->decay_abs_update = ((prod->times_updated - 1)*old_abs + fabs(update)) / prod->times_updated;
				if (fabs(old_abs - prod->decay_abs_update) < 0.1 && fabs(prod->decay_abs_update) < 0.1) { prod->conv_value = TRUE;
				} else { prod->conv_value = FALSE; }*/
				// prod->decay_abs_update = fabs(update) + prod->decay_abs_update*current_agent(gamma);
				// prod->decay_normalization = 1 + prod->decay_normalization*current_agent(gamma);
				// prod->decay_abs_update = ((prod->times_updated - 1)*prod->decay_abs_update + fabs(update)) / prod->times_updated;
				// prod->increasing = (prod->decay_abs_update > temp ? 1 : 0);
				// prod->decay_abs_update = fabs(update);

				/*if (fabs(prod->avg_var) < 0.02) {
					prod->conv_mean = TRUE;
				} else prod->conv_mean = FALSE; }*/



				print_with_symbols("\n%y  ", prod->name);
 				print("Cycle %d ", current_agent(d_cycle_count));
				print("Update %f\n", update);
	    		//print_with_symbols("value %y ", rhs_value_to_symbol(prod->action_list->referent));
				//print("Mean %f ", prod->mean);
				//print("Std dev %f ", prod->std_dev);
				//print("firings %d\n", prod->times_updated);
 				// print("Variance in value %f ", prod->var);
				// print("Update %f ", update);
				// print("%s\n", (current_agent(new_exploit) ? "Exploit" : "Explore"));
				// print("Difference %f\n", current_agent(prev_diff));
				// print("Average update %f ", current_agent(updates_mean));
				// print("Update stddev %f\n", current_agent(updates_stddev));
//				print("Decayed average %f ", (prod->decay_abs_update / prod->decay_normalization));
				// print("Average value %f ", prod->avg_value);
				// print("Var in average value %f ", prod->avg_var);
				// print("firings %d\n", prod->times_updated);

			//	prod->times_applied++;
			}
		}

 			symbol_remove_ref(record->op);
		 	record->op = NIL;
			record->reward = 0.0;
			record->step = 0;
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

	/*for (prod = current_agent(all_productions_of_type[RL_PRODUCTION_TYPE]) ; prod ; prod = prod->next){
		print_with_symbols("\n%y  ", prod->name);
		print_with_symbols("value %y ", rhs_value_to_symbol(prod->action_list->referent));
	}*/

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

production *build_RL_production(condition *top_cond, condition *bottom_cond, not *nots, preference *pref)
{

	action *a;
	Symbol *prod_name;
	production *prod;
	bool chunk_var;

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
                                 Symbol *val_sym)
{
    condition *newcond;

    allocate_with_pool (&current_agent(condition_pool),  &newcond);
    newcond->type = POSITIVE_CONDITION;
    newcond->next = NULL;
    newcond->prev = NULL;
	// newcond->test_for_acceptable_preference = acceptable;


	
    if (strcmp(attr_sym->sc.name, "operator") == 0)
    {
        newcond->test_for_acceptable_preference = TRUE;
    }
    else
    {
        newcond->test_for_acceptable_preference = FALSE;
    }

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
void trace_to_prod(stored_wme * sw, tc_number tc_prod, condition ** cond){
	tc_number tc_seen = get_new_tc_number();
	int i = 1;
	condition *c;

	while(!trace_to_prod_depth(sw, tc_prod, tc_seen, cond, i)){
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
bool trace_to_prod_depth(stored_wme * w, tc_number tc_prod, tc_number tc_seen, condition ** cond, int depth){
	Symbol * id = w->id;
	Symbol * attr = w->attr;
	Symbol * value = w->value;
	condition *new_cond;
	stored_wme * z;
	dl_cons *dc;

	if (depth == 0) return FALSE;
	// if(symbol_is_in_tc(id, tc_seen)) return FALSE;
	new_cond = make_simple_condition(id, attr, value);
//	if (new_cond->test_for_acceptable_preference == w->acceptable)
	insert_at_head_of_dll(*cond, new_cond, next, prev);
	if (symbol_is_in_tc(id, tc_prod)){
		return TRUE;
	} else {
		// bug - add attr test
		// add_symbol_to_tc(value, tc_seen, NIL, NIL);
		for (dc = w->parents ; dc ; dc = dc->next){
			z = dc->item;
			if (trace_to_prod_depth(z, tc_prod, tc_seen, cond, depth - 1))
				return TRUE;
		}
		remove_from_dll(*cond,*cond,next,prev);
		return FALSE;
	}
}

int list_length(list *L){
	int k=0;
	cons *c;

	for (c = L; c ; c = c->rest){
		if (c->first)
			k++;
	}

	return k;
}
 
list *SAN_extract_list_elements(list **header, void *item){
	  cons *first_extracted_element, *tail_of_extracted_elements;
    cons *c, *prev_c, *next_c;

    first_extracted_element = NIL;
    tail_of_extracted_elements = NIL;

    prev_c = NIL;
    for (c = (*header); c != NIL; c = next_c) {
        next_c = c->rest;
        if (!(c->first == item)) {
            prev_c = c;
            continue;
        }
        if (prev_c)
            prev_c->rest = next_c;
        else
            *header = next_c;
        if (first_extracted_element)
            tail_of_extracted_elements->rest = c;
        else
            first_extracted_element = c;
        tail_of_extracted_elements = c;
    }
    if (first_extracted_element)
        tail_of_extracted_elements->rest = NIL;
    return first_extracted_element;
}


void STDDEV(double values[], int n, double *mean_pointer, double *std_dev_pointer){
	double mean = 0, std_dev = 0;
	int i;

	for (i = 0; i < n ; i++)
		mean += values[i];

	mean = mean / n;
	*mean_pointer = mean;


	for (i = 0; i < n ; i++)
       std_dev += pow((mean - values[i]), 2);

	std_dev = std_dev / (n - 1);
	std_dev = pow(std_dev , .5);

	*std_dev_pointer = std_dev;
}

void RL_update_symbolically_chosen(slot *s, preference *candidates){
	if (!candidates) return;
	if (current_agent(sysparams)[RL_ON_SYSPARAM]){
		  preference *temp;
		  double temp_Q = 0;   // DEFAULT_INDIFFERENT_VALUE;
		  RL_record *rec;
		  int goal_level = s->id->id.level;
		  for (rec = current_agent(records) ; rec->level > goal_level; rec = rec->next)
			  rec->next_Q = 0;
		/* SAN - compute Q-value when winner decided by symbolic preferences */
		// if (!candidates->value->common.decider_flag){

		 	for (temp=s->preferences[NUMERIC_INDIFFERENT_PREFERENCE_TYPE]; temp!=NIL; temp=temp->next){
				if (candidates->value == temp->value){
					  if (temp->referent->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE)
						temp_Q += temp->referent->ic.value;
					if (temp->referent->common.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE)
						  temp_Q += temp->referent->fc.value;
				}
			}
			print("Operator value is %f\n", temp_Q);
			print("%s\n", (current_agent(new_exploit) ? "Exploit" : "Explore"));
         	rec->next_Q = temp_Q;
			learn_RL_productions(goal_level, temp_Q);
			// current_agent(prev_exploit) = current_agent(new_exploit);
			// current_agent(prev_diff) = current_agent(new_diff);

	  }
}

stored_wme *make_stored_wme(wme *w)
{
    stored_wme *swme;

    // current_agent(num_existing_wmes)++;
    swme = malloc(sizeof(stored_wme));
    swme->id = w->id;
    swme->attr = w->attr;
    swme->value = w->value;
    symbol_add_ref(w->id);
    symbol_add_ref(w->attr);
    symbol_add_ref(w->value);
    swme->acceptable = w->acceptable;
	swme->parents = NIL;
	w->stored_wme = swme;
   
    return swme;
}

stored_instantiation *make_stored_instantiation(instantiation *ist){
	
	stored_instantiation *stored_ist;

	stored_ist = malloc(sizeof(stored_instantiation));
	stored_ist->prod = ist->prod;
	copy_condition_list(ist->top_of_instantiated_conditions, &(stored_ist->top_of_instantiated_conditions), &(stored_ist->bottom_of_instantiated_conditions));
	copy_nots(ist->nots, &(stored_ist->nots));
	stored_ist->state = ist->preferences_generated->id;
	stored_ist->op = ist->preferences_generated->value;

	return stored_ist;
}