/*
 * symbol_factory.h
 *
 *  Created on: Aug 19, 2016
 *      Author: mazzin
 */

#ifndef CORE_SOARKERNEL_SRC_SOAR_REPRESENTATION_SYMBOL_FACTORY_H_
#define CORE_SOARKERNEL_SRC_SOAR_REPRESENTATION_SYMBOL_FACTORY_H_

#include "kernel.h"

#include "Export.h"

class Symbol_Factory {

    public:
        Symbol_Factory(agent* pAgent);
        ~Symbol_Factory();

    private:

        agent* thisAgent;

};

void init_symbol_tables(agent* thisAgent);
void create_predefined_symbols(agent* thisAgent);
void release_predefined_symbols(agent* thisAgent);
void print_internal_symbols(agent* thisAgent);

EXPORT Symbol* make_variable(agent* thisAgent, const char* name);
EXPORT Symbol* make_str_constant(agent* thisAgent, char const* name);
EXPORT Symbol* make_int_constant(agent* thisAgent, int64_t value);
EXPORT Symbol* make_float_constant(agent* thisAgent, double value);
EXPORT Symbol* make_new_identifier(agent* thisAgent, char name_letter, goal_stack_level level, uint64_t name_number = NIL);
extern Symbol* generate_new_str_constant(agent* thisAgent, const char* prefix, uint64_t* counter);

EXPORT void deallocate_symbol(agent* thisAgent, Symbol*& sym);
EXPORT void deallocate_symbol_list_removing_references(agent* thisAgent, ::cons*& sym_list);
::cons* copy_symbol_list_adding_references(agent* thisAgent, ::cons* sym_list);

EXPORT Symbol* find_variable(agent* thisAgent, const char* name);
EXPORT Symbol* find_identifier(agent* thisAgent, char name_letter, uint64_t name_number);
EXPORT Symbol* find_str_constant(agent* thisAgent, const char* name);
EXPORT Symbol* find_int_constant(agent* thisAgent, int64_t value);
EXPORT Symbol* find_float_constant(agent* thisAgent, double value);

bool reset_id_counters(agent* thisAgent);
void reset_id_and_variable_tc_numbers(agent* thisAgent);
void reset_variable_gensym_numbers(agent* thisAgent);

#endif /* CORE_SOARKERNEL_SRC_SOAR_REPRESENTATION_SYMBOL_FACTORY_H_ */
