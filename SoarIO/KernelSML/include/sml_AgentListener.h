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
*/
/////////////////////////////////////////////////////////////////
#ifndef AGENT_LISTENER_H
#define AGENT_LISTENER_H

#include "gSKI_Events.h"
#include "gSKI_Enumerations.h"
#include "IgSKI_Iterator.h"
#include "IgSKI_Agent.h"
#include "sml_EventManager.h"

#include <map>

namespace sml {

class KernelSML ;
class Connection ;

class AgentListener : public gSKI::IRunListener, public EventManager
{
protected:
	KernelSML*		m_KernelSML ;
	gSKI::IAgent*	m_Agent ;

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
	virtual bool AddListener(egSKIEventId eventID, Connection* pConnection)
	{
		bool first = EventManager::AddListener(eventID, pConnection) ;

		if (first)
			m_Agent->AddRunListener(eventID, this) ;

		return first ;
	}

	// Returns true if at least one connection remains listening for this event
	virtual bool RemoveListener(egSKIEventId eventID, Connection* pConnection)
	{
		bool last = EventManager::RemoveListener(eventID, pConnection) ;

		if (last)
			m_Agent->RemoveRunListener(eventID, this) ;

		return last ;
	}

	// Called when the event occurs in the kernel
	virtual void HandleEvent(egSKIEventId eventId, gSKI::IAgent* agentPtr, egSKIPhaseType phase) ;
} ;

}

#endif
