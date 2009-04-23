#include <portability.h>

/////////////////////////////////////////////////////////////////
// AgentOutputFlusher class file.
//
// Author: Jonathan Voigt
// Date  : February 2005
//
/////////////////////////////////////////////////////////////////

#include "sml_AgentOutputFlusher.h"

#include "assert.h"

#include "sml_Utils.h"
#include "sml_PrintListener.h"

using namespace sml ;

AgentOutputFlusher::AgentOutputFlusher(PrintListener* pPrintListener, AgentSML* pAgent, smlPrintEventId eventID) : m_pPrintListener(pPrintListener)
{
	m_EventID = eventID ;
	this->SetAgentSML(pAgent) ;
	this->RegisterWithKernel(smlEVENT_AFTER_DECISION_CYCLE) ;
	this->RegisterWithKernel(smlEVENT_AFTER_RUNNING) ;
}

AgentOutputFlusher::~AgentOutputFlusher()
{
	this->UnregisterWithKernel(smlEVENT_AFTER_DECISION_CYCLE) ;
	this->UnregisterWithKernel(smlEVENT_AFTER_RUNNING) ;
}

void AgentOutputFlusher::OnKernelEvent(int eventID, AgentSML* /*pAgentSML*/, void* /*pCallData*/)
{
	unused(eventID);
	assert(eventID == smlEVENT_AFTER_DECISION_CYCLE || eventID == smlEVENT_AFTER_RUNNING);

	if (m_pPrintListener)
		m_pPrintListener->FlushOutput(static_cast<smlPrintEventId>(m_EventID));
}
