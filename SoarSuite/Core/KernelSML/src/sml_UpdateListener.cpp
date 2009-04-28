#include <portability.h>

/////////////////////////////////////////////////////////////////
// UpdateListener class file.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : May 2005
//
// This class's HandleEvent method is called when
// specific events occur within the agent:
//
/////////////////////////////////////////////////////////////////

#include "sml_UpdateListener.h"

#include "sml_Utils.h"
#include "sml_Connection.h"
#include "sml_KernelSML.h"
#include "sml_AgentSML.h"

#include "assert.h"

using namespace sml ;

// Returns true if this is the first connection listening for this event
bool UpdateListener::AddListener(smlUpdateEventId eventID, Connection* pConnection)
{
	bool first = BaseAddListener(eventID, pConnection) ;

	return first ;
}

// Returns true if at least one connection remains listening for this event
bool UpdateListener::RemoveListener(smlUpdateEventId eventID, Connection* pConnection)
{
	bool last = BaseRemoveListener(eventID, pConnection) ;

	return last ;
}

// Called when an event occurs in the kernel
void UpdateListener::OnKernelEvent(int eventIDIn, AgentSML* pAgentSML, void* pCallData)
{
	// There are currently no kernel events corresponding to this SML event.
	// They are all directly generated from SML.  If we later add kernel callbacks
	// for this class of events they would come here.

	smlUpdateEventId eventID = static_cast<smlUpdateEventId>(eventIDIn);
	int* pRunFlags = static_cast<int*>(pCallData);
	assert(pRunFlags);

	// Get the first listener for this event (or return if there are none)
	ConnectionListIter connectionIter ;
	if (!EventManager<smlUpdateEventId>::GetBegin(eventID, &connectionIter))
		return ;

	// We need the first connection for when we're building the message.  Perhaps this is a sign that
	// we shouldn't have rolled these methods into Connection.
	Connection* pConnection = *connectionIter ;

	// Convert eventID to a string
	char const* event = m_pKernelSML->ConvertEventToString(eventID) ;

	// Convert phase to a string
	std::string runStr;

	// Build the SML message we're doing to send.
	soarxml::ElementXML* pMsg = pConnection->CreateSMLCommand(sml_Names::kCommand_Event) ;
	pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamEventID, event) ;
	pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamValue, to_string(*pRunFlags, runStr).c_str()) ;

	// Send the message out
	AnalyzeXML response ;
	SendEvent(pAgentSML, pConnection, pMsg, &response, connectionIter, GetEnd(eventID)) ;

	// Clean up
	delete pMsg ;
}
