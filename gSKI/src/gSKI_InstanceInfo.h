/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gski_instanceinfo.h 
*********************************************************************
* created:	   6/28/2002   12:06
*
* purpose: 
*********************************************************************/
#ifndef GSKI_INSTANCEINFO_H
#define GSKI_INSTANCEINFO_H

#include "gSKI_Enumerations.h"
#include "IgSKI_InstanceInfo.h"

#include <string>

namespace gSKI
{
   struct Error;

   class InstanceInfo : public IInstanceInfo
   {
   public:
      /**
       * @brief Constructor for InstanceInfo;
       */
      InstanceInfo(const char*         instanceName, 
                   const char*         serverName, 
                   egSKIProcessType    processType, 
                   egSKIThreadingModel threadingModel);
      
      /**
      * @brief Destructor for InstanceInfo
      */
      ~InstanceInfo();

      /**
       * @brief   Gets the name of the kernel instance this information refers to
       *
       * Each instance of the kernel has an associated name.  The names are not necessarily
       *  unique, but they can be used as a non-unique identifier for an instance.
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       * @returns The name of the kernel instance this info object represents.
       *            This method never returns 0 even on error.  Worst case is it
       *            returns an empty string.
       */
      const char* GetInstanceName(Error* err = 0) const;
      
      /**
       * @brief Gets the full pathname of the application hosting the soar kernel (the server)
       *
       * The format of the pathname will be dependent on the system on which the client resides
       *  and on the particular type of transport layer being used.  Typically, the format is
       *  either the client's local file-system format or a URL (for remote servers).
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       * @returns This is the fully qualified pathname of the server hosting the kernel.  
       *             If the kernel is in-process, this will be the name of the client application.
       *             This method never returns 0 even on error.  Worst case is it
       *             returns an empty string.
       */
      const char* GetServerName(Error* err = 0) const;
      
      /**
       * @brief  Returns the type of process hosting the kernel instance 
       *
       * The process types are gSKI_IN_PROCESS, gSKI_LOCAL_OUT_OF_PROCESS, and 
       *   gSKI_REMOTE_OUT_OF_PROCESS.
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       * @returns The type of process hosting the kernel instance this instance info
       *           object refers to.  If there is a call failure, the return value is
       *           undefined.
       */
      egSKIProcessType    GetProcessType(Error* err = 0) const;
      
      /**
       * @brief  Return the threading model for this kernel instance
       *
       * Possible threading models are gSKI_MULTI_THREAD and gSKI_SINGLE_THREAD.
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       * @returns The threading model of the kernel instance this instance info object
       *             refers to.  If there is a failure, the return value is undefined.
       */
      egSKIThreadingModel GetThreadingModel(Error* err = 0) const;

   private:
      std::string         m_instanceName;
      std::string         m_serverName;
      egSKIThreadingModel m_threadingModel;
      egSKIProcessType    m_processType;

   };
}
#endif
