/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gski_kernelfactory.h 
*********************************************************************
* created:	   6/26/2002   15:58
*
* purpose: 
*********************************************************************/

#ifndef GSKI_KERNELFACTORY_H
#define GSKI_KERNELFACTORY_H

#include "IgSKI_KernelFactory.h"
#include "IterUtils.h"
#include "gSKI_Iterator.h"

#include <vector>


namespace gSKI {

   class IInstanceInfo;
   class Kernel;

   /**
    * @brief A class that creates kernel instances
    *
    * A kernel factory creates kernel instances and attaches to already running instances.  
    * The interface provides methods for discovering what type of instance the kernel can
    * create or attach to, as well as the usual creation method and an attach method.
    */
   class KernelFactory : public IKernelFactory
   {
   public:

      /**
       * 
       */
      KernelFactory();

      /**
      * @brief Destructor 
      *
      * The destructor cleans up all gSKI objects including the agents.
      */
      ~KernelFactory() {}

      /**
      * @brief Gets the major and minor revision number of this version of gSKI
      *
      * @returns The version of gSKI as a floating point value.  The version number is
	   *           formatted as MAJOR_VERSION_NUM.MINOR_VERSION_NUM.  Return value on 
      *           failure is 0.0.
      * @param  err Pointer to client-owned error structure.  If the pointer
      *               is not 0 this structure is filled with extended error
      *               information.  If it is 0 (the default) extended error
      *               information is not returned.
      */
      Version GetGSKIVersion(Error* err = 0) const;

      /**
      * @brief Gets the major and minor revision number of the soar kernel that
	   *            this factory will create
      *
      * @param  err Pointer to client-owned error structure.  If the pointer
      *               is not 0 this structure is filled with extended error
      *               information.  If it is 0 (the default) extended error
      *               information is not returned.
      *
      * @returns The version of Soar as a floating point value.  The version number is
	   *           formatted as MAJOR_VERSION_NUM.MINOR_VERSION_NUM.  Return value on
      *           failure is 0.0.
      */
      Version GetKernelVersion(Error* err = 0) const;

	  /**
      * @brief Returns this factory's support for creating in-process instances of the kernel.
	   *
	   * An in-process instance of the kernel is one that shares the same memory space as
	   *  the client process.
	   *
      * @param  err Pointer to client-owned error structure.  If the pointer
      *               is not 0 this structure is filled with extended error
      *               information.  If it is 0 (the default) extended error
      *               information is not returned.
      *
      * @returns true if this factory supports creating an in-process instance of the kernel.
	   *            false if it cannot create an in-process instance.
      */
      bool CanCreateInProcess(Error* err = 0) const;

      /**
      * @brief Returns this factory's support for attaching to local (same machine)
	   *             out-of-process instances of the kernel
      *
	   * Attaching to a local out-of-process kernel consists of obtaining an already existing
	   *  instance of the kernel from process separate from the client's process.
	   *
      * @param  err Pointer to client-owned error structure.  If the pointer
      *               is not 0 this structure is filled with extended error
      *               information.  If it is 0 (the default) extended error
      *               information is not returned.
      *
      * @returns true if this factory supports attaching to a local out-of-process kernel instance.  
	   *              false is returned if it does not support local out-of-process connections.
      */
       bool CanAttachLocalOutOfProcess(Error* err = 0) const;

	  /**
      * @brief Returns this factory's support for creating local out-of-process instances of
	   *           the soar kernel 
      *
	   * An out-of-process instance is an instance of the kernel contained within a 
	   *  separate process than the client process. 
	   *
      * @param  err Pointer to client-owned error structure.  If the pointer
      *               is not 0 this structure is filled with extended error
      *               information.  If it is 0 (the default) extended error
      *               information is not returned.
      *
      * @returns true if this factory supports creation of out-of-process kernel instances.
	   *               false is returned if it only supports in-process creation.
      */
      bool CanCreateLocalOutOfProcess(Error* err = 0) const;

      /**
      * @brief Returns this factory's support for attaching to remote instances of the kernel
      *
	   * Attaching remotely means that this factory can link to an already existing instance of the
	   *  of the kernel running on a different machine.
	   *
      * @param  err Pointer to client-owned error structure.  If the pointer
      *               is not 0 this structure is filled with extended error
      *               information.  If it is 0 (the default) extended error
      *               information is not returned.
      *
      * @returns true if this factory supports attaching to a kernel processes on
	   *               remote machines.  false is returned if it only supports local
	   *				  connections.
      */
      bool CanAttachRemote(Error* err = 0) const;

	  /**
      * @brief Returns this factory's support for remotely creating instances of
	   *           the soar kernel 
      *
	   * You may want to query CanAttachRemotely also to determine if this factory
	   *  can attach to an already running remote kernel (this is different than
	   *  creating a remote instance).
	   *
      * @param  err Pointer to client-owned error structure.  If the pointer
      *               is not 0 this structure is filled with extended error
      *               information.  If it is 0 (the default) extended error
      *               information is not returned.
      *
      * @returns true if this factory supports creation of kernel processes on
	   *               remote machines.  false is returned if it only supports local
	   *				  creation.
      */
      bool CanCreateRemote(Error* err = 0) const;

      /**
      * @brief Returns this factory's support for multiple simultaneous instances of the kernel
	   *
	   * Factories that can produce out-of-process connections to the kernel usually support
	   *   multiple instances.  In process versions typically only support a single instance.
      *
      * @param  err Pointer to client-owned error structure.  If the pointer
      *               is not 0 this structure is filled with extended error
      *               information.  If it is 0 (the default) extended error
      *               information is not returned.
      *
      * @returns  true if this factory can support creating multiple instances
	   *                  of the soar kernel.  false if it can only create a single
	   *                  instance (a singleton).
      */
      bool CanCreateMultipleInstances(Error* err = 0) const;

	  /**
      * @brief Returns an iterator to all existing instances of the kernel to which
	   *          this factory can attach.
      *
	   * Multi-process implementations (and even some single process implementations) may
	   *  allow you to attach to an existing instance of the kernel.  Use this method to
	   *  search through the list of available instances of the kernel and pick one to
	   *  attach to.
	   *
      * @param  err Pointer to client-owned error structure.  If the pointer
      *               is not 0 this structure is filled with extended error
      *               information.  If it is 0 (the default) extended error
      *               information is not returned.
      *
      * @returns An iterator to the instance information for each instance of the kernel
	   *            to which this factory is capable to attach.  If there are no available
	   *            instances, the iterator will be invalid immediately.
      */
      tIInstanceInfoIterator* GetInstanceIterator(Error* err = 0) const;

     /**
      * @brief Creates a new instance of the soar kernel
      *
      * Call this method to create an instance of the Soar Kernel.  The method will create
      *  an instance that matches the criteria passed in.
      *
      * Possible Errors:
      *      @li  gSKIERR_INSTANCE_EXISTS - If an instance of the kernel already exists
      *                                     and the given factory does not support creating
      *                                     multiple instances of the kernel.
      *      @li  gSKIERR_REMOTE_CONNECTION_FAILED - If the factory tried to connect remotely
      *                                            to another machine and failed to connect.
      *      @li  gSKIERR_THREADING_MODEL_NOT_SUPPORTED - If the given threading model is not supported
      *                                                 by the kernel instance created by this factory.
      *
      * @param szInstanceName  Name to give to this kernel instance.  This name may be used
      *                         to identify this instance in a remote process.  Pass 0 if
      *                         you want a name auto-generated.
      * @param eTModel The threading model for the kernel instance to be created. 
      * @param ePType  The process model for the kernel instance to be created.  Pass
      *                      gSKI_ANY_PROCESS if you don't care what type of process is created.
      * @param szLocation The location of the machine on which to create the kernel instance.
      *                      This location parameter is factory specific, but is usually an IP address
      *                      or URL.  Pass 0 (the default) if you don't care what machine your kernel
      *                      is created on.
      * @param szLogLocation The path (on the server) relative to the server location where the
      *                            debug logs should be kept.  If this is 0, no debug log files are 
      *                            generated (though a client can still listen for log messages).  The
      *                            debug log location cannot be moved once the kernel is instantiated.
      * @param eLogActivity       The types of errors to log.  See IKernel::SetLogActivity for possible
      *                            values.
      * @param  err Pointer to client-owned error structure.  If the pointer
      *               is not 0 this structure is filled with extended error
      *               information.  If it is 0 (the default) extended error
      *               information is not returned.
	   *
      * @returns A pointer to an instance of a newly created kernel.  If the
	   *           creation process fails (e.g. a kernel instance already
	   *           exists and the factory does not support multi-instance
	   *           creation), the method returns 0.
      */
      IKernel* Create(const char*           szInstanceName     = 0,
                      egSKIThreadingModel   eTModel            = gSKI_MULTI_THREAD, 
                      egSKIProcessType      ePType             = gSKI_ANY_PROCESS, 
                      const char*           szLocation         = 0, 
                      const char*           szLogLocation = 0,
                      egSKILogActivityLevel eLogActivity       = gSKI_LOG_ERRORS,
                      Error*                err                = 0) const;

      /**
       * @brief Destroys one of the kernels held by this KernelFactory.
       *
       * @param krnl The kernel to be destroyed.
       */
      void DestroyKernel(IKernel *krnl, Error *err=0);

      /**
      * @brief Attaches this instance of gSKI to an existing instance
	   *           of the kernel.
	   *
	   * This method is typically used to connect a tool to a soar kernel
	   *  running out of process.
	   *
      * Possible Errors:
      *      @li  gSKIERR_REMOTE_CONNECTION_FAILED - If the factory tried to connect remotely
      *                                               to another machine and failed to connect.
      *      @li  gSKIERR_ATTACH_REJECTED          - If the server rejected the attempt to attach.
      *
	   * @param  pInstanceInfo  Pointer to the instance information of the
	   *          kernel instance to which  you wish to attach.  Cannot be 0.
      * @param  err Pointer to client-owned error structure.  If the pointer
      *               is not 0 this structure is filled with extended error
      *               information.  If it is 0 (the default) extended error
      *               information is not returned.
	   *
      * @returns A pointer to an instance of an already running kernel
	   *           a specified by pInstanceInfo.  If the attach fails
	   *           (e.g. the instance went away before this function call),
	   *           or if this factory does not support attaching to running
	   *           instances, the return value is 0.
      */
      IKernel* Attach(const IInstanceInfo* pInstanceInfo, Error* err = 0) const;

      
      /**
      * @brief Release this object
      * 
      * Call this method to remove this object from memory.  The pointer used to call Release
      *  will no longer be valid after the Release method returns.
      */
      bool Release(Error* err = 0);

      /**
       * @brief Required by the IRelease interface
       */
      // TODO: Properly implement and document this method
      bool IsClientOwned(Error* err = 0) const;

   private:
      /**
       * @brief: The Typedefs for a std vector of IInstanceInfo pointers.
       */
      typedef FwdContainerType< std::vector<IInstanceInfo * > >  tInstanceInfoVec;
      typedef FwdContainerType< std::vector< const IKernel *> >          tKernelVec;

      /**
       * @brief: The iterator that will hold the container of IInstanceInfo pointers.
       */
      typedef Iterator<tInstanceInfoVec::V, tInstanceInfoVec::t>  tInstanceInfoIter;

      static tInstanceInfoVec::t      m_instances;
      static tKernelVec::t            m_kernels;

   };
}
#endif
