#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

/////////////////////////////////////////////////////////////////
// AgentOutputFlusher class file.
//
// Author: Jonathan Voigt
// Date  : February 2005
//
/////////////////////////////////////////////////////////////////

#include "sml_AgentOutputFlusher.h"
#include "assert.h"
#include "sml_AgentListener.h"

using namespace sml ;

#ifndef unused
#define unused(x) (void)(x)
#endif

AgentOutputFlusher::AgentOutputFlusher(AgentListener* pAgentListener, gSKI::IAgent* pAgent) : m_AgentListener(pAgentListener), m_Agent(pAgent)
{
	m_Agent->AddRunListener(gSKIEVENT_AFTER_DECISION_CYCLE, this);
	m_Agent->AddRunListener(gSKIEVENT_AFTER_RUNNING, this);
}

AgentOutputFlusher::~AgentOutputFlusher()
{
	m_Agent->RemoveRunListener(gSKIEVENT_AFTER_DECISION_CYCLE, this);
	m_Agent->RemoveRunListener(gSKIEVENT_AFTER_RUNNING, this);
}

void AgentOutputFlusher::HandleEvent(egSKIEventId eventId, gSKI::IAgent* agentPtr, egSKIPhaseType phase)
{
	assert(eventId == gSKIEVENT_AFTER_DECISION_CYCLE || eventId == gSKIEVENT_AFTER_RUNNING);
	assert(agentPtr == m_Agent);
	unused(eventId);
	unused(agentPtr);
	unused(phase);

	assert(m_AgentListener);
	m_AgentListener->FlushOutput();
}
