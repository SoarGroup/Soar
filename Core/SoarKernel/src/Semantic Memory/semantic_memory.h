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
#include <set>

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
		struct activation_data
		{
			double activation_value; // This is the piece of metadata to sort by. It varies with each "time step".
			
			//The below only make sense with respect to thisAgent->smem_max_cycle
			std::list<uint64_t> activation_time_history; // These should only ever have 10 elements in them. 1 = newest, 10 = oldest.
			std::list<uint64_t> activation_touches_history; // These should only ever have 10 elements in them.
			// The above two lists could also be allowed to have less than 10 elements until populated with sufficient activation.
			// That or all zeros. Design choice. I'm picking all zeros. - scijones
			
			uint64_t total_activation_num; // How many activations have occurred.
			uint64_t first_activation_time; // When the first activation occurred.
			//These two are for the petrov 2006 approximation.
			
			//uint64_t last_activation_time; // When the most recent activation occurred. It's just handy.
			activation_data()
			{
				activation_value = 0;
				for(int i = 0; i < 10; i++)
				{
					activation_time_history.push_front(0);
					activation_touches_history.push_front(0);
				}
				activation_time_history.push_front(0);
				activation_touches_history.push_front(0);
				uint64_t total_activation_num =0;
			}
		};
		
		inline size_t hash_combine(const size_t& seed, const size_t& other)
		{
			return seed ^ (other + 0x9e3779b9 + (seed<<6) + (seed>>2));
		}
	}
}

// Specializations

namespace std
{
	template<> struct greater<Symbol*> {
		bool operator()(const Symbol* k1, const Symbol* k2) const
		{
			return k1->id->activation_info->activation_value > k2->id->activation_info->activation_value;
		}
	};
	
	template<> struct equal_to<Symbol*> {
		bool operator() ( const Symbol* k1, const Symbol* k2 ) const
		{
			return k1 == k2;
		}
	};
	
	template<> struct hash<Symbol*>
	{
		size_t operator() (const Symbol* idSymbol) const
		{
			return idSymbol->hash_id;
		}
	};
}

namespace soar
{
	namespace semantic_memory
	{
////////////////////////////////////////////////////////////////////////////////
//
//		typedefs for readability
//
////////////////////////////////////////////////////////////////////////////////
        typedef std::set<Symbol*> pooled_symbol_set;
		typedef struct ::condition_struct condition;
		typedef struct ::action_struct action;
		typedef struct ::agent_struct agent;

////////////////////////////////////////////////////////////////////////////////
//
//		Class Declarations
//
////////////////////////////////////////////////////////////////////////////////
        static int query_slot, retrieve_slot, store_slot;
        
        struct state_data
        {
            uint64_t last_cmd_time[3];
            uint64_t last_cmd_count[3];
			
			uint64_t time_id;
            
            std::list<preference*> wmes;
        };

		class semantic_memory
		{
            static std::shared_ptr<semantic_memory> singleton;
			
			friend class CommandLineInterface;
		public:
			semantic_memory(storage* storage_container);
			~semantic_memory();

			////////////////////////////////////////////////////////////////////////////////
			//
			//		Public Declarations
			//
			////////////////////////////////////////////////////////////////////////////////
			
			// Statics
			static const std::shared_ptr<semantic_memory> create_singleton(storage* storage);
            static void destroy_singleton();
            static const std::shared_ptr<semantic_memory> get_singleton();
			
			// Class declaration
			typedef std::list<soar_module::symbol_triple*> buffered_wme_list;
			
			bool set_storage_container(storage* storage_container);
			const storage* get_storage_container();

			// Front for parse_cue, parse_retrieve, parse_remove
            void parse_agent_command(agent* theAgent);

			bool remove_lti(agent* theAgent, const Symbol* lti_to_remove, std::string* result_message, bool force = false);
			bool remove_ltis(agent* theAgent, const std::list<const Symbol*> lti_to_remove, std::string* result_message, bool force = false);

			bool retrieve_lti(agent* theAgent, const Symbol* lti_to_retrieve, std::string* result_message);
			Symbol* retrieve_lti(agent* theAgent, const char lti_name, const uint64_t lti_number, std::string* result_message);

			void export_memory_to_graphviz(std::string* graphviz);
			void export_lti_to_graphviz(const Symbol* lti, std::string* graphviz);

			void print_memory(agent* theAgent, std::string* result_message);
			void print_lti(agent* theAgent, const char lti_name, const uint64_t lti_number, std::string* result_message, unsigned int depth = 0, bool history = false);
            void print_lti(agent* theAgent, Symbol* lti, std::string* result_message, unsigned int depth = 0, bool history = false);

			uint64_t lti_count();

			bool valid_production(condition* lhs, action* rhs);
			const Symbol* lti_for_id(char lti_letter, uint64_t lti_number);
			
			bool parse_add_command(agent* theAgent, std::string add_commands, std::string* error_message);

			void reset_storage(agent* theAgent);

			bool backup_to_file(std::string& file, std::string* error_message);
            
            ////////////////////////////////////////////////////////////////////////////////
            //
            //		Preference Variable Functions
            //
            ////////////////////////////////////////////////////////////////////////////////
            bool is_mirroring();
            void set_mirror(bool mirror);
			
			bool is_storing_recursively();
			void set_store_recursive(bool recursive);
            
		private:
			////////////////////////////////////////////////////////////////////////////////
			//
			//		Private Variables
			//
			////////////////////////////////////////////////////////////////////////////////

			storage* backend;
            bool mirroring;
			bool recursive;
			uint64_t smem_cycle_age; //previously thisAgent->smem_max_cycle
			

			////////////////////////////////////////////////////////////////////////////////
			//
			//		Private Declarations
			//
			////////////////////////////////////////////////////////////////////////////////
			
			// Helper Functions
			void lti_from_test(test t, std::list<Symbol*>* valid_ltis);
			void lti_from_rhs_value(rhs_value rv, std::list<Symbol*>* valid_ltis);
            
			void add_activation_history(activation_data*);// When a query or a retrieve occurs, add a touch to the LTI.
            void query(agent* theAgent, const Symbol* state, std::list<wme*>& command_wmes, buffered_wme_list& buffered_wme_changes);
			
			void add_preferences_to_query_result(agent* theAgent, Symbol* root);
			
			// Error/Success Handling
			void buffered_add_error_message(agent* theAgent, buffered_wme_list* buffered_wme_changes, const Symbol* state, std::string error_message);

			void buffered_add_error(agent* theAgent, buffered_wme_list* buffered_wme_changes, const Symbol* state, Symbol* command_type, Symbol* command, std::string error_message);
			void buffered_add_success(agent* theAgent, buffered_wme_list* buffered_wme_changes, const Symbol* state, Symbol* command_type, Symbol* command, std::string success_message, Symbol* success_result = nullptr);
			
			// Buffered WME Processing
			void process_buffered_wmes(agent* theAgent, Symbol* state, std::set<wme*>& justification, buffered_wme_list* buffered_wme_changes);
			
			// Add Command Parsing
			bool parse_chunk(agent* thisAgent, std::list<soar_module::symbol_triple*>* chunks, std::string* error_message, std::map<uint64_t, uint64_t>& number_replacement);
			
            // Print + helpers
			bool print_augs_of_lti(agent* theAgent, const Symbol* lti, std::string* result_message, unsigned int depth, unsigned int max_depth, const tc_number tc);
		};
	}
}

#endif
