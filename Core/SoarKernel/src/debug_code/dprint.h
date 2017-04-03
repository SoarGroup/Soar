/*
 * dprint.h
 *
 *  Created on: Jul 2, 2016
 *      Author: mazzin
 */
#ifndef CORE_SOARKERNEL_SRC_DEBUG_CODE_DPRINT_H_
#define CORE_SOARKERNEL_SRC_DEBUG_CODE_DPRINT_H_

#include "kernel.h"

#ifdef SOAR_RELEASE_VERSION

    /* These definitions will cause debug printing to be compiled out */

    #define dprint(mode, format, ...) ((void)0)
    #define dprint_noprefix(mode, format, ...) ((void)0)
    #define dprint_header(mode, h, ...) ((void)0)

    #define dprint_saved_test_list(mode, st) ((void)0)
    #define dprint_partial_matches(mode, p_p_node) ((void)0)
    #define dprint_variablization_table(mode, ...) ((void)0)
    #define dprint_id_to_identity_map(mode) ((void)0)
    #define dprint_instantiation_identities_map(mode) ((void)0)

#else

    /* Since SOAR_RELEASE_VERSION is off, define debug print macros that do something */

    #include "output_manager.h"

    #define dprint(mode, format, ...) Output_Manager::Get_OM().debug_print_sf (mode, format , ##__VA_ARGS__)
    #define dprint_noprefix(mode, ...) Output_Manager::Get_OM().debug_print_sf_noprefix (mode , ##__VA_ARGS__)
    #define dprint_header(mode, h, ...) Output_Manager::Get_OM().debug_print_header (mode , h , ##__VA_ARGS__)

    /* -- The rest of these could be migrated to soar format strings -- */
    #define dprint_saved_test_list(mode, st) Output_Manager::Get_OM().print_saved_test_list (mode, st)
    #define dprint_partial_matches(mode, p_p_node) Output_Manager::Get_OM().print_partial_matches (mode, p_p_node)
    #define dprint_variablization_table(mode) thisAgent->explanationBasedChunker->print_variablization_table (mode)
    #define dprint_id_to_identity_map(mode) thisAgent->explanationBasedChunker->print_id_to_identity_map (mode)
    #define dprint_instantiation_identities_map(mode) thisAgent->explanationBasedChunker->print_instantiation_identities_map (mode)

#endif

#endif /* CORE_SOARKERNEL_SRC_DEBUG_CODE_DPRINT_H_ */
