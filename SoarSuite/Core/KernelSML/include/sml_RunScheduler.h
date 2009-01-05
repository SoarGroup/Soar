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
#ifndef SML_RUN_SCHEDULER_H
#define SML_RUN_SCHEDULER_H

#include "sml_Events.h"

namespace sml {

// Forward declarations
class KernelSML ;
class AgentSML ;

class RunScheduler
{
protected:
	KernelSML*	m_pKernelSML ;
	smlRunFlags	m_RunFlags ;
	bool		m_IsRunning ;

	// When running by decision stop before this phase runs.
	smlPhase m_StopBeforePhase ;

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
	smlRunStepSize DefaultInterleaveStepSize(bool forever, smlRunStepSize runStepSize) ;

    /********************************************************************
    * @brief	Don't try to Run with an nonsense interleaveStepSize
    *
	* @param runStepSize -- increment to run an agent, phase, decision etc.
 	* @param interleaveStepSize -- how large of a step each agent is run 
	*                              before other agents are run
    *********************************************************************/
    bool VerifyStepSizeForRunType(bool forever, smlRunStepSize runStepSize, smlRunStepSize interleave) ;

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
	smlRunResult RunScheduledAgents(bool forever, smlRunStepSize runStepSize, unsigned long count, smlRunFlags runFlags, smlRunStepSize interleaveStepSize, bool synchronize) ;

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
	void SetStopBefore(smlPhase phase)	{ m_StopBeforePhase = phase ; }
	smlPhase GetStopBefore()				{ return m_StopBeforePhase ; }

protected:
	bool            AgentsStillStepping() ;
	bool			AreAgentsSynchronized(AgentSML* pSynchAgent) ;
	bool			AllAgentsAtStopBeforePhase() ;
	bool			AreAllOutputPhasesComplete() ;
	void            MoveTo_StopBeforePhase(bool forever, smlRunStepSize runStepSize) ;
	void			FireBeforeRunStartsEvents() ;
    smlRunResult  GetOverallRunResult() ;
	bool			HaveAllGeneratedOutput() ;
	void            InitializeRunCounters(smlRunStepSize runStepSize) ;
    void            InitializeStepList() ;
	void			InitializeUpdateWorldEvents(bool addListeners) ;
	bool			IsAgentFinished(AgentSML* pAgentSML, bool forever,  smlRunStepSize runStepSize, unsigned long count) ;
 	void			ResetRunCounters(smlRunStepSize runStepSize) ;
	void			TerminateUpdateWorldEvents(bool removeListeners) ;
	void			TestForFiringUpdateWorldEvents();
	bool			TestIfAllFinished(bool forever, smlRunStepSize runStepSize, unsigned long count) ;

	AgentSML*		GetAgentToSynchronizeWith() ;
} ;

} // namespace

#endif // SML_RUN_SCHEDULER_H
