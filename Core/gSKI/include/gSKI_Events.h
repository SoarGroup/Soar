/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file igski_events.h 
*********************************************************************
* created:	   6/20/2002   16:57
*
* purpose: 
*********************************************************************/

#ifndef IGSKI_EVENTS_HEADER
#define IGSKI_EVENTS_HEADER

#include "gSKI_Enumerations.h"
#include "IgSKI_Iterator.h"

namespace gSKI {

   /** Forward declarations of all listeners */
   class ISystemListener;
   class ILogListener;
   class IRhsFunctionChangeListener;
   class IRhsFunctionListener;
   class IConnectionLostListener;


   // No Longer Deprecated.  Used for Structured Output
   class IPrintListener;


/*********************** Interface definitions *******************************/


   class Agent;
   class Kernel;
   class IProduction;
   class IProductionInstance;

   /** 
   * @brief Print callback listener
   *
   *  The print callback is used for old Print callbacks and new
   *      structured output callbacks.
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
      virtual void HandleEvent(egSKIPrintEventId eventId, Agent* agentPtr, const char* msg) = 0;
   };

   /** 
   * @brief XML callback listener
   *
   *  The xml callback is used for xml trace generation callbacks.
   */
   class IXMLListener
   {
   public:
      /** Virtual destructor */
      virtual ~IXMLListener() {}

      /** 
      * @brief Event callback function
      *
      * This method recieves callbacks when the xml event occurs for an agent.
      *
      * @param eventId		Id of the event that occured (can only be gSKIEVENT_XML_TRACE_OUTPUT)
      * @param agentPtr		Pointer to the agent that fired the print event
      * @param funcType     Pointer to c-style string containing the function type (i.e. addTag, addAttributeValuePair, endTag)
	  * @param attOrTag     Pointer to c-style string containing the tag to add or remove or the attribute to add
	  * @param value		Pointer to c-style string containing the value to add (may be NULL if just adding/ending a tag)
      */
      virtual void HandleEvent(egSKIXMLEventId eventId, Agent* agentPtr, const char* funcType, const char* attOrTag, const char* value) = 0;
   };

   /** 
   * @brief Working memory callback listener
   *
   * Called when events occur that change working memory.
   *
   */
   class IWorkingMemoryListener
   {
   public:
      /** Virtual destructor */
      virtual ~IWorkingMemoryListener() {}

      /** 
      * @brief Event callback function
      *
      * This method recieves callbacks when a working memory event occurs for an agent.
      *
      * @param eventId  Id of the event that occured
      * @param agentPtr Pointer to the agent that fired the event
	  * @param change	The type of change that just occured
      * @param wmelist  Pointer to list of wmes that are affected by this event
      */
      virtual void HandleEvent(egSKIWorkingMemoryEventId eventId, Agent* agentPtr, egSKIWorkingMemoryChange change, tIWmeIterator* wmelist) = 0;
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
      virtual void HandleEvent(egSKIRunEventId   eventId, 
                               Agent*        agentPtr, 
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
      virtual void HandleEvent(egSKIProductionEventId eventId, 
                               Agent*                agentPtr, 
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
      virtual void HandleEvent(egSKIAgentEventId      eventId, 
                               Agent*                agentPtr) = 0;
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
      virtual void HandleEvent(egSKISystemEventId eventId, Kernel* kernel) = 0;
   };


	class IRhsListener
	{
	public:
		/** Virtual destructor */
		virtual ~IRhsListener() {}

		// Handler for RHS (right hand side) function firings
		// pFunctionName and pArgument define the RHS function being called (the client may parse pArgument to extract other values)
		// commandLine is true if we expect this to be handled by the command line processor (false => a custom RHS function that the user provides)
		// pResultValue is a string allocated by the caller than is of size maxLengthReturnValue that should be filled in with the return value.
		// The bool return value should be "true" if a return value is filled in, otherwise return false.
		virtual bool HandleEvent(egSKIRhsEventId eventId, Agent* pAgent, bool commandLine, char const* pFunctionName, char const* pArgument,
								 int maxLengthReturnValue, char* pReturnValue) = 0;
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
         virtual void HandleEvent(egSKIPrintEventId  eventID, 
                                  Kernel*      kernel, 
                                  const char*   msg) = 0;
   };
}

#endif
