/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file agentrunmanager.h
*********************************************************************
* created:	   6/27/2002   10:44
*
* purpose: 
*********************************************************************/
#ifndef GSKI_AGENTRUNMANAGER_H
#define GSKI_AGENTRUNMANAGER_H

#include <vector>
#include "gSKI_Events.h"

namespace gSKI
{
   class Agent;
   struct Error;

   /**  
    * @brief Run listener used to manage the list of agents currently
    *           being scheduled by RunAllInClientThread
    */
   class AgentRunManager: public IAgentListener
   {
   public:

      /** 
       * @brief Structure to hold data for mult-agent running
       */
      struct AgentRunData
      {
         /** Pointer to the agent */
         Agent*           a;
      
         /** Number of steps left to execute */
         unsigned long     steps;
         
         /** Whether or not this agent is interrupted */
         unsigned long     maxSteps;

         /** Initialize members */
         AgentRunData(Agent* _a, unsigned long _steps, unsigned long _maxSteps);
      };

      /** 
      * @brief typedefs for list of agents being run
      */
      //{
      typedef std::vector<AgentRunData>           tAgentRunList;
      typedef tAgentRunList::iterator             tAgentRunListIt;
      //}

      typedef std::vector<Agent*>                tAgentList;
      typedef tAgentList::iterator                tAgentListIt;

   public:

      /** 
       * @brief
       */
      AgentRunManager(Kernel* pKernel): m_groupRunning(false), m_pKernel(pKernel) {}

      /** 
       *@brief
       */
      virtual ~AgentRunManager() {}

      /** 
       * @brief  Adds an agent to the buffer of agents that can be run next cycle
       */ 
      void AddAgentToRunList(Agent* a);

      /** 
       * @brief Removes an agent from the run list
       *
       * This will take effect at the end of the current cycle in multi-agent
       *  running (when SynchronizeRunList is called).
       */
      void RemoveAgentFromRunList(Agent* a);

      /** 
       * @brief runs the agents in the run list.
       *
       * The parameters are the same as those for RunInClientThread.
       * AgentManager::RunInClientThread calls this method to execute
       *  agent running.
       */
      virtual egSKIRunResult Run(egSKIRunType        runLength, 
                                unsigned long       count,
                                egSKIInterleaveType runInterleave,
                                Error*              err);

      /** 
       * @brief The callback method for run events
	   *
	   *  KJC:  2/21  Actually the only event is for an egSKIAgentEvent, 
	   *   gSKIEVENT_BEFORE_AGENT_DESTROYED, so it's not really a RunEvent.
       */
      virtual void HandleEvent(egSKIAgentEventId eventId, Agent* agentPtr);

   private:

      /** 
       * @brief Tell the caller whether or not this agent is valid to run
       *
       *  If it is not in the removed list, it is valid. This is called
       *   from RunAllInClientThread
       */
      bool isValidAgent(Agent* a);

      /** 
       * @brief Puts new agents in the run list and removes old ones
       *
       * If any agent is added to the run list, its counts are initialized.
       */
      void synchronizeRunList(egSKIRunType  runLength, 
                              unsigned long steps,
                              bool          forceReinit);

      
      /** 
       * @brief Adds an agent to our run list
       *
       * The runLength and steps are used to initialize newly added agents.
       */ 
      void addToRunList(Agent* a, egSKIRunType runLength, unsigned long steps);

      /** 
       * @brief Removes an agent from our run list
       */
      void removeFromRunList(Agent* a);

      /** 
       * @brief Gets the run counter relevant to the given runType
       *
       * Used by RunInClientThread.
       */
      unsigned long getReleventStepCount(Agent* a, egSKIRunType runType);

      /** 
       * @brief called by synchronizeRunList when it needs to reinitialize
       *          all of the agents in the run list
       */
      void initializeForRun(egSKIRunType  runLength, unsigned long steps);

	  /** Look up the kernel for this run manager **/
	  Kernel* getKernel() ;

	  /** Notify listeners that agents in the run list are starting or finishing their runs **/
	  void FireBeforeRunStartsEvents() ;
	  void FireAfterRunEndsEvents() ;

   private:

      /** List of agents in the run list */
      tAgentRunList   m_runningAgents;

      /** Agents that are to be removed from the list of running agents */
      tAgentList      m_removedAgents;

      /** Agents that are to be added to the list of running agents */
      tAgentList      m_addedAgents;

      /** Flag indicating that agent group is running */
      bool            m_groupRunning;

	  /** Pointer to the kernel */
	  Kernel*		   m_pKernel ;
   };
}


#endif
