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

        class storage_iterator : public std::iterator<std::input_iterator_tag, const Symbol*>
        {
        public:
            virtual ~storage_iterator() {};
            
            virtual storage_iterator& operator++() = 0;
            virtual bool operator==(const storage_iterator& rhs) = 0;
            virtual bool operator!=(const storage_iterator& rhs) = 0;
        };
        
		class storage
		{
		public:
			virtual ~storage() {}

			virtual bool query(agent* theAgent, const Symbol* root_of_cue, const Symbol* result, std::string** result_message) = 0;
            virtual bool store(agent* theAgent, const Symbol* root_of_cue, std::string** result_message) = 0;

			virtual bool remove_lti(agent* theAgent, const Symbol* lti_to_remove, bool force, std::string** result_message) = 0;
			virtual bool remove_lti(agent* theAgent, const char lti_letter, const uint64_t lti_number, bool force, std::string** result_message) = 0;

			virtual const Symbol* retrieve_lti(agent* theAgent, const Symbol* lti_to_retrieve, std::string** result_message) = 0;
			virtual const Symbol* retrieve_lti(agent* theAgent, const char lti_letter, const uint64_t lti_number, std::string** result_message) = 0;

			virtual bool retrieve_all_ltis(agent* theAgent, std::list<const Symbol*>* ltis) = 0;

			virtual uint64_t lti_count() = 0;

			virtual bool valid_production(condition* lhs, action* rhs) = 0;
			virtual const Symbol* lti_for_id(char lti_letter, uint64_t lti_number) = 0;

			virtual void reset() = 0;
			virtual bool backup_to_file(std::string& file, std::string** error_message) = 0;
            
            virtual storage_iterator begin();
            virtual const storage_iterator& end();
		};
	}
}

#endif
