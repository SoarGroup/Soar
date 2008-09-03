/////////////////////////////////////////////////////////////////
// RunScheduler class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : May 2005
//
// Used to run Soar and send appropriate events so that an environment
// can function well in concert with a debugger.
//
/////////////////////////////////////////////////////////////////
// If this is defined, USE_OLD_SCHEDULER (in sml_OldRunScheduler.h) must not be
#define USE_NEW_SCHEDULER

#ifndef USE_NEW_SCHEDULER
#include "sml_OldRunScheduler.h"
#endif

#ifdef USE_NEW_SCHEDULER

#ifndef SML_RUN_SCHEDULER_H
#define SML_RUN_SCHEDULER_H

#include "gSKI_Enumerations.h"
#include "gSKI_Events.h"
#include "sml_Events.h"	// To get smlRunFlags

namespace gSKI {
	class IAgent ;
	struct Error ;
}

namespace sml {

// Forward declarations
class KernelSML ;
class AgentSML ;

class RunScheduler : public gSKI::IRunListener
{
protected:
	KernelSML*	m_pKernelSML ;
	smlRunFlags	m_RunFlags ;
	bool		m_IsRunning ;
	bool		m_AllGeneratedOutputEventFired ;

	// When running by decision stop before this phase runs.
	egSKIPhaseType m_StopBeforePhase ;

	// When running multiple agents, we synchronize them to this agent (same phase) before starting the real run.
	AgentSML*	m_pSynchAgentSML ;

public:
	RunScheduler(KernelSML* pKernelSML) ;

    /********************************************************************
    * @brief	This is a method for getting the default value
    *			for the interleaveStepSize for running agents.
	*
	* @return   interleaveStepSize -- how large of a step each agent is run 
	*           before other agents are run
    *********************************************************************/
	egSKIInterleaveType RunScheduler::DefaultInterleaveStepSize(egSKIRunType runStepSize) ;

    /********************************************************************
    * @brief	Don't try to Run with an nonsense interleaveStepSize
    *
	* @param runStepSize -- increment to run an agent, phase, decision etc.
 	* @param interleaveStepSize -- how large of a step each agent is run 
	*                              before other agents are run
    *********************************************************************/
    bool RunScheduler::VerifyStepSizeForRunType(egSKIRunType runStepSize, egSKIInterleaveType interleave) ;

	/*************************************************************
	* @brief	Indicate that the next time RunScheduledAgents() is called
	*			this agent should (or should not) run.
	*************************************************************/	
	void ScheduleAgentToRun(AgentSML* pAgentSML, bool state) ;

	/*************************************************************
	* @brief	Indicate that the next time RunScheduledAgents() is called
	*			all agents should (or should not) run.
	*************************************************************/	
	void ScheduleAllAgentsToRun(bool state) ;

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
	egSKIRunResult RunScheduledAgents(egSKIRunType runStepSize, unsigned long count, smlRunFlags runFlags, egSKIRunType interleaveStepSize, bool synchronize, gSKI::Error* pError) ;

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
	egSKIRunResult RunScheduledAgents(egSKIRunType runStepSize, unsigned long count, smlRunFlags runFlags, egSKIInterleaveType interleaveStepSize, bool synchronize, gSKI::Error* pError) ;

	/*************************************************************
	* @brief	Returns true if at least one agent is currently running.
	*************************************************************/	
	bool IsRunning() ;

	/*************************************************************
	* @brief	Returns true if at least any scheduled agent halted during run.
	*************************************************************/	
	bool AnAgentHaltedDuringRun() ;

	/*********************************************************************
	* @brief	Defines which phase we stop before when running by decision.
	*			E.g. Pass input phase to stop just after generating output and before receiving input.
	*			This is a setting which modifies the future behavior of "run <n> --decisions" commands.
	**********************************************************************/	
	void SetStopBefore(egSKIPhaseType phase)	{ m_StopBeforePhase = phase ; }
	egSKIPhaseType GetStopBefore()				{ return m_StopBeforePhase ; }

protected:
	bool            AgentsStillStepping() ;
	bool			AreAgentsSynchronized(AgentSML* pSynchAgent) ;
	bool			AllAgentsAtStopBeforePhase() ;
	bool			AreAllOutputPhasesComplete() ;
	void            MoveTo_StopBeforePhase(egSKIRunType runStepSize) ;
	void			FireBeforeRunStartsEvents() ;
	unsigned long	GetStepCounter(gSKI::IAgent* pAgent, egSKIRunType runStepSize) ; //for old scheduler...
    unsigned long   GetStepCounter(gSKI::IAgent* pAgent, egSKIInterleaveType stepSize) ;
	unsigned long	GetRunCounter(gSKI::IAgent* pAgent, egSKIRunType runStepSize) ;
    egSKIRunResult  RunScheduler::GetOverallRunResult() ;
	void			HandleEvent(egSKIRunEventId eventID, gSKI::IAgent* pAgent, egSKIPhaseType phase) ;
	bool			HaveAllGeneratedOutput() ;
	void            InitializeRunCounters(egSKIRunType runStepSize, egSKIInterleaveType stepSize) ;
    void            InitializeStepList() ;
	void			InitializeUpdateWorldEvents(bool addListeners) ;
	bool			IsAgentFinished(gSKI::IAgent* pAgent, AgentSML* pAgentSML, egSKIRunType runStepSize, unsigned long count) ;
 	void			RecordInitialRunCounters(egSKIRunType runStepSize) ;
 	void			ResetRunCounters(egSKIRunType runStepSize) ;
	void			TerminateUpdateWorldEvents(bool removeListeners) ;
	void			TestForFiringUpdateWorldEvents();
	bool			TestIfAllFinished(egSKIRunType runStepSize, unsigned long count) ;

	AgentSML*		GetAgentToSynchronizeWith() ;
} ;

} // namespace

#endif // SML_RUN_SCHEDULER_H

#endif // USE_NEW_SCHEDULER

