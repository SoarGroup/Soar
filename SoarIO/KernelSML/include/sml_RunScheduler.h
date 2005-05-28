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

#ifndef SML_RUN_SCHEDULERH
#define SML_RUN_SCHEDULER_H

#include "gSKI_Enumerations.h"
#include "gSKI_Events.h"

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
	int			m_RunFlags ;

public:
	RunScheduler(KernelSML* pKernelSML) { m_pKernelSML = pKernelSML ; m_RunFlags = 0 ; }

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
	* @param pError		 -- any error
	*
	* @return Not clear on how to set this when have multiple agents.
	*		  Can query each for "GetLastRunResult()".
	*************************************************************/	
	egSKIRunResult RunScheduledAgents(egSKIRunType runStepSize, unsigned long count, int runFlags, gSKI::Error* pError) ;

protected:
	bool			IsAgentFinished(gSKI::IAgent* pAgent, AgentSML* pAgentSML, egSKIRunType runStepSize, unsigned long count) ;
	void			FireBeforeRunStartsEvents() ;
	unsigned long	GetStepCounter(gSKI::IAgent* pAgent, egSKIRunType runStepSize) ;
	void			RecordInitialRunCounters(egSKIRunType runStepSize) ;
	void			InitializeUpdateWorldEvents(bool addListeners) ;
	void			TerminateUpdateWorldEvents(bool removeListeners) ;
	void			HandleEvent(egSKIRunEventId eventID, gSKI::IAgent* pAgent, egSKIPhaseType phase) ;
	bool			AreAllOutputPhasesComplete() ;

} ;

} // namespace

#endif // SML_RUN_SCHEDULER_H
