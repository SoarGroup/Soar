/////////////////////////////////////////////////////////////////
// StringListener class file.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : June 2005
//
// This class's HandleEvent method is called when
// specific events occur within the agent.
//
/////////////////////////////////////////////////////////////////

#ifndef STRING_LISTENER_H
#define STRING_LISTENER_H

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

class StringListener : public EventManager<egSKIStringEventId>
{
protected:
	KernelSML*		m_pKernelSML ;

public:
	StringListener()
	{
		m_pKernelSML = NULL ;
	}

	virtual ~StringListener()
	{
		Clear() ;
	}

	// Initialize this listener
	void Init(KernelSML* pKernelSML) { m_pKernelSML = pKernelSML ; }

	// Returns true if this is the first connection listening for this event
	virtual bool AddListener(egSKIStringEventId eventID, Connection* pConnection) ;

	// Returns true if at least one connection remains listening for this event
	virtual bool RemoveListener(egSKIStringEventId eventID, Connection* pConnection) ;

	// Called when an "StringEvent" occurs in the kernel
	virtual void HandleEvent(egSKIStringEventId eventId, char const* data) ;
} ;

}

#endif
