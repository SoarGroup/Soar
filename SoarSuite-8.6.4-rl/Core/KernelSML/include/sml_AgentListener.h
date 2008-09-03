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
*     gSKIEVENT_AFTER_AGENT_CREATED,
*     gSKIEVENT_BEFORE_AGENT_DESTROYED,
*	  gSKIEVENT_BEFORE_AGENTS_RUN_STEP,
*     gSKIEVENT_BEFORE_AGENT_REINITIALIZED,
*     gSKIEVENT_AFTER_AGENT_REINITIALIZED,
*/
/////////////////////////////////////////////////////////////////

#ifndef AGENT_LISTENER_H
#define AGENT_LISTENER_H

#include "gSKI_Events.h"
#include "gSKI_Enumerations.h"
#include "IgSKI_Iterator.h"
#include "gSKI_Agent.h"
#include "gSKI_Kernel.h"
#include "sml_EventManager.h"

#include <string>
#include <map>

namespace sml {

class KernelSML ;
class Connection ;

class AgentListener : gSKI::IAgentListener, public EventManager<egSKIAgentEventId>
{
protected:
	KernelSML*		m_pKernelSML ;

public:
	AgentListener() {}

	virtual ~AgentListener()
	{
		Clear() ;
	}

	// Initialize this listener
	void Init(KernelSML* pKernelSML) ;

	// Returns true if this is the first connection listening for this event
	virtual bool AddListener(egSKIAgentEventId eventID, Connection* pConnection) ;

	// Returns true if at least one connection remains listening for this event
	virtual bool RemoveListener(egSKIAgentEventId eventID, Connection* pConnection) ;

	// Called when an "AgentEvent" occurs in the kernel
	virtual void HandleEvent(egSKIAgentEventId eventId, gSKI::Agent* agentPtr) ;
} ;

}

#endif

