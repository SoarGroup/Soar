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

using namespace sml ;

RunScheduler::RunScheduler(KernelSML* pKernelSML)
{
	m_pKernelSML = pKernelSML ;
	m_RunFlags = sml_NONE ;
	m_IsRunning = false ;
}

void RunScheduler::ScheduleAgentToRun(AgentSML* pAgentSML, bool state)
{
	if (pAgentSML)
		pAgentSML->ScheduleAgentToRun(state) ;
}

void RunScheduler::ScheduleAllAgentsToRun(bool state)
{
	for (AgentMapIter iter = m_pKernelSML->m_AgentMap.begin() ; iter != m_pKernelSML->m_AgentMap.end() ; iter++)
	{
		AgentSML* pAgentSML = iter->second ;
		pAgentSML->ScheduleAgentToRun(state) ;
	}
}

unsigned long RunScheduler::GetStepCounter(gSKI::IAgent* pAgent, egSKIRunType runStepSize)
{
	switch(runStepSize)
	{
	case gSKI_RUN_SMALLEST_STEP:
		return pAgent->GetNumSmallestStepsExecuted();
	case gSKI_RUN_PHASE:
		return pAgent->GetNumPhasesExecuted();
	case gSKI_RUN_ELABORATION_PHASE:
		return pAgent->GetNumElaborationsExecuted();
	case gSKI_RUN_DECISION_CYCLE:
		return pAgent->GetNumDecisionCyclesExecuted();
	case gSKI_RUN_UNTIL_OUTPUT:
		return pAgent->GetNumOutputsExecuted();
	default:
		return 0;
	}
}

bool RunScheduler::IsAgentFinished(gSKI::IAgent* pAgent, AgentSML* pAgentSML, egSKIRunType runStepSize, unsigned long count)
{
	unsigned long current = GetStepCounter(pAgent, runStepSize) ;
	unsigned long initial = pAgentSML->GetInitialStepCount() ;

	unsigned long difference = current - initial ;

	return (difference >= count && runStepSize != gSKI_RUN_FOREVER) ;
}

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

void RunScheduler::InitializeUpdateWorldEvents(bool addListeners)
{
	for (AgentMapIter iter = m_pKernelSML->m_AgentMap.begin() ; iter != m_pKernelSML->m_AgentMap.end() ; iter++)
	{
		AgentSML* pAgentSML = iter->second ;

		pAgentSML->SetCompletedOutputPhase(false) ;
		pAgentSML->SetGeneratedOutput(false) ;

		if (addListeners)
		{
			gSKI::IAgent* pAgent = pAgentSML->GetIAgent() ;
			pAgent->AddRunListener(gSKIEVENT_AFTER_PHASE_EXECUTED, this) ;
		}
	}
}

bool RunScheduler::AreAllOutputPhasesComplete()
{
	// We only check the agents that are still scheduled to run.
	// This allows us to start <n> agents and have some drop out (stopped by user or breakpoint etc.) and still
	// generate the event.  However, it also means if we do a "run --self" to only run some agents this event will
	// still fire, so we'll need to know not to update the world based on the runFlags.
	for (AgentMapIter iter = m_pKernelSML->m_AgentMap.begin() ; iter != m_pKernelSML->m_AgentMap.end() ; iter++)
	{
		AgentSML* pAgentSML = iter->second ;

		if (pAgentSML->IsAgentScheduledToRun() && !pAgentSML->HasCompletedOutputPhase())
			return false ;
	}

	return true ;
}

void RunScheduler::HandleEvent(egSKIRunEventId eventID, gSKI::IAgent* pAgent, egSKIPhaseType phase)
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
	}
}

void RunScheduler::TerminateUpdateWorldEvents(bool removeListeners)
{
	for (AgentMapIter iter = m_pKernelSML->m_AgentMap.begin() ; iter != m_pKernelSML->m_AgentMap.end() ; iter++)
	{
		AgentSML* pAgentSML = iter->second ;

		if (removeListeners)
		{
			gSKI::IAgent* pAgent = pAgentSML->GetIAgent() ;
			pAgent->RemoveRunListener(gSKIEVENT_AFTER_PHASE_EXECUTED, this) ;
		}
	}
}

bool RunScheduler::IsRunning()
{
	return m_IsRunning ;
}

egSKIRunResult RunScheduler::RunScheduledAgents(egSKIRunType runStepSize, unsigned long count, smlRunFlags runFlags, gSKI::Error* pError)
{
	// We store this as a member so we can access it in gSKI event handlers
	m_RunFlags = runFlags ;

	// Record the current counter (that we're about to be incrementing)
	RecordInitialRunCounters(runStepSize) ;

	// Initialize state required for update world events
	InitializeUpdateWorldEvents(true) ;

	gSKI::IKernel* pKernel = m_pKernelSML->GetKernel() ;

	// Fire one event to indicate the entire system is starting
	// (Above gSKI we can choose to suppress this event in some situations)
	pKernel->FireSystemStart() ;

	// Send event for each agent to signal its about to start running
	FireBeforeRunStartsEvents() ;

	bool runFinished = false ;
	long stepCount   = 0 ;
	egSKIRunResult overallResult = gSKI_RUN_COMPLETED ;

	pKernel->GetAgentManager()->ClearAllInterrupts();

	// Record that we're now running, so we can poll for our status during a run.
	m_IsRunning = true ;

	// Run all agents that have previously been marked as "scheduled to run".
	while (!runFinished)
	{
		// Assume it is finished until proven otherwise
		runFinished = true ;

		 // Callback to clients to see if they wish to stop this run.
		 // A client can use any event, but this one is designed to allow clients
		 // to throttle back the frequency of the event to control performance.
		if ((stepCount % pKernel->GetInterruptCheckRate()) == 0)
			pKernel->FireInterruptCheckEvent() ;
		stepCount++ ;

		 // Notify listeners that Soar is running.  This event is a kernel level (agent manager) event
		 // which allows a single listener to check for client driven interrupts for all agents.
		 // Sometimes that's easier to work with than the agent specific events (where you get <n> events from <n> agents)
		pKernel->GetAgentManager()->FireBeforeAgentsRunStepEvent() ;

		for (AgentMapIter iter = m_pKernelSML->m_AgentMap.begin() ; iter != m_pKernelSML->m_AgentMap.end() ; iter++)
		{
			AgentSML* pAgentSML = iter->second ;

			if (pAgentSML->IsAgentScheduledToRun())
			{
				// Run all agents one elaboration phase (or one full phase).
				// This is the smallest discrete unit we can use for running and stopping Soar.
				// Anything larger and we couldn't support a "run 1 -e" call (i.e. run one elaboration request at a higher level).
				gSKI::IAgent* pAgent = pAgentSML->GetIAgent() ;
				egSKIRunResult runResult = pAgent->RunInClientThread(gSKI_RUN_SMALLEST_STEP, 1, pError) ;

				// Have to test the run state to find out if we are still ok to keep running
				// (not sure if runResult provides this as well, but they're from different enums).
				egSKIRunState runState = pAgent->GetRunState() ;

				// Decide if this agent has reached its run limit
				// Usually all agents will reach this limit at the same time.  [Should this be a requirement?]
				bool agentFinishedRun = IsAgentFinished(pAgent, pAgentSML, runStepSize, count) ;

				// An agent should return "stopped" if it's just pausing in the middle of a run
				// before we run it for the next phase.  Anything else means this agent is done running.
				if (runState != gSKI_RUNSTATE_STOPPED || agentFinishedRun)
				{
					pAgentSML->ScheduleAgentToRun(false) ;
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

	m_IsRunning = false ;

	// Clean up anything stored for update world events
	TerminateUpdateWorldEvents(false) ;

	// Fire one event to indicate the entire system should stop.
	// (Above gSKI we can choose to suppress this event in some situations)
	pKernel->FireSystemStop() ;

	// Not sure how to quantify the results of running <n> agents in a single value
	return overallResult ;
}

