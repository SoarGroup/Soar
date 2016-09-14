/*
 * constants.h
 *
 *  Created on: Sep 2, 2016
 *      Author: mazzin
 */

#ifndef CORE_SOARKERNEL_SRC_SHARED_CONSTANTS_H_
#define CORE_SOARKERNEL_SRC_SHARED_CONSTANTS_H_

/* ------------------------------------------------------------------------
                      Explanation-Based Chunking
------------------------------------------------------------------------ */
#define NULL_IDENTITY_SET 0

#define PE_PRODS 0
#define IE_PRODS 1
#define NO_SAVED_PRODS -1

/* ------------------------------------------------------------------------
                             Impasse Types
------------------------------------------------------------------------ */

#define NONE_IMPASSE_TYPE 0                   /* no impasse */
#define CONSTRAINT_FAILURE_IMPASSE_TYPE 1
#define CONFLICT_IMPASSE_TYPE 2
#define TIE_IMPASSE_TYPE 3
#define NO_CHANGE_IMPASSE_TYPE 4

/* ---------------------------------------
    Match Set print parameters
--------------------------------------- */

#define MS_ASSERT_RETRACT 0      /* print both retractions and assertions */
#define MS_ASSERT         1      /* print just assertions */
#define MS_RETRACT        2      /* print just retractions */

/* ---------------------------------------
    How much information to print about
    the wmes matching an instantiation
--------------------------------------- */

#define NONE_WME_TRACE    1      /* don't print anything */
#define TIMETAG_WME_TRACE 2      /* print just timetag */
#define FULL_WME_TRACE    3      /* print whole wme */
#define NO_WME_TRACE_SET  4

/* -------------------------------
      Ways to Do User-Select
------------------------------- */

#define USER_SELECT_BOLTZMANN 1   /* boltzmann algorithm, with respect to temperature */
#define USER_SELECT_E_GREEDY  2   /* with probability epsilon choose random, otherwise greedy */
#define USER_SELECT_FIRST     3   /* just choose the first candidate item */
#define USER_SELECT_LAST      4   /* choose the last item   AGR 615 */
#define USER_SELECT_RANDOM    5   /* pick one at random */
#define USER_SELECT_SOFTMAX   6   /* pick one at random, probabalistically biased by numeric preferences */
#define USER_SELECT_INVALID   7   /* should be 1+ last item, used for validity checking */

/* -------------------------------
      Exploration constants
------------------------------- */
#define EXPLORATION_REDUCTION_EXPONENTIAL   0
#define EXPLORATION_REDUCTION_LINEAR        1
#define EXPLORATION_REDUCTIONS              2 // set as greatest reduction + 1

#define EXPLORATION_PARAM_EPSILON           0
#define EXPLORATION_PARAM_TEMPERATURE       1
#define EXPLORATION_PARAMS                  2 // set as greatest param + 1

/* -------------------------------------------------- */
/*     EpMem Constants  */
/* -------------------------------------------------- */

// algorithm constants
#define EPMEM_MEMID_NONE                            0
#define EPMEM_NODEID_ROOT                           0
#define EPMEM_NODEID_BAD                            -1
#define EPMEM_HASH_ACCEPTABLE                       1

#define EPMEM_NODE_POS                              0
#define EPMEM_NODE_NEG                              1
#define EPMEM_RANGE_START                           0
#define EPMEM_RANGE_END                             1
#define EPMEM_RANGE_EP                              0
#define EPMEM_RANGE_NOW                             1
#define EPMEM_RANGE_POINT                           2

#define EPMEM_RIT_ROOT                              0
#define EPMEM_RIT_OFFSET_INIT                       -1
#define EPMEM_LN_2                                  0.693147180559945

#define EPMEM_DNF                                   2

#define EPMEM_RIT_STATE_NODE                        0
#define EPMEM_RIT_STATE_EDGE                        1

#define EPMEM_SCHEMA_VERSION "2.0"

/* -------------------------------------------------- */
/*     Smem Constants  */
/* -------------------------------------------------- */

#define SMEM_ACT_MAX static_cast<uint64_t>( static_cast<uint64_t>( 0 - 1 ) / static_cast<uint64_t>(2) )
#define NO_WME_LEVEL 0
#define SMEM_AUGMENTATIONS_NULL 0
#define SMEM_AUGMENTATIONS_NULL_STR "0"
#define SMEM_ACT_HISTORY_ENTRIES 10
#define SMEM_ACT_LOW -1000000000
#define SMEM_SCHEMA_VERSION "3.0"


/* -------------------------------------------------- */
/*     Global constants, type declarations, etc.      */
/* -------------------------------------------------- */

#define BUFFER_MSG_SIZE 128
#define COLUMNS_PER_LINE 80
#define TOP_GOAL_LEVEL 1
#define ATTRIBUTE_IMPASSE_LEVEL 32767
#define LOWEST_POSSIBLE_GOAL_LEVEL 32767
#define NIL (0)
#define PRINT_BUFSIZE 5000   /* --- size of output buffer for a calls to print routines --- */
#define kChunkNamePrefixMaxLength  64  /* kjh (B14) */

/* ====================================================================
             Global System Parameters and Related Definitions
   ==================================================================== */

/* ====== Sysparams for what to trace === */
#define INVALID_SYSPARAM                          0
#define TRACE_CONTEXT_DECISIONS_SYSPARAM          1
#define TRACE_PHASES_SYSPARAM                     2

/* --- Warning: these next four MUST be consecutive and in the order of the production types defined above --- */
#define TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM      3
#define TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM   4
#define TRACE_FIRINGS_OF_CHUNKS_SYSPARAM          5
#define TRACE_FIRINGS_OF_JUSTIFICATIONS_SYSPARAM  6
#define TRACE_FIRINGS_OF_TEMPLATES_SYSPARAM       7
#define TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM     8
#define TRACE_FIRINGS_PREFERENCES_SYSPARAM        9
#define TRACE_WM_CHANGES_SYSPARAM                10
#define TRACE_CHUNK_NAMES_SYSPARAM               11
#define TRACE_JUSTIFICATION_NAMES_SYSPARAM       12
#define TRACE_CHUNKS_SYSPARAM                    13
#define TRACE_JUSTIFICATIONS_SYSPARAM            14
#define TRACE_BACKTRACING_SYSPARAM               15
/* ===== watch loading flag =====  KJC 7/96 */
#define TRACE_LOADING_SYSPARAM                   16
/* ====== User Select === */
#define USER_SELECT_MODE_SYSPARAM                17
/* ====== Print Warnings === */
#define PRINT_WARNINGS_SYSPARAM                  18
/* ====== Whether to print out aliases as they're defined === */
#define TRACE_OPERAND2_REMOVALS_SYSPARAM         19
#define REAL_TIME_SYSPARAM                       20
#define ATTENTION_LAPSE_ON_SYSPARAM              21
#define TRACE_INDIFFERENT_SYSPARAM               22
#define TIMERS_ENABLED                           23
/* auto-reduction of exploration parameters */
#define USER_SELECT_REDUCE_SYSPARAM              24
/* Soar-RL trace information */
#define TRACE_RL_SYSPARAM                        25
/* Chunk through local negations */
#define TRACE_WATERFALL_SYSPARAM                 26
#define TRACE_WMA_SYSPARAM                       27
#define TRACE_EPMEM_SYSPARAM                     28
#define TRACE_SMEM_SYSPARAM                      29
#define TRACE_GDS_SYSPARAM                       30
/* --- Warning: if you add sysparams, be sure to update the next line! --- */
#define HIGHEST_SYSPARAM_NUMBER                  31

#endif /* CORE_SOARKERNEL_SRC_SHARED_CONSTANTS_H_ */
