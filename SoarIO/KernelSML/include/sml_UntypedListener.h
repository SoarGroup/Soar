/////////////////////////////////////////////////////////////////
// UntypedListener class file.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : June 2005
//
// This class's HandleEvent method is called when
// specific events occur within the agent.
//
/////////////////////////////////////////////////////////////////

#ifndef UNTYPED_LISTENER_H
#define UNTYPED_LISTENER_H

#include "gSKI_Events.h"
#include "gSKI_Enumerations.h"
#include "IgSKI_Iterator.h"
#include "IgSKI_Agent.h"
#include "sml_EventManager.h"

#include <string>
#include <map>

namespace sml {

class KernelSML ;
class Connection ;

class UntypedListener : public EventManager<egSKIUntypedEventId>
{
protected:
	KernelSML*		m_pKernelSML ;

public:
	UntypedListener()
	{
		m_pKernelSML = NULL ;
	}

	virtual ~UntypedListener()
	{
		Clear() ;
	}

	// Initialize this listener
	void Init(KernelSML* pKernelSML) { m_pKernelSML = pKernelSML ; }

	// Returns true if this is the first connection listening for this event
	virtual bool AddListener(egSKIUntypedEventId eventID, Connection* pConnection) ;

	// Returns true if at least one connection remains listening for this event
	virtual bool RemoveListener(egSKIUntypedEventId eventID, Connection* pConnection) ;

	// Called when an "UntypedEvent" occurs in the kernel
	virtual void HandleEvent(egSKIUntypedEventId eventId, void* data) ;
} ;

}

#endif
