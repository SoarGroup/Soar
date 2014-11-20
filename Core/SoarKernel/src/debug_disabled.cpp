/*
 * debug_disabled.cpp
 *
 *  Created on: Nov 10, 2013
 *      Author: mazzin
 */


#include "portability.h"
#include "kernel.h"

#ifndef SOAR_DEBUG_UTILITIES
#include "debug.h"
#include "lexer.h"

//#include "soar_db.h"

typedef char* rhs_value;
typedef struct test_struct test_info;
typedef test_info* test;
typedef struct condition_struct condition;
typedef struct action_struct action;
typedef struct production_struct production;
typedef struct saved_test_struct saved_test;
typedef char varnames;
typedef struct node_varnames_struct node_varnames;
typedef struct identity_struct identity_info;

/* -- Empty functions that should get optimized away in the release build -- */
void dprint_start_fresh_line(TraceMode mode) {}
void dprint(TraceMode mode, const char* format, ...) {}
void dprint_noprefix(TraceMode mode, const char* format, ...) {}
void dprint_test_simple(test t, const char* pre_string, const char* post_string) {}
void dprint_test_old(TraceMode mode, test t, const char* indent_string , const char* conj_indent_string) {}
void dprint_test(TraceMode mode, test t, bool print_actual, bool print_original, bool print_identity, const char* pre_string, const char* post_string) {}
void dprint_current_lexeme(TraceMode mode, soar::Lexer* lexer) {}
void dprint_condition(TraceMode mode, condition* cond, const char* indent_string , bool print_actual, bool print_original, bool print_identity) {}
void dprint_condition_list(TraceMode mode, condition* top_cond, const char* indent_string , bool print_actual, bool print_original, bool print_identity) {}
void dprint_action(TraceMode mode, action* a, const char* indent_string) {}
void dprint_action_list(TraceMode mode, action* action_list, struct token_struct* tok, wme* w, const char* indent_string ) {}
void dprint_instantiation(TraceMode mode, instantiation* inst, const char* indent_string ) {}
void dprint_cond_prefs(TraceMode mode, condition* top_cond, preference* top_pref, const char* indent_string , int print_inst_prefs) {}
void dprint_cond_actions(TraceMode mode, condition* top_cond, action* top_action, const char* indent_string ) {}
void dprint_cond_results(TraceMode mode, condition* top_cond, preference* top_pref, const char* indent_string ) {}
void dprint_preference(TraceMode mode, preference* pref, const char* indent_string, bool print_actual, bool print_original, bool print_identity) {}
void dprint_preferences(TraceMode mode, preference* top_pref, const char* indent_string , bool print_actual, bool print_original, bool print_identity, int pref_list_type) {}
void dprint_production(TraceMode mode, production* prod) {}
void dprint_identifiers(TraceMode mode) {}
void dprint_condition_cons(TraceMode mode, cons* c, bool print_actual, bool print_original, bool print_identity, const char* pre_string) {}
void dprint_rhs_value(TraceMode mode, rhs_value rv, struct token_struct* tok, wme* w) {}
void dprint_saved_test_list(TraceMode mode, saved_test* st) {}
void dprint_varnames(TraceMode mode, varnames* var_names) {}
void dprint_varnames_node(TraceMode mode, node_varnames* var_names_node) {}
void dprint_identity(TraceMode mode, identity_info* i, const char* pre_string, const char* post_string) {}
void dprint_all_inst(TraceMode mode) {}
void dprint_wmes(TraceMode mode, bool pOnlyWithIdentity) {}
void dprint_wme(TraceMode mode, wme* w, bool pOnlyWithIdentity) {}

void debug_init_db(agent* thisAgent) {}
void debug_print_db_err(TraceMode mode) {}
void debug_print_epmem_table(const char* table_name, TraceMode mode) {}
void debug_print_smem_table(const char* table_name, TraceMode mode) {}

void debug_store_refcount(Symbol* sym, bool isAdd) {}
void debug_destroy_for_refcount(agent* delete_agent) {}

//void debug_test(int type) {}

/* MToDo | Temporary debugging code.  Remove.
           Allows breakpointing on a single, hardcoded ID when encountered by
           calls to check_symbol or check_symbol_in_test. */
//#define DEBUG_CHECK_SYMBOL "topfoo-copy"

//std::string get_stacktrace(const char* prefix) { return NULL; }
//bool check_symbol(agent* thisAgent, Symbol* sym, const char* message) {return false;}
//bool check_symbol_in_test(agent* thisAgent, test t, const char* message) {return false;}

#endif
