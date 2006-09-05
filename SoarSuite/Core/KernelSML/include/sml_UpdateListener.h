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

#include "gSKI_Events.h"
#include "gSKI_Enumerations.h"
#include "IgSKI_Iterator.h"
#include "gSKI_Agent.h"
#include "sml_EventManager.h"

#include <string>
#include <map>

namespace sml {

class KernelSML ;
class Connection ;

class UpdateListener : public EventManager<egSKIUpdateEventId>
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

	// Returns true if this is the first connection listening for this event
	virtual bool AddListener(egSKIUpdateEventId eventID, Connection* pConnection) ;

	// Returns true if at least one connection remains listening for this event
	virtual bool RemoveListener(egSKIUpdateEventId eventID, Connection* pConnection) ;

	// Called when an "UpdateEvent" occurs in the kernel
	virtual void HandleEvent(egSKIUpdateEventId eventId, int runFlags) ;
} ;

}

#endif
