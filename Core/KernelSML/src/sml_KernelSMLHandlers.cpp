#include <portability.h>

/////////////////////////////////////////////////////////////////
// KernelSML handlers file.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : August 2004
//
// These are the command handler methods for KernelSML.
// Just moved to a separate implementation file to
// keep the code more manageable.
//
/////////////////////////////////////////////////////////////////

#include "sml_KernelSML.h"

#include "sml_Utils.h"
#include "sml_AgentSML.h"
#include "sml_Connection.h"
#include "sml_OutputListener.h"
#include "sml_ConnectionManager.h"
#include "sml_TagResult.h"
#include "sml_TagName.h"
#include "sml_TagWme.h"
#include "sml_TagFilter.h"
#include "sml_TagCommand.h"
#include "sml_Events.h"
#include "sml_RunScheduler.h"
#include "KernelHeaders.h"

#include <iostream>
#include <fstream>
#include <map>
#include <stdlib.h>
#include <assert.h>
#include <algorithm>
#include "xml.h"

using namespace sml ;

void KernelSML::BuildCommandMap()
{
	m_CommandMap[sml_Names::kCommand_CreateAgent]		= &sml::KernelSML::HandleCreateAgent ;
	m_CommandMap[sml_Names::kCommand_DestroyAgent]		= &sml::KernelSML::HandleDestroyAgent ;
	m_CommandMap[sml_Names::kCommand_GetInputLink]		= &sml::KernelSML::HandleGetInputLink ;
	m_CommandMap[sml_Names::kCommand_Input]				= &sml::KernelSML::HandleInput ;
	m_CommandMap[sml_Names::kCommand_CommandLine]		= &sml::KernelSML::HandleCommandLine ;
	m_CommandMap[sml_Names::kCommand_ExpandCommandLine]	= &sml::KernelSML::HandleExpandCommandLine ;
	m_CommandMap[sml_Names::kCommand_CheckForIncomingCommands] = &sml::KernelSML::HandleCheckForIncomingCommands ;
	m_CommandMap[sml_Names::kCommand_GetAgentList]		= &sml::KernelSML::HandleGetAgentList ;
	m_CommandMap[sml_Names::kCommand_RegisterForEvent]	= &sml::KernelSML::HandleRegisterForEvent ;
	m_CommandMap[sml_Names::kCommand_UnregisterForEvent]= &sml::KernelSML::HandleRegisterForEvent ;	// Note -- both register and unregister go to same handler
	m_CommandMap[sml_Names::kCommand_FireEvent]			= &sml::KernelSML::HandleFireEvent ;
	m_CommandMap[sml_Names::kCommand_SuppressEvent]		= &sml::KernelSML::HandleSuppressEvent ;
	m_CommandMap[sml_Names::kCommand_SetInterruptCheckRate] = &sml::KernelSML::HandleSetInterruptCheckRate ;
	m_CommandMap[sml_Names::kCommand_GetVersion]		= &sml::KernelSML::HandleGetVersion ;
	m_CommandMap[sml_Names::kCommand_Shutdown]			= &sml::KernelSML::HandleShutdown ;
	m_CommandMap[sml_Names::kCommand_IsSoarRunning]		= &sml::KernelSML::HandleIsSoarRunning ;
	m_CommandMap[sml_Names::kCommand_GetConnections]	= &sml::KernelSML::HandleGetConnections ;
	m_CommandMap[sml_Names::kCommand_SetConnectionInfo] = &sml::KernelSML::HandleSetConnectionInfo ;
	m_CommandMap[sml_Names::kCommand_GetAllInput]		= &sml::KernelSML::HandleGetAllInput ;
	m_CommandMap[sml_Names::kCommand_GetAllOutput]		= &sml::KernelSML::HandleGetAllOutput ;
	m_CommandMap[sml_Names::kCommand_GetRunState]		= &sml::KernelSML::HandleGetRunState ;
	m_CommandMap[sml_Names::kCommand_IsProductionLoaded]= &sml::KernelSML::HandleIsProductionLoaded ;
	m_CommandMap[sml_Names::kCommand_SendClientMessage] = &sml::KernelSML::HandleSendClientMessage ;
	m_CommandMap[sml_Names::kCommand_WasAgentOnRunList] = &sml::KernelSML::HandleWasAgentOnRunList ;
	m_CommandMap[sml_Names::kCommand_GetResultOfLastRun]= &sml::KernelSML::HandleGetResultOfLastRun ;
	m_CommandMap[sml_Names::kCommand_GetInitialTimeTag] = &sml::KernelSML::HandleGetInitialTimeTag ;
	m_CommandMap[sml_Names::kCommand_ConvertIdentifier] = &sml::KernelSML::HandleConvertIdentifier;
	m_CommandMap[sml_Names::kCommand_GetListenerPort]	= &sml::KernelSML::HandleGetListenerPort;
	m_CommandMap[sml_Names::kCommand_GetLibraryLocation]= &sml::KernelSML::HandleGetLibraryLocation;
}

/*************************************************************
* @brief	A command handler (SML message->appropriate gSKI handling).
*
* @param pAgent			The agent this command is for (can be NULL if the command is not agent specific)
* @param pCommandName	The SML command name (so one handler can handle many incoming calls if we wish)
* @param pConnection	The connection this command came in on
* @param pIncoming		The incoming, analyzed message.
* @param pResponse		The partially formed response.  This handler needs to fill in more of this.
* @returns False if we had an error and wish to generate a generic error message (based on the incoming call + pError)
*          True if the call succeeded or we generated another more specific error already.
*************************************************************/
bool KernelSML::HandleCreateAgent(AgentSML* pAgentSML, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, soarxml::ElementXML* pResponse)
{
	assert( !pAgentSML ); // FIXME: handle gracefully

	// Get the parameters
	char const* pName = pIncoming->GetArgString(sml_Names::kParamName) ;

	if (!pName)
	{
		return InvalidArg(pConnection, pResponse, pCommandName, "Agent name missing") ;
	}

	agent* pSoarAgent = create_soar_agent( const_cast< char* >(pName) );

	pAgentSML = new AgentSML(this, pSoarAgent) ;

	// Update our maps
	m_KernelAgentMap[ pSoarAgent ] = pAgentSML ;
	m_AgentMap[ pAgentSML->GetName() ] = pAgentSML ;

	pAgentSML->InitListeners() ;	// This must happen before the soar agent is initialized

	pAgentSML->Init() ;

	// Notify listeners that there is a new agent
	this->FireAgentEvent(pAgentSML, smlEVENT_AFTER_AGENT_CREATED) ;

	xml_invoke_callback( pAgentSML->GetSoarAgent() );

	// Register for output from this agent

	// Mark the agent's status as just having been created for all connections
	// Note--agent status for connections just refers to the last agent created, i.e. this one.
	m_pConnectionManager->SetAgentStatus(sml_Names::kStatusCreated) ;

	// We also need to listen to input events so we can pump waiting sockets and get interrupt messages etc.
	// moved to sml_InputListener.cpp

	//pAgentSML->m_inputlink->GetInputLinkMemory()->m_RemoveWmeCallback = RemoveInputWMERecordsCallback;

	if (this->m_pRunScheduler->IsRunning()) 
	{
		// bug 952: if soar is running, the agent should start running

		// FIXME: this is duplicated code from the following functions:
		// InitializeRunCounters():
		pAgentSML->ResetLastOutputCount() ;
		uint64_t count = pAgentSML->GetRunCounter(this->m_pRunScheduler->GetCurrentRunStepSize()) ;
		pAgentSML->SetInitialRunCount(count) ;
		pAgentSML->ResetLocalRunCounters() ;
		// InitializeUpdateWorldEvents():
		pAgentSML->SetCompletedOutputPhase(false) ;
		pAgentSML->SetGeneratedOutput(false) ;
		pAgentSML->SetInitialOutputCount(pAgentSML->GetNumOutputsGenerated()) ;
		pAgentSML->GetAgentRunCallback()->RegisterWithKernel(smlEVENT_AFTER_OUTPUT_PHASE) ;

		this->m_pRunScheduler->ScheduleAgentToRun(pAgentSML, true);
	}

	// Return true if we got an agent constructed.
	return true ;
}

// Handle registering and unregistering for kernel events
bool KernelSML::HandleRegisterForEvent(AgentSML* pAgentSML, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, soarxml::ElementXML* pResponse)
{
	// Decide if registering or unregistering
	bool registerForEvent = (strcmp(pCommandName, sml_Names::kCommand_RegisterForEvent) == 0) ;

	// Get the parameters
	char const* pEventName = pIncoming->GetArgString(sml_Names::kParamEventID) ;

	if (!pEventName)
	{
		return InvalidArg(pConnection, pResponse, pCommandName, "Event id is missing") ;
	}

	// Convert from the event name to the id value
	int id = ConvertStringToEvent(pEventName) ;

	// Decide what type of event this is and where to register/unregister it
	if(IsSystemEventID(id))
	{
		// System Events
		if (registerForEvent)
			this->AddSystemListener(static_cast<smlSystemEventId>(id), pConnection) ;
		else
			this->RemoveSystemListener(static_cast<smlSystemEventId>(id), pConnection) ;

	} else if(IsAgentEventID(id)) {

		// Agent events
		if (registerForEvent)
			this->AddAgentListener(static_cast<smlAgentEventId>(id), pConnection) ;
		else
			this->RemoveAgentListener(static_cast<smlAgentEventId>(id), pConnection) ;
	} else if(IsRhsEventID(id)) {

		// Rhs user functions
		char const* pRhsFunctionName = pIncoming->GetArgString(sml_Names::kParamName) ;

		if (!pRhsFunctionName)
			return InvalidArg(pConnection, pResponse, pCommandName, "Registering for rhs user function, but no function name was provided") ;

		if (registerForEvent)
			this->AddRhsListener(pRhsFunctionName, pConnection) ;
		else
			this->RemoveRhsListener(pRhsFunctionName, pConnection) ;
	} else if(IsRunEventID(id)) {

		// Run events
		if (!pAgentSML)
			return InvalidArg(pConnection, pResponse, pCommandName, "No agent name for an event that is handled by an agent") ;

		// Register or unregister for this event
		if (registerForEvent)
			pAgentSML->AddRunListener(static_cast<smlRunEventId>(id), pConnection) ;
		else
			pAgentSML->RemoveRunListener(static_cast<smlRunEventId>(id), pConnection) ;
	} else if(IsProductionEventID(id)) {

		// Production event
		if (!pAgentSML)
			return InvalidArg(pConnection, pResponse, pCommandName, "No agent name for an event that is handled by an agent") ;

		// Register or unregister for this event
		if (registerForEvent)
			pAgentSML->AddProductionListener(static_cast<smlProductionEventId>(id), pConnection) ;
		else
			pAgentSML->RemoveProductionListener(static_cast<smlProductionEventId>(id), pConnection) ;
	} else if(IsXMLEventID(id)) {

		// XML Event
		if (!pAgentSML)
			return InvalidArg(pConnection, pResponse, pCommandName, "No agent name for an event that is handled by an agent") ;

		// Register or unregister for this event
		if (registerForEvent)
			pAgentSML->AddXMLListener(static_cast<smlXMLEventId>(id), pConnection) ;
		else
			pAgentSML->RemoveXMLListener(static_cast<smlXMLEventId>(id), pConnection) ;

	} else if (IsUpdateEventID(id))	{
		if (registerForEvent)
			AddUpdateListener(static_cast<smlUpdateEventId>(id), pConnection) ;
		else
			RemoveUpdateListener(static_cast<smlUpdateEventId>(id), pConnection) ;
	} else if (IsStringEventID(id))	{
		if (registerForEvent)
			AddStringListener(static_cast<smlStringEventId>(id), pConnection) ;
		else
			RemoveStringListener(static_cast<smlStringEventId>(id), pConnection) ;
	}
	else if(IsPrintEventID(id)) {

		// Print event
		if (!pAgentSML)
			return InvalidArg(pConnection, pResponse, pCommandName, "No agent name for an event that is handled by an agent") ;

		// Register or unregister for this event
		if (registerForEvent)
			pAgentSML->AddPrintListener(static_cast<smlPrintEventId>(id), pConnection) ;
		else
			pAgentSML->RemovePrintListener(static_cast<smlPrintEventId>(id), pConnection) ;
	} else if( id == smlEVENT_OUTPUT_PHASE_CALLBACK ) {

		// Output event
		OutputListener* pOutputListener = pAgentSML->GetOutputListener() ;

		// Register this connection as listening for this event
		if (registerForEvent)
			pOutputListener->AddListener(smlEVENT_OUTPUT_PHASE_CALLBACK, pConnection) ;
		else
			pOutputListener->RemoveListener(smlEVENT_OUTPUT_PHASE_CALLBACK, pConnection) ;
	} else {
		// The event didn't match any of our handlers
		return InvalidArg(pConnection, pResponse, pCommandName, "KernelSML doesn't know how to handle that event id") ;
	}

	return true ;
}

bool KernelSML::HandleSetConnectionInfo(AgentSML* /*pAgentSML*/, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, soarxml::ElementXML* pResponse)
{
	// Get the parameters
	char const* pName   = pIncoming->GetArgString(sml_Names::kConnectionName) ;
	char const* pStatus = pIncoming->GetArgString(sml_Names::kConnectionStatus) ;
	char const* pAgentStatus = pIncoming->GetArgString(sml_Names::kAgentStatus) ;

	if (!pName)
	{
		return InvalidArg(pConnection, pResponse, pCommandName, "Connection name is missing") ;
	}

	if (!pStatus)
	{
		return InvalidArg(pConnection, pResponse, pCommandName, "Connection status is missing") ;
	}

	if (!pAgentStatus)
	{
		return InvalidArg(pConnection, pResponse, pCommandName, "Agent status is missing") ;
	}

	// Execute the command
	pConnection->SetName(pName) ;
	pConnection->SetStatus(pStatus) ;
	pConnection->SetAgentStatus(pAgentStatus) ;

	return true ;
}

bool KernelSML::HandleGetConnections(AgentSML* /*pAgentSML*/, char const* /*pCommandName*/, Connection* /*pCallingConnection*/, AnalyzeXML* /*pIncoming*/, soarxml::ElementXML* pResponse)
{
	// Create the result tag
	TagResult* pTagResult = new TagResult() ;
	pTagResult->AddAttribute(sml_Names::kCommandOutput, sml_Names::kStructuredOutput) ;

	// Walk the list of connections and return their info
	int index = 0 ;
	Connection* pConnection = m_pConnectionManager->GetConnectionByIndex(index) ;

	while (pConnection)
	{
		// Create the connection tag
		soarxml::ElementXML* pTagConnection = new soarxml::ElementXML() ;
		pTagConnection->SetTagName(sml_Names::kTagConnection) ;

		// Fill in the info
		pTagConnection->AddAttribute(sml_Names::kConnectionId, pConnection->GetID()) ;
		pTagConnection->AddAttribute(sml_Names::kConnectionName, pConnection->GetName()) ;
		pTagConnection->AddAttribute(sml_Names::kConnectionStatus, pConnection->GetStatus()) ;
		pTagConnection->AddAttribute(sml_Names::kAgentStatus, pConnection->GetAgentStatus()) ;

		// Add the connection into the result
		pTagResult->AddChild(pTagConnection) ;

		// Get the next connection.  Returns null when go beyond limit.
		// (This provides thread safe access to the list, in case it changes during this enumeration)
		index++ ;
		pConnection = m_pConnectionManager->GetConnectionByIndex(index) ;
	}

	// Add the result tag to the response
	pResponse->AddChild(pTagResult) ;

	// Return true to indicate we've filled in all of the result tag we need
	return true ;

}

bool KernelSML::HandleDestroyAgent(AgentSML* pAgentSML, char const* /*pCommandName*/, Connection* /*pConnection*/, AnalyzeXML* /*pIncoming*/, soarxml::ElementXML* /*pResponse*/)
{
	if (!pAgentSML)
		return false ;

	FireAgentEvent( pAgentSML, smlEVENT_BEFORE_AGENT_DESTROYED );

	// Close log
	if (m_CommandLineInterface.IsLogOpen())
	{
		m_CommandLineInterface.DoCommand(0, pAgentSML, "clog --close", false, true, 0) ;
	}

	// Release any wmes or other objects we're keeping
	pAgentSML->DeleteSelf() ;
	pAgentSML = NULL ;	// At this point the pointer is invalid so clear it.

	return true ;
}

// Shutdown is an irrevocal request to delete all agents and prepare for kernel deletion.
bool KernelSML::HandleShutdown(AgentSML* /*pAgentSML*/, char const* /*pCommandName*/, Connection* /*pConnection*/, AnalyzeXML* /*pIncoming*/, soarxml::ElementXML* /*pResponse*/)
{
	// Notify everyone that the system is about to shutdown.
	FireSystemEvent(smlEVENT_BEFORE_SHUTDOWN) ;

	// Delete all agents explicitly now (so listeners can hear that the agents have been destroyed).
	DeleteAllAgents(true) ;

	return true ;
}

// Return information about the current runtime state of the agent (e.g. phase, decision cycle count etc.)
bool KernelSML::HandleGetRunState(AgentSML* pAgentSML, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, soarxml::ElementXML* pResponse)
{
	// Look up what type of information to report.
	char const* pValue = pIncoming->GetArgString(sml_Names::kParamValue) ;

	if (!pValue)
	{
		return InvalidArg(pConnection, pResponse, pCommandName, "Need to specify the type of information wanted.") ;
	}

	std::ostringstream buffer;

	if (strcmp(pValue, sml_Names::kParamPhase) == 0)
	{
		// Report the current phase.
		buffer << pAgentSML->GetCurrentPhase();
	}
	else if (strcmp(pValue, sml_Names::kParamDecision) == 0)
	{
		// Report the current decision number of decisions that have been executed
		buffer << pAgentSML->GetNumDecisionsExecuted();
	}
	else if (strcmp(pValue, sml_Names::kParamRunState) == 0) 	 
	{ 	 
		// Report the current run state 	 
		buffer << pAgentSML->GetRunState() ; 	 
	}
	else
	{
		return InvalidArg(pConnection, pResponse, pCommandName, "Didn't recognize the type of information requested in GetRunState().") ;
	}

	std::string bufferStdString = buffer.str();
	const char* bufferCString = bufferStdString.c_str();
	return this->ReturnResult(pConnection, pResponse, bufferCString) ;
}

// Return information about the current runtime state of the agent (e.g. phase, decision cycle count etc.)
bool KernelSML::HandleWasAgentOnRunList(AgentSML* pAgentSML, char const* /*pCommandName*/, Connection* pConnection, AnalyzeXML* /*pIncoming*/, soarxml::ElementXML* pResponse)
{
	bool wasRun = pAgentSML->WasAgentOnRunList() ;
	return this->ReturnBoolResult(pConnection, pResponse, wasRun) ;
}

// Return the result code from the last run
bool KernelSML::HandleGetResultOfLastRun(AgentSML* pAgentSML, char const* /*pCommandName*/, Connection* pConnection, AnalyzeXML* /*pIncoming*/, soarxml::ElementXML* pResponse)
{
	smlRunResult runResult = pAgentSML->GetResultOfLastRun() ;
	return this->ReturnIntResult(pConnection, pResponse, runResult) ;
}

// Return a starting value for client side time tags for this client to use 	 
bool KernelSML::HandleGetInitialTimeTag(AgentSML* /*pAgentSML*/, char const* /*pCommandName*/, Connection* pConnection, AnalyzeXML* /*pIncoming*/, soarxml::ElementXML* pResponse) 	 
{ 	 
	// We use negative values for client time tags (so we can tell they're client side not kernel side) 	 
	int64_t timeTagStart = -1 ; 	 

	// Allow up to 8 simultaneous clients using different ids 	 
	int maxTries = 8 ; 	 
	bool done = false ; 	 

	while (maxTries > 0 && !done) 	 
	{ 	 
		// Walk the list of connections and see if we can find an time tag start value that's not in use 	 
		// (We do this walking so if connections are made, broken and remade and we'll reuse the id space). 	 
		int index = 0 ; 	 
		Connection* connect = m_pConnectionManager->GetConnectionByIndex(index) ; 	 

		// See if any existing connection is using the timeTagStart value already 	 
		bool ok = true ; 	 
		while (connect && ok) 	 
		{ 	 
			if (connect->GetInitialTimeTagCounter() == timeTagStart) 	 
			{ 	 
				ok = false ; 	 
				timeTagStart -= (1<<27) ;       // 8 * (1<<27) is (1<<30) so won't overflow.  Allows (1<<27) values per client w/o collision or more than 100 million wmes each. 	 
			} 	 

			index++ ; 	 
			connect = m_pConnectionManager->GetConnectionByIndex(index) ; 	 
		} 	 

		// If this value's not already in use we're done 	 
		// Otherwise, we'll test the new value. 	 
		if (ok) 	 
			done = true ; 	 

		maxTries-- ; 	 
	} 	 

	// If we fail this it means we couldn't find a valid start value for the time tag counter. 	 
	// Either we have 8 existing connections or there's a bug in this code. 	 
	assert(maxTries >= 0) ; 	 

	// Record the value we picked and return it. 	 
	pConnection->SetInitialTimeTagCounter(timeTagStart) ; 	 
	return this->ReturnIntResult(pConnection, pResponse, timeTagStart) ; 	 
}

bool KernelSML::HandleConvertIdentifier(AgentSML* pAgentSML, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, soarxml::ElementXML* pResponse) 	 
{
	// Get the identifier to convert
	char const* pClientId = pIncoming->GetArgString(sml_Names::kParamName) ;

	if (!pClientId)
	{
		return InvalidArg(pConnection, pResponse, pCommandName, "Need to specify the client-side identifier to convert.") ;
	}

	std::string convertedId;
	if (pAgentSML->ConvertID(pClientId, &convertedId)) 
	{
		return ReturnResult(pConnection, pResponse, convertedId.c_str()) ;
	} 
	else 
	{
		return ReturnResult(pConnection, pResponse, "") ;
	}
}

 // Returns true if the production name is currently loaded
bool KernelSML::HandleIsProductionLoaded(AgentSML* pAgentSML, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, soarxml::ElementXML* pResponse)
{
	// Look up the name of the production
	char const* pName = pIncoming->GetArgString(sml_Names::kParamName) ;

	if (!pName)
	{
		return InvalidArg(pConnection, pResponse, pCommandName, "Need to specify the production name to check.") ;
	}

	Symbol* sym = find_sym_constant( pAgentSML->GetSoarAgent(), pName );

	bool found = true;
	if (!sym || !(sym->sc.production))
	{
		found = false;
	}

	return ReturnBoolResult(pConnection, pResponse, found) ;
}

bool KernelSML::HandleGetVersion(AgentSML* /*pAgentSML*/, char const* /*pCommandName*/, Connection* pConnection, AnalyzeXML* /*pIncoming*/, soarxml::ElementXML* pResponse)
{
	return this->ReturnResult(pConnection, pResponse, sml_Names::kSoarVersionValue ) ;
}

bool KernelSML::HandleIsSoarRunning(AgentSML* /*pAgentSML*/, char const* /*pCommandName*/, Connection* pConnection, AnalyzeXML* /*pIncoming*/, soarxml::ElementXML* pResponse)
{
	bool isRunning = this->GetRunScheduler()->IsRunning() ;

	return this->ReturnBoolResult(pConnection, pResponse, isRunning) ;
}

bool KernelSML::HandleGetAgentList(AgentSML* /*pAgentSML*/, char const* /*pCommandName*/, Connection* /*pConnection*/, AnalyzeXML* /*pIncoming*/, soarxml::ElementXML* pResponse)
{
	// Create the result tag
	TagResult* pTagResult = new TagResult() ;
	pTagResult->AddAttribute(sml_Names::kCommandOutput, sml_Names::kStructuredOutput) ;

	// Walk the list of agents and return their names
	for (AgentMapIter iter = m_AgentMap.begin() ; iter != m_AgentMap.end() ; iter++)
	{
		// Add a name tag to the output
		TagName* pTagName = new TagName() ;
		pTagName->SetName( iter->first.c_str() ) ;
		pTagResult->AddChild(pTagName) ;
	}

	// Add the result tag to the response
	pResponse->AddChild(pTagResult) ;

	// Return true to indicate we've filled in all of the result tag we need
	return true ;
}

// Controls the frequency of the smlEVENT_INTERRUPT_CHECK event
bool KernelSML::HandleSetInterruptCheckRate(AgentSML* /*pAgentSML*/, char const* /*pCommandName*/, Connection* /*pConnection*/, AnalyzeXML* pIncoming, soarxml::ElementXML* /*pResponse*/)
{
	// Get the parameters
	int newRate = pIncoming->GetArgInt(sml_Names::kParamValue, 1) ;

	// Make the call.
	m_InterruptCheckRate = newRate;

	return true ;
}

// Fire a particular event at the request of the client.
bool KernelSML::HandleFireEvent(AgentSML* /*pAgentSML*/, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, soarxml::ElementXML* pResponse)
{
	// Get the parameters
	char const* pEventName = pIncoming->GetArgString(sml_Names::kParamEventID) ;

	if (!pEventName)
	{
		return InvalidArg(pConnection, pResponse, pCommandName, "Event id is missing") ;
	}

	// Convert from the event name to the id value
	int id = ConvertStringToEvent(pEventName) ;

	// Make the call.  These are the only events which we allow
	// explicit client control over to date.
	if (id == smlEVENT_SYSTEM_START || id == smlEVENT_SYSTEM_STOP)
		this->FireSystemEvent(smlSystemEventId(id)) ;

	return true ;
}

bool KernelSML::HandleSendClientMessage(AgentSML* pAgentSML, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, soarxml::ElementXML* pResponse)
{
	// Get the parameters
	char const* pMessageType = pIncoming->GetArgString(sml_Names::kParamName) ;
	char const* pMessage     = pIncoming->GetArgString(sml_Names::kParamMessage) ;

	if (!pMessageType || !pMessage)
	{
		return InvalidArg(pConnection, pResponse, pCommandName, "Require a message type and a message and one is missing") ;
	}

	std::string result = this->SendClientMessage(pAgentSML, pMessageType, pMessage) ;

	return ReturnResult(pConnection, pResponse, result.c_str()) ;
}

// Prevent a particular event from firing when it next would normally do so
bool KernelSML::HandleSuppressEvent(AgentSML* /*pAgentSML*/, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, soarxml::ElementXML* pResponse)
{
	// Get the parameters
	char const* pEventName = pIncoming->GetArgString(sml_Names::kParamEventID) ;
	bool state = pIncoming->GetArgBool(sml_Names::kParamValue, true) ;

	if (!pEventName)
	{
		return InvalidArg(pConnection, pResponse, pCommandName, "Event id is missing") ;
	}

	// Convert from the event name to the id value
	int id = ConvertStringToEvent(pEventName) ;

	// Make the call.
	if (id == smlEVENT_SYSTEM_STOP)
	{
		SetSuppressSystemStop(state) ;
	}

	return true ;
}

// Check if anyone has sent us a command (e.g. over a socket from a remote debugger)
bool KernelSML::HandleCheckForIncomingCommands(AgentSML* /*pAgentSML*/, char const* /*pCommandName*/, Connection* pConnection, AnalyzeXML* /*pIncoming*/, soarxml::ElementXML* pResponse)
{
	// We let the caller know if we read at least one message
	bool receivedOneMessage = false ;

	// Also check for any incoming calls from remote sockets
	if (m_pConnectionManager)
		receivedOneMessage = m_pConnectionManager->ReceiveAllMessages() ;

	return this->ReturnBoolResult(pConnection, pResponse, receivedOneMessage) ;
}

bool KernelSML::HandleGetInputLink(AgentSML* pAgentSML, char const* /*pCommandName*/, Connection* pConnection, AnalyzeXML* /*pIncoming*/, soarxml::ElementXML* pResponse)
{
	if (!pAgentSML)
		return false ;

	Symbol* sym = pAgentSML->GetSoarAgent()->io_header_input;

	// Turn the id symbol into an actual string
	char buf[ MAX_LEXEME_LENGTH ];
	char * id = symbol_to_string ( pAgentSML->GetSoarAgent(), sym, true, buf, MAX_LEXEME_LENGTH );
	
	if (id)
	{
		// FIXME: this doesn't work
		//id[0] = static_cast<char>(tolower( id[0] )); // sending client side id

		// Fill in the id string as the result of this command
		this->ReturnResult(pConnection, pResponse, id) ;
	}

	// We succeeded if we got an id string
	return (id != NULL) ;
}

static bool AddWmeChildrenToXML( AgentSML* pAgentSML, wme* pRoot, soarxml::ElementXML* pTagResult, std::list< wme* >& traversedList )
{
	if (!pRoot || !pTagResult)
		return false ;

    for (wme* w = pRoot->value->id.input_wmes; w != NIL; w = w->next)
	{
		TagWme* pTagWme = OutputListener::CreateTagWme( pAgentSML, w ) ;

#ifdef _DEBUG
		// Set a break point in here to look at the message as a string
		char *pStr = pTagWme->GenerateXMLString(true) ;
		pTagWme->DeleteString(pStr) ;
#endif

		// Add this wme into the result
		pTagResult->AddChild(pTagWme) ;

		// If this is an identifier then add all of its children too
		if ( w->value->sc.common_symbol_info.symbol_type == IDENTIFIER_SYMBOL_TYPE )
		{	
			if ( std::find( traversedList.begin(), traversedList.end(), w ) == traversedList.end() )
			{
				traversedList.push_back( w );
				AddWmeChildrenToXML( pAgentSML, w, pTagResult, traversedList );
			}
		}
	}

	return true ;
}

// Send the current state of the input link back to the caller.  (This is not a commonly used method).
bool KernelSML::HandleGetAllInput(AgentSML* pAgentSML, char const* /*pCommandName*/, Connection* /*pConnection*/, AnalyzeXML* /*pIncoming*/, soarxml::ElementXML* pResponse)
{
	// Create the result tag
	TagResult* pTagResult = new TagResult() ;

	agent* pSoarAgent = pAgentSML->GetSoarAgent() ;

	// Find the input link wme I1 ^input-link I2
	wme* pInputLinkWme = 0;
    for (wme* w = pSoarAgent->io_header->id.input_wmes; w != NIL; w = w->next)
	{
		if ( w->attr == pSoarAgent->input_link_symbol )
		{
			pInputLinkWme = w;
			break;
		}
	}
	assert( pInputLinkWme );
	if( !pInputLinkWme )
	{
		return false;
	}

	std::list< wme* > traversedList ;

	AddWmeChildrenToXML( pAgentSML, pInputLinkWme, pTagResult, traversedList ) ;

	// Add the message to the response
	pResponse->AddChild(pTagResult) ;

#ifdef _DEBUG
	// Set a break point in here to look at the message as a string
	char *pStr = pResponse->GenerateXMLString(true) ;
	pResponse->DeleteString(pStr) ;
#endif

	// Return true to indicate we've filled in all of the result tag we need
	return true ;
}

// Send the current state of the output link back to the caller.  (This is not a commonly used method).
bool KernelSML::HandleGetAllOutput(AgentSML* pAgentSML, char const* /*pCommandName*/, Connection* /*pConnection*/, AnalyzeXML* /*pIncoming*/, soarxml::ElementXML* pResponse)
{
	// Build the SML message we're doing to send which in this case is an output command
	// (just like one you'd get if the agent was generating output rather than being queried for its output link)
	TagCommand* pTagResult = new TagCommand() ;
	pTagResult->SetName(sml_Names::kCommand_Output) ;

	agent* pSoarAgent = pAgentSML->GetSoarAgent() ;

	output_link *ol = pSoarAgent->existing_output_links ;	// This is technically a list but we only support one output link

	//remove_output_link_tc_info (pSoarAgent, ol);
	//calculate_output_link_tc_info (pSoarAgent, ol);
	io_wme* iw_list = get_io_wmes_for_output_link (pSoarAgent, ol);

	// Start with the output link itself
	TagWme* pOutputLinkWme = OutputListener::CreateTagWme( pAgentSML, ol->link_wme ) ;
	pTagResult->AddChild(pOutputLinkWme) ;

	for (;iw_list != 0 ; iw_list = iw_list->next) {
		// Create the wme tag for the output link itself
		TagWme* pTagWme = OutputListener::CreateTagIOWme( pAgentSML, iw_list ) ;

		// Add this wme into the result
		pTagResult->AddChild(pTagWme) ;
	}

	deallocate_io_wme_list(pSoarAgent, iw_list) ;

	// Add the message to the response
	pResponse->AddChild(pTagResult) ;

#ifdef _DEBUG
	// Set a break point in here to look at the message as a string
	char *pStr = pResponse->GenerateXMLString(true) ;
	pResponse->DeleteString(pStr) ;
#endif

	// Return true to indicate we've filled in all of the result tag we need
	return true ;
}

// Add or remove a list of wmes we've been sent
bool KernelSML::HandleInput(AgentSML* pAgentSML, char const* /*pCommandName*/, Connection* /*pConnection*/, AnalyzeXML* pIncoming, soarxml::ElementXML* /*pResponse*/)
{
	// Flag to control printing debug information about the input link
#ifdef _DEBUG
	bool kDebugInput = false ;
#else
	bool kDebugInput = false ;
#endif

	if (!pAgentSML)
		return false ;

	// Record the input coming input message on a list
	pAgentSML->AddToPendingInputList(pIncoming->GetElementXMLHandle()) ;

	bool ok = true ;

	// Get the command tag which contains the list of wmes
	soarxml::ElementXML const* pCommand = pIncoming->GetCommandTag() ;

	// Echo back the list of wmes received, so other clients can see what's been added (rarely used).
	pAgentSML->FireInputReceivedEvent(pCommand) ;

	if (kDebugInput)
		sml::PrintDebugFormat("--------- %s ending input ----------", pAgentSML->GetName()) ;

	// Returns false if any of the adds/removes fails
	return ok ;
}

// Executes a generic command line for a specific agent
bool KernelSML::HandleCommandLine(AgentSML* pAgentSML, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, soarxml::ElementXML* pResponse)
{
#ifdef _DEBUG
	bool kDebugCommandLine = false ;
#else
	bool kDebugCommandLine = false ;
#endif

	// Get the parameters
	char const* pLine = pIncoming->GetArgString(sml_Names::kParamLine) ;
	bool echoResults  = pIncoming->GetArgBool(sml_Names::kParamEcho, false) ;
	bool noFiltering  = pIncoming->GetArgBool(sml_Names::kParamNoFiltering, false) ;

	// If the user chooses to enable this feature, certain commands are always echoed back.
	// This is primarily to support two users connected to and debugging the same kernel at once.
	if (GetEchoCommands() && m_CommandLineInterface.ShouldEchoCommand(pLine))
		echoResults = true ;

	bool rawOutput = false;

	// The caller can ask for simple string output (raw output) or more complex, structured XML output
	// which can then be parsed.
	soarxml::ElementXML const* pCommand = pIncoming->GetCommandTag() ;
	const char* pCommandOutput = pCommand->GetAttribute(sml_Names::kCommandOutput) ;

	if (pCommandOutput)
		rawOutput = (strcmp(pCommandOutput, sml_Names::kRawOutput) == 0) ;

	if (!pLine)
	{
		return InvalidArg(pConnection, pResponse, pCommandName, "Command line missing") ;
	}

	if (kDebugCommandLine)
		sml::PrintDebugFormat("Executing %s", pLine) ;
	
	// If we're echoing the results, also echo the command we're executing
	if (echoResults && pAgentSML)
		pAgentSML->FireEchoEvent(pConnection, pLine) ;

	if (kDebugCommandLine)
		sml::PrintDebugFormat("Echoed line\n") ;

	// Send this command line through anyone registered filters.
	// If there are no filters (or this command requested not to be filtered), this copies the original line into the filtered line unchanged.
	char const* pFilteredLine   = pLine ;
	bool filteredError = false ;
	soarxml::ElementXML* pFilteredXML = NULL ;

	if (!noFiltering && HasFilterRegistered())
	{
		// Expand any aliases before passing the command to the filter.
		// It's possible this is a mistake because a really powerful filter might want to do
		// something with the original, unaliased form (and could call this expansion itself) but
		// it seems this will be correct in almost all cases, so let's start with this assumption and
		// wait until it's proved incorrect.
		std::string expandedLine ;
		if (m_CommandLineInterface.ExpandCommandToString(pLine, &expandedLine))
			pLine = expandedLine.c_str() ;

		// We'll send the command over as an XML packet, so there's some structure to work with.
		// The current structure is:
		// <filter command="command" output="generated output" error="true | false"></filter>
		// Each filter is passed this string and can modify it as they go.
		// All attributes are optional although either command or output & error should exist.
		// It's possible, although unlikely that all 3 could exist at once (i.e. another command to execute, yet still have output already)
		TagFilter filterXML ;
		filterXML.SetCommand(pLine) ;

		char* pXMLString = filterXML.GenerateXMLString(true) ;

		std::string filteredXML ;
		bool filtered = this->SendFilterMessage(pAgentSML, pXMLString, &filteredXML) ;

		// Clean up the XML message
		filterXML.DeleteString(pXMLString) ;

		// If a filter consumed the entire command, there's no more work for us to do.
		if (filteredXML.empty())
			return true ;

		if (filtered)
		{
			pFilteredXML = soarxml::ElementXML::ParseXMLFromString(filteredXML.c_str()) ;
			if (!pFilteredXML)
			{
				// Error parsing the XML that the filter returned
				return false ;
			}

			// Get the results of the filtering
			pFilteredLine    = pFilteredXML->GetAttribute(sml_Names::kFilterCommand) ;
			char const* pFilteredOutput  = pFilteredXML->GetAttribute(sml_Names::kFilterOutput) ;
			char const* pErr = pFilteredXML->GetAttribute(sml_Names::kFilterError) ;
			filteredError    = (pErr && strcasecmp(pErr, "true") == 0) ;

			// See if the filter consumed the command.  If so, we just need to return the output.
			if (!pFilteredLine || strlen(pFilteredLine) == 0)
			{
				// We may have no output defined and that's not an error so cover that case
				if (pFilteredOutput == NULL)
					pFilteredOutput = "" ;

				bool res = this->ReturnResult(pConnection, pResponse, pFilteredOutput) ;

				// Can only clean this up after we're finished using it or pFilteredLine will become invalid
				delete pFilteredXML ;

				return res ;
			}
		}
	}

	if (kDebugCommandLine)
		sml::PrintDebugFormat("Filtered line is %s\n", pFilteredLine) ;

	// Make the call.
	//std::cout << std::endl << "handling: " << pFilteredLine;
	bool result = m_CommandLineInterface.DoCommand(pConnection, pAgentSML, pFilteredLine, echoResults, rawOutput, pResponse) ;

	if (kDebugCommandLine)
		sml::PrintDebugFormat("Completed %s", pLine) ;

	// Can only clean this up after we're finished using it or pFilteredLine will become invalid
	delete pFilteredXML ;

	return result ;
}

// Expands a command line's aliases and returns it without executing it.
bool KernelSML::HandleExpandCommandLine(AgentSML* /*pAgentSML*/, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, soarxml::ElementXML* pResponse)
{
	// Get the parameters
	char const* pLine = pIncoming->GetArgString(sml_Names::kParamLine) ;

	if (!pLine)
	{
		return InvalidArg(pConnection, pResponse, pCommandName, "Command line missing") ;
	}

	// Make the call.
	return m_CommandLineInterface.ExpandCommand(pConnection, pLine, pResponse) ;
}

bool KernelSML::HandleGetListenerPort(AgentSML* /*pAgentSML*/, char const* /*pCommandName*/, Connection* pConnection, AnalyzeXML* /*pIncoming*/, soarxml::ElementXML* pResponse)
{
	return this->ReturnIntResult(pConnection, pResponse, this->GetListenerPort());
}

bool KernelSML::HandleGetLibraryLocation(AgentSML* /*pAgentSML*/, char const* /*pCommandName*/, Connection* pConnection, AnalyzeXML* /*pIncoming*/, soarxml::ElementXML* pResponse)
{
	return this->ReturnResult(pConnection, pResponse, this->GetLibraryLocation() ) ;
}

