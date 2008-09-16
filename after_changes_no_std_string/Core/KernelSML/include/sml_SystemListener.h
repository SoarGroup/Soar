/////////////////////////////////////////////////////////////////
// SystemListener class file.
//
// Author: Doug Pearson, www.threepenny.net
// Date  : October 2004
//
// This class's HandleEvent method is called when
// specific events occur within the kernel:
/*
*      // System events
*      smlEVENT_BEFORE_SHUTDOWN            = 1,
*      smlEVENT_AFTER_CONNECTION_LOST,
*      smlEVENT_BEFORE_RESTART,
*      smlEVENT_AFTER_RESTART,
*      smlEVENT_BEFORE_RHS_FUNCTION_ADDED,
*      smlEVENT_AFTER_RHS_FUNCTION_ADDED,
*      smlEVENT_BEFORE_RHS_FUNCTION_REMOVED,
*      smlEVENT_AFTER_RHS_FUNCTION_REMOVED,
*      smlEVENT_BEFORE_RHS_FUNCTION_EXECUTED,
*      smlEVENT_AFTER_RHS_FUNCTION_EXECUTED,
*/////////////////////////////////////////////////////////////////

#ifndef SYSTEM_LISTENER_H
#define SYSTEM_LISTENER_H

#include "sml_EventManager.h"
#include "sml_Events.h"

#include <string>
#include <map>

namespace sml {

class Connection ;

class SystemListener : public EventManager<smlSystemEventId>
{
protected:
	// The kernel
	KernelSML* m_pKernelSML ;

public:
	SystemListener() {}

	virtual ~SystemListener()
	{
		Clear() ;
	}

	// Initialize this listener
	void Init(KernelSML* pKernelSML) ;

	// Called when an event occurs in the kernel
	virtual void OnKernelEvent(int eventID, AgentSML* pAgentSML, void* pCallData) ;

	// Returns true if this is the first connection listening for this event
	virtual bool AddListener(smlSystemEventId eventID, Connection* pConnection) ;

	// Returns true if at least one connection remains listening for this event
	virtual bool RemoveListener(smlSystemEventId eventID, Connection* pConnection) ;

} ;

}

#endif
