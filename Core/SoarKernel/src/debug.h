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

#define SOAR_DEBUG_PRINTING

#include "portability.h"
#include "kernel.h"
#include "soar_db.h"
#include "soar_module.h"
#include "Export.h"
#include "output_manager.h"

#include <string>

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
#ifdef SOAR_DEBUG_PRINTING

    //extern sqlite_database  *db_err_epmem_db, *db_err_smem_db;

    #define dprint_set_params(mode, args...) Output_Manager::Get_OM().set_dprint_params (mode , ##args)
    #define dprint_clear_params(mode, args...) Output_Manager::Get_OM().clear_dprint_params (mode , ##args)
    #define dprint_y(mode, format, ...) Output_Manager::Get_OM().debug_print_sf (mode, format , ##args)
    #define dprint_noprefix(mode, args...) Output_Manager::Get_OM().debug_print_sf_noprefix (mode , ##args)
    #define dprint_start_fresh_line(mode) Output_Manager::Get_OM().debug_start_fresh_line (mode)
    #define dprint_current_lexeme(mode) Output_Manager::Get_OM().print_current_lexeme (mode)
    #define dprint_instantiation(mode, inst) \
            Output_Manager::Get_OM().print_instantiation (mode, inst)
    #define dprint_cond_prefs_inst(mode, top_cond, top_pref) \
            Output_Manager::Get_OM().print_cond_prefs (mode, top_cond, top_pref)
    #define dprint_cond_actions(mode, thisagent, top_cond, top_action) \
            Output_Manager::Get_OM().print_cond_actions (mode, thisagent, top_cond, top_action)
    #define dprint_cond_results(mode, top_cond, top_pref) \
            Output_Manager::Get_OM().print_cond_results (mode, top_cond, top_pref)
    #define dprint_production(mode, prod) Output_Manager::Get_OM().debug_print_production (mode, prod)
    #define dprint_identifiers(mode) Output_Manager::Get_OM().print_identifiers (mode)
    #define dprint_saved_test_list(mode, st) Output_Manager::Get_OM().print_saved_test_list (mode, st)
    #define dprint_varnames_node(mode, var_names_node) Output_Manager::Get_OM().print_varnames_node (mode, var_names_node)
    #define dprint_varnames(mode, var_names) Output_Manager::Get_OM().print_varnames (mode, var_names)
    #define dprint_all_inst(mode) Output_Manager::Get_OM().print_all_inst (mode)
    #define dprint_wmes(mode, pOnlyWithIdentity) Output_Manager::Get_OM().print_wmes (mode, pOnlyWithIdentity)
    #define dprint_wme(mode, w) Output_Manager::Get_OM().print_wme (mode, w, false)
    #define dprint(mode, format, args...) Output_Manager::Get_OM().debug_print_sf (mode, format , ##args)
#else
    #define dprint_set_params(mode, args...) ((void)0)
    #define dprint_clear_params(mode, args...) ((void)0)
    #define dprint_y(mode, format, ...) ((void)0)
    #define dprint_noprefix(mode, format, ...) ((void)0)
    #define dprint_start_fresh_line(mode) ((void)0)
    #define dprint_current_lexeme(mode) ((void)0)
    #define dprint_instantiation(mode, inst) ((void)0)
    #define dprint_cond_prefs_inst(mode, top_cond, top_pref) ((void)0)
    #define dprint_cond_actions(mode, top_cond, top_action) ((void)0)
    #define dprint_cond_results(mode, top_cond, top_pref) ((void)0)
    #define dprint_production(mode, prod) ((void)0)
    #define dprint_identifiers(mode) ((void)0)
    #define dprint_saved_test_list(mode, st) ((void)0)
    #define dprint_varnames(mode, var_names) ((void)0)
    #define dprint_varnames_node(mode, var_names_node) ((void)0)
    #define dprint_all_inst(mode) ((void)0)
    #define dprint_wmes(mode, pOnlyWithIdentity) ((void)0)
    #define dprint_wme(mode, w) ((void)0)
    #define dprint(mode, format, args...) ((void)0)
#endif

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

