#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include "portability.h"

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gSKI_AgentRunManager.cpp
*********************************************************************
* created:	   6/27/2002   10:44
*
* purpose: 
*********************************************************************/

#include "AgentRunManager.h"

#include "MegaAssert.h"
#include "gSKI_Agent.h"
#include "gSKI_Error.h"
#include "gSKI_Enumerations.h"
#include "gSKI_AgentManager.h"
#include "gSKI_Kernel.h"

#include <algorithm>

//#include "MegaUnitTest.h"
//DEF_EXPOSE(gSKI_AgentRunManager);

namespace gSKI
{
   /*
   =============================
      Constructor for this inner class
   =============================
   */   
   AgentRunManager::AgentRunData::AgentRunData(Agent* _a, unsigned long _steps, unsigned long _maxSteps): 
      a(_a), steps(_steps), maxSteps(_maxSteps) { }

   /*
   =============================
      
   =============================
   */   
   void AgentRunManager::AddAgentToRunList(Agent* a)
   {
      // We make sure we don't add it twice
      tAgentListIt it = std::find(m_addedAgents.begin(), m_addedAgents.end(), a);
      if(it == m_addedAgents.end())
      {
         m_addedAgents.push_back(a);

         // If it is in the remove list, we remove it (this way we don't try
         //  to add and remove the same agent)
         it = std::find(m_removedAgents.begin(), m_removedAgents.end(), a);
         if(it != m_removedAgents.end())
            m_removedAgents.erase(it);
      }
   }

   /*
   =============================
      
   =============================
   */   
   void AgentRunManager::RemoveAgentFromRunList(Agent* a)
   {
      // We make sure we don't remove it twice
      tAgentListIt it = std::find(m_removedAgents.begin(), m_removedAgents.end(), a);
      if(it == m_removedAgents.end())
      {
         m_removedAgents.push_back(a);
      
         // If it is in the add list, we remove it (this way we don't try
         //  to add and remove the same agent)
         it = std::find(m_addedAgents.begin(), m_addedAgents.end(), a);
         if(it != m_addedAgents.end())
            m_addedAgents.erase(it);
      }
   }

   /*
   =============================
      
   =============================
   */   
   void AgentRunManager::HandleEvent(egSKIAgentEventId eventId, Agent* agentPtr)
   {
      if(eventId == gSKIEVENT_BEFORE_AGENT_DESTROYED)
         m_removedAgents.push_back(agentPtr);
   }

   /*
   =============================
      
   =============================
   */   
   bool AgentRunManager::isValidAgent(Agent* a)
   {
      if(m_removedAgents.size() > 0)
      {
         tAgentListIt it = std::find(m_removedAgents.begin(), m_removedAgents.end(), a);
         return (it == m_removedAgents.end())? true: false;
      }
      // There are no agents to remove
      return true;
   }

   /*
   =============================
      
   =============================
   */   
   void AgentRunManager::synchronizeRunList(egSKIRunType  runLength, 
                                            unsigned long steps,
                                            bool          forceReinit)
   {
      // It is guarranteed that no agent can be in both
      //  the add and remove list at the same time, so we
      //  don't have to wory about that.
      if(m_addedAgents.size() > 0)
      {
         // For each added agent, put it into our list
         for(tAgentListIt itA = m_addedAgents.begin();
               itA != m_addedAgents.end(); ++itA)
         {
            addToRunList(*itA, runLength, steps);
         }
         m_addedAgents.clear();
      }

      if(m_removedAgents.size() > 0)
      {
         // For each removed agent, take it out of our list
         for(tAgentListIt itR = m_removedAgents.begin(); 
               itR != m_removedAgents.end(); ++itR)
         {
            // Find and remove it from the run list
            removeFromRunList(*itR);
         }
         m_removedAgents.clear();
      }

      // Initialize the agents for a new  run
      if(forceReinit)
         initializeForRun(runLength, steps);
   }

   /*
   =============================
      
   =============================
   */   
   void AgentRunManager::initializeForRun(egSKIRunType runLength, unsigned long steps)
   {
      tAgentRunListIt it;
      for(it = m_runningAgents.begin(); it != m_runningAgents.end(); ++it)
      {
         // All we need to do is set the step counts
         (*it).steps    = getReleventStepCount((*it).a, runLength);
         (*it).maxSteps = (*it).steps + steps;
      }
   }

   /*
   =============================
      
   =============================
   */   
   void AgentRunManager::addToRunList(Agent* a, egSKIRunType runLength, unsigned long steps)
   {
      // Iterate until we find the agent if it is in the list.
      tAgentRunListIt it = m_runningAgents.begin(); 
      while((it != m_runningAgents.end()))
      {
         if((*it).a == a)
         {
            break;
         }
         ++it;
      }

      // 
      // If the agents is not already in the list, add it.
      if(it == m_runningAgents.end())
      {
         m_runningAgents.push_back(AgentRunData(a, getReleventStepCount(a, runLength), steps));
      }
   }

   /*
   =============================
      
   =============================
   */   
   void AgentRunManager::removeFromRunList(Agent* a)
   {
      // Iterate until we find the agent
      tAgentRunListIt it = m_runningAgents.begin(); 
      while((it != m_runningAgents.end()) && ((*it).a != a))
         ++it;

      // If we found it, remove it
      if(it != m_runningAgents.end())
         m_runningAgents.erase(it);
   }

   Kernel* AgentRunManager::getKernel()
   {
		return m_pKernel ;
   }

   /*
   =============================

   =============================
   */
   egSKIRunResult AgentRunManager::Run(egSKIRunType        runLength, 
                                       unsigned long       count,
                                       egSKIInterleaveType runInterleave,
                                       Error*              err)
   {
      // Only works for single threaded apps.  Stop a reentrant call
      //  from a run callback handler.
       if(m_groupRunning) {
         SetError(err, gSKIERR_AGENT_RUNNING);
         return  gSKI_RUN_ERROR;
       }

      m_groupRunning = true;

      egSKIRunResult runResult;
      AgentRunData*  curData;
      bool           runFinished = false;
	  int			 stepCount = 0 ;

      ClearError(err);

      // Cause the loop to think we always have one more step to go
      //  in run forever
      if(runLength == gSKI_RUN_FOREVER)
         count = 1;

      // Initialize our run list
      synchronizeRunList(runLength, count, true);

      // Tell the client that there is nothing to run
      if(m_runningAgents.size() == 0)
      {
         SetError(err, gSKIERR_NO_AGENTS_TO_RUN);
         m_groupRunning = false; 
         return gSKI_RUN_ERROR;
      }

	  // Send event for each agent to signal its about to start running
	  FireBeforeRunStartsEvents() ;

      // Run all the ones that can be run
      while(!runFinished)
      {
         // Assume it is finished until proven otherwise
         runFinished = true;

		 // Callback to clients to see if they wish to stop this run.
		 // A client can use any event, but this one is designed to allow clients
		 // to throttle back the frequency of the event to control performance.
		if ((stepCount % getKernel()->GetInterruptCheckRate()) == 0)
			((Kernel*)getKernel())->FireInterruptCheckEvent() ;
		stepCount++ ;

		 // Notify listeners that Soar is running.  This event is a kernel level (agent manager) event
		 // which allows a single listener to check for client driven interrupts for all agents.
		 // Sometimes that's easier to work with than the agent specific events (where you get <n> events from <n> agents)
		 AgentManager* pManager = (AgentManager*)getKernel()->GetAgentManager() ;
		 pManager->FireBeforeAgentsRunStepEvent() ;

         // Run each agent for the interleave amount
         for(tAgentRunListIt it = m_runningAgents.begin(); it != m_runningAgents.end(); ++it)
         {
            curData = &(*it);

            // Check to see if the agent is still valid and that we have
            //  not reached our run limit for this agent
            if(isValidAgent(curData->a) && (curData->steps < curData->maxSteps))
            {
               if(curData->a->GetRunState() == gSKI_RUNSTATE_STOPPED)
               {
                  runResult = curData->a->RunInClientThread(
                     (egSKIRunType)(runInterleave), 1, err);               
                  
                  // We only stop everyone running if we've received
				  // an error from one agent.  Interrupting one agent
				  // does not stop the others from continuing to run.
                  if(runResult == gSKI_RUN_ERROR)
                  {
                     m_groupRunning = false;
                     return gSKI_RUN_ERROR;
                  } 
               }
            }

            // See if we've finished the run
            if(isValidAgent(curData->a))
            {
               // Halting an agent removes it from the run list, so does
               //  running it individularlly
               if(curData->a->GetRunState() != gSKI_RUNSTATE_STOPPED)
               {
				  // Notify listeners that this agent is finished running
				  ((Agent*)curData->a)->FireRunEndsEvent() ;

                  RemoveAgentFromRunList(curData->a);
               }
               else
               {
                  // Update the number of steps this agent has executed
                  curData->steps = getReleventStepCount(curData->a, runLength);
                  if(curData->steps < curData->maxSteps)
                     runFinished = false;
               }
            }
         }

         // Now synchronize the run list adding any new agents and
         //  removing any old agents.
         synchronizeRunList(runLength, count, false);
      }

	  // Send event for each agent still in the run list that it has finished run
	  FireAfterRunEndsEvents() ;

	  // Fire one event to indicate the entire system (simulation) should stop.
	  // (Above gSKI we can choose to suppress this event in some situations)
	  ((Kernel*)getKernel())->FireSystemStop() ;

      // Finshed
      m_groupRunning = false;
      return gSKI_RUN_COMPLETED;
   }

	/** Notify listeners that agents in the run list are starting or finishing their runs **/
	void AgentRunManager::FireBeforeRunStartsEvents()
	{
         for(tAgentRunListIt it = m_runningAgents.begin(); it != m_runningAgents.end(); ++it)
         {
            AgentRunData* curData = &(*it);

            // Fire the event for this agent
            if(isValidAgent(curData->a))
				((Agent*)curData->a)->FireRunStartsEvent() ;
		 }
	}

	void AgentRunManager::FireAfterRunEndsEvents()
	{
         for(tAgentRunListIt it = m_runningAgents.begin(); it != m_runningAgents.end(); ++it)
         {
            AgentRunData* curData = &(*it);

            // Fire the event for this agent
            if(isValidAgent(curData->a))
				((Agent*)curData->a)->FireRunEndsEvent() ;
		 }
	}

   /*
   =============================

   =============================
   */
   unsigned long AgentRunManager::getReleventStepCount(Agent* a, egSKIRunType runType)
   {
      switch(runType)
      {
      case gSKI_RUN_SMALLEST_STEP:
         return a->GetNumSmallestStepsExecuted();
      case gSKI_RUN_PHASE:
         return a->GetNumPhasesExecuted();
      case gSKI_RUN_ELABORATION_CYCLE:
         return a->GetNumElaborationsExecuted();
      case gSKI_RUN_DECISION_CYCLE:
         return a->GetNumDecisionCyclesExecuted();
      case gSKI_RUN_UNTIL_OUTPUT:
         return a->GetNumOutputsExecuted();
      default:
         return 0;
      }
   }

}
