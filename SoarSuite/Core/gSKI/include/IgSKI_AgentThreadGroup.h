/********************************************************************
* @file igski_agentthreadgroup.h 
*********************************************************************
* @remarks Copyright (C) 2002 Soar Technology, All rights reserved. 
* The U.S. government has non-exclusive license to this software 
* for government purposes. 
*********************************************************************
* created:	   6/13/2002   15:00
*
* purpose: 
*********************************************************************/
#ifndef IGSKI_AGENTTHREADGROUP_H
#define IGSKI_AGENTTHREADGROUP_H

#include "IgSKI_AgentRunControl.h"

namespace gSKI {

   /** 
   * @brief Definition of the Agent Thread Group Interface
   *
   * An agent thread group is a collection of agents associated with
   *  a single thread of execution.  All agents within a thread
   *  group are run and stopped simultaneously.  Agents in different
   *  thread groups are run and stopped independently.
   *
   * An agent can be in only one thread group at a time.  An agent
   *  must be in a thread group to run.  When running in single
   *  threaded mode, a special client-thread group is created and
   *  all agents are added to that thread group. 
   *
   * @see IAgentManager::CreateAgentThreadGroup
   * @see IAgentManager::CreateSingleAgentThreadGroup
   * @see IAgentManager::RunInClientThread
   * @see IAgentManager::RunInSeparateThread
   * @see IAgentRunControl
   */
   class IAgentThreadGroup: public IAgentRunControl {
   public:
      /**
       * @brief Agent Thread Group destructor
       *
       * Thread groups don't own agents, so this destructor
       *  does not destroy them. However, if agents
       *  are running, they will be stopped before this 
       *  destructor returns.
       */
      virtual ~IAgentThreadGroup() = 0;

      /**
       * @brief Add an agent to this thread group
       *
       * Call this method to add an agent to this thread group.  Once an
       *  agent is part of a thread group, it can be run by calling one
       *  of this thread group's Run methods.
       *
       * Agents cannot belong to more than one thread group, so if the
       *  given agent already belongs to a thread group, this call will
       *  fail.
       *
       * If the thread group is currently executing the agent will be
       *   added as soon as all the existing agents finish their current
       *   execution phase.  The exact phase at which the agent is added
       *   depends on the run interleaving setting used when Run was
       *   called.  
       *
       * Possible Errors:
       *   @li gSKIERR_AGENT_ALREADY_IN_THREAD_GROUP if the given agent is already
       *    a member of another thread group.  Remove the agent from the old
       *    thread group before adding to this one.
       *   @li gSKIERR_INVALID_PTR if the given agent ptr is invalid
       *
       * @param agent A pointer to the agent to add to this thread group.
       * @param  err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       * @returns true if the agent is successfully added.  false if there is 
       *           an error (see err for extended error information).
       */
      virtual bool AddAgent(Agent* agent, Error* err = 0) = 0;

      /**
       * @brief Remove an agent from this thread group
       *
       * Call this method when you want to remove an agent from a 
       *  threadgroup.  Typically you only do this when switching
       *  an agent from one thread group to another; however, the
       *  system calls this method just before an agent is destroyed.
       *
       * It is safe to remove an agent from a running thread group; however
       *  the agent will not be removed until the end of the current
       *  elaboration or decision phase (whichever comes first).
       *
       * Possible Errors: 
       *   @li gSKIERR_INVALID_PTR if the given agent ptr is invalid
       *   @li gSKIERR_AGENT_DOES_NOT_EXIST if the given agent pointer points to an
       *           agent that is not part of this thread group.
       *
       * @param agent Pointer to the agent to remove from this thread group
       * @param  err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       * @returns true if the agent could be removed, false if there
       *   was an error (see err for extended info).
       */
      virtual bool RemoveAgent(Agent* agent, Error* err = 0) = 0;

      /**
      *  @brief Sets whether to destroy this thread group when its last agent is destroyed
      * 
      *  Call this method to set a flag telling the systemm whether to 
      *   destroy this thread group when its last agent is removed.
      *   Destroying an agent removes it from its thread group automatically.
      *
      *  You can set this flag when creating the agent thread group through
      *    IAgentManager::AddAgentThreadGroup and IAgentManager::AddSingleAgentThreadGroup
      *
      *  @note Autodestroy only executes when the last agent is removed.  If you
      *         create an empty thread group, the thread group will not be
      *         destroyed until you add and remove at least one agent.
      * 
      *  @param autoDestroy true to have this agent thread group be automatically
      *           destroyed when it's last agent is removed.
       * @param  err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
      */
      virtual void SetAutoDestroy(bool autoDestroy, Error* err = 0) = 0;

      /** 
      * @brief Gets the value of the auto destroy flag for this thread group
      *
      * The auto destroy flag indicates whether this agent thread group
      *  will be automatically destroyed when its last agent is removed.
      *
      * @see IAgentThreadGroup::SetAutoDestroy
      *
      * @param  err Pointer to client-owned error structure.  If the pointer
      *               is not 0 this structure is filled with extended error
      *               information.  If it is 0 (the default) extended error
      *               information is not returned.
      *
      * @return true if this thread group will be automatically destroyed
      *          when its last agent is removed, false if it will not
      *          be automatically destroyed.
      */
      virtual bool GetAutoDestroy(Error* err = 0) = 0;

      /**
       * @brief Gets an iterator to all of the agents in this thread group
       *  
       * @param err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       * @returns A pointer to an iterator into all of the agents in this
       *           thread group.  This pointer will never be 0.
       */
      virtual tIAgentIterator* GetAgentIterator(Error* err = 0) = 0;

   };
}

#endif
