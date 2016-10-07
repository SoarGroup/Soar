/*
 * dprint.h
 *
 *  Created on: Jul 2, 2016
 *      Author: mazzin
 */

#ifndef CORE_SOARKERNEL_SRC_DEBUG_CODE_DPRINT_H_
#define CORE_SOARKERNEL_SRC_DEBUG_CODE_DPRINT_H_

#ifndef SOAR_RELEASE_VERSION

    #include "output_manager.h"

    /* Sometimes it's useful to break when a single, hardcoded ID is encountered
     * in a piece of code being debugged.  You can set this variable to the name
     * of the symbol and call check_symbol or check_symbol_in_test to break there */
    //#define DEBUG_CHECK_SYMBOL "topfoo-copy"
    //extern sqlite_database  *db_err_epmem_db, *db_err_smem_db;

    #define dprint(mode, format, ...) Output_Manager::Get_OM().debug_print_sf (mode, format , ##__VA_ARGS__)
    #define dprint_set_indents(mode, ...) Output_Manager::Get_OM().set_dprint_indents (mode , ##__VA_ARGS__)
    #define dprint_set_test_format(mode, ...) Output_Manager::Get_OM().set_dprint_test_format (mode , ##__VA_ARGS__)
    #define dprint_clear_indents(mode, ...) Output_Manager::Get_OM().clear_dprint_indents (mode , ##__VA_ARGS__)
    #define dprint_clear_test_format(mode, ...) Output_Manager::Get_OM().clear_dprint_test_format (mode , ##__VA_ARGS__)
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

    #define dprint_variablization_table(mode) thisAgent->explanationBasedChunker->print_variablization_table (mode)
    #define dprint_tables(mode) thisAgent->explanationBasedChunker->print_tables (mode)
    #define dprint_o_id_tables(mode) thisAgent->explanationBasedChunker->print_o_id_tables (mode)
    #define dprint_attachment_points(mode) thisAgent->explanationBasedChunker->print_attachment_points (mode)
    #define dprint_constraints(mode) thisAgent->explanationBasedChunker->print_constraints (mode)
    #define dprint_merge_map(mode) thisAgent->explanationBasedChunker->print_merge_map (mode)
    #define dprint_ovar_to_o_id_map(mode) thisAgent->explanationBasedChunker->print_ovar_to_o_id_map (mode)
    #define dprint_o_id_substitution_map(mode) thisAgent->explanationBasedChunker->print_o_id_substitution_map (mode)
    #define dprint_o_id_to_ovar_debug_map(mode) thisAgent->explanationBasedChunker->print_o_id_to_ovar_debug_map (mode)

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

    #define dprint_variablization_table(mode, ...) ((void)0)
    #define dprint_tables(mode) ((void)0)
    #define dprint_o_id_tables(mode) ((void)0)
    #define dprint_attachment_points(mode) ((void)0)
    #define dprint_constraints(mode) ((void)0)
    #define dprint_merge_map(mode) ((void)0)
    #define dprint_ovar_to_o_id_map(mode) ((void)0)
    #define dprint_o_id_substitution_map(mode) ((void)0)
    #define dprint_o_id_to_ovar_debug_map(mode) ((void)0)

#endif

#endif /* CORE_SOARKERNEL_SRC_DEBUG_CODE_DPRINT_H_ */
