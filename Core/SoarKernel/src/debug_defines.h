/* -----------------------------------------------------------------------------
 *                                debug_defines.h
 * --------------- Compiler directives for debugging messages ------------------
 *
 *  Created on: Jul 17, 2013
 *      Author: Mazin
 * ----------------------------------------------------------------------------- */

#ifndef DEBUG_DEFINES_H_
#define DEBUG_DEFINES_H_

/* -- The schema version used by the output manager's debug database -- */
#define DEBUG_SCHEMA_VERSION "0.1"

//#define DEBUG_FREE_SETTINGS
#define DEBUG_UNITTEST_SETTINGS

#define EBC_MERGE_CONDITIONS
#define EBC_ADD_CONSTRAINTS_IDENTITIES

#ifdef DEBUG_FREE_SETTINGS
/* -- Which trace messages should be printed -- */

#define TRACE_Init_No_Mode                      true
#define TRACE_Init_TM_EPMEM                     true
#define TRACE_Init_TM_SMEM                      true
#define TRACE_Init_TM_LEARNING                  true
#define TRACE_Init_TM_CHUNKING                  true
#define TRACE_Init_TM_RL                        true
#define TRACE_Init_TM_WMA                       true

/* -- Which debug messages should be printed -- */
#define TRACE_Init_DT_No_Mode                     true
#define TRACE_Init_DT_DEBUG                       true
//--
#define TRACE_Init_DT_REFCOUNT_ADDS               false
#define TRACE_Init_DT_REFCOUNT_REMS               false
#define TRACE_Init_DT_DEALLOCATES                 false
#define TRACE_Init_DT_DEALLOCATE_SYMBOLS          false
#define TRACE_Init_DT_ID_LEAKING                  false
//--
#define TRACE_Init_DT_SOAR_INSTANCE               false
#define TRACE_Init_DT_CLI_LIBRARIES               false
#define TRACE_Init_DT_EPMEM_CMD                   false
#define TRACE_Init_DT_PARSER                      false
#define TRACE_Init_DT_GDS                         false
//--
#define TRACE_Init_DT_MILESTONES                  true
#define TRACE_Init_DT_PRINT_INSTANTIATIONS        true
//--
#define TRACE_Init_DT_ADD_ADDITIONALS             false
#define TRACE_Init_DT_VARIABLIZATION_MANAGER      true
#define TRACE_Init_DT_VM_MAPS                     false
#define TRACE_Init_DT_BACKTRACE                   true
#define TRACE_Init_DT_BUILD_CHUNK_CONDS           false
#define TRACE_Init_DT_IDENTITY_PROP               false
#define TRACE_Init_DT_UNIFICATION                 false
#define TRACE_Init_DT_CONSTRAINTS                 false
#define TRACE_Init_DT_LHS_VARIABLIZATION          false
#define TRACE_Init_DT_RHS_VARIABLIZATION          false
#define TRACE_Init_DT_NCC_VARIABLIZATION          false
#define TRACE_Init_DT_RL_VARIABLIZATION           false
#define TRACE_Init_DT_UNGROUNDED_STI              false
#define TRACE_Init_DT_MERGE                       false
#define TRACE_Init_DT_REORDERER                   false
#define TRACE_Init_DT_EBC_CLEANUP                 false
//--
#define TRACE_Init_DT_NONE_1                      false
#define TRACE_Init_DT_NONE_2                      false
#define TRACE_Init_DT_NONE_3                      false
#define TRACE_Init_DT_NONE_4                      false

/* -- Which output listeners should be initially turned on -- */
#define OM_Init_print_enabled     on
#define OM_Init_db_mode           off
#define OM_Init_callback_mode     off
#define OM_Init_stdout_mode       on

/* -- Which output debug listeners should be initially turned on -- */
#define OM_Init_db_dbg_mode       off
#define OM_Init_callback_dbg_mode off
#define OM_Init_stdout_dbg_mode   on
#endif

#ifdef DEBUG_UNITTEST_SETTINGS
/* -- Which trace messages should be printed -- */

#define TRACE_Init_No_Mode                      true
#define TRACE_Init_TM_EPMEM                     true
#define TRACE_Init_TM_SMEM                      true
#define TRACE_Init_TM_LEARNING                  true
#define TRACE_Init_TM_CHUNKING                  true
#define TRACE_Init_TM_RL                        true
#define TRACE_Init_TM_WMA                       true

/* -- Which debug messages should be printed -- */
#define TRACE_Init_DT_No_Mode                     true
#define TRACE_Init_DT_DEBUG                       true
//--
#define TRACE_Init_DT_REFCOUNT_ADDS               false
#define TRACE_Init_DT_REFCOUNT_REMS               false
#define TRACE_Init_DT_DEALLOCATES                 false
#define TRACE_Init_DT_DEALLOCATE_SYMBOLS          false
#define TRACE_Init_DT_ID_LEAKING                  false
//--
#define TRACE_Init_DT_SOAR_INSTANCE               false
#define TRACE_Init_DT_CLI_LIBRARIES               false
#define TRACE_Init_DT_EPMEM_CMD                   false
#define TRACE_Init_DT_PARSER                      false
#define TRACE_Init_DT_GDS                         false
//--
#define TRACE_Init_DT_MILESTONES                  false
#define TRACE_Init_DT_PRINT_INSTANTIATIONS        false
//--
#define TRACE_Init_DT_ADD_ADDITIONALS             false
#define TRACE_Init_DT_VARIABLIZATION_MANAGER      false
#define TRACE_Init_DT_VM_MAPS                     false
#define TRACE_Init_DT_BACKTRACE                   false
#define TRACE_Init_DT_BUILD_CHUNK_CONDS           false
#define TRACE_Init_DT_IDENTITY_PROP               false
#define TRACE_Init_DT_UNIFICATION                 false
#define TRACE_Init_DT_CONSTRAINTS                 false
#define TRACE_Init_DT_LHS_VARIABLIZATION          false
#define TRACE_Init_DT_RHS_VARIABLIZATION          false
#define TRACE_Init_DT_NCC_VARIABLIZATION          false
#define TRACE_Init_DT_RL_VARIABLIZATION           false
#define TRACE_Init_DT_UNGROUNDED_STI              false
#define TRACE_Init_DT_MERGE                       false
#define TRACE_Init_DT_REORDERER                   false
#define TRACE_Init_DT_EBC_CLEANUP                 false
//--
#define TRACE_Init_DT_NONE_1                      false
#define TRACE_Init_DT_NONE_2                      false
#define TRACE_Init_DT_NONE_3                      false
#define TRACE_Init_DT_NONE_4                      false

/* -- Which output listeners should be initially turned on -- */
#define OM_Init_print_enabled     on
#define OM_Init_db_mode           off
#define OM_Init_callback_mode     on
#define OM_Init_stdout_mode       off

/* -- Which output debug listeners should be initially turned on -- */
#define OM_Init_db_dbg_mode       off
#define OM_Init_callback_dbg_mode off
#define OM_Init_stdout_dbg_mode   off
#endif

#endif /* DEBUG_DEFINES_H_ */
