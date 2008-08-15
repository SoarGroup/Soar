/////////////////////////////////////////////////////////////////
// ProductionListener class file.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : September 2004
//
// This class's HandleEvent method is called when
// specific events occur within the agent:
//
/*
*      smlEVENT_AFTER_PRODUCTION_ADDED,
*      smlEVENT_BEFORE_PRODUCTION_REMOVED,
*     //smlEVENT_BEFORE_PRODUCTION_FIRED,
*      smlEVENT_AFTER_PRODUCTION_FIRED,
*      smlEVENT_BEFORE_PRODUCTION_RETRACTED,
*/
/////////////////////////////////////////////////////////////////

#ifndef PRODUCTION_LISTENER_H
#define PRODUCTION_LISTENER_H

#include "sml_EventManager.h"

#include <string>
#include <map>

namespace sml {

class KernelSML ;
class Connection ;

class ProductionListener : public EventManager < smlProductionEventId >
{
protected:
	KernelSML*		m_pKernelSML ;

public:
	ProductionListener()
	{
		m_pKernelSML = 0 ;
	}

	virtual ~ProductionListener()
	{
		Clear() ;
	}

	void Init(KernelSML* pKernelSML, AgentSML* pAgentSML) ;

	// Called when an event occurs in the kernel
	virtual void OnKernelEvent(int eventID, AgentSML* pAgentSML, void* pCallData) ;

	// Returns true if this is the first connection listening for this event
	virtual bool AddListener(smlProductionEventId eventID, Connection* pConnection) ;

	// Returns true if at least one connection remains listening for this event
	virtual bool RemoveListener(smlProductionEventId eventID, Connection* pConnection) ;
} ;

}

#endif
