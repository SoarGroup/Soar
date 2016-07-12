/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/* gsysparam.h */

/* ====================================================================
             Global System Parameters and Related Definitions

   A set of system parameters (sysparam's for short) affect many operations
   of Soar, including learning, tracing, deciding, etc.  In order to
   provide a simple, uniform update mechanism (a single callback routine that
   gets called when any parameter changes), we store these parameters in
   an array sysparams[].  Below, we #define various indices into this array
   corresponding to various system parameters.

   Most of the parameters are of type "int64_t".  A few parameters are more
   naturally handled as lists; for these, the array value is just a dummy,
   and callback routines must inspect a global variable to get the real
   value.  Chunk_free_problem_spaces is an example of this.

   The array of sysparams[] can be read directly, but should be modified
   ONLY via calls to set_sysparam(), which is defined in init-soar.c.
==================================================================== */

#ifndef GSYSPARAM_H
#define GSYSPARAM_H

typedef struct agent_struct agent;

/* ====== Sysparams for what to trace === */
#define INVALID_SYSPARAM                          0
#define TRACE_CONTEXT_DECISIONS_SYSPARAM          1
#define TRACE_PHASES_SYSPARAM                     2

/* --- Warning: these next four MUST be consecutive and in the order of the
   production types defined above --- */
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
/* ====== Max Elaborations === */
#define MAX_ELABORATIONS_SYSPARAM                17
/* ====== Max Chunks === */
#define MAX_CHUNKS_SYSPARAM                      18
#define RESPOND_TO_LOAD_ERRORS_SYSPARAM          19
/* ====== User Select === */
#define USER_SELECT_MODE_SYSPARAM                20
/* ====== Print Warnings === */
#define PRINT_WARNINGS_SYSPARAM                  21
#define PRINT_ALIAS_SYSPARAM                     22
/* ====== Whether to print out aliases as they're defined === */
#define TRACE_OPERAND2_REMOVALS_SYSPARAM         23
#define REAL_TIME_SYSPARAM                       24
#define ATTENTION_LAPSE_ON_SYSPARAM              25
/* limit number of cycles in run_til_output */
#define MAX_NIL_OUTPUT_CYCLES_SYSPARAM           26
#define TRACE_INDIFFERENT_SYSPARAM               27
#define TIMERS_ENABLED                           28
#define MAX_GOAL_DEPTH                           29
/* generate warning and event if memory usage exceeds this value */
#define MAX_MEMORY_USAGE_SYSPARAM                30
/* auto-reduction of exploration parameters */
#define USER_SELECT_REDUCE_SYSPARAM              31
/* Soar-RL trace information */
#define TRACE_RL_SYSPARAM                        32
/* Chunk through local negations */
#define TRACE_WATERFALL_SYSPARAM                 33
#define TRACE_WMA_SYSPARAM                       34
#define TRACE_EPMEM_SYSPARAM                     35
#define TRACE_SMEM_SYSPARAM                      36
#define TRACE_GDS_SYSPARAM                       37
/* Break on long decision cycle time */
#define DECISION_CYCLE_MAX_USEC_INTERRUPT        38
/* Chunk over evaluation rules in subgoals */
#define TRACE_PARSER                             39
/* --- Warning: if you add sysparams, be sure to update the next line! --- */
#define HIGHEST_SYSPARAM_NUMBER                  40

/* -----------------------------------------
   Sysparams[] stores the parameters; set_sysparam()
   should be used to modify them.
----------------------------------------- */

extern void init_sysparams(agent* thisAgent);
extern void set_sysparam(agent* thisAgent, int param_number, int64_t new_value);
#endif
