/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/*------------------------------------------------------------------
                       debug.h

   @brief debug.h provides some utility functions for inspecting and
          manipulating the data structures of the Soar kernel at run
          time.

          (Not much here now.  Will move some other utility stuff from
           experimental chunking and memory consolidation branches
           later.)

------------------------------------------------------------------ */

#ifndef SOARDEBUG_H
#define SOARDEBUG_H

#include "portability.h"
#include "kernel.h"
#include "soar_db.h"
#include "soar_module.h"
#include "Export.h"

#include <string>

#define OLD_DUPLICATE_CHUNK_METHOD
#define IGNORE_GDS_ERROR

#ifdef SOAR_DEBUG_UTILITIES

//extern sqlite_database  *db_err_epmem_db, *db_err_smem_db;
//extern agent *debug_agent;

#ifdef DEBUG_EPMEM_SQL
static void profile_sql(void* context, const char* sql, sqlite3_uint64 ns)
{
    fprintf(stderr, "Execution Time of %llu ms for: %s\n", ns / 1000000, sql);
}
static void trace_sql(void* /*arg*/, const char* query)
{
    fprintf(stderr, "Query: %s\n", query);
}
#endif
#endif

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

#ifdef SOAR_DEBUG_UTILITIES
#define dprint_macro(mode, format, args...) dprint (mode, format , ##args)
#else
#define dprint_macro(mode, format, args...) { }
//#define dprint_macro(mode, format, args...) ((void)0)
#endif

extern void dprint(TraceMode mode, const char* format, ...);
extern void dprint_noprefix(TraceMode mode, const char* format, ...);
extern void dprint_start_fresh_line(TraceMode mode);
extern void dprint_test_simple(test t, const char* pre_string, const char* post_string);
extern void dprint_test_old(TraceMode mode, test t, const char* indent_string = "          ", const char* conj_indent_string = "+ ");
extern void dprint_test(TraceMode mode, test t, bool print_actual = true, bool print_original = false, bool print_identity = true, const char* pre_string = "", const char* post_string = "");
extern void dprint_current_lexeme(TraceMode mode);
extern void dprint_condition(TraceMode mode, condition* cond, const char* indent_string = "          ", bool print_actual = true, bool print_original = false, bool print_identity = true);
extern void dprint_condition_list(TraceMode mode, condition* top_cond, const char* indent_string = "          ", bool print_actual = true, bool print_original = false, bool print_identity = true);
extern void dprint_action(TraceMode mode, action* a, const char* indent_string = "           ");
extern void dprint_action_list(TraceMode mode, action* action_list, const char* indent_string = "           ");
extern void dprint_instantiation(TraceMode mode, instantiation* inst, const char* indent_string = "          ");
extern void dprint_cond_prefs(TraceMode mode, condition* top_cond, preference* top_pref, const char* indent_string = "          ", int print_inst_prefs = 1);
extern void dprint_cond_actions(TraceMode mode, condition* top_cond, action* top_action, const char* indent_string = "          ");
extern void dprint_cond_results(TraceMode mode, condition* top_cond, preference* top_pref, const char* indent_string = "          ");
extern void dprint_preference(TraceMode mode, preference* pref, const char* indent_string, bool print_actual = true, bool print_original = false, bool print_identity = true);
extern void dprint_preferences(TraceMode mode, preference* top_pref, const char* indent_string = "           ", bool print_actual = true, bool print_original = false, bool print_identity = true, int pref_list_type = 1);
extern void dprint_production(TraceMode mode, production* prod);
extern void dprint_identifiers(TraceMode mode);
extern void dprint_condition_cons(TraceMode mode, cons* c, bool print_actual = true, bool print_original = false, bool print_identity = true, const char* pre_string = "");
extern void dprint_rhs_value(TraceMode mode, rhs_value rv, struct token_struct* tok = NIL, wme* w = NIL);
extern void dprint_saved_test_list(TraceMode mode, saved_test* st);
extern void dprint_varnames(TraceMode mode, varnames* var_names);
extern void dprint_varnames_node(TraceMode mode, node_varnames* var_names_node);
extern void dprint_identity(TraceMode mode, identity_info* i, const char* pre_string = "", const char* post_string = "");
extern void dprint_all_inst(TraceMode mode);
extern void dprint_wmes(TraceMode mode, bool pOnlyWithIdentity = false);
extern void dprint_wme(TraceMode mode, wme* w, bool pOnlyWithIdentity = false);

extern void debug_init_db(agent* thisAgent);
extern void debug_print_db_err(TraceMode mode = DT_DEBUG);
extern void debug_print_epmem_table(const char* table_name, TraceMode mode = DT_DEBUG);
extern void debug_print_smem_table(const char* table_name, TraceMode mode = DT_DEBUG);

extern void debug_store_refcount(Symbol* sym, bool isAdd);
extern void debug_destroy_for_refcount(agent* delete_agent);

extern void debug_test(int type = 1);

/* MToDo | Temporary debugging code.  Remove.
           Allows breakpointing on a single, hardcoded ID when encountered by
           calls to check_symbol or check_symbol_in_test. */
//#define DEBUG_CHECK_SYMBOL "topfoo-copy"

extern std::string get_stacktrace(const char* prefix = NULL);
extern bool check_symbol(agent* thisAgent, Symbol* sym, const char* message = "ChkSym | ");
extern bool check_symbol_in_test(agent* thisAgent, test t, const char* message = "ChkSym | ");

/**
 * @brief Contains the parameters for the debug command
 */
class debug_param_container: public soar_module::param_container
{
    public:

        soar_module::boolean_param* epmem_commands, *smem_commands, *sql_commands, *use_new_chunking;

        debug_param_container(agent* new_agent);

};

#endif

