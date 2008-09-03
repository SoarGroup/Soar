#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include "portability.h"

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gSKI_AgentManager.cpp
*********************************************************************
* created:	   6/27/2002   10:44
*
* purpose: 
*********************************************************************/
#include "gSKI_AgentManager.h"
#include "gSKI_Error.h"
#include "gSKI_Agent.h"
#include "gSKI_Enumerations.h"
#include "gSKI_SetActiveAgent.h"
#include "gSKI_Kernel.h"
#include "gSKI_ProductionManager.h"

#include "MegaAssert.h"

#include "init_soar.h"
#include "agent.h"
#include "kernel_struct.h"

//#include "MegaUnitTest.h"
//DEF_EXPOSE(gSKI_AgentManager);

namespace gSKI 
{

 

   /*
   =============================
    _                    _   __  __
   / \   __ _  ___ _ __ | |_|  \/  | __ _ _ __   __ _  __ _  ___ _ __
  / _ \ / _` |/ _ \ '_ \| __| |\/| |/ _` | '_ \ / _` |/ _` |/ _ \ '__|
 / ___ \ (_| |  __/ | | | |_| |  | | (_| | | | | (_| | (_| |  __/ |
/_/   \_\__, |\___|_| |_|\__|_|  |_|\__,_|_| |_|\__,_|\__, |\___|_|
        |___/                                         |___/
   =============================
   */
   AgentManager::AgentManager(Kernel* krnl) : m_kernel(krnl), m_runManager(krnl)
   {
      m_runCompleteListener.SetAgentManager(this);

      // Let the run manager know when agents are destroyed.
      AddAgentListener(gSKIEVENT_BEFORE_AGENT_DESTROYED, &m_runManager);
   }

   AgentManager::~AgentManager() 
   {
      for(tAgentMap::It it = m_agents.begin(); it != m_agents.end(); ++it)
      {
         // TODO: anything special if the agent is running?
         // maybe just require that the agents be stopped..
         delete it->second;
      }
   }

   /*
   =============================

   =============================
   */
   void AgentManager::AddAgentToRunList(IAgent* agentToAdd, Error* err)
   {
      ClearError(err);
      m_runManager.AddAgentToRunList(agentToAdd);
   }

   /*
   =============================

   =============================
   */
   void AgentManager::RemoveAgentFromRunList(IAgent* agentToAdd, Error* err)
   {
      ClearError(err);
      m_runManager.RemoveAgentFromRunList(agentToAdd);
   }

   /*
   =============================

   =============================
   */
   void AgentManager::AddAllAgentsToRunList(Error* err)
   {
      ClearError(err);
      for(tAgentMap::It it = m_agents.begin(); it != m_agents.end(); ++it)
         m_runManager.AddAgentToRunList((*it).second);
   }

   /*
   =============================

   =============================
   */
   void AgentManager::RemoveAllAgentsFromRunList(Error* err)
   {
      ClearError(err);
      for(tAgentMap::It it = m_agents.begin(); it != m_agents.end(); ++it)
         m_runManager.RemoveAgentFromRunList((*it).second);
   }

   /*
   =============================
 ____              ___        ____ _ _            _  _____ _                        _
|  _ \ _   _ _ __ |_ _|_ __  / ___| (_) ___ _ __ | ||_   _| |__  _ __ ___  __ _  __| |
| |_) | | | | '_ \ | || '_ \| |   | | |/ _ \ '_ \| __|| | | '_ \| '__/ _ \/ _` |/ _` |
|  _ <| |_| | | | || || | | | |___| | |  __/ | | | |_ | | | | | | | |  __/ (_| | (_| |
|_| \_\\__,_|_| |_|___|_| |_|\____|_|_|\___|_| |_|\__||_| |_| |_|_|  \___|\__,_|\__,_|
   =============================
   */
    egSKIRunResult AgentManager::RunInClientThread(egSKIRunType        runLength, 
                                                   unsigned long       count,
                                                   egSKIInterleaveType runInterleave,
                                                   Error*              err)
   {

      MegaAssert((count > 0) || (runLength == gSKI_RUN_FOREVER), "Cannot run agents for fewer than one steps.");
      return m_runManager.Run(runLength, count, runInterleave, err);
   }



   /*
   =============================
 ____              ___       ____                             _
|  _ \ _   _ _ __ |_ _|_ __ / ___|  ___ _ __   __ _ _ __ __ _| |_ ___
| |_) | | | | '_ \ | || '_ \\___ \ / _ \ '_ \ / _` | '__/ _` | __/ _ \
|  _ <| |_| | | | || || | | |___) |  __/ |_) | (_| | | | (_| | ||  __/
|_|_\_\\__,_|_| |_|___|_| |_|____/ \___| .__/ \__,_|_|  \__,_|\__\___|
|_   _| |__  _ __ ___  __ _  __| |     |_|
  | | | '_ \| '__/ _ \/ _` |/ _` |
  | | | | | | | |  __/ (_| | (_| |
  |_| |_| |_|_|  \___|\__,_|\__,_|
   =============================
   */
   egSKIRunResult AgentManager::RunInSeparateThread(egSKIRunType        runLength,    
                                                    unsigned long       count,        
                                                    egSKIInterleaveType runInterleave,
                                                    Error*              err)
   {
      MegaAssert(false, "Cannot deal with multithreading yet");
      SetError(err, gSKIERR_NOT_IMPLEMENTED);

      return gSKI_RUN_ERROR;
   }

   /*
   =============================
 ___       _                             _      _    _ _
|_ _|_ __ | |_ ___ _ __ _ __ _   _ _ __ | |_   / \  | | |
 | || '_ \| __/ _ \ '__| '__| | | | '_ \| __| / _ \ | | |
 | || | | | ||  __/ |  | |  | |_| | |_) | |_ / ___ \| | |
|___|_| |_|\__\___|_|  |_|   \__,_| .__/ \__/_/   \_\_|_|
                                  |_|
   =============================
   */
   bool AgentManager::InterruptAll(egSKIStopLocation    stopLoc, 
                                   Error*               err)
   {
      IAgent* a;

      // This type of stopping requires full threading
      MegaAssert(stopLoc  != gSKI_STOP_ON_CALLBACK_RETURN, "This mode is not implemented.");
      MegaAssert(stopLoc  != gSKI_STOP_AFTER_ALL_CALLBACKS_RETURN, "This mode is not implemented.");
      if((stopLoc  == gSKI_STOP_ON_CALLBACK_RETURN) ||
         (stopLoc  == gSKI_STOP_AFTER_ALL_CALLBACKS_RETURN))
      { 
         SetError(err, gSKIERR_NOT_IMPLEMENTED);
         return false;
      }

      ClearError(err);

      // Iterate over each of the agents and call interrupt on each of
      //  them, checking first to see if they are actually running.
      for(tAgentMap::It aIt = m_agents.begin() ; aIt != m_agents.end(); ++aIt)
      {
         a = (*aIt).second;

         // This will clear the error if there is no error
         if(a->Interrupt(stopLoc, gSKI_STOP_BY_RETURNING, err) == false)
            return false;
      }

      // We succeeded
      return true;
   }

   /*
   =============================
  ____ _                  _    _ _ ___       _                             _
 / ___| | ___  __ _ _ __ / \  | | |_ _|_ __ | |_ ___ _ __ _ __ _   _ _ __ | |_ ___
| |   | |/ _ \/ _` | '__/ _ \ | | || || '_ \| __/ _ \ '__| '__| | | | '_ \| __/ __|
| |___| |  __/ (_| | | / ___ \| | || || | | | ||  __/ |  | |  | |_| | |_) | |_\__ \
 \____|_|\___|\__,_|_|/_/   \_\_|_|___|_| |_|\__\___|_|  |_|   \__,_| .__/ \__|___/
                                                                    |_|
   =============================
   */
   void AgentManager::ClearAllInterrupts(Error* err)
   {
      IAgent* a;

      // Just in case there are no agents
      ClearError(err);

      // Iterate over each of the agents and if they are interrupted,
      //  call continue on them
      for(tAgentMap::It aIt = m_agents.begin() ; aIt != m_agents.end(); ++aIt)
      {
         a = (*aIt).second;

         // This will clear the error if there is no error
         if(a->GetRunState() == gSKI_RUNSTATE_INTERRUPTED)
            a->ClearInterrupts(err);
      }
   }

   /*
   =============================
 _   _       _ _      _    _ _
| | | | __ _| | |_   / \  | | |
| |_| |/ _` | | __| / _ \ | | |
|  _  | (_| | | |_ / ___ \| | |
|_| |_|\__,_|_|\__/_/   \_\_|_|
   =============================
   */
   void AgentManager::HaltAll(Error* err)
   {
      IAgent* a;

      ClearError(err);

      // Iterate over each of the agents and if they are interrupted,
      //  call continue on them
      for(tAgentMap::It aIt = m_agents.begin() ; aIt != m_agents.end(); ++aIt)
      {
         a = (*aIt).second;

         // Halt the system and return the error value if there is an error
         a->Halt(err);
         if(err->Id != gSKIERR_NONE)
            return;
      }
   }

   /*
   =============================
  ____      _      _                    _   ___ _                 _
 / ___| ___| |_   / \   __ _  ___ _ __ | |_|_ _| |_ ___ _ __ __ _| |_ ___  _ __
| |  _ / _ \ __| / _ \ / _` |/ _ \ '_ \| __|| || __/ _ \ '__/ _` | __/ _ \| '__|
| |_| |  __/ |_ / ___ \ (_| |  __/ | | | |_ | || ||  __/ | | (_| | || (_) | |
 \____|\___|\__/_/   \_\__, |\___|_| |_|\__|___|\__\___|_|  \__,_|\__\___/|_|
                       |___/
   =============================
   */
   tIAgentIterator* AgentManager::GetAgentIterator(Error* err)
   {
		ClearError(err);

		// I don't see a way to build a gSKI iterator from a map
		// so copying the agents into a vector and creating an iterator for that.
		std::vector<IAgent*> agents ;

		for(tAgentMap::It iter = m_agents.begin() ; iter != m_agents.end(); ++iter)
		{
			IAgent* pAgent = iter->second ;
			agents.push_back(pAgent) ;
		}

		// Create an iterator for our vector.  I assume the vector can now go out of scope...I hope that's right.
		tIAgentIterator* pAgentIter = new tAgentIter(agents) ;

		return pAgentIter;
   }

   /*
   =============================
  ____      _      _                    _
 / ___| ___| |_   / \   __ _  ___ _ __ | |_
| |  _ / _ \ __| / _ \ / _` |/ _ \ '_ \| __|
| |_| |  __/ |_ / ___ \ (_| |  __/ | | | |_
 \____|\___|\__/_/   \_\__, |\___|_| |_|\__|
                       |___/
   =============================
   */
   IAgent* AgentManager::GetAgent(const char* name, Error* err)
   {
      ClearError(err);

      if(IsInvalidPtr(name, err))
         return 0;

      //
      // Search the map for this agent (by name) and
      // return the IAgent pointer if it is in the list.
      //
      tAgentMap::It agentIt = m_agents.find(name);
      if(agentIt == m_agents.end())
      {
         SetError(err, gSKIERR_AGENT_DOES_NOT_EXIST);
         return 0;
      }

      return agentIt->second;
   }

   /*
   =============================
 ____                                  _                    _   ____
|  _ \ ___ _ __ ___   _____   _____   / \   __ _  ___ _ __ | |_| __ ) _   _
| |_) / _ \ '_ ` _ \ / _ \ \ / / _ \ / _ \ / _` |/ _ \ '_ \| __|  _ \| | | |
|  _ <  __/ | | | | | (_) \ V /  __// ___ \ (_| |  __/ | | | |_| |_) | |_| |
|_| \_\___|_| |_| |_|\___/ \_/ \___/_/   \_\__, |\___|_| |_|\__|____/ \__, |
| \ | | __ _ _ __ ___   ___                |___/                      |___/
|  \| |/ _` | '_ ` _ \ / _ \
| |\  | (_| | | | | | |  __/
|_| \_|\__,_|_| |_| |_|\___|
   =============================
   */
   void AgentManager::RemoveAgentByName(const char *name, Error* err)
   {
      ClearError(err);

      if(IsInvalidPtr(name, err))
         return;

      //
      // Find the agent and delete it if you find it.
      // Otherwise, return an error.
      //
      tAgentMap::It agentIt = m_agents.find(name);
      if(agentIt != m_agents.end())
      {
         Agent* _agent = (*agentIt).second;

         // If it is running, we listen for it to stop running and
         //  delete it after it is stopped.
         if(_agent->GetRunState() == gSKI_RUNSTATE_RUNNING)
         {
            // Set up to listen for this agent being done running.  This will
            //  call RemoveAgentByName after the agent completes its run
            _agent->AddRunListener(gSKIEVENT_AFTER_RUN_ENDS, &m_runCompleteListener);

            // Tell it to halt
            _agent->Halt();
         }
         else
         {
            // It is ok to remove it here

            // Notify everyone that the agent is going to be destroyed
            AgentNotifier nf((*agentIt).second);
            m_agentListeners.Notify(gSKIEVENT_BEFORE_AGENT_DESTROYED, nf);

            // Destroy it
            delete((*agentIt).second);
            m_agents.erase(agentIt);
         }

      } 
      else 
      {
         SetError(err, gSKIERR_AGENT_DOES_NOT_EXIST);
      }
   }

   /*
   =============================
 ____                                  _                    _
|  _ \ ___ _ __ ___   _____   _____   / \   __ _  ___ _ __ | |_
| |_) / _ \ '_ ` _ \ / _ \ \ / / _ \ / _ \ / _` |/ _ \ '_ \| __|
|  _ <  __/ | | | | | (_) \ V /  __// ___ \ (_| |  __/ | | | |_
|_| \_\___|_| |_| |_|\___/ \_/ \___/_/   \_\__, |\___|_| |_|\__|
                                           |___/
   =============================
   */
   void AgentManager::RemoveAgent(IAgent* agent, Error* err)
   {
      ClearError(err);

      if(IsInvalidPtr(agent, err))
         return;

      tAgentMap::It agentIt = m_agents.begin();

      //
      // Find the agent in the map as a 'second' value.
      for(; agentIt != m_agents.end(); ++agentIt)
      {
         //
         // Is this one the agent we are looking for?
         if(agentIt->second == agent)
         {
            //
            // Remove the agent and leave the method.
            RemoveAgentByName(agentIt->second->GetName());
            return;
         }
      }
      SetError(err, gSKIERR_AGENT_DOES_NOT_EXIST);
   }

   /*
   =============================
    _       _     _    _                    _
   / \   __| | __| |  / \   __ _  ___ _ __ | |_
  / _ \ / _` |/ _` | / _ \ / _` |/ _ \ '_ \| __|
 / ___ \ (_| | (_| |/ ___ \ (_| |  __/ | | | |_
/_/   \_\__,_|\__,_/_/   \_\__, |\___|_| |_|\__|
                           |___/
   =============================
   */
   IAgent* AgentManager::AddAgent(const char*       name, 
                                  const char*       prodFileName, 
                                  bool              learningOn,
                                  egSKIOSupportMode oSupportMode,
                                  Error*            err)
   {
      ClearError(err);
	  //MegaAssert(false, "Stop here for debugging!");

      if(IsInvalidPtr(name, err))
         return 0;

      Agent* _agent = new Agent(name, m_kernel);

      std::string tmpStr(name);
      m_agents.insert(std::pair<std::string, Agent *>(tmpStr, _agent));

      // Notify everyone that the agent was created
      AgentNotifier nf(_agent);
      m_agentListeners.Notify(gSKIEVENT_AFTER_AGENT_CREATED, nf);

	  if ( prodFileName ) {
		_agent->GetProductionManager()->LoadSoarFile(prodFileName, err);
	  }
      
      return _agent;
   }

   // TODO: Implement ReinitializeAll() method from the IAgentRunControl
   // interface
   bool AgentManager::ReinitializeAll(Error* err) 
   { 
      MegaAssert(false, "Implement this method!");
      return true; 
   } 


   /*
   =============================

   =============================
   */
   void AgentManager::AddAgentListener(egSKIAgentEventId eventId, 
                                     IAgentListener*     listener, 
                                     bool                allowAsynch,
                                     Error*              err)
   {
      AddListenerToManager(m_agentListeners, eventId, listener, err);
   }

   /*
   =============================

   =============================
   */
   void AgentManager::RemoveAgentListener(egSKIAgentEventId  eventId,
                                        IAgentListener*      listener,
                                        Error*               err)
   {
      RemoveListenerFromManager(m_agentListeners, eventId, listener, err);
   }

   /*
   =============================

   =============================
   */
   void AgentManager::FireBeforeAgentReinitialized(IAgent* a)
   {
      AgentNotifier nf(a);
      m_agentListeners.Notify(gSKIEVENT_BEFORE_AGENT_REINITIALIZED, nf);
   }
   
   /*
   =============================

   =============================
   */
   void AgentManager::FireAfterAgentReinitialized(IAgent* a)
   {
      AgentNotifier nf(a);
      m_agentListeners.Notify(gSKIEVENT_AFTER_AGENT_REINITIALIZED, nf);
   }

   /*
   =============================

   =============================
   */
   void AgentManager::FireBeforeAgentsRunStepEvent()
   {
	  AgentNotifier nf(NULL) ;	// Not an agent specific event but an agent manager event.
      m_agentListeners.Notify(gSKIEVENT_BEFORE_AGENTS_RUN_STEP, nf);
   }

   /*
   =============================

   =============================
   */
   void AgentManager::AgentRunCompletedListener::HandleEvent(egSKIRunEventId eventId, IAgent* agentPtr, egSKIPhaseType phase)
   {
      MegaAssert(eventId == gSKIEVENT_AFTER_RUN_ENDS, "Getting an unexpected event in the agent removal listener.");
      m_am->RemoveAgentByName(agentPtr->GetName());
   }

}

//////////////////////////////////////////////////////////////////////////////////////////

