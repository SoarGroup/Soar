#include "soarkernel.h"

// extern action *copy_and_variablize_result_list (preference *pref);
extern void variablize_condition_list (condition *cond);                       /*********/
extern void variablize_nots_and_insert_into_conditions(not *, condition *);    /***********/
extern void variablize_symbol (Symbol **sym);
// extern not *get_nots_for_instantiated_conditions(list *, tc_number);

void check_conds(tc_number, RL_record *);
not *check_nots(tc_number, RL_record *);
rhs_value compute_Q_value();
// void build_op_tree(slot *s, condition **c);
void add_goal_tests (condition *cond);
action *make_simple_action( Symbol *, Symbol *, Symbol *);

void push_record(RL_record **r, Symbol *level_sym){
	RL_record *new_record;

	new_record = malloc(sizeof(RL_record));
	if (new_record != NULL) {
		new_record->reward = 0;
	    new_record->previous_Q = 0;
		new_record->op = NIL;
		new_record->goal_level = level_sym;
		symbol_add_ref(new_record->goal_level);
	    new_record->RL_bottom = NIL;
		new_record->RL_top = NIL;
		new_record->RL_nots = NIL;
		new_record->step = 0;
		new_record->level = level_sym->id.level;
	 	new_record->next = *r;
		*r = new_record;
	}
}

void pop_record(RL_record **r){
	RL_record *new_record;

	new_record = *r;
	deallocate_condition_list(new_record->RL_top);
	deallocate_list_of_nots(new_record->RL_nots);
	symbol_remove_ref(new_record->goal_level);
	if (new_record->op)
		symbol_remove_ref(new_record->op);
	*r = new_record->next;
	free(new_record);
}

void reset_RL(){

	while(current_agent(records))
 		pop_record(&current_agent(records));
	
}
/*******/
void add_goal_tests( condition *cond ){
  condition *cc;
  tc_number tc;   /* mark each id as we add a test for it, so we don't add
                     a test for the same id in two different places */
  Symbol *id;
  test t;
  complex_test *ct;

  tc = get_new_tc_number();
  for (cc=cond; cc!=NIL; cc=cc->next) {
    if (cc->type!=POSITIVE_CONDITION) continue;
    id = referent_of_equality_test (cc->data.tests.id_test);
    if ( (id->id.isa_goal) &&
         (id->id.tc_num != tc) ) {
      allocate_with_pool (&current_agent(complex_test_pool), &ct);
      ct->type = GOAL_ID_TEST;
      t = make_test_from_complex_test(ct);
      add_new_test_to_test (&(cc->data.tests.id_test), t);
      id->id.tc_num = tc;
    }
  }
}

/* After an operator has been selected,  */
/* ********** */
void learn_RL_productions(int level){
	RL_record *record;
	Symbol *prod_name;
	byte prod_type;
	production *prod;
	rhs_value Q;
	byte rete_add_result;
	not *nots;
	action *a;
	slot* s;
	
	record = current_agent(records);
	
	do{
	    
	
	if (record->level < level)
		return;

	// s = goal->id.operator_slot;
	
	if (record->op){
     
		// symbol_add_ref(record->op);
		// symbol_add_ref(record->goal_level);
//		print_with_symbols("\nOp %y ", s->op);
//		print("at start learn_RL_productions with reference count %d\n", s->op->common.reference_count);
	{
	 tc_number RL_tc;
	 RL_tc = get_new_tc_number();
	 check_conds(RL_tc, record);
     nots = check_nots(RL_tc,record);
	}
		
	// print_condition_list(s->RL_top, 2, TRUE);
	add_goal_tests( record->RL_top );
	current_agent(variablize_this_chunk) = 1;
	reset_variable_generator (record->RL_top, NIL);
    current_agent(variablization_tc) = get_new_tc_number();
    variablize_condition_list(record->RL_top);
//		print_with_symbols("\nOp %y ", s->op);
//		print("whatever with reference count %d\n", s->op->common.reference_count);
	variablize_nots_and_insert_into_conditions(nots, record->RL_top);   
//print_with_symbols("\nOp %y ", s->op);
//print("before make_simple_action with reference count %d\n", s->op->common.reference_count);
	a = make_simple_action(record->goal_level, current_agent(operator_symbol), record->op);
//	print_with_symbols("\nOp %y ", s->op);
//	print("after make_simple_action with reference count %d\n", s->op->common.reference_count);
	
	// print_condition_list(record->RL_top, 2, TRUE);
	
	current_agent(making_binary) = TRUE; // I think this setting allows duplicate productions.
	// probably there is a better place to put this
 
	
	prod_type = USER_PRODUCTION_TYPE;    // temporary, perhaps will have new type one day
 
	// build LHS
    prod_name = generate_new_sym_constant("RL-" , &current_agent(RL_count));
  
 	// build RHS
	
	Q = compute_Q_value(record);
    a->preference_type = BINARY_INDIFFERENT_PREFERENCE_TYPE;
    a->referent = Q;
	
	// print_action_list(a, 2, TRUE);
	
	prod = make_production (prod_type, prod_name, &(record->RL_top), &(record->RL_bottom), &a, FALSE);
	// print("\n Printing action to go on prod.");
	// print_action_list(prod->action_list, 2, TRUE);
	rete_add_result = add_production_to_rete(prod, record->RL_top, NULL, TRUE);
    	
	if (rete_add_result == DUPLICATE_PRODUCTION){
		excise_production (prod, FALSE);
		current_agent(RL_count)--;
	}
	deallocate_condition_list(record->RL_top);
	record->RL_top = NULL;
	record->RL_bottom = NULL;
 	deallocate_list_of_nots(nots);
 	symbol_remove_ref(record->op);
	// symbol_remove_ref(record->goal_level);
	record->op = NIL;
	// current_agent(RL_count)++;
	record->reward = 0.0;
	record->step = 0;
	record->previous_Q = 0;
	current_agent(making_binary) = FALSE;
	}

	if (record->level > level){
		pop_record(&current_agent(records));
		record = current_agent(records);
	} else
		record = record->next;


	} while(record);
}

/*********/
rhs_value compute_Q_value(RL_record* r){
	float Q;

	
	// print_with_symbols("\n Q value for %y\n", r->op);
	
	Q = r->reward;

    // print("\n Q after reward is %f\n" , Q);
	Q += pow(current_agent(gamma), r->step)*current_agent(next_Q);
	// print("Q after next_Q update is %f", Q);
	// print("\n alpha is %f\n", current_agent(alpha));
	Q -= r->previous_Q;
	// print("Q after previous - %f\n", Q);
	Q *= current_agent(alpha);
	// print("Q after alpha - %f\n", Q);

	

	return symbol_to_rhs_value(make_float_constant(Q));
}

/******/
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

/*
wme* find_operator(){
	Symbol *goal;
	slot *s;

	goal = current_agent(bottom_goal);
	s = goal->id.operator_slot;
	if (s->wmes)
		return (s->wmes);
	else
		return 0; 
}*/

/* ===================================================================
   make_simple_action

   This function allocates and initializes an action struct for
   a simple positive action given the id, attribute and value
   symbols for the wme.

   CAVEAT:  If the symbol given for the WME''s value is an identifier
            then a new variable will be generated instead.

   Created: 04 Dec 2002
   =================================================================== */
/*************/
action *make_simple_action(Symbol *id_sym, Symbol *attr_sym, Symbol *val_sym){
    action *rhs;
    Symbol *temp;

    allocate_with_pool (&current_agent(action_pool),  &rhs);
    rhs->next = NULL;
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
    
    return rhs;

}//make_simple_action


// write this to be faster
// it puts tc on conditions to be used for nots, and removes duplicate conditions
// to add - if value is dangling id (ie only matches on RHS), remove condition
/************/
void check_conds(tc_number RL_tc, RL_record *r){
	condition * next_cond, *temp_cond, *iter_cond;

	next_cond = r->RL_top;
	while(next_cond){
		add_cond_to_tc(next_cond, RL_tc, NIL, NIL);
		temp_cond = next_cond->next;
		for (iter_cond = temp_cond ; iter_cond != NIL ; iter_cond = iter_cond->next){
			if (conditions_are_equal(iter_cond, next_cond)){
				// remove condition
				if (next_cond == r->RL_bottom)
					r->RL_bottom = next_cond->prev;
				fast_remove_from_dll(r->RL_top, next_cond, condition, next, prev);
				next_cond->next = NIL;
				deallocate_condition_list(next_cond);
				break;
			}
		}
		next_cond = temp_cond;
	}
}

not *check_nots(tc_number RL_tc, RL_record *r){
	not *n1, *n2, *collected_nots;
	bool add;

	collected_nots = NIL;
	while(r->RL_nots){
		n1 = r->RL_nots;
		r->RL_nots = n1->next;
		add = TRUE;
		if ((n1->s1->id.tc_num != RL_tc) || (n1->s2->id.tc_num != RL_tc))
			add = FALSE;
		else{
			for (n2 = collected_nots; n2 != NIL ; n2 = n2->next){
				if ((n2->s1 == n1->s1) && (n2->s2 == n1->s2)) add = FALSE;
				if ((n2->s1 == n1->s2) && (n2->s2 == n1->s1)) add = FALSE;
			}
		}
		if (add){
			n1->next = collected_nots;
			collected_nots = n1;
		} else {
			symbol_remove_ref (n1->s1);
			symbol_remove_ref (n1->s2);
			free_with_pool (&current_agent(not_pool), n1);
		}
	}
	return collected_nots;
}

/**********/
void copy_nots(instantiation *inst, not **dest_top){
    not *n, *new_not;

	for(n = inst->nots; n != NIL ; n=n->next){
      allocate_with_pool (&current_agent(not_pool), &new_not);
      new_not->next = *dest_top;
      *dest_top = new_not;
      new_not->s1 = n->s1;
      symbol_add_ref (new_not->s1);
      new_not->s2 = n->s2;
      symbol_add_ref (new_not->s2);
	}

}

/*
action *copy_and_variablize_result (preference *pref) {
  action *a;
  Symbol *temp;
  
  if (!pref) return NIL;
  allocate_with_pool (&current_agent(action_pool), &a);
  a->type = MAKE_ACTION;

  temp = pref->id;
  symbol_add_ref (temp);
  variablize_symbol (&temp);
  a->id = symbol_to_rhs_value (temp);

  temp = pref->attr;
  symbol_add_ref (temp);
  variablize_symbol (&temp);
  a->attr = symbol_to_rhs_value (temp);

  temp = pref->value;
  symbol_add_ref (temp);
  variablize_symbol (&temp);
  a->value = symbol_to_rhs_value (temp);

  a->preference_type = pref->type;

  if (preference_is_binary(pref->type)) {
    temp = pref->referent;
    symbol_add_ref (temp);
    variablize_symbol (&temp);
    a->referent = symbol_to_rhs_value (temp);
  }
  
  a->next = NIL;
  return a;  
}*/


/**********/
condition *RL_copy_condition (condition *cond) {
  condition *new;

  if (!cond) return NIL;
  allocate_with_pool (&current_agent(condition_pool), &new);
  new->type = cond->type;
  
  switch (cond->type) {
  case POSITIVE_CONDITION:
    /* ... and fall through to next case */
  case NEGATIVE_CONDITION:
    new->data.tests.id_test = copy_test (cond->data.tests.id_test);
    new->data.tests.attr_test = copy_test (cond->data.tests.attr_test);
    new->data.tests.value_test = copy_test (cond->data.tests.value_test);
    new->test_for_acceptable_preference = cond->test_for_acceptable_preference;
    break;
  case CONJUNCTIVE_NEGATION_CONDITION:
    copy_condition_list (cond->data.ncc.top, &(new->data.ncc.top),
                         &(new->data.ncc.bottom));
    break;
  }
  return new;
}

/* ----------------------------------------------------------------
   Copies the given condition list, returning pointers to the
   top-most and bottom-most conditions in the new copy.
---------------------------------------------------------------- */
/*******/
void RL_copy_condition_list (condition *top_cond,
                          condition **dest_top,
                          condition **dest_bottom) {
  condition *newc, *temp;

  temp = NIL;
  while (top_cond) {
	// print_condition(top_cond);
    newc = RL_copy_condition (top_cond);
    if (!temp) temp = newc;
	if (*dest_bottom) (*dest_bottom)->next = newc;
	newc->prev = *dest_bottom;
	newc->next = NIL;
	*dest_bottom = newc;
	 
    top_cond = top_cond->next;
  }
  *dest_top = temp;
}



/*********/
condition *make_simple_condition(Symbol *id_sym,
                                 Symbol *attr_sym,
                                 Symbol *val_sym)
{
    condition *newcond;
    
    allocate_with_pool (&current_agent(condition_pool),  &newcond);
    newcond->type = POSITIVE_CONDITION;
    newcond->next = NULL;
    newcond->prev = NULL;

    if (strcmp(attr_sym->var.name, "operator") == 0)
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

/*
void add_goal_or_impasse_test (condition *cond) {
  condition *c;
  tc_number tc;   /* mark each id as we add a test for it, so we don't add
                     a test for the same id in two different places 
  Symbol *id;
  test t;
  complex_test *ct;

  tc = get_new_tc_number();
  for (c=cond; c!=NIL; c=c->next) {
    if (c->type!=POSITIVE_CONDITION) continue;
    id = referent_of_equality_test (c->data.tests.id_test);
    if ( (id->id.isa_goal || id->id.isa_impasse) &&
         (id->id.tc_num != tc) ) {
      allocate_with_pool (&current_agent(complex_test_pool), &ct);
      ct->type = (id->id.isa_goal) ? GOAL_ID_TEST : IMPASSE_ID_TEST;
      t = make_test_from_complex_test(ct);
      add_new_test_to_test (&(c->data.tests.id_test), t);
      id->id.tc_num = tc;
    }
  }
}*/

/*
void build_op_tree(slot *op, condition **c){
	wme *w;
	slot *s;

	for ( s = op ; s ; s = s->next){
		for (w=s->wmes ; w ; w = w->next){
			(*c)->next = make_simple_condition(w->id, w->attr, w->value);
			(*c)->next->prev = *c;
			*c = (*c)->next;
			//print_with_symbols("\nvalue of wme %y\n", w->value);
			if (w->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE)
			 	build_op_tree(w->value->id.slots, c);
		}
	}
}*/

/*******/
void record_for_RL(){
	wme *chosenOp;
	slot *s;
	instantiation *ist;
	preference *pref;
	RL_record *record;
 	int i = 0;
		
  // SAN - catch operator ID here
  s = current_agent(bottom_goal)->id.operator_slot;
  chosenOp = s->wmes;
  if (chosenOp){
	  
	  // print_wme(chosenOp);

	  record = current_agent(records);
	  ist = chosenOp->preference->inst;
	  
	  //print_with_symbols("\n Chosen op - %y \n" , chosenOp);
   

	// print("\n Printing condition list. \n");
	// print_condition_list(current_agent(RL_top), 1, 0);
	 // print("\n Printing action list. \n");
	// print_action_list(current_agent(prev_op), 1, 0);
	

	// print("\n Printing conditions before.");
	// print_condition_list(current_agent(RL_top), 2, TRUE);
	//if (current_agent(prev_op))
	//	learn_RL_production();

 	
	// print_condition_list(current_agent(RL_top), 2, TRUE);	
	// print("\n Printing operator conditions. ");
	// print_condition_list(chosenOp->preference->inst->top_of_instantiated_conditions, 2, TRUE);
	record->op = chosenOp->value;
	symbol_add_ref(record->op);       // SAN ??
//	print_with_symbols("\nOp %y ", record->op);
//	print("with next_Q %f\n", record->previous_Q);
//print("after setting in slot with reference count %d\n", s->op->common.reference_count);
	// symbol_add_ref(chosenOp->value);             // ref covered by conditions?
	
	record->previous_Q = current_agent(next_Q);
	
	  
    RL_copy_condition_list (ist->top_of_instantiated_conditions, &(record->RL_top),
                                 &(record->RL_bottom)); // collect conditions in operator proposal

    copy_nots(ist, &(record->RL_nots));

	for (pref = ist->preferences_generated ; pref != NIL; pref = pref->inst_next){
		// print("Type is %d\n", pref->type);
		if (pref->type == ACCEPTABLE_PREFERENCE_TYPE){
			record->RL_bottom->next = make_simple_condition(pref->id, pref->attr, pref->value);
			// print_condition(s->RL_bottom->next);
			record->RL_bottom->next->prev = record->RL_bottom;
			record->RL_bottom = record->RL_bottom->next;
		}
	}

	
  	// add condition testing for acceptable operator on state, to top of condition list
	// current_agent(RL_top)->prev = make_simple_condition(chosenOp->id, chosenOp->attr, chosenOp->value);
	// current_agent(RL_top)->prev->next = current_agent(RL_top);
	// current_agent(RL_top) = current_agent(RL_top)->prev;
	 /* s->RL_bottom->next = make_simple_condition(chosenOp->id, chosenOp->attr, chosenOp->value);
	 s->RL_bottom->next->prev = s->RL_bottom;
	 s->RL_bottom = s->RL_bottom->next; */

	// current_agent(prev_op) = chosenOp->value;
	// current_agent(RL_state) = chosenOp->id;
	
	// s = chosenOp->value->id.slots;
    	
    // add conditions off of operator
	// build_op_tree(s, &current_agent(RL_bottom));
 
	// add test for state on state condition (is it safe to assume this is at the top?)

	
   // variablize_nots_and_insert_into_conditions (nots, current_agent(RL_top));
		
	// print("\n Printing conditions after.");
	// print_condition_list(current_agent(RL_top), 2, TRUE);

	/*for (condition = condition_list->top_of_instantiated_conditions; condition ; condition = condition->next){ 
		if (condition->type == POSITIVE_CONDITION){
			RL_top = copy_condition(condition);
			RL_bottom = RL_top;
		}
	}
	for ( ; condition; condition = condition->next){
		if (condition->type == POSITIVE_CONDITION){
			RL_bottom->next = copy_condition(condition);
			RL_bottom = RL_bottom->next;
		}
	}
	RL_bottom->next = NIL;
    */
 	// then look through slots list in chosenOp struct, copy any wmes in these slots
    // finally search for any wmes specified to be saved
    // call make_wme to make memory wmes for these
    // call add_wme_to_wm to add these wmes in the second "do_buffered_wm_etc." call
  // SAN - end

  }
}
/*
void collect_RL_conditions(){
	// after firing new productions in do_preference_phase
	// go down newly_created_instantiations list
	// if production tests the operator currently to be updated
	// collect conditions from production

	instantiation *inst;
	condition *c;
	wme *w;

	if current_agent(prev_op){
		for (inst=current_agent(newly_created_instantiations); inst!=NIL; inst=inst->next){
			for (c=inst->top_of_instantiated_conditions; c!=NIL; c=c->next){
				if (c->type==POSITIVE_CONDITION){
					w = c->bt.wme;
					if (w->value == current_agent(prev_op)){
						RL_copy_condition_list (inst->top_of_instantiated_conditions, &(current_agent(RL_bottom)->next),
                                 &current_agent(RL_bottom));
						copy_nots(inst, &current_agent(RL_nots));
						// print_condition_list(inst->top_of_instantiated_conditions, 2, TRUE);
						return;
					}
				}
			}
		}
	}
  	return;
}
*/
