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

#include "sml_EventManager.h"
#include "sml_Events.h"

#include <string>
#include <map>

namespace sml {

class KernelSML ;
class Connection ;

struct StringListenerCallbackData 
{
	char const* pData;					// Input
	char* pReturnStringBuffer;			// Output, client owned buffer, zero length string when there is no output (former function returned false)
	int maxLengthReturnStringBuffer;	// Output, length of client owned buffer
};

class StringListener : public EventManager<smlStringEventId>
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

	// Called when an event occurs in the kernel, see comments for StringListenerCallbackData
	virtual void OnKernelEvent(int eventID, AgentSML* pAgentSML, void* pCallData) ;

	// Returns true if this is the first connection listening for this event
	virtual bool AddListener(smlStringEventId eventID, Connection* pConnection) ;

	// Returns true if at least one connection remains listening for this event
	virtual bool RemoveListener(smlStringEventId eventID, Connection* pConnection) ;
} ;

}

#endif
