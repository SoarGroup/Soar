#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

/////////////////////////////////////////////////////////////////
// Agent class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : Sept 2004
//
// This class is used by a client app (e.g. an environment) to represent
// a Soar agent and to send commands and I/O to and from that agent.
//
/////////////////////////////////////////////////////////////////

#include "sml_ClientAgent.h"
#include "sml_ClientKernel.h"
#include "sml_Connection.h"
#include "sml_ClientIdentifier.h"
#include "sml_OutputDeltaList.h"
#include "sml_StringOps.h"

#include "sml_ClientDirect.h"
#include "sml_EmbeddedConnection.h"	// For access to direct methods

#include <iostream>     
#include <sstream>     
#include <iomanip>

#include <cassert>
#include <string>

using namespace sml;

Agent::Agent(Kernel* pKernel, char const* pName)
{
	m_Kernel = pKernel ;
	m_Name	 = pName ;
	m_WorkingMemory.SetAgent(this) ;
}

Agent::~Agent()
{
}

Connection* Agent::GetConnection() const
{
	return m_Kernel->GetConnection() ;
}

/*************************************************************
* @brief This function is called when output is received
*		 from the Soar kernel.
*
* @param pIncoming	The output command (list of wmes added/removed from output link)
* @param pResponse	The reply (no real need to fill anything in here currently)
*************************************************************/
void Agent::ReceivedOutput(AnalyzeXML* pIncoming, ElementXML* pResponse)
{
	GetWM()->ReceivedOutput(pIncoming, pResponse) ;
}

void Agent::ReceivedEvent(AnalyzeXML* pIncoming, ElementXML* pResponse)
{
	smlEventId id  = (smlEventId)pIncoming->GetArgInt(sml_Names::kParamEventID, smlEVENT_INVALID_EVENT) ;

	switch (id)
	{
	// Agent listener events
	case smlEVENT_BEFORE_SMALLEST_STEP:
	case smlEVENT_AFTER_SMALLEST_STEP:
	case smlEVENT_BEFORE_ELABORATION_CYCLE:
	case smlEVENT_AFTER_ELABORATION_CYCLE:
	case smlEVENT_BEFORE_PHASE_EXECUTED:
	case smlEVENT_AFTER_PHASE_EXECUTED:
	case smlEVENT_BEFORE_DECISION_CYCLE:
	case smlEVENT_AFTER_DECISION_CYCLE:
	case smlEVENT_AFTER_INTERRUPT:
	case smlEVENT_BEFORE_RUNNING:
	case smlEVENT_AFTER_RUNNING:
		ReceivedRunEvent(id, pIncoming, pResponse) ;
		break ;

	// Production Manager events too
	case smlEVENT_AFTER_PRODUCTION_ADDED:
	case smlEVENT_BEFORE_PRODUCTION_REMOVED:
	case smlEVENT_AFTER_PRODUCTION_FIRED:
	case smlEVENT_BEFORE_PRODUCTION_RETRACTED:
		ReceivedProductionEvent(id, pIncoming, pResponse) ;
		break;

	// Agent manager events too
	case smlEVENT_AFTER_AGENT_CREATED:
	case smlEVENT_BEFORE_AGENT_DESTROYED:
	case smlEVENT_BEFORE_AGENT_REINITIALIZED:
	case smlEVENT_AFTER_AGENT_REINITIALIZED:
		ReceivedAgentEvent(id, pIncoming, pResponse) ;
		break;

	case smlEVENT_PRINT:
		ReceivedPrintEvent(id, pIncoming, pResponse) ;
		break ;
	}
}

/*************************************************************
* @brief This function is called when an event is received
*		 from the Soar kernel.
*
* @param pIncoming	The event command
* @param pResponse	The reply (no real need to fill anything in here currently)
*************************************************************/
void Agent::ReceivedRunEvent(smlEventId id, AnalyzeXML* pIncoming, ElementXML* pResponse)
{
	unused(pResponse) ;

	smlPhase phase = (smlPhase)pIncoming->GetArgInt(sml_Names::kParamPhase, -1) ;

	// Look up the handler(s) from the map
	RunEventMap::ValueList* pHandlers = m_RunEventMap.getList(id) ;

	if (!pHandlers)
		return ;

	// Go through the list of event handlers calling each in turn
	for (RunEventMap::ValueListIter iter = pHandlers->begin() ; iter != pHandlers->end() ; iter++)
	{
		RunEventHandlerPlusData handlerWithData = *iter ;

		RunEventHandler handler = handlerWithData.first ;
		void* pUserData = handlerWithData.second ;

		// Call the handler
		handler(id, pUserData, this, phase) ;
	}
}

/*************************************************************
* @brief This function is called when an event is received
*		 from the Soar kernel.
*
* @param pIncoming	The event command
* @param pResponse	The reply (no real need to fill anything in here currently)
*************************************************************/
void Agent::ReceivedAgentEvent(smlEventId id, AnalyzeXML* pIncoming, ElementXML* pResponse)
{
	unused(pResponse) ;
	unused(pIncoming) ;

	// Look up the handler(s) from the map
	AgentEventMap::ValueList* pHandlers = m_AgentEventMap.getList(id) ;

	if (!pHandlers)
		return ;

	// Go through the list of event handlers calling each in turn
	for (AgentEventMap::ValueListIter iter = pHandlers->begin() ; iter != pHandlers->end() ; iter++)
	{
		AgentEventHandlerPlusData handlerPlus = *iter ;
		AgentEventHandler handler = handlerPlus.first ;
		void* pUserData = handlerPlus.second ;

		// Call the handler
		handler(id, pUserData, this) ;
	}
}

/*************************************************************
* @brief This function is called when an event is received
*		 from the Soar kernel.
*
* @param pIncoming	The event command
* @param pResponse	The reply (no real need to fill anything in here currently)
*************************************************************/
void Agent::ReceivedPrintEvent(smlEventId id, AnalyzeXML* pIncoming, ElementXML* pResponse)
{
	unused(pResponse) ;

	char const* pMessage = pIncoming->GetArgValue(sml_Names::kParamMessage) ;

	// Look up the handler(s) from the map
	PrintEventMap::ValueList* pHandlers = m_PrintEventMap.getList(id) ;

	if (!pHandlers)
		return ;

	// Go through the list of event handlers calling each in turn
	for (PrintEventMap::ValueListIter iter = pHandlers->begin() ; iter != pHandlers->end() ; iter++)
	{
		PrintEventHandlerPlusData handlerPlus = *iter ;
		PrintEventHandler handler = handlerPlus.first ;

		// BUGBUG: DJP -- This shouldn't be hard-coded to pIncoming, something is wrong here.
		//void* pUserData = handlerPlus.second ;
		void* pUserData = pIncoming ;

		// Call the handler
		handler(id, pUserData, this, pMessage) ;
	}
}

/*************************************************************
* @brief This function is called when an event is received
*		 from the Soar kernel.
*
* @param pIncoming	The event command
* @param pResponse	The reply (no real need to fill anything in here currently)
*************************************************************/
void Agent::ReceivedProductionEvent(smlEventId id, AnalyzeXML* pIncoming, ElementXML* pResponse)
{
	unused(pResponse) ;

	char const* pProductionName = pIncoming->GetArgValue(sml_Names::kParamName) ;
	char const* pInstance = 0 ;	// gSKI defines this but doesn't support it yet.

	// Look up the handler(s) from the map
	ProductionEventMap::ValueList* pHandlers = m_ProductionEventMap.getList(id) ;

	if (!pHandlers)
		return ;

	// Go through the list of event handlers calling each in turn
	for (ProductionEventMap::ValueListIter iter = pHandlers->begin() ; iter != pHandlers->end() ; iter++)
	{
		ProductionEventHandlerPlusData handlerPlus = *iter ;

		ProductionEventHandler handler = handlerPlus.first ;
		void* pUserData = handlerPlus.second ;

		// Call the handler
		handler(id, pUserData, this, pProductionName, pInstance) ;
	}
}

/*************************************************************
* @brief Load a set of productions from a file.
*
* The file must currently be on a filesystem that the kernel can
* access (i.e. can't send to a remote PC unless that PC can load
* this file).
*************************************************************/
bool Agent::LoadProductions(char const* pFilename)
{
	// gSKI's LoadProductions command doesn't support all of the command line commands we need,
	// so we'll send this through the command line processor instead by creating a "source" command.
	std::string cmd = "source " ;
	cmd += pFilename ;

	// Execute the source command
	char const* pResult = GetKernel()->ExecuteCommandLine(cmd.c_str(), GetAgentName()) ;

	bool ok = GetKernel()->GetLastCommandLineResult() ;

	if (ok)
		ClearError() ;
	else
		SetDetailedError(Error::kDetailedError, pResult) ;

	return ok ;
}

/*************************************************************
* @brief Register for a particular event at the kernel
*************************************************************/
void Agent::RegisterForEvent(smlEventId id)
{
	AnalyzeXML response ;

	char buffer[kMinBufferSize] ;
	Int2String(id, buffer, sizeof(buffer)) ;

	// Send the register command
	GetConnection()->SendAgentCommand(&response, sml_Names::kCommand_RegisterForEvent, GetAgentName(), sml_Names::kParamEventID, buffer) ;
}

/*************************************************************
* @brief Unregister for a particular event at the kernel
*************************************************************/
void Agent::UnregisterForEvent(smlEventId id)
{
	AnalyzeXML response ;

	char buffer[kMinBufferSize] ;
	Int2String(id, buffer, sizeof(buffer)) ;

	// Send the unregister command
	GetConnection()->SendAgentCommand(&response, sml_Names::kCommand_UnregisterForEvent, GetAgentName(), sml_Names::kParamEventID, buffer) ;
}

/*************************************************************
* @brief Register for a "RunEvent".
*
* Current set is:
* smlEVENT_BEFORE_SMALLEST_STEP,
* smlEVENT_AFTER_SMALLEST_STEP,
* smlEVENT_BEFORE_ELABORATION_CYCLE,
* smlEVENT_AFTER_ELABORATION_CYCLE,
* smlEVENT_BEFORE_PHASE_EXECUTED,
* smlEVENT_AFTER_PHASE_EXECUTED,
* smlEVENT_BEFORE_DECISION_CYCLE,
* smlEVENT_AFTER_DECISION_CYCLE,
* smlEVENT_AFTER_INTERRUPT,
* smlEVENT_BEFORE_RUNNING,
* smlEVENT_AFTER_RUNNING,
*************************************************************/
void Agent::RegisterForRunEvent(smlEventId id, RunEventHandler handler, void* pUserData)
{
	// If we have no handlers registered with the kernel, then we need
	// to register for this event.  No need to do this multiple times.
	if (m_RunEventMap.getListSize(id) == 0)
	{
		RegisterForEvent(id) ;
	}

	// Record the handler
	RunEventHandlerPlusData handlerPlus = std::make_pair(handler, pUserData) ;
	m_RunEventMap.add(id, handlerPlus) ;
}

/*************************************************************
* @brief Unregister for a particular event
*************************************************************/
void Agent::UnregisterForRunEvent(smlEventId id, RunEventHandler handler, void* pUserData)
{
	// Remove the handler from our map
	RunEventHandlerPlusData handlerPlus = std::make_pair(handler, pUserData) ;
	m_RunEventMap.remove(id, handlerPlus) ;

	// If we just removed the last handler, then unregister from the kernel for this event
	if (m_RunEventMap.getListSize(id) == 0)
	{
		UnregisterForEvent(id) ;
	}
}

/*************************************************************
* @brief Register for a "ProductionEvent".
*		 Multiple handlers can be registered for the same event.
*
* Current set is:
* Production Manager
* smlEVENT_AFTER_PRODUCTION_ADDED,
* smlEVENT_BEFORE_PRODUCTION_REMOVED,
* smlEVENT_AFTER_PRODUCTION_FIRED,
* smlEVENT_BEFORE_PRODUCTION_RETRACTED,
*************************************************************/
void Agent::RegisterForProductionEvent(smlEventId id, ProductionEventHandler handler, void* pUserData)
{
	// If we have no handlers registered with the kernel, then we need
	// to register for this event.  No need to do this multiple times.
	if (m_ProductionEventMap.getListSize(id) == 0)
	{
		RegisterForEvent(id) ;
	}

	// Record the handler
	m_ProductionEventMap.add(id, std::make_pair(handler, pUserData)) ;
}

/*************************************************************
* @brief Unregister for a particular event
*************************************************************/
void Agent::UnregisterForProductionEvent(smlEventId id, ProductionEventHandler handler, void* pUserData)
{
	// Remove the handler from our map
	m_ProductionEventMap.remove(id, std::make_pair(handler, pUserData)) ;

	// If we just removed the last handler, then unregister from the kernel for this event
	if (m_ProductionEventMap.getListSize(id) == 0)
	{
		UnregisterForEvent(id) ;
	}
}

/*************************************************************
* @brief Register for an "AgentEvent".
*		 Multiple handlers can be registered for the same event.
*
* Current set is:
* // Agent manager
* smlEVENT_AFTER_AGENT_CREATED,
* smlEVENT_BEFORE_AGENT_DESTROYED,
* smlEVENT_BEFORE_AGENT_REINITIALIZED,
* smlEVENT_AFTER_AGENT_REINITIALIZED,
*************************************************************/
void Agent::RegisterForAgentEvent(smlEventId id, AgentEventHandler handler, void* pUserData)
{
	// If we have no handlers registered with the kernel, then we need
	// to register for this event.  No need to do this multiple times.
	if (m_AgentEventMap.getListSize(id) == 0)
	{
		RegisterForEvent(id) ;
	}

	// Record the handler
	m_AgentEventMap.add(id, std::make_pair(handler, pUserData)) ;
}

/*************************************************************
* @brief Unregister for a particular event
*************************************************************/
void Agent::UnregisterForAgentEvent(smlEventId id, AgentEventHandler handler, void* pUserData)
{
	// Remove the handler from our map
	m_AgentEventMap.remove(id, std::make_pair(handler, pUserData)) ;

	// If we just removed the last handler, then unregister from the kernel for this event
	if (m_AgentEventMap.getListSize(id) == 0)
	{
		UnregisterForEvent(id) ;
	}
}

/*************************************************************
* @brief Register for a "PrintEvent".
*		 Multiple handlers can be registered for the same event.
*
* Current set is:
* // Agent manager
* smlEVENT_PRINT
*************************************************************/
void Agent::RegisterForPrintEvent(smlEventId id, PrintEventHandler handler, void* pUserData)
{
	// If we have no handlers registered with the kernel, then we need
	// to register for this event.  No need to do this multiple times.
	if (m_PrintEventMap.getListSize(id) == 0)
	{
		RegisterForEvent(id) ;
	}

	// Record the handler
	m_PrintEventMap.add(id, std::make_pair(handler, pUserData)) ;
}

/*************************************************************
* @brief Unregister for a particular event
*************************************************************/
void Agent::UnregisterForPrintEvent(smlEventId id, PrintEventHandler handler, void* pUserData)
{
	// Remove the handler from our map
	m_PrintEventMap.remove(id, std::make_pair(handler, pUserData)) ;

	// If we just removed the last handler, then unregister from the kernel for this event
	if (m_PrintEventMap.getListSize(id) == 0)
	{
		UnregisterForEvent(id) ;
	}
}

/*************************************************************
* @brief Returns the id object for the input link.
*		 The agent retains ownership of this object.
*************************************************************/
Identifier* Agent::GetInputLink()
{
	return GetWM()->GetInputLink() ;
}

/*************************************************************
* @brief Returns the id object for the output link.
*		 The agent retains ownership of this object.
*************************************************************/
Identifier* Agent::GetOutputLink()
{
	return GetWM()->GetOutputLink() ;
}

/*************************************************************
* @brief Get number of changes to output link.
*        (This is since last call to "ClearOuputLinkChanges").
*************************************************************/
int	Agent::GetNumberOutputLinkChanges()
{
	OutputDeltaList* pDeltas = GetWM()->GetOutputLinkChanges() ;
	return pDeltas->GetSize() ;
}

/*************************************************************
* @brief Get the n-th wme added or deleted to output link
*        (This is since last call to "ClearOuputLinkChanges").
*************************************************************/
WMElement* Agent::GetOutputLinkChange(int index)
{
	OutputDeltaList* pDeltas = GetWM()->GetOutputLinkChanges() ;
	WMDelta* pDelta = pDeltas->GetDeltaWME(index) ;

	if (!pDelta)
		return NULL ;

	WMElement* pWME = pDelta->getWME() ;
	
	return pWME ;
}

/*************************************************************
* @brief Returns true if the n-th wme change to the output-link
*		 was a wme being added.  (false => it was a wme being deleted).
*        (This is since last call to "ClearOuputLinkChanges").
*************************************************************/
bool Agent::IsOutputLinkChangeAdd(int index)
{
	OutputDeltaList* pDeltas = GetWM()->GetOutputLinkChanges() ;
	WMDelta* pDelta = pDeltas->GetDeltaWME(index) ;

	if (!pDelta)
		return false ;

	bool isAddition = (pDelta->getChangeType() == WMDelta::kAdded) ;

	return isAddition ;
}

/*************************************************************
* @brief Clear the current list of changes to the output-link.
*		 You should call this after processing the list of changes.
*************************************************************/
void Agent::ClearOutputLinkChanges()
{
	GetWM()->ClearOutputLinkChanges() ;
}

/*************************************************************
* @brief Get the number of "commands".  A command in this context
*		 is an identifier wme that have been added to the top level of
*		 the output-link since the last call to "ClearOutputLinkChanges".
*
*		 NOTE: This function may involve searching a list so it's
*		 best to not call it repeatedly.
*		 
*************************************************************/
int	Agent::GetNumberCommands()
{
	// Method is to search all top level output link wmes and see which have
	// just been added and are identifiers.
	int count = 0 ;

	Identifier* pOutputLink = GetOutputLink() ;

	if (!pOutputLink)
		return 0 ;

	for (Identifier::ChildrenIter iter = pOutputLink->GetChildrenBegin() ; iter != pOutputLink->GetChildrenEnd() ; iter++)
	{
		WMElement *pWME = *iter ;

		if (pWME->IsIdentifier() && pWME->IsJustAdded())
			count++ ;
	}

	return count ;
}

/*************************************************************
* @brief Get the n-th "command".  A command in this context
*		 is an identifier wme that have been added to the top level of
*		 the output-link since the last call to "ClearOutputLinkChanges".
*
*		 Returns NULL if index is out of range.
*
* @param index	The 0-based index for which command to get.
*************************************************************/
Identifier* Agent::GetCommand(int index)
{
	// Method is to search all top level output link wmes and see which have
	// just been added and are identifiers.
	Identifier* pOutputLink = GetOutputLink() ;

	if (!pOutputLink)
		return NULL ;

	for (Identifier::ChildrenIter iter = pOutputLink->GetChildrenBegin() ; iter != pOutputLink->GetChildrenEnd() ; iter++)
	{
		WMElement *pWME = *iter ;

		if (pWME->IsIdentifier() && pWME->IsJustAdded())
		{
			if (index == 0)
				return (Identifier*)pWME ;
			index-- ;
		}
	}

	return NULL ;
}

/*************************************************************
* @brief Builds a new WME that has a string value and schedules
*		 it for addition to Soar's input link.
*
*		 The agent retains ownership of this object.
*		 The returned object is valid until the caller
*		 deletes the parent identifier.
*		 The WME is not added to Soar's input link until the
*		 client calls "Commit".
*************************************************************/
StringElement* Agent::CreateStringWME(Identifier* parent, char const* pAttribute, char const* pValue)
{
	if (!parent)
		return NULL ;

	return GetWM()->CreateStringWME(parent, pAttribute, pValue) ;
}

/*************************************************************
* @brief Same as CreateStringWME but for a new WME that has
*		 an identifier as its value.
*
*		 The identifier value is generated here and will be
*		 different from the value Soar assigns in the kernel.
*		 The kernel will keep a map for translating back and forth.
*************************************************************/
Identifier* Agent::CreateIdWME(Identifier* parent, char const* pAttribute)
{
	if (!parent)
		return NULL ;

	return GetWM()->CreateIdWME(parent, pAttribute) ;
}

/*************************************************************
* @brief Creates a new WME that has an identifier as its value.
*		 The value in this case is the same as an existing identifier.
*		 This allows us to create a graph rather than a tree.
*************************************************************/
Identifier*	Agent::CreateSharedIdWME(Identifier* parent, char const* pAttribute, Identifier* pSharedValue)
{
	if (!parent || !pSharedValue)
		return NULL ;

	return GetWM()->CreateSharedIdWME(parent, pAttribute, pSharedValue) ;
}

/*************************************************************
* @brief Same as CreateStringWME but for a new WME that has
*		 an int as its value.
*************************************************************/
IntElement* Agent::CreateIntWME(Identifier* parent, char const* pAttribute, int value)
{
	if (!parent)
		return NULL ;

	return GetWM()->CreateIntWME(parent, pAttribute, value) ;
}

/*************************************************************
* @brief Same as CreateStringWME but for a new WME that has
*		 a floating point value.
*************************************************************/
FloatElement* Agent::CreateFloatWME(Identifier* parent, char const* pAttribute, double value)
{
	if (!parent)
		return NULL ;

	return GetWM()->CreateFloatWME(parent, pAttribute, value) ;
}

/*************************************************************
* @brief Update the value of an existing WME.
*		 The value is not actually sent to the kernel
*		 until "Commit" is called.
*************************************************************/
void Agent::Update(StringElement* pWME, char const* pValue) { GetWM()->UpdateString(pWME, pValue) ; }
void Agent::Update(IntElement* pWME, int value)				{ GetWM()->UpdateInt(pWME, value) ; }
void Agent::Update(FloatElement* pWME, double value)		{ GetWM()->UpdateFloat(pWME, value) ; }

/*************************************************************
* @brief Searches for a WME that has the given identifier value.
*		 There can be multiple WMEs that share the same identifier value.
*		 (You can use the index to find a specific one).
*
* @param pId			The id to look for (e.g. "O4" -- kernel side or "p3" -- client side)
* @param searchInput	If true, searches from input-link down
* @param searchOutput	If true, searches from output-link down
* @param index			If non-zero, finds the n-th match
*************************************************************/
Identifier*	Agent::FindIdentifier(char const* pID, bool searchInput, bool searchOutput, int index)
{
	return GetWM()->FindIdentifier(pID, searchInput, searchOutput, index) ;
}

/*************************************************************
* @brief Schedules a WME from deletion from the input link and removes
*		 it from the client's model of working memory.
*
*		 The caller should not access this WME after calling
*		 DestroyWME().
*		 The WME is not removed from the input link until
*		 the client calls "Commit"
*************************************************************/
bool Agent::DestroyWME(WMElement* pWME)
{
	if (!pWME)
		return false ;

	return GetWM()->DestroyWME(pWME) ;
}

/*************************************************************
* @brief Send the most recent list of changes to working memory
*		 over to the kernel.
*************************************************************/
bool Agent::Commit()
{
	return GetWM()->Commit() ;
}

/*************************************************************
* @brief Reinitialize this Soar agent.
*		 This will also cause the output link structures stored
*		 here to be erased and the current input link to be sent over
*		 to the Soar agent for the start of its next run.
*************************************************************/
char const* Agent::InitSoar()
{
	std::string cmd = "init-soar" ;

	// Execute the command.
	// The init-soar causes an event to be sent back from the kernel and when
	// we get that, we'll queue up the input link information to be sent over again
	// and also erase our output link information.
	char const* pResult = GetKernel()->ExecuteCommandLine(cmd.c_str(), GetAgentName()) ;
	return pResult ;
}

/*************************************************************
* @brief Run Soar for the specified number of decisions
*************************************************************/
char const* Agent::Run(unsigned long decisions)
{
	// Convert int to a string
	std::ostringstream ostr ;
	ostr << decisions ;

	// Create the command line for the run command
	std::string cmd = "run -d " + ostr.str() ;

	// Execute the run command.
	char const* pResult = GetKernel()->ExecuteCommandLine(cmd.c_str(), GetAgentName()) ;
	return pResult ;
}

/*************************************************************
* @brief   Controls whether Soar will break when it next generates
*		   output while running.
*
* @param state	If true, causes Soar to break on output.  If false, Soar will not break.
*************************************************************/
bool Agent::SetStopOnOutput(bool state)
{
	AnalyzeXML response ;

	bool ok = GetConnection()->SendAgentCommand(&response, sml_Names::kCommand_StopOnOutput, GetAgentName(), sml_Names::kParamValue, state ? sml_Names::kTrue : sml_Names::kFalse) ;
	return ok ;
}

/*************************************************************
* @brief   Run Soar until either output is generated or
*		   the maximum number of decisions is reached.
*
* This function also calls "ClearOutputLinkChanges" so methods
* like "IsJustAdded" will refer to the changes that occur as a result of
* this run.
*
* We don't generally want Soar to just run until it generates
* output without any limit as an error in the AI logic might cause
* it to never return control to the environment.
*
* @param maxDecisions	If Soar runs for this many decisions without generating output, stop.
*						15 was used in SGIO.
*************************************************************/
char const* Agent::RunTilOutput(unsigned long maxDecisions)
{
	ClearOutputLinkChanges() ;

#ifdef SML_DIRECT
	if (GetConnection()->IsDirectConnection())
	{
		((EmbeddedConnection*)GetConnection())->DirectRunTilOutput(GetAgentName()) ;
		return "DirectCall -- no output" ;
	}
#endif

	SetStopOnOutput(true) ;
	return Run(maxDecisions) ;
}

/*************************************************************
* @brief Resend the complete input link to the kernel
*		 and remove our output link structures.
*		 We do this when the user issues an "init-soar" event.
*		 There should be no reason for the client to call this method directly.
*************************************************************/
void Agent::Refresh()
{
	GetWM()->Refresh() ;
}
