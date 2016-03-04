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

/* ----------------------------------------------------------------------------
 * Comment the following line out to start with settings that are equivalent to
 * standard Soar printing.  To completely compile out debug statements, make
 * sure SOAR_RELEASE_VERSION is defined in kernel.h */
//#define DEBUG_OUTPUT_ON

#include "kernel.h"

#include "soar_db.h"
#include "soar_module.h"
#include "Export.h"
#include "output_manager.h"

#include <string>

extern void debug_set_mode_info(trace_mode_info mode_info[num_trace_modes], bool pEnabled);
/* --------------------------------------------------------------------------*/


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
//#if !defined(SOAR_RELEASE_VERSION) && defined(DEBUG_OUTPUT_ON)
    /* Not checking DEBUG_OUTPUT_ON anymore so that dprint is defined and we can turn debug output on
     * and off at runtime.  Downside is that we can have debug printing secretly slowing things down
     * if people forget to define SOAR_RELEASE_VERSION is defined in kernel.h */
#if !defined(SOAR_RELEASE_VERSION)

    /* Sometimes it's useful to break when a single, hardcoded ID is encountered
     * in a piece of code being debugged.  You can set this variable to the name
     * of the symbol and call check_symbol or check_symbol_in_test to break there */
    //#define DEBUG_CHECK_SYMBOL "topfoo-copy"
    //extern sqlite_database  *db_err_epmem_db, *db_err_smem_db;

    #define dprint(mode, format, ...) Output_Manager::Get_OM().debug_print_sf (mode, format , ##__VA_ARGS__)
    #define dprint_set_indents(mode, ...) Output_Manager::Get_OM().set_dprint_indents (mode , ##__VA_ARGS__)
    #define dprint_set_default_test_format(mode, ...) Output_Manager::Get_OM().set_default_dprint_test_format (mode , ##__VA_ARGS__)
    #define dprint_clear_indents(mode, ...) Output_Manager::Get_OM().clear_dprint_indents (mode , ##__VA_ARGS__)
    #define dprint_reset_test_format(mode, ...) Output_Manager::Get_OM().reset_dprint_test_format (mode , ##__VA_ARGS__)
    #define dprint_y(mode, format, ...) Output_Manager::Get_OM().debug_print_sf (mode, format , ##__VA_ARGS__)
    #define dprint_noprefix(mode, ...) Output_Manager::Get_OM().debug_print_sf_noprefix (mode , ##__VA_ARGS__)
    #define dprint_start_fresh_line(mode) Output_Manager::Get_OM().debug_start_fresh_line (mode)
    #define dprint_header(mode, h, ...) Output_Manager::Get_OM().debug_print_header (mode , h , ##__VA_ARGS__)

    /* -- The rest of these should all be migrated to soar format strings -- */
    #define dprint_current_lexeme(mode) Output_Manager::Get_OM().print_current_lexeme (mode)
    #define dprint_production(mode, prod) Output_Manager::Get_OM().debug_print_production (mode, prod)
    #define dprint_identifiers(mode) Output_Manager::Get_OM().print_identifiers (mode)
    #define dprint_saved_test_list(mode, st) Output_Manager::Get_OM().print_saved_test_list (mode, st)
    #define dprint_varnames_node(mode, var_names_node) Output_Manager::Get_OM().print_varnames_node (mode, var_names_node)
    #define dprint_varnames(mode, var_names) Output_Manager::Get_OM().print_varnames (mode, var_names)
    #define dprint_all_inst(mode) Output_Manager::Get_OM().print_all_inst (mode)

    #define dprint_variablization_tables(mode, ...) thisAgent->ebChunker->print_variablization_tables (mode , ##__VA_ARGS__)
    #define dprint_tables(mode) thisAgent->ebChunker->print_tables (mode)
    #define dprint_o_id_tables(mode) thisAgent->ebChunker->print_o_id_tables (mode)
    #define dprint_attachment_points(mode) thisAgent->ebChunker->print_attachment_points (mode)
    #define dprint_constraints(mode) thisAgent->ebChunker->print_constraints (mode)
    #define dprint_merge_map(mode) thisAgent->ebChunker->print_merge_map (mode)
    #define dprint_ovar_to_o_id_map(mode) thisAgent->ebChunker->print_ovar_to_o_id_map (mode)
    #define dprint_o_id_substitution_map(mode) thisAgent->ebChunker->print_o_id_substitution_map (mode)
    #define dprint_o_id_to_ovar_debug_map(mode) thisAgent->ebChunker->print_o_id_to_ovar_debug_map (mode)

#else
    #define dprint(mode, format, ...) ((void)0)
    #define dprint_set_indents(mode, ...) ((void)0)
    #define dprint_set_default_test_format(mode, ...) ((void)0)
    #define dprint_clear_indents(mode, ...) ((void)0)
    #define dprint_reset_test_format(mode, ...) ((void)0)
    #define dprint_y(mode, format, ...) ((void)0)
    #define dprint_noprefix(mode, format, ...) ((void)0)
    #define dprint_start_fresh_line(mode) ((void)0)
    #define dprint_header(mode, h, ...) ((void)0)

    #define dprint_current_lexeme(mode) ((void)0)
    #define dprint_production(mode, prod) ((void)0)
    #define dprint_identifiers(mode) ((void)0)
    #define dprint_saved_test_list(mode, st) ((void)0)
    #define dprint_varnames(mode, var_names) ((void)0)
    #define dprint_varnames_node(mode, var_names_node) ((void)0)
    #define dprint_all_inst(mode) ((void)0)

    #define dprint_variablization_tables(mode, ...) ((void)0)
    #define dprint_tables(mode) ((void)0)
    #define dprint_o_id_tables(mode) ((void)0)
    #define dprint_attachment_points(mode) ((void)0)
    #define dprint_constraints(mode) ((void)0)
    #define dprint_merge_map(mode) ((void)0)
    #define dprint_ovar_to_o_id_map(mode) ((void)0)
    #define dprint_o_id_substitution_map(mode) ((void)0)
    #define dprint_o_id_to_ovar_debug_map(mode) ((void)0)

#endif

extern void initialize_debug_trace(trace_mode_info mode_info[num_trace_modes]);
extern void debug_init_db(agent* thisAgent);
extern void debug_print_db_err(TraceMode mode = DT_DEBUG);
extern void debug_print_epmem_table(const char* table_name, TraceMode mode = DT_DEBUG);
extern void debug_print_smem_table(const char* table_name, TraceMode mode = DT_DEBUG);

extern void debug_store_refcount(Symbol* sym, bool isAdd);
extern void debug_destroy_for_refcount(agent* delete_agent);

extern void debug_test(int type = 1);
extern void debug_trace_set(int dt_num, bool pEnable);

extern std::string get_stacktrace(const char* prefix = NULL);
extern bool check_symbol(agent* thisAgent, Symbol* sym, const char* message = "ChkSym | ");
extern bool check_symbol_in_test(agent* thisAgent, test t, const char* message = "ChkSym | ");

extern bool wme_matches_string(wme *w, const char* match_id, const char* match_attr, const char* match_value);
extern bool symbol_matches_string(Symbol* sym, const char* match);
extern bool wme_matches_bug(wme *w);

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

