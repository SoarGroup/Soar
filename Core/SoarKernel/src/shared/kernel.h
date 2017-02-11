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

#define SOAR_RELEASE_VERSION

#ifdef NDEBUG
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

/* Whether to increment refcounts on prefs and WMEs for the conditions in top level
 * instantiation matches.  May be more safe, but allows a situations where many
 * data structures are never deallocated because of a sequence of dependent
 * instantiation firings in the top state.
 * - This option was turned on in Soar 6 to 8.6 and turned off in 9.0 to 9.5.1b
 */
//#define DO_TOP_LEVEL_COND_REF_CTS

/* Print a warning whenever we are ignoring a situation when there's no instance to
 * retract for a justification.  We can't find documentation on what the original bug
 * was.  We have seen the warning pop up in agents still.  */
//#define BUG_139_WORKAROUND_WARNING
#define BUG_139_WORKAROUND

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
    #define MEM_POOLS_ENABLED 1   /* Whether to use memory pools or the heap for allocation */
    #ifdef MEM_POOLS_ENABLED
        #define USE_MEM_POOL_ALLOCATORS 1   /* Whether to use custom STL allocators that use memory pools */
    #endif

    //#define DEBUG_MEMORY            /* Fills with garbage on deallocation. Can be set to also zero out memory on init.*/
    //#define DEBUG_ATTR_AS_LINKS     /* Experimental link count setting */

    //#define DEBUG_EPMEM_WME_ADD
    //#define DEBUG_WATERFALL       /* Use DT_WATERFALL. This setting adds retraction and nil goal retraction list printing */
    //#define DEBUG_GDS             /* Use DT_GDS and DT_GDS_HIGH.  This setting just adds parent instantiations that it recurses through */
    //#define DEBUG_INCOMING_SML    /* Prints message coming in via KernelSML::ProcessIncomingSML */

#else
    //#define MEMORY_POOL_STATS
    #define MEM_POOLS_ENABLED 1
    #ifdef MEM_POOLS_ENABLED
        #define USE_MEM_POOL_ALLOCATORS 1
    #endif
#endif

#endif

/* ------------------------------------
 *    Format strings for Soar printing:
 *
 *       %c   character
 *       %d   int64_t
 *       %u   uint64_t
 *       %s   string
 *       %e   fresh line (adds newline if not at column 1)
 *       %f   double
 *       %-   fill to next column indent with spaces
 *       %=   fill to next column indent with periods
 *
 *       %a   action
 *       %l   condition
 *       %n   funcall list
 *       %p   preference
 *       %r   rhs value
 *       %y   symbol
 *       %t   test
 *       %g   variablization identity of test
 *       %h   like %g but with second argument with symbol to use if STI
 *       %w   wme
 *
 *       %1   condition list
 *       %2   action list
 *       %3   cons list of conditions
 *       %4   condition action lists (2 args: cond, action)
 *       %5   condition preference lists (2 args: cond, preference)
 *       %6   condition results lists (2 args: cond, preference)
 *       %7   instantiation
 *
 *   Alphabetical
 *
 *       %-   fill to next column indent with spaces
 *       %=   fill to next column indent with periods
 *       %1   condition list
 *       %2   action list
 *       %3   cons list of conditions
 *       %4   condition action lists (2 args: cond, action)
 *       %5   condition preference lists (2 args: cond, preference)
 *       %6   condition results lists (2 args: cond, preference)
 *       %7   instantiation
 *       %a   action
 *       %c   character
 *       %d   int64_t
 *       %e   fresh line (adds newline if not at column 1)
 *       %f   double
 *       %g   variablization identity of test
 *       %h   like %g but with second argument containing symbol to use if STI
 *       %l   condition
 *       %n   funcall list
 *       %p   preference
 *       %r   rhs value
 *       %s   string
 *       %t   test
 *       %u   uint64_t
 *       %w   wme
 *       %y   symbol
   ------------------------------------*/
