/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/* kernel.h */

#ifndef KERNEL_H
#define KERNEL_H

#include "enums.h"
typedef struct agent_struct agent;
extern void print(agent* thisAgent, const char* format, ...);

#define SOAR_RELEASE_VERSION

#ifndef SOAR_RELEASE_VERSION
    /* --  The following enables debugging traces/modes. Individual debug
     *     #defines are found in debug_defines.h -- */
    #define SOAR_DEBUG_PRINTING
    #define DEBUG_SAVE_IDENTITY_TO_RULE_SYM_MAPPINGS

    /* -- Enables tracing functions that print SQL processing and errors -- */
    //#define DEBUG_EPMEM_SQL

    /* -- Enables the printing of the call trace within debug messages.  Tested
     *    on OSX (Mountain Lion).  Compiles and might also work on Linux,
     *    but not tested. Does not work on Windows. -- */
    //#define DEBUG_MAC_STACKTRACE

    /* -- Enables extensive refcount and deallocation data tracking into
     *    the debug database -- */
    //#define DEBUG_TRACE_REFCOUNT_INVENTORY

    //#define DEBUG_EPMEM_WME_ADD
    //#define DEBUG_MEMORY  /* -- Zeroes out memory on init and fills with garbage on dealloc -- */
    //#define DEBUG_PREFS         /* -- Preference printouts -- */
    //#define DEBUG_RETE_PNODES
    //#define DEBUG_WATERFALL
    //#define DEBUG_LINKS       /* -- Get links, gc printouts -- */
    //#define DEBUG_CT_OSUPPORT /* Print names of productions that can't be fully compile-time o-support evaluated */

    /* -- Low level GDS debug information -- */
    //#define DEBUG_GDS

    /* -- High-level information on the instantiations that created an
     * o-supported element and lead to the elaboration of the GDS */
    //#define DEBUG_GDS_HIGH

    #define MEMORY_POOL_STATS   /* -- Collects memory pool stats for stats command -- */
    #define MEM_POOLS_ENABLED 1
    #define USE_MEM_POOL_ALLOCATORS 1
#else
    //#define MEMORY_POOL_STATS   /* -- Collects memory pool stats for stats command -- */
    #define MEM_POOLS_ENABLED 1
//    #define USE_MEM_POOL_ALLOCATORS 1
#endif

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

//typedef uint64_t tc_number;  /* Moving this here breaks windows non-scu build for some reason */
typedef unsigned char byte;

/* ----------------- Compiles directives that alter Soar behavior ---------------------- */

//#define NO_TIMING_STUFF
//#define DO_TOP_LEVEL_REF_CTS
#define O_REJECTS_FIRST
#define BUG_139_WORKAROUND
#define DISCARD_CHUNK_VARNAMES false

/* -- These enable rete stat tracking code that is broken right now (may be superficial) -- */
//#define TOKEN_SHARING_STATS       /* get statistics on token counts with and without sharing */
//#define SHARING_FACTORS           /* gather statistics on beta node sharing */
//#define NULL_ACTIVATION_STATS     /* gather statistics on null activation */

#ifndef NO_TIMING_STUFF
#define DETAILED_TIMING_STATS
#endif

//#define DO_COMPILE_TIME_O_SUPPORT_CALCS      /* comment out the following line to suppress compile-time o-support calculations */
//#define LIST_COMPILE_TIME_O_SUPPORT_FAILURES   /* get printouts of names of productions that can't be fully compile-time o-support evaluated*/

/* ---------------- Experimental modes.  Probably don't work any more -------------- */
//#define REAL_TIME_BEHAVIOR
//#define ATTENTION_LAPSE

/* --------------- Explanation of directives that alter Soar behavior ----------------
 *
 *  - MEMORY_POOL_STATS: Keep statistics on memory pool usage
 *
 *  - NO_TIMING_STUFF: Eliminates all timing statistics. The "stats" command
 *                     will have much shorter output.
 *
 *  - DETAILED_TIMING_STATS: Keep statistics on how much time is spent in
 *                           various parts of the system.
 *      Note: If you have DETAILED_TIMING_STATS defined, you must NOT define
 *            NO_TIMING_STUFF
 *
 *  - DO_TOP_LEVEL_REF_CTS: Maintain refcounts on wmes/prefs at the top level.
 *    Note: This can result in larger memory growth due to top-level objects
 *    that never get deallocated because the ref counts never drop to 0.
 *    The default for Soar v6 - v8.6.1 was to maintain the ref cts.  It's
 *    possible that in your particular application, weird things could happen
 *    if you don't do these ref cts, but if you are trying to improve performance
 *    and reduce memory, it's worth testing your system with the
 *    top-level-ref-cts turned off. Soar will be much more efficient.
 *    See comments in recmem.cpp
 *
 *  - BUG_139_WORKAROUND: Enable workaround for bug 139
 *  - BUG_139_WORKAROUND_WARNING: Print a warning whenever we are ignoring a
 *    situation when there's no instance to retract for a justification
 *
 *  - O_REJECTS_FIRST: Enable Soar to deal with certain productions in a more
 *    intuitive manner.  In particular, productions that modify a wme value by
 *    reject its current value and asserting its new value need to ensure that
 *    the current and new values differ.  This option may add a small run time
 *    cost, since two loops are made through the preferences list.
 *
 *  - DISCARD_CHUNK_VARNAMES: Set to false to preserve variable names in chunks
 *    within the rete.  This takes extra memory.
 *
 *  - TOKEN_SHARING_STATS: Get statistics on token counts in rete with and
 *    without sharing.
 *
 *  - TOKEN_SHARING_STATS: Gather statistics on null activations in the rete.
 *
 *  - SHARING_FACTORS:Gather statistics on beta node sharing in the rete.
 *
 * ----------------------------------------------------------------------------- */


#endif

