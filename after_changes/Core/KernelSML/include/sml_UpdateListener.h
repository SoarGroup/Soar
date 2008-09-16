/////////////////////////////////////////////////////////////////
// UpdateListener class file.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : May 2005
//
// This class's HandleEvent method is called when
// specific events occur within the agent:
//
/////////////////////////////////////////////////////////////////

#ifndef UPDATE_LISTENER_H
#define UPDATE_LISTENER_H

#include "sml_EventManager.h"
#include "sml_Events.h"

#include <string>
#include <map>

namespace sml {

class KernelSML ;
class Connection ;

class UpdateListener : public EventManager<smlUpdateEventId>
{
protected:
	KernelSML*		m_pKernelSML ;

public:
	UpdateListener()
	{
		m_pKernelSML = NULL ;
	}

	virtual ~UpdateListener()
	{
		Clear() ;
	}

	// Initialize this listener
	void Init(KernelSML* pKernelSML) { m_pKernelSML = pKernelSML ; }

	// Called when an event occurs in the kernel
	virtual void OnKernelEvent(int eventID, AgentSML* pAgentSML, void* pCallData) ;

	// Returns true if this is the first connection listening for this event
	virtual bool AddListener(smlUpdateEventId eventID, Connection* pConnection) ;

	// Returns true if at least one connection remains listening for this event
	virtual bool RemoveListener(smlUpdateEventId eventID, Connection* pConnection) ;
} ;

}

#endif
