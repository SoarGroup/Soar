#ifdef HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H
#include "portability.h"

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

#include "kernel_struct.h"
#include "gski_event_system_functions.h"

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

   // DJP-FREE: Need to be sure to zero this out so it is re-initialized correctly.
   free(soar_version_string) ;
   soar_version_string = 0 ;
}
