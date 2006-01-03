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
#include "IgSKI_Agent.h"

namespace sml {

class PrintListener;
class XMLListener ;

class AgentOutputFlusher : public gSKI::IRunListener
{
protected:
	gSKI::IAgent* m_pAgent;
	int m_EventID ;

	// Only one listener will be filled in.
	PrintListener* m_pPrintListener;
	XMLListener*   m_pXMLListener;

public:
	AgentOutputFlusher(PrintListener* pPrintListener, gSKI::IAgent* pAgent, egSKIPrintEventId eventID);
	AgentOutputFlusher(XMLListener* pXMLListener, gSKI::IAgent* pAgent, egSKIXMLEventId eventID);
	virtual ~AgentOutputFlusher();

	virtual void HandleEvent(egSKIRunEventId eventId, gSKI::IAgent* agentPtr, egSKIPhaseType phase);
};

}

#endif
