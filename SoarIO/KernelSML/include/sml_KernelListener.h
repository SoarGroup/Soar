/////////////////////////////////////////////////////////////////
// KernelListener class file.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : October 2004
//
// This class's HandleEvent method is called when
// specific events occur within the kernel:
/*
*      // System events
*      gSKIEVENT_BEFORE_SHUTDOWN            = 1,
*      gSKIEVENT_AFTER_CONNECTION_LOST,
*      gSKIEVENT_BEFORE_RESTART,
*      gSKIEVENT_AFTER_RESTART,
*      gSKIEVENT_BEFORE_RHS_FUNCTION_ADDED,
*      gSKIEVENT_AFTER_RHS_FUNCTION_ADDED,
*      gSKIEVENT_BEFORE_RHS_FUNCTION_REMOVED,
*      gSKIEVENT_AFTER_RHS_FUNCTION_REMOVED,
*      gSKIEVENT_BEFORE_RHS_FUNCTION_EXECUTED,
*      gSKIEVENT_AFTER_RHS_FUNCTION_EXECUTED,
*/////////////////////////////////////////////////////////////////
#ifndef KERNEL_LISTENER_H
#define KERNEL_LISTENER_H

#include "gSKI_Events.h"
#include "gSKI_Enumerations.h"
#include "IgSKI_Iterator.h"
#include "IgSKI_Kernel.h"
#include "sml_EventManager.h"

#include <map>

namespace sml {

class Connection ;

class KernelListener : public gSKI::ISystemListener, public gSKI::IAgentListener, public EventManager
{
protected:
	bool IsSystemEvent(egSKIEventId id)
	{
		switch (id)
		{
		case gSKIEVENT_BEFORE_SHUTDOWN:
		case gSKIEVENT_AFTER_CONNECTION_LOST:
		case gSKIEVENT_BEFORE_RESTART:
		case gSKIEVENT_AFTER_RESTART:
		case gSKIEVENT_BEFORE_RHS_FUNCTION_ADDED:
		case gSKIEVENT_AFTER_RHS_FUNCTION_ADDED:
		case gSKIEVENT_BEFORE_RHS_FUNCTION_REMOVED:
		case gSKIEVENT_AFTER_RHS_FUNCTION_REMOVED:
		case gSKIEVENT_BEFORE_RHS_FUNCTION_EXECUTED:
		case gSKIEVENT_AFTER_RHS_FUNCTION_EXECUTED:
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
			default:
				break;
		}
		return false ;
	}

public:
	KernelListener()
	{
	}

	virtual ~KernelListener()
	{
		Clear() ;
	}

	// Returns true if this is the first connection listening for this event
	virtual bool AddListener(egSKIEventId eventID, Connection* pConnection) ;

	// Returns true if at least one connection remains listening for this event
	virtual bool RemoveListener(egSKIEventId eventID, Connection* pConnection) ;

	// Called when a "SystemEvent" occurs in the kernel
	virtual void HandleEvent(egSKIEventId eventId, gSKI::IKernel* kernel) ;

	// Called when an "AgentEvent" occurs in the kernel
	virtual void HandleEvent(egSKIEventId eventId, gSKI::IAgent* agentPtr) ;
} ;

}

#endif
