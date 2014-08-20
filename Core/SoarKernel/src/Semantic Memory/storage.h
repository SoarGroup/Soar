//
//  storage.h
//  Soar
//
//  Created by Alex Turner on 8/5/14.
//  Copyright (c) 2014 University of Michigan. All rights reserved.
//

#ifndef Storage_H
#define Storage_H

////////////////////////////////////////////////////////////////////////////////
//
// System Headers
//
////////////////////////////////////////////////////////////////////////////////

#include <list>
#include <map>
#include <string>
#include <iostream>

////////////////////////////////////////////////////////////////////////////////
//
// Semantic Memory
//
////////////////////////////////////////////////////////////////////////////////

namespace soar
{
	namespace semantic_memory
	{
////////////////////////////////////////////////////////////////////////////////
//
//		Class Definitions
//
////////////////////////////////////////////////////////////////////////////////
		
		class storage_preference
		{
			// Friend classes
			// All storage containers must be listed here
		public:
			enum class storage_preference_type
			{
				string,
				integer,
				floating_point,
				boolean
			};
			
		private:
			std::string string_value;
			uint64_t int_value;
			long double float_value;
			bool bool_value;
			
			storage_preference_type type;
		public:
			virtual ~storage_preference() {}
			
			const storage_preference_type get_type() { return type; }
			
			std::string string() { return string_value; }
			uint64_t integer() { return int_value; }
			long double floating_point() { return float_value; }
			bool boolean() { return bool_value; }
		};
        
		class storage
		{
		public:
			virtual ~storage() {}

            virtual Symbol* query(agent* theAgent, const Symbol* root_of_query, const Symbol* root_of_neg_query, std::list<Symbol*>& prohibit, std::string* result_message) = 0;
            virtual bool store(agent* theAgent, const Symbol* id, std::string* result_message, bool recursive) = 0;

			virtual bool remove_lti(agent* theAgent, const Symbol* lti_to_remove, bool force, std::string* result_message) = 0;
			virtual bool remove_lti(agent* theAgent, const char lti_letter, const uint64_t lti_number, bool force, std::string* result_message) = 0;

			virtual Symbol* retrieve_lti(agent* theAgent, const Symbol* lti_to_retrieve, std::string* result_message) = 0;
			virtual Symbol* retrieve_lti(agent* theAgent, const char lti_letter, const uint64_t lti_number, std::string* result_message) = 0;

			virtual bool retrieve_all_ltis(agent* theAgent, std::list<const Symbol*>* ltis) = 0;

			virtual uint64_t lti_count() = 0;

			virtual const Symbol* lti_for_id(char lti_letter, uint64_t lti_number) = 0;

			virtual void reset(agent* theAgent) = 0;
			virtual bool backup_to_file(std::string& file, std::string* error_message) = 0;
            
			virtual std::list<Symbol*>::iterator begin() = 0;
            virtual std::list<Symbol*>::iterator end() = 0;
			
			virtual std::map<std::string, storage_preference> get_preferences() = 0;
		};
	}
}

#endif
