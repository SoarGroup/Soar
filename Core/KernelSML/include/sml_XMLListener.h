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
*       gSKIEVENT_XML_TRACE_OUTPUT
*/
/////////////////////////////////////////////////////////////////

#ifndef XML_LISTENER_H
#define XML_LISTENER_H

#include "gSKI_Events.h"
#include "gSKI_Enumerations.h"
#include "IgSKI_Iterator.h"
#include "IgSKI_Agent.h"
#include "IgSKI_Kernel.h"
#include "sml_EventManager.h"
#include "sml_AgentOutputFlusher.h"
#include "sml_XMLTrace.h"

#include <string>
#include <map>

namespace sml {

class KernelSML ;
class Connection ;

class XMLListener : public gSKI::IXMLListener, public EventManager<egSKIXMLEventId>
{
protected:
	const static int kNumberEvents = gSKIEVENT_LAST_XML_EVENT - gSKIEVENT_XML_TRACE_OUTPUT + 1 ;
	KernelSML*		m_pKernelSML ;
	gSKI::IAgent*	m_pAgent ;
	XMLTrace		m_BufferedXMLOutput[kNumberEvents];
	AgentOutputFlusher* m_pAgentOutputFlusher[kNumberEvents];

	// When false we don't forward print callback events to the listeners.  (Useful when we're backdooring into the kernel)
	bool			m_EnablePrintCallback ;

public:
	XMLListener(KernelSML* pKernelSML, gSKI::IAgent* pAgent)
	{
		m_pKernelSML = pKernelSML ;
		m_pAgent	 = pAgent ;
		m_EnablePrintCallback = true ;

		for (int i = 0 ; i < kNumberEvents ; i++)
			m_pAgentOutputFlusher[i] = NULL ;
	}

	virtual ~XMLListener()
	{
		Clear() ;
	}

	// Returns true if this is the first connection listening for this event
	virtual bool AddListener(egSKIXMLEventId eventID, Connection* pConnection) ;

	// Returns true if at least one connection remains listening for this event
	virtual bool RemoveListener(egSKIXMLEventId eventID, Connection* pConnection) ;

	/** 
	* @brief Event callback function
	*
	* This method recieves callbacks when the xml event occurs for an agent.
	*
	* @param eventId	  Id of the event that occured (can only be gSKIEVENT_XML_TRACE_OUTPUT)
	* @param agentPtr	  Pointer to the agent that fired the print event
	* @param funcType     Pointer to c-style string containing the function type (i.e. addTag, addAttributeValuePair, endTag)
	* @param attOrTag     Pointer to c-style string containing the tag to add or remove or the attribute to add
	* @param value		  Pointer to c-style string containing the value to add (may be NULL if just adding/ending a tag)
	*/
	virtual void HandleEvent(egSKIXMLEventId eventId, gSKI::IAgent* agentPtr, const char* funcType, const char* attOrTag, const char* value);

	// Allows us to temporarily stop forwarding print callback output from the kernel to the SML listeners
	void EnablePrintCallback(bool enable) { m_EnablePrintCallback = enable ; }

	// Send the SML event to the clients (flush output)
	void FlushOutput(egSKIXMLEventId eventID);

	// Echo the list of wmes received back to any listeners
	void XMLListener::FireInputReceivedEvent(ElementXML const* pCommands) ;
} ;

}

#endif
