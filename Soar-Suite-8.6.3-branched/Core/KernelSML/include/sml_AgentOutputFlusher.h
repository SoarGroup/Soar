/////////////////////////////////////////////////////////////////
// AgentOutputFlusher class file.
//
// Author: Jonathan Voigt
// Date  : February 2005
//
/////////////////////////////////////////////////////////////////
#ifndef AGENT_OUTPUT_FLUSHER_H
#define AGENT_OUTPUT_FLUSHER_H

#include "gSKI_Events.h"
#include "gSKI_Enumerations.h"
#include "gSKI_Agent.h"

namespace sml {

class PrintListener;
class XMLListener ;

class AgentOutputFlusher : public gSKI::IRunListener
{
protected:
	gSKI::Agent* m_pAgent;
	int m_EventID ;

	// Only one listener will be filled in.
	PrintListener* m_pPrintListener;
	XMLListener*   m_pXMLListener;

public:
	AgentOutputFlusher(PrintListener* pPrintListener, gSKI::Agent* pAgent, egSKIPrintEventId eventID);
	AgentOutputFlusher(XMLListener* pXMLListener, gSKI::Agent* pAgent, egSKIXMLEventId eventID);
	virtual ~AgentOutputFlusher();

	virtual void HandleEvent(egSKIRunEventId eventId, gSKI::Agent* agentPtr, egSKIPhaseType phase);
};

}

#endif
