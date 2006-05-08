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

#ifdef USE_NEW_SCHEDULER

#include "sml_KernelSML.h"
#include "sml_AgentSML.h"
#include "sml_Events.h"

#include "../../gSKI/src/gSKI_Error.h"
#include "IgSKI_AgentManager.h"

#include <assert.h>

using namespace sml ;

RunScheduler::RunScheduler(KernelSML* pKernelSML)
{
	m_pKernelSML = pKernelSML ;
	m_RunFlags = sml_NONE ;
	m_IsRunning = false ;
	m_StopBeforePhase = gSKI_APPLY_PHASE ;
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
		//return gSKI_INTERLEAVE_PHASE;		 // tested ok in SoarJavaDebugger 01/18/06
		return gSKI_INTERLEAVE_DECISION_CYCLE;
	case gSKI_RUN_UNTIL_OUTPUT:
		//return gSKI_INTERLEAVE_PHASE;      // tested ok in SoarJavaDebugger 01/18/06
		//return gSKI_INTERLEAVE_DECISION_CYCLE;  // tested ok in SoarJavaDebugger 01/18/06
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
		return (runStepSize == gSKI_RUN_SMALLEST_STEP);
	case gSKI_RUN_PHASE:
		return ( gSKI_INTERLEAVE_PHASE == interleave ) ;
	case gSKI_RUN_ELABORATION_CYCLE:
		return ( gSKI_INTERLEAVE_ELABORATION_PHASE == interleave ) ;
	case gSKI_RUN_DECISION_CYCLE:
		return (gSKI_INTERLEAVE_PHASE == interleave || 
			   gSKI_INTERLEAVE_ELABORATION_PHASE == interleave || 
			   gSKI_INTERLEAVE_DECISION_CYCLE == interleave) ;
	case gSKI_RUN_UNTIL_OUTPUT:
		return (gSKI_INTERLEAVE_PHASE == interleave || 
			   gSKI_INTERLEAVE_ELABORATION_PHASE == interleave || 
			   gSKI_INTERLEAVE_DECISION_CYCLE == interleave ||
			   gSKI_INTERLEAVE_OUTPUT == interleave) ;
	case gSKI_RUN_FOREVER:
		// can interleave by any egSKIInterleaveType
		return (gSKI_INTERLEAVE_PHASE == interleave || 
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
	case gSKI_RUN_FOREVER:
		return pAgent->GetNumDecisionCyclesExecuted();
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
* @brief	Agent will run one fewer RunType if not at m_StopBeforePhase.
**************************************************************************/
bool RunScheduler::AllAgentsAtStopBeforePhase()
{
    bool same = true ;
	for (AgentMapIter iter = m_pKernelSML->m_AgentMap.begin() ; iter != m_pKernelSML->m_AgentMap.end() ; iter++)
	{
		AgentSML* pAgentSML = iter->second ;

		if (pAgentSML->IsAgentScheduledToRun())
		{
			gSKI::IAgent* pAgent = pAgentSML->GetIAgent() ;

			if (pAgent->GetCurrentPhase() != m_StopBeforePhase)
			{
				//don't let this agent continue to run ahead
//				pAgentSML->IncrementLocalRunCounter() ;
//				pAgentSML->SetInitialRunCount(1+(pAgentSML->GetInitialRunCount())) ;  
				same = false;
			}
		}
	}
	return same;
}

/*************************************************************************
* @brief	Returns true if the given agent has reached the end of its run
**************************************************************************/
bool RunScheduler::IsAgentFinished(gSKI::IAgent* pAgent, AgentSML* pAgentSML, egSKIRunType runStepSize, unsigned long count)
{
	unsigned long current = GetRunCounter(pAgent, runStepSize) ;
	unsigned long initial = pAgentSML->GetInitialRunCount() ;

	unsigned long difference = current - initial ;

	//fprintf(stdout, "Agent %s current is %d initial is %d diff is %d\n", pAgent->GetName(), current, initial, difference) ; fflush(stdout) ;

	bool finished = difference >= count && runStepSize != gSKI_RUN_FOREVER ;

						   
	// if a gSKI_STOP_AFTER_DECISION_CYCLE is requested,  then
	// agents that are running by Decisions should get marked as finished.
	// They will generate interrupt in MoveTo_StopBeforePhase
					   
	if ( ((gSKI_RUN_DECISION_CYCLE == runStepSize) || (gSKI_RUN_FOREVER == runStepSize)) &&
		  (pAgent->GetInterruptFlags() & gSKI_STOP_AFTER_DECISION_CYCLE) )					  
	{
		finished = true;					 
	}

	// The code that runs an agent to its appropriate StopBeforePhase only runs
	// after all agents Finish the run.  See MoveTo_StopBeforePhase.   KJC 12/05

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
			pAgent->ResetNilOutputCounter();
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

		// We register a listener so that the agent counters/flags get updated at the end of Output.
		if (addListeners)
		{
			gSKI::IAgent* pAgent = pAgentSML->GetIAgent() ;
			pAgent->AddRunListener(gSKIEVENT_AFTER_OUTPUT_PHASE, this) ;
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
	bool agents_running = false;

	for (AgentMapIter iter = m_pKernelSML->m_AgentMap.begin() ; iter != m_pKernelSML->m_AgentMap.end() ; iter++)
	{
		AgentSML* pAgentSML = iter->second ;

		// Agents that are halted or interrupted are no longer m_ScheduledToRun
		// Agents that are paused waiting for other agents finish a RunType, are still m_ScheduledToRun
		if (pAgentSML->IsAgentScheduledToRun() )
		{
			agents_running = true;
			if (!pAgentSML->HasCompletedOutputPhase())
				return false;
		} 
	}	
		
	// If all running agents completed output, we'll get here.  BUT we could also reach this
	// point if ALL agents are interrupted/halted, and none are m_ScheduledToRun

	if (agents_running) 
		return true;  // we got here only if there are running agents and they all completed output
	else
	{
		// ALL the agents from this run are halted or interrupted.  Check interrupted agents 
		// to see if any of them stopped after output, then return true.  We don't force ALL
		// agents to be at end of output, because some may have been RHS interrupted 
		// or --self earlier and there's no way to tell the difference.
		for (AgentMapIter iter = m_pKernelSML->m_AgentMap.begin() ; iter != m_pKernelSML->m_AgentMap.end() ; iter++)
		{
			AgentSML* pAgentSML = iter->second ;	
			if (pAgentSML->WasAgentOnRunList() && 	
				(gSKI_RUNSTATE_HALTED != (pAgentSML->GetIAgent()->GetRunState())) &&	
				pAgentSML->HasCompletedOutputPhase())	
				return true;
		}
	}

	// IF we're interrupted at the end of Decision_cycle, then we SHOULD return true above, 
	// although if somehow we don't, it's possible that agents will SNC one cycle waiting for
	// I/O to catch up.  But if we returned true here, we could get many false positive events.

	return false ;
}

/********************************************************************
* @brief	Returns true if all currently active agents have generated
*			output.  (This is a tighter requirement than just having
*			completed the output phase).
*           Called only after HaveAllCompletedOutput is true, and that
*           method worries about interrupted and halted agents, so this
*           method doesn't have to duplicate that logic.
*********************************************************************/
bool RunScheduler::HaveAllGeneratedOutput()
{
	for (AgentMapIter iter = m_pKernelSML->m_AgentMap.begin() ; iter != m_pKernelSML->m_AgentMap.end() ; iter++)
	{
		AgentSML* pAgentSML = iter->second ;

		// Agents that are halted or interrupted are no longer m_ScheduledToRun
		// Agents that are paused waiting for other agents finish a RunType, are still m_ScheduledToRun
		if (pAgentSML->IsAgentScheduledToRun()&& !pAgentSML->HasGeneratedOutput())
			return false ;
	}

	return true ;
}
/********************************************************************
* @brief	 Users and applications can choose to have agents always
*			 stop before a particular phase.  If all agents have finished
*            finished running, this routine will check whether they
*            need to be stepped to a different phase.
*********************************************************************/
void RunScheduler::MoveTo_StopBeforePhase(egSKIRunType runStepSize)
{
	// If we ran by decision_cycle and we've run the appropriate number of decisions, 
	// then  step agent until it reached the correct phase where it should stop.  
	// Just before this point, (in scheduler while loop) we've checked and fired the
	// OutputComplete and OutputGenerated events, so it should be fine to step 
	// an agent by phases until it gets to StopBefore, and then step the other
	// agents in turn.   
	//                    
	// If an interrupt was requested, it will be detected inside StepInClientThread 
	// and the appropriate events will be generated.  The runResult will come back as
	// INTERRUPTED, and the while loops will exit so that the run stops.
	//
	// Support for "Run 0" is somewhat complicated, since we might very well 
	// cross the I/O boundary.  So we'll try stepping til OUTPUT done, then generate
	// Update events, then step again til StopBeforePt is reached.  Note:  when we support
	// StopBeforeUpdateWorld, we'll need to explicitly check for that before generating events.

	if (( runStepSize == gSKI_RUN_DECISION_CYCLE) || ( runStepSize == gSKI_RUN_FOREVER))
	{
		for (AgentMapIter iter = m_pKernelSML->m_AgentMap.begin() ; iter != m_pKernelSML->m_AgentMap.end() ; iter++)	
		{	
			AgentSML* pAgentSML = iter->second ;		
			if (pAgentSML->WasAgentOnRunList()) 
			{
				gSKI::IAgent* pAgent = pAgentSML->GetIAgent() ;			
				egSKIPhaseType phase = pAgent->GetCurrentPhase() ;
				egSKIRunResult runResult = pAgentSML->GetResultOfLastRun() ;
				if (! pAgent->GetOperand2Mode()) continue;  // we don't support for agents in Soar7 mode...

				// as in Bug 648, it's possible that a client has requested STOP_AFTER_DECISION
				// while the agent was stopped at the m_StopBeforePhase, yet the agent run logic has
				// dropped thru to this point without generating the interrupt yet.
				if ((m_StopBeforePhase == phase) && (pAgent->GetRunState() == gSKI_RUNSTATE_STOPPED) &&
					(pAgent->GetInterruptFlags() & gSKI_STOP_AFTER_DECISION_CYCLE))
				{
					pAgent->SetRunState(gSKI_RUNSTATE_INTERRUPTED);
					runResult = pAgent->StepInClientThread(gSKI_INTERLEAVE_PHASE, 1) ; // force interrupt
				}					

				while ((phase != m_StopBeforePhase) && (gSKI_RUN_COMPLETED == runResult))
				{
					runResult = pAgent->StepInClientThread(gSKI_INTERLEAVE_PHASE, 1) ;
					phase = pAgent->GetCurrentPhase() ;
					// don't cross Output-Input boundary here just to get to StopBefore phase for "Run 0"
					if (gSKI_INPUT_PHASE == phase) break; 
				}						
				pAgentSML->SetResultOfRun(runResult) ;

				// Notify listeners that this agent is finished running
				/* pAgent->FireRunEndsEvent() ; */   // not if supporting run0 over O/I boundary
			}
		}
		// If agents haven't reached the StopBeforePhase, then agents finished output
		// and we need to possibly generate the UpdateWorld events before stepping any further.
		// If not all agents finished output, then nothing will fire.
		TestForFiringUpdateWorldEvents();
  			
		// This second While loop is only needed if we allow agents to cross the
		// output-update-input boundary on a "run 0" command.  If we don't allow that
		// then all that is left to do is generate the RunEndsEvents. (and we shouldn't
		// even do the update events above.  The FireRunEnds event can go in the 
		// first FOR loop.
		for (AgentMapIter iter = m_pKernelSML->m_AgentMap.begin() ; iter != m_pKernelSML->m_AgentMap.end() ; iter++)	
		{	
			AgentSML* pAgentSML = iter->second ;				
			if (pAgentSML->WasAgentOnRunList()) 
			{
				gSKI::IAgent* pAgent = pAgentSML->GetIAgent() ;			
				egSKIPhaseType phase = pAgent->GetCurrentPhase() ;
				egSKIRunResult runResult = pAgentSML->GetResultOfLastRun() ;
				if (! pAgent->GetOperand2Mode()) continue;  // we don't support for agents in Soar7 mode...
 
				while ((phase != m_StopBeforePhase) && (gSKI_RUN_COMPLETED == runResult))
				{
					runResult = pAgent->StepInClientThread(gSKI_INTERLEAVE_PHASE, 1) ;
					phase = pAgent->GetCurrentPhase() ;
					// We should never get to this point again, so we need to generate ERROR 
					assert(gSKI_INPUT_PHASE != phase);
				}		

				pAgentSML->SetResultOfRun(runResult) ;

				// Notify listeners that this agent is finished running
				pAgent->FireRunEndsEvent() ;
			}
		}
	}
}

/********************************************************************
* @brief	Called when the agent completes an output phase.
*			used to be Called when the agent completed any phase.
*           1/7/06:  KJC registered runListener only on AFTER_OUTPUT_PHASE
*			 
*********************************************************************/
void RunScheduler::HandleEvent(egSKIRunEventId eventID, gSKI::IAgent* pAgent, egSKIPhaseType phase)
{
	unused(phase) ;
	AgentSML* pAgentSML = m_pKernelSML->GetAgentSML(pAgent) ;

    if (gSKIEVENT_AFTER_OUTPUT_PHASE == eventID) 	
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

/********************************************************************
* @brief	Check if the "AFTER_ALL_OUTPUT_PHASES" event should be
*			fired or not.  Then check if the "AFTER_ALL_GENERATED_OUTPUT" 
*           event should be fired or not.
*********************************************************************/
void RunScheduler::TestForFiringUpdateWorldEvents()
{
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
			// the GeneratedOutput can only be true if output phases completed was true
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
			pAgent->RemoveRunListener(gSKIEVENT_AFTER_OUTPUT_PHASE, this) ;
		}
	}
}
/********************************************************************
* @brief	Return most severe RunResult of
            gSKI_RUN_ERROR,
            gSKI_RUN_EXECUTING,
            gSKI_RUN_INTERRUPTED,
            gSKI_RUN_COMPLETED,
            gSKI_RUN_COMPLETED_AND_INTERRUPTED 

*********************************************************************/
egSKIRunResult RunScheduler::GetOverallRunResult()
{
	egSKIRunResult overallResult = gSKI_RUN_COMPLETED;

	for (AgentMapIter iter = m_pKernelSML->m_AgentMap.begin() ; iter != m_pKernelSML->m_AgentMap.end() ; iter++)
	{
		AgentSML* pAgentSML = iter->second ;
		egSKIRunResult result = pAgentSML->GetResultOfLastRun();
	//	if (result < overallResult) overallResult = result;
		if (result == gSKI_RUN_INTERRUPTED) overallResult = gSKI_RUN_INTERRUPTED;
	}
	return overallResult ;
}

/********************************************************************
* @brief	Checks to see if any-agents on the runlist were halted.
*			Returns true if any agent has runstate == halted.
*			Does not remove anyone from the run list.
*********************************************************************/
bool RunScheduler::AnAgentHaltedDuringRun()
{
	for (AgentMapIter iter = m_pKernelSML->m_AgentMap.begin() ; iter != m_pKernelSML->m_AgentMap.end() ; iter++)
	{
		AgentSML* pAgentSML = iter->second ;
		if (pAgentSML->WasAgentOnRunList() )
		{
			gSKI::IAgent* pAgent = pAgentSML->GetIAgent() ;
			if (pAgent->GetRunState() == gSKI_RUNSTATE_HALTED) return true;
		}
	}
	return false;
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

	// Depending on RunType, set the stop location for gSKI_STOP_AFTER_DECISION_CYCLE interrupts
	pKernel->SetStopPoint(runStepSize, m_StopBeforePhase);

	// Fire one event to indicate the entire system is starting
	pKernel->FireSystemStart() ;

	// IF we did a StopBeforeUpdate, this is where we need to test and generate update events...
	TestForFiringUpdateWorldEvents();
 	m_AllGeneratedOutputEventFired = false ;

	// Initialize state required for update world events
	// Should we do this even if previous Run was interrupted?  Probably not.
	InitializeUpdateWorldEvents(true) ;

	// Send event for each agent to signal its about to start running
	FireBeforeRunStartsEvents() ;

	bool runFinished = false ;
	long stepCount   = 0 ;
//	long runCount    = 0 ;
	egSKIRunResult overallResult = gSKI_RUN_COMPLETED ;	
	for (AgentMapIter iter = m_pKernelSML->m_AgentMap.begin() ; iter != m_pKernelSML->m_AgentMap.end() ; iter++)		
	{		
		AgentSML* pAgentSML = iter->second ;
		if (pAgentSML->IsAgentScheduledToRun()) pAgentSML->SetResultOfRun(gSKI_RUN_COMPLETED) ;
	}

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
	if (synchronize && (gSKI_INTERLEAVE_ELABORATION_PHASE != interleaveStepSize))
	{
		AgentSML* pSynchAgentSML = this->GetAgentToSynchronizeWith() ;
		bool inSynch = AreAgentsSynchronized(pSynchAgentSML) ;

		if (!inSynch)
			m_pSynchAgentSML = pSynchAgentSML ;
	}

	//  Before running, check to see if an agent is at a point other than the StopBeforePhase.
	//  If so, we'll decrement  the RunCount before entering the Run loop so  
	//  as not to run more Decision phases than specified in the runCount.  See bug #710.
	if (gSKI_RUN_DECISION_CYCLE == runStepSize) 
		 if (!AllAgentsAtStopBeforePhase() && (count > 0)) count--;
 

	// If we issue a "run 0" and all agents are synched and in the correct state we're done.
	if (!m_pSynchAgentSML && TestIfAllFinished(runStepSize, count))
		runFinished = true ;

	// Run all agents that have previously been marked as "scheduled to run".
	while (!runFinished)
	{
		// Assume it is finished until proven otherwise
		runFinished = true ;

		//initialize stepList = copy of agentRunList
		InitializeStepList();

		// while not all agents have complete one RunType (agents still on stepList)
		while (AgentsStillStepping())
		{		
			// Callback to clients to see if they wish to stop this run.
			// A client can use any event, but this one is designed to allow clients
			// to throttle back the frequency of the event to control performance.
			if ((stepCount % interruptCheckRate) == 0)
				pKernel->FireInterruptCheckEvent() ;
			stepCount++ ;

			// Notify listeners that Soar is going to run.  This event is a kernel level (agent manager) event
			// which allows a single listener to check for client driven interrupts for all agents.
			// Sometimes that's easier to work with than the agent specific events (where you get <n> events from <n> agents)
			//    note that there is not a corresponding AFTER_AGENTS_RUN_STEP event...
			pKernel->GetAgentManager()->FireBeforeAgentsRunStepEvent() ;

			for (AgentMapIter iter = m_pKernelSML->m_AgentMap.begin() ; iter != m_pKernelSML->m_AgentMap.end() ; iter++)
			{
				AgentSML* pAgentSML = iter->second ;
 
				if (pAgentSML->IsAgentOnStepList())
				{			
					// Run all agents one "interleaveStepSize".		
					gSKI::IAgent* pAgent = pAgentSML->GetIAgent() ;			
					egSKIRunResult runResult = pAgent->StepInClientThread(interleaveStepSize, 1, pError) ;
					// ?? pAgentSML->IncrementLocalStepCounter();

					// halted and running agents will return an error from StepInClientThread
					// 

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

				   // if agent finished count runTypes, remove from RunList, else runFinished = false;	
				   // can also return true if a gSKI_STOP_AFTER_DECISION_CYCLE interrupt occurred 
				   // or is pending on agents with RunType DECISION or FOREVER.
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
					// If we know we won't have to step to StopBefore phase
					// notify listeners that this agent is finished running
					if ((runStepSize != gSKI_RUN_DECISION_CYCLE) && (runStepSize != gSKI_RUN_FOREVER))
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
		//  KJC Is this where we might want to add a "phase" for StopBeforePhase?   
		//  We'd need to use m_AllGeneratedOutputEventFired
	    TestForFiringUpdateWorldEvents();

		// Check for whether the kernel events requested a stop-soar.
		if (TestIfAllFinished(runStepSize, count))
			runFinished = true ;

	}  // END of While (!runFinished)

	// kernel events might fire in next method...
	MoveTo_StopBeforePhase(runStepSize);  // agents will have done FireRunEndsEvent() here or above.

	// Fire one event to indicate the entire system should stop.
	pKernel->FireSystemStop() ;

	// clean up
	m_IsRunning = false ;

	// Clean up anything stored for update world events
	TerminateUpdateWorldEvents(true) ;

	// Not sure how to quantify the results of running <n> agents in a single value
	// You can query each agent for its GetResultOfRun() to know more.

	overallResult = GetOverallRunResult();

	return overallResult ;
}

#endif // USE_NEW_SCHEDULER
