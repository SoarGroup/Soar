/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gski_event_system_kernel.h 
*********************************************************************
* created:	   6/17/2002   13:16
*
* purpose: 
*********************************************************************/

#ifndef KERNEL_STRUCT_H
#define KERNEL_STRUCT_H

#include "gski_event_system_data.h"

typedef unsigned long tc_number;
typedef struct cons_struct cons;
typedef cons list;

/**
 * @brief The kernel structure represents a collection of agents.
 *
 * The kernel structure was introduced to help make the kernel reentrant.
 *  All non-agent global structures were placed here.  Obviously, this
 *  is a bit of a hack, but it is the best that can be done quickly.
 */
typedef struct kernel_struct {

   list *all_soar_agents;

   /* Moved these here from production.cpp. -AJC (8/9/02) */
   int agent_counter;
   int agent_count;

   /* Moved this here from rhsfun.cpp. -AJC (8/9/02) */
   /* rhs_function *available_rhs_functions; */

   /** Contains the list of kernel callbacks for gSKI */
   // gSKI_K_CallbackData     gskiCallbacks[gSKI_K_MAX_KERNEL_EVENTS];

} Kernel;

/* ============================== FUNCTIONS ============================*/

/** 
 * @brief Creates an instance of the soar kernel structure.
 * 
 * Call this before trying to create any agents or anything
 *  else in the system.
 */
extern Kernel* create_kernel();

/** 
 * @brief Destroys the kernel object and clean up memory used by it
 *
 * This method destroys the kernel and all memory it is currently using.
 *
 */
extern void    destroy_kernel(Kernel* soarKernel);


#endif
