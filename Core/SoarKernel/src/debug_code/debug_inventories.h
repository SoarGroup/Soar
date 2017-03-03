/*
 * debug_inventories.h
 *
 *  Created on: Feb 1, 2017
 *      Author: mazzin
 */

#ifndef CORE_SOARKERNEL_SRC_DEBUG_CODE_DEBUG_INVENTORIES_H_
#define CORE_SOARKERNEL_SRC_DEBUG_CODE_DEBUG_INVENTORIES_H_

#include "kernel.h"

/* The debug inventories keep track of instantiations/prefs/wme allocation/deallocation
 * using numeric IDs to see if any are still around at soar init or exit.  Code that
 * changes refcounts are instrumented to add these counts.  Compiled out in optimized build.
 *
 * Note: Unit tests that use multiple agents will fail if inventories are enabled. If
 *       someone wanted to use this stuff with multiple agents, they'd need to move the
 *       inventory tracking maps somewhere that they can be agent specific. */

//#define DEBUG_GDS_INVENTORY
//#define DEBUG_INSTANTIATION_INVENTORY
//#define DEBUG_PREFERENCE_INVENTORY
//#define DEBUG_WME_INVENTORY
#define DEBUG_IDSET_INVENTORY

/* These are used to record the change in a refcount across the two calls.  The
 * twoPart argument is used when there is a range you want to look at while tracking
 * another range of calls, for example the refcount changes during chunking that
 * happen in the middle of the ones for instantiation allocation.  Very crude and
 * sometimes wrong, but helpful. */
void debug_refcount_change_start(agent* thisAgent, bool twoPart);
void debug_refcount_change_end(agent* thisAgent, const char* callerString, bool twoPart);
void debug_refcount_reset();

void IDI_add(agent* thisAgent, instantiation* pInst);
void IDI_remove(agent* thisAgent, uint64_t pID);
void IDI_print_and_cleanup(agent* thisAgent);

void PDI_add(agent* thisAgent, preference* pPref, bool isShallow = false);
void PDI_remove(agent* thisAgent, preference* pPref);
void PDI_print_and_cleanup(agent* thisAgent);

void WDI_add(agent* thisAgent, wme* pWME);
void WDI_remove(agent* thisAgent, wme* pWME);
void WDI_print_and_cleanup(agent* thisAgent);

void GDI_add(agent* thisAgent, goal_dependency_set* pGDS);
void GDI_remove(agent* thisAgent, goal_dependency_set* pGDS);
void GDI_print_and_cleanup(agent* thisAgent);

void ISI_add(agent* thisAgent, identity_set* pIDSet);
void ISI_remove(agent* thisAgent, identity_set* pIDSet);
void ISI_print_and_cleanup(agent* thisAgent);

#endif /* CORE_SOARKERNEL_SRC_DEBUG_CODE_DEBUG_INVENTORIES_H_ */
