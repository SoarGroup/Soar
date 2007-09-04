// in trunk\Core\CLI\src\cli_structToString.cpp
bool StringToInt(std::string str, int& n){
	std::istringstream str2int(str);
	if (str2int >> n) {
		return true;
	}
	else{
		return false;
	}
}

bool CommandLineInterface::ParseStructToString(gSKI::Agent* pAgent, std::vector<std::string>& argv) {
	
	if(argv.size() < 2){
		m_Result << "No input identifier";
	}
	string struct_id = argv.at(1);
	string additional_feature = "";
	string excluded_feature = "";
	string equivalent_symbols_id = "";
	
	int current_index = 2;
	while(argv.size() > current_index + 1){
		string option = argv[current_index];
		string value = argv[current_index+1];

		if(option == "-e"){
			excluded_feature = value;
		}
		else if(option == "-a"){
			additional_feature = value;
		}
		else if(option == "-i"){
			equivalent_symbols_id = value;
		}
		current_index += 2;
	}
	

	// Attain the evil back door of doom, even though we aren't the TgD
	gSKI::EvilBackDoor::TgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();
	for(std::vector<string>::iterator itr = argv.begin(); itr != argv.end(); ++itr){
		//cout << *itr << endl;
	}
	if(struct_id.length() >= 3 && struct_id[0] =='|' && struct_id[struct_id.length() - 1] == '|'){
		struct_id = struct_id.substr(1, struct_id.length() - 2);
	}
	if(equivalent_symbols_id.length() >= 3 && equivalent_symbols_id[0] =='|' && equivalent_symbols_id[equivalent_symbols_id.length() - 1] == '|'){
		equivalent_symbols_id = equivalent_symbols_id.substr(1, equivalent_symbols_id.length() - 2);
	}
	int input_int;
	
	if(StringToInt(struct_id, input_int)){
		string gs_str = pKernelHack->IdToStruct(pAgent, input_int);
		m_Result << struct_id << endl;
		m_Result << gs_str;
	}
	else{
		string hash_str = pKernelHack->StructToString(pAgent, struct_id, additional_feature, excluded_feature, equivalent_symbols_id);
		m_Result << hash_str;
	}
	
	return true;
}

//trunk\Core\gSKI\src\gSKI_DoNotTouch.cpp

std::string TgDWorkArounds::IdToStruct(Agent* pIAgent, int hash_id){
			std::string returned_str = "";
			Agent* pAgent2 = (Agent*)(pIAgent);
			agent* thisAgent = pAgent2->GetSoarAgent();

			if(hash_id >= thisAgent->int_to_gs->size()){
				return "";
			}
			return thisAgent->int_to_gs->at(hash_id);
		}

std::string TgDWorkArounds::StructToString(Agent* pIAgent, std::string id, std::string additional_feature, std::string excluded_feature, std::string equivalent_symbols_id){
			std::string returned_str = "";
			Agent* pAgent2 = (Agent*)(pIAgent);
			agent* thisAgent = pAgent2->GetSoarAgent();
			std::map<std::string, std::string> equivalent_symbol_map;
			if(equivalent_symbols_id != ""){
				Symbol* equivalent_symbols_identifier = find_identifier(thisAgent, equivalent_symbols_id.at(0), StringToInt(equivalent_symbols_id.substr(1)) );
				if(equivalent_symbols_identifier != NIL){

					for (slot* s = equivalent_symbols_identifier->id.slots; s != NIL; s = s->next){
						std::string attr = symbol_constant_to_string(thisAgent, s->attr);
						std::vector<std::string> equivalent_symbols;
						for (wme* w= s->wmes; w != NIL; w = w->next){
							std::string val = symbol_constant_to_string(thisAgent, w->value);
							equivalent_symbols.push_back(val);
						}
						
						if(equivalent_symbols.size() >= 2){
							sort(equivalent_symbols.begin(), equivalent_symbols.end());
							std::string mapped_val =  equivalent_symbols.at(0);
							for(int i=1; i<equivalent_symbols.size(); ++i){
								std::string val =  equivalent_symbols.at(i);
								equivalent_symbol_map.insert(std::pair<std::string, std::string>(attr+" "+val, mapped_val));
							}
						}
					}
				}
			}

			Symbol* identifier = find_identifier(thisAgent, id.at(0), StringToInt(id.substr(1)) );
			if (identifier == NIL){
				return returned_str;
			}
			std::vector<std::string> gs_features_array = std::vector<std::string>();
			std::set<std::string> unique_gs_features = std::set<std::string>();

			for (slot* s = identifier->id.slots; s != NIL; s = s->next){
				std::string attr = symbol_constant_to_string(thisAgent, s->attr);
				if(attr == excluded_feature || attr == "action-counter" || attr == "action-sequence"){
						continue;
				}
				for (wme* w= s->wmes; w != NIL; w = w->next){
					std::string attr = symbol_constant_to_string(thisAgent, w->attr);
					std::string val = symbol_constant_to_string(thisAgent, w->value);
					
					// GS features must has two levels, the first level value is of identifier type
					if(w->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE){
						std::string gs_feature = attr + " ";
						
						for(slot* feature_slot = w->value->id.slots; feature_slot != NIL; feature_slot = feature_slot->next){
							for (wme* feature_w= feature_slot->wmes; feature_w != NIL; feature_w = feature_w->next){
								std::string feature_attr = symbol_constant_to_string(thisAgent, feature_w->attr);
								std::string feature_val = symbol_constant_to_string(thisAgent, feature_w->value);
								// GS features must has two levels, the second level value is of constant types
								if(feature_w->value->common.symbol_type == SYM_CONSTANT_SYMBOL_TYPE ||
									feature_w->value->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE ||
									feature_w->value->common.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE){
										std::map<std::string, std::string>::iterator itr = equivalent_symbol_map.find(feature_attr+" "+feature_val);
										if(itr != equivalent_symbol_map.end()){ // map the feature value to the same equivalent value
											feature_val = itr->second;
										}
										gs_feature = gs_feature + "^" + feature_attr + " " + feature_val+" ";
								}
							}
						}
						
						if(unique_gs_features.find(gs_feature) == unique_gs_features.end()){ // avoid duplicated features
							unique_gs_features.insert(gs_feature);
							gs_features_array.push_back(gs_feature);
						}
					}
					
				}
			}

			sort(gs_features_array.begin(), gs_features_array.end()); // get canonical ordering for all unique features
			
			std::string gs_str = additional_feature;
			for(int i=0; i<gs_features_array.size(); ++i){
				gs_str += "(" + gs_features_array[i] + ")";
			}

			unsigned gs_hash_value;
			std::map<std::string, int>::iterator itr = thisAgent->gs_to_int->find(gs_str);
			if(itr != thisAgent->gs_to_int->end()){
				gs_hash_value = itr->second;
			}
			else{
				thisAgent->int_to_gs->push_back(gs_str);
				gs_hash_value = thisAgent->int_to_gs->size() - 1;
				thisAgent->gs_to_int->insert(std::pair<std::string, int>(gs_str, gs_hash_value));
				
			}

			returned_str = IntToString(gs_hash_value);

			return returned_str;
		}
//trunk\Core\gSKI\src\gSKI_DoNotTouch.h
std::string StructToString(Agent* pIAgent, std::string id, std::string additional_feature = "",  std::string excluded_feature = "", std::string identical_symbols_id = "");
std::string IdToStruct(Agent* pIAgent, int hash_id);

// agent.h
std::vector<std::string>* int_to_gs;

//agent.cpp
newAgent->int_to_gs = new std::vector<std::string>;