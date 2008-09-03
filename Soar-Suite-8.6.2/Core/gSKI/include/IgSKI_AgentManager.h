/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file igski_agentmanager.h 
*********************************************************************
* created:	   6/13/2002   14:50
*
* purpose: 
*********************************************************************/
#ifndef IGSKI_AGENTMANAGER_H
#define IGSKI_AGENTMANAGER_H

#include "gSKI_Enumerations.h"
#include "gSKI_Events.h"
#include "IgSKI_Iterator.h"

namespace gSKI {

   class IAgent;
   class IAgentThreadGroup;
   class Kernel;
   struct Error;

   /**
   * @brief Interface for the Soar agent manager
   *
   * The agent manager is responsible for creating, destroying
   *  and managing agents.  It is also responsible for creating
   *  and managing agent thread groups.  Thread groups are used
   *  to manage running the agents.
   *
   * @see IAgentRunControl
   * @see IAgentThreadGroup
   */
   class IAgentManager
   {
   public:
     
      /** 
       * @brief Destructor for the agent manager
       *
       * The agent manager destructor ensures that all agents are destroyed
       *  before the agent manager goes away.
       */
      virtual ~IAgentManager() {}


      /**
      * @brief  Adds an agent to the agent manager.
      *
      * After an agent is added to the agent manager, it is available for
      *  manipulation by the client.  A newly added agent is inactive, you must
      *  initialize it before it is activated.  If the agents are being run
      *  as a group (through IAgentManager::RunInClientThread or 
      *  IAgentManager::RunInSeparateThread, the agent is put into the
      *  running thread group and will begin running after being initialized.
      * 
      * Possible Errors:
      *   @li gSKIERR_INVALID_PTR if name is 0
      *   @li gSKIERR_AGENT_EXISTS if an agent by the given name already exists
      *   @li Any error returned by IProductionManager::LoadSoarFile if a production filename
      *         is supplied and the file doesn't exist or is somehow not usable (e.g. bad format)
      *
      * @param name Name of the Agent to add.  This name must be unique for this instance of the
      *              agent manager.  The agent names are used as the agent ids.
      * @param prodFileName (Optional) Name of a soar production file to load with the agent.  
      *                      If you don't want to load a production file with the agent, set
      *                      this parameter to 0.
      * @param learningOn  Pass true to initialize learning to on for this agent
      *                     Pass false to disable learning for this agent.  You
      *                     can change the learning setting after the agent
      *                     is created (even while it is running).
      * @param  oSupportMode O support mode to use for this agent.  The o-support
      *                      mode determines how o-support is calculated for
      *                      wmes.  See egSKIOSupportMode for mode details.
      * @param  err Pointer to client-owned error structure.  If the pointer
      *               is not 0 this structure is filled with extended error
      *               information.  If it is 0 (the default) extended error
      *               information is not returned.
      *
      * @returns A pointer to the agent that was added.  This pointer will be 0 if
      *           the function fails.
      */
      virtual IAgent* AddAgent(const char*       name, 
                               const char*       prodFileName = 0, 
                               bool              learningOn   = false,
                               egSKIOSupportMode oSupportMode = gSKI_O_SUPPORT_MODE_4,
                               Error*            err          = 0) = 0;

      /**
      * @brief Removes an agent from this agent manager
      *
      * Call this method to remove an agent from the agent manager.  Removing
      *  an agent causes it to be destroyed.  Thus, any object refering to
      *  the destroyed agent will be holding an invalid pointer.
      *
      * The agent is not actually removed until the end of a phase if it is currently
      *  in the middle of a decision cycle.
      *
      * Possible Errors:
      *   @li gSKIERR_INVALID_PTR if the given agent ptr is invalid
      *   @li gSKIERR_AGENT_DOES_NOT_EXIST if the given agent pointer points to an
      *           agent that is not managed by this agent manager.
      *
      * @param agent Pointer to the agent to remove. 
      * @param  err Pointer to client-owned error structure.  If the pointer
      *               is not 0 this structure is filled with extended error
      *               information.  If it is 0 (the default) extended error
      *               information is not returned.
      *
      */
      virtual void RemoveAgent(IAgent* agent, Error* err = 0) = 0;

      /**
      * @brief Removes an agent from this agent manager given the agent's name.
      *
      * Call this method to remove an agent from the agent manager using only 
      *  its name.  See IAgentManager::RemoveAgent for details.
      *
      * @see IAgentManager::RemoveAgent
      *
      * @param  name Name of the agent to remove.  
      * @param  err Pointer to client-owned error structure.  If the pointer
      *               is not 0 this structure is filled with extended error
      *               information.  If it is 0 (the default) extended error
      *               information is not returned.
      *
      */
      virtual void RemoveAgentByName(const char* name, Error* err = 0) = 0;

      /**
      * @brief Retrieves a pointer to the agent with the given name.
      *
      * Call this method to obtain a pointer to an agent when only its
      *  name is available.
      *
      * Possible Errors:
      *   @li gSKIERR_INVALID_PTR if name is an invalid pointer
      *   @li gSKIERR_AGENT_DOES_NOT_EXIST if name is the name of an agent that
      *          this agent manager is not currently managing.
      *
      * @param name Name of the agent to retrieve.  
      * @param  err Pointer to client-owned error structure.  If the pointer
      *               is not 0 this structure is filled with extended error
      *               information.  If it is 0 (the default) extended error
      *               information is not returned.
      *
      * @returns A pointer to the agent with the given name managed by this AgentManager.
      *           If such an agent doesn't exist, the pointer returned is 0.
      */
      virtual IAgent* GetAgent(const char* name, Error* err = 0) = 0;

      /**
      * @brief Retrieves an iterator to all of the agents currently managed by this AgentManager
      *
      * Call this method when you wish to iterate over all agents or need to do a manual 
      *  search accross the agents
      *
      * @param  err Pointer to client-owned error structure.  If the pointer
      *               is not 0 this structure is filled with extended error
      *               information.  If it is 0 (the default) extended error
      *               information is not returned.
      * 
      * @returns A pointer to an iterator to the agents.  All iterators are client
      *           owned and must be cleaned up by the client using Destroy().
      */
      virtual tIAgentIterator* GetAgentIterator(Error* err = 0) = 0;

     /** 
      * @brief Initializes all of the agents managed by this object
      *
      * This method calls the IAgent::ReinitializeWithOldSettings method on
      *  all of the agents managed by this object.  It will not return until
      *  all agents in this thread group have been reinitialized.
      *
      * This call will stop the entire thread group (if it is running)
      *  at the next valid stop time (see IAgentRunControl::Stop).  
      *
      * @see IAgent::ReinitializeWithOldSettings
      *
      * Possible Errors:
      *   @li Any errors that can be generated by ReinitializeWithOldSettings
      *
      * @param  err Pointer to client-owned error structure.  If the pointer
      *               is not 0 this structure is filled with extended error
      *               information.  If it is 0 (the default) extended error
      *               information is not returned.
      *
      * @return true if all agents were reinitialized successfully, false 
      *           if there was a problem reinitializing one or more agents.
      */
      virtual bool    ReinitializeAll(Error* err = 0) = 0;

            /** 
       * @brief Adds an agent to the list of agents run when a RunXXX function is called
       *
       * Call this method to add an agent to the list of agents to run when
       *  IAgentManager::RunInClientThread or IAgentManager::RunInSeparateThread 
       *  are called.  
       *
       * If the agent manager is currently running agents in the run list, the
       *  given agent will be run at the next interleave step.
       *
       * If the given agent is already part of the runlist, this method leaves
       *  it as part of the run list.
       *
       * @param agentToAdd Pointer to the agent to add to the run list for
       *         this agent manager.
       * @param  err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       */
      virtual void    AddAgentToRunList(IAgent* agentToAdd, Error* err = 0) = 0;

      /** 
       * @brief Adds all agents to the list of agents run when a RunXXX function is called
       *
       * Call this method to add all agents to the list of agents to run when
       *  IAgentManager::RunInClientThread or IAgentManager::RunInSeparateThread 
       *  are called.  
       *
       * If the agent manager is currently running agents in the run list, the
       *  agents previously not in the list will be run at the next interleave step.
       *
       * If the any agent is already part of the runlist, this method leaves
       *  it as part of the run list.
       *
       * @param agentToAdd Pointer to the agent to add to the run list for
       *         this agent manager.
       * @param  err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       */
      virtual void    AddAllAgentsToRunList(Error* err = 0) = 0;

      /** 
       * @brief Removes an agent from the list of agents run when a RunXXX function is called
       *
       * Call this method to remove an agent from the list of agents to run when
       *  IAgentManager::RunInClientThread or IAgentManager::RunInSeparateThread 
       *  are called.  Agents are automatically removed from the run list when
       *  they are removed from the agent manager.
       *
       * If the agent manager is currently running agents in the run list, the
       *  given agent will be removed at the next interleave step.
       *
       * If the given agent is not part of the run list, nothing happens.
       *
       * @param agentToAdd Pointer to the agent to add to the run list for
       *         this agent manager.
       * @param  err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       */
      virtual void      RemoveAgentFromRunList(IAgent* agentToAdd, Error* err = 0) = 0;

      /** 
       * @brief Removes all agents from this manager's run list
       *
       * Call this method to remove all agents from the list of agents to run when
       *  IAgentManager::RunInClientThread or IAgentManager::RunInSeparateThread 
       *  are called.  Agents are automatically removed from the run list when
       *  they are removed from the agent manager.
       *
       * If the agent manager is currently running agents in the run list, the
       *  given agent will be removed at the next interleave step.
       *
       * If the given agent is not part of the run list, nothing happens.
       *
       * @param agentToAdd Pointer to the agent to add to the run list for
       *         this agent manager.
       * @param  err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       */
      virtual void           RemoveAllAgentsFromRunList(Error* err = 0) = 0;

      /**
       * @brief Runs all agents currently in the agent manager run list
       *
       * This method creates a separate thread and runs all of the agents
       *  this manager has in its run list in that thread.
       *
       * Already running and halted agents are taken out of the agent manager's
       *  run list automatically when this method tries to run them.
       *
       * This method returns immediately after starting the agents running.
       *  It does not block until run completion.
       *
       * Possible Errors:
       *   @li gSKIERR_NO_AGENTS_TO_RUN if the run list for this agent manager
       *                 does not contain any agents.
       *
       * @see egSKIRunType
       * @see IAgentManager::AddAgentToRunList
       * @see IAgentManager::AddAllAgentsToRunList
       * @see IAgentManager::RemoveAgentFromRunList
       *
       * @param runLength How long to run the system.  Choices are       
       *          gSKI_RUN_SMALLEST_STEP, gSKI_RUN_PHASE, gSKI_RUN_ELABORATION_PHASE,
       *          gSKI_RUN_DECISION_CYCLE, gSKI_RUN_UNTIL_OUTPUT, and
       *          gSKI_RUN_FOREVER.  See egSKIRunType for details.
       * @param  count For gSKI_RUN_SMALLEST_STEP, gSKI_RUN_ELABORATION_PHASE, gSKI_RUN_PHASE,
       *          and gSKI_RUN_DECISION_CYCLE this parameter tells the method
       *          how many elaboration phases, decision phase or decision cycles
       *          to run before the thread groups return. For other run types
       *          this parameter is ignored.
       * @param runInterleave How to interleave agent execution.  Choices are
       *          gSKI_INTERLEAVE_ELAB_PHASE, gSKI_INTERLEAVE_DECISION_PHASE
       *          gSKI_INTERLEAVE_DECISION_CYCLE, and gSKI_INTERLEAVE_OUTPUT.
       *          See egSKIInterleaveType for more details.
       * @param  err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       * @return An egSKIRunResult value indicating how the state of the system
       *            after continuation executed. @see egSKIRunResult  This method
       *            will never return gSKI_RUN_INTERRUPTED or 
       *            gSKI_RUN_COMPLETED_AND_INTERRUPTED because it returns before
       *            any interrupt can occur.
       */
      virtual egSKIRunResult RunInSeparateThread(egSKIRunType        runLength     = gSKI_RUN_FOREVER, 
                                            unsigned long       count         = 1,
                                            egSKIInterleaveType runInterleave = gSKI_INTERLEAVE_SMALLEST_STEP,
                                            Error*              err           = 0) = 0;

      /** 
       * @brief Runs all of the agents in the run list together in round-robin fashion.
       *
       * Call this method to execute all of the agents currently in the agent
       *  manager together in the client thread. 
       *
       * Already running and halted agents are taken out of the agent manager's
       *  run list automatically when this method tries to run them.
       *  
       * Possible Errors:
       *   @li gSKIERR_NO_AGENTS_TO_RUN if the run list for this agent manager
       *                 does not contain any agents.
       *
       * @see egSKIRunType
       * @see IAgentManager::AddAgentToRunList
       * @see IAgentManager::AddAllAgentsToRunList
       * @see IAgentManager::RemoveAgentFromRunList
       *
       * @param runLength How long to run the system.  Choices are       
       *          gSKI_RUN_SMALLEST_STEP, gSKI_RUN_ELABORATION_PHASE, gSKI_RUN_PHASE,
       *          gSKI_RUN_DECISION_CYCLE, gSKI_RUN_UNTIL_OUTPUT, and
       *          gSKI_RUN_FOREVER.  See egSKIRunType for details.
       * @param  count For gSKI_RUN_SMALLEST_STEP, gSKI_RUN_PHASE, gSKI_RUN_ELABORATION_PHASE
       *          and gSKI_RUN_DECISION_CYCLE this parameter tells the method
       *          how many elaboration phases, decision phase or decision cycles
       *          to run before returning. For other run types this parameter
       *          is ignored.
       * @param runInterleave How to interleave agent execution.  Choices are
       *          gSKI_INTERLEAVE_ELAB_PHASE, gSKI_INTERLEAVE_DECISION_PHASE
       *          gSKI_INTERLEAVE_DECISION_CYCLE, and gSKI_INTERLEAVE_OUTPUT.
       *          See egSKIInterleaveType for more details.
       * @param  err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       * @return An egSKIRunResult value indicating how the state of the system
       *            after continuation executed. @see egSKIRunResult
       */
      virtual egSKIRunResult RunInClientThread(egSKIRunType        runLength     = gSKI_RUN_FOREVER, 
                                       unsigned long       count         = 1,
                                       egSKIInterleaveType runInterleave = gSKI_INTERLEAVE_SMALLEST_STEP,
                                       Error*              err           = 0) = 0;

      /**
       * @brief Interrupts all agent execution
       *
       * Call this method to interrupt agent execution.  If the agent is not running,
       *  nothing happens.
       *
       * This method can be used to interrupt all agents no matter how they were
       *  initially run (either as part of the run list or singly).
       *
       * Agents do not stop immediately upon being notified to interrupt. They stop
       *  at one of the safe stopping points listed in the egSKIStopLocation 
       *  enumeration.  Essentially this Interrupt method is a request to the agents
       *  to stop processing.  It will stop when it gets a chance.  
       *
       * This method always stops the agents by returning.  You cannot suspend all
       *  agents simultaneously with this method.  Allowing that functionality
       *  would cause problems with multiple agents running in the same thread.
       *
       * Possible Errors:
       *   @li gSKIERR_CANNOT_STOP_FOR_CALLBACKS if you specify 
       *           gSKI_STOP_ON_CALLBACK_RETURN or gSKI_STOP_AFTER_ALL_CALLBACKS_RETURN.
       *           This method does not support stopping for callbacks.
       * 
       * @see egSKIStopLocation
       * @see egSKIStopType
       *
       * @param stopLoc Where to interrupt agent execution.  Possible locations are
       *         gSKI_STOP_ON_CALLBACK_RETURN, gSKI_STOP_AFTER_ALL_CALLBACKS_RETURN,
       *         gSKI_STOP_AFTER_CURRENT_PHASE, gSKI_STOP_NEXT_ELABORATION_CYCLE,
       *         gSKI_STOP_NEXT_PHASE, and gSKI_STOP_NEXT_DECISION_CYCLE.
       *         See egSKIStopLocation for more details.
       * @param  err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       * @return true if the agent could be stopped.  false if there was an
       *           error preventing it from being stopped.  See err for 
       *           detailed error information.
       */
      virtual bool InterruptAll(egSKIStopLocation    stopLoc, 
                        Error*               err   = 0) = 0;

      /** 
       * @brief Continues agent execution at the point at which it left off
       *          when Interrupt was last called.
       *
       * Call this method to clear interrupts for all agents that this agent
       *  manager manages (those in and out of the run list).  This method
       *  will return immediately after clearing the interrupts. If an agent
       *  was interrupted by suspending its execution thread, it will continue
       *  running, otherwise, each cleared agent is set to the stop state.
       *
       * This method does NOT generate an error if any agent is already running
       *  or stopped.
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       */
      virtual void ClearAllInterrupts(Error* err = 0) = 0;

      /** 
       * @brief Halts all agents managed by this agent manager
       *
       * Call this method to stop execution permanently for all agents in this
       *  agent manager.  You must reinitialize the agents before they can
       *  be run again.
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       */
      virtual void HaltAll(Error* err = 0) = 0;

      /*************************** Listeners *********************************/

      /**
      *  @brief Adds a listener for agent events
      *
      *  Call this method to register a listener to recieve agent events.
      *  Agent events are:
      *     @li gSKIEVENT_AFTER_AGENT_CREATED
      *     @li gSKIEVENT_BEFORE_AGENT_DESTROYED
      *     @li gSKIEVENT_BEFORE_AGENT_REINITIALIZED
      *     @li gSKIEVENT_AFTER_AGENT_REINITIALIZED
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
      virtual void AddAgentListener(egSKIAgentEventId   eventId, 
                                    IAgentListener*     listener, 
                                    bool                allowAsynch = false,
                                    Error*              err         = 0) = 0;

     /**
      *  @brief Removes an agent event listener
      *
      *  Call this method to remove a previously added event listener.
      *  The system will automatically remove all listeners when the kernel shutsdown;
      *   however, since all listeners are client owned, the client is responsible for
      *   cleaning up memory used by listeners.
      *
      *  If the given listener is not registered to recieve the given event, this
      *     function will do nothing (but a warning is logged).
      *
      *  Agent events are:
      *     @li gSKIEVENT_AFTER_AGENT_CREATED
      *     @li gSKIEVENT_BEFORE_AGENT_DESTROYED
      *     @li gSKIEVENT_BEFORE_AGENT_REINITIALIZED
      *     @li gSKIEVENT_AFTER_AGENT_REINITIALIZED
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
      virtual void RemoveAgentListener(egSKIAgentEventId    eventId,
                                       IAgentListener*      listener,
                                       Error*               err = 0) = 0;

	  virtual void FireBeforeAgentsRunStepEvent() = 0 ;
   };


}

#endif




