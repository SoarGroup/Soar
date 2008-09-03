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

class AgentListener;

class AgentOutputFlusher : public gSKI::IRunListener
{
protected:
	gSKI::IAgent* m_Agent;
	AgentListener* m_AgentListener;

public:
	AgentOutputFlusher(AgentListener* pAgentListener, gSKI::IAgent* pAgent);
	virtual ~AgentOutputFlusher();

	virtual void HandleEvent(egSKIEventId eventId, gSKI::IAgent* agentPtr, egSKIPhaseType phase);
};

}

#endif
