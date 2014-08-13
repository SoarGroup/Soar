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

// Specializations

namespace std
{
	template<> struct greater<Symbol*> {
		bool operator() ( const Symbol* k1, const Symbol* k2 ) const;
	};
	
	template<> struct equal_to<Symbol*> {
		bool operator() ( const Symbol* k1, const Symbol* k2 ) const;
	};
	
	template<> struct hash<Symbol*>
	{
		inline size_t operator() (const Symbol* id) const;
	};
};

// Hash Storage

namespace soar
{
	namespace semantic_memory
	{
		inline bool operator== (wme const& lhs, wme const& rhs);

		class hash_storage_iterator : public storage_iterator
		{
		public:
			virtual Symbol* operator*();
			
			virtual storage_iterator& operator++();
		};
		
		class hash_storage : public storage
		{
			// Hash functions
			inline size_t hash_combine(size_t& seed, size_t& other);
			inline size_t hash_sym(const Symbol* sym);
			
			struct hash_wme_wildcard : public std::hash<const wme*>
			{
				virtual inline size_t operator() (const wme* w) const;
			};
			
			struct hash_wme : public std::hash<const wme*>
			{
				virtual inline size_t operator() (const wme* w) const;
			};
			
			struct equal_to_attr_value : public std::equal_to<const wme*>
			{
				bool operator() ( wme const *lhs, wme const *rhs ) const;
			};
			
			struct equal_to_attr : public std::equal_to<const wme*>
			{
				bool operator() ( wme const *lhs, wme const *rhs ) const;
			};
			
			// Definitions

			std::unordered_map<const wme*, std::unordered_set<const Symbol*>, hash_wme_wildcard, equal_to_attr> attr_map;
			std::unordered_map<const wme*, std::unordered_set<const Symbol*>, hash_wme, equal_to_attr_value> attr_value_map;

		public:
			hash_storage();
			~hash_storage();
			
			virtual Symbol* query(agent* theAgent, const Symbol* root_of_query, const Symbol* root_of_neg_query, std::list<const Symbol*>& prohibit, std::string* result_message);
			virtual bool store(agent* theAgent, const Symbol* id, std::string* result_message, bool recursive);
			
			virtual bool remove_lti(agent* theAgent, const Symbol* lti_to_remove, bool force, std::string* result_message);
			virtual bool remove_lti(agent* theAgent, const char lti_letter, const uint64_t lti_number, bool force, std::string* result_message);
			
			virtual Symbol* retrieve_lti(agent* theAgent, const Symbol* lti_to_retrieve, std::string* result_message);
			virtual Symbol* retrieve_lti(agent* theAgent, const char lti_letter, const uint64_t lti_number, std::string* result_message);
			
			virtual bool retrieve_all_ltis(agent* theAgent, std::list<const Symbol*>* ltis);
			
			virtual uint64_t lti_count();
			
			virtual bool valid_production(condition* lhs, action* rhs);
			virtual const Symbol* lti_for_id(char lti_letter, uint64_t lti_number);
			
			virtual void reset();
			virtual bool backup_to_file(std::string& file, std::string* error_message);
			
			virtual storage_iterator* begin();
			virtual storage_iterator* end();
			
			virtual std::map<std::string, storage_preference> get_preferences();
		};
	}
}

#endif /* defined(__Soar__hash_storage__) */
