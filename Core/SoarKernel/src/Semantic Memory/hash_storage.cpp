//
//  hash_storage.cpp
//  Soar
//
//  Created by Alex Turner on 8/13/14.
//  Copyright (c) 2014 University of Michigan. All rights reserved.
//

#include "hash_storage.h"

namespace soar
{
	namespace semantic_memory
	{
		Symbol* hash_storage_iterator::operator*()
		{
			
		}
		
		storage_iterator& hash_storage_iterator::operator++()
		{
			
		}
		
		hash_storage::hash_storage()
		{
			
		}
		hash_storage::~hash_storage()
		{
			
		}
		
		Symbol* hash_storage::query(agent* theAgent, const Symbol* root_of_query, const Symbol* root_of_neg_query, std::list<const Symbol*>& prohibit, std::string* result_message)
		{
			
		}
		bool hash_storage::store(agent* theAgent, const Symbol* id, std::string* result_message, bool recursive)
		{
			
		}
		
		bool hash_storage::remove_lti(agent* theAgent, const Symbol* lti_to_remove, bool force, std::string* result_message)
		{
			
		}
		bool hash_storage::remove_lti(agent* theAgent, const char lti_letter, const uint64_t lti_number, bool force, std::string* result_message)
		{
			
		}
		
		Symbol* hash_storage::retrieve_lti(agent* theAgent, const Symbol* lti_to_retrieve, std::string* result_message)
		{
			
		}
		Symbol* hash_storage::retrieve_lti(agent* theAgent, const char lti_letter, const uint64_t lti_number, std::string* result_message)
		{
			
		}
		
		bool hash_storage::retrieve_all_ltis(agent* theAgent, std::list<const Symbol*>* ltis)
		{
			
		}
		
		uint64_t hash_storage::lti_count()
		{
			
		}
		
		bool hash_storage::valid_production(condition* lhs, action* rhs)
		{
			
		}
		const Symbol* hash_storage::lti_for_id(char lti_letter, uint64_t lti_number)
		{
			
		}
		
		void hash_storage::reset()
		{
			
		}
		bool hash_storage::backup_to_file(std::string& file, std::string* error_message)
		{
			
		}
		
		storage_iterator* hash_storage::begin()
		{
			
		}
		storage_iterator* hash_storage::end()
		{
			
		}
		
		std::map<std::string, storage_preference> hash_storage::get_preferences()
		{
			
		}
	};
};
