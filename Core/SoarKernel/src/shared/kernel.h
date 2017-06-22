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
#include "macros.h"

#define SOAR_RELEASE_VERSION

#ifdef NDEBUG
    #define SOAR_RELEASE_VERSION
#endif

/* Defining SOAR_RELEASE_VERSION will:
 *
 * 1 -  Remove debug trace statements
 * 2 -  Soar will not re-direct all printing output from callbacks to cout
 *         (useful for debugging, keeps things in the right order)
 * 3 -  Memory pools and memory pool allocators will be turned on (if disabled below)
 * 4 -  Debugging flags below will be disabled
 *
 * Note:  The default build is optimized, so NDEBUG will be true, which will automatically
 *        make SOAR_RELEASE_VERSION also true.
 *
 *        (use --dbg for debug build)
 *
 * Note:  Debug printing will prevent Soar from sending output to the SoarJavaDebugger.
 * */

/* BUG_139_WORKAROUND:  Print a warning whenever we are ignoring a situation when there's
 * no instance to retract for a justification.  We can't find documentation on what the
 * original bug was.  We have seen the warning pop up in agents still.  */

/* DO_TOP_LEVEL_COND_REF_CTS: Whether to increment refcounts on prefs and WMEs
 * for the conditions in top level instantiation matches.  May be more safe,
 * but allows a situations where many data structures are never deallocated
 * because of a sequence of dependent instantiation firings in the top state.
 * - This option was turned on in Soar 6 to 8.6 and turned off in 9.0 to 9.5.1b
 */
/*  RETE stat tracking                     Note:  May be broken right now though bug might be superficial */

#define BUG_139_WORKAROUND
//#define BUG_139_WORKAROUND_WARNING
//#define DO_TOP_LEVEL_COND_REF_CTS
//#define TOKEN_SHARING_STATS           /* get statistics on token counts with and without sharing */
//#define SHARING_FACTORS               /* gather statistics on beta node sharing */
//#define NULL_ACTIVATION_STATS         /* gather statistics on null activation */

/* Timer settings */
//#define NO_TIMING_STUFF             /* Eliminates all timing statistics. */
#ifndef NO_TIMING_STUFF               /* Tracks additional statistics on how much time is spent in various parts of the system. */
//    #define DETAILED_TIMING_STATS
#endif

/* Spreading Activation Switch: Spreading can incur some small cost even when off
 * because of record-keeping in case it is later turned on. To reduce this cost,
 * spreading can be "more disabled" by eliminating that record-keeping.*/
#define SPREADING_ACTIVATION_ENABLED

/* --------------- Compiler directives for debugging ---------------------- *
 *   Note: #defines that enable trace messages pf SQL processing and errors   *
 *   can be found in soar_db.cpp                                              */
/* =============================== */
#ifdef SOAR_RELEASE_VERSION

    #define EBC_DETAILED_STATISTICS

    //#define MEMORY_POOL_STATS
    #define MEM_POOLS_ENABLED 1
    #ifdef MEM_POOLS_ENABLED
        #define USE_MEM_POOL_ALLOCATORS 1
//        #define USE_UNORDERED_STL
    #endif

#else

    /* Memory settings */

    //#define DEBUG_MEMORY            /* Fills with garbage on deallocation. Can be set to also zero out memory on init.*/
    //#define MEMORY_POOL_STATS             /* Collects memory pool stats for stats command */
    #define MEM_POOLS_ENABLED 1             /* Whether to use memory pools or the heap for allocation */
    #ifdef MEM_POOLS_ENABLED
        #define USE_MEM_POOL_ALLOCATORS 1   /* Whether to use custom STL allocators that use memory pools */
        //#define USE_UNORDERED_STL           /* Whether to use unordered STL structures that don't use memory pools instead of ordered ones that do */
    #endif

    /* The following provide trace messages that could not be easily switch to dprints */
    //#define DEBUG_EPMEM_WME_ADD
    //#define DEBUG_WATERFALL       /* Use DT_WATERFALL. This setting adds retraction and nil goal retraction list printing */
    //#define DEBUG_GDS             /* Use DT_GDS and DT_GDS_HIGH.  This setting just adds parent instantiations that it recurses through */
    //#define DEBUG_INCOMING_SML    /* Prints message coming in via KernelSML::ProcessIncomingSML */

    /* Only used for EBC debugging and experimentation */
    //#define EBC_SANITY_CHECK_RULES
    //#define EBC_DONT_PROPAGATE_IDENTITIES
    #define EBC_DETAILED_STATISTICS
    #define EBC_DEBUG_STATISTICS
    #define EBC_DETAILED_TIMERS

    //#define DEBUG_ATTR_AS_LINKS     /* Experimental link count setting that increments and decrements for identifiers in attribute elements*/

    //#define DEBUG_REFCOUNT_CHANGE_REGIONS
    //#define DEBUG_TRACE_IDSET_REFCOUNTS

    /* The debug inventories are a tool to keep track of instantiations/prefs/wme
     * allocation/deallocation manually using numeric IDs to see if any are still
     * around at soar init or exit.  Code that changes refcounts are instrumented
     * to add these counts.  Compiled out in optimized build.
     *
     * Note: Unit tests that use multiple agents will fail if inventories are enabled. If
     *       someone wanted to use this stuff with multiple agents, they'd need to refactor
     *       the code to be agent specific. */

    //#define DEBUG_GDS_INVENTORY
    //#define DEBUG_INSTANTIATION_INVENTORY
    //#define DEBUG_PREFERENCE_INVENTORY
    //#define DEBUG_WME_INVENTORY
    //#define DEBUG_IDSET_INVENTORY
    //#define DEBUG_RHS_SYMBOL_INVENTORY
    //#define DEBUG_RHS_FUNCTION_INVENTORY
    //#define DEBUG_ACTION_INVENTORY
    //#define DEBUG_TEST_INVENTORY
#endif

#endif
