/*
 * debug_inventories.h
 *
 *  Created on: Feb 1, 2017
 *      Author: mazzin
 */

#ifndef CORE_SOARKERNEL_SRC_DEBUG_CODE_DEBUG_INVENTORIES_H_
#define CORE_SOARKERNEL_SRC_DEBUG_CODE_DEBUG_INVENTORIES_H_

#include "kernel.h"

/* These are used to record the change in a refcount across the two calls */
void debug_refcount_change_start(agent* thisAgent, const char* symString, bool twoPart);
void debug_refcount_change_end(agent* thisAgent, const char* symString, const char* callerString, bool twoPart);

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
