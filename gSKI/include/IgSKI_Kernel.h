/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file igski_kernel.h 
*********************************************************************
* created:         6/11/2002   16:57
*
* purpose: 
*********************************************************************/

#ifndef IGSKI_KERNEL_H
#define IGSKI_KERNEL_H

#include "gSKI_Enumerations.h"
#include "IgSKI_Iterator.h"
#include "IgSKI_DoNotTouch.h"
#include "gSKI_Events.h"

namespace TgD {
  class TgD;
}

namespace cli {
	class CommandLineInterface;
}

namespace gSKI {

  class IAgentManager;
  class IPerformanceMonitor;
  class IKernelFactory;
  class IInstanceInfo;
  class ISymbolFactory;
  class ISymbol;
  struct Error;

  /** 
   * @brief This class encapsulates Kernel Functionality.
   *
   * The kernel represents the entire Soar system.  It is the central access
   *  point for all functionality of the soar system.  It provides access to
   *  the AgentManager, Performance Monitor, RHS functions and debug logging.
   *  It also serves as an event source for system-wide events.
   *
   * Events Fired:
   *   <ul>
   *     <li> SYSTEM EVENTS
   *     <ul>
   *         <li> gSKIEVENT_SHUTDOWN
   *         <li> gSKIEVENT_RESTART
   *     </ul>
   *     <li> DEBUG LOG EVENTS
   *     <ul>
   *           <li> gSKIEVENT_LOG_ERROR
   *           <li> gSKIEVENT_LOG_WARNING
   *           <li> gSKIEVENT_LOG_INFO
   *           <li> gSKIEVENT_LOG_DEBUG
   *     </ul>
   *     <li> RHS FUNCTION CHANGE EVENTS
   *     <ul>
   *           <li> gSKIEVENT_RHS_FUNCTION_ADDED
   *           <li> gSKIEVENT_RHS_FUNCTION_REMOVED
   *     </ul>
   *     <li> RHS FUNCTION EVENTS
   *     <ul>
   *           <li> gSKIEVENT_RHS_FUNCTION_EXECUTED
   *     </ul>
   *     <ul>
   *           <li> gSKIEVENT_CONNECTION_LOST
   *     </ul>
   *   </ul>
   */
  class IKernel
    {
    public:

      /**
       * @brief Destructor for the kernel.
       * 
       * The destructor for the kernel makes sure everything held by the kernel is
       *  cleaned up (including agents).  It will not clean up client owned elements
       *  like cloned WMEs and KernelFactories
       */
       virtual ~IKernel() {}

      /**
       * @brief  Returns the agent manager for this kernel.
       *
       * Use the agent manager to add and remove agents as well as 
       *  set up the agents to run, etc.
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       * @return A pointer to the agent manager.  The pointer may be 0 if there
       *          is a failure.
       */
      virtual IAgentManager* GetAgentManager(Error* err = 0) = 0;

      /**
       * @brief Returns the performance monitor for this kernel
       *
       * Use the performance monitor to look at timing and memory usage by 
       *  the kernel and its agents.
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       * @returns A pointer to the performance monitor for this kernel.  This pointer
       *           may be 0 if there is a failure.
       */
      /* TODO: This interface needs to be defined */
      virtual IPerformanceMonitor* GetPerformanceMonitor(Error* err = 0) = 0;

      /**
       * @brief Returns a pointer to the Kernel Factory that created it.
       *
       * @returns The pointer to the creating Kernel Factory.
       */
      virtual const IKernelFactory* GetKernelFactory(Error* err = 0)const = 0;


      /**
       * @brief Returns the instance information for this instance of the kernel
       *
       * Use the instance information to discover the threading model, process model
       *  and other instance information for this instance of the kernel.
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       * @returns A pointer to an InstanceInfo object containing information on 
       *           this kernel instance.  This pointer may be 0 if there is
       *           a failure.
       */
      virtual IInstanceInfo* GetInstanceInformation(Error* err = 0) = 0;

      /**
       * @brief    Get the current log location
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       * @returns  A c-style string with the path where debug logs are being kept.
       *            The format of the path follows the format of file paths on the server
       *            machine.  If there are no debug logs or an unrecoverable
       *            error occurs, 0 is returned.
       */
      virtual const char* GetLogLocation(Error* err = 0) const = 0;

      /**
       * @brief    Sets the activity level of the debug log output
       *
       * The following are all possible debug log activity settings:
       *
       *  @li gSKI_LOG_NONE     : No logs are generated
       *  @li gSKI_LOG_ERRORS   : Only errors are logged
       *  @li gSKI_LOG_ALL      : Everything is logged (errors, warnings, info, debug)
       *  @li gSKI_LOG_ALL_EXCEPT_DEBUG : Everything except debug-only messages are logged
       *
       * The debug logs files are named as follows:
       *     @li gSKIError.txt   : for errors (critical problems)
       *     @li gSKIWarning.txt : for warnings (non-critical problems)
       *     @li gSKIInfo.txt    : for general information
       *     @li gSKIDebug.txt   : for debug information
       *
       * @param aLevel The desired debug log activity level.  This setting will effect
       *                  both the output file logs and the debug log messages routed
       *                  through the event system.
       * @param  err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       */
      virtual void SetLogActivityLevel(egSKILogActivityLevel aLevel,
                                            Error*                err = 0) = 0;

      /**
       * @brief Gets the current debug log activity level
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       * @returns The current debug log activity level.  If there is a connection
       *            failure, the return value is undefined.
       */
      virtual egSKILogActivityLevel GetLogActivityLevel(Error* err) const = 0;

      /********************** LISTENERS ***********************************/

      /**
      *  @brief Adds a listener for system events
      *
      *  Call this method to register a listener to recieve system events.
      *  System events are:
      *     @li gSKIEVENT_SHUTDOWN
      *     @li gSKIEVENT_RESTART
      *
      *  If this listener has already been added for the given event, nothing happens
      *
      *  Possible Errors:
      *     @li gSKIERR_INVALID_PTR -- If you pass an invalid pointer for a listener.
      *
      *  @param eventId  One of the valid event ids listed above
      *  @param listener A pointer to a client owned listener that will be called back when
      *                      an event occurs.  Because the listener is client owned, it is not
      *                      cleaned up by the kernel when it shuts down.  The same listener
      *                      can be registered to recieve multiple events.  If this listener
      *                      is 0, no listener is added and an error is recored in err.
      *  @param allowAsynch A flag indicating whether or not it is ok for the listener to be
      *                         notified asynchonously of system operation.  If you specify "true"
      *                         the system may not callback the listener until some time after
      *                         the event occurs. This flag is only a hint, the system may callback
      *                         your listener synchronously.  The main purpose of this flag is to
      *                         allow for efficient out-of-process implementation of event callbacks
      *  @param  err Pointer to client-owned error structure.  If the pointer
      *               is not 0 this structure is filled with extended error
      *               information.  If it is 0 (the default) extended error
      *               information is not returned.
      */
      virtual void AddSystemListener(egSKIEventId        eventId, 
                                     ISystemListener*    listener, 
                                     bool                allowAsynch = false,
                                     Error*              err         = 0) = 0;

      /**
      *  @brief Removes a system event listener
      *
      *  Call this method to remove a previously added event listener.
      *  The system will automatically remove all listeners when the kernel shutsdown;
      *   however, since all listeners are client owned, the client is responsible for
      *   cleaning up memory used by listeners.
      *
      *  If the given listener is not registered to recieve the given event, this
      *     function will do nothing (but a warning is logged).
      *
      *  System events are:
      *     @li gSKIEVENT_SHUTDOWN
      *     @li gSKIEVENT_RESTART
      *
      *  Possible Errors:
      *     @li gSKIERR_INVALID_PTR -- If you pass an invalid pointer for a listener.
      *
      *  @param eventId  One of the valid event ids listed above
      *  @param listener A pointer to the listener you would like to remove.  Passing a 0
      *                     pointer causes nothing to happen except an error being recorded
      *                     to err.
      *  @param  err Pointer to client-owned error structure.  If the pointer
      *               is not 0 this structure is filled with extended error
      *               information.  If it is 0 (the default) extended error
      *               information is not returned.
      */
      virtual void RemoveSystemListener(egSKIEventId         eventId,
                                        ISystemListener*     listener,
                                        Error*               err = 0) = 0;

     /**
      *  @brief Adds a listener for debug log events
      *
      *  Call this method to register a listener to recieve debug log events.
      *  Debug Log events are:
      *     @li gSKIEVENT_LOG_ERROR
      *     @li gSKIEVENT_LOG_WARNING
      *     @li gSKIEVENT_LOG_INFO
      *     @li gSKIEVENT_LOG_DEBUG
      *
      *  If this listener has already been added for the given event, nothing happens
      *
      *  Possible Errors:
      *    @li gSKIERR_INVALID_PTR -- If you pass an invalid pointer for a listener.
      *
      *  @param eventId  One of the valid system ids listed above
      *  @param listener A pointer to a client owned listener that will be called back when
      *                      an event occurs.  Because the listener is client owned, it is not
      *                      cleaned up by the kernel when it shuts down.  The same listener
      *                      can be registered to recieve multiple events.  If this listener
      *                      is 0, no listener is added and an error is recored in err.
      *  @param allowAsynch A flag indicating whether or not it is ok for the listener to be
      *                         notified asynchonously of system operation.  If you specify "true"
      *                         the system may not callback the listener until some time after
      *                         the event occurs. This flag is only a hint, the system may callback
      *                         your listener synchronously.  The main purpose of this flag is to
      *                         allow for efficient out-of-process implementation of event callbacks
      *  @param  err Pointer to client-owned error structure.  If the pointer
      *               is not 0 this structure is filled with extended error
      *               information.  If it is 0 (the default) extended error
      *               information is not returned.
       */
      virtual void AddLogListener(egSKIEventId   eventId, 
                                  ILogListener*  listener, 
                                  bool           allowAsynch = false,
                                  Error*         err         = 0) = 0;
   
      /**
      *  @brief Removes a debug log event listener
      *
      *  Call this method to remove a previously added event listener.
      *  The system will automatically remove all listeners when the kernel shutsdown;
      *   however, since all listeners are client owned, the client is responsible for
      *   cleaning up memory used by listeners.
      *
      *  If the given listener is not registered to recieve the given event, this
      *     function will do nothing (but a warning is logged).
      *
      *  Debug Log events are:
      *     @li gSKIEVENT_LOG_ERROR
      *     @li gSKIEVENT_LOG_WARNING
      *     @li gSKIEVENT_LOG_INFO
      *     @li gSKIEVENT_LOG_DEBUG
      *
      *  Possible Errors:
      *     @li gSKIERR_INVALID_PTR -- If you pass an invalid pointer for a listener.
      *
      *  @param eventId  One of the valid event ids listed above
      *  @param listener A pointer to the listener you would like to remove.  Passing a 0
      *                     pointer causes nothing to happen except an error being recorded
      *                     to err.
      *  @param  err Pointer to client-owned error structure.  If the pointer
      *               is not 0 this structure is filled with extended error
      *               information.  If it is 0 (the default) extended error
      *               information is not returned.
      */
      virtual void RemoveLogListener(egSKIEventId   eventId,
                                     ILogListener*  listener,
                                     Error*         err = 0) = 0;

      /**
      *  @brief Adds a listener for connection lost events
      *
      *  Call this method to register a listener to recieve connection lost events.
      *  Connection lost events are:
      *     @li gSKIEVENT_CONNECTION_LOST
      *
      *  Connections can only be lost with out-of-process communications.  You can use
      *   this callback to do whatever you need to do in the client to recover from
      *   a lost connection.
      *
      *  If this listener has already been added for the given event, nothing happens
      *
      *  Possible Errors:
      *    @li gSKIERR_INVALID_PTR -- If you pass an invalid pointer for a listener.
      *
      *  @param eventId  One of the valid system ids listed above
      *  @param listener A pointer to a client owned listener that will be called back when
      *                      an event occurs.  Because the listener is client owned, it is not
      *                      cleaned up by the kernel when it shuts down.  The same listener
      *                      can be registered to recieve multiple events.  If this listener
      *                      is 0, no listener is added and an error is recored in err.
      *  @param allowAsynch A flag indicating whether or not it is ok for the listener to be
      *                         notified asynchonously of system operation.  If you specify "true"
      *                         the system may not callback the listener until some time after
      *                         the event occurs. This flag is only a hint, the system may callback
      *                         your listener synchronously.  The main purpose of this flag is to
      *                         allow for efficient out-of-process implementation of event callbacks
      *  @param  err Pointer to client-owned error structure.  If the pointer
      *               is not 0 this structure is filled with extended error
      *               information.  If it is 0 (the default) extended error
      *               information is not returned.
       */
      virtual void AddConnectionLostListener(egSKIEventId             eventId, 
                                             IConnectionLostListener* listener, 
                                             bool                     allowAsynch = false,
                                             Error*                   err         = 0) = 0;

   
      /**
      *  @brief Removes a connection lost event listener
      *
      *  Call this method to remove a previously added event listener.
      *  The system will automatically remove all listeners when the kernel shutsdown;
      *   however, since all listeners are client owned, the client is responsible for
      *   cleaning up memory used by listeners.
      *
      *  If the given listener is not registered to recieve the given event, this
      *     function will do nothing (but a warning is logged).
      *
      *  Connection lost events are:
      *     @li gSKIEVENT_CONNECTION_LOST
      *
      *  Possible Errors:
      *     @li gSKIERR_INVALID_PTR -- If you pass an invalid pointer for a listener.
      *
      *  @param eventId  One of the valid event ids listed above
      *  @param listener A pointer to the listener you would like to remove.  Passing a 0
      *                     pointer causes nothing to happen except an error being recorded
      *                     to err.
      *  @param  err Pointer to client-owned error structure.  If the pointer
      *               is not 0 this structure is filled with extended error
      *               information.  If it is 0 (the default) extended error
      *               information is not returned.
      */
      virtual void RemoveConnectionLostListener(egSKIEventId             eventId,
                                                IConnectionLostListener* listener,
                                                Error*                   err = 0) = 0;

      private:
         //
         // We are adding an interface here to allow for some workarounds
         // that are used by a preliminary Tcl gSKI Debugger Interface.
         //

      virtual EvilBackDoor::ITgDWorkArounds* getWorkaroundObject() = 0;

      friend class TgD::TgD;
	  friend class cli::CommandLineInterface;

   };

}

#endif
