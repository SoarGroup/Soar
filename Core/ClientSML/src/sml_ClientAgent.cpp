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

#include "sml_Utils.h"
#include "sml_ClientAgent.h"
#include "sml_ClientKernel.h"
#include "sml_Connection.h"
#include "sml_ClientIdentifier.h"
#include "sml_OutputDeltaList.h"
#include "sml_Events.h"
#include "sml_ClientXML.h"
#include "sml_ClientTraceXML.h"

#include "sml_ClientDirect.h"
#include "sml_EmbeddedConnection.h"	// For access to direct methods

#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>

#include <cassert>
#include <string>

#ifndef _WIN32
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#endif // !_WIN32

using namespace sml;
using namespace soarxml;

const char *DEBUGGER_NAME = "SoarJavaDebugger.jar";

namespace sml
{
	struct DebuggerProcessInformation
	{
#ifdef _WIN32
		STARTUPINFO debuggerStartupInfo;
		PROCESS_INFORMATION debuggerProcessInformation;
#else // _WIN32
		pid_t debuggerPid;
#endif // _WIN32
	};
}

Agent::Agent(Kernel* pKernel, char const* pName)
{
	m_Kernel = pKernel ;
	m_Name	 = pName ;
	m_CallbackIDCounter = 0 ;
	m_XMLCallback = -1 ;
	m_BlinkIfNoChange = true ;

	m_WorkingMemory.SetAgent(this) ;

	m_pDPI = 0;

	ClearError() ;
}

Agent::~Agent()
{
	KillDebugger();
}

Connection* Agent::GetConnection() const
{
	return m_Kernel->GetConnection() ;
}

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
		ReceivedRunEvent(smlRunEventId(id), pIncoming, pResponse) ;
	} else if (IsProductionEventID(id))
	{
		ReceivedProductionEvent(smlProductionEventId(id), pIncoming, pResponse) ;
	} else if (IsPrintEventID(id))
	{
		ReceivedPrintEvent(smlPrintEventId(id), pIncoming, pResponse) ;
	} else if (IsXMLEventID(id))
	{
		ReceivedXMLEvent(smlXMLEventId(id), pIncoming, pResponse) ;
	}
}

void Agent::ReceivedRunEvent(smlRunEventId id, AnalyzeXML* pIncoming, ElementXML* /*pResponse*/)
{
	smlPhase phase = smlPhase(pIncoming->GetArgInt(sml_Names::kParamPhase, -1)) ;

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

void Agent::ReceivedPrintEvent(smlPrintEventId id, AnalyzeXML* pIncoming, ElementXML* /*pResponse*/)
{
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

void Agent::ReceivedProductionEvent(smlProductionEventId id, AnalyzeXML* pIncoming, ElementXML* /*pResponse*/)
{
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

bool Agent::LoadProductions(char const* pFilename, bool echoResults)
{
    if (!pFilename)
        return false;

    // remove quotes or braces if they exist
    std::string cmd("source {");
    size_t len = strlen(pFilename);
    if ((pFilename[0] == '\"' && pFilename[len - 1] == '\"') || (pFilename[0] == '{' && pFilename[len - 1] == '}'))
        cmd.append(pFilename + 1, len - 2);
    else
        cmd.append(pFilename, len);
    cmd.push_back('}');

	// Execute the source command, insert braces
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

int	Agent::AddOutputHandler(char const* pAttributeName, OutputEventHandler handler, void* pUserData, bool addToBack)
{
    m_WorkingMemory.SetOutputLinkChangeTracking(true);

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

bool Agent::IsRegisteredForOutputEvent()
{
	return m_OutputEventMap.getSize() > 0 ;
}

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

void Agent::ReceivedXMLEvent(smlXMLEventId id, AnalyzeXML* pIncoming, ElementXML* /*pResponse*/)
{
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

void Agent::ReceivedXMLTraceEvent(smlXMLEventId id, ElementXML* pIncoming, ElementXML* /*pResponse*/)
{
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

Identifier* Agent::GetInputLink()
{
	return GetWM()->GetInputLink() ;
}

Identifier* Agent::GetOutputLink()
{
	return GetWM()->GetOutputLink() ;
}

int	Agent::GetNumberOutputLinkChanges()
{
	OutputDeltaList* pDeltas = GetWM()->GetOutputLinkChanges() ;
	return pDeltas->GetSize() ;
}

WMElement* Agent::GetOutputLinkChange(int index)
{
	OutputDeltaList* pDeltas = GetWM()->GetOutputLinkChanges() ;
	WMDelta* pDelta = pDeltas->GetDeltaWME(index) ;

	if (!pDelta)
		return NULL ;

	return pDelta->getWME() ;
}

bool Agent::IsOutputLinkChangeAdd(int index)
{
	OutputDeltaList* pDeltas = GetWM()->GetOutputLinkChanges() ;
	WMDelta* pDelta = pDeltas->GetDeltaWME(index) ;

	if (!pDelta)
		return false ;

	bool isAddition = (pDelta->getChangeType() == WMDelta::kAdded) ;

	return isAddition ;
}

void Agent::ClearOutputLinkChanges()
{
	GetWM()->ClearOutputLinkChanges() ;
}

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
				return static_cast<Identifier*>(pWME) ;
			index-- ;
		}
	}

	return NULL ;
}

StringElement* Agent::CreateStringWME(Identifier* parent, char const* pAttribute, char const* pValue)
{
	if (!parent || parent->GetAgent() != this)
		return NULL ;

	return GetWM()->CreateStringWME(parent, pAttribute, pValue) ;
}

Identifier* Agent::CreateIdWME(Identifier* parent, char const* pAttribute)
{
	if (!parent || parent->GetAgent() != this)
		return NULL ;

	return GetWM()->CreateIdWME(parent, pAttribute) ;
}

Identifier*	Agent::CreateSharedIdWME(Identifier* parent, char const* pAttribute, Identifier* pSharedValue)
{
	if (!parent || parent->GetAgent() != this || !pSharedValue)
		return NULL ;

	return GetWM()->CreateSharedIdWME(parent, pAttribute, pSharedValue) ;
}

IntElement* Agent::CreateIntWME(Identifier* parent, char const* pAttribute, long long value)
{
	if (!parent || parent->GetAgent() != this)
		return NULL ;

	return GetWM()->CreateIntWME(parent, pAttribute, value) ;
}

FloatElement* Agent::CreateFloatWME(Identifier* parent, char const* pAttribute, double value)
{
	if (!parent || parent->GetAgent() != this)
		return NULL ;

	return GetWM()->CreateFloatWME(parent, pAttribute, value) ;
}

void Agent::Update(StringElement* pWME, char const* pValue) 
{ 
	GetWM()->UpdateString(pWME, pValue) ; 
}
void Agent::Update(IntElement* pWME, long long value)				
{ 
	GetWM()->UpdateInt(pWME, value) ; 
}
void Agent::Update(FloatElement* pWME, double value)		
{ 
	GetWM()->UpdateFloat(pWME, value) ; 
}

bool Agent::DestroyWME(WMElement* pWME)
{
	if (!pWME || pWME->GetAgent() != this)
		return false ;

	return GetWM()->DestroyWME(pWME) ;
}

bool Agent::Commit()
{
	return GetWM()->Commit() ;
}

bool Agent::IsCommitRequired()
{
	return GetWM()->IsCommitRequired() ;
}

bool Agent::IsAutoCommitEnabled()
{
	return m_Kernel->IsAutoCommitEnabled() ;
}

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

char const*	Agent::StopSelf()
{
	std::string cmd = "stop-soar --self" ;

	// Execute the command.
	char const* pResult = ExecuteCommandLine(cmd.c_str()) ;
	return pResult ;
}

char const* Agent::RunSelf(int numberSteps, smlRunStepSize stepSize)
{
	if (IsCommitRequired())
	{
		assert(false) ;
		return "Need to commit changes before calling a run method" ;
	}	

#ifdef SML_DIRECT
	if (GetConnection()->IsDirectConnection())
	{
		EmbeddedConnection* ec = static_cast<EmbeddedConnection*>(GetConnection());
		ec->DirectRun(this->GetAgentName(), false, stepSize, sml_DECISION, numberSteps) ;
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
			EmbeddedConnection*	ec = static_cast<EmbeddedConnection*>(GetConnection());
			ec->DirectRun(this->GetAgentName(), true, sml_DECISION, sml_PHASE, 1) ;
			return "DirectRun completed" ;
		}
#endif

	// Create the command line for the run command
	std::string cmd = "run --self" ;

	// Execute the run command.
	char const* pResult = ExecuteCommandLine(cmd.c_str()) ;
	return pResult ;
}

bool Agent::WasAgentOnRunList()
{
	AnalyzeXML response ;

	bool ok = GetConnection()->SendAgentCommand(&response, sml_Names::kCommand_WasAgentOnRunList, GetAgentName()) ;

	if (!ok)
		return false ;

	bool wasRun = response.GetResultBool(false) ;
	return wasRun ;
}

smlRunResult Agent::GetResultOfLastRun()
{
	AnalyzeXML response ;

	bool ok = GetConnection()->SendAgentCommand(&response, sml_Names::kCommand_GetResultOfLastRun, GetAgentName()) ;

	if (!ok)
		return sml_RUN_ERROR ;

	smlRunResult result = smlRunResult(response.GetResultInt(int(sml_RUN_ERROR))) ;

	return result ;
}

/*
bool Agent::SetStopSelfOnOutput(bool state)
{
	AnalyzeXML response ;

	bool ok = GetConnection()->SendAgentCommand(&response, sml_Names::kCommand_StopOnOutput, GetAgentName(), sml_Names::kParamValue, state ? sml_Names::kTrue : sml_Names::kFalse) ;
	return ok ;
}
*/

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
			EmbeddedConnection*	ec = static_cast<EmbeddedConnection*>(GetConnection());
			ec->DirectRun(this->GetAgentName(), false, sml_UNTIL_OUTPUT, sml_PHASE, 1) ;
			return "DirectRun completed" ;
		}
#endif

	// Run this agent until it generates output.
	// For now, maxDecisions is being ignored.  We should make this a separate call
	// to set this parameter.
	std::string cmd = "run --self --output" ;

	return ExecuteCommandLine(cmd.c_str()) ;
}

void Agent::Refresh()
{
	// If this asserts fails, we had some changes to working memory that were
	// not committed and then an init-soar came in.  This is a programming error
	// as all working memory changes should be committed before other user-input (e.g. init-soar)
	// can be called.
	assert(!IsCommitRequired()) ;

	GetWM()->Refresh() ;
}

smlPhase Agent::GetCurrentPhase()
{
	AnalyzeXML response ;

	bool ok = GetConnection()->SendAgentCommand(&response, sml_Names::kCommand_GetRunState, GetAgentName(), sml_Names::kParamValue, sml_Names::kParamPhase) ;

	if (!ok)
		return sml_INPUT_PHASE ;

	smlPhase phase = smlPhase(response.GetResultInt(int(sml_INPUT_PHASE))) ;

	return phase ;
}

int Agent::GetDecisionCycleCounter()
{
	AnalyzeXML response ;

	bool ok = GetConnection()->SendAgentCommand(&response, sml_Names::kCommand_GetRunState, GetAgentName(), sml_Names::kParamValue, sml_Names::kParamDecision) ;

	if (!ok)
		return 0 ;

	return response.GetResultInt(0) ;
}

smlRunState Agent::GetRunState()
{
	AnalyzeXML response ;

	bool ok = GetConnection()->SendAgentCommand(&response, sml_Names::kCommand_GetRunState, GetAgentName(), sml_Names::kParamValue, sml_Names::kParamRunState) ;

	if (!ok)
		return smlRunState(0) ;

	return smlRunState(response.GetResultInt(0)) ;
}

char const* Agent::ExecuteCommandLine(char const* pCommandLine, bool echoResults, bool noFilter)
{
	return GetKernel()->ExecuteCommandLine(pCommandLine, GetAgentName(), echoResults, noFilter) ;
}

bool Agent::ExecuteCommandLineXML(char const* pCommandLine, ClientAnalyzedXML* pResponse)
{
	return GetKernel()->ExecuteCommandLineXML(pCommandLine, GetAgentName(), pResponse) ;
}

bool Agent::GetLastCommandLineResult()
{
	return GetKernel()->GetLastCommandLineResult() ;
}

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

bool Agent::SynchronizeInputLink()
{
	return GetWM()->SynchronizeInputLink() ;
}

bool Agent::SynchronizeOutputLink()
{
	return GetWM()->SynchronizeOutputLink() ;
}

// Test if a path exists and is not a directory.
bool isfile(const char *path)
{
#ifdef _WIN32
	DWORD a = GetFileAttributes(path);
	return a != INVALID_FILE_ATTRIBUTES && !(a & FILE_ATTRIBUTE_DIRECTORY);
#else
	struct stat st;
	return (stat(path, &st) == 0 && !S_ISDIR(st.st_mode));
#endif
}

bool Agent::SpawnDebugger(int port, const char* jarpath)
{
	std::string p;
	if (jarpath) {
		if (!isfile(jarpath)) {
			return false;
		}
		p = jarpath;
	} else if (isfile(DEBUGGER_NAME)) {
		p = DEBUGGER_NAME;
	} else {
		char *e = getenv("SOAR_HOME");
		if (!e) {
			return false;
		}
		std::string h(e);
		if (h.find_last_of("/\\") != h.size() - 1) {
			h += '/';
		}
		h += DEBUGGER_NAME;
		if (!isfile(h.c_str())) {
			return false;
		}
		p = h;
	}
	
	if (port == -1)
		port = m_Kernel->GetListenerPort();

	if (m_pDPI) 
		return false;
	m_pDPI = new DebuggerProcessInformation();

#ifdef _WIN32
	ZeroMemory( &m_pDPI->debuggerStartupInfo, sizeof( m_pDPI->debuggerStartupInfo ) );
	m_pDPI->debuggerStartupInfo.cb = sizeof( m_pDPI->debuggerStartupInfo );
	ZeroMemory( &m_pDPI->debuggerProcessInformation, sizeof( m_pDPI->debuggerProcessInformation ) );

	// Start the child process. 
	std::stringstream commandLine;
	commandLine << "javaw.exe -jar \"" << p << "\" -remote -port " << port << " -agent \"" << this->GetAgentName() << "\"";

	BOOL ret = CreateProcess(
		0,
		const_cast< LPSTR >( commandLine.str().c_str() ),	// Command line
		0,								// Process handle not inheritable
		0,								// Thread handle not inheritable
		FALSE,							// Set handle inheritance to FALSE
		0,								// No creation flags
		0,								// Use parent's environment block
		0,								// Use parent's starting directory 
		&m_pDPI->debuggerStartupInfo,			// Pointer to STARTUPINFO structure
		&m_pDPI->debuggerProcessInformation );	// Pointer to PROCESS_INFORMATION structure

	if ( ret == 0 ) 
	{
		std::cout << "Error code: " << GetLastError() << std::endl;
		delete m_pDPI;
		m_pDPI = 0;
		return false;
	}
	return true;

#else // _WIN32
	m_pDPI->debuggerPid = fork();
	if ( m_pDPI->debuggerPid < 0 ) 
	{ 
		delete m_pDPI;
		m_pDPI = 0;
		return false;
	}

	if ( m_pDPI->debuggerPid == 0 ) 
	{
		// child
		std::string portstring;
		to_string(port, portstring);

    std::string java_library_path = "-Djava.library.path=" + p;
    for(int i = java_library_path.size() - 1; i != -1; --i) {
      if(java_library_path[i] == '/') {
        java_library_path.resize(i);
        break;
      }
    }

#ifdef SCONS_DARWIN
		execlp("java", "java", "-XstartOnFirstThread", java_library_path.c_str(), "-jar", p.c_str(), "-remote", 
			"-port", portstring.c_str(), "-agent", this->GetAgentName(), NULL );
#else
		execlp("java", "java", java_library_path.c_str(), "-jar", p.c_str(), "-remote", 
			"-port", portstring.c_str(), "-agent", this->GetAgentName(), 0 );
#endif
		// does not return on success

		std::cerr << "Debugger spawn failed: " << strerror(errno) << std::endl;
		exit(1);
	}

	// parent
	return true;
#endif // _WIN32
}

bool Agent::KillDebugger()
{
	if (!m_pDPI)
		return false;
	bool successful = false;

#ifdef _WIN32
	// Wait until child process exits.
	BOOL ret = TerminateProcess(m_pDPI->debuggerProcessInformation.hProcess, 0);
	CloseHandle( m_pDPI->debuggerProcessInformation.hProcess );
	CloseHandle( m_pDPI->debuggerProcessInformation.hThread );
	if (ret) 
		successful = true;

#else // _WIN32
	if ( !kill( m_pDPI->debuggerPid, SIGTERM ) )
	{
		successful = true;
	}
#endif // _WIN32

	delete m_pDPI;
	m_pDPI = 0;
	return successful;
}

char const* Agent::ConvertIdentifier(char const* pClientIdentifier)
{
	// need to keep the result around after the function returns
	// bad
	static std::string kernelIdentifier;

	AnalyzeXML response;

	// Send the command to the kernel
	bool ret = m_Kernel->GetConnection()->SendAgentCommand(&response, sml_Names::kCommand_ConvertIdentifier, GetAgentName(), sml_Names::kParamName, pClientIdentifier);

	if (ret)
	{
		// Get the result as a string
		char const *pResult = response.GetResultString();
		if (pResult && strlen(pResult)) 
		{
			kernelIdentifier.assign(pResult);
			return kernelIdentifier.c_str();
		} 
	}
	return pClientIdentifier;
}
