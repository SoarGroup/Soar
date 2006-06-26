// testings
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H
#include "portability.h"

//#ifdef SEMANTIC_MEMORY


#include "smem.h"


wme *add_input_wme_with_history(agent* thisAgent, Symbol *id, Symbol *attr, Symbol *value){
	wme* w = add_input_wme(thisAgent, id, attr, value);
	return w;
}

Symbol* find_smem_link_id(agent* thisAgent){
	for(wme* w = thisAgent->bottom_goal->id.input_wmes; w != NIL; w=w->next){
		if(strcmp(symbol_constant_to_string(thisAgent, w->attr).c_str(), "smem") == 0){
			return w->value;
		}
	}
	return NIL;
}

Symbol* find_save_link_id(agent* thisAgent){
	Symbol* smem_link_id = find_smem_link_id(thisAgent);
	if(smem_link_id == NIL){
		return NIL;
	}
	for(wme* w = smem_link_id->id.input_wmes; w != NIL; w=w->next){
		if(strcmp(symbol_constant_to_string(thisAgent, w->attr).c_str(), "save") == 0){
			return w->value;
		}
	}
	return NIL;
}

Symbol* find_retrieve_link_id(agent* thisAgent){
	Symbol* smem_link_id = find_smem_link_id(thisAgent);
	if(smem_link_id == NIL){
		return NIL;
	}
	for(wme* w = smem_link_id->id.input_wmes; w != NIL; w=w->next){
		if(strcmp(symbol_constant_to_string(thisAgent, w->attr).c_str(), "retrieve") == 0){
			return w->value;
		}
	}
	return NIL;
}

Symbol* find_cluster_link_id(agent* thisAgent){
	Symbol* smem_link_id = find_smem_link_id(thisAgent);
	if(smem_link_id == NIL){
		return NIL;
	}
	for(wme* w = smem_link_id->id.input_wmes; w != NIL; w=w->next){
		if(strcmp(symbol_constant_to_string(thisAgent, w->attr).c_str(), "cluster") == 0){
			return w->value;
		}
	}
	return NIL;
}

int StringToInt(string str){
	int n = 0;
	istringstream str2int(str);
	if (str2int.good()) {
		 str2int >> n;
	}
	return n;
}

string IntToString(int x){
	ostringstream int2str;
	if (int2str.good()) {
		 int2str << x;
	}
	return int2str.str();
}

long StringToLong(string str){
	long n = 0;
	istringstream str2int(str);
	if (str2int.good()) {
		 str2int >> n;
	}
	return n;
}

unsigned long StringToUnsignedLong(string str){
	unsigned long n = 0;
	istringstream str2int(str);
	if (str2int.good()) {
		 str2int >> n;
	}
	return n;
}

float StringToFloat(string str){
	float n = 0;
	istringstream str2int(str);
	if (str2int.good()) {
		 str2int >> n;
	}
	return n;
}
string StringToSym(string str){ // get rid of quoting marks '||'
	
	string ret = "";
	if(str[0] == '|' && str[str.length()-1] == '|'){
		ret = str.substr(1, str.length()-2);
	}
	else{
		ret = str;
	}
	if(YJ_debug) cout << "Changed value " << str << endl;
	return ret;
}


unsigned long id_to_unique_index(identifier *id){
	return ((id->name_letter<<20)+id->name_number);
}


test smem_make_test_from_symbol(agent* thisAgent, Symbol* s){

	if(s->common.symbol_type != IDENTIFIER_SYMBOL_TYPE){ // constant
		if(s->common.symbol_type == VARIABLE_SYMBOL_TYPE){ // variable
			return (make_equality_test(s));
		}		
		else if(s->common.symbol_type == SYM_CONSTANT_SYMBOL_TYPE) {// char*
			return (make_equality_test(make_sym_constant(thisAgent, s->sc.name)));
		}
		else if(s->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE) { // int
			return (make_equality_test(make_int_constant(thisAgent, s->ic.value)));
		}
		else{ // float
			return (make_equality_test(make_float_constant(thisAgent, s->fc.value)));
		}
	}
	return NULL;
}

test smem_make_rhs_value_from_symbol(agent* thisAgent, Symbol* s){

	if(s->common.symbol_type != IDENTIFIER_SYMBOL_TYPE){ // constant
		if(s->common.symbol_type == VARIABLE_SYMBOL_TYPE){ // variable
			return (symbol_to_rhs_value(s));
		}		
		else if(s->common.symbol_type == SYM_CONSTANT_SYMBOL_TYPE) {// char*
			return (symbol_to_rhs_value(make_sym_constant(thisAgent, s->sc.name)));
		}
		else if(s->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE) { // int
			return (symbol_to_rhs_value(make_int_constant(thisAgent, s->ic.value)));
		}
		else{ // float
			return (symbol_to_rhs_value(make_float_constant(thisAgent, s->fc.value)));
		}
	}
	return NULL;
}

// call this recursively to build the first level condistion lists start from top level ids, then connect the lists to top state
// The binding_variable argument is the variable generated as the value_test from one level higher, the same variable be passed all sibling slots.
// Problem: doesn't consider multiple paths problem. Need to test same WMEs from different slots. WME hash? Yes
condition* smem_make_condition_list_from_wmes(agent* thisAgent, 
											  identifier *id, 
											  Symbol* binding_variable, 
											  condition** lhs_bottom, 
											  std::map<unsigned long, Symbol*> *indetifier_2_variable_map){
	slot *s;
	wme *w;
	condition* lhs_top = NULL, *temp_ptr = NULL, *current_ptr;
	
	*lhs_bottom = NULL;

	//Symbol *s;

	//reset_variable_generator (NULL, NULL);
	//s = generate_new_variable(id->name_letter); // Generate a variable for that id, and use it as id_test for all slots.

	for (s = id->slots; s != NIL; s = s->next) {
		for (w = s->wmes; w != NIL; w = w->next){
			//allocate_with_pool (thisAgent, &thisAgent->action_pool, &a);
			allocate_with_pool (thisAgent, &thisAgent->condition_pool,  &temp_ptr);
			temp_ptr->type = POSITIVE_CONDITION;
			temp_ptr->next = NULL;
			temp_ptr->prev = NULL;
			temp_ptr->test_for_acceptable_preference = FALSE;

			temp_ptr->data.tests.id_test = make_equality_test(binding_variable);
			temp_ptr->data.tests.attr_test = smem_make_test_from_symbol(thisAgent, w->attr); // attr should always be constant in WMEs
			temp_ptr->data.tests.value_test = smem_make_test_from_symbol(thisAgent, w->value);

			if(temp_ptr->data.tests.value_test == NULL){ // identifier
				
				identifier value_id = w->value->id;
				
				unsigned long index_value = id_to_unique_index(&value_id);
				
				Symbol* variable;
				std::map<unsigned long, Symbol*>::iterator itr = indetifier_2_variable_map->find(index_value);
				if(itr == indetifier_2_variable_map->end()){
					char name[2];
					
					name[0] = w->attr->sc.name[0];
					name[1] = 0;
					variable = generate_new_variable(thisAgent, name); // generate a binding variable, and pass it down
					indetifier_2_variable_map->insert(std::pair<unsigned long, Symbol*>(index_value, variable));
				}
				else{
					variable = itr->second;
				}
				
				temp_ptr->data.tests.value_test = smem_make_test_from_symbol(thisAgent, variable); // variable test

				temp_ptr->next = smem_make_condition_list_from_wmes(thisAgent, &value_id, variable, lhs_bottom, indetifier_2_variable_map); // recursive call, lhs_bottom updated
				temp_ptr->next->prev = temp_ptr;
			}
			else{
//				print_wme(w);
				*lhs_bottom = temp_ptr; // not called recursively, then lhs_bottom is temp_ptr itself
			}

			// for sibling conditions
			if(lhs_top == NULL){ // first condition
				lhs_top = temp_ptr;
				current_ptr = *lhs_bottom;
			}
			else{ // later conditions
				current_ptr->next = temp_ptr;
				temp_ptr->prev = current_ptr;
				current_ptr = *lhs_bottom;

			}
		}
	}

	return lhs_top;
}


action* smem_make_action_list_from_wmes(agent* thisAgent, 
											  identifier *id, 
											  Symbol* binding_variable, 
											  action** rhs_bottom,
											  std::map<unsigned long, Symbol*> *indetifier_2_variable_map){
	slot *s;
	wme *w;
	action* rhs = NULL, *temp_ptr = NULL, *current_ptr;
	*rhs_bottom = NULL;

	//Symbol *s;

	//reset_variable_generator (NULL, NULL);
	//s = generate_new_variable(id->name_letter); // Generate a variable for that id, and use it as id_test for all slots.

	for (s = id->slots; s != NIL; s = s->next) {
		for (w = s->wmes; w != NIL; w = w->next){
			allocate_with_pool (thisAgent, &thisAgent->action_pool,  &temp_ptr);
			temp_ptr->next = NULL;
			temp_ptr->type = MAKE_ACTION;
			temp_ptr->preference_type = ACCEPTABLE_PREFERENCE_TYPE;
			
			temp_ptr->id = symbol_to_rhs_value(binding_variable);
			symbol_add_ref (binding_variable); // for variable need add_ref, only for action???

			temp_ptr->attr = smem_make_rhs_value_from_symbol(thisAgent, w->attr);// attr should always be constant in WMEs

			temp_ptr->value = smem_make_rhs_value_from_symbol(thisAgent, w->value);


			if(temp_ptr->value == NULL){ // identifier
				
				identifier value_id = w->value->id;
				
				unsigned long index_value = id_to_unique_index(&value_id);
				
				Symbol* variable;
				std::map<unsigned long, Symbol*>::iterator itr = indetifier_2_variable_map->find(index_value);
				if(itr == indetifier_2_variable_map->end()){
					char name[2];
					
					name[0] = w->attr->sc.name[0];
					name[1] = 0;
					variable = generate_new_variable(thisAgent, name); // generate a binding variable, and pass it down
					indetifier_2_variable_map->insert(std::pair<unsigned long, Symbol*>(index_value, variable));
				}
				else{
					variable = itr->second;
				}
				
				temp_ptr->value = smem_make_rhs_value_from_symbol(thisAgent, variable); // variable test

				temp_ptr->next = smem_make_action_list_from_wmes(thisAgent, &value_id, variable, rhs_bottom, indetifier_2_variable_map); // recursive call, lhs_bottom updated

			}
			else{
//				print_wme(w);
				*rhs_bottom = temp_ptr; // not called recursively, then lhs_bottom is temp_ptr itself
			}

			// for sibling conditions
			if(rhs == NULL){ // first condition
				rhs = temp_ptr;
				current_ptr = *rhs_bottom;
			}
			else{ // later conditions
				current_ptr->next = temp_ptr;
				current_ptr = *rhs_bottom;

			}
		}
	}

	return rhs;
}


void check_linkedlist(agent* thisAgent, condition* top, condition* bottom){
	condition* temp = top;
	print(thisAgent, "TOP DOWN\n");
	while(temp != NULL){
		//;test_to_string(id_test, NULL, 0)
		print(thisAgent, "%s, %s, %s\n", test_to_string(thisAgent, temp->data.tests.id_test, NULL,0), test_to_string(thisAgent, temp->data.tests.attr_test, NULL,0), test_to_string(thisAgent, temp->data.tests.value_test,NULL,0));
		temp = temp->next;
	}
	print(thisAgent, "BOTTOM UP\n");
	temp = bottom;
	while(temp != NULL){
		print(thisAgent, "%s, %s, %s\n", test_to_string(thisAgent, temp->data.tests.id_test, NULL,0), test_to_string(thisAgent, temp->data.tests.attr_test, NULL,0), test_to_string(thisAgent, temp->data.tests.value_test,NULL,0));
		temp = temp->prev;
	}
}


condition* smem_build_condition_from_root(agent* thisAgent, 
										  identifier *root, Symbol* top_state, 
										  condition** lhs_bottom, 
										  std::map<unsigned long, Symbol*> *indetifier_2_variable_map){

	condition *lhs_top = NULL, *temp_ptr, *current_ptr;
	complex_test *ct;
	test id_goal_impasse_test, id_test;
	slot *s;
	wme *w;

/*
allocate_with_pool (&current_agent(condition_pool),  &lhs_top);
			lhs_top->type = POSITIVE_CONDITION;
			lhs_top->next = NULL;
			lhs_top->prev = NULL;
			lhs_top->test_for_acceptable_preference = FALSE;
*/
	allocate_with_pool (thisAgent, &thisAgent->complex_test_pool,  &ct);
	ct->type = GOAL_ID_TEST;
	id_goal_impasse_test = make_test_from_complex_test(ct);


	id_test = make_equality_test(top_state);

	add_new_test_to_test (thisAgent, &id_test, id_goal_impasse_test);

	for (s = root->slots; s != NIL; s = s->next) { // top level conditions
		for (w = s->wmes; w != NIL; w = w->next){
			

			allocate_with_pool (thisAgent, &thisAgent->condition_pool,  &temp_ptr);
			temp_ptr->type = POSITIVE_CONDITION;
			temp_ptr->next = NULL;
			temp_ptr->prev = NULL;
			temp_ptr->test_for_acceptable_preference = FALSE;
			
			if(lhs_top == NULL){
				temp_ptr->data.tests.id_test = id_test; // state <s>
			}
			else{
				temp_ptr->data.tests.id_test = make_equality_test(top_state); // state <s>
			}
			
			temp_ptr->data.tests.attr_test = smem_make_test_from_symbol(thisAgent, w->attr);
			temp_ptr->data.tests.value_test = smem_make_test_from_symbol(thisAgent, w->value);
			if(temp_ptr->data.tests.value_test == NULL){ // identifier
				identifier value_id = w->value->id;
				char name[2];
				Symbol* variable;
				name[0] = w->attr->sc.name[0];
				name[1] = 0;
				variable = generate_new_variable(thisAgent, name); // generate a binding variable, and pass it down
				temp_ptr->data.tests.value_test = smem_make_test_from_symbol(thisAgent, variable); // variable test

				temp_ptr->next = smem_make_condition_list_from_wmes(thisAgent, &value_id, variable, lhs_bottom, indetifier_2_variable_map);
				temp_ptr->next->prev = temp_ptr;
			}
			else{
				*lhs_bottom = temp_ptr; // not called recursively, then lhs_bottom is temp_ptr itself
			}
			// for sibling conditions
			if(lhs_top == NULL){ // first condition
				lhs_top = temp_ptr; // temp_ptr is the head of returned condition list.

				current_ptr = *lhs_bottom;
			}
			else{ // later conditions
		
				current_ptr->next = temp_ptr;
				temp_ptr->prev = current_ptr;
				current_ptr = *lhs_bottom;				
			}

		}
	}
	//lhs_top->data.tests.attr_test = make_equality_test(make_sym_constant("condition_attr"));
	//lhs_top->data.tests.value_test = make_equality_test(make_sym_constant("condition_value"));
	
	check_linkedlist(thisAgent, lhs_top, *lhs_bottom);
	return lhs_top;
}


action* smem_build_action_from_root(agent* thisAgent, 
										  identifier *root, Symbol* top_state_variable, 
										  action** rhs_bottom, 
										  std::map<unsigned long, Symbol*> *indetifier_2_variable_map){

	action *rhs_top = NULL, *temp_ptr, *current_ptr;
	complex_test *ct;
	slot *s;
	wme *w;

/*
allocate_with_pool (&current_agent(condition_pool),  &lhs_top);
			lhs_top->type = POSITIVE_CONDITION;
			lhs_top->next = NULL;
			lhs_top->prev = NULL;
			lhs_top->test_for_acceptable_preference = FALSE;
*/

	for (s = root->slots; s != NIL; s = s->next) { // top level conditions
		for (w = s->wmes; w != NIL; w = w->next){
			
			
			allocate_with_pool (thisAgent, &thisAgent->action_pool,  &temp_ptr);
			temp_ptr->next = NULL;
			temp_ptr->type = MAKE_ACTION;
			temp_ptr->preference_type = ACCEPTABLE_PREFERENCE_TYPE;
			
			temp_ptr->id = symbol_to_rhs_value(top_state_variable);
			symbol_add_ref (top_state_variable);
			temp_ptr->attr = smem_make_rhs_value_from_symbol(thisAgent, w->attr);
			temp_ptr->value = smem_make_rhs_value_from_symbol(thisAgent, w->value);


			if(temp_ptr->value == NULL){ // identifier
				identifier value_id = w->value->id;
				char name[2];
				Symbol* variable;
				name[0] = w->attr->sc.name[0];
				name[1] = 0;
				variable = generate_new_variable(thisAgent, name); // generate a binding variable, and pass it down
				temp_ptr->value = smem_make_rhs_value_from_symbol(thisAgent, variable); // variable test

				temp_ptr->next = smem_make_action_list_from_wmes(thisAgent, &value_id, variable, rhs_bottom, indetifier_2_variable_map);
				
			}
			else{
				*rhs_bottom = temp_ptr; // not called recursively, then lhs_bottom is temp_ptr itself
			}
			// for sibling conditions
			if(rhs_top == NULL){ // first condition
				rhs_top = temp_ptr; // temp_ptr is the head of returned condition list.
				current_ptr = *rhs_bottom;
			}
			else{ // later conditions
		
				current_ptr->next = temp_ptr;
				current_ptr = *rhs_bottom;				
			}

		}
	}
	
	
	return rhs_top;
}

action* smem_build_action_from_root(agent *thisAgent, identifier *root, Symbol* top_state){
	action *rhs;
	/* --- create the RHS --- */
	allocate_with_pool (thisAgent, &thisAgent->action_pool,  &rhs);
	rhs->next = NULL;
	rhs->type = MAKE_ACTION;
	rhs->preference_type = ACCEPTABLE_PREFERENCE_TYPE;

	rhs->id = symbol_to_rhs_value(top_state);
	symbol_add_ref (top_state); // for variable need add_ref
	rhs->attr = symbol_to_rhs_value(make_sym_constant(thisAgent, "action_attr"));
	rhs->value = symbol_to_rhs_value(make_sym_constant(thisAgent, "action_value"));
	
	return rhs;

}

void smem_build_rule_from_rule_link(agent *thisAgent, Symbol *rule_link_symbol){
	Symbol *s, *prod_name;
	condition *lhs_top = NULL, *lhs_bottom = NULL;
	action *rhs = NULL, *rhs_bottom = NULL;
	identifier *rule_link;
	
	production *p;
	byte rete_addition_result;
	
	rule_link = &(rule_link_symbol->id);
	
	// First check status
	for (wme *w = rule_link->input_wmes; w != NIL; w = w->next) { 
		if(strcmp(w->attr->sc.name, "status") == 0 &&
			w->value->common.symbol_type == SYM_CONSTANT_SYMBOL_TYPE &&
			strcmp(w->value->sc.name, "complete") == 0){ // check status
				print_with_symbols(thisAgent, "\nRule %y already created, status complete\n", w->id);
				return;
			}
	}

	std::map<unsigned long, Symbol*> *indetifier_2_variable_map = new std::map<unsigned long, Symbol*>();
	
	// conditions are connected to top state
	/* state <s> */
	reset_variable_generator (thisAgent, NULL, NULL);
	s = generate_new_variable(thisAgent, "s");
	
	// left hand side
	// shoudl start from the value of 'conditions' attribute of all first level wmes, if the attribite is 'rule' 
	
	for (slot *s1 = rule_link->slots; s1 != NIL; s1 = s1->next) { // top level conditions
		if(strcmp(s1->attr->sc.name, "conditions") == 0){ // slot attr should be unique for a given id
		//for (wme *w = s1->wmes; w != NIL; w = w->next){
			wme *w = s1->wmes; // just the first wme
			if(w->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE){
				lhs_top = smem_build_condition_from_root(thisAgent, &(w->value->id), s, &(lhs_bottom), indetifier_2_variable_map);
				break;
			}
		}
	}

	


	// right hand side
	for (slot *s1 = rule_link->slots; s1 != NIL; s1 = s1->next) { // top level conditions
		if(strcmp(s1->attr->sc.name, "actions") == 0){ // slot attr should be unique for a given id
		//for (wme *w = s1->wmes; w != NIL; w = w->next){
			wme *w = s1->wmes; // just the first wme
			if(w->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE){
				rhs = smem_build_action_from_root(thisAgent, &(w->value->id), s, &(rhs_bottom), indetifier_2_variable_map);
				break;
			}
		}
	}
	delete indetifier_2_variable_map;

	if(lhs_top == NULL || rhs == NULL)
		return;

	/* --- make the production structure --- */
	prod_name = generate_new_sym_constant (thisAgent, "association*rule*",&thisAgent->association_rule_counter); // counter automatically incremented by this function
	
	p = make_production (thisAgent, USER_PRODUCTION_TYPE, prod_name, &lhs_top, &lhs_bottom, &rhs, TRUE);

	if (!p) {
		print_with_symbols (thisAgent, "(Failure adding test production)\n\n");
		deallocate_condition_list (thisAgent, lhs_top);
		deallocate_action_list (thisAgent, rhs);
		return;
	}
	

	rete_addition_result = add_production_to_rete (thisAgent, p, lhs_top, NULL, TRUE);
	deallocate_condition_list (thisAgent, lhs_top); // why not deallocate_action_list; rete copied conditions but actions need to keep its own?
	//deallocate_action_list (thisAgent, rhs);
	

	if (rete_addition_result==DUPLICATE_PRODUCTION) {
		excise_production (thisAgent, p, FALSE);
		print(thisAgent, "\nAny kind of duplicate\n");
		thisAgent->association_rule_counter--;
		p = NULL;
	}
	else{
		print_with_symbols(thisAgent, "Production \"%y\" from rec_link ^ %y created\n", prod_name, rule_link_symbol);
		add_input_wme(thisAgent, rule_link_symbol, make_sym_constant(thisAgent, "status"), make_sym_constant(thisAgent, "complete"));
		//wme *w = make_wme (thisAgent, rule_link_symbol, make_sym_constant(thisAgent, "status"), make_sym_constant(thisAgent, "complete"), FALSE);
		//add_wme_to_wm (thisAgent, w);
		//add_wme_to_rete (thisAgent, w);

		print(thisAgent, "WME added \n");
	}
	
	

}
void smem_build_rules_from_rec_link(agent *thisAgent){
	slot *s;
	wme *w;
	for (s = thisAgent->rec_link_header->id.slots; s != NIL; s = s->next) { // top level conditions
		for (w = s->wmes; w != NIL; w = w->next){
			if(strcmp(w->attr->sc.name, "rule") == 0){ // is a rule link
			if(w->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE)
				smem_build_rule_from_rule_link(thisAgent, w->value);
				
			}
		}
	}
	
}

// Adding this to function create_new_context()
void append_smem_links(agent* thisAgent){
	//print (thisAgent, "creating new smem links\n");
	Symbol* goal_id = thisAgent->bottom_goal;
	goal_stack_level goal_level = goal_id->id.level;
	Symbol* smem_id = make_new_identifier(thisAgent, 'M', goal_level); // get_new_io_identifier assumes TOP_GOAL_LEVEL !
	Symbol* save_id = make_new_identifier(thisAgent, 'E', goal_level);
	Symbol* retrieve_id = make_new_identifier(thisAgent, 'R', goal_level);
	Symbol* cluster_id = make_new_identifier(thisAgent, 'C', goal_level);

	wme* smem_wme = add_input_wme(thisAgent, goal_id, make_sym_constant(thisAgent, "smem"), smem_id);	 
	wme* retrieve_wme = add_input_wme(thisAgent, smem_id, make_sym_constant(thisAgent, "retrieve"), retrieve_id);		
	wme* save_wme = add_input_wme(thisAgent, smem_id, make_sym_constant(thisAgent, "save"), save_id);
	wme* cluster_wme = add_input_wme(thisAgent, smem_id, make_sym_constant(thisAgent, "cluster"), cluster_id);

	// The id's created above are leaked on init-soar.  I think if we do a
	// symbol_remove_ref() here on each of them, they will no longer leak.  The wme's
	// themselves get cleaned up when the state is removed.  KJC 06/06
    symbol_remove_ref(thisAgent, smem_id);
    symbol_remove_ref(thisAgent, save_id);
    symbol_remove_ref(thisAgent, retrieve_id);
    symbol_remove_ref(thisAgent, cluster_id);


	vector<wme*> wmes;
	
	wmes.push_back(retrieve_wme);
	wmes.push_back(save_wme);
	wmes.push_back(smem_wme);
	wmes.push_back(cluster_wme);
	thisAgent->gold_level_to_smem_links->push_back(wmes);

	//thisAgent->gold_to_smem_links
	

}
// This should be done at the time a subgoal is created
// instead of checking existence of smeme links every cycle
void create_subgoal_links(agent* thisAgent){
	//print (thisAgent, "Checking smem links\n");
	bool added = false;
	for(wme* w = thisAgent->bottom_goal->id.input_wmes; w != NIL; w=w->next){
		if(strcmp(symbol_constant_to_string(thisAgent, w->attr).c_str(), "smem") == 0){
			added = true;
			break;
		}
	}
	if(!added){
		append_smem_links(thisAgent);
	}
	else{
		//print(thisAgent, "Already exist smem links\n");
	}
}

void YJ_input_phase_call(agent* thisAgent){
	//create_subgoal_links(thisAgent);
}
void YJ_decision_phase_call(agent* thisAgent){

}
void YJ_application_phase_call(agent* thisAgent){
//	create_subgoal_links(thisAgent);
}

// This is the output_phase call

void smem_routine(agent* thisAgent){
	//wme* w = current_agent(rec_header_link);
	//get_augs_of_id();
	//print_augs_of_id(current_agent(top_state), 10, 0,0,10);
	//current_agent(top_state)->id.tc_num = 1;
	// tc_num is used to mark off a id

	//make_production_test();
	//make_foo_production("test*foo");


	//smem_build_rules_from_rec_link(thisAgent);
	
	//create_subgoal_links(thisAgent); // this is done automatically now
	//print(thisAgent, "\nprinting\n");
	//thisAgent->semantic_memory->dump(std::if(YJ_debug) cout);
	
	//save_wmes(thisAgent);
	//thisAgent->semantic_memory->dump(std::if(YJ_debug) cout);
	
	if(thisAgent->sysparams[SMEM_SYSPARAM] == 1){
	  //print(thisAgent, "\nSMEM Enabled (%d)\n",thisAgent->sysparams[SMEM_SYSPARAM]);
		save_wmes_12_21(thisAgent);
		//retrieve(thisAgent);
		//retrieve_1_20(thisAgent);
		retrieve_3_13(thisAgent);
		cluster(thisAgent);
	}
	else if(thisAgent->sysparams[SMEM_SYSPARAM] == 0){// deliberate saving
	  //print(thisAgent, "\nSMEM Disabled (%d)\n",thisAgent->sysparams[SMEM_SYSPARAM]);
		//save_wmes_old(thisAgent);
		save_wmes_deliberate_merge_identical(thisAgent);
		//retrieve(thisAgent); // should still return failure anyway
		//retrieve_1_20(thisAgent);
		retrieve_3_13(thisAgent);
		cluster(thisAgent);
	}
	//print(thisAgent, "\nCurrent cycle %d\n", thisAgent->d_cycle_count);
	//thisAgent->semantic_memory.dump(std::if(YJ_debug) cout);
	//thisAgent->semantic_memory->print();
	//thisAgent->semantic_memory.dump(std::if(YJ_debug) cout);
	//stdext::hash_map<std::string, std::string, std::hash_compare<std::string, std::string>,
	//	SoarMemoryPoolAllocator<pair<std::string, std::string> > >::iterator itr = thisAgent->test_hash->begin();

}


// Semantic memory links related functions
// Save Link
// Cue Link
// Expand Link

// Save the wmes pinted to by SaveLink into semantic memroy
string symbol_constant_to_string(agent* thisAgent, Symbol* s){
	//int len = strlen(s->sc.name);
	//#define IDENTIFIER_SYMBOL_TYPE 1
	//#define SYM_CONSTANT_SYMBOL_TYPE 2
	//#define INT_CONSTANT_SYMBOL_TYPE 3
	//#define FLOAT_CONSTANT_SYMBOL_TYPE 4
	
	if(s->common.symbol_type != VARIABLE_SYMBOL_TYPE &&
		s->common.symbol_type != IDENTIFIER_SYMBOL_TYPE &&
		s->common.symbol_type != INT_CONSTANT_SYMBOL_TYPE &&
		s->common.symbol_type != FLOAT_CONSTANT_SYMBOL_TYPE &&
		s->common.symbol_type != SYM_CONSTANT_SYMBOL_TYPE){
		return "bad symbol";
	}

	char sym_char[1000];
	symbol_to_string (thisAgent, s, TRUE, sym_char, 1000);
	return string(sym_char);
}
/*
void save_wmes_old(agent* thisAgent){

	
	//thisAgent->semantic_memory->dump(if(YJ_debug) cout);

	slot *s;
	wme *w;
	for (s = thisAgent->save_link_id->id.slots; s != NIL; s = s->next) { // top level conditions
		for (w = s->wmes; w != NIL; w = w->next){ // slots should be holding unique values in WM 
			
			string id = symbol_constant_to_string(thisAgent, w->id); // ID must be SYM_CONSTANT_SYMBOL_TYPE
			string attr = symbol_constant_to_string(thisAgent, w->attr); // attr must be SYM_CONSTANT_SYMBOL_TYPE
			string value = "";
			int value_type = w->value->common.symbol_type;
			bool constant = false;
			if(w->value->common.symbol_type == SYM_CONSTANT_SYMBOL_TYPE){
				value = symbol_constant_to_string(thisAgent, w->value);
				constant = true;

			}
			else if(w->value->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE ||
				w->value->common.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE){
				value = symbol_constant_to_string(thisAgent, w->value);
				constant = true;
			}
			else if(w->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE){
				constant = false;
			}
			print(thisAgent, "\n");
			print_wme(thisAgent, w);
			print_with_symbols(thisAgent, "%y, %y,%y\n", w->id, w->attr, w->value);
			//if(YJ_debug) cout << id<<"?"<<attr<<"?"<<value<<endl;
			print(thisAgent, "\n");
			
			thisAgent->semantic_memory->insert_LME(id,attr,value,value_type);
		}
	}
	thisAgent->semantic_memory->dump(std::if(YJ_debug) cout);
	
}
*/
bool long_term_value(agent* thisAgent, const set<std::string>& saved_ids, string value, int value_type){
	if(YJ_debug) cout << value <<"," << value_type<<endl;
	if(value_type == INT_CONSTANT_SYMBOL_TYPE || value_type == FLOAT_CONSTANT_SYMBOL_TYPE || value_type == SYM_CONSTANT_SYMBOL_TYPE
		|| 
		(value_type == IDENTIFIER_SYMBOL_TYPE && (saved_ids.find(value) !=saved_ids.end() || thisAgent->semantic_memory->test_id(value)))
		)
	{
		if(YJ_debug) cout << "YES"<<endl;
		return true;
	}
	else{
		if(YJ_debug) cout << "NO" << endl;
		return false;
	}
}
#if (0)
// Don't do retrieve every cycle as the older version - need to reset the retrieve ready mark
// Every time the cue structure is not dectected or as empty, the retrieve ready mark is set on
// Only when it's ready and there is a non-empty cue strcuture, does it performe the actual retrieval.
// And don't do retrive id and expand id separately.
// Failure is also returned together.
// If old cue is cleared and new clue is placed in the same decision cycle, it won't perfrom the new retrieval.
// And the old retrieved result won't be cleared.
// Normally, should assume that, there is a follow-on step to use the retrieved result.
// Clearing old cue and placing new cue must be done by separate operators. And clear cue should probably be in the application phase of use-result operator
void retrieve_1_20(agent* thisAgent){
	slot *retrieve_s;
	Symbol* retrieve_link_id = find_retrieve_link_id(thisAgent);
	wme *retrieve_w;
	Symbol* cue_id = NIL;

	bool find_cue = false;
	for (retrieve_s = retrieve_link_id->id.slots; retrieve_s != NIL; retrieve_s = retrieve_s->next) {
		// The slots can potentially have any structure
		for (retrieve_w = retrieve_s->wmes; retrieve_w != NIL; retrieve_w = retrieve_w->next){ 

			if(strcmp(symbol_constant_to_string(thisAgent, retrieve_w->attr).c_str(), "cue") == 0){
				find_cue = true;
				// Do retrieve only if ready mark is set on
				if(!thisAgent->retrieve_ready){
					print(thisAgent, "Retrieve is not ready\n");
					break;
				}
				// Set ready mark off after the retrieval
				else if(retrieve_w->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE){ // cue must be an identifier, as the chunk name. Constant cue is not an option
					thisAgent->retrieve_ready = false;
					
					cue_id = retrieve_w->value;
					slot *s;
					wme *w;
					string picked_id = "";
					Symbol* picked_id_sym = NIL;
					set<CueTriplet> cue_set;
					for (s = cue_id->id.slots; s != NIL; s = s->next){ // collect the cue WMEs
						for(w = s->wmes; w != NIL; w = w->next){
							string id = symbol_constant_to_string(thisAgent, w->id); // ID must be SYM_CONSTANT_SYMBOL_TYPE
							string attr = symbol_constant_to_string(thisAgent, w->attr); // attr must be SYM_CONSTANT_SYMBOL_TYPE
							string value = symbol_constant_to_string(thisAgent, w->value);
							int value_type = w->value->common.symbol_type;
							if(value_type == SYM_CONSTANT_SYMBOL_TYPE){
								value = StringToSym(value);
							}
							cue_set.insert(CueTriplet(id, attr, value, value_type));
							if(YJ_debug)
								print(thisAgent, "\nCue WME<%s, %s,%s, %d>\n", id.c_str(), attr.c_str(), value.c_str(),value_type);
						}
					}

					// The following is the retrieval code given the cue collected
					
					set<CueTriplet> result = thisAgent->semantic_memory->match_retrieve_single_level_2006_1_22(cue_set, picked_id);
					// this may duplicate what is already in working memory
					// If it's already in WM, just return that pointer and put the extra attributes there
					// By current code, the existing wme is removed!
					picked_id_sym = make_identifier_from_str(thisAgent, picked_id);

					wme* retrieved_wme = add_input_wme_with_history(thisAgent, retrieve_link_id, make_sym_constant(thisAgent, "retrieved"), picked_id_sym);
					for(set<CueTriplet>::const_iterator itr = result.begin(); itr != result.end(); ++itr){
						string id = itr->id; // this id must == picked_id
						string attr = itr->attr;
						string value = itr->value;
						int value_type = itr->value_type;
						CueTriplet current_wme = CueTriplet(id, attr, value, value_type);
						Symbol* attr_sym = make_sym_constant(thisAgent, (char*)attr.c_str());
						Symbol* value_sym = NIL; 
						if(value_type == IDENTIFIER_SYMBOL_TYPE){
							value_sym = make_identifier_from_str(thisAgent, value);
						}
						else{
							if(value_type == SYM_CONSTANT_SYMBOL_TYPE){
								value = StringToSym(value);
								value_sym = make_sym_constant(thisAgent, (char*)value.c_str()); 
							}
							else if(value_type == INT_CONSTANT_SYMBOL_TYPE){
								value_sym = make_int_constant(thisAgent, StringToLong(value)); 
							}
							else if(value_type == FLOAT_CONSTANT_SYMBOL_TYPE){
								value_sym = make_float_constant(thisAgent, StringToFloat(value)); 
							}
						}
						if(YJ_debug)
							print(thisAgent, "\nAdding <%s(%s), %s,%s, %d> as input_wme\n", picked_id.c_str(),id.c_str(), attr.c_str(), value.c_str(), value_type);
						add_input_wme_with_history(thisAgent, picked_id_sym, attr_sym, value_sym);
					}


					/*
					set<string> ids = thisAgent->semantic_memory->match_retrieve_single_level(cue_set);// retrieving
					if(YJ_debug) cout << "Matched IDS:" << endl;
					if(YJ_debug) cout << ids << endl;
					if(ids.size() > 0){ // should never return empty set, may need to set a long-term-id for FAILURE chunk

						picked_id = *(ids.begin()); // just pick the 1st for the moment, This must be a long term id
						// it could be the failure chunk id
						picked_id_sym = make_identifier_from_str(thisAgent, picked_id);
						wme* retrieved_wme = add_input_wme_with_history(thisAgent, retrieve_link_id, make_sym_constant(thisAgent, "retrieved"), picked_id_sym);
					}
					else{ // failure of retrieval
						picked_id = "F0";
						picked_id_sym = make_identifier_from_str(thisAgent, picked_id);

						wme* retrieved_wme = add_input_wme_with_history(thisAgent, retrieve_link_id, make_sym_constant(thisAgent, "retrieved"), picked_id_sym);

						string attr = "status";
						string value = "failure";
						int value_type = 2;
						add_input_wme_with_history(thisAgent, picked_id_sym, make_sym_constant(thisAgent, "status"), make_sym_constant(thisAgent, "failure"));
					}

					if(picked_id != "F0"){ // failure id, it's not actually in semantic memory, don't expand it
						set<LME> retrieved_LMEs = thisAgent->semantic_memory->expand_id(picked_id, cue_set);
						// One situation is, when retrieved is already in WM, only add attributes that is not already there.
						// Need to add new wmes using the retrieved information, with picked_id_sym being the identifier.
						// If a wme is previously retrieved, then do nothing, otherwise add that wme
						for(set<LME>::iterator itr = retrieved_LMEs.begin(); itr != retrieved_LMEs.end(); ++itr){
							string id = itr->id; // this id must == picked_id
							string attr = itr->attr;
							string value = itr->value;
							int value_type = itr->value_type;
							CueTriplet current_wme = CueTriplet(id, attr, value, value_type);
							Symbol* attr_sym = make_sym_constant(thisAgent, (char*)attr.c_str());
							Symbol* value_sym = NIL; 
							if(value_type == IDENTIFIER_SYMBOL_TYPE){
								value_sym = make_identifier_from_str(thisAgent, value);
							}
							else{
								if(value_type == SYM_CONSTANT_SYMBOL_TYPE){
									value = StringToSym(value);
									value_sym = make_sym_constant(thisAgent, (char*)value.c_str()); 
								}
								else if(value_type == INT_CONSTANT_SYMBOL_TYPE){
									value_sym = make_int_constant(thisAgent, StringToLong(value)); 
								}
								else if(value_type == FLOAT_CONSTANT_SYMBOL_TYPE){
									value_sym = make_float_constant(thisAgent, StringToFloat(value)); 
								}
							}
							if(YJ_debug)
								print(thisAgent, "\nAdding <%s(%s), %s,%s, %d> as input_wme\n", picked_id.c_str(),id.c_str(), attr.c_str(), value.c_str(), value_type);
							add_input_wme_with_history(thisAgent, picked_id_sym, attr_sym, value_sym);

						} // for each retrieved wme
					}
					else{
						if(YJ_debug)
							print(thisAgent, "Retrieval Failure\n");
					}*/

					break; // only consider the first cue, even if there are mulitple cues.
				}	
			}
		}
	}


	if(!find_cue){//no cue structure, ready for new retrieval
		thisAgent->retrieve_ready = true;
		// Should automatically clear 'retrieved' link if it exists.
		for(wme* w=retrieve_link_id->id.input_wmes; w != NIL; w=w->next){ // clear retrieved result, which are input_wmes
			// 'cue' is regular wme.., 'save' should be regular too.
			// Only input_wme under smem link is retrieve link, only input_wme under 'retrieve' link is 'retrieved' link
			// S1 ^smem M1
			//		M1 ^retrieve R1
			//			R1 ^cue ?? ^retrieved ??
			// Better still check the attribute
			if(strcmp(symbol_constant_to_string(thisAgent, w->attr).c_str(), "retrieved") == 0){
				remove_input_wme(thisAgent, w); 
				// Just need to remove the link (retrieved).
				// substructures should be automatically taken care of.
			}
		}
	}

}
#endif

// This version of retrieval intends to summarize the target attribute, and return the meta-information about the 
//  retrieved value.
// Currently, seems 2 things are useful: 
// a. The percentage of the retrieved value (with the highest percentage of course): for categorical attribute. 
// For example, 80% percent of such mushrooms are edible (20% percent are other types, here only poisonous, for example)
//		For numerical ones, the average/mean might be wanted. But in some situations, average is not informative,
//		such as a distribution with multiple peaks.
// b. The experience with the query: how many times such chunks have been seen
//		This reflects the confidence about the retrieval.
//		For example, this type of muchroom have been observed 100 times, so it's pretty confident. If only 1 instances 
//		have been observed, it might not generalize well
// At different specificity level, the decision based on confidence should be different.
// For example, at the first level, the edibility might not generalize as well as for the second level given the same number of observations
// 
// It's hard to define, in general, what are the types of meta-info required.

void retrieve_3_13(agent* thisAgent){
	slot *retrieve_s;
	Symbol* retrieve_link_id = find_retrieve_link_id(thisAgent);
	wme *retrieve_w;
	Symbol* cue_id = NIL;

	bool find_cue = false;
	for (retrieve_s = retrieve_link_id->id.slots; retrieve_s != NIL; retrieve_s = retrieve_s->next) {
		// The slots can potentially have any structure
		if(strcmp(symbol_constant_to_string(thisAgent, retrieve_link_id->id.slots->attr).c_str(), "cue") == 0){
			
			for (retrieve_w = retrieve_s->wmes; retrieve_w != NIL; retrieve_w = retrieve_w->next){ 
				find_cue = true;
				// Do retrieve only if ready mark is set on
				if(!thisAgent->retrieve_ready){
					print(thisAgent, "Retrieve is not ready\n");
					break;
				}
				// Set ready mark off after the retrieval
				else if(retrieve_w->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE){ // cue must be an identifier, as the chunk name. Constant cue is not an option
					thisAgent->retrieve_ready = false;
					
					cue_id = retrieve_w->value;
					slot *s;
					wme *w;
					string picked_id = "";
					Symbol* picked_id_sym = NIL;
					set<CueTriplet> cue_set;
					for (s = cue_id->id.slots; s != NIL; s = s->next){ // collect the cue WMEs
						for(w = s->wmes; w != NIL; w = w->next){
							string id = symbol_constant_to_string(thisAgent, w->id); // ID must be SYM_CONSTANT_SYMBOL_TYPE
							string attr = symbol_constant_to_string(thisAgent, w->attr); // attr must be SYM_CONSTANT_SYMBOL_TYPE
							string value = symbol_constant_to_string(thisAgent, w->value);
							int value_type = w->value->common.symbol_type;
							if(value_type == SYM_CONSTANT_SYMBOL_TYPE){
								value = StringToSym(value);
							}
							cue_set.insert(CueTriplet(id, attr, value, value_type));
							if(YJ_debug)
								print(thisAgent, "\nCue WME<%s, %s,%s, %d>\n", id.c_str(), attr.c_str(), value.c_str(),value_type);
						}
					}

					// The following is the retrieval code given the cue collected
					set<CueTriplet> retrieved;
					float confidence = 0, experience = 0;
					
					// Clustering with summarization
					bool retrieve_status = thisAgent->semantic_memory->match_retrieve_single_level_2006_3_15(cue_set, picked_id, retrieved, confidence, experience);

					// Exemplar without summarization
					//float threshold = 0;
					//bool retrieve_status = thisAgent->semantic_memory->partial_match(cue_set, picked_id, retrieved, threshold, confidence, experience);


					// this may duplicate what is already in working memory
					// If it's already in WM, just return that pointer and put the extra attributes there
					// By current code, the existing wme is removed! ???
					picked_id_sym = make_identifier_from_str(thisAgent, picked_id);

					wme* retrieved_wme = add_input_wme_with_history(thisAgent, retrieve_link_id, make_sym_constant(thisAgent, "retrieved"), picked_id_sym);
					for(set<CueTriplet>::const_iterator itr = retrieved.begin(); itr != retrieved.end(); ++itr){
						string id = itr->id; // this id must == picked_id
						string attr = itr->attr;
						string value = itr->value;
						int value_type = itr->value_type;
						CueTriplet current_wme = CueTriplet(id, attr, value, value_type);
						Symbol* attr_sym = make_sym_constant(thisAgent, (char*)attr.c_str());
						Symbol* value_sym = NIL; 
						if(value_type == IDENTIFIER_SYMBOL_TYPE){
							value_sym = make_identifier_from_str(thisAgent, value);
						}
						else{
							if(value_type == SYM_CONSTANT_SYMBOL_TYPE){
								value = StringToSym(value);
								value_sym = make_sym_constant(thisAgent, (char*)value.c_str()); 
							}
							else if(value_type == INT_CONSTANT_SYMBOL_TYPE){
								value_sym = make_int_constant(thisAgent, StringToLong(value)); 
							}
							else if(value_type == FLOAT_CONSTANT_SYMBOL_TYPE){
								value_sym = make_float_constant(thisAgent, StringToFloat(value)); 
							}
						}
						if(YJ_debug)
							print(thisAgent, "\nAdding <%s(%s), %s,%s, %d> as input_wme\n", picked_id.c_str(),id.c_str(), attr.c_str(), value.c_str(), value_type);
						add_input_wme_with_history(thisAgent, picked_id_sym, attr_sym, value_sym);
					}
					if(!retrieve_status){ // success, need to put the meta-status
						// What are the meta-info for failure ? How confident is it to be fail? Doesn't make sense
					}
					add_input_wme_with_history
							(thisAgent, retrieve_link_id, make_sym_constant(thisAgent, "target-confidence"), make_float_constant(thisAgent, confidence));
					add_input_wme_with_history
							(thisAgent, retrieve_link_id, make_sym_constant(thisAgent, "target-experience"), make_int_constant(thisAgent, (int)experience));
					break; // only consider the first cue, even if there are mulitple cues.
				}	
			}
		}
	}


	if(!find_cue){//no cue structure, ready for new retrieval
		thisAgent->retrieve_ready = true;
		// Should automatically clear 'retrieved' link if it exists.
		for(wme* w=retrieve_link_id->id.input_wmes; w != NIL; w=w->next){ // clear retrieved result, which are input_wmes
			// 'cue' is regular wme.., 'save' should be regular too.
			// Only input_wme under smem link is retrieve link, only input_wme under 'retrieve' link is 'retrieved' link
			// S1 ^smem M1
			//		M1 ^retrieve R1
			//			R1 ^cue ?? ^retrieved ??
			// Better still check the attribute
			if(strcmp(symbol_constant_to_string(thisAgent, w->attr).c_str(), "retrieved") == 0 ||
				strcmp(symbol_constant_to_string(thisAgent, w->attr).c_str(), "target-confidence") == 0 ||
				strcmp(symbol_constant_to_string(thisAgent, w->attr).c_str(), "target-experience") == 0){
				remove_input_wme(thisAgent, w); 
				// Just need to remove the link (retrieved).
				// substructures should be automatically taken care of.
			}
		}
	}

}

#if (0)
void retrieve(agent* thisAgent){
	slot *retrieve_s;
	wme *retrieve_w;
	bool checked_cue = false;
	// Only for the first level
	Symbol* cue_id = NIL;
	Symbol* retrieve_link_id = find_retrieve_link_id(thisAgent);
	if(retrieve_link_id == NIL){
		if(YJ_debug){
			print (thisAgent, "No retrieve link\n");
		}
		return;
	}
	
	// Keep track of what have been retrieved, then if it should still be there according to the cue, then don't remove it,
	// otherwise, remove the wme
	string previous_retrieved_id = ""; // There should be only one top id.
	Symbol* previous_retrieved_id_sym = NIL;
	wme* previous_retrieved_wme = NIL;

	set<CueTriplet> previous_retrieved_wmes; // Need to know this set when adding new wmes
	set<CueTriplet> current_retrieved_wmes; // Need to know this set when removing old wmes
	
	for (retrieve_w = retrieve_link_id->id.input_wmes; retrieve_w != NIL; retrieve_w = retrieve_w->next){
			if(strcmp(symbol_constant_to_string(thisAgent, retrieve_w->attr).c_str(), "retrieved") == 0){ // add retrieved to be deleted wmes for next cycle
				//remove_input_wme(thisAgent, retrieve_w);
				previous_retrieved_id = symbol_constant_to_string(thisAgent, retrieve_w->value);
				previous_retrieved_id_sym = retrieve_w->value;
				previous_retrieved_wme = retrieve_w;

				if(retrieve_w->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE){//this should always be true,as retrieved is always identifier
					for (wme* w = retrieve_w->value->id.input_wmes; w != NIL; w = w->next){
						string wme_id = symbol_constant_to_string(thisAgent,w->id);
						string wme_attr = symbol_constant_to_string(thisAgent,w->attr);
						string wme_value = symbol_constant_to_string(thisAgent,w->value);
						int wme_value_type = w->value->common.symbol_type;
						
						if(wme_value_type == SYM_CONSTANT_SYMBOL_TYPE){
							// The might better done at saving time
							//wme_value = wme_value.substr(1, wme_value.length()-2); // get rid of quting marks
						}
						previous_retrieved_wmes.insert(CueTriplet(wme_id, wme_attr, wme_value, wme_value_type));
						if(YJ_debug)
						print(thisAgent, "\nPrevious retrieved: <%s, %s, %s, %d>\n", wme_id.c_str(), wme_attr.c_str(), wme_value.c_str(), wme_value_type);
					}
				}

				break; // just do one retrieve link
			
			}
			
		
	}


	for (retrieve_s = retrieve_link_id->id.slots; retrieve_s != NIL; retrieve_s = retrieve_s->next) { // cue link
		for (retrieve_w = retrieve_s->wmes; retrieve_w != NIL; retrieve_w = retrieve_w->next){ // slots should be holding unique values in WM , only look at normal wmes
			
			
			
			if(!checked_cue && 
				strcmp(symbol_constant_to_string(thisAgent, retrieve_w->attr).c_str(), "cue") == 0){
				if(retrieve_w->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE){ // cue must be an identifier, as the chunk name
					cue_id = retrieve_w->value;
					slot *s;
					wme *w;
					
					//print(thisAgent, "CUE\n");
					string pointed_id = symbol_constant_to_string(thisAgent,cue_id);
					string picked_id = "";
					Symbol* picked_id_sym = NIL;
					set<CueTriplet> cue_set;
					
					for (s = cue_id->id.slots; s != NIL; s = s->next){ // collect the cue WMEs
							for(w = s->wmes; w != NIL; w = w->next){
								string id = symbol_constant_to_string(thisAgent, w->id); // ID must be SYM_CONSTANT_SYMBOL_TYPE
								string attr = symbol_constant_to_string(thisAgent, w->attr); // attr must be SYM_CONSTANT_SYMBOL_TYPE
								string value = symbol_constant_to_string(thisAgent, w->value);
								int value_type = w->value->common.symbol_type;
								cue_set.insert(CueTriplet(id, attr, value, value_type));
								if(YJ_debug)
									print(thisAgent, "\nCue WME<%s, %s,%s, %d>\n", id.c_str(), attr.c_str(), value.c_str(),value_type);
							}
						}

					if(!thisAgent->semantic_memory->test_id(pointed_id)){ // temporary id, first retrieve
						// Retrieved wmes will be placed separately with the cue
						checked_cue = true; // new retrieval will shield expanding, but not vice versa
						// So as long as there is a short term id as cue, there will be a retrieved.
						// If there is no cue, or all cues are long-term id whihc is to be expanded, then
						// there won't be a 'retrieved' slot (old retrieved will be removed since there is no new retrieve)

						set<string> ids = thisAgent->semantic_memory->match_retrieve_single_level(cue_set);// retrieving
						if(YJ_debug) cout << "Matched IDS:" << endl;
						if(YJ_debug) cout << ids << endl;
						if(ids.size() > 0){ // should never return empty set, may need to set a long-term-id for FAILURE chunk
							
							picked_id = *(ids.begin()); // just pick the 1st for the moment, This must be a long term id
														// it could be the failure chunk id
							if(picked_id == previous_retrieved_id){
								picked_id_sym = previous_retrieved_id_sym;
								if(YJ_debug)
									print_with_symbols(thisAgent, "\n<%y> was alreay there\n", previous_retrieved_id_sym);
							}
							else{
								if(YJ_debug)
									print(thisAgent, "%s != %s", picked_id.c_str(), previous_retrieved_id.c_str());
								picked_id_sym = make_identifier_from_str(thisAgent, picked_id);
								wme* retrieved_wme = add_input_wme_with_history(thisAgent, retrieve_link_id, make_sym_constant(thisAgent, "retrieved"), picked_id_sym);
								if(YJ_debug)
									print_with_symbols(thisAgent, "\nAdding Top <%y, %y,%y>\n", retrieved_wme->id, retrieved_wme->attr, retrieved_wme->value);
								
								if(previous_retrieved_id != ""){ //previous retrieved, but not this time
									if(YJ_debug)
										print_with_symbols(thisAgent, "\nRemoving Top <%y, %y, %y>\n", 
											previous_retrieved_wme->id,previous_retrieved_wme->attr, previous_retrieved_wme->value);
									remove_input_wme(thisAgent, previous_retrieved_wme);
								}
							}
							// Need to find a way to make a identifier given its name. Previously may never reuse identifier, now need to reuse
						
						// don't need these removing adding stuffs
							
						//	print_with_symbols(thisAgent, "\nModifying <%y, %y,%y>\n", cue_w->id, cue_w->attr, cue_w->value);
						//	cue_w->value->id.name_letter = picked_id[0];
						//	cue_w->value->id.name_number = StringToInt(picked_id.substr(1));
						//	print_with_symbols(thisAgent, "\nBecomes<%y, %y,%y>\n", cue_w->id, cue_w->attr, cue_w->value);

						//	wme* retrieved_wme  = make_wme(thisAgent, cue_w->id, cue_w->attr, picked_id_sym, FALSE); // acceptable preference
						//	print_with_symbols(thisAgent, "\nRemoving <%y, %y,%y>\n", cue_w->id, cue_w->attr, cue_w->value);
						//	remove_from_dll (cue_s->wmes, cue_w, next, prev);
						//	remove_wme_from_wm(thisAgent, cue_w); // must remove then create new one
							
							
							
							//insert_at_head_of_dll (cue_s->wmes, retrieved_wme, next, prev);
							//add_wme_to_wm(thisAgent, retrieved_wme);

						//	add_input_wme(thisAgent, cue_w->id, cue_w->attr, picked_id_sym);

						}
						else{ // failure of retrieval
							picked_id = "F0";
							if(previous_retrieved_id != ""){ //previous retrieved, but not this time
								if(YJ_debug)
									print_with_symbols(thisAgent, "\nRemoving Top <%y, %y, %y>\n", 
										previous_retrieved_wme->id,previous_retrieved_wme->attr, previous_retrieved_wme->value);
									remove_input_wme(thisAgent, previous_retrieved_wme);
							} // remove older WME ^retrieved, new one would be ^retrieved F0

							picked_id_sym = make_identifier_from_str(thisAgent, picked_id);
							
							wme* retrieved_wme = add_input_wme_with_history(thisAgent, retrieve_link_id, make_sym_constant(thisAgent, "retrieved"), picked_id_sym);
							//if(YJ_debug)
							//	print(thisAgent, "\nAdding <%y ^%y %y> as input_wme\n", retrieve_link_id, make_sym_constant(thisAgent, "retrieved"), picked_id_sym);

							string attr = "status";
							string value = "failure";
							int value_type = 2;
							CueTriplet current_wme = CueTriplet(picked_id, attr, value, value_type);
							current_retrieved_wmes.insert(current_wme); // add this to current_retrieved_wmes, to avoid removing in the final step
							if(previous_retrieved_wmes.find(current_wme) == previous_retrieved_wmes.end()){
								if(YJ_debug)
									print(thisAgent, "\nAdding <%s, %s,%s, %d> as input_wme\n", picked_id.c_str(), attr.c_str(), value.c_str(), value_type);
								add_input_wme_with_history(thisAgent, picked_id_sym, make_sym_constant(thisAgent, "status"), make_sym_constant(thisAgent, "failure"));
							}
							else{
								if(YJ_debug)
									print(thisAgent, "\n<%s, %s,%s, %d> previously retrieved already\n", picked_id.c_str(), attr.c_str(), value.c_str(), value_type);
								}

						}



					}// if is NOT long-term-id, first retrieve to get the long-term-id
					else{// already long-term-id, just expand it
						// In this case, expand the original id

						picked_id = pointed_id;
						picked_id_sym = cue_id; //cue_w->value
						print(thisAgent, "\nRetrieved is same as cue\n");
						// This need to be called for all situations except for when previous retrieved id is the same
						wme* retrieved_wme = add_input_wme_with_history(thisAgent, retrieve_link_id, make_sym_constant(thisAgent, "retrieved"), picked_id_sym);
					}
					
					// retrieve on temporary id is equivalent ot retrieve long-term-id then expand on that id
					
					
					if(picked_id != "F0"){ // failure id, it's not actually in semantic memory, don't expand it
						set<LME> retrieved_LMEs = thisAgent->semantic_memory->expand_id(picked_id, cue_set);
						// One situation is, when retrieved is already in WM, only add attributes that is not already there.
					
					
					
						// Need to add new wmes using the retrieved information, with picked_id_sym being the identifier.
						// If a wme is previously retrieved, then do nothing, otherwise add that wme
						for(set<LME>::iterator itr = retrieved_LMEs.begin(); itr != retrieved_LMEs.end(); ++itr){
							string id = itr->id; // this id must == picked_id
							string attr = itr->attr;
							string value = itr->value;
							int value_type = itr->value_type;
							CueTriplet current_wme = CueTriplet(id, attr, value, value_type);
							current_retrieved_wmes.insert(current_wme); // add this to current_retrieved_wmes, to avoid removing in the final step

							if(previous_retrieved_wmes.find(current_wme) == previous_retrieved_wmes.end()){ //this is new, insert new wme

								Symbol* attr_sym = make_sym_constant(thisAgent, (char*)attr.c_str());
								Symbol* value_sym = NIL; 
								if(value_type == IDENTIFIER_SYMBOL_TYPE){
									value_sym = make_identifier_from_str(thisAgent, value);
								}
								else{
									if(value_type == SYM_CONSTANT_SYMBOL_TYPE){
										value = StringToSym(value);
										value_sym = make_sym_constant(thisAgent, (char*)value.c_str()); 
									}
								else if(value_type == INT_CONSTANT_SYMBOL_TYPE){
										value_sym = make_int_constant(thisAgent, StringToLong(value)); 
									}
									else if(value_type == FLOAT_CONSTANT_SYMBOL_TYPE){
										value_sym = make_float_constant(thisAgent, StringToFloat(value)); 
									}
	
								}
						
								// There might be problems creating new identifer symbol from retrieved string.
								// If it's already in WM, then should use that

							//	wme* new_wme = make_wme(thisAgent, picked_id_sym, attr_sym, value_sym, FALSE);
							//	print_with_symbols(thisAgent, "\nAdding <%y, %y,%y> \n", new_wme->id, new_wme->attr, new_wme->value);
							//	insert_at_head_of_dll (picked_id_sym->id.slots->wmes, new_wme, next, prev);
							//	add_wme_to_wm(thisAgent, new_wme);

								if(YJ_debug)
									print(thisAgent, "\nAdding <%s(%s), %s,%s, %d> as input_wme\n", picked_id.c_str(),id.c_str(), attr.c_str(), value.c_str(), value_type);
								add_input_wme_with_history(thisAgent, picked_id_sym, attr_sym, value_sym);
							}
							else{ // old wme, do nothing
								if(YJ_debug)
									print(thisAgent, "\n<%s(%s), %s,%s, %d> previously retrieved already\n", picked_id.c_str(), id.c_str(), attr.c_str(), value.c_str(), value_type);
							}
						} // for each retrieved wme
					}
					else{
						if(YJ_debug)
							print(thisAgent, "Retrieval Failure\n");
					}
					
				}
			}
		}
	}

		
	// At last, remove not retrieved wmes from previous retrieved id, if it's still the current retrieved id
	if(YJ_debug)
		print(thisAgent, ">>> Clearing old retrieved_id\n");
	if(previous_retrieved_id_sym != NIL){
		for(wme* w=previous_retrieved_id_sym->id.input_wmes; w != NIL; w=w->next){
			string wme_id = symbol_constant_to_string(thisAgent,w->id);
			string wme_attr = symbol_constant_to_string(thisAgent,w->attr);
			string wme_value = symbol_constant_to_string(thisAgent,w->value);
			int wme_value_type = w->value->common.symbol_type;
			if(wme_value_type == SYM_CONSTANT_SYMBOL_TYPE){
					//wme_value = wme_value.substr(1, wme_value.length()-2); // get rid of quting marks
			}
			
			if(current_retrieved_wmes.find(CueTriplet(wme_id, wme_attr, wme_value, wme_value_type)) == current_retrieved_wmes.end()){
				remove_input_wme(thisAgent, w);
				//print_with_symbols(thisAgent, "\nRemoving <%y, %y,%y>\n", w->id, w->attr, w->value);
				if(YJ_debug)
					print(thisAgent, "\nRemoving <%s, %s,%s, %d>\n", wme_id.c_str(), wme_attr.c_str(), wme_value.c_str(), wme_value_type);
			}
			else{
				//print_with_symbols(thisAgent, "\n<%y, %y,%y> retrieved again\n", w->id, w->attr, w->value);
				if(YJ_debug)
					print(thisAgent, "\n<%s, %s,%s, %d> retrieved again\n", wme_id.c_str(), wme_attr.c_str(), wme_value.c_str(), wme_value_type);
			}
		}

	}

}
#endif


// when retrieve LMEs from semantic memory, need to make identifiers from information saved in semantic memory
// id_str is assumed to have the format [A-Z][0-9]+, as it's saved from WMEs
Symbol *make_identifier_from_str (agent* thisAgent, string id_str, goal_stack_level level) {
  Symbol *sym;
  
  level = thisAgent->bottom_goal->id.level;
//  if (isalpha(name_letter)) {
//	  if (islower(name_letter)) name_letter = toupper(name_letter);
//  } else {
//    name_letter = 'I';
//  }
  allocate_with_pool (thisAgent, &thisAgent->identifier_pool, &sym);
  sym->common.symbol_type = IDENTIFIER_SYMBOL_TYPE;
  sym->common.reference_count = 1;
  sym->common.hash_id = thisAgent->current_symbol_hash_id += 137;// get_next_symbol_hash_id(thisAgent); inline fucntion in symtab.cpp
  sym->id.name_letter = id_str[0]; // name_letter;
  sym->id.name_number = StringToInt(id_str.substr(1));
  //sym->id.name_number = thisAgent->id_counter[name_letter-'A']++;
  sym->id.level = level;
  sym->id.promotion_level = level;
  sym->id.slots = NIL;
  sym->id.isa_goal = FALSE;
  sym->id.isa_impasse = FALSE;
  sym->id.isa_operator = 0;
  sym->id.link_count = 0;
  sym->id.unknown_level = NIL;
  sym->id.could_be_a_link_from_below = FALSE;
  sym->id.impasse_wmes = NIL;
  sym->id.higher_goal = NIL;
/* REW: begin 09.15.96 */
  sym->id.gds = NIL;
/* REW: end   09.15.96 */
/* REW: begin 08.20.97 */
  sym->id.saved_firing_type = NO_SAVED_PRODS;
  sym->id.ms_o_assertions = NIL; 
  sym->id.ms_i_assertions = NIL; 
  sym->id.ms_retractions = NIL;  
/* REW: end   08.20.97 */
  sym->id.lower_goal = NIL;
  sym->id.operator_slot = NIL;
  sym->id.preferences_from_goal = NIL;
  sym->id.tc_num = 0;
  sym->id.associated_output_links = NIL;
  sym->id.input_wmes = NIL;
  add_to_hash_table (thisAgent, thisAgent->identifier_hash_table, sym);
  return sym;
}

void smem_save_wme (agent* thisAgent, wme* w){ // collect new wmes for later adding to semantic memory
	if(thisAgent->sysparams[SMEM_SYSPARAM] == 0){ // disabled
		return;
	}
		//return;
		//if(YJ_debug) cout << "ID " << w->id->common.symbol_type << endl;
		string id = symbol_constant_to_string(thisAgent, w->id); // ID must be SYM_CONSTANT_SYMBOL_TYPE
		
		//if(YJ_debug) cout << "ATTR " << w->attr->common.symbol_type << endl;
		string attr = symbol_constant_to_string(thisAgent, w->attr); // attr must be SYM_CONSTANT_SYMBOL_TYPE
		
		//if(YJ_debug) cout << "VALUE " << w->value->common.symbol_type << endl;
		string value = symbol_constant_to_string(thisAgent, w->value);
		int value_type = w->value->common.symbol_type;
			
		if(value_type == IDENTIFIER_SYMBOL_TYPE){
			// should not save any ungrounded identifiers, because they are probably the variables
			// Sometimes the context in really important. If some Wmes are 'imaginated', will they have the same creidts?
			// Seems have to know the 'source' of the wmes ...
			if(w->value->id.slots == NIL){
				print_with_symbols(thisAgent, "\nNot to save (%y ^%y %y)\n", w->id, w->attr, w->value);
				return;
			}

			if(w->value->id.isa_goal){ // don't save state link
				print_with_symbols(thisAgent, "\nNot to save (%y ^%y %y)\n", w->id, w->attr, w->value);
				return;
			}
			// Just never save cue structure in the first place.
			if(strcmp(symbol_constant_to_string(thisAgent, w->attr).c_str(), "cue") == 0){
				print_with_symbols(thisAgent, "\nNot to save cue (%y ^%y %y)\n", w->id, w->attr, w->value);
				thisAgent->prohibited_ids->insert(symbol_constant_to_string(thisAgent, w->value));
				return;
			}
			// to remove wmes with cue being identifier, need to do that before retrieval.
		}
		//thisAgent->semantic_memory->insert_LME(id,attr,value,value_type);
		thisAgent->to_be_saved_wmes->insert(LME(id,attr,value,value_type)); // they need to be examined
}

void save_wmes_12_21(agent* thisAgent) {
        //set<LME> final_lmes;
	vector<LME> final_lmes;
	for(set<LME>::iterator itr = thisAgent->to_be_saved_wmes->begin(); itr != thisAgent->to_be_saved_wmes->end(); ++itr){
		if (thisAgent->prohibited_ids->find(itr->id) == thisAgent->prohibited_ids->end()){
			if(YJ_debug){
				print(thisAgent, "\nSaving (%s ^%s %s)\n", itr->id.c_str(), itr->attr.c_str(), itr->value.c_str());
			}
			//thisAgent->semantic_memory->insert_LME(itr->id,itr->attr,itr->value,itr->value_type);
			// final_lmes.insert(LME(itr->id,itr->attr,itr->value,itr->value_type));
			final_lmes.push_back(LME(itr->id,itr->attr,itr->value,itr->value_type));
		}
		else{
			if(YJ_debug){
				print(thisAgent, "\n(%s ^%s %s) Prohibited\n", itr->id.c_str(), itr->attr.c_str(), itr->value.c_str());
			}
			//thisAgent->to_be_saved_wmes->erase(itr);
		}
	}
	
	thisAgent->semantic_memory->merge_LMEs(final_lmes, thisAgent->d_cycle_count);

	thisAgent->to_be_saved_wmes->clear();
	thisAgent->prohibited_ids->clear();
}





// Just deliberately save one level
void save_wmes_old(agent* thisAgent){

	set<std::string> saved_ids;
	set<LME> saved_wmes;
	find_save_ids(thisAgent, saved_ids); // figure out the new ids to be saved
	//if(YJ_debug) cout << "To be saved ids " << saved_ids << endl;
	find_save_wmes(thisAgent, saved_wmes); // figure out the wmes whose identifier is being pointed by save link.
	//find_save_wmes_all(thisAgent, saved_wmes);

	// Then find out from entire WM that which wmes need to be saved
	// save_wmes + newly added wmes whose  value is either constant or saved_identifiers (in long-term memory or among saved ids for this cycle)
	// newly added, or just consider all WMEs ?
	
	for(set<LME>::iterator itr = saved_wmes.begin(); itr != saved_wmes.end(); ++itr){
		if(YJ_debug) cout << *itr << endl;
		if(long_term_value(thisAgent, saved_ids, itr->id, IDENTIFIER_SYMBOL_TYPE) && 
			long_term_value(thisAgent, saved_ids, itr->value, itr->value_type)){

				thisAgent->semantic_memory->insert_LME(itr->id,itr->attr,itr->value,itr->value_type);
			}
		else{
			if(YJ_debug) cout << "Not long term values" << endl;
		}
	}

}

void save_wmes_deliberate_merge_identical(agent* thisAgent){
	
	//set<LME> final_lmes;
	vector<LME> final_lmes;
	set<std::string> saved_ids;
	set<LME> saved_wmes;
	find_save_ids(thisAgent, saved_ids); // figure out the new ids to be saved
	//if(YJ_debug) cout << "To be saved ids " << saved_ids << endl;
	find_save_wmes(thisAgent, saved_wmes); // figure out the wmes whose identifier is being pointed by save link.
	//find_save_wmes_all(thisAgent, saved_wmes);

	// Then find out from entire WM that which wmes need to be saved
	// save_wmes + newly added wmes whose  value is either constant or saved_identifiers (in long-term memory or among saved ids for this cycle)
	// newly added, or just consider all WMEs ?
	
	for(set<LME>::iterator itr = saved_wmes.begin(); itr != saved_wmes.end(); ++itr){
		if(YJ_debug) cout << *itr << endl;
		if(long_term_value(thisAgent, saved_ids, itr->id, IDENTIFIER_SYMBOL_TYPE) && 
			long_term_value(thisAgent, saved_ids, itr->value, itr->value_type)){

				//thisAgent->semantic_memory->insert_LME(itr->id,itr->attr,itr->value,itr->value_type);
				//final_lmes.insert(LME(itr->id,itr->attr,itr->value,itr->value_type));
				final_lmes.push_back(LME(itr->id,itr->attr,itr->value,itr->value_type));
			}
		else{
			if(YJ_debug) cout << "Not long term values" << endl;
		}
	}
	thisAgent->semantic_memory->merge_LMEs(final_lmes, thisAgent->d_cycle_count);
}


void find_save_wmes(agent* thisAgent, set<LME>& saved_wmes){

	
	//thisAgent->semantic_memory->dump(if(YJ_debug) cout);

	slot *save_s;
	wme *save_w;
	Symbol* smem_link_id = find_smem_link_id(thisAgent);
	Symbol* save_link_id = find_save_link_id(thisAgent);
	
	if(smem_link_id == NIL){
		if(YJ_debug)
			print (thisAgent, "No retrieve link\n");
		return;
	}
	
	if(save_link_id == NIL){
		if(YJ_debug)
			print (thisAgent, "No Save link\n");
		return;
	}
	// Only for the first level
	if(YJ_debug){
		print(thisAgent, "Saving WMEs from smem Link\n");
		print_with_symbols(thisAgent, "smem link id: <%y>\n", smem_link_id);
	}
	if(smem_link_id->id.slots == NIL){
		if(YJ_debug)
			print(thisAgent, "smem link no slots\n");

	}
	for (save_s = save_link_id->id.slots; save_s != NIL; save_s = save_s->next) { // save link id is the to-be-saved id
		for (save_w = save_s->wmes; save_w != NIL; save_w = save_w->next){ // slots should be holding unique values in WM 
			
			if(strcmp(symbol_constant_to_string(thisAgent, save_w->attr).c_str(), "save") == 0
				&&
				save_w->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE){ // if it's constant under save link, no need to save it.
					
				slot *s;
				wme *w;
				for (s = save_w->value->id.slots; s != NIL; s = s->next){
					for (w = s->wmes; w != NIL; w = w->next){

						string id = symbol_constant_to_string(thisAgent, w->id); // ID must be SYM_CONSTANT_SYMBOL_TYPE
						string attr = symbol_constant_to_string(thisAgent, w->attr); // attr must be SYM_CONSTANT_SYMBOL_TYPE
						string value = symbol_constant_to_string(thisAgent, w->value);
						int value_type = w->value->common.symbol_type;
						//if(value_type == SYM_CONSTANT_SYMBOL_TYPE){
						//	value = value.substr(1, value.length()-2); // get rif of quting marks
						//}
						if(YJ_debug)
							print_with_symbols(thisAgent, "\n%y, %y, %y\n", w->id, w->attr, w->value);
						saved_wmes.insert(LME(id, attr, value, value_type));
						
						//thisAgent->semantic_memory->insert_LME(id,attr,value,value_type);
					}
				}
			}
			else{
				if(YJ_debug)
					print_with_symbols(thisAgent, "\nOther smem WME <%y, %y,%y>\n", save_w->id, save_w->attr, save_w->value);
			}
		}
	}
	//thisAgent->semantic_memory->dump(std::if(YJ_debug) cout);
	
}


// Maybe there should be just 1 saved id every cycle, but allow that to be more than 1 at this point for more convenient implementation
void find_save_ids(agent* thisAgent, set<std::string>& id_set){
	slot *s;
	wme *w;
	// Should not do this, should check with memory link, save attribute to find out to-be-saved identifiers.
	// This way it will not work for pointers, but have to copy every attribute under the save link identifier
	// So, no need to create save link wme at all, just leave it to check for the attribute 'save'
	// How about multiple save attribute slots? -- allow it for now...
	Symbol* save_link_id = find_save_link_id(thisAgent);
	//for (s = thisAgent->save_link_id->id.slots; s != NIL; s = s->next) { // save link id is the to-be-saved id
	for (s = save_link_id->id.slots; s != NIL; s = s->next) { // save link id is the to-be-saved id
		for (w = s->wmes; w != NIL; w = w->next){ // slots should be holding unique values in WM 
			if(strcmp(symbol_constant_to_string(thisAgent, w->attr).c_str(), "save") == 0){
				//Symbol* save_id = w->value;
				print_with_symbols(thisAgent, "Saving %y\n", w->value);
			}
			string value = symbol_constant_to_string(thisAgent, w->value);
			if(w->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE){ // save only identifier types, constant types are considered long-term
				id_set.insert(value);
			}
		}
	}
}


vector<pair<string, string> > parse_cluster_input_str(string input_str){
	istringstream isstr(input_str);	
	vector<pair<string, string> > one_instance;
	string attr, value;
	while(isstr >> attr >> value){
		one_instance.push_back(pair<string, string>(attr, value));
	}
	return one_instance;
}

// The input shoul be: state <s> ^smem.cluster.input
// It only check one level, so make sure the structure is flattened appropriately
// The output shoul be: state <s> ^smem.cluster.output
void cluster(agent* thisAgent){
	Symbol* cluster_link_id = find_cluster_link_id(thisAgent);
	vector<pair<string, string> > input_attr_val_pairs;
	vector<pair<string, string> > training_attr_val_pairs;
	wme* cluster_training_link = NIL;
	
	if(YJ_debug){
		print_with_symbols(thisAgent, "cluster link id: <%y>\n", cluster_link_id);
	}

	// 1. Cluster Train Link: Training doesn't need to wait for output, so the training link is automatically cleared after taking each instance
	// 2. Cluster Input Link: While for input, it needs to wait to see the output, and output won't be touched unless cluster_input_link is cleared by rule
	for (slot* cluster_s = cluster_link_id->id.slots; cluster_s != NIL; cluster_s = cluster_s->next) {
		if(strcmp(symbol_constant_to_string(thisAgent, cluster_s->attr).c_str(), "train") == 0){
			
			for(wme* cluster_w = cluster_s->wmes; cluster_w != NIL; cluster_w = cluster_w->next){
				cluster_training_link = cluster_w;
				string id = symbol_constant_to_string(thisAgent, cluster_w->id);
				string attr = symbol_constant_to_string(thisAgent, cluster_w->attr);
				string value = symbol_constant_to_string(thisAgent, cluster_w->value);
				int value_type = cluster_w->value->common.symbol_type;
				// All values are treated as strings
				// Assume sensory inputs are binary detector for symbolic values
				// This is more realistic at neuron level, where firing/non-firing is just binary
				// Not sure about continuous numerical values ...
				
				//training_attr_val_pairs.push_back(pair<string, string>(attr, value));
				
				training_attr_val_pairs = parse_cluster_input_str(value);
				if(YJ_debug) cout << "Cluster Training " << value << endl;
				if(YJ_debug){
					print_with_symbols(thisAgent, "Cluster Training WME\n");
					print_wme(thisAgent, cluster_w);
				}
				
				break;
			}
			
		}		
	}

	
	bool find_input = false;
	for (slot* cluster_s = cluster_link_id->id.slots; cluster_s != NIL; cluster_s = cluster_s->next) {
		if(strcmp(symbol_constant_to_string(thisAgent, cluster_s->attr).c_str(), "input") == 0){
			for(wme* cluster_w = cluster_s->wmes; cluster_w != NIL; cluster_w = cluster_w->next){
				find_input = true;
				string id = symbol_constant_to_string(thisAgent, cluster_w->id);
				string attr = symbol_constant_to_string(thisAgent, cluster_w->attr);
				string value = symbol_constant_to_string(thisAgent, cluster_w->value);
				int value_type = cluster_w->value->common.symbol_type;
				// All values are treated as strings
				// Assume sensory inputs are binary detector for symbolic values
				// This is more realistic at neuron level, where firing/non-firing is just binary
				//input_attr_val_pairs.push_back(pair<string, string>(attr, value));
				input_attr_val_pairs = parse_cluster_input_str(value);
				if(YJ_debug) cout << "Cluster Input " <<  value << endl;
				if(YJ_debug){
					print_with_symbols(thisAgent, "Cluster Input WME\n");
					print_wme(thisAgent, cluster_w);
				}
				break;
			}
			
		}		
	}
	

	if(!find_input){
		thisAgent->cluster_ready = true;
		// clear all output: should just be one real output
		for(wme* cluster_w = cluster_link_id->id.input_wmes; cluster_w != NIL; cluster_w = cluster_w->next){
		if(strcmp(symbol_constant_to_string(thisAgent, cluster_w->attr).c_str(), "output") == 0){
				remove_input_wme(thisAgent, cluster_w);
			}
		}
	}
	// cluster_ready && find_input must take another cycle
	// If cluster_read is ture, which means old input has been cleared in some cycle before current input is placed, 
	// then take the current input and generate new output
	if(thisAgent->cluster_ready && !input_attr_val_pairs.empty()){ // If there is input, performe the clustering
		// generate outpu and set ready state to false
		// Won't be ready unless input is cleared
		thisAgent->cluster_ready = false;
		// First remove previous outputs
		
		vector<int> winners = thisAgent->clusterNet->cluster_input(input_attr_val_pairs, false);
		goal_stack_level goal_level = thisAgent->bottom_goal->id.level;
		Symbol* output_id = make_new_identifier(thisAgent, 'C', goal_level);
		add_input_wme_with_history(thisAgent, cluster_link_id , make_sym_constant(thisAgent,"output"), output_id);
		
		if(winners.size() == 0){ // Actually impossible unless there is not input
			
		}
		for(int i=0; i< winners.size(); ++i){
			
			string attr = string("level")+IntToString((i+1));
			string value = string("C")+IntToString(winners[i]);
			add_input_wme_with_history(thisAgent, output_id , make_sym_constant(thisAgent,(char*)attr.c_str()), make_sym_constant(thisAgent,(char*)value.c_str()));
			print(thisAgent, "%s, %s\n", attr.c_str(), value.c_str());
		}
		print(thisAgent, "Recognizing input\n");
	}
	
	bool trained = false;
	//for(wme* cluster_w = cluster_link_id->id.input_wmes; cluster_w != NIL; cluster_w = cluster_w->next){
	//	if(strcmp(symbol_constant_to_string(thisAgent, cluster_w->attr).c_str(), "train-status") == 0){
	//		trained = true; // Don't look at the value. Should remove this flag to train new instance.
	//	}
	//}
	// Don't generate any flag
	// Rules are responsible to flag cluster input and remove the cluster input link on next cycle.
	// If a input is there for 3 cycles, it'll be trained for 3 times
	if(cluster_training_link != NIL){
		if(!trained && !training_attr_val_pairs.empty()){ // Train it
			// Actually don't care about winners, just training
			vector<int> winners = thisAgent->clusterNet->cluster_input(input_attr_val_pairs, true);
		//	add_input_wme_with_history(thisAgent, cluster_link_id , make_sym_constant(thisAgent,"train-status"), make_sym_constant(thisAgent,"trained"));
			print(thisAgent, "Training\n");
		}
		// How to automatically clear training link, so that it is just trained once ?
		// Input is automatically cleared
		// Only leave output there
		//remove_input_wme(thisAgent, cluster_input_link); // cluster_input_link is not input_wme, it's added by rules
	}
}

// Unused Code
// They are not used any more, just keep the history
void save_wmes(agent* thisAgent){
	
	return; // this now happens in the do_buffered_wm_changes automatically

	set<std::string> saved_ids;
	set<LME> saved_wmes;
	//find_save_ids(thisAgent, saved_ids); // figure out the new ids to be saved
	//if(YJ_debug) cout << "To be saved ids " << saved_ids << endl;
	//find_save_wmes(thisAgent, saved_wmes); // figure out the wmes whose identifier is being pointed by save link.
	//find_save_wmes_all(thisAgent, saved_wmes);

	// Then find out from entire WM that which wmes need to be saved
	// save_wmes + newly added wmes whose  value is either constant or saved_identifiers (in long-term memory or among saved ids for this cycle)
	// newly added, or just consider all WMEs ?
	
	for(set<LME>::iterator itr = saved_wmes.begin(); itr != saved_wmes.end(); ++itr){
		if(YJ_debug) cout << *itr << endl;
		
		thisAgent->semantic_memory->insert_LME(itr->id,itr->attr,itr->value,itr->value_type);
		
	}
}

//

//#endif SEMANTIC_MEMORY
