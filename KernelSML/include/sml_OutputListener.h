/////////////////////////////////////////////////////////////////
// OutputListener class file.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : September 2004
//
// This class's HandleEvent method is called when
// the agent adds wmes to the output link.
//
/////////////////////////////////////////////////////////////////

#ifndef OUTPUT_LISTENER_H
#define OUTPUT_LISTENER_H

#include "gSKI_Events.h"
#include "gSKI_Enumerations.h"
#include "IgSKI_Iterator.h"
#include "IgSKI_Agent.h"
#include "sml_EventManager.h"

#include <map>

namespace sml {

class KernelSML ;
class Connection ;

// This map is from time tag to bool to say whether a given tag has been seen in the latest event or not
typedef std::map< long, bool >		OutputTimeTagMap ;
typedef OutputTimeTagMap::iterator	OutputTimeTagIter ;

class OutputListener : public gSKI::IWorkingMemoryListener, public gSKI::IAgentListener, public EventManager<egSKIWorkingMemoryEventId>
{
protected:
	KernelSML*		m_KernelSML ;
	gSKI::IAgent*	m_Agent ;

	// A list of the time tags of output wmes that we've already seen and sent to the client
	// This allows us to only send changes over.
	OutputTimeTagMap m_TimeTags ;

	// We need the ability to break Soar when output is generated.
	// gSKI can do this, but we can't say "run until output OR 10 decisions" which is what we really need.
	bool	m_StopOnOutput ;

public:
	OutputListener(KernelSML* pKernelSML, gSKI::IAgent* pAgent)
	{
		m_KernelSML = pKernelSML ;
		m_Agent		= pAgent ;
		m_StopOnOutput = false; // default to not stopping on output
	}

	virtual ~OutputListener()
	{
		Clear() ;
	}

	// Register for the events that KernelSML itself needs to know about in order to work correctly.
	void RegisterForKernelSMLEvents() ;

	// UnRegister for the events that KernelSML itself needs to know about in order to work correctly.
	void UnRegisterForKernelSMLEvents() ;

	// Returns true if this is the first connection listening for this event
	virtual bool AddListener(egSKIWorkingMemoryEventId eventID, Connection* pConnection) ;

	// Returns true if at least one connection remains listening for this event
	virtual bool RemoveListener(egSKIWorkingMemoryEventId eventID, Connection* pConnection) ;

	// Working memory event listener (called when the agent generates output)
	virtual void HandleEvent(egSKIWorkingMemoryEventId eventId, gSKI::IAgent* agentPtr, egSKIWorkingMemoryChange change, gSKI::tIWmeIterator* wmelist) ;

	// Agent event listener (called when soar is re-initialized)
	virtual void HandleEvent(egSKIAgentEventId eventId, gSKI::IAgent* agentPtr) ;

	void SetStopOnOutput(bool state) { m_StopOnOutput = state ; }
} ;

}

#endif
