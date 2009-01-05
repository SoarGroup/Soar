/////////////////////////////////////////////////////////////////
// RunListener class file.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : September 2004
//
// This class's HandleEvent method is called when
// specific events occur within the agent:
//
/*
*     @li smlEVENT_BEFORE_SMALLEST_STEP
*     @li smlEVENT_AFTER_SMALLEST_STEP
*     @li smlEVENT_BEFORE_ELABORATION_CYCLE
*     @li smlEVENT_AFTER_ELABORATION_CYCLE
*     @li smlEVENT_BEFORE_PHASE_EXECUTED
*     @li smlEVENT_AFTER_PHASE_EXECUTED
*     @li smlEVENT_BEFORE_DECISION_CYCLE
*     @li smlEVENT_AFTER_DECISION_CYCLE
*     @li smlEVENT_AFTER_INTERRUPT
*     @li smlEVENT_BEFORE_RUNNING
*     @li smlEVENT_AFTER_RUNNING
*/
/////////////////////////////////////////////////////////////////

#ifndef RUN_LISTENER_H
#define RUN_LISTENER_H

#include "sml_EventManager.h"
#include "sml_Events.h"

#include <string>
#include <map>

namespace sml {

class KernelSML ;
class Connection ;

class RunListener : public EventManager<smlRunEventId>
{
protected:
	KernelSML*		m_pKernelSML ;

public:
	RunListener()
	{
		m_pKernelSML = 0 ;
	}

	virtual ~RunListener()
	{
		Clear() ;
	}

	void Init(KernelSML* pKernelSML, AgentSML* pAgentSML) ;

	// Called when an event occurs in the kernel
	virtual void OnKernelEvent(int eventID, AgentSML* pAgentSML, void* pCallData) ;

	// Returns true if this is the first connection listening for this event
	virtual bool AddListener(smlRunEventId eventID, Connection* pConnection) ;

	// Returns true if at least one connection remains listening for this event
	virtual bool RemoveListener(smlRunEventId eventID, Connection* pConnection) ;
} ;

}

#endif
