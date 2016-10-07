/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/* kernel.h */

#ifndef KERNEL_H
#define KERNEL_H

#include "portability.h"
#include "enums.h"
#include "forward.h"

//#define SOAR_RELEASE_VERSION

#ifndef NDEBUG
    #define SOAR_RELEASE_VERSION
#endif

/* Defining SOAR_RELEASE_VERSION will:
 *
 * 1 -  Remove debug trace statements
 * 2 -  Soar will not re-direct all printing output from callbacks to cout
 *         (useful for debugging, keeps things in the right order)
 * 3 -  Use a union for type-specific Symbol pointers (artifact of refactoring)
 * 4 -  Chunking debug information that maps identities to variables will NOT be cached
 * 5 -  Memory pools and memory pool allocators will be turned on (if disabled below)
 * 6 -  Debugging flags below will be disabled
 *
 * Note:  The default build is optimized, so NDEBUG will be true, which will automatically
 *        make SOAR_RELEASE_VERSION also true.
 *
 *        (use --dbg for debug build)
 *
 * Note:  Debug printing will prevent Soar from sending output to the SoarJavaDebugger.
 * */

/* --------------- Compiler directives that alter Soar behavior --------------------*/

//#define DO_TOP_LEVEL_REF_CTS          /* Maintains refcounts on wme/pref at top level.  May be more safe, but less efficient.  Was standard in v6-v8.6 */
#define BUG_139_WORKAROUND_WARNING      /* Print a warning whenever we are ignoring a situation when there's no instance to retract for a justification */
#define BUG_139_WORKAROUND
#define DISCARD_CHUNK_VARNAMES true     /* Do not save variable names in RETE when learning chunks.  Saves memory. */

/* -------- Compiler directives for potentially expensive statistics ---------------*/
//#define NO_TIMING_STUFF             /* Eliminates all timing statistics. */
#ifndef NO_TIMING_STUFF               /* Tracks additional statistics on how much time is spent in various parts of the system. */
    //#define DETAILED_TIMING_STATS
#endif

/*  rete stat tracking (may be broken right now though bug might be superficial) */
//#define TOKEN_SHARING_STATS           /* get statistics on token counts with and without sharing */
//#define SHARING_FACTORS               /* gather statistics on beta node sharing */
//#define NULL_ACTIVATION_STATS         /* gather statistics on null activation */

/* -------------- Macros for safe counters ------------*/

#define increment_counter(counter) counter++; if (counter == 0) counter = 1;
#define add_to_counter(counter, amt) uint64_t lastcnt = counter; counter += amt; if (counter < lastcnt) counter = amt;

/* --------------- Compiler directives for debugging ---------------------- *
 *   Note: #defines that enable trace messages pf SQL processing and errors   *
 *   can be found in soar_db.cpp                                              */
/* =============================== */

#ifndef SOAR_RELEASE_VERSION

    //#define MEMORY_POOL_STATS     /* Collects memory pool stats for stats command */
    //#define MEM_POOLS_ENABLED 1   /* Whether to use memory pools or the heap for allocation */
    #ifdef MEM_POOLS_ENABLED
        #define USE_MEM_POOL_ALLOCATORS 1   /* Whether to use custom STL allocators that use memory pools */
    #endif

    #define DEBUG_SAVE_IDENTITY_TO_RULE_SYM_MAPPINGS

    //#define DEBUG_ATTR_AS_LINKS   /* Experimental link count setting */
    //#define DEBUG_MAC_STACKTRACE  /* Enables the printing of the call stack within debug messages.
                                    /* Tested on OSX (Mountain Lion).  Does not work on Windows. */
    //#define DEBUG_REFCOUNT_DB     /* Enables extensive refcount and deallocation data tracking into a database */

    //#define DEBUG_EPMEM_WME_ADD
    #define DEBUG_MEMORY            /* Zeroes out memory on init and fills with garbage on dealloc */
    //#define DEBUG_PREFS
    //#define DEBUG_RETE_PNODES
    //#define DEBUG_WATERFALL
    //#define DEBUG_GDS             /* Low level GDS debug information */
    //#define DEBUG_GDS_HIGH        /* Include instantiations that created an o-supported element and
                                    /* lead to the elaboration of the GDS */
#else
    //#define MEMORY_POOL_STATS
    #define MEM_POOLS_ENABLED 1
    #ifdef MEM_POOLS_ENABLED
        #define USE_MEM_POOL_ALLOCATORS 1
    #endif
#endif

#endif

