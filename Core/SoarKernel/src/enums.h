/*
 * enums.h
 *
 *  Created on: Jul 17, 2013
 *      Author: mazzin
 */

#ifndef ENUMS_H_
#define ENUMS_H_

typedef unsigned char byte;

/* ------------------------- Various trace and debug modes ----------------------
 *
 *     NOTE: IF YOU ADD A NEW TRACE OR DEBUG MODE, MAKE SURE TO
 *
 *       (1) KEEP num_trace_modes UPDATED (below immediately after enum)
 *       (2) SET AN INITIAL VALUE IN debug_defines.h
 *       (3) INITIALIZE OUTPUT PREFIX INFO AND INITIAL VALUE IN
 *           Output_Manager::fill_mode_info (AT BOTTOM OF output_manager.cpp)!
 *
 * ------------------------------------------------------------------------------ */

enum TraceMode {
  No_Mode,
  TM_EPMEM,
  TM_SMEM,
  TM_LEARNING,
  TM_CHUNKING,
  TM_RL,
  TM_WMA,
  DT_DEBUG,
  DT_ID_LEAKING,
  DT_LHS_VARIABLIZATION,
  DT_UNIQUE_VARIABLIZATION,
  DT_ADD_CONSTRAINTS_ORIG_TESTS,
  DT_RHS_VARIABLIZATION,
  DT_VARIABLIZATION_MANAGER,
  DT_PRINT_INSTANTIATIONS,
  DT_ADD_TEST_TO_TEST,
  DT_DEALLOCATES,
  DT_DEALLOCATE_SYMBOLS,
  DT_REFCOUNT_ADDS,
  DT_REFCOUNT_REMS,
  DT_PARSER,
  DT_FUNC_PRODUCTIONS,
  DT_VARIABLIZATION_REV,
  DT_REORDERER,
  DT_BACKTRACE,
  DT_SAVEDVARS,
  DT_GDS,
  DT_RL_VARIABLIZATION,
  DT_NCC_VARIABLIZATION,
  DT_IDENTITY_PROP,
  DT_SOAR_INSTANCE,
  DT_CLI_LIBRARIES,
  num_trace_modes
};

enum MessageType {
  debug_msg,
  trace_msg,
  refcnt_msg
};

/* MToDo | This was moved from soar_module to make it easier to use debug_defines without including soar_module,
 *    since that file is included in many places.  Naming it "boolean" doesn't seem to conflict with other things,
 *    but this seems risky.  Maybe we can move it back and just do some sort of forward declaration or perhaps
 *    rename to something less common. -- */

/* -- An implementation of an on/off boolean parameter --*/

enum boolean { off, on };

/* -- Possible modes for numeric indifference -- */


/* -- MToDo | Change these to enums -- */
/* -------------------------------
      Types of Productions
------------------------------- */

#define USER_PRODUCTION_TYPE 0
#define DEFAULT_PRODUCTION_TYPE 1
#define CHUNK_PRODUCTION_TYPE 2
#define JUSTIFICATION_PRODUCTION_TYPE 3
#define TEMPLATE_PRODUCTION_TYPE 4

#define NUM_PRODUCTION_TYPES 5
// Soar-RL assumes that the production types start at 0 and go to (NUM_PRODUCTION_TYPES-1) sequentially

/* ---------------------------------------
    Match Set print parameters
--------------------------------------- */

#define MS_ASSERT_RETRACT 0      /* print both retractions and assertions */
#define MS_ASSERT         1      /* print just assertions */
#define MS_RETRACT        2      /* print just retractions */

typedef byte ms_trace_type;   /* must be one of the above constants */

/* ---------------------------------------
    How much information to print about
    the wmes matching an instantiation
--------------------------------------- */

#define NONE_WME_TRACE    1      /* don't print anything */
#define TIMETAG_WME_TRACE 2      /* print just timetag */
#define FULL_WME_TRACE    3      /* print whole wme */
#define NO_WME_TRACE_SET  4

typedef byte wme_trace_type;   /* must be one of the above constants */

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

#endif /* ENUMS_H_ */
