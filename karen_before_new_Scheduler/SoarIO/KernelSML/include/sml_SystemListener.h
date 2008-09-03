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

#ifndef SYSTEM_LISTENER_H
#define SYSTEM_LISTENER_H

#include "gSKI_Events.h"
#include "gSKI_Enumerations.h"
#include "IgSKI_Iterator.h"
#include "IgSKI_Kernel.h"
#include "sml_EventManager.h"

#include <string>
#include <map>

namespace sml {

class Connection ;

class SystemListener : public gSKI::ISystemListener, public EventManager<egSKISystemEventId>
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

	// Returns true if this is the first connection listening for this event
	virtual bool AddListener(egSKISystemEventId eventID, Connection* pConnection) ;

	// Returns true if at least one connection remains listening for this event
	virtual bool RemoveListener(egSKISystemEventId eventID, Connection* pConnection) ;

	// Called when a "SystemEvent" occurs in the kernel
	virtual void HandleEvent(egSKISystemEventId eventId, gSKI::IKernel* kernel) ;

} ;

}

#endif
