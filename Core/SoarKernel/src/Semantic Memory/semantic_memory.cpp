#include "semantic_memory.h"

namespace soar
{
	namespace semantic_memory
	{
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

		// Front for parse_cue, parse_retrieve, parse_remove
		bool semantic_memory::parse_agent_command(agent* theAgent, std::string** error_message)
		{

		}

		bool semantic_memory::parse_cue(agent* theAgent, const idSymbol* root_of_cue, std::string** result_message)
		{
			return backend->parse_cue(theAgent, root_of_cue, result_message);
		}

		bool semantic_memory::remove_lti(agent* theAgent, const idSymbol* lti_to_remove, std::string** result_message, bool force)
		{
			return backend->remove_lti(theAgent, lti_to_remove, force, result_message);
		}

		bool semantic_memory::remove_ltis(agent* theAgent, const std::list<const idSymbol*> lti_to_remove, std::string** result_message, bool force)
		{
			for (const auto& id : lti_to_remove)
			{
				if (!backend->remove_lti(theAgent, id, force, result_message))
					return false;
			}

			return true;
		}

		bool semantic_memory::retrieve_lti(agent* theAgent, const idSymbol* lti_to_retrieve, std::string** result_message)
		{
			return backend->retrieve_lti(theAgent, lti_to_retrieve, result_message);
		}

		void semantic_memory::export_memory_to_graphviz(std::string** graphviz)
		{

		}

		void semantic_memory::export_lti_to_graphviz(const idSymbol* lti, std::string** graphviz)
		{

		}

		void semantic_memory::print_memory(std::string** result_message)
		{
			
		}

		void semantic_memory::print_lti(const idSymbol* lti, std::string** result_message, unsigned int depth, bool history)
		{
			
		}

		uint64_t semantic_memory::lti_count()
		{

		}

		void lti_from_test(test t, std::list<Symbol*>* valid_ltis)
		{
			if (test_is_blank_test(t))
				return;

			if (test_is_blank_or_equality_test(t))
			{
				Symbol* referent = referent_of_equality_test(t);
				if (referent->symbol_type == IDENTIFIER_SYMBOL_TYPE && referent->id->smem_lti != NIL)
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
				if (sym->symbol_type == IDENTIFIER_SYMBOL_TYPE && sym->id->smem_lti != NIL)
					valid_ltis->push_back(sym);
			}
			else
			{
				list* fl = rhs_value_to_funcall_list(rv);
				for (cons* c = fl->rest; c != NIL; c = c->rest)
					lti_from_rhs_value(static_cast<rhs_value>(c->first), valid_ltis);
			}
		}

		bool semantic_memory::valid_production(condition* lhs, action* rhs)
		{
			std::list<Symbol*> valid_ltis;
			std::list<Symbol*>::iterator lti_p;

			// collect valid ltis
			for (condition* c = lhs_top; c != NIL; c = c->next)
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

			for (a = rhs_top; a != NIL; a = a->next)
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

				for (a = rhs_top; a != NIL; a = a->next)
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
							else if (id->id->smem_lti == NIL)
								good_action = true;
							// valid long-term identifiers are ok
							else if (valid_ltis.find(id) != valid_ltis.end())
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

		bool semantic_memory::backup_to_file(std::string& file, std::string** error_message)
		{
			return backend->backup_to_file(file, error_message);
		}
	}
}