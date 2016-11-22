/*
 * symbol_factory.h
 *
 *  Created on: Aug 19, 2016
 *      Author: mazzin
 */

#ifndef CORE_SOARKERNEL_SRC_SOAR_REPRESENTATION_SYMBOL_MANAGER_H_
#define CORE_SOARKERNEL_SRC_SOAR_REPRESENTATION_SYMBOL_MANAGER_H_

#include "kernel.h"

#include "symbol.h"
#include "symbols_predefined.h"

#include <iostream>
#include <string>
bool is_DT_mode_enabled(TraceMode mode);

#ifdef DEBUG_TRACE_REFCOUNT_FOR
    /* -- Reference count functions for symbols
     *      All symbol creation/copying use these now, so we can avoid accidental leaks more easily.
     *      If DEBUG_REFCOUNT_DB is defined, an alternate version of the function is used
     *      that sends a bunch of trace information to the debug database for deeper analysis of possible
     *      bugs. -- */
    std::string get_stacktrace(const char* prefix);
#endif

class EXPORT_INTERNAL Symbol_Manager {

        friend Output_Manager;

    public:
        Symbol_Manager(agent* pAgent);
        ~Symbol_Manager();

        predefined_symbols  soarSymbols;

        void init_symbol_tables();
        void retesave(FILE* f);

        void create_predefined_symbols();
        void release_predefined_symbols();
        void print_internal_symbols();

        Symbol* make_variable(const char* name);
        Symbol* make_str_constant(char const* name);
        Symbol* make_int_constant(int64_t value);
        Symbol* make_float_constant(double value);
        Symbol* make_new_identifier(char name_letter, goal_stack_level level, uint64_t name_number = NIL);
        Symbol* generate_new_str_constant(const char* prefix, uint64_t* counter);

        void deallocate_symbol_list_removing_references(cons*& sym_list);
        cons* copy_symbol_list_adding_references(cons* sym_list);

        Symbol* find_variable(const char* name);
        Symbol* find_identifier(char name_letter, uint64_t name_number);
        Symbol* find_str_constant(const char* name);
        Symbol* find_int_constant(int64_t value);
        Symbol* find_float_constant(double value);

        bool remove_if_sti(agent* thisAgent, void* item, void* userdata);
        bool reset_id_counters();
        void reset_id_and_variable_tc_numbers();
        void reset_hash_table(MemoryPoolType lHashTable);

        uint64_t* get_id_counter(uint64_t name_letter ) { return &id_counter[name_letter]; }

        /* --------------------------------------------------------------------
                                 Variable Generator

           These routines are used for generating new variables.  The variables
           aren't necessarily "completely" new--they might occur in some existing
           production.  But we usually need to make sure the new variables don't
           overlap with those already used in a *certain* production--for instance,
           when variablizing a chunk, we don't want to introduce a new variable that
           conincides with the name of a variable already in an NCC in the chunk.

           To use these routines, first call reset_variable_generator(), giving
           it lists of conditions and actions whose variables should not be
           used.  Then call generate_new_variable() any number of times; each
           time, you give it a string to use as the prefix for the new variable's
           name.  The prefix string should not include the opening "<".
        -------------------------------------------------------------------- */

        void reset_variable_generator(condition* conds_with_vars_to_avoid, action* actions_with_vars_to_avoid);
        Symbol* generate_new_variable(const char* prefix);

        //-- symbol_add_ref -----------------

        #ifdef DEBUG_REFCOUNT_DB
        #define symbol_add_ref(sym) \
            ({debug_store_refcount(sym, true); \
                symbol_add_ref_func(sym); })

        inline void symbol_add_ref_func(Symbol* x)
        #else
        inline void symbol_add_ref(Symbol* x)
        #endif
        {
        #ifdef DEBUG_REFCOUNT_DB
            //dprint(DT_REFCOUNT_ADDS, "ADD-REF %t -> %u\n", x, (x)->reference_count + 1);
        #else
//            dprint(DT_REFCOUNT_ADDS, "ADD-REF %y -> %u\n", x, (x)->reference_count + 1);
            if (is_DT_mode_enabled(DT_REFCOUNT_ADDS))
            {
                std::cout << "ADD-REF " << x->to_string() << "->" <<  (x->reference_count + 1) << "\n";
            }
        #endif

        #ifdef DEBUG_TRACE_REFCOUNT_FOR
            std::string strName(x->to_string());
            if (strName == DEBUG_TRACE_REFCOUNT_FOR)
            {
                std::string caller_string = get_stacktrace("add_ref");
        //        dprint(DT_ID_LEAKING, "-- | %s(%u) | %s++\n", strName.c_str(), x->reference_count, caller_string.c_str());
                if (is_DT_mode_enabled(DT_ID_LEAKING))
                {
                    std::cout << "++ | " << strName.c_str() << " | " << x->reference_count << " | " << caller_string.c_str() << "\n";
                }
            }
        #endif

            (x)->reference_count++;
        }

        //-- symbol_remove_ref -----------------

        #ifdef DEBUG_REFCOUNT_DB
        #define symbol_remove_ref(&sym) \
            ({debug_store_refcount(sym, false); \
                symbol_remove_ref_func(sym);})

        inline void symbol_remove_ref_func(Symbol*& x)
        #else
        inline void symbol_remove_ref(Symbol** x)
        #endif
        {
        #ifdef DEBUG_REFCOUNT_DB
            //dprint(DT_REFCOUNT_REMS, "REMOVE-REF %y -> %u\n", x, (x)->reference_count - 1);
        #else
//            dprint(DT_REFCOUNT_REMS, "REMOVE-REF %y -> %u\n", *x, (*x)->reference_count - 1);
//            if (is_DT_mode_enabled(DT_REFCOUNT_REMS))
//            {
//                std::cout << "REMOVE-REF " << (*x)->to_string() << "->" <<  ((*x)->reference_count - 1) << "\n";
//            }
        #endif
        //    assert((x)->reference_count > 0);
            (*x)->reference_count--;

        #ifdef DEBUG_TRACE_REFCOUNT_FOR
            std::string strName((*x)->to_string());
            if (strName == DEBUG_TRACE_REFCOUNT_FOR)
            {
                std::string caller_string = get_stacktrace("remove_ref");
        //        dprint(DT_ID_LEAKING, "-- | %s(%u) | %s--\n", strName.c_str(), (*x)->reference_count, caller_string.c_str());
                if (is_DT_mode_enabled(DT_ID_LEAKING))
                {
                    std::cout << "-- | " << strName.c_str() << " | " << (*x)->reference_count << " | " << caller_string.c_str() << "\n";
                }
            }
        #endif

            if ((*x)->reference_count == 0)
            {
                deallocate_symbol(*x);
                (*x) = NULL;
            }
        }

    private:

        agent*      thisAgent;
        uint32_t    current_symbol_hash_id;
        uint64_t    id_counter[26];
        uint64_t    current_variable_gensym_number;
        uint64_t    gensymed_variable_count[26];

        struct hash_table_struct* float_constant_hash_table;
        struct hash_table_struct* identifier_hash_table;
        struct hash_table_struct* int_constant_hash_table;
        struct hash_table_struct* str_constant_hash_table;
        struct hash_table_struct* variable_hash_table;

        void clear_variable_gensym_numbers();

        void deallocate_symbol(Symbol*& sym);

        uint32_t get_next_symbol_hash_id(agent* thisAgent) { return (current_symbol_hash_id += 137); }

};

#endif /* CORE_SOARKERNEL_SRC_SOAR_REPRESENTATION_SYMBOL_MANAGER_H_ */
