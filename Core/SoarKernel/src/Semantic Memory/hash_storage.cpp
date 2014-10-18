//
//  hash_storage.cpp
//  Soar
//
//  Created by Alex Turner on 8/13/14.
//  Copyright (c) 2014 University of Michigan. All rights reserved.
//

#include "hash_storage.h"

#include "wmem.h"
#include "decide.h"
#include "print.h"
#include "test.h"
#include "instantiations.h"
#include "prefmem.h"
#include "parser.h"

#include <list>

using namespace std;

namespace soar
{
	namespace semantic_memory
	{
		template<typename T>
		void unordered_set_intersection(std::list<T> &in1,
										const std::unordered_set<T> &in2)
		{
			auto current = in1.begin();
			const auto& end = in1.end();
			const auto& in2_end = in2.end();
			
			while (current != end)
			{
				if (in2.find(*current) != in2_end)
					++current;
				else
					current = in1.erase(current);
			}
		}
		
		template<typename T>
		void unordered_set_diff_intersection(std::list<T> &in1,
											 const std::unordered_set<T> &in2)
		{
			auto current = in1.begin();
			const auto& end = in1.end();
			const auto& in2_end = in2.end();
			
			while (current != end)
			{
				if (in2.find(*current) != in2_end)
					current = in1.erase(current);
				else
					++current;
			}
		}
		
		template<typename T>
		std::list<T> unordered_set_intersection_const(const std::list<T> &in1,
													  const std::unordered_set<T> &in2)
		{
			list<T> out;
			
			const auto& in2_end = in2.end();
			
			for (auto& id : in1)
			{
				if (in2.find(id) != in2_end)
					out.push_back(id);
			}
			
			return out;
		}
		
		hash_storage::hash_storage()
		{}
		
		hash_storage::~hash_storage()
		{}
		
		Symbol* hash_storage::query(agent* theAgent, const Symbol* root_of_query, const Symbol* root_of_neg_query, std::list<Symbol*>& prohibit, std::string* result_message)
		{
			unordered_set<wme*> cue, neg_cue;
			
			for (slot* s = root_of_query->id->slots;s != nullptr;s = s->next)
				for (wme* w = s->wmes;w != nullptr;w = w->next)
					cue.insert(w);
			
			if (root_of_neg_query)
			{
				for (slot* s = root_of_neg_query->id->slots;s != nullptr;s = s->next)
					for (wme* w = s->wmes;w != nullptr;w = w->next)
						neg_cue.insert(w);
			}
			
			auto it = cue.begin();
			auto cue_end = cue.end();
			
			if (it == cue_end)
				return nullptr;
			
			unordered_map<const wme*, unordered_set<Symbol*>>::iterator jt;
			set<unordered_set<Symbol*>*, less<unordered_set<Symbol*>*>> cue_sets, neg_cue_sets;
			
			for (;it != cue_end;++it)
			{
				if ((*it)->id->is_variable())
				{
					// attr map
					jt = attr_map.find(*it);
					
					if (jt == attr_map.end())
					{
						*result_message = "Could not find any object matching query.";
						return nullptr;
					}
				}
				else
				{
					// value map
					jt = attr_value_map.find(*it);
					
					if (jt == attr_value_map.end())
					{
						*result_message = "Could not find any object matching query.";
						return nullptr;
					}
				}
				
				cue_sets.insert(&jt->second);
			}
			
			it = neg_cue.begin();
			auto neg_cue_end = neg_cue.end();
			
			for (;it != neg_cue_end;++it)
			{
				if ((*it)->id->is_variable())
				{
					// attr map
					jt = attr_map.find(*it);
					
					if (jt == attr_map.end())
					{
						*result_message = "Could not find any object matching query.";
						return nullptr;
					}
				}
				else
				{
					// value map
					jt = attr_value_map.find(*it);
					
					if (jt == attr_value_map.end())
					{
						*result_message = "Could not find any object matching query.";
						return nullptr;
					}
				}
				
				neg_cue_sets.insert(&jt->second);
			}
			
			auto ct = cue_sets.begin();
			
			list<Symbol*> intersection;
			bool sorted = false;
			
			if ((*ct)->size() > 10 * id_cache.size())
			{
				intersection = unordered_set_intersection_const(id_cache,
																**ct);
				sorted = true;
			}
			
			if (!intersection.size())
			{
				intersection = list<Symbol*>((*ct)->begin(), (*ct)->end());
				sorted = false;
			}
			
			ct++;
			
			auto cue_sets_end = cue_sets.end();
			
			while (intersection.size() && ct != cue_sets_end)
			{
				unordered_set_intersection(intersection, **ct);
				ct++;
			}
			
			ct = neg_cue_sets.begin();
			
			auto neg_cue_sets_end = neg_cue_sets.end();
			
			while (intersection.size() && ct != neg_cue_sets_end)
			{
				unordered_set_diff_intersection(intersection, **ct);
				ct++;
			}
			
			unordered_set<Symbol*> prohibit_set(prohibit.begin(), prohibit.end());
			unordered_set_diff_intersection(intersection, prohibit_set);
			
			if (!intersection.size())
			{
				*result_message = "Could not find any object matching query and neg query.";
				return nullptr;
			}
			
			if (sorted)
			{
				*result_message = "Success!";
				return intersection.front();
			}
			
			Symbol* highest = *intersection.begin();
			
			for (const auto& id : intersection)
			{
				if (id->id->activation_info->activation_value > highest->id->activation_info->activation_value)
					highest = id;
			}
			
			*result_message = "Success!";
			return highest;
		}
		
		
		
		bool hash_storage::store(agent* theAgent, const Symbol* id, std::string* result_message, bool recursive)
		{
			id_set.insert(const_cast<Symbol*>(id));
			id_list.push_back(const_cast<Symbol*>(id));
			
			symbol_add_ref(theAgent, const_cast<Symbol*>(id));
			
			for (slot* s = id->id->slots;s != nullptr; s = s->next)
				for (wme* w = s->wmes;w != nullptr; w = w->next)
				{
					cout << "Storing: " << id->id->to_string() << " ^" << w->attr->to_string() << " " << w->value->to_string() << endl;
					
					attr_map[w].insert(const_cast<Symbol*>(id));
					attr_value_map[w].insert(const_cast<Symbol*>(id));
					
					wme_add_ref(w);
					symbol_add_ref(theAgent, w->attr);
					
					if (recursive &&
						w->value->is_identifier()
						&&
						id_set.find(w->value) == id_set.end())
						store(theAgent, w->value, result_message, recursive);
					else if (!w->value->is_identifier() || !recursive)
						symbol_add_ref(theAgent, w->value);
				}
			
			id->id->isa_lti = true;
			
			*result_message = "Success! Added LTI: @";
			*result_message += id->id->name_letter;
			*result_message += std::to_string(id->id->name_number);
			
			return true;
		}
		
		bool hash_storage::remove_lti(agent* theAgent, const Symbol* lti_to_remove, bool force, std::string* result_message)
		{
			if (!lti_to_remove)
			{
				*result_message = "Cannot remove a nullptr LTI";
				return false;
			}
			
			if (!lti_to_remove->id->isa_lti)
			{
				*result_message = "Cannot remove a non-LTI from Semantic Memory!";
				return false;
			}
			
			if (id_set.find(const_cast<Symbol*>(lti_to_remove)) == id_set.end())
			{
				*result_message = "Cannot find LTI in Semantic Memory.  This is probably an error as the LTI isn't in our ID set and its marked as an LTI.";
				return force;
			}
			
			bool foundLTI = false;
			
			for (slot* s = lti_to_remove->id->slots; s != nullptr; s = s->next)
				for (wme* w = s->wmes; w != nullptr; w = w->next)
				{
					auto attr_map_it = attr_map.find(w);
					auto attr_value_map_it = attr_value_map.find(w);
					
					if (attr_map_it != attr_map.end())
					{
						// Found the WME
						// Check if our LTI is in the unordered_set
						auto it = attr_map_it->second.find(const_cast<Symbol*>(lti_to_remove));
						
						if (it != attr_map_it->second.end())
						{
							attr_map_it->second.erase(it);
							foundLTI = true;
						}
					}
					
					if (attr_value_map_it != attr_map.end())
					{
						// Found the WME
						// Check if our LTI is in the unordered_set
						auto it = attr_value_map_it->second.find(const_cast<Symbol*>(lti_to_remove));
						
						if (it != attr_value_map_it->second.end())
						{
							attr_value_map_it->second.erase(it);
							foundLTI = true;
						}
					}
				}
			
			if (!foundLTI)
				*result_message = "Could not find LTI.";
			
			return foundLTI || force;
		}
		
		bool hash_storage::remove_lti(agent* theAgent, const char lti_letter, const uint64_t lti_number, bool force, std::string* result_message)
		{
			Symbol* id = find_identifier(theAgent, lti_letter, lti_number);
			
			if (id != nullptr)
				return remove_lti(theAgent, id, force, result_message);
			
			*result_message = "Cannot find LTI: @";
			*result_message += lti_letter;
			*result_message += std::to_string(lti_number);
			
			return false;
		}
		
		Symbol* hash_storage::retrieve_lti(agent* theAgent, const Symbol* lti_to_retrieve, std::string* result_message)
		{
			if (!lti_to_retrieve)
			{
				*result_message = "Cannot retrieve a nullptr LTI";
				return nullptr;
			}
			
			if (lti_to_retrieve->id->isa_lti)
				return const_cast<Symbol*>(lti_to_retrieve);
			
			*result_message = "Cannot find LTI";
			
			return nullptr;
		}
		
		Symbol* hash_storage::retrieve_lti(agent* theAgent, const char lti_letter, const uint64_t lti_number, std::string* result_message)
		{
			Symbol* id = find_identifier(theAgent, lti_letter, lti_number);
			
			if (id != nullptr)
				return retrieve_lti(theAgent, id, result_message);
			
			*result_message = "Cannot find LTI: @";
			*result_message += lti_letter;
			*result_message += std::to_string(lti_number);
			
			return nullptr;
		}
		
		bool hash_storage::retrieve_all_ltis(agent* theAgent, std::list<const Symbol*>* ltis)
		{
			list<const Symbol*> lti_list(id_set.begin(), id_set.end());
			
			*ltis = lti_list;
			
			return true;
		}
		
		uint64_t hash_storage::lti_count()
		{
			return id_set.size();
		}
		
		const Symbol* hash_storage::lti_for_id(char lti_letter, uint64_t lti_number)
		{
			idSymbol lti;
			
			lti.name_letter = lti_letter;
			lti.name_number = lti_number;
			
			lti.id = &lti;
			
			auto it = id_set.find((Symbol*)&lti);
			if (it != id_set.end())
				return *it;
			
			return nullptr;
		}
		
		void hash_storage::reset(agent* theAgent)
		{
			for (auto pair_set : attr_map)
				wme_remove_ref(theAgent, const_cast<wme*>(pair_set.first));
			
			attr_map.clear();
			
			for (auto pair_set : attr_value_map)
				wme_remove_ref(theAgent, const_cast<wme*>(pair_set.first));
			
			attr_value_map.clear();
			id_set.clear();
		}
		
		bool hash_storage::backup_to_file(std::string& file, std::string* error_message)
		{
			*error_message = "Not supported by hash database";
			
			return false;
		}
		
		std::list<Symbol*>::iterator hash_storage::begin()
		{
			return id_list.begin();
		}
		
		std::list<Symbol*>::iterator hash_storage::end()
		{
			return id_list.end();
		}
		
		std::map<std::string, storage_preference> hash_storage::get_preferences()
		{
			return map<string, storage_preference>();
		}
	};
};
