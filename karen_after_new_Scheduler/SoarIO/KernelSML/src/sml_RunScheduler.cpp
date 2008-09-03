#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

/////////////////////////////////////////////////////////////////
// Run class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : May 2005
//
// Used to run Soar and send appropriate events so that an environment
// can function well in concert with a debugger.
//
/////////////////////////////////////////////////////////////////

#include "sml_RunScheduler.h"
#include "sml_KernelSML.h"
#include "sml_AgentSML.h"
#include "sml_ClientEvents.h"

#include "../../gSKI/src/gSKI_Error.h"
#include "IgSKI_AgentManager.h"

#include <assert.h>

using namespace sml ;

RunScheduler::RunScheduler(KernelSML* pKernelSML)
{
	m_pKernelSML = pKernelSML ;
	m_RunFlags = sml_NONE ;
	m_IsRunning = false ;
	m_StopBeforePhase = gSKI_INPUT_PHASE ;
}

/*************************************************************
* @brief	Each agent is set to either run or not when the
*			next Run command is executed.
*************************************************************/
void RunScheduler::ScheduleAgentToRun(AgentSML* pAgentSML, bool state)
{
	if (pAgentSML)
		pAgentSML->ScheduleAgentToRun(state) ;
}

/*************************************************************
* @brief	Sets all agents to either run or not when the
*			next Run command is executed.
*************************************************************/
void RunScheduler::ScheduleAllAgentsToRun(bool state)
{
	for (AgentMapIter iter = m_pKernelSML->m_AgentMap.begin() ; iter != m_pKernelSML->m_AgentMap.end() ; iter++)
	{
		AgentSML* pAgentSML = iter->second ;
		pAgentSML->ScheduleAgentToRun(state) ;
	}
}
/********************************************************************
* @brief	This is a method for getting the default value
*			for the interleaveStepSize for running agents.
*
*********************************************************************/
egSKIInterleaveType RunScheduler::DefaultInterleaveStepSize(egSKIRunType runStepSize)
{
	switch(runStepSize)
	{
	case gSKI_RUN_SMALLEST_STEP:  // deprecated
		return (static_cast <egSKIInterleaveType>(0));
	case gSKI_RUN_PHASE:
		return gSKI_INTERLEAVE_PHASE;
	case gSKI_RUN_ELABORATION_CYCLE:
		return gSKI_INTERLEAVE_ELABORATION_PHASE;
	case gSKI_RUN_DECISION_CYCLE:
		//return gSKI_INTERLEAVE_PHASE;
		return gSKI_INTERLEAVE_DECISION_CYCLE;
	case gSKI_RUN_UNTIL_OUTPUT:
		return gSKI_INTERLEAVE_OUTPUT;
	default:
		return gSKI_INTERLEAVE_PHASE;
	}
}

/********************************************************************
* @brief	Don't try to Run with an nonsense interleaveStepSize
*
*********************************************************************/
bool RunScheduler::VerifyStepSizeForRunType(egSKIRunType runStepSize, egSKIInterleaveType interleave)
{
	switch(runStepSize)
	{
	case gSKI_RUN_SMALLEST_STEP:  // deprecated
		assert (runStepSize == gSKI_RUN_SMALLEST_STEP);
		return true;
	case gSKI_RUN_PHASE:
 		assert( gSKI_INTERLEAVE_PHASE == interleave ) ;
		return true;
	case gSKI_RUN_ELABORATION_CYCLE: 
		assert( gSKI_INTERLEAVE_ELABORATION_PHASE == interleave ) ;
		return true;
	case gSKI_RUN_DECISION_CYCLE:
		assert(gSKI_INTERLEAVE_PHASE == interleave || 
			   gSKI_INTERLEAVE_ELABORATION_PHASE == interleave || 
			   gSKI_INTERLEAVE_DECISION_CYCLE == interleave) ;
		return true;
	case gSKI_RUN_UNTIL_OUTPUT:
		assert(gSKI_INTERLEAVE_PHASE == interleave || 
			   gSKI_INTERLEAVE_ELABORATION_PHASE == interleave || 
			   gSKI_INTERLEAVE_DECISION_CYCLE == interleave ||
			   gSKI_INTERLEAVE_OUTPUT == interleave) ;

	default:
		return false;
	}
}
/********************************************************************
* @brief	Agents maintain a number of counters (for how many phase,
*			decisions etc.) they have ever executed.
*			We use these counters to determine when a run should stop.
*********************************************************************/
unsigned long RunScheduler::GetStepCounter(gSKI::IAgent* pAgent, egSKIRunType runStepSize)
{
	switch(runStepSize)
	{
	case gSKI_RUN_SMALLEST_STEP:
		return pAgent->GetNumSmallestStepsExecuted();
	case gSKI_RUN_PHASE:
		return pAgent->GetNumPhasesExecuted();
	case gSKI_RUN_ELABORATION_CYCLE:
		return pAgent->GetNumElaborationsExecuted();
	case gSKI_RUN_DECISION_CYCLE:
		return pAgent->GetNumDecisionCyclesExecuted();
	case gSKI_RUN_UNTIL_OUTPUT:
		return pAgent->GetNumOutputsExecuted();
	default:
		return 0;
	}
}
unsigned long RunScheduler::GetStepCounter(gSKI::IAgent* pAgent, egSKIInterleaveType stepSize)
{
	switch(stepSize)
	{
	case gSKI_INTERLEAVE_SMALLEST_STEP:
		return pAgent->GetNumSmallestStepsExecuted();
	case gSKI_INTERLEAVE_PHASE:
		return pAgent->GetNumPhasesExecuted();
	case gSKI_INTERLEAVE_ELABORATION_PHASE:
		return pAgent->GetNumElaborationsExecuted();
	case gSKI_INTERLEAVE_DECISION_CYCLE:
		return pAgent->GetNumDecisionCyclesExecuted();
	case gSKI_INTERLEAVE_OUTPUT:
		return pAgent->GetNumOutputsExecuted();
	default:
		return 0;
	}
}

/********************************************************************
* @brief	Agents maintain a number of counters (for how many phase,
*			decisions etc.) they have ever executed.
*			We use these counters to determine when a run should stop.
*********************************************************************/
unsigned long RunScheduler::GetRunCounter(gSKI::IAgent* pAgent, egSKIRunType runStepSize)
{
	switch(runStepSize)
	{
	case gSKI_RUN_SMALLEST_STEP:
		return pAgent->GetNumSmallestStepsExecuted();
	case gSKI_RUN_PHASE:
		return pAgent->GetNumPhasesExecuted();
	case gSKI_RUN_ELABORATION_CYCLE:
		return pAgent->GetNumElaborationsExecuted();
	case gSKI_RUN_DECISION_CYCLE:
		return pAgent->GetNumDecisionCyclesExecuted();
	case gSKI_RUN_UNTIL_OUTPUT:
		return pAgent->GetNumOutputsExecuted();
	default:
		return 0;
	}
}

/*************************************************************************
* @brief	Returns true if phase1 is later in the execution cycle than phase2
**************************************************************************/
static bool IsPhaseLater(egSKIPhaseType phase1, egSKIPhaseType phase2)
{
	// Right now the enum is in the correct order, but if we change Soar we
	// might need to make this more complex.
	// KJC:  won't work for Soar7-mode.  Enum not same and Decision is last.
	return phase1 > phase2 ;
}

/*************************************************************************
* @brief	Returns the agent whose phase we should synch up to.
**************************************************************************/
AgentSML* RunScheduler::GetAgentToSynchronizeWith()
{
	AgentSML* pSynchAgent = NULL ;

	for (AgentMapIter iter = m_pKernelSML->m_AgentMap.begin() ; iter != m_pKernelSML->m_AgentMap.end() ; iter++)
	{
		AgentSML* pAgentSML = iter->second ;

		if (pAgentSML->IsAgentScheduledToRun())
		{
			gSKI::IAgent* pAgent = pAgentSML->GetIAgent() ;

			// What this says is:
			// If we don't have a current synch agent or
			// if this agent is later in decision cycle count or
			// matches decision cycle count and has a later phase count then
			// adopt it as the agent to synchronize with.
			if (!pSynchAgent || pAgent->GetNumDecisionCyclesExecuted() > pSynchAgent->GetIAgent()->GetNumDecisionCyclesExecuted() ||
				(pAgent->GetNumDecisionCyclesExecuted() == pSynchAgent->GetIAgent()->GetNumDecisionCyclesExecuted() &&
				IsPhaseLater(pAgent->GetCurrentPhase(), pSynchAgent->GetIAgent()->GetCurrentPhase())))
					pSynchAgent = pAgentSML ;
		}
	}

	return pSynchAgent ;
}

/*************************************************************************
* @brief	Returns true if all agents scheduled to run are in the same phase.
**************************************************************************/
bool RunScheduler::AreAgentsSynchronized(AgentSML* pSynchAgent)
{
	if (!pSynchAgent)
		return true ;

	bool same = true ;
	egSKIPhaseType phase = pSynchAgent->GetIAgent()->GetCurrentPhase() ;

	for (AgentMapIter iter = m_pKernelSML->m_AgentMap.begin() ; iter != m_pKernelSML->m_AgentMap.end() ; iter++)
	{
		AgentSML* pAgentSML = iter->second ;

		if (pAgentSML->IsAgentScheduledToRun())
		{
			gSKI::IAgent* pAgent = pAgentSML->GetIAgent() ;

			if (pAgent->GetCurrentPhase() != phase)
				same = false ;
		}
	}

	return same ;
}

/*************************************************************************
* @brief	Returns true if the given agent has reached the end of its run
**************************************************************************/
bool RunScheduler::IsAgentFinished(gSKI::IAgent* pAgent, AgentSML* pAgentSML, egSKIRunType runStepSize, unsigned long count)
{
	//unsigned long current = GetStepCounter(pAgent, runStepSize) ;
	//unsigned long initial = pAgentSML->GetInitialStepCount() ;
	unsigned long current = GetRunCounter(pAgent, runStepSize) ;
	unsigned long initial = pAgentSML->GetInitialRunCount() ;

	unsigned long difference = current - initial ;

	//fprintf(stdout, "Agent %s current is %d initial is %d diff is %d\n", pAgent->GetName(), current, initial, difference) ; fflush(stdout) ;

	bool finished = difference >= count && runStepSize != gSKI_RUN_FOREVER ;

	// If we're running by decision and we've run the appropriate number of decisions, then keep running until
	// we reach the correct phase where we should stop.
	egSKIPhaseType phase = pAgent->GetCurrentPhase() ;

	if (finished && runStepSize == gSKI_RUN_DECISION_CYCLE && m_StopBeforePhase != phase)
		finished = false ;

	return finished ;
}

/********************************************************************
* @brief	Some agents will complete a RunType sooner than others.
*			This list records who still hasn't finished.
*********************************************************************/
void RunScheduler::InitializeStepList()
{
	// We only check the agents that are scheduled to run.
	// This allows us to start <n> agents and have them drop out as they
	// finish one "runStepSize" 

	for (AgentMapIter iter = m_pKernelSML->m_AgentMap.begin() ; iter != m_pKernelSML->m_AgentMap.end() ; iter++)
	{
		AgentSML* pAgentSML = iter->second ;

		pAgentSML->PutAgentOnStepList(pAgentSML->IsAgentScheduledToRun()) ;
		 
	}
}

/********************************************************************
* @brief	Returns true if all currently active agents have not yet
*			completed a RunType.
*********************************************************************/
bool RunScheduler::AgentsStillStepping()
{
	// We only check the agents that are scheduled to run.
	// This allows us to start <n> agents and have them drop out as they
	// finish one "runStepSize" 

	for (AgentMapIter iter = m_pKernelSML->m_AgentMap.begin() ; iter != m_pKernelSML->m_AgentMap.end() ; iter++)
	{
		AgentSML* pAgentSML = iter->second ;

		if (pAgentSML->IsAgentScheduledToRun() && pAgentSML->IsAgentOnStepList())
			return true ;
	}

	return false ;
}

/*************************************************************************
* @brief	This event is fired to indicate that the agent is about to run.
**************************************************************************/
void RunScheduler::FireBeforeRunStartsEvents()
{
	for (AgentMapIter iter = m_pKernelSML->m_AgentMap.begin() ; iter != m_pKernelSML->m_AgentMap.end() ; iter++)
	{
		AgentSML* pAgentSML = iter->second ;

		if (pAgentSML->IsAgentScheduledToRun())
		{
			gSKI::IAgent* pAgent = pAgentSML->GetIAgent() ;
			pAgent->FireRunStartsEvent() ;
		}
	}
}

/********************************************************************
* @brief	Agents maintain a number of counters (for how many phase,
*			decisions etc.) they have ever executed.
*			We use these counters to determine when a run should stop.
*********************************************************************/
void RunScheduler::RecordInitialRunCounters(egSKIRunType runStepSize)
{
	for (AgentMapIter iter = m_pKernelSML->m_AgentMap.begin() ; iter != m_pKernelSML->m_AgentMap.end() ; iter++)
	{
		AgentSML* pAgentSML = iter->second ;

		if (pAgentSML->IsAgentScheduledToRun())
		{
 			gSKI::IAgent* pAgent = pAgentSML->GetIAgent() ;
			unsigned long count = GetStepCounter(pAgent, runStepSize) ;  
			pAgentSML->SetInitialStepCount(count) ; 
		}
	}
}

// The gSKI_Agent counters will maintain global values.  The counters in SML
// are used locally within a single call to RunScheduledAgents
void RunScheduler::InitializeRunCounters(egSKIRunType runStepSize, egSKIInterleaveType interleaveStepSize)
{
	for (AgentMapIter iter = m_pKernelSML->m_AgentMap.begin() ; iter != m_pKernelSML->m_AgentMap.end() ; iter++)
	{
		AgentSML* pAgentSML = iter->second ;

		if (pAgentSML->IsAgentScheduledToRun())
		{
 			gSKI::IAgent* pAgent = pAgentSML->GetIAgent() ;
			unsigned long count = GetRunCounter(pAgent, runStepSize) ;
			pAgentSML->SetInitialRunCount(count) ;
			//pAgentSML->SetInitialStepCount(0) ;
			count = GetStepCounter(pAgent, interleaveStepSize) ; 
			pAgentSML->SetInitialStepCount(count) ;
			pAgentSML->ResetLocalRunCounters() ;
		}
	}
} 
/********************************************************************
* @brief	Certain events are designed to fire "after the agents have done useful work".
*			We use these to update the environment.
*			The most common example is after all agents have completed the output phase
*			the environment is updated to reflect their actions.
*
*			Why do we use events to do this?  You could just call "run x" and then update the
*			world after the run completes.  Well, by listening for the event anyone can issue
*			the "run x" command--specifically either the debugger or the environment and
*			the environment still behaves correctly.
*********************************************************************/
void RunScheduler::InitializeUpdateWorldEvents(bool addListeners)
{
	for (AgentMapIter iter = m_pKernelSML->m_AgentMap.begin() ; iter != m_pKernelSML->m_AgentMap.end() ; iter++)
	{
		AgentSML* pAgentSML = iter->second ;

		pAgentSML->SetCompletedOutputPhase(false) ;
		pAgentSML->SetGeneratedOutput(false) ;
		pAgentSML->SetInitialOutputCount(pAgentSML->GetIAgent()->GetNumOutputsExecuted()) ;

		if (addListeners)
		{
			gSKI::IAgent* pAgent = pAgentSML->GetIAgent() ;
			//pAgent->AddRunListener(gSKIEVENT_AFTER_PHASE_EXECUTED, this) ;
			// This is a huge hack.  These need to be registered by a gSKI RunListener
			// because the gSKI event handler has to increment counters on these events.
			// Added here for now because I couldn't reconcile the code for making
			// a gSKI agent a RunListener too.  (might be circular...)
			pAgent->AddRunListener(gSKIEVENT_AFTER_ELABORATION_CYCLE, this) ;
			pAgent->AddRunListener(gSKIEVENT_AFTER_INPUT_PHASE, this) ;
			pAgent->AddRunListener(gSKIEVENT_AFTER_PROPOSE_PHASE, this) ;
			pAgent->AddRunListener(gSKIEVENT_AFTER_DECISION_PHASE, this) ;
			pAgent->AddRunListener(gSKIEVENT_AFTER_APPLY_PHASE, this) ;
			pAgent->AddRunListener(gSKIEVENT_AFTER_OUTPUT_PHASE, this) ;
			pAgent->AddRunListener(gSKIEVENT_AFTER_PREFERENCE_PHASE, this) ;  // Soar-7 mode only
			pAgent->AddRunListener(gSKIEVENT_AFTER_WM_PHASE, this) ;          // Soar-7 mode only
		}
	}
}

/********************************************************************
* @brief	Returns true if all currently active agents have completed
*			the output phase (they may not have generated output).
*********************************************************************/
bool RunScheduler::AreAllOutputPhasesComplete()
{
	// We only check the agents that are still scheduled to run.
	// This allows us to start <n> agents and have some drop out (stopped by user or breakpoint etc.) and still
	// generate the event.  However, it also means if we do a "run --self" to only run some agents this event will
	// still fire, so we'll need to know not to update the world based on the runFlags.
	for (AgentMapIter iter = m_pKernelSML->m_AgentMap.begin() ; iter != m_pKernelSML->m_AgentMap.end() ; iter++)
	{
		AgentSML* pAgentSML = iter->second ;

		if (pAgentSML->WasAgentOnRunList() && !pAgentSML->HasCompletedOutputPhase())
			return false ;
	}

	return true ;
}

/********************************************************************
* @brief	Returns true if all currently active agents have generated
*			output.  (This is a tighter requirement than just having
*			completed the output phase).
*********************************************************************/
bool RunScheduler::HaveAllGeneratedOutput()
{
	for (AgentMapIter iter = m_pKernelSML->m_AgentMap.begin() ; iter != m_pKernelSML->m_AgentMap.end() ; iter++)
	{
		AgentSML* pAgentSML = iter->second ;

		if (pAgentSML->WasAgentOnRunList() && !pAgentSML->HasGeneratedOutput())
			return false ;
	}

	return true ;
}

/********************************************************************
* @brief	Called when the agent completes a given phase.
*			We're interested in the end of the output phase.
*********************************************************************/
void RunScheduler::OldHandleEvent(egSKIRunEventId eventID, gSKI::IAgent* pAgent, egSKIPhaseType phase)
{
	if (eventID == gSKIEVENT_AFTER_PHASE_EXECUTED && phase == gSKI_OUTPUT_PHASE)
	{
		AgentSML* pAgentSML = m_pKernelSML->GetAgentSML(pAgent) ;
		pAgentSML->SetCompletedOutputPhase(true) ;

		// See if this was the last agent to complete the output phase
		if (AreAllOutputPhasesComplete())
		{
			// If so fire the after_all_output_phases event
			m_pKernelSML->FireUpdateListenerEvent(gSKIEVENT_AFTER_ALL_OUTPUT_PHASES, m_RunFlags) ;

			// Then clear the completed output flags and repeat the process.
			for (AgentMapIter iter = m_pKernelSML->m_AgentMap.begin() ; iter != m_pKernelSML->m_AgentMap.end() ; iter++)
			{
				AgentSML* pAgentSML = iter->second ;

				pAgentSML->SetCompletedOutputPhase(false) ;
			}
		}

		// If the number of outputs generated by this agent has changed record it as
		// having generated output and possibly fire a notification event.
		// InitialOutputCount is updated when the OutputGenerated event is fired.
		unsigned long count = pAgent->GetNumOutputsExecuted() ;
		if (count != pAgentSML->GetInitialOutputCount())
		{
			pAgentSML->SetGeneratedOutput(true) ;
			TestForFiringGeneratedOutputEvent() ;
		}
	}
}

/********************************************************************
* @brief	Called when the agent completes any phase.
*			 
*********************************************************************/
void RunScheduler::HandleEvent(egSKIRunEventId eventID, gSKI::IAgent* pAgent, egSKIPhaseType phase)
{		
	AgentSML* pAgentSML = m_pKernelSML->GetAgentSML(pAgent) ;

	if (IsAFTERPhaseEventID(static_cast<int>(eventID))) 
	{
        if (phase == gSKI_OUTPUT_PHASE)	
		{
		    pAgentSML->SetCompletedOutputPhase(true) ;
			// If the number of outputs generated by this agent has changed record it as
			// having generated output and possibly fire a notification event.
			// InitialOutputCount is updated when the OutputGenerated event is fired.
			unsigned long count = pAgent->GetNumOutputsExecuted() ;
			if (count != pAgentSML->GetInitialOutputCount())
			{
				pAgentSML->SetGeneratedOutput(true) ;
			}
		}
	} 
}

/********************************************************************
* @brief	Check if the "AFTER_ALL_GENERATED_OUTPUT" event should be
*			fired or not.
*********************************************************************/
void RunScheduler::TestForFiringGeneratedOutputEvent()
{
	// If the event has already been fired (for this step of the run) nothing to do.
	// This could happen if the last agent generates output and then stops running.
	if (m_AllGeneratedOutputEventFired)
		return ;

	// See if this was the last agent to generate output
	if (HaveAllGeneratedOutput())
	{
		// If all agents have generated output fire the event and reset the counters
		m_AllGeneratedOutputEventFired = true ;

		// If so fire the after_all_generated_output event
		m_pKernelSML->FireUpdateListenerEvent(gSKIEVENT_AFTER_ALL_GENERATED_OUTPUT, m_RunFlags) ;

		// Then clear the generated output flags and repeat the process.
		for (AgentMapIter iter = m_pKernelSML->m_AgentMap.begin() ; iter != m_pKernelSML->m_AgentMap.end() ; iter++)
		{
			AgentSML* pAgentSML = iter->second ;

			pAgentSML->SetGeneratedOutput(false) ;
			pAgentSML->SetInitialOutputCount(pAgentSML->GetIAgent()->GetNumOutputsExecuted()) ;
		}
	}
}

/********************************************************************
* @brief	Clean up for the update world events.
*********************************************************************/
void RunScheduler::TerminateUpdateWorldEvents(bool removeListeners)
{
	for (AgentMapIter iter = m_pKernelSML->m_AgentMap.begin() ; iter != m_pKernelSML->m_AgentMap.end() ; iter++)
	{
		AgentSML* pAgentSML = iter->second ;

		if (removeListeners)
		{
			gSKI::IAgent* pAgent = pAgentSML->GetIAgent() ;
			//pAgent->RemoveRunListener(gSKIEVENT_AFTER_PHASE_EXECUTED, this) ;
			// This is a huge hack.  These need to be registered by a gSKI RunListener
			// because the gSKI event handler has to increment counters on these events.
			// Added here for now because I couldn't reconcile the code for making
			// a gSKI agent a RunListener too.  (might be circular...)
			pAgent->RemoveRunListener(gSKIEVENT_AFTER_ELABORATION_CYCLE, this) ;
			pAgent->RemoveRunListener(gSKIEVENT_AFTER_INPUT_PHASE, this) ;
			pAgent->RemoveRunListener(gSKIEVENT_AFTER_PROPOSE_PHASE, this) ;
			pAgent->RemoveRunListener(gSKIEVENT_AFTER_DECISION_PHASE, this) ;
			pAgent->RemoveRunListener(gSKIEVENT_AFTER_APPLY_PHASE, this) ;
			pAgent->RemoveRunListener(gSKIEVENT_AFTER_OUTPUT_PHASE, this) ;
			pAgent->RemoveRunListener(gSKIEVENT_AFTER_PREFERENCE_PHASE, this) ;  // Soar-7 mode only
			pAgent->RemoveRunListener(gSKIEVENT_AFTER_WM_PHASE, this) ;          // Soar-7 mode only
	
		}
	}
}
/********************************************************************
* @brief	Checks each agent to see if it has finished running.
*			Returns true if all agents are done.
*			Does not remove anyone from the run list.
*********************************************************************/
bool RunScheduler::TestIfAllFinished(egSKIRunType runStepSize, unsigned long count)
{
	bool allDone = true ;

	for (AgentMapIter iter = m_pKernelSML->m_AgentMap.begin() ; iter != m_pKernelSML->m_AgentMap.end() ; iter++)
	{
		AgentSML* pAgentSML = iter->second ;

		bool agentFinishedRun = IsAgentFinished(pAgentSML->GetIAgent(), pAgentSML, runStepSize, count) ;

		if (!agentFinishedRun)
			allDone = false ;
	}

	return allDone ;
}

/********************************************************************
* @brief	Returns true if some agents are currently running.
*********************************************************************/
bool RunScheduler::IsRunning()
{
	return m_IsRunning ;
}

/*************************************************************
* @brief	Run all agents previously marked as being scheduled to run.
*
* @param runStepSize -- decision/phase etc.
* @param count		 -- how many steps to run
* @param runFlags	 -- type of run we're doing (passed back to environment)
* @param interleaveStepSize -- how large of a step each agent is run before other agents are run
* @param synchronize -- if true, synchronize all agents scheduled to run to the same phase before running all agents in step
* @param pError		 -- any error
*
* @return Not clear on how to set this when have multiple agents.
*		  Can query each for "GetLastRunResult()".
*************************************************************/	
egSKIRunResult RunScheduler::OldRunScheduledAgents(egSKIRunType runStepSize, unsigned long count, smlRunFlags runFlags, egSKIRunType interleaveStepSize, bool synchronize, gSKI::Error* pError)
{
	// In general we really want to assert that runStepSize >= interleaveStepSize but I'm not sure there's
	// a strict relationship and certainly the enums aren't labelled as being in a guaranteed processing size order.
	// So for now, we'll just check some specific cases.
	// If we're running by elaboration cycles, can't be running each agent by something bigger (like a phase).
	if (runStepSize == gSKI_RUN_ELABORATION_CYCLE)
		assert(interleaveStepSize == gSKI_RUN_ELABORATION_CYCLE) ;

	// If we want to synchronize the agents to the same phase, we need to run them by phase (or they'll never synch)
	if (synchronize)
		assert(interleaveStepSize == gSKI_RUN_PHASE) ;

	// We store this as a member so we can access it in gSKI event handlers
	m_RunFlags = runFlags ;

	// Record the current counter (that we're about to be incrementing)
	RecordInitialRunCounters(runStepSize) ;

	// Initialize state required for update world events
	InitializeUpdateWorldEvents(true) ;

	gSKI::IKernel* pKernel = m_pKernelSML->GetKernel() ;

	// Fire one event to indicate the entire system is starting
	pKernel->FireSystemStart() ;

	// Send event for each agent to signal its about to start running
	FireBeforeRunStartsEvents() ;

	bool runFinished = false ;
	long stepCount   = 0 ;
	egSKIRunResult overallResult = gSKI_RUN_COMPLETED ;

	pKernel->GetAgentManager()->ClearAllInterrupts();

	// Record that we're now running, so we can poll for our status during a run.
	m_IsRunning = true ;

	int interruptCheckRate = pKernel->GetInterruptCheckRate() ;

	// If we need to synchronize agents, we'll set the synchAgent pointer.
	// Otherwise, we'll clear it to indicate no synch needed.
	m_pSynchAgentSML = NULL ;
	if (synchronize)
	{
		AgentSML* pSynchAgentSML = this->GetAgentToSynchronizeWith() ;
		bool inSynch = AreAgentsSynchronized(pSynchAgentSML) ;

		if (!inSynch)
			m_pSynchAgentSML = pSynchAgentSML ;
	}

	// If we issue a "run 0" and all agents are synched and in the correct state we're done.
	if (!m_pSynchAgentSML && TestIfAllFinished(runStepSize, count))
		runFinished = true ;

	// Run all agents that have previously been marked as "scheduled to run".
	while (!runFinished)
	{
		// Assume it is finished until proven otherwise
		runFinished = true ;

		 // Callback to clients to see if they wish to stop this run.
		 // A client can use any event, but this one is designed to allow clients
		 // to throttle back the frequency of the event to control performance.
		if ((stepCount % interruptCheckRate) == 0)
			pKernel->FireInterruptCheckEvent() ;
		stepCount++ ;

		// Use a flag to make sure we only fire the all generated event once per step.
		// Otherwise, if the last agent generates output and then stops running we might try to fire it twice.
		m_AllGeneratedOutputEventFired = false ;

		 // Notify listeners that Soar is running.  This event is a kernel level (agent manager) event
		 // which allows a single listener to check for client driven interrupts for all agents.
		 // Sometimes that's easier to work with than the agent specific events (where you get <n> events from <n> agents)
		pKernel->GetAgentManager()->FireBeforeAgentsRunStepEvent() ;

		for (AgentMapIter iter = m_pKernelSML->m_AgentMap.begin() ; iter != m_pKernelSML->m_AgentMap.end() ; iter++)
		{
			AgentSML* pAgentSML = iter->second ;

			// This is only true if we are in the process of synching up to one agent and this agent is at the correct phase
			// In that case we don't run this agent for a step--which eventually means all agents will synch to the same point.
			bool synched = (m_pSynchAgentSML != NULL && pAgentSML->GetIAgent()->GetCurrentPhase() == m_pSynchAgentSML->GetIAgent()->GetCurrentPhase()) ;

			if (pAgentSML->IsAgentScheduledToRun() && !synched)
			{
				// Run all agents one "interleaveStepSize".
				gSKI::IAgent* pAgent = pAgentSML->GetIAgent() ;
				egSKIRunResult runResult = pAgent->RunInClientThread(interleaveStepSize, 1, pError) ;
 
				// Have to test the run state to find out if we are still ok to keep running
				// (not sure if runResult provides this as well, but they're from different enums).
				egSKIRunState runState = pAgent->GetRunState() ;

				// Decide if this agent has reached its run limit
				// Usually all agents will reach this limit at the same time.
				bool agentFinishedRun = IsAgentFinished(pAgent, pAgentSML, runStepSize, count) ;

				// An agent should return "stopped" if it's just pausing in the middle of a run
				// before we run it for the next phase.  Anything else means this agent is done running.
				if (runState != gSKI_RUNSTATE_STOPPED || agentFinishedRun)
				{
					pAgentSML->ScheduleAgentToRun(false) ;
					pAgentSML->SetResultOfRun(runResult) ;

					// If an agent stops running and the others have either stopped or have all generated output
					// we should fire the "ALL_GENERATED_OUTPUT" event.
					TestForFiringGeneratedOutputEvent() ;

					// Notify listeners that this agent is finished running
					pAgent->FireRunEndsEvent() ;
				}
				else
				{
					// If at least one agent wants to keep running, we keep running.
					runFinished = false ;
				}
			}
		}

		// If we're synching to an agent (i.e. matching the phases of all agents) see if we're done now.
		if (m_pSynchAgentSML)
		{
			// This isn't as optimally efficient as possible but only happens while synching agents (just a few phases)
			// and synching should be rare anyway.
			if (AreAgentsSynchronized(m_pSynchAgentSML))
			{
				m_pSynchAgentSML = NULL ;
				runFinished = TestIfAllFinished(runStepSize, count) ;
			}
		}
	}

	m_IsRunning = false ;

	// Clean up anything stored for update world events
	TerminateUpdateWorldEvents(false) ;

	// Fire one event to indicate the entire system should stop.
	pKernel->FireSystemStop() ;

	// Not sure how to quantify the results of running <n> agents in a single value
	// You can query each agent for its GetResultOfRun() to know more.
	return overallResult ;
}

/*************************************************************
* @brief	Run all agents previously marked as being scheduled to run.
*
* @param runStepSize -- decision/phase etc.
* @param count		 -- how many steps to run
* @param runFlags	 -- type of run we're doing (passed back to environment)
* @param interleaveStepSize -- how large of a step each agent is run before other agents are run
* @param synchronize -- if true, synchronize all agents scheduled to run to the same phase before running all agents in step
* @param pError		 -- any error
*
* @return Not clear on how to set this when have multiple agents.
*		  Can query each for "GetLastRunResult()".
*************************************************************/	
egSKIRunResult RunScheduler::RunScheduledAgents(egSKIRunType runStepSize, 
												   unsigned long count, 
												   smlRunFlags runFlags, 
												   egSKIInterleaveType interleaveStepSize, 
												   bool synchronize, 
												   gSKI::Error* pError)
{
    // Agents were already appropriately added (or not) to the RunList before calling this method.
	// For every Run Command issued, we can find out if agent is still scheduled to run,
	// and/or whether it was scheduled to run at all.  When agents stop or Halt, the
	// AgentScheduledToRun bool is set to false, but WasScheduledToRun is unchanged.

	// We store this as a member so we can access it in gSKI event handlers
	m_RunFlags = runFlags ;

	// Make sure the args of the Run command are valid.
	VerifyStepSizeForRunType(runStepSize, interleaveStepSize) ;

	// Record initial counts and zero the local "run" counter (that we're about to be incrementing)
 	InitializeRunCounters(runStepSize,interleaveStepSize) ;

 	gSKI::IKernel* pKernel = m_pKernelSML->GetKernel() ;

	// Fire one event to indicate the entire system is starting
	pKernel->FireSystemStart() ;

	// IF we did a StopBeforeUpdate, this is where we need to test and generate event...
	// TestForFiringOutputCompletedEvent();
	TestForFiringGeneratedOutputEvent();

	// Initialize state required for update world events
	// Should we do this even if previous Run was interrupted?  Probably not.
	InitializeUpdateWorldEvents(true) ;

	// Send event for each agent to signal its about to start running
	FireBeforeRunStartsEvents() ;

	bool runFinished = false ;
	long stepCount   = 0 ;
//	long runCount    = 0 ;
	egSKIRunResult overallResult = gSKI_RUN_COMPLETED ;

	pKernel->GetAgentManager()->ClearAllInterrupts();

	// Record that we're now running, so we can poll for our status during a run.
	m_IsRunning = true ;

	int interruptCheckRate = pKernel->GetInterruptCheckRate() ;
 
	// If we need to synchronize agents, we'll set the synchAgent pointer.
	// Otherwise, we'll clear it to indicate no synch needed.
	// This only matters when interleaving by Phases, since SoarKernel methods
	// will naturally synch agents interleaving by Decision or Output.
	// When interleaving by Elaboration cycles, synching can't be done.
	m_pSynchAgentSML = NULL ;
	if (synchronize && (gSKI_INTERLEAVE_PHASE == interleaveStepSize))
	{
		AgentSML* pSynchAgentSML = this->GetAgentToSynchronizeWith() ;
		bool inSynch = AreAgentsSynchronized(pSynchAgentSML) ;

		if (!inSynch)
			m_pSynchAgentSML = pSynchAgentSML ;
	}

	// If we issue a "run 0" and all agents are synched and in the correct state we're done.
	if (!m_pSynchAgentSML && TestIfAllFinished(runStepSize, count))
		runFinished = true ;


	// Run all agents that have previously been marked as "scheduled to run".
	while (!runFinished)
	{
		// Assume it is finished until proven otherwise
		runFinished = true ;

		 // Callback to clients to see if they wish to stop this run.
		 // A client can use any event, but this one is designed to allow clients
		 // to throttle back the frequency of the event to control performance.
		if ((stepCount % interruptCheckRate) == 0)
			pKernel->FireInterruptCheckEvent() ;
		stepCount++ ;

		//update relevant counters
		//pAgent->MaxNilOutputCyclesReached = false;

		//initialize stepList = copy of agentRunList
		InitializeStepList();


		// while not all agents have complete one RunType (agents still on stepList)
		while (AgentsStillStepping())
		{		
			for (AgentMapIter iter = m_pKernelSML->m_AgentMap.begin() ; iter != m_pKernelSML->m_AgentMap.end() ; iter++)
			{
				AgentSML* pAgentSML = iter->second ;

			//  KJC??  can we remove this synch code now that we synch on RunType ??
			// This is only true if we are in the process of synching up to one agent and this agent is at the correct phase
			// In that case we don't run this agent for a step--which eventually means all agents will synch to the same point.
		//	bool synched = (m_pSynchAgentSML != NULL && pAgentSML->GetIAgent()->GetCurrentPhase() == m_pSynchAgentSML->GetIAgent()->GetCurrentPhase()) ;
		//	if (pAgentSML->IsAgentScheduledToRun() && !synched)
 
				if (pAgentSML->IsAgentOnStepList())
				{			
					// Run all agents one "interleaveStepSize".		
					gSKI::IAgent* pAgent = pAgentSML->GetIAgent() ;			
					egSKIRunResult runResult = pAgent->StepInClientThread(interleaveStepSize, 1, pError) ;
					// ?? pAgentSML->IncrementLocalStepCounter();

					// if agent finished one runType, incr counter and remove from stepList				
					if (pAgentSML->CompletedRunType(GetRunCounter(pAgent,runStepSize)) /* || pAgent->MaxNilOutputCyclesReached */ )
				   {
					   pAgentSML->IncrementLocalRunCounter();
					   pAgentSML->PutAgentOnStepList(false);
				   } 
				   else 
				   {   
					   runFinished = false; 
				   }

		//         if agent finished count runTypes, remove from RunList, else runFinished = false;	
				   bool agentFinishedRun = IsAgentFinished(pAgent, pAgentSML, runStepSize, count) ;
	
				   // Have to test the run state to find out if we are still ok to keep running
			       // (not sure if runResult provides this as well, but they're from different enums).
				   egSKIRunState runState = pAgent->GetRunState() ;

				// An agent should return "stopped" if it's just pausing in the middle of a run
				// before we run it for the next phase.  Anything else means this agent is done running.
				if (runState != gSKI_RUNSTATE_STOPPED || agentFinishedRun)
				{
					pAgentSML->RemoveAgentFromRunList() ;
					pAgentSML->SetResultOfRun(runResult) ;
					// Notify listeners that this agent is finished running
					pAgent->FireRunEndsEvent() ;
				}
				else
				{
					// If at least one agent wants to keep running, we keep running.
					runFinished = false ;
				}					
				}
			}
		}
		// All agents have either halted or completed a RunType, check for 
		// any kernel-level events that are satisfied
		//  KJC Is this where we might want to add a "phase" for updating?   
		//  We'd need to use m_AllGeneratedOutputEventFired

		if (AreAllOutputPhasesComplete())
		{
			// If so fire the after_all_output_phases event
			m_pKernelSML->FireUpdateListenerEvent(gSKIEVENT_AFTER_ALL_OUTPUT_PHASES, m_RunFlags) ;

			// Then clear the completed output flags so we can repeat the process.
			for (AgentMapIter iter = m_pKernelSML->m_AgentMap.begin() ; iter != m_pKernelSML->m_AgentMap.end() ; iter++)
			{
				AgentSML* pAgentSML = iter->second ;
				pAgentSML->SetCompletedOutputPhase(false) ;
			}
		}
		TestForFiringGeneratedOutputEvent();


	}  // END of While (!runFinished)


	  // The code below isn't quite right now, but somehow we need to check that the
	  // agents have reached the StopBeforePhase phase.
		// If we're synching to an agent (i.e. matching the phases of all agents) see if we're done now.
		if (m_pSynchAgentSML)
		{
			// This isn't as optimally efficient as possible but only happens while synching agents (just a few phases)
			// and synching should be rare anyway.
			if (AreAgentsSynchronized(m_pSynchAgentSML))
			{
				m_pSynchAgentSML = NULL ;
				runFinished = TestIfAllFinished(runStepSize, count) ;
			}
		}
		// probably need to check for kernel-level events again after stepping to StopBeforePhase
		// might have synched with an agent at end of output, this would be the only place
		// we could check for generating outputComplete and outputGenerated events.

 	// Send event for each agent to signal it has finished running
	//  -->> no, each agent does this as it finishes... FireAfterRunEndsEvents() ;

	// Fire one event to indicate the entire system should stop.
	pKernel->FireSystemStop() ;

	// clean up
	m_IsRunning = false ;

	// Clean up anything stored for update world events
	TerminateUpdateWorldEvents(true) ;


	// Not sure how to quantify the results of running <n> agents in a single value
	// You can query each agent for its GetResultOfRun() to know more.
	return overallResult ;
}