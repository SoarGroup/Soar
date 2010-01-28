/////////////////////////////////////////////////////////////////
// XMLListener class file.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : May 2005
//
// This class's HandleEvent method is called when
// specific events occur within the agent:
//
/*
*       smlEVENT_XML_TRACE_OUTPUT
*/
/////////////////////////////////////////////////////////////////

#ifndef XML_LISTENER_H
#define XML_LISTENER_H

#include "sml_EventManager.h"
#include "XMLTrace.h"
#include "sml_Events.h"

#include <string>
#include <map>

namespace sml {

class KernelSML ;
class Connection ;

class XMLListener : public EventManager<smlXMLEventId>
{
protected:
	KernelSML*				m_pKernelSML ;

	// When false we don't forward print callback events to the listeners.  (Useful when we're backdooring into the kernel)
	bool					m_EnablePrintCallback ;

public:
	XMLListener()
	{
		m_pKernelSML = 0 ;
	}

	virtual ~XMLListener()
	{
		Clear() ;
	}

	void Init(KernelSML* pKernelSML, AgentSML* pAgentSML) ;

	// Called when an event occurs in the kernel
	virtual void OnKernelEvent(int eventID, AgentSML* pAgentSML, void* pCallData) ;

	// Returns true if this is the first connection listening for this event
	virtual bool AddListener(smlXMLEventId eventID, Connection* pConnection) ;

	// Returns true if at least one connection remains listening for this event
	virtual bool RemoveListener(smlXMLEventId eventID, Connection* pConnection) ;

	// Allows us to temporarily stop forwarding print callback output from the kernel to the SML listeners
	void EnablePrintCallback(bool enable) { m_EnablePrintCallback = enable ; }

	// Echo the list of wmes received back to any listeners
	void FireInputReceivedEvent(soarxml::ElementXML const* pCommands) ;
} ;

}

#endif
