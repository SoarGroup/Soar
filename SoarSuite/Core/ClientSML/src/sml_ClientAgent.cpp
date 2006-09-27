#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include <portability.h>

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
#include "sml_Events.h"
#include "sml_ClientXML.h"
#include "sml_ClientTraceXML.h"

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
	m_CallbackIDCounter = 0 ;
	m_XMLCallback = -1 ;
	m_BlinkIfNoChange = true ;

	ClearError() ;
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
	char const* pEventName = pIncoming->GetArgString(sml_Names::kParamEventID) ;

	// This event had no event id field
	if (!pEventName)
	{
		return ;
	}

	// Go from the string form of the event back to the integer ID
	int id = GetKernel()->m_pEventMap->ConvertToEvent(pEventName) ;

	if (IsRunEventID(id))
	{
		ReceivedRunEvent((smlRunEventId)id, pIncoming, pResponse) ;
	} else if (IsProductionEventID(id))
	{
		ReceivedProductionEvent((smlProductionEventId)id, pIncoming, pResponse) ;
	} else if (IsPrintEventID(id))
	{
		ReceivedPrintEvent((smlPrintEventId)id, pIncoming, pResponse) ;
	} else if (IsXMLEventID(id))
	{
		ReceivedXMLEvent((smlXMLEventId)id, pIncoming, pResponse) ;
	}
}

/*************************************************************
* @brief This function is called when an event is received
*		 from the Soar kernel.
*
* @param pIncoming	The event command
* @param pResponse	The reply (no real need to fill anything in here currently)
*************************************************************/
void Agent::ReceivedRunEvent(smlRunEventId id, AnalyzeXML* pIncoming, ElementXML* pResponse)
{
	unused(pResponse) ;

	smlPhase phase = (smlPhase)pIncoming->GetArgInt(sml_Names::kParamPhase, -1) ;

	// Look up the handler(s) from the map
	RunEventMap::ValueList* pHandlers = m_RunEventMap.getList(id) ;

	if (!pHandlers)
		return ;

	// Go through the list of event handlers calling each in turn
	for (RunEventMap::ValueListIter iter = pHandlers->begin() ; iter != pHandlers->end() ;)
	{
		RunEventHandlerPlusData handlerWithData = *iter ;
		iter++ ;

		RunEventHandler handler = handlerWithData.m_Handler ;
		void* pUserData = handlerWithData.m_UserData ;

		// Call the handler
		handler(id, pUserData, this, phase) ;
	}
}

/*************************************************************
* @brief This function is called after output has been received
*		 and processed from the kernel.
*************************************************************/
void Agent::FireOutputNotification()
{
	smlWorkingMemoryEventId id = smlEVENT_OUTPUT_PHASE_CALLBACK ;

	// Look up the handler(s) from the map
	OutputNotificationMap::ValueList* pHandlers = m_OutputNotificationMap.getList(id) ;

	if (!pHandlers)
		return ;

	// Go through the list of event handlers calling each in turn
	for (OutputNotificationMap::ValueListIter iter = pHandlers->begin() ; iter != pHandlers->end() ;)
	{
		OutputNotificationHandlerPlusData handlerWithData = *iter ;
		iter++ ;

		OutputNotificationHandler handler = handlerWithData.m_Handler ;
		void* pUserData = handlerWithData.m_UserData ;

		// Call the handler
		handler(pUserData, this) ;
	}
}

/*************************************************************
* @brief This function is called when an event is received
*		 from the Soar kernel.
*
* @param pIncoming	The event command
* @param pResponse	The reply (no real need to fill anything in here currently)
*************************************************************/
void Agent::ReceivedPrintEvent(smlPrintEventId id, AnalyzeXML* pIncoming, ElementXML* pResponse)
{
	unused(pResponse) ;

	char const* pMessage = pIncoming->GetArgString(sml_Names::kParamMessage) ;

	// This argument is only present on echo messages.
	bool self = pIncoming->GetArgBool(sml_Names::kParamSelf, false) ;

	// Look up the handler(s) from the map
	PrintEventMap::ValueList* pHandlers = m_PrintEventMap.getList(id) ;

	if (!pHandlers)
		return ;

	// Go through the list of event handlers calling each in turn
	for (PrintEventMap::ValueListIter iter = pHandlers->begin() ; iter != pHandlers->end() ; iter++)
	{
		PrintEventHandlerPlusData handlerPlus = *iter ;
		PrintEventHandler handler = handlerPlus.m_Handler ;
		bool ignoreOwnEchos = handlerPlus.m_IgnoreOwnEchos ;

		// If this is an echo event triggered by a command issued by ourselves ignore it.
		if (id == smlEVENT_ECHO && ignoreOwnEchos && self)
			continue ;

		void* pUserData = handlerPlus.m_UserData ;

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
void Agent::ReceivedProductionEvent(smlProductionEventId id, AnalyzeXML* pIncoming, ElementXML* pResponse)
{
	unused(pResponse) ;

	char const* pProductionName = pIncoming->GetArgString(sml_Names::kParamName) ;
	char const* pInstance = 0 ;	// gSKI defines this but doesn't support it yet.

	// Look up the handler(s) from the map
	ProductionEventMap::ValueList* pHandlers = m_ProductionEventMap.getList(id) ;

	if (!pHandlers)
		return ;

	// Go through the list of event handlers calling each in turn
	for (ProductionEventMap::ValueListIter iter = pHandlers->begin() ; iter != pHandlers->end() ; iter++)
	{
		ProductionEventHandlerPlusData handlerPlus = *iter ;

		ProductionEventHandler handler = handlerPlus.m_Handler ;
		void* pUserData = handlerPlus.m_UserData ;

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
*
* @param echoResults  If true the results are also echoed through the smlEVENT_ECHO event, so they can appear in a debugger (or other listener)
* @returns True if finds file to load successfully.
*************************************************************/
bool Agent::LoadProductions(char const* pFilename, bool echoResults)
{
	// gSKI's LoadProductions command doesn't support all of the command line commands we need,
	// so we'll send this through the command line processor instead by creating a "source" command.
	std::string cmd = "source ";
	if (strlen(pFilename) && (pFilename[0] == '\"') && (pFilename[strlen(pFilename) - 1] == '\"'))
	{
		cmd += pFilename ;
	} else {
		cmd += '\"';
		cmd += pFilename ;
		cmd += '\"';
	}

	// Execute the source command
	char const* pResult = ExecuteCommandLine(cmd.c_str(), echoResults) ;

	bool ok = GetLastCommandLineResult() ;

	if (ok)
		ClearError() ;
	else
		SetDetailedError(Error::kDetailedError, pResult) ;

	return ok ;
}

// These are little utility classes we define to help with searching the event maps
class Agent::TestRunCallback : public RunEventMap::ValueTest
{
private:
	int m_ID ;
public:
	TestRunCallback(int id) { m_ID = id ; }

	bool isEqual(RunEventHandlerPlusData handler)
	{
		return handler.m_CallbackID == m_ID ;
	}
} ;

class Agent::TestRunCallbackFull : public RunEventMap::ValueTest
{
private:
	int				m_EventID ;
	RunEventHandler m_Handler ;
	void*			m_UserData ;

public:
	TestRunCallbackFull(int id, RunEventHandler handler, void* pUserData)
	{ m_EventID = id ; m_Handler = handler ; m_UserData = pUserData ; }

	bool isEqual(RunEventHandlerPlusData handlerPlus)
	{
		return handlerPlus.m_EventID == m_EventID &&
			   handlerPlus.m_Handler == m_Handler &&
			   handlerPlus.m_UserData == m_UserData ;
	}
} ;

class Agent::TestOutputNotificationCallback : public OutputNotificationMap::ValueTest
{
private:
	int m_ID ;
public:
	TestOutputNotificationCallback(int id) { m_ID = id ; }

	bool isEqual(OutputNotificationHandlerPlusData handler)
	{
		return handler.m_CallbackID == m_ID ;
	}
} ;

class Agent::TestOutputNotificationCallbackFull : public OutputNotificationMap::ValueTest
{
private:
	int				m_EventID ;
	OutputNotificationHandler m_Handler ;
	void*			m_UserData ;

public:
	TestOutputNotificationCallbackFull(int id, OutputNotificationHandler handler, void* pUserData)
	{ m_EventID = id ; m_Handler = handler ; m_UserData = pUserData ; }

	bool isEqual(OutputNotificationHandlerPlusData handlerPlus)
	{
		return handlerPlus.m_EventID == m_EventID &&
			   handlerPlus.m_Handler == m_Handler &&
			   handlerPlus.m_UserData == m_UserData ;
	}
} ;

class Agent::TestOutputCallback : public OutputEventMap::ValueTest
{
private:
	int m_ID ;
public:
	TestOutputCallback(int id) { m_ID = id ; }

	bool isEqual(OutputEventHandlerPlusData handler)
	{
		return handler.m_CallbackID == m_ID ;
	}
} ;

class Agent::TestOutputCallbackFull : public OutputEventMap::ValueTest
{
private:
	std::string			m_AttributeName ;
	OutputEventHandler	m_Handler ;
	void*				m_UserData ;

public:
	TestOutputCallbackFull(char const* attributeName, OutputEventHandler handler, void* pUserData)
	{ m_AttributeName = attributeName ; m_Handler = handler ; m_UserData = pUserData ; }

	virtual ~TestOutputCallbackFull() { } ;

	bool isEqual(OutputEventHandlerPlusData handlerPlus)
	{
		return handlerPlus.m_AttributeName.compare(m_AttributeName) == 0 &&
			   handlerPlus.m_Handler == m_Handler &&
			   handlerPlus.m_UserData == m_UserData ;
	}
} ;

class Agent::TestProductionCallback : public ProductionEventMap::ValueTest
{
private:
	int m_ID ;
public:
	TestProductionCallback(int id) { m_ID = id ; }

	bool isEqual(ProductionEventHandlerPlusData handler)
	{
		return handler.m_CallbackID == m_ID ;
	}
} ;

class Agent::TestProductionCallbackFull : public ProductionEventMap::ValueTest
{
private:
	int				m_EventID ;
	ProductionEventHandler m_Handler ;
	void*			m_UserData ;

public:
	TestProductionCallbackFull(int id, ProductionEventHandler handler, void* pUserData)
	{ m_EventID = id ; m_Handler = handler ; m_UserData = pUserData ; }

	bool isEqual(ProductionEventHandlerPlusData handlerPlus)
	{
		return handlerPlus.m_EventID == m_EventID &&
			   handlerPlus.m_Handler == m_Handler &&
			   handlerPlus.m_UserData == m_UserData ;
	}
} ;

class Agent::TestPrintCallback : public PrintEventMap::ValueTest
{
private:
	int m_ID ;
public:
	TestPrintCallback(int id) { m_ID = id ; }

	bool isEqual(PrintEventHandlerPlusData handler)
	{
		return handler.m_CallbackID == m_ID ;
	}
} ;

class Agent::TestPrintCallbackFull : public PrintEventMap::ValueTest
{
private:
	int				m_EventID ;
	PrintEventHandler m_Handler ;
	void*			m_UserData ;

public:
	TestPrintCallbackFull(int id, PrintEventHandler handler, void* pUserData)
	{ m_EventID = id ; m_Handler = handler ; m_UserData = pUserData ; }

	bool isEqual(PrintEventHandlerPlusData handlerPlus)
	{
		return handlerPlus.m_EventID == m_EventID &&
			   handlerPlus.m_Handler == m_Handler &&
			   handlerPlus.m_UserData == m_UserData ;
	}
} ;

class Agent::TestXMLCallback : public XMLEventMap::ValueTest
{
private:
	int m_ID ;
public:
	TestXMLCallback(int id) { m_ID = id ; }

	bool isEqual(XMLEventHandlerPlusData handler)
	{
		return handler.m_CallbackID == m_ID ;
	}
} ;

class Agent::TestXMLCallbackFull : public XMLEventMap::ValueTest
{
private:
	int				m_EventID ;
	XMLEventHandler m_Handler ;
	void*			m_UserData ;

public:
	TestXMLCallbackFull(int id, XMLEventHandler handler, void* pUserData)
	{ m_EventID = id ; m_Handler = handler ; m_UserData = pUserData ; }

	bool isEqual(XMLEventHandlerPlusData handlerPlus)
	{
		return handlerPlus.m_EventID == m_EventID &&
			   handlerPlus.m_Handler == m_Handler &&
			   handlerPlus.m_UserData == m_UserData ;
	}
} ;

/*************************************************************
* @brief Register for a "RunEvent".
*		 Multiple handlers can be registered for the same event.
* @param smlEventId		The event we're interested in (see the list below for valid values)
* @param handler		A function that will be called when the event happens
* @param pUserData		Arbitrary data that will be passed back to the handler function when the event happens.
* @param addToBack		If true add this handler is called after existing handlers.  If false, called before existing handlers.
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
* smlEVENT_BEFORE_RUN_STARTS,
* smlEVENT_AFTER_RUN_ENDS,
* smlEVENT_BEFORE_RUNNING,
* smlEVENT_AFTER_RUNNING
*
* @returns A unique ID for this callback (used to unregister the callback later) 
*************************************************************/
int Agent::RegisterForRunEvent(smlRunEventId id, RunEventHandler handler, void* pUserData, bool addToBack)
{
	// Start by checking if this id, handler, pUSerData combination has already been registered
	TestRunCallbackFull test(id, handler, pUserData) ;

	// See if this handler is already registered
	RunEventHandlerPlusData plus(0,0,0,0) ;
	bool found = m_RunEventMap.findFirstValueByTest(&test, &plus) ;

	if (found && plus.m_Handler != 0)
		return plus.getCallbackID() ;

	// If we have no handlers registered with the kernel, then we need
	// to register for this event.  No need to do this multiple times.
	if (m_RunEventMap.getListSize(id) == 0)
	{
		GetKernel()->RegisterForEventWithKernel(id, GetAgentName()) ;
	}

	// Record the handler
	m_CallbackIDCounter++ ;

	// We use a struct rather than a pointer to a struct, so there's no need to new/delete
	// everything as the objects are added and deleted.
	RunEventHandlerPlusData handlerPlus(id, handler, pUserData, m_CallbackIDCounter) ;
	m_RunEventMap.add(id, handlerPlus, addToBack) ;

	// Return the ID.  We use this later to unregister the callback
	return m_CallbackIDCounter ;
}

/*************************************************************
* @brief Unregister for a particular event
*************************************************************/
bool Agent::UnregisterForRunEvent(int callbackID)
{
	// Build a test object for the callbackID we're interested in
	TestRunCallback test(callbackID) ;

	// Find the event ID for this callbackID
	smlRunEventId id = m_RunEventMap.findFirstKeyByTest(&test, (smlRunEventId)-1) ;

	if (id == -1)
		return false ;

	// Remove the handler from our map
	m_RunEventMap.removeAllByTest(&test) ;

	// If we just removed the last handler, then unregister from the kernel for this event
	if (m_RunEventMap.getListSize(id) == 0)
	{
		GetKernel()->UnregisterForEventWithKernel(id, GetAgentName()) ;
	}

	return true ;
}

/*************************************************************
* @brief Register to be notified when output has been received from the agent.
*		 This event is a bit special, because we ensure that the client side data structures
*		 have been updated before this event is triggered.  So you can examine the current contents
*		 of the output link (GetOutputLink() etc.) and it will be up to date.
*************************************************************/
int Agent::RegisterForOutputNotification(OutputNotificationHandler handler, void* pUserData, bool addToBack)
{
	smlWorkingMemoryEventId id = smlEVENT_OUTPUT_PHASE_CALLBACK ;

	// Start by checking if this id, handler, pUSerData combination has already been registered
	TestOutputNotificationCallbackFull test(id, handler, pUserData) ;

	// See if this handler is already registered
	OutputNotificationHandlerPlusData plus(0,0,0,0) ;
	bool found = m_OutputNotificationMap.findFirstValueByTest(&test, &plus) ;

	if (found && plus.m_Handler != 0)
		return plus.getCallbackID() ;

	// If we have no handlers registered with the kernel, then we need
	// to register for this event.  No need to do this multiple times.
	// (Only do this if we were ignoring output from the kernel already--otherwise we'll get two copies of everything
	//  because we're already registered for this event)
	if (GetKernel()->m_bIgnoreOutput && m_OutputNotificationMap.getListSize(id) == 0)
	{
		GetKernel()->RegisterForEventWithKernel(id, GetAgentName()) ;
	}

	// Record the handler
	m_CallbackIDCounter++ ;

	// We use a struct rather than a pointer to a struct, so there's no need to new/delete
	// everything as the objects are added and deleted.
	OutputNotificationHandlerPlusData handlerPlus(id, handler, pUserData, m_CallbackIDCounter) ;
	m_OutputNotificationMap.add(id, handlerPlus, addToBack) ;

	// Return the ID.  We use this later to unregister the callback
	return m_CallbackIDCounter ;
}

bool Agent::UnregisterForOutputNotification(int callbackID)
{
	// Build a test object for the callbackID we're interested in
	TestOutputNotificationCallback test(callbackID) ;

	// Find the event ID for this callbackID
	smlWorkingMemoryEventId id = m_OutputNotificationMap.findFirstKeyByTest(&test, (smlWorkingMemoryEventId)-1) ;

	if (id == -1)
		return false ;

	// Remove the handler from our map
	m_OutputNotificationMap.removeAllByTest(&test) ;

	// If we just removed the last handler, then unregister from the kernel for this event
	if (GetKernel()->m_bIgnoreOutput && m_OutputNotificationMap.getListSize(id) == 0)
	{
		GetKernel()->UnregisterForEventWithKernel(id, GetAgentName()) ;
	}

	return true ;
}

/*************************************************************
* @brief Register an "Output event handler".
*		 This is one way to be notified when output occurs on the output link.
*		 You register for a specific attribute name (e.g. "move") and when that attribute is added to the
*		 output link the handler you have registered for that name is called.
* @param pAttributeName	The attribute which will trigger this callback ("move" in the example).
* @param handler		A function that will be called when the event happens
* @param pUserData		Arbitrary data that will be passed back to the handler function when the event happens.
* @param addToBack		If true add this handler is called after existing handlers.  If false, called before existing handlers.
* 
* @returns A unique ID for this callback (used to unregister the callback later) 
*************************************************************/
int	Agent::AddOutputHandler(char const* pAttributeName, OutputEventHandler handler, void* pUserData, bool addToBack)
{
	// Start by checking if this attributeName, handler, pUSerData combination has already been registered
	TestOutputCallbackFull test(pAttributeName, handler, pUserData) ;

	// See if this handler is already registered
	OutputEventHandlerPlusData plus(0, 0,0,0,0) ;
	bool found = m_OutputEventMap.findFirstValueByTest(&test, &plus) ;

	if (found && plus.m_Handler != 0)
		return plus.getCallbackID() ;

	// Record the handler
	m_CallbackIDCounter++ ;
	OutputEventHandlerPlusData handlerPlus(0, pAttributeName, handler, pUserData, m_CallbackIDCounter) ;
	m_OutputEventMap.add(pAttributeName, handlerPlus, addToBack) ;

	// Return the ID.  We use this later to unregister the callback
	return m_CallbackIDCounter ;
}

/*************************************************************
* @brief Unregister for a particular output event
*************************************************************/
bool Agent::RemoveOutputHandler(int callbackID)
{
	// Build a test object for the callbackID we're interested in
	TestOutputCallback test(callbackID) ;

	// Find the function for this callbackID (for RHS functions the key is a function name not an event id)
	std::string functionName = m_OutputEventMap.findFirstKeyByTest(&test, "") ;

	if (functionName.length() == 0)
		return false ;

	// Remove the handler from our map
	m_OutputEventMap.removeAllByTest(&test) ;

	return true ;
}

/*************************************************************
* @brief Returns true if there's at least one listener for output events.
*************************************************************/
bool Agent::IsRegisteredForOutputEvent()
{
	return m_OutputEventMap.getSize() > 0 ;
}

/*************************************************************
* @brief Call any registered handlers to notify them when
*		 a new working memory element is added to the top
*		 level of the output link.
*************************************************************/
void Agent::ReceivedOutputEvent(WMElement* pWmeAdded)
{
	char const* pAttributeName = pWmeAdded->GetAttribute() ;

	// Look up the handler(s) from the map
	OutputEventMap::ValueList* pHandlers = m_OutputEventMap.getList(pAttributeName) ;

	if (!pHandlers)
		return ;

	// Go through the list of event handlers calling each in turn
	for (OutputEventMap::ValueListIter iter = pHandlers->begin() ; iter != pHandlers->end() ;)
	{
		OutputEventHandlerPlusData handlerWithData = *iter ;
		iter++ ;

		OutputEventHandler handler = handlerWithData.m_Handler ;
		void* pUserData = handlerWithData.getUserData() ;

		// Call the handler
		handler(pUserData, this, pAttributeName, pWmeAdded) ;
	}
}

/*************************************************************
* @brief Register for a "ProductionEvent".
*		 Multiple handlers can be registered for the same event.
* @param smlEventId		The event we're interested in (see the list below for valid values)
* @param handler		A function that will be called when the event happens
* @param pUserData		Arbitrary data that will be passed back to the handler function when the event happens.
* @param addToBack		If true add this handler is called after existing handlers.  If false, called before existing handlers.
*
* Current set is:
* Production Manager
* smlEVENT_AFTER_PRODUCTION_ADDED,
* smlEVENT_BEFORE_PRODUCTION_REMOVED,
* smlEVENT_AFTER_PRODUCTION_FIRED,
* smlEVENT_BEFORE_PRODUCTION_RETRACTED,
*
* @returns A unique ID for this callback (used to unregister the callback later) 
*************************************************************/
int Agent::RegisterForProductionEvent(smlProductionEventId id, ProductionEventHandler handler, void* pUserData, bool addToBack)
{
	// Start by checking if this id, handler, pUSerData combination has already been registered
	TestProductionCallbackFull test(id, handler, pUserData) ;

	// See if this handler is already registered
	ProductionEventHandlerPlusData plus(0,0,0,0) ;
	bool found = m_ProductionEventMap.findFirstValueByTest(&test, &plus) ;

	if (found && plus.m_Handler != 0)
		return plus.getCallbackID() ;

	// If we have no handlers registered with the kernel, then we need
	// to register for this event.  No need to do this multiple times.
	if (m_ProductionEventMap.getListSize(id) == 0)
	{
		GetKernel()->RegisterForEventWithKernel(id, GetAgentName()) ;
	}

	// Record the handler
	m_CallbackIDCounter++ ;
	ProductionEventHandlerPlusData handlerPlus(id, handler, pUserData, m_CallbackIDCounter) ;
	m_ProductionEventMap.add(id, handlerPlus, addToBack) ;

	// Return the ID.  We use this later to unregister the callback
	return m_CallbackIDCounter ;
}

/*************************************************************
* @brief Unregister for a particular event
*************************************************************/
bool Agent::UnregisterForProductionEvent(int callbackID)
{
	// Build a test object for the callbackID we're interested in
	TestProductionCallback test(callbackID) ;

	// Find the event ID for this callbackID
	smlProductionEventId id = m_ProductionEventMap.findFirstKeyByTest(&test, (smlProductionEventId)-1) ;

	if (id == -1)
		return false ;

	// Remove the handler from our map
	m_ProductionEventMap.removeAllByTest(&test) ;

	// If we just removed the last handler, then unregister from the kernel for this event
	if (m_ProductionEventMap.getListSize(id) == 0)
	{
		GetKernel()->UnregisterForEventWithKernel(id, GetAgentName()) ;
	}

	return true ;
}

/*************************************************************
* @brief Register for an "PrintEvent".
*		 Multiple handlers can be registered for the same event.
* @param smlEventId		The event we're interested in (see the list below for valid values)
* @param handler		A function that will be called when the event happens
* @param pUserData		Arbitrary data that will be passed back to the handler function when the event happens.
* @param ignoreOwnEchos If true and registering for echo event, commands issued through this connection won't echo.  If false, echos all commands.  Ignored for non-echo events.
* @param addToBack		If true add this handler is called after existing handlers.  If false, called before existing handlers.
*
* Current set is:
* // Agent manager
* smlEVENT_PRINT
*
* @returns A unique ID for this callback (used to unregister the callback later) 
*************************************************************/
int Agent::RegisterForPrintEvent(smlPrintEventId id, PrintEventHandler handler, void* pUserData, bool ignoreOwnEchos, bool addToBack)
{
	// Start by checking if this id, handler, pUSerData combination has already been registered
	TestPrintCallbackFull test(id, handler, pUserData) ;

	// See if this handler is already registered
	PrintEventHandlerPlusData plus(0,0,0,false,0) ;
	bool found = m_PrintEventMap.findFirstValueByTest(&test, &plus) ;

	if (found && plus.m_Handler != 0)
		return plus.getCallbackID() ;

	// If we have no handlers registered with the kernel, then we need
	// to register for this event.  No need to do this multiple times.
	if (m_PrintEventMap.getListSize(id) == 0)
	{
		GetKernel()->RegisterForEventWithKernel(id, GetAgentName()) ;
	}

	// Record the handler
	m_CallbackIDCounter++ ;

	PrintEventHandlerPlusData handlerPlus(id, handler, pUserData, ignoreOwnEchos, m_CallbackIDCounter) ;
	m_PrintEventMap.add(id, handlerPlus, addToBack) ;

	// Return the ID.  We use this later to unregister the callback
	return m_CallbackIDCounter ;
}

/*************************************************************
* @brief Unregister for a particular event
*************************************************************/
bool Agent::UnregisterForPrintEvent(int callbackID)
{
	// Build a test object for the callbackID we're interested in
	TestPrintCallback test(callbackID) ;

	// Find the event ID for this callbackID
	smlPrintEventId id = m_PrintEventMap.findFirstKeyByTest(&test, (smlPrintEventId)-1) ;

	if (id == -1)
		return false ;

	// Remove the handler from our map
	m_PrintEventMap.removeAllByTest(&test) ;

	// If we just removed the last handler, then unregister from the kernel for this event
	if (m_PrintEventMap.getListSize(id) == 0)
	{
		GetKernel()->UnregisterForEventWithKernel(id, GetAgentName()) ;
	}

	return true ;
}

void Agent::ReceivedXMLEvent(smlXMLEventId id, AnalyzeXML* pIncoming, ElementXML* pResponse)
{
	unused(pResponse) ;

	// Retrieve the original message
	ElementXML* pXMLMessage = new ElementXML(pIncoming->GetElementXMLHandle()) ;

	// Need to record our new reference to this handle.
	pXMLMessage->AddRefOnHandle() ;

	// NOTE: This object needs to stay in scope for as long as we're calling clients
	// and then when it is deleted it will delete pXMLMessage.
	ClientXML clientXML(pXMLMessage) ;

	// Look up the handler(s) from the map
	XMLEventMap::ValueList* pHandlers = m_XMLEventMap.getList(id) ;

	if (!pHandlers)
		return ;

	// Go through the list of event handlers calling each in turn
	for (XMLEventMap::ValueListIter iter = pHandlers->begin() ; iter != pHandlers->end() ; iter++)
	{
		XMLEventHandlerPlusData handlerPlus = *iter ;
		XMLEventHandler handler = handlerPlus.m_Handler ;

		void* pUserData = handlerPlus.m_UserData ;

		// Call the handler
		handler(id, pUserData, this, &clientXML) ;
	}
}

void Agent::ReceivedXMLTraceEvent(smlXMLEventId id, ElementXML* pIncoming, ElementXML* pResponse)
{
	unused(pResponse) ;

#ifdef _DEBUG
	char* pStr = pIncoming->GenerateXMLString(true) ;
	pIncoming->DeleteString(pStr) ;
#endif

	// For speed we don't analyze incoming XML trace messages.  Instead we just
	// look for the correct parts of the original message.  This makes these messages a bit
	// more brittle than the rest of the system but speed really counts on these messages and
	// this saves the analysis step (which is significant).
	// If this assert fails we've changed the structure of the XML event messages and we'll
	// need to update the code to match.
	assert (pIncoming->GetNumberChildren() == 2) ;

	//  Get the trace tag (again, we rely on the order for speed).
	ElementXML* pTrace = new ElementXML(NULL) ;
	pIncoming->GetChild(pTrace, 1) ;

	// NOTE: This object needs to stay in scope for as long as we're calling clients
	// and then when it is deleted it will delete pTrace.
	ClientXML clientXML(pTrace) ;

	// Look up the handler(s) from the map
	XMLEventMap::ValueList* pHandlers = m_XMLEventMap.getList(id) ;

	if (!pHandlers)
		return ;

	// Go through the list of event handlers calling each in turn
	for (XMLEventMap::ValueListIter iter = pHandlers->begin() ; iter != pHandlers->end() ;)
	{
		XMLEventHandlerPlusData handlerPlus = *iter ;
		XMLEventHandler handler = handlerPlus.m_Handler ;

		// Advance to the next handler before we make the callback, in case
		// the callback deletes the current handler from the list, invalidating the iterator.
		iter++ ;

		void* pUserData = handlerPlus.m_UserData ;

		// Call the handler
		handler(id, pUserData, this, &clientXML) ;
	}
}

int Agent::RegisterForXMLEvent(smlXMLEventId id, XMLEventHandler handler, void* pUserData, bool addToBack)
{
	// Start by checking if this id, handler, pUSerData combination has already been registered
	TestXMLCallbackFull test(id, handler, pUserData) ;

	// See if this handler is already registered
	XMLEventHandlerPlusData plus(0,0,0,0) ;
	bool found = m_XMLEventMap.findFirstValueByTest(&test, &plus) ;

	if (found && plus.m_Handler != 0)
		return plus.getCallbackID() ;

	// If we have no handlers registered with the kernel, then we need
	// to register for the print event (which we'll then parse to create XML objects).
	// No need to do this multiple times.
	if (m_XMLEventMap.getListSize(id) == 0)
	{
		GetKernel()->RegisterForEventWithKernel(id, GetAgentName()) ;
	}

	// Record the handler
	m_CallbackIDCounter++ ;

	XMLEventHandlerPlusData handlerPlus(id, handler, pUserData, m_CallbackIDCounter) ;
	m_XMLEventMap.add(id, handlerPlus, addToBack) ;

	// Return the ID.  We use this later to unregister the callback
	return m_CallbackIDCounter ;
}

bool Agent::UnregisterForXMLEvent(int callbackID)
{
	// Build a test object for the callbackID we're interested in
	TestXMLCallback test(callbackID) ;

	// Find the event ID for this callbackID
	smlXMLEventId id = m_XMLEventMap.findFirstKeyByTest(&test, (smlXMLEventId)-1) ;

	if (id == -1)
		return false ;

	// Remove the handler from our map
	m_XMLEventMap.removeAllByTest(&test) ;

	// If we just removed the last handler, then unregister
	// for the matching print event (that we use to implement XML)
	if (m_XMLEventMap.getListSize(id) == 0)
	{
		GetKernel()->UnregisterForEventWithKernel(id, GetAgentName()) ;
	}

	return true ;
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
*		 If this is an identifier then all of its children will be
*		 deleted too (assuming it's the only parent -- i.e. part of a tree not a full graph).
*
*		 The caller should not access this WME after calling
*		 DestroyWME() or any of its children if this is an identifier.
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
* @brief Returns true if this agent has uncommitted changes.
*************************************************************/
bool Agent::IsCommitRequired()
{
	return GetWM()->IsCommitRequired() ;
}

/*************************************************************
* @brief Returns true if changes to i/o links should be
*		 committed (sent to kernelSML) immediately when they
*		 occur, so the client doesn't need to remember to call commit.
*************************************************************/
bool Agent::IsAutoCommitEnabled()
{
	return m_Kernel->IsAutoCommitEnabled() ;
}

/*************************************************************
* @brief Reinitialize this Soar agent.
*		 This will also cause the output link structures stored
*		 here to be erased and the current input link to be sent over
*		 to the Soar agent for the start of its next run.
*************************************************************/
char const* Agent::InitSoar()
{
	// Must commit everything before doing an init-soar.
	assert(!GetWM()->IsCommitRequired()) ;

	std::string cmd = "init-soar" ;

	// Execute the command.
	// The init-soar causes an event to be sent back from the kernel and when
	// we get that, we'll queue up the input link information to be sent over again
	// and also erase our output link information.
	// (See the InitSoarHandler in ClientKernel.cpp)
	char const* pResult = ExecuteCommandLine(cmd.c_str()) ;
	return pResult ;
}

/*************************************************************
* @brief Interrupt the currently running Soar agent.
*
* Call this after calling "Run" in order to stop a Soar agent.
* The usual way to do this is to register for an event (e.g. AFTER_DECISION_CYCLE)
* and in that event handler decide if the user wishes to stop soar.
* If so, call to this method inside that handler.
* If so, call to this method inside that handler (this ensures you're calling on the same
* thread that Soar is running on so you don't get blocked).
*
* The request to Stop may not be honored immediately.
* Soar will stop at the next point it is considered safe to do so.
*
*************************************************************/
char const*	Agent::StopSelf()
{
	std::string cmd = "stop-soar --self" ;

	// Execute the command.
	char const* pResult = ExecuteCommandLine(cmd.c_str()) ;
	return pResult ;
}

/*************************************************************
* @brief Run Soar for the specified number of decisions
*************************************************************/
char const* Agent::RunSelf(unsigned long numberSteps, smlRunStepSize stepSize)
{
	if (IsCommitRequired())
	{
		assert(false) ;
		return "Need to commit changes before calling a run method" ;
	}	

#ifdef SML_DIRECT
		if (GetConnection()->IsDirectConnection())
		{
			((EmbeddedConnection*)GetConnection())->DirectRun(this->GetAgentName(), false, stepSize, sml_INTERLEAVE_PHASE, (int)numberSteps) ;
			return "DirectRun completed" ;
		}
#endif

	// Convert int to a string
	std::ostringstream ostr ;
	ostr << numberSteps ;

	// Create the command line for the run command
	// Create the command line for the run command
	std::string step ;
	
	switch (stepSize)
	{
		case sml_DECISION:		step = "-d" ; break ;
		case sml_PHASE:			step = "-p" ; break ;
		case sml_ELABORATION:	step = "-e" ; break ;
		case sml_UNTIL_OUTPUT:	step = "-o" ; break ;
		default: return "Unrecognized step size parameter passed to RunSelf" ;
	}

	std::string cmd = "run --self " + step + " " + ostr.str() ;

	// Execute the run command.
	char const* pResult = ExecuteCommandLine(cmd.c_str()) ;
	return pResult ;
}

char const* Agent::RunSelfForever()
{
	if (IsCommitRequired())
	{
		assert(false) ;
		return "Need to commit changes before calling a run method" ;
	}

#ifdef SML_DIRECT
		if (GetConnection()->IsDirectConnection())
		{
			((EmbeddedConnection*)GetConnection())->DirectRun(this->GetAgentName(), true, sml_DECISION, sml_INTERLEAVE_PHASE, 1) ;
			return "DirectRun completed" ;
		}
#endif

	// Create the command line for the run command
	std::string cmd = "run --self" ;

	// Execute the run command.
	char const* pResult = ExecuteCommandLine(cmd.c_str()) ;
	return pResult ;
}

/*************************************************************
* @brief Returns true if this agent was part of the last set
*		 of agents that was run.
*************************************************************/
bool Agent::WasAgentOnRunList()
{
	AnalyzeXML response ;

	bool ok = GetConnection()->SendAgentCommand(&response, sml_Names::kCommand_WasAgentOnRunList, GetAgentName()) ;

	if (!ok)
		return false ;

	bool wasRun = response.GetResultBool(false) ;
	return wasRun ;
}

/*************************************************************
* @brief Returns whether the last run for this agent was
*		 interrupted (by a stop call) or completed normally.
*************************************************************/
smlRunResult Agent::GetResultOfLastRun()
{
	AnalyzeXML response ;

	bool ok = GetConnection()->SendAgentCommand(&response, sml_Names::kCommand_GetResultOfLastRun, GetAgentName()) ;

	if (!ok)
		return sml_RUN_ERROR ;

	smlRunResult result = (smlRunResult)response.GetResultInt((int)sml_RUN_ERROR) ;

	return result ;
}

/*************************************************************
* @brief   Controls whether this agent will break when it next generates
*		   output while running.
*
*		   Now deprecated.  Use RunSelfTilOutput instead.
*
* @param state	If true, causes Soar to break on output.  If false, Soar will not break.
*************************************************************/
/*
bool Agent::SetStopSelfOnOutput(bool state)
{
	AnalyzeXML response ;

	bool ok = GetConnection()->SendAgentCommand(&response, sml_Names::kCommand_StopOnOutput, GetAgentName(), sml_Names::kParamValue, state ? sml_Names::kTrue : sml_Names::kFalse) ;
	return ok ;
}
*/

/*************************************************************
* @brief   Run Soar until either output is generated or
*		   the maximum number of decisions is reached.
*
* This function also calls "ClearOutputLinkChanges" so methods
* like "IsJustAdded" will refer to the changes that occur as a result of
* this run.
*
* This function also calls "Commit" to make sure any pending input
* link changes have been sent to Soar.
*
* We don't generally want Soar to just run until it generates
* output without any limit as an error in the AI logic might cause
* it to never return control to the environment, so there is a maximum
* decision count (currently 15) and if the agent fails to produce output
* before then this command returns.  (This value can be changed with the
* max-nil-output-cycles command).
*************************************************************/
char const* Agent::RunSelfTilOutput()
{
	if (IsCommitRequired())
	{
		assert(false) ;
		return "Need to commit changes before calling a run method" ;
	}

#ifdef SML_DIRECT
		if (GetConnection()->IsDirectConnection())
		{
			((EmbeddedConnection*)GetConnection())->DirectRun(this->GetAgentName(), false, sml_UNTIL_OUTPUT, sml_INTERLEAVE_PHASE, 1) ;
			return "DirectRun completed" ;
		}
#endif

	// Run this agent until it generates output.
	// For now, maxDecisions is being ignored.  We should make this a separate call
	// to set this parameter.
	std::string cmd = "run --self --output" ;

	return ExecuteCommandLine(cmd.c_str()) ;
}

/*************************************************************
* @brief Resend the complete input link to the kernel
*		 and remove our output link structures.
*		 We do this when the user issues an "init-soar" event.
*		 There should be no reason for the client to call this method directly.
*************************************************************/
void Agent::Refresh()
{
	// If this asserts fails, we had some changes to working memory that were
	// not committed and then an init-soar came in.  This is a programming error
	// as all working memory changes should be committed before other user-input (e.g. init-soar)
	// can be called.
	assert(!IsCommitRequired()) ;

	GetWM()->Refresh() ;
}

/*************************************************************
* @brief Returns the phase that the agent will execute when next
*		 asked to run.
*************************************************************/
smlPhase Agent::GetCurrentPhase()
{
	AnalyzeXML response ;

	bool ok = GetConnection()->SendAgentCommand(&response, sml_Names::kCommand_GetRunState, GetAgentName(), sml_Names::kParamValue, sml_Names::kParamPhase) ;

	if (!ok)
		return sml_INPUT_PHASE ;

	smlPhase phase = (smlPhase)response.GetResultInt((int)sml_INPUT_PHASE) ;

	return phase ;
}

/*************************************************************
* @brief Returns the current decision cycle counter.
*************************************************************/
int Agent::GetDecisionCycleCounter()
{
	AnalyzeXML response ;

	bool ok = GetConnection()->SendAgentCommand(&response, sml_Names::kCommand_GetRunState, GetAgentName(), sml_Names::kParamValue, sml_Names::kParamDecision) ;

	if (!ok)
		return 0 ;

	return response.GetResultInt(0) ;
}

/*************************************************************
* @brief Returns the current run state of the agent.
*		 Mostly of use to determine if agent halted in last run.
*************************************************************/
smlRunState Agent::GetRunState()
{
	AnalyzeXML response ;

	bool ok = GetConnection()->SendAgentCommand(&response, sml_Names::kCommand_GetRunState, GetAgentName(), sml_Names::kParamValue, sml_Names::kParamRunState) ;

	if (!ok)
		return (smlRunState)0 ;

	return (smlRunState)response.GetResultInt(0) ;
}

/*************************************************************
* @brief Process a command line command
*
* @param pCommandLine Command line string to process.
* @param echoResults  If true the results are also echoed through the smlEVENT_ECHO event, so they can appear in a debugger (or other listener)
* @param noFilter	  If true this command line by-passes any external filters that have been registered (this is not common) and is executed immediately.
* @returns The string form of output from the command.
*************************************************************/
char const* Agent::ExecuteCommandLine(char const* pCommandLine, bool echoResults, bool noFilter)
{
	return GetKernel()->ExecuteCommandLine(pCommandLine, GetAgentName(), echoResults, noFilter) ;
}

/*************************************************************
* @brief Execute a command line command and return the result
*		 as an XML object.
*
* @param pCommandLine Command line string to process.
* @param pAgentName   Agent name to apply the command line to.
* @param pResponse    The XML response will be returned within this object.
*                     The caller should allocate this and pass it in.
* @returns True if the command succeeds.
*************************************************************/
bool Agent::ExecuteCommandLineXML(char const* pCommandLine, ClientAnalyzedXML* pResponse)
{
	return GetKernel()->ExecuteCommandLineXML(pCommandLine, GetAgentName(), pResponse) ;
}

/*************************************************************
* @brief Get last command line result
*
* (This is the last result for any command sent to the kernel,
*  not just for this agent).
*
* @returns True if the last command line call succeeded.
*************************************************************/
bool Agent::GetLastCommandLineResult()
{
	return GetKernel()->GetLastCommandLineResult() ;
}

/*************************************************************
* @brief Returns true if this string is the name of a production
*		 that is currently loaded in the agent.
*************************************************************/
bool Agent::IsProductionLoaded(char const* pProductionName)
{
	if (!pProductionName)
		return false ;

	AnalyzeXML response ;

	bool ok = GetConnection()->SendAgentCommand(&response, sml_Names::kCommand_IsProductionLoaded, GetAgentName(), sml_Names::kParamName, pProductionName) ;

	if (!ok)
		return false ;

	return response.GetResultBool(false) ;
}

/*************************************************************
* @brief This method is used to update this client's representation
*		 of the input link to match what is currently on the agent's
*		 input link.
*		 Calling this method recreates the entire input link tree on the
*		 client side, invalidating any existing pointers.
*
*		 NOTE: This is the reverse of how a client normally uses the input link
*		 but can be useful for tools that wish to debug or monitor changes in the input link.
*
*		 NOTE: If two clients try to modify the input link at once we don't
*		 make any guarantees about what will or won't work.
*************************************************************/
bool Agent::SynchronizeInputLink()
{
	return GetWM()->SynchronizeInputLink() ;
}

/*************************************************************
* @brief This method is used to update this client's representation
*		 of the output link to match what is currently on the agent's
*		 output link.
*		 Calling this method recreates the entire output link tree on the
*		 client side, invalidating any existing pointers.
*
*		 NOTE: Calling this method shouldn't generally be necessary as the output link
*		 structures in the client are usually automatically kept in synch with the agent.
*		 It's here in case a client connects to an existing kernel and agent
*		 and wants to get up to date on the current state of the output link.
*************************************************************/
bool Agent::SynchronizeOutputLink()
{
	return GetWM()->SynchronizeOutputLink() ;
}
