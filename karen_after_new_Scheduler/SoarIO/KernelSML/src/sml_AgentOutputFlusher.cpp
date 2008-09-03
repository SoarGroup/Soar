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
#include "sml_XMLListener.h"

using namespace sml ;

#ifndef unused
#define unused(x) (void)(x)
#endif

AgentOutputFlusher::AgentOutputFlusher(PrintListener* pPrintListener, gSKI::IAgent* pAgent, egSKIPrintEventId eventID) : m_pAgent(pAgent), m_pPrintListener(pPrintListener)
{
	m_pXMLListener = NULL ;
	m_EventID = eventID ;
	m_pAgent->AddRunListener(gSKIEVENT_AFTER_DECISION_CYCLE, this);
	m_pAgent->AddRunListener(gSKIEVENT_AFTER_RUNNING, this);
}

AgentOutputFlusher::AgentOutputFlusher(XMLListener* pXMLListener, gSKI::IAgent* pAgent, egSKIXMLEventId eventID) : m_pAgent(pAgent), m_pXMLListener(pXMLListener)
{
	m_pPrintListener = NULL ;
	m_EventID = eventID ;
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

	if (m_pPrintListener)
		m_pPrintListener->FlushOutput((egSKIPrintEventId)m_EventID);
	if (m_pXMLListener)
		m_pXMLListener->FlushOutput((egSKIXMLEventId)m_EventID) ;
}
