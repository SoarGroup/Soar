#include "semantic_memory.h"
#include "wmem.h"
#include "decide.h"
#include "print.h"
#include "test.h"

#include <string>
#include <vector>
#include <list>
#include <cstdlib>

// Externs

extern void mark_depths_augs_of_id(agent* thisAgent, Symbol* id, int depth, tc_number tc);

using namespace std;

namespace soar
{
	namespace semantic_memory
	{
		const std::shared_ptr<semantic_memory> semantic_memory::create_singleton(storage* storage)
		{
			if (singleton.get())
				return singleton;
			
			if (storage == nullptr)
			{
				// Handle default case
			}
			
			singleton = std::unique_ptr<semantic_memory>(new semantic_memory(storage));
			return singleton;
		}
		
		const std::shared_ptr<semantic_memory> semantic_memory::get_singleton()
		{
			if (!singleton.get())
				return nullptr;
			
			return singleton;
		}
		
		void semantic_memory::destroy_singleton()
		{ singleton.reset(); }
		
		semantic_memory::semantic_memory(storage* storage_container)
		: backend(storage_container)
		{}
		
		semantic_memory::~semantic_memory()
		{
			delete backend;
		}
		
		bool semantic_memory::set_storage_container(storage* storage_container)
		{
			if (backend)
				delete backend;
			
			backend = storage_container;
			
			return true;
		}
		
		const storage* semantic_memory::get_storage_container()
		{
			return backend;
		}
		
		void semantic_memory::buffered_add_error_message(agent* theAgent, buffered_wme_list* buffered_wme_changes, const Symbol* state, std::string error_message, std::list<wme*> &justification)
		{
			Symbol* error_value = make_str_constant(theAgent, error_message.c_str());
			
			buffered_wme* wme = new buffered_wme;
			wme->w = new soar_module::symbol_triple(state->id->smem_result_header,
													theAgent->smem_sym_failure,
													error_value);
			
			wme->justification = justification;
			buffered_wme_changes->push_back(wme);
			
			symbol_add_ref(theAgent, state->id->smem_result_header);
			symbol_add_ref(theAgent, theAgent->smem_sym_failure);
			symbol_add_ref(theAgent, error_value);
		}
		
		void semantic_memory::buffered_add_success_message(agent* theAgent, buffered_wme_list* buffered_wme_changes, const Symbol* state, std::string success_message, std::list<wme*> &justification)
		{
			Symbol* success_value = make_str_constant(theAgent, success_message.c_str());
			buffered_wme* wme = new buffered_wme;
			wme->w = new soar_module::symbol_triple(state->id->smem_result_header,
													theAgent->smem_sym_success,
													success_value);
			wme->justification = justification;
			buffered_wme_changes->push_back(wme);
			
			symbol_add_ref(theAgent, state->id->smem_result_header);
			symbol_add_ref(theAgent, theAgent->smem_sym_failure);
			symbol_add_ref(theAgent, success_value);
		}
		
		void semantic_memory::buffered_add_success_result(agent* theAgent, buffered_wme_list* buffered_wme_changes, const Symbol* state, Symbol* result, std::list<wme*> &justification)
		{
			buffered_wme* wme = new buffered_wme;
			wme->w = new soar_module::symbol_triple(state->id->smem_result_header,
													theAgent->smem_sym_retrieved,
													result);
			wme->justification = justification;
			buffered_wme_changes->push_back(wme);
			
			symbol_add_ref(theAgent, state->id->smem_result_header);
			symbol_add_ref(theAgent, theAgent->smem_sym_failure);
			symbol_add_ref(theAgent, result);
		}
		
		void semantic_memory::query(agent* theAgent, const Symbol* state, list<wme*>& command_wmes, buffered_wme_list& buffered_wme_changes)
		{
			const Symbol* root_of_query = nullptr, *root_of_neg_query = nullptr;
			std::list<const Symbol*> prohibit;
			std::string result_message = "Unknown Result";
			
			for (const wme* w : command_wmes)
			{
				if (w->attr == theAgent->smem_sym_query)
				{
					if (!root_of_query)
						root_of_query = w->value;
					else
					{
						buffered_add_error_message(theAgent, &buffered_wme_changes, state, "Cannot have duplicate query objects", command_wmes);
						return;
					}
				}
				else if (w->attr == theAgent->smem_sym_negquery)
				{
					if (!root_of_neg_query)
						root_of_neg_query = w->value;
					else
					{
						buffered_add_error_message(theAgent, &buffered_wme_changes, state, "Cannot have duplicate negative query objects", command_wmes);
						return;
					}
				}
				else if (w->attr == theAgent->smem_sym_prohibit)
				{
					if (!w->value->id->isa_lti)
					{
						buffered_add_error_message(theAgent, &buffered_wme_changes, state, "Cannot prohibit non-ltis", command_wmes);
						return;
					}
					
					prohibit.push_back(w->value);
				}
			}
			
			if (!root_of_query && !root_of_neg_query)
				buffered_add_error_message(theAgent, &buffered_wme_changes, state, "Query commands must have a (neg)query attribute", command_wmes);
			else
			{
				Symbol* result = backend->query(theAgent, root_of_query, root_of_neg_query, prohibit, &result_message);
				
				if (!result)
					buffered_add_error_message(theAgent, &buffered_wme_changes, state, result_message, command_wmes);
				else
				{
					buffered_add_success_message(theAgent, &buffered_wme_changes, state, result_message, command_wmes);
					buffered_add_success_result(theAgent, &buffered_wme_changes, state, result, command_wmes);
				}
			}
		}
		
		list<wme*> wmes_of_id(const Symbol* id)
		{
			list<wme*> wmes;
			
			for (slot* s = id->id->slots; s != NIL; s = s->next)
				for (wme* w = s->wmes; w != NIL; w = w->next)
					wmes.push_back(w);
			
			return wmes;
		}
		
		// Front for parse_cue, parse_retrieve, parse_remove
		void semantic_memory::parse_agent_command(agent* theAgent)
		{
			// Start at the bottom state and work our way up
			const Symbol* state = theAgent->bottom_goal;
			
			// Transitive Closure Number
			tc_number tc = get_new_tc_number(theAgent);
			
			buffered_wme_list buffered_wme_changes;
			
			// While we have another state to parse
			while (state != nullptr)
			{
				// Get the command link for the state
				const Symbol* smem_command_link = state->id->smem_cmd_header;
				
				// Get the command wmes
				list<wme*> command_wmes = wmes_of_id(smem_command_link);
				
				// Check if each is a command
				enum {
					BAD = -1,
					NONE = 0,
					QUERY = 1,
					RETRIEVE = 2,
					STORE = 3
				} command = NONE;
				
				string error_message = "Unknown Error";
				int64_t depth = 1;
				
				for (const wme* w : command_wmes)
				{
					if (w->value->symbol_type == INT_CONSTANT_SYMBOL_TYPE &&
						w->attr == theAgent->smem_sym_depth)
					{
						if (w->value->ic->value > 0)
							depth = w->value->ic->value;
						else
						{
							command = BAD;
							error_message = "Depth must be greater than 0, default is 1.";
							break;
						}
					}
					else if (w->value->symbol_type != IDENTIFIER_SYMBOL_TYPE)
					{
						command = BAD;
						error_message = "Non-Depth commands must have an identifier value for their wme.";
						break;
					}
					
					
					if (w->timetag > state->id->smem_info->last_cmd_time[query_slot] &&
						(w->attr == theAgent->smem_sym_query ||
						 w->attr == theAgent->smem_sym_negquery ||
						 w->attr == theAgent->smem_sym_prohibit))
					{
						// Potentially valid command
						if (command != NONE && command != QUERY)
						{
							command = BAD;
							error_message = "Commands must only ever be of one type.  You cannot mix retrieve(s), query(ies), and store(s).";
							break;
						}
						
						// Query
						state->id->smem_info->last_cmd_time[query_slot] = w->timetag;
						
						command = QUERY;
					}
					else if (w->timetag > state->id->smem_info->last_cmd_time[retrieve_slot] &&
							 w->attr == theAgent->smem_sym_retrieve)
					{
						if (command != NONE && command != RETRIEVE)
						{
							command = BAD;
							error_message = "Commands must only ever be of one type.  You cannot mix retrieve(s), query(ies), and store(s).";
							break;
						}
						else if (!w->value->id->isa_lti)
						{
							command = BAD;
							error_message = "Retrieve commands can only ever have an LTI for their value.";
							break;
						}
						
						state->id->smem_info->last_cmd_time[retrieve_slot] = w->timetag;
						
						command = RETRIEVE;
					}
					else if (w->timetag > state->id->smem_info->last_cmd_time[store_slot] &&
							 w->attr == theAgent->smem_sym_store)
					{
						if (command != NONE && command != STORE)
						{
							command = BAD;
							error_message = "Commands must only ever be of one type.  You cannot mix retrieve(s), query(ies), and store(s).";
							break;
						}
						
						state->id->smem_info->last_cmd_time[store_slot] = w->timetag;
						
						command = STORE;
					}
				}
				
				if (command == BAD)
					buffered_add_error_message(theAgent, &buffered_wme_changes, state, error_message, command_wmes);
				else if (command == QUERY)
					query(theAgent, state, command_wmes, buffered_wme_changes);
				else if (command == RETRIEVE)
				{
					string result;
					for (const wme* w : command_wmes)
					{
						bool success = retrieve_lti(theAgent, w->value, &result);
						
						if (!success)
							buffered_add_error_message(theAgent, &buffered_wme_changes, state, result, command_wmes);
						else
						{
							buffered_add_success_message(theAgent, &buffered_wme_changes, state, result, command_wmes);
							buffered_add_success_result(theAgent, &buffered_wme_changes, state, w->value, command_wmes);
						}
					}
				}
				else if (command == STORE)
				{
					string result;
					
					for (const wme* w : command_wmes)
					{
						bool success = backend->store(theAgent, w->value, &result, recursive);
						
						if (!success)
							buffered_add_error_message(theAgent, &buffered_wme_changes, state, result, command_wmes);
						else
						{
							buffered_add_success_message(theAgent, &buffered_wme_changes, state, result, command_wmes);
							buffered_add_success_result(theAgent, &buffered_wme_changes, state, w->value, command_wmes);
						}
					}
				}
			}
			
			if (mirroring)
			{
				string result;
				
				for (Symbol* sym : *theAgent->smem_changed_ids)
				{
					// require that the lti has at least one augmentation
					if (sym->id->slots)
						for (slot* s = sym->id->slots; s != NIL; s = s->next)
							for (wme* w = s->wmes; w != NIL; w = w->next)
								backend->store(theAgent, sym, &result, recursive);
					
					symbol_remove_ref(theAgent, sym);
				}
			}
			
			process_buffered_wmes(theAgent, state, &buffered_wme_changes);
		}
		
		bool semantic_memory::remove_lti(agent* theAgent, const Symbol* lti_to_remove, std::string* result_message, bool force)
		{
			return backend->remove_lti(theAgent, lti_to_remove, force, result_message);
		}
		
		bool semantic_memory::remove_ltis(agent* theAgent, const std::list<const Symbol*> lti_to_remove, std::string* result_message, bool force)
		{
			for (const auto& id : lti_to_remove)
				if (!backend->remove_lti(theAgent, id, force, result_message))
					return false;
			
			return true;
		}
		
		bool semantic_memory::retrieve_lti(agent* theAgent, const Symbol* lti_to_retrieve, std::string* result_message)
		{
			return backend->retrieve_lti(theAgent, lti_to_retrieve, result_message);
		}
		
		void semantic_memory::export_memory_to_graphviz(std::string* graphviz)
		{
			// TODO: add back graphviz support
		}
		
		void semantic_memory::export_lti_to_graphviz(const Symbol* lti, std::string* graphviz)
		{
			// TODO: add back graphviz support
		}
		
		void semantic_memory::print_memory(agent* theAgent, std::string* result_message)
		{
			for (auto it = backend->begin(), end = backend->end(); it != end; ++it)
				print_lti(theAgent, **it, result_message, 0, false);
		}
		
		bool semantic_memory::print_augs_of_lti(agent* theAgent, const Symbol* lti, std::string* result_message, unsigned int depth, unsigned int max_depth, const tc_number tc)
		{
			vector<wme*> list;
			
			/* AGR 652  The plan is to go through the list of WMEs and find out how
			 many there are.  Then we malloc an array of that many pointers.
			 Then we go through the list again and copy all the pointers to that array.
			 Then we qsort the array and print it out.  94.12.13 */
			
			if (lti->id->tc_num == tc)
				return true;    // this has already been printed, so return RPM 4/07 bug 988
			
			if (lti->id->depth > depth)
				return true;    // this can be reached via an equal or shorter path, so return without printing RPM 4/07 bug 988
			
			// if we're here, then we haven't printed this id yet, so print it
			depth = lti->id->depth; // set the depth to the depth via the shallowest path, RPM 4/07 bug 988
			int indent = (max_depth - depth) * 2; // set the indent based on how deep we are, RPM 4/07 bug 988
			
			lti->id->tc_num = tc;  // mark id as printed
			
			/* --- next, construct the array of wme pointers and sort them --- */
			for (slot* s = lti->id->slots; s != NIL; s = s->next)
				for (wme* w = s->wmes; w != NIL; w = w->next)
					list.push_back(w);
			
			sort(list.begin(), list.end(), [](wme* p1, wme* p2) {
				char s1[MAX_LEXEME_LENGTH * 2 + 20], s2[MAX_LEXEME_LENGTH * 2 + 20];
				
				// passing null thisAgent is OK as long as dest is guaranteed != 0
				symbol_to_string(0, p1->attr, true, s1, MAX_LEXEME_LENGTH * 2 + 20);
				symbol_to_string(0, p2->attr, true, s2, MAX_LEXEME_LENGTH * 2 + 20);
				
				return strcmp(s1, s2);
			});
			
			/* --- finally, print the sorted wmes and deallocate the array --- */
			for (wme* w : list)
			{
				print_spaces(theAgent, indent);
				print_wme(theAgent, w);
			}
			
			// If there is still depth left, recurse
			if (depth > 1)
			{
				for (const wme* w : list)
				{
					/* --- call this routine recursively --- */
					const Symbol* value = w->value;
					
					if (value->id->isa_lti)
						if (!retrieve_lti(theAgent, value->id, result_message))
							return false;
					
					if (!print_augs_of_lti(theAgent, value, result_message, depth - 1, max_depth, tc))
						return false;
				}
			}
			
			return true;
		}
		
		void semantic_memory::print_lti(agent* theAgent, const char lti_name, const uint64_t lti_number, std::string* result_message, unsigned int depth, bool history)
		{
			// TODO: put back lti activation history
			
			// Taken from print command but modernized and modified for smem
			
			Symbol* lti = backend->retrieve_lti(theAgent, lti_name, lti_number, result_message);
			print_lti(theAgent, lti, result_message, depth, history);
		}
		
		void semantic_memory::print_lti(agent* theAgent, Symbol* lti, std::string* result_message, unsigned int depth, bool history)
		{
			if (!lti->id->isa_lti)
			{
				*result_message = "Cannot print non-lti.";
				return;
			}
			
			tc_number tc;
			
			// RPM 4/07: first mark the nodes with their shallowest depth
			//           then print them at their shallowest depth
			tc = get_new_tc_number(theAgent);
			mark_depths_augs_of_id(theAgent, lti, depth, tc);
			tc = get_new_tc_number(theAgent);
			print_augs_of_lti(theAgent, lti, result_message, depth, depth, tc);
		}
		
		uint64_t semantic_memory::lti_count()
		{
			return backend->lti_count();
		}
		
		void lti_from_test(test t, std::list<Symbol*>* valid_ltis)
		{
			if (test_is_blank_test(t))
				return;
			
			if (test_is_blank_or_equality_test(t))
			{
				Symbol* referent = referent_of_equality_test(t);
				if (referent->symbol_type == IDENTIFIER_SYMBOL_TYPE && referent->id->isa_lti)
					valid_ltis->push_back(referent);
				
				return;
			}
			
			complex_test* ct = complex_test_from_test(t);
			
			if (ct->type == CONJUNCTIVE_TEST)
				for (cons* c = ct->data.conjunct_list; c != NIL; c = c->rest)
					lti_from_test(static_cast<test>(c->first), valid_ltis);
		}
		
		void lti_from_rhs_value(rhs_value rv, std::list<Symbol*>* valid_ltis)
		{
			if (rhs_value_is_symbol(rv))
			{
				Symbol* sym = rhs_value_to_symbol(rv);
				if (sym->symbol_type == IDENTIFIER_SYMBOL_TYPE && sym->id->isa_lti)
					valid_ltis->push_back(sym);
			}
			else
			{
				::list* fl = rhs_value_to_funcall_list(rv);
				for (cons* c = fl->rest; c != NIL; c = c->rest)
					lti_from_rhs_value(static_cast<rhs_value>(c->first), valid_ltis);
			}
		}
		
		bool semantic_memory::valid_production(condition* lhs, action* rhs)
		{
			std::list<Symbol*> valid_ltis;
			std::list<Symbol*>::iterator lti_p;
			
			// collect valid ltis
			for (condition* c = lhs; c != NIL; c = c->next)
			{
				if (c->type == POSITIVE_CONDITION)
				{
					lti_from_test(c->data.tests.attr_test, &valid_ltis);
					lti_from_test(c->data.tests.value_test, &valid_ltis);
				}
			}
			
			// validate ltis in actions
			// copied primarily from add_all_variables_in_action
			Symbol* id;
			action* a;
			int action_counter = 0;
			
			for (a = rhs; a != NIL; a = a->next)
			{
				a->already_in_tc = false;
				action_counter++;
			}
			
			// good_pass detects infinite loops
			bool good_pass = true;
			bool good_action = true;
			while (good_pass && action_counter)
			{
				good_pass = false;
				
				for (a = rhs; a != NIL; a = a->next)
				{
					if (!a->already_in_tc)
					{
						good_action = false;
						
						if (a->type == MAKE_ACTION)
						{
							id = rhs_value_to_symbol(a->id);
							
							// non-identifiers are ok
							if (!id->is_identifier())
								good_action = true;
							// short-term identifiers are ok
							else if (!id->id->isa_lti)
								good_action = true;
							// valid long-term identifiers are ok
							else if (lti_for_id(id->id->name_letter, id->id->name_number) != nullptr)
								good_action = true;
						}
						else
							good_action = true;
						
						// we've found a new good action
						// mark as good, collect all goodies
						if (good_action)
						{
							a->already_in_tc = true;
							
							// everyone has values
							lti_from_rhs_value(a->value, &valid_ltis);
							
							// function calls don't have attributes
							if (a->type == MAKE_ACTION)
								lti_from_rhs_value(a->attr, &valid_ltis);
							
							// note that we've dealt with another action
							action_counter--;
							good_pass = true;
						}
					}
				}
			};
			
			return (action_counter == 0);
		}
		
		const Symbol* semantic_memory::lti_for_id(char lti_letter, uint64_t lti_number)
		{
			return backend->lti_for_id(lti_letter, lti_number);
		}
		
		void semantic_memory::reset_storage()
		{
			backend->reset();
		}
		
		bool semantic_memory::backup_to_file(std::string& file, std::string* error_message)
		{
			return backend->backup_to_file(file, error_message);
		}
		
		bool semantic_memory::is_storing_recursively()
		{
			return recursive;
		}
		
		void semantic_memory::set_store_recursive(bool recursive)
		{
			this->recursive = recursive;
		}
		
		void semantic_memory::process_buffered_wmes(agent* theAgent, const Symbol* state, buffered_wme_list* buffered_wme_changes)
		{
			if (buffered_wme_changes->empty())
				return;
			
			instantiation* inst = soar_module::make_fake_instantiation(thisAgent, state, &cue_wmes, &my_list);
			
			for (preference* pref = inst->preferences_generated; pref; pref = pref->inst_next)
			{
				// add the preference to temporary memory
				add_preference_to_tm(thisAgent, pref);
				// and add it to the list of preferences to be removed
				// when the goal is removed
				insert_at_head_of_dll(state->id->preferences_from_goal, pref, all_of_goal_next, all_of_goal_prev);
				pref->on_goal_list = true;
				
				if (meta)
				{
					// if this is a meta wme, then it is completely local
					// to the state and thus we will manually remove it
					// (via preference removal) when the time comes
					state->id->smem_info->smem_wmes->push_back(pref);
				}
			}
			
			if (!meta)
			{
				// otherwise, we submit the fake instantiation to backtracing
				// such as to potentially produce justifications that can follow
				// it to future adventures (potentially on new states)
				instantiation* my_justification_list = NIL;
				chunk_instantiation(thisAgent, inst, false, &my_justification_list);
				
				// if any justifications are created, assert their preferences manually
				// (copied mainly from assert_new_preferences with respect to our circumstances)
				if (my_justification_list != NIL)
				{
					preference* just_pref = NIL;
					instantiation* next_justification = NIL;
					
					for (instantiation* my_justification = my_justification_list;
						 my_justification != NIL;
						 my_justification = next_justification)
					{
						next_justification = my_justification->next;
						
						if (my_justification->in_ms)
						{
							insert_at_head_of_dll(my_justification->prod->instantiations, my_justification, next, prev);
						}
						
						for (just_pref = my_justification->preferences_generated; just_pref != NIL; just_pref = just_pref->inst_next)
						{
							add_preference_to_tm(thisAgent, just_pref);
							if (wma_enabled(thisAgent))
							{
								wma_activate_wmes_in_pref(thisAgent, just_pref);
							}
						}
					}
				}
			}
		}
	}
}