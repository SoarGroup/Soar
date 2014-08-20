//
//  hash_storage.h
//  Soar
//
//  Created by Alex Turner on 8/13/14.
//  Copyright (c) 2014 University of Michigan. All rights reserved.
//

#ifndef __Soar__hash_storage__
#define __Soar__hash_storage__

#include "semantic_memory.h"

#include <unordered_map>
#include <unordered_set>

// Hash Storage

namespace soar
{
	namespace semantic_memory
	{
		inline bool operator== (wme const& lhs, wme const& rhs);
				
		class hash_storage : public storage
		{
			// Hash functions
			struct hash_wme_wildcard : public std::hash<const wme*>
			{
				virtual inline size_t operator() (const wme* w) const
				{
					return w->attr->hash_id;
				}
			};
			
			struct hash_wme : public std::hash<const wme*>
			{
				virtual inline size_t operator() (const wme* w) const
				{
					return hash_combine(w->attr->hash_id, w->value->hash_id);
				}
			};
			
			struct equal_to_attr_value : public std::equal_to<const wme*>
			{
				bool operator() ( wme const *lhs, wme const *rhs ) const
				{
					return	lhs->attr == rhs->attr &&
							lhs->value == rhs->value;
				}
			};
			
			struct equal_to_attr : public std::equal_to<const wme*>
			{
				bool operator() ( wme const *lhs, wme const *rhs ) const
				{
					return lhs->attr == rhs->attr;
				}
			};
			
			// Definitions

			std::unordered_map<const wme*, std::unordered_set<Symbol*>, hash_wme_wildcard, equal_to_attr> attr_map;
			std::unordered_map<const wme*, std::unordered_set<Symbol*>, hash_wme, equal_to_attr_value> attr_value_map;
			std::unordered_set<Symbol*> id_set;
			std::list<Symbol*> id_list;
			
			std::list<Symbol*> id_cache;
			
		public:
			hash_storage();
			~hash_storage();
			
			virtual Symbol* query(agent* theAgent, const Symbol* root_of_query, const Symbol* root_of_neg_query, std::list<Symbol*>& prohibit, std::string* result_message);
			virtual bool store(agent* theAgent, const Symbol* id, std::string* result_message, bool recursive);
			
			virtual bool remove_lti(agent* theAgent, const Symbol* lti_to_remove, bool force, std::string* result_message);
			virtual bool remove_lti(agent* theAgent, const char lti_letter, const uint64_t lti_number, bool force, std::string* result_message);
			
			virtual Symbol* retrieve_lti(agent* theAgent, const Symbol* lti_to_retrieve, std::string* result_message);
			virtual Symbol* retrieve_lti(agent* theAgent, const char lti_letter, const uint64_t lti_number, std::string* result_message);
			
			virtual bool retrieve_all_ltis(agent* theAgent, std::list<const Symbol*>* ltis);
			
			virtual uint64_t lti_count();
			
			virtual const Symbol* lti_for_id(char lti_letter, uint64_t lti_number);
			
			virtual void reset(agent* theAgent);
			virtual bool backup_to_file(std::string& file, std::string* error_message);
			
			virtual std::list<Symbol*>::iterator begin();
			virtual std::list<Symbol*>::iterator end();
			
			virtual std::map<std::string, storage_preference> get_preferences();
		};
	}
}

#endif /* defined(__Soar__hash_storage__) */
