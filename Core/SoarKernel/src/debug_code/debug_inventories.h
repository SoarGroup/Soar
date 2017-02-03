/*
 * debug_inventories.h
 *
 *  Created on: Feb 1, 2017
 *      Author: mazzin
 */

#ifndef CORE_SOARKERNEL_SRC_DEBUG_CODE_DEBUG_INVENTORIES_H_
#define CORE_SOARKERNEL_SRC_DEBUG_CODE_DEBUG_INVENTORIES_H_

#include "kernel.h"

#define DEBUG_INST_DEALLOCATION_INVENTORY   // To keep track of instantiations/prefs allocation/deallocation */
#define DEBUG_PREF_DEALLOCATION_INVENTORY   // to see if any are still around at soar init or exit.  Code is
                                            // instrumented to add these counts.  Compiled out in optimized build.

                                            // Note: Unit  tests that use multiple agents will fail if inventories are enabled. We'd need
                                            //       to move the inventory tracking maps somewhere that they can be agent specific. */


/* These are used to record the change in a refcount across the two calls */
void debug_refcount_change_start(agent* thisAgent, bool twoPart);
void debug_refcount_change_end(agent* thisAgent, const char* callerString, bool twoPart);
void debug_refcount_reset();

/* These are used when DEBUG_INST_DEALLOCATION_INVENTORY is defined.  They are used to print a report
 * of instantiations that aren't deallocated */

void IDI_add(agent* thisAgent, instantiation* pInst);
void IDI_remove(agent* thisAgent, uint64_t pID);
void IDI_print_and_cleanup(agent* thisAgent);

/* If DEBUG_PREF_DEALLOCATION_INVENTORY is #defined, this functions prints a report of preferences
 * that aren't deallocated */
void PDI_print_and_cleanup(agent* thisAgent);
void PDI_remove(agent* thisAgent, preference* pPref);
void PDI_add(agent* thisAgent, preference* pPref, bool isShallow = false);

#endif /* CORE_SOARKERNEL_SRC_DEBUG_CODE_DEBUG_INVENTORIES_H_ */
