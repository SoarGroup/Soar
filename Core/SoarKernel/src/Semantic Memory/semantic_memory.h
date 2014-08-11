/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

////////////////////////////////////////////////////////////////////////////////
//
// Semantic Memory
//
////////////////////////////////////////////////////////////////////////////////

#ifndef SEMANTIC_MEMORY_H
#define SEMANTIC_MEMORY_H


////////////////////////////////////////////////////////////////////////////////
//
// System Headers
//
////////////////////////////////////////////////////////////////////////////////

#include <string>
#include <list>
#include <memory>

////////////////////////////////////////////////////////////////////////////////
//
// Soar Headers
//
////////////////////////////////////////////////////////////////////////////////

#include "portability.h"

#include "production.h"
#include "rhs.h"
#include "agent.h"
#include "symtab.h"

////////////////////////////////////////////////////////////////////////////////
//
// Semantic Memory Headers
//
////////////////////////////////////////////////////////////////////////////////

#include "storage.h"

namespace soar
{
	namespace semantic_memory
	{
////////////////////////////////////////////////////////////////////////////////
//
//		typedefs for readability
//
////////////////////////////////////////////////////////////////////////////////
#ifdef USE_MEM_POOL_ALLOCATORS
        typedef std::set< Symbol*, std::less< Symbol* >, soar_module::soar_memory_pool_allocator< Symbol* > > pooled_symbol_set;
#else
        typedef std::set< Symbol* > smem_pooled_symbol_set;
#endif

		typedef struct ::condition_struct condition;
		typedef struct ::action_struct action;
		typedef struct ::agent_struct agent;

////////////////////////////////////////////////////////////////////////////////
//
//		Class Declarations
//
////////////////////////////////////////////////////////////////////////////////

		class semantic_memory
		{
            semantic_memory(storage* storage_container);
            ~semantic_memory();
            
            static std::shared_ptr<semantic_memory> singleton;
            
		public:
			////////////////////////////////////////////////////////////////////////////////
			//
			//		Public Declarations
			//
			////////////////////////////////////////////////////////////////////////////////
            static const std::shared_ptr<semantic_memory> create_singleton(storage* storage);
            static const std::shared_ptr<semantic_memory> get_singleton();
            
            void reset(agent* theAgent, Symbol* state);

			bool set_storage_container(storage* storage_container);
			const storage* get_storage_container();

			// Front for parse_cue, parse_retrieve, parse_remove
			bool parse_agent_command(agent* theAgent, std::string** error_message);

			bool parse_cue(agent* theAgent, const Symbol* root_of_cue, std::string** result_message);

			bool remove_lti(agent* theAgent, const Symbol* lti_to_remove, std::string** result_message, bool force = false);
			bool remove_ltis(agent* theAgent, const std::list<const Symbol*> lti_to_remove, std::string** result_message, bool force = false);

			bool retrieve_lti(agent* theAgent, const Symbol* lti_to_retrieve, std::string** result_message);

			void export_memory_to_graphviz(std::string** graphviz);
			void export_lti_to_graphviz(const Symbol* lti, std::string** graphviz);

			void print_memory(agent* theAgent, std::string** result_message);
			void print_lti(agent* theAgent, const char lti_name, const uint64_t lti_number, std::string** result_message, unsigned int depth = 0, bool history = false);
            void print_lti(agent* theAgent, const Symbol* lti, std::string** result_message, unsigned int depth = 0, bool history = false);

			uint64_t lti_count();

			bool valid_production(condition* lhs, action* rhs);
			const Symbol* lti_for_id(char lti_letter, uint64_t lti_number);

			void reset_storage();

			bool backup_to_file(std::string& file, std::string** error_message);
            
            ////////////////////////////////////////////////////////////////////////////////
            //
            //		Preference Variable Functions
            //
            ////////////////////////////////////////////////////////////////////////////////
            bool is_mirroring();
            void set_mirror(bool mirror);
            
		private:
			////////////////////////////////////////////////////////////////////////////////
			//
			//		Private Variables
			//
			////////////////////////////////////////////////////////////////////////////////

			storage* backend;
            bool mirroring;

			////////////////////////////////////////////////////////////////////////////////
			//
			//		Private Declarations
			//
			////////////////////////////////////////////////////////////////////////////////
			void lti_from_test(test t, std::list<Symbol*>* valid_ltis);
			void lti_from_rhs_value(rhs_value rv, std::list<Symbol*>* valid_ltis);
            
            // Print + helpers
            void print_augs_of_lti(agent* theAgent, const Symbol* lti, std::string** result_message, unsigned int depth, unsigned int max_depth, const tc_number tc);
		};
	}
}

#endif
