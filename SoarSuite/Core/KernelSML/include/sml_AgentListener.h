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
*     smlEVENT_AFTER_AGENT_CREATED,
*     smlEVENT_BEFORE_AGENT_DESTROYED,
*	  smlEVENT_BEFORE_AGENTS_RUN_STEP,
*     smlEVENT_BEFORE_AGENT_REINITIALIZED,
*     smlEVENT_AFTER_AGENT_REINITIALIZED,
*/
/////////////////////////////////////////////////////////////////

#ifndef AGENT_LISTENER_H
#define AGENT_LISTENER_H

#include "sml_EventManager.h"
#include "sml_Events.h"

#include <string>
#include <map>

namespace sml {

class KernelSML ;
class Connection ;

class AgentListener : public EventManager<smlAgentEventId>
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

	// Called when an event occurs in the kernel
	virtual void OnKernelEvent(int eventID, AgentSML* pAgentSML, void* pCallData) ;

	// Returns true if this is the first connection listening for this event
	virtual bool AddListener(smlAgentEventId eventID, Connection* pConnection) ;

	// Returns true if at least one connection remains listening for this event
	virtual bool RemoveListener(smlAgentEventId eventID, Connection* pConnection) ;

	virtual void OnEvent(smlAgentEventId eventID, AgentSML* pAgentSML) ;

} ;

}

#endif

