#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
//FIXME: #include <portability.h>

/////////////////////////////////////////////////////////////////
// StringListener class file.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : June 2005
//
// This class's HandleEvent method is called when
// specific events occur within the agent.
//
/////////////////////////////////////////////////////////////////

#include "sml_UntypedListener.h"
#include "sml_Connection.h"
#include "sml_StringOps.h"
#include "IgSKI_Production.h"
#include "gSKI_ProductionManager.h"
#include "gSKI_Kernel.h"
#include "sml_KernelSML.h"
#include "sml_AgentSML.h"

#include "assert.h"

using namespace sml ;

// Returns true if this is the first connection listening for this event
bool StringListener::AddListener(egSKIStringEventId eventID, Connection* pConnection)
{
	bool first = BaseAddListener(eventID, pConnection) ;

	return first ;
}

// Returns true if at least one connection remains listening for this event
bool StringListener::RemoveListener(egSKIStringEventId eventID, Connection* pConnection)
{
	bool last = BaseRemoveListener(eventID, pConnection) ;

	return last ;
}

// Called when a event occurs in the kernel
bool StringListener::HandleEvent(egSKIStringEventId eventID, char const* pData, int maxLengthReturnValue, char* pReturnValue)
{
	bool result = false ;

	// Get the first listener for this event (or return if there are none)
	ConnectionListIter connectionIter ;
	if (!EventManager<egSKIStringEventId>::GetBegin(eventID, &connectionIter))
		return result;

	// We need the first connection for when we're building the message.  Perhaps this is a sign that
	// we shouldn't have rolled these methods into Connection.
	Connection* pConnection = *connectionIter ;

	// Convert eventID to a string
	char const* event = m_pKernelSML->ConvertEventToString(eventID) ;

	// Build the SML message we're doing to send.
	ElementXML* pMsg = pConnection->CreateSMLCommand(sml_Names::kCommand_Event) ;
	pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamEventID, event) ;

	if (pData)
		pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamValue, pData) ;

	// Note: we should be telling the client the maximum length of the result,
	// however, we're planning on changing this so there is no maximum length
	// so we're not implementing this.

	// Send the message out
	AnalyzeXML response ;
	SendEvent(pConnection, pMsg, &response, connectionIter, GetEnd(eventID)) ;

	char const* pResult = response.GetResultString() ;

	if (pResult != NULL)
	{
		// If the listener returns a result then take that
		// value and return it in "pReturnValue" to the caller.
		// If the client returns a longer string than the caller allowed we just truncate it.
		// (In practice this shouldn't be a problem--just need to make sure nobody crashes on a super long return string).
		strncpy(pReturnValue, pResult, maxLengthReturnValue) ;
		pReturnValue[maxLengthReturnValue-1] = 0 ;	// Make sure it's NULL terminated
		result = true ;
	}

	// Clean up
	delete pMsg ;

	return result;
}
