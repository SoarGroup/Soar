/********************************************************************
* @file igski_events.h 
*********************************************************************
* @remarks Copyright (C) 2002 Soar Technology, All rights reserved. 
* The U.S. government has non-exclusive license to this software 
* for government purposes. 
*********************************************************************
* created:	   6/20/2002   16:57
*
* purpose: 
*********************************************************************/

#ifndef IGSKI_EVENTS_HEADER
#define IGSKI_EVENTS_HEADER

#include "gSKI_Enumerations.h"

namespace gSKI {

   /** Forward declarations of all listeners */
   class ISystemListener;
   class ILogListener;
   class IRhsFunctionChangeListener;
   class IRhsFunctionListener;
   class IConnectionLostListener;


   // Deprecated
   class IPrintListener;


/*********************** Interface definitions *******************************/


   class IAgent;
   class IKernel;
   class IProduction;
   class IProductionInstance;

   /** 
   * @brief Print callback listener
   *
   * The print callback is deprecated.  If you don't know what it is or how
   *  to use it, don't use it.
   */
   class IPrintListener
   {
   public:
      /** Virtual destructor */
      virtual ~IPrintListener() {}

      /** 
      * @brief Event callback function
      *
      * This method recieves callbacks when the print event occurs for an agent.
      *
      * @param eventId  Id of the event that occured (can only be gSKIEVENT_PRINT)
      * @param agentPtr Pointer to the agent that fired the print event
      * @param msg      Pointer to c-style string containing the print text
      */
      virtual void HandleEvent(egSKIEventId eventId, IAgent* agentPtr, const char* msg) = 0;
   };

   /** 
    * @brief Listener for run events for an agent.
    *
    * Derive from this listener if you wish to intercept events related to
    *  running an agent.  
    *
    */
   class IRunListener
   {
   public:
      /** Virtual destructor */
      virtual ~IRunListener() {}

      /** 
       * @brief Called back on a run event
       *
       * Implement your event handling code in the derived version of this method.
       *
       * @param eventId  Id of the event that occured
       * @param agentPtr Pointer to the agent for which the run event occured
       * @param phase    The run phase to which the event applies.
       */
      virtual void HandleEvent(egSKIEventId   eventId, 
                               IAgent*        agentPtr, 
                               egSKIPhaseType phase) = 0;
   };

   /** 
    * @brief Listener for production events.
    *
    * Derive from this class to create an object that can listen to and
    *  respond to production events such as production addded or fired.
    */
   class IProductionListener
   {
   public:
      /** Virtual destructor */
      virtual ~IProductionListener() {}

      /** 
       * @brief Called back on a production event
       *
       * Implement your event handling code in the derived version of this method
       *
       * @param eventId  Id of the event that occured
       * @param agentPtr Pointer to the agent for which the production event occured
       * @param prod     The production to which the event applies
       * @param match    The match related to the event (this is 0 for production
       *                    added and removed events).
       */
      virtual void HandleEvent(egSKIEventId           eventId, 
                               IAgent*                agentPtr, 
                               IProduction*           prod,
                               IProductionInstance*   match) = 0;
   };

   /** 
    * @brief Listener for agent events
    *
    * Agent events are registered with the agent manager.  They include
    *  agent creation, destruction and initialization.
    */
   class IAgentListener
   {
   public:
      /** Virtual destructor */
      virtual ~IAgentListener() {}

      /** 
       * @brief Called back on an agent event
       *
       * Implement your event handling code in the derived version of this method
       *
       * @param eventId  Id of the event that occured
       * @param agentPtr Pointer to the agent for which the event occured
       */
      virtual void HandleEvent(egSKIEventId           eventId, 
                               IAgent*                agentPtr) = 0;
   };

   /** 
    *
    */
   class ISystemListener
   {
   public:
      /** Virtual destructor */
      virtual ~ISystemListener() {}

      /** 
       *
       */
      virtual void HandleEvent(egSKIEventId eventId, IKernel* kernel) = 0;
   };


   /**
    * @brief: 
    */
   class ILogListener
   {
   public:
      /** Virtual destructor */
      virtual ~ILogListener() {}

         /**
          * @brief: 
          */
         virtual void HandleEvent(egSKIEventId  eventID, 
                                  IKernel*      kernel, 
                                  const char*   msg) = 0;
   };
}

#endif
