/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file igski_instanceinfo.h 
*********************************************************************
* created:	   6/13/2002   15:00
*
* purpose: 
*********************************************************************/

#ifndef IGSKI_INSTANCEINFO_H
#define IGSKI_INSTANCEINFO_H

#include "gSKI_Enumerations.h"

namespace gSKI {

   struct Error;

   /**
   * @brief Interface for obtaining information about a kernel instance
   * 
   * This class encapsulates information related to a running instance
   *   of the Soar kernel.  This information is typically used in
   *   decisions of which running version of the kernel to attach to
   *   (especially in a networked environment where several machines
   *   may be running kernel servers).
   *
   * @author Jacob Crossman
   */
   class IInstanceInfo 
   {
   public:
      /**
      * @brief Destructor for InstanceInfo
      */
      virtual ~IInstanceInfo() {}

      /**
       * @brief   Gets the name of the kernel instance this information refers to
       *
       * Each instance of the kernel has an associated name.  The names are not necessarily
       *  unique, but they can be used as a non-unique identifier for an instance.
       *
       * @param  pErr Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       * @returns The name of the kernel instance this info object represents.
       *            This method never returns 0 even on error.  Worst case is it
       *            returns an empty string.
       */
      virtual const char* GetInstanceName(Error* pErr = 0) const = 0;
      
      /**
       * @brief Gets the full pathname of the application hosting the soar kernel (the server)
       *
       * The format of the pathname will be dependent on the system on which the client resides
       *  and on the particular type of transport layer being used.  Typically, the format is
       *  either the client's local file-system format or a URL (for remote servers).
       *
       * @param  pErr Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       * @returns This is the fully qualified pathname of the server hosting the kernel.  
       *             If the kernel is in-process, this will be the name of the client application.
       *             This method never returns 0 even on error.  Worst case is it
       *             returns an empty string.
       */
      virtual const char* GetServerName(Error* pErr = 0) const = 0;
      
      /**
       * @brief  Returns the type of process hosting the kernel instance 
       *
       * The process types are gSKI_IN_PROCESS, gSKI_LOCAL_OUT_OF_PROCESS, and 
       *   gSKI_REMOTE_OUT_OF_PROCESS.
       *
       * @param  pErr Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       * @returns The type of process hosting the kernel instance this instance info
       *           object refers to.  If there is a call failure, the return value is
       *           undefined.
       */
      virtual egSKIProcessType    GetProcessType(Error* pErr = 0) const = 0;
      
      /**
       * @brief  Return the threading model for this kernel instance
       *
       * Possible threading models are gSKI_MULTI_THREAD and gSKI_SINGLE_THREAD.
       *
       * @param  pErr Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       * @returns The threading model of the kernel instance this instance info object
       *             refers to.  If there is a failure, the return value is undefined.
       */
      virtual egSKIThreadingModel GetThreadingModel(Error* pErr = 0) const = 0;
   };
}

#endif
