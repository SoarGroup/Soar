/********************************************************************
* @file gski_event_system_kernel.h 
*********************************************************************
* @remarks Copyright (C) 2002 Soar Technology, All rights reserved. 
* The U.S. government has non-exclusive license to this software 
* for government purposes. 
*********************************************************************
* created:	   6/17/2002   13:16
*
* purpose: 
*********************************************************************/

#include "kernel_struct.h"
#include "gski_event_system_functions.h"

#include <stdlib.h>

/** 
 * @brief Creates an instance of the soar kernel structure.
 * 
 * Call this before trying to create any agents or anything
 *  else in the system.
 */
Kernel* create_kernel()
{
   Kernel* soarKernel;

   /* Create the kernel instance */
   soarKernel = (Kernel*)malloc(sizeof(Kernel));

   soarKernel->all_soar_agents = NIL;

   soarKernel->agent_count = -1;
   soarKernel->agent_counter = -1;

   /* soarKernel->available_rhs_functions = NIL; */

   /* Initialize all the members */
   //gSKI_InitializeKernelCallbacks(soarKernel);
  
   return soarKernel;
}

/** 
 * @brief Destroys the kernel object and clean up memory used by it
 *
 * This method destroys the kernel and all memory it is currently using.
 *
 */
void destroy_kernel(Kernel* soarKernel)
{
   assert(soarKernel != 0 && "");
   if(soarKernel)
   {
      /* Do destruction first if necessary */
      free(soarKernel);
   }
}
