/////////////////////////////////////////////////////////////////
// AgentListener class file.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : September 2004
//
// This class's HandleEvent method is called when
// specific events occur within the agent:
//
/*
*     // Agent events
*     @li gSKIEVENT_BEFORE_SMALLEST_STEP
*     @li gSKIEVENT_AFTER_SMALLEST_STEP
*     @li gSKIEVENT_BEFORE_ELABORATION_CYCLE
*     @li gSKIEVENT_AFTER_ELABORATION_CYCLE
*     @li gSKIEVENT_BEFORE_PHASE_EXECUTED
*     @li gSKIEVENT_AFTER_PHASE_EXECUTED
*     @li gSKIEVENT_BEFORE_DECISION_CYCLE
*     @li gSKIEVENT_AFTER_DECISION_CYCLE
*     @li gSKIEVENT_AFTER_INTERRUPT
*     @li gSKIEVENT_BEFORE_RUNNING
*     @li gSKIEVENT_AFTER_RUNNING
*
*      // Agent manager events
*      gSKIEVENT_AFTER_AGENT_CREATED,
*      gSKIEVENT_BEFORE_AGENT_DESTROYED,
*      gSKIEVENT_BEFORE_AGENT_REINITIALIZED,
*      gSKIEVENT_AFTER_AGENT_REINITIALIZED,
*
*      // Production Manager
*      gSKIEVENT_AFTER_PRODUCTION_ADDED,
*      gSKIEVENT_BEFORE_PRODUCTION_REMOVED,
*     //gSKIEVENT_BEFORE_PRODUCTION_FIRED,
*      gSKIEVENT_AFTER_PRODUCTION_FIRED,
*      gSKIEVENT_BEFORE_PRODUCTION_RETRACTED,
*
*/
/////////////////////////////////////////////////////////////////
#ifndef AGENT_LISTENER_H
#define AGENT_LISTENER_H

#include "gSKI_Events.h"
#include "gSKI_Enumerations.h"
#include "IgSKI_Iterator.h"
#include "IgSKI_Agent.h"
#include "IgSKI_Kernel.h"
#include "sml_EventManager.h"

#include <map>

namespace sml {

class KernelSML ;
class Connection ;

class AgentListener : public gSKI::IRunListener, public gSKI::IAgentListener, public gSKI::IProductionListener, public EventManager
{
protected:
	KernelSML*		m_KernelSML ;
	gSKI::IAgent*	m_Agent ;

	bool IsRunEvent(egSKIEventId id)
	{
		switch (id)
		{
			// Agent listener events
			case gSKIEVENT_BEFORE_SMALLEST_STEP:
			case gSKIEVENT_AFTER_SMALLEST_STEP:
			case gSKIEVENT_BEFORE_ELABORATION_CYCLE:
			case gSKIEVENT_AFTER_ELABORATION_CYCLE:
			case gSKIEVENT_BEFORE_PHASE_EXECUTED:
			case gSKIEVENT_AFTER_PHASE_EXECUTED:
			case gSKIEVENT_BEFORE_DECISION_CYCLE:
			case gSKIEVENT_AFTER_DECISION_CYCLE:
			case gSKIEVENT_AFTER_INTERRUPT:
			case gSKIEVENT_BEFORE_RUNNING:
			case gSKIEVENT_AFTER_RUNNING:
				return true ;
		}
		return false ;
	}

	bool IsProductionEvent(egSKIEventId id)
	{
		switch (id)
		{
			case gSKIEVENT_AFTER_PRODUCTION_ADDED:
			case gSKIEVENT_BEFORE_PRODUCTION_REMOVED:
			case gSKIEVENT_AFTER_PRODUCTION_FIRED:
			case gSKIEVENT_BEFORE_PRODUCTION_RETRACTED:
				return true ;
		}
		return false ;
	}

	bool IsAgentEvent(egSKIEventId id)
	{
		switch (id)
		{
			case gSKIEVENT_AFTER_AGENT_CREATED:
			case gSKIEVENT_BEFORE_AGENT_DESTROYED:
			case gSKIEVENT_BEFORE_AGENT_REINITIALIZED:
			case gSKIEVENT_AFTER_AGENT_REINITIALIZED:
				return true ;
		}
		return false ;
	}

public:
	AgentListener(KernelSML* pKernelSML, gSKI::IAgent* pAgent)
	{
		m_KernelSML = pKernelSML ;
		m_Agent		= pAgent ;
	}

	virtual ~AgentListener()
	{
	}

	// Returns true if this is the first connection listening for this event
	bool AddListener(egSKIEventId eventID, Connection* pConnection) ;

	// Returns true if at least one connection remains listening for this event
	bool RemoveListener(egSKIEventId eventID, Connection* pConnection) ;

	// Called when a "RunEvent" occurs in the kernel
	virtual void HandleEvent(egSKIEventId eventId, gSKI::IAgent* agentPtr, egSKIPhaseType phase) ;

	// Called when an "AgentEvent" occurs in the kernel
	virtual void HandleEvent(egSKIEventId eventId, gSKI::IAgent* agentPtr) ;

	// Called when a "ProductionEvent" occurs in the kernel
	virtual void HandleEvent(egSKIEventId eventId, gSKI::IAgent* agentPtr, gSKI::IProduction* prod, gSKI::IProductionInstance* match) ;
} ;

}

#endif
