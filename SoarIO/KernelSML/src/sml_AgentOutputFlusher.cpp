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
#include "sml_PrintListener.h"

using namespace sml ;

#ifndef unused
#define unused(x) (void)(x)
#endif

AgentOutputFlusher::AgentOutputFlusher(PrintListener* pPrintListener, gSKI::IAgent* pAgent) : m_pAgent(pAgent), m_pPrintListener(pPrintListener)
{
	m_pAgent->AddRunListener(gSKIEVENT_AFTER_DECISION_CYCLE, this);
	m_pAgent->AddRunListener(gSKIEVENT_AFTER_RUNNING, this);
}

AgentOutputFlusher::~AgentOutputFlusher()
{
	m_pAgent->RemoveRunListener(gSKIEVENT_AFTER_DECISION_CYCLE, this);
	m_pAgent->RemoveRunListener(gSKIEVENT_AFTER_RUNNING, this);
}

void AgentOutputFlusher::HandleEvent(egSKIRunEventId eventId, gSKI::IAgent* agentPtr, egSKIPhaseType phase)
{
	assert(eventId == gSKIEVENT_AFTER_DECISION_CYCLE || eventId == gSKIEVENT_AFTER_RUNNING);
	assert(agentPtr == m_pAgent);
	unused(eventId);
	unused(agentPtr);
	unused(phase);

	assert(m_pPrintListener);
	m_pPrintListener->FlushOutput();
}
