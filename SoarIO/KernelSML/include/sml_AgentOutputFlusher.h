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

class AgentOutputFlusher : public gSKI::IRunListener
{
protected:
	gSKI::IAgent* m_pAgent;
	PrintListener* m_pPrintListener;

public:
	AgentOutputFlusher(PrintListener* pPrintListener, gSKI::IAgent* pAgent);
	virtual ~AgentOutputFlusher();

	virtual void HandleEvent(egSKIRunEventId eventId, gSKI::IAgent* agentPtr, egSKIPhaseType phase);
};

}

#endif
