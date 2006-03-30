#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

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

#ifdef _WIN32
#include <crtdbg.h>
#endif

#include "sml_KernelSML.h"
#include "sml_AgentSML.h"
#include "sml_Connection.h"
#include "sml_StringOps.h"
#include "sml_OutputListener.h"
#include "sml_ConnectionManager.h"
#include "sml_TagResult.h"
#include "sml_TagName.h"
#include "sml_TagWme.h"
#include "sml_TagCommand.h"
#include "sml_ClientEvents.h"
#include "sml_Events.h"
#include "sml_RunScheduler.h"

#include "sock_Debug.h"	// For PrintDebugFormat

#include "gSKI.h"
#include <iostream>
#include <fstream>
#include <map>
#include <stdlib.h>
#include <assert.h>

#include "IgSKI_KernelFactory.h"
#include "gSKI_Stub.h"
#include "IgSKI_Kernel.h"

// BADBAD: I think we should be using an error class instead to work with error objects.
#include "../../gSKI/src/gSKI_Error.h"
#include "gSKI_ErrorIds.h"
#include "gSKI_Enumerations.h"
#include "gSKI_Events.h"
#include "IgSKI_AgentManager.h"
#include "IgSKI_Agent.h"
#include "IgSKI_ProductionManager.h"
#include "IgSKI_OutputProcessor.h"
#include "IgSKI_OutputLink.h"
#include "IgSKI_InputProducer.h"
#include "IgSKI_Symbol.h"
#include "IgSKI_Wme.h"

using namespace sml ;
using namespace gSKI ;

// BADBAD: Not sure where this macro is coming from but I saw this
// in IgSKI_Symbol.h and it's needed for the GetObject() calls to compile.
#ifdef _WIN32
#undef GetObject
#undef SendMessage
#endif

/*
static void DebugPrint(char const* pFilename, int line, char const* pMsg)
{
#ifdef _WIN32
#ifdef _DEBUG
	_CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG );
	_CrtSetReportFile( _CRT_WARN, _CRTDBG_FILE_STDOUT );

	_CrtDbgReport(_CRT_WARN, pFilename, line, "KernelSML", pMsg);
#endif
#endif
}
*/

/*
  ==================================
  SML input producer
  ==================================
*/
class sml_InputProducer: public IInputProducer
{
public:

   // Simple constructor
   sml_InputProducer(KernelSML* pKernelSML)
   {
	   m_KernelSML	= pKernelSML ;
   }
   
   virtual ~sml_InputProducer() 
   {
   }
   
   // The update function required by the IInputProducer interface
   // (Responsible for updating the state of working memory)
   virtual void Update(IWorkingMemory* wmemory,
                       IWMObject* obj)
   {
	   unused(wmemory) ;
	   unused(obj) ;

	   // Check for any new incoming commands from remote connections.
	   // We do this in an input producer so it's once per decision during a run and
	   // the input phase seems like the correct point for incoming commands.
	   m_KernelSML->ReceiveAllMessages() ;
   }

private:
	sml::KernelSML*		m_KernelSML ;
};

void KernelSML::BuildCommandMap()
{
	m_CommandMap[sml_Names::kCommand_CreateAgent]		= &sml::KernelSML::HandleCreateAgent ;
	m_CommandMap[sml_Names::kCommand_DestroyAgent]		= &sml::KernelSML::HandleDestroyAgent ;
	m_CommandMap[sml_Names::kCommand_LoadProductions]	= &sml::KernelSML::HandleLoadProductions ;
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
}

/*************************************************************
* @brief	A command handler (SML message->appropriate gSKI handling).
*
* @param pAgent			The agent this command is for (can be NULL if the command is not agent specific)
* @param pCommandName	The SML command name (so one handler can handle many incoming calls if we wish)
* @param pConnection	The connection this command came in on
* @param pIncoming		The incoming, analyzed message.
* @param pResponse		The partially formed response.  This handler needs to fill in more of this.
* @param pError			A gSKI error object, which gSKI will fill in if there are errors.
* @returns False if we had an error and wish to generate a generic error message (based on the incoming call + pError)
*          True if the call succeeded or we generated another more specific error already.
*************************************************************/
bool KernelSML::HandleCreateAgent(gSKI::IAgent* pAgentPtr, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError)
{
	unused(pCommandName) ; unused(pAgentPtr) ; unused(pResponse) ;

	// Get the parameters
	char const* pName = pIncoming->GetArgString(sml_Names::kParamName) ;

	if (!pName)
	{
		return InvalidArg(pConnection, pResponse, pCommandName, "Agent name missing") ;
	}

	// Make the call.
	IAgent* pAgent = GetKernel()->GetAgentManager()->AddAgent(pName, NULL, false, gSKI_O_SUPPORT_MODE_4, pError) ;

	// Register for output from this agent
	if (pAgent)
	{
		// We store additional, agent specific information required for SML in the AgentSML object.
		// NOTE: This call to GetAgentSML() will create the object if necessary...which it will be in this case.
		AgentSML* pAgentSML = GetAgentSML(pAgent) ;

		// Mark the agent's status as just having been created for all connections
		// Note--agent status for connections just refers to the last agent created, i.e. this one.
		m_pConnectionManager->SetAgentStatus(sml_Names::kStatusCreated) ;

		// We also need to listen to input events so we can pump waiting sockets and get interrupt messages etc.
		sml_InputProducer* pInputProducer = new sml_InputProducer(this) ;
		pAgentSML->SetInputProducer(pInputProducer) ;

		// Add the input producer to the top level of the input link (doesn't matter for us which WME it's attached to)
		IInputLink* pInputLink = pAgent->GetInputLink() ;
		IWMObject* pRoot = NULL ;
		pInputLink->GetRootObject(&pRoot) ;
		pAgent->GetInputLink()->AddInputProducer(pRoot, pInputProducer) ;
		pRoot->Release() ;
	}

	// Return true if we got an agent constructed.
	// If not, pError should contain the error and returning false
	// means we'll pass that back to the caller.
	return (pAgent != NULL) ;
}

// Handle registering and unregistering for kernel events
bool KernelSML::HandleRegisterForEvent(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError)
{
	unused(pError) ;

	// Decide if registering or unregistering
	bool registerForEvent = (strcmp(pCommandName, sml_Names::kCommand_RegisterForEvent) == 0) ;

	// The value sent over is actually defined in sml_ClientEvents.h but we're just casting it over to egSKIEventId.
	// So let's add some checks here to make sure that the two tables are synchronized.
	// (If we wish we could introduce a mapping here between the two sets of ids but for now we're not doing that).
	assert(gSKIEVENT_INVALID_EVENT == (egSKIGenericEventId)smlEVENT_INVALID_EVENT) ;	// First matches
	assert(gSKIEVENT_AFTER_RUNNING == (egSKIRunEventId)smlEVENT_AFTER_RUNNING) ;	// Random one in middle matches
	assert(gSKIEVENT_BEFORE_AGENT_REINITIALIZED == (egSKIAgentEventId)smlEVENT_BEFORE_AGENT_REINITIALIZED) ;	// Another middle one matches
	assert(gSKIEVENT_PRINT == (egSKIPrintEventId)smlEVENT_PRINT); // What the heck, another one
	assert(gSKIEVENT_RHS_USER_FUNCTION == (egSKIRhsEventId)smlEVENT_RHS_USER_FUNCTION) ;
	assert(gSKIEVENT_LAST== (egSKIGenericEventId)smlEVENT_LAST) ;

	// Get the parameters
	char const* pEventName = pIncoming->GetArgString(sml_Names::kParamEventID) ;

	if (!pEventName)
	{
		return InvalidArg(pConnection, pResponse, pCommandName, "Event id is missing") ;
	}

	// Convert from the event name to the id value
	int id = ConvertStringToEvent(pEventName) ;

	// Decide what type of event this is and where to register/unregister it
	// gSKI uses a different class for each type of event.  We collect those together
	// where possible to reduce the amount of extra scaffolding code.
	if(IsSystemEventID(id))
	{
		// System Events
		if (registerForEvent)
			this->AddSystemListener((egSKISystemEventId)id, pConnection) ;
		else
			this->RemoveSystemListener((egSKISystemEventId)id, pConnection) ;

	} else if(IsAgentEventID(id)) {

		// Agent events
		if (registerForEvent)
			this->AddAgentListener((egSKIAgentEventId)id, pConnection) ;
		else
			this->RemoveAgentListener((egSKIAgentEventId)id, pConnection) ;
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
		if (!pAgent)
			return InvalidArg(pConnection, pResponse, pCommandName, "No agent name for an event that is handled by an agent") ;

		// Register or unregister for this event
		AgentSML* pAgentSML = GetAgentSML(pAgent) ;

		if (registerForEvent)
			pAgentSML->AddRunListener((egSKIRunEventId)id, pConnection) ;
		else
			pAgentSML->RemoveRunListener((egSKIRunEventId)id, pConnection) ;
	} else if(IsProductionEventID(id)) {

		// Production event
		if (!pAgent)
			return InvalidArg(pConnection, pResponse, pCommandName, "No agent name for an event that is handled by an agent") ;

		// Register or unregister for this event
		AgentSML* pAgentSML = GetAgentSML(pAgent) ;

		if (registerForEvent)
			pAgentSML->AddProductionListener((egSKIProductionEventId)id, pConnection) ;
		else
			pAgentSML->RemoveProductionListener((egSKIProductionEventId)id, pConnection) ;
	} else if(IsXMLEventID(id)) {

		// XML Event
		if (!pAgent)
			return InvalidArg(pConnection, pResponse, pCommandName, "No agent name for an event that is handled by an agent") ;

		// Register or unregister for this event
		AgentSML* pAgentSML = GetAgentSML(pAgent) ;

		if (registerForEvent)
			pAgentSML->AddXMLListener((egSKIXMLEventId)id, pConnection) ;
		else
			pAgentSML->RemoveXMLListener((egSKIXMLEventId)id, pConnection) ;

	} else if (IsUpdateEventID(id))	{
		if (registerForEvent)
			AddUpdateListener((egSKIUpdateEventId)id, pConnection) ;
		else
			RemoveUpdateListener((egSKIUpdateEventId)id, pConnection) ;
	} else if (IsStringEventID(id))	{
		if (registerForEvent)
			AddStringListener((egSKIStringEventId)id, pConnection) ;
		else
			RemoveStringListener((egSKIStringEventId)id, pConnection) ;
	}
	else if(IsPrintEventID(id)) {

		// Print event
		if (!pAgent)
			return InvalidArg(pConnection, pResponse, pCommandName, "No agent name for an event that is handled by an agent") ;

		// Register or unregister for this event
		AgentSML* pAgentSML = GetAgentSML(pAgent) ;

		if (registerForEvent)
			pAgentSML->AddPrintListener((egSKIPrintEventId)id, pConnection) ;
		else
			pAgentSML->RemovePrintListener((egSKIPrintEventId)id, pConnection) ;
	} else if(id == (int)gSKIEVENT_OUTPUT_PHASE_CALLBACK) {

		// Output event
		AgentSML* pAgentSML = GetAgentSML(pAgent) ;
		OutputListener* pOutputListener = pAgentSML->GetOutputListener() ;

		// Register this connection as listening for this event
		if (registerForEvent)
			pOutputListener->AddListener(gSKIEVENT_OUTPUT_PHASE_CALLBACK, pConnection) ;
		else
			pOutputListener->RemoveListener(gSKIEVENT_OUTPUT_PHASE_CALLBACK, pConnection) ;
	} else {
		// The event didn't match any of our handlers
		return InvalidArg(pConnection, pResponse, pCommandName, "KernelSML doesn't know how to handle that event id") ;
	}

	return true ;
}

bool KernelSML::HandleSetConnectionInfo(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError)
{
	unused(pAgent) ; unused(pCommandName) ; unused(pError) ;

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

bool KernelSML::HandleGetConnections(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pCallingConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError)
{
	unused(pAgent) ; unused(pCommandName) ; unused(pIncoming) ; unused(pCallingConnection) ; unused(pError) ;

	// Create the result tag
	TagResult* pTagResult = new TagResult() ;
	pTagResult->AddAttribute(sml_Names::kCommandOutput, sml_Names::kStructuredOutput) ;

	// Walk the list of connections and return their info
	int index = 0 ;
	Connection* pConnection = m_pConnectionManager->GetConnectionByIndex(index) ;

	while (pConnection)
	{
		// Create the connection tag
		ElementXML* pTagConnection = new ElementXML() ;
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

bool KernelSML::HandleDestroyAgent(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError)
{
	unused(pCommandName) ; unused(pResponse) ; unused(pConnection) ; unused(pIncoming) ;

	if (!pAgent)
		return false ;

	// gSKI's RemoveAgent call isn't guaranteed to complete immediately.
	// Instead it's a request (if the agent is running we have to stop it first and wait for that to happen
	// before the delete it honored).  So we register for this notification that the agent is actually about
	// to be deleted and then we release the data we have on this agent.
	// It's also important that we register this listener now so that it's added to the *end* of the list of listeners.
	// That's required as we want to send out calls to our clients for this before_agent_destroyed event and then
	// clean up our agent information.  This will only work if our clean up comes last, which it will be because
	// we're adding it immediately prior to the notification (although if the listener implementation is changed to not use push_back
	// this will break).
	GetAgentSML(pAgent)->RegisterForBeforeAgentDestroyedEvent() ;

	// Make the call to actually delete the agent
	// This will trigger a call to our m_pBeforeDestroyedListener
	GetKernel()->GetAgentManager()->RemoveAgent(pAgent, pError) ;

	return true ;
}

class KernelSML::OnSystemStopDeleteAll: public gSKI::ISystemListener
{
public:
	// This handler is called right before the agent is actually deleted
	// inside gSKI.  We need to clean up any object we own now.
	virtual void HandleEvent(egSKISystemEventId, gSKI::IKernel* pKernel)
	{
		unused(pKernel) ;

		KernelSML* pKernelSML = KernelSML::GetKernelSML() ;

		pKernelSML->DeleteAllAgents(true) ;

		delete this ;
	}
};

// Shutdown is an irrevocal request to delete all agents and prepare for kernel deletion.
bool KernelSML::HandleShutdown(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError)
{
	unused(pAgent) ; unused(pCommandName) ; unused(pConnection) ; unused(pIncoming) ; unused(pResponse) ; unused(pError) ;

	// Notify everyone that the system is about to shutdown.
	// Note -- this event is not currently implemented in gSKI and we would
	// prefer to send it now before the gSKI kernel object or any others are actually deleted
	// so that there's still a stable system to respond to the event.
	FireSystemEvent(gSKIEVENT_BEFORE_SHUTDOWN) ;

	// If we're actively running, stop everyone first and then delete.
	/* DJP: This is experimental code -- yet to be tested.  I think we may need to handle this in the client (in another thread)
	if (this->GetRunScheduler()->IsRunning())
	{
		// Request the agents to stop.  Only delete them once they actually have.
		m_pSystemStopListener = new OnSystemStopDeleteAll() ;
		GetKernel()->AddSystemListener(gSKIEVENT_SYSTEM_STOP, m_pSystemStopListener) ;
		GetKernel()->GetAgentManager()->InterruptAll(gSKI_STOP_AFTER_SMALLEST_STEP, pError) ;

		return false ;
	}
	*/

	// Delete all agents explicitly now (so listeners can hear that the agents have been destroyed).
	DeleteAllAgents(true) ;

	return true ;
}

// Return information about the current runtime state of the agent (e.g. phase, decision cycle count etc.)
bool KernelSML::HandleGetRunState(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError)
{
	unused(pCommandName) ;

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
		buffer << pAgent->GetCurrentPhase(pError);
	}
	else if (strcmp(pValue, sml_Names::kParamDecision) == 0)
	{
		// Report the current decision cycle counter (the counter reports one more than the user expects)
		buffer << (pAgent->GetNumDecisionCyclesExecuted(pError)-1);
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
bool KernelSML::HandleWasAgentOnRunList(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError)
{
	unused(pCommandName) ; unused(pIncoming) ; unused(pError) ;

	bool wasRun = GetAgentSML(pAgent)->WasAgentOnRunList() ;
	return this->ReturnBoolResult(pConnection, pResponse, wasRun) ;
}

// Return information about the current runtime state of the agent (e.g. phase, decision cycle count etc.)
bool KernelSML::HandleGetResultOfLastRun(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError)
{
	unused(pCommandName) ; unused(pIncoming) ; unused(pError) ;

	egSKIRunResult runResult = GetAgentSML(pAgent)->GetResultOfLastRun() ;
	return this->ReturnIntResult(pConnection, pResponse, runResult) ;
}

// Returns true if the production name is currently loaded
bool KernelSML::HandleIsProductionLoaded(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError)
{
	unused(pCommandName) ; unused(pIncoming) ;

	// Look up the name of the production
	char const* pName = pIncoming->GetArgString(sml_Names::kParamName) ;

	if (!pName)
	{
		return InvalidArg(pConnection, pResponse, pCommandName, "Need to specify the production name to check.") ;
	}

	tIProductionIterator* prodIter = pAgent->GetProductionManager()->GetProduction(pName, false, pError) ;
	bool found = prodIter->GetNumElements() > 0 ;
	prodIter->Release() ;

	return ReturnBoolResult(pConnection, pResponse, found) ;
}

bool KernelSML::HandleGetVersion(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError)
{
	unused(pAgent) ; unused(pCommandName) ;	unused(pIncoming) ;

	std::ostringstream buffer;

	// Look up the current version of Soar and return it as a string
	gSKI::Version version = this->m_pKernelFactory->GetKernelVersion(pError) ;
	buffer << version.major << "." << version.minor << "." << version.micro;
	std::string bufferStdString = buffer.str();
	const char* bufferCString = bufferStdString.c_str();

	// Our hard-coded string should match the version returned from Soar
	assert(strcmp(sml_Names::kSoarVersionValue, bufferCString) == 0) ;

	return this->ReturnResult(pConnection, pResponse, bufferCString) ;
}

bool KernelSML::HandleIsSoarRunning(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError)
{
	unused(pAgent) ; unused(pCommandName) ; unused(pIncoming) ; unused(pError) ;

	bool isRunning = this->GetRunScheduler()->IsRunning() ;

	return this->ReturnBoolResult(pConnection, pResponse, isRunning) ;
}

bool KernelSML::HandleGetAgentList(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError)
{
	unused(pAgent) ; unused(pCommandName) ; unused(pConnection) ; unused(pIncoming) ;

	// Make the call.
	IAgentManager* pManager = GetKernel()->GetAgentManager() ;

	// Get the list of agents
	tIAgentIterator* iter = pManager->GetAgentIterator(pError) ;

	if (!iter)
		return false ;

	// Create the result tag
	TagResult* pTagResult = new TagResult() ;
	pTagResult->AddAttribute(sml_Names::kCommandOutput, sml_Names::kStructuredOutput) ;

	// Walk the list of agents and return their names
	while (iter->IsValid())
	{
		IAgent* pAgent = iter->GetVal() ;

		// Add a name tag to the output
		TagName* pTagName = new TagName() ;
		pTagName->SetName(pAgent->GetName()) ;
		pTagResult->AddChild(pTagName) ;

		// Apparently, we don't release agent objects return from an agent iterator.  There is no such method.
		//pAgent->Release() ;

		iter->Next() ;
	}

	iter->Release() ;

	// Add the result tag to the response
	pResponse->AddChild(pTagResult) ;

	// Return true to indicate we've filled in all of the result tag we need
	return true ;
}

// Controls the frequency of the smlEVENT_INTERRUPT_CHECK event
bool KernelSML::HandleSetInterruptCheckRate(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError)
{
	unused(pCommandName) ; unused(pResponse) ; unused(pConnection) ; unused(pError) ; unused(pAgent) ;

	// Get the parameters
	int newRate = pIncoming->GetArgInt(sml_Names::kParamValue, 1) ;

	// Make the call.
	GetKernel()->SetInterruptCheckRate(newRate) ;

	return true ;
}

// Fire a particular event at the request of the client.
bool KernelSML::HandleFireEvent(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError)
{
	unused(pError) ; unused(pAgent) ;

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
	if (id == smlEVENT_SYSTEM_START)
		GetKernel()->FireSystemStart() ;
	else if (id == smlEVENT_SYSTEM_STOP)
		GetKernel()->FireSystemStop() ;

	return true ;
}

bool KernelSML::HandleSendClientMessage(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError)
{
	unused(pError) ;

	// Get the parameters
	char const* pMessageType = pIncoming->GetArgString(sml_Names::kParamName) ;
	char const* pMessage     = pIncoming->GetArgString(sml_Names::kParamMessage) ;

	if (!pMessageType || !pMessage)
	{
		return InvalidArg(pConnection, pResponse, pCommandName, "Require a message type and a message and one is missing") ;
	}

	std::string result = this->SendClientMessage(pAgent, pMessageType, pMessage) ;

	return ReturnResult(pConnection, pResponse, result.c_str()) ;
}

// Prevent a particular event from firing when it next would normally do so
bool KernelSML::HandleSuppressEvent(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError)
{
	unused(pResponse) ; unused(pConnection) ; unused(pError) ; unused(pAgent) ;

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
bool KernelSML::HandleCheckForIncomingCommands(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError)
{
	unused(pAgent) ; unused(pCommandName) ; unused(pError) ; unused(pIncoming) ;

	// We let the caller know if we read at least one message
	bool receivedOneMessage = false ;

	// Also check for any incoming calls from remote sockets
	if (m_pConnectionManager)
		receivedOneMessage = m_pConnectionManager->ReceiveAllMessages() ;

	return this->ReturnBoolResult(pConnection, pResponse, receivedOneMessage) ;
}

bool KernelSML::HandleLoadProductions(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError)
{
	unused(pCommandName) ; unused(pResponse) ;

	if (!pAgent)
		return false ;

	// Get the parameters
	char const* pFilename = pIncoming->GetArgString(sml_Names::kParamFilename) ;

	if (!pFilename)
	{
		return InvalidArg(pConnection, pResponse, pCommandName, "Filename missing") ;
	}

	// Make the call.
	bool ok = pAgent->GetProductionManager()->LoadSoarFile(pFilename, pError) ;

	return ok ;
}

bool KernelSML::HandleGetInputLink(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError)
{
	unused(pCommandName) ; unused(pConnection) ; unused(pIncoming) ;

	if (!pAgent)
		return false ;

	// We want the input link's id
	// Start with the root object for the input link
	IWMObject* pRootObject = NULL ;
	pAgent->GetInputLink()->GetRootObject(&pRootObject, pError) ;

	if (pRootObject == NULL)
		return false ;

	// Get the symbol for the id of this object
	ISymbol const* pID = pRootObject->GetId(pError) ;

	if (pID == NULL)
	{
		pRootObject->Release() ;
		return false ;
	}

	// Turn the id symbol into an actual string
	char const* id = pID->GetString(pError) ;
	
	if (id)
	{
		// Fill in the id string as the result of this command
		this->ReturnResult(pConnection, pResponse, id) ;
	}

	// No need to release the pID
	// because it's returned as a const by GetId().
	// pID->Release() ;

	// Clean up
	pRootObject->Release() ;

	// We succeeded if we got an id string
	return (id != NULL) ;
}

bool KernelSML::AddInputWME(gSKI::IAgent* pAgent, char const* pID, char const* pAttribute, char const* pValue, char const* pType, char const* pTimeTag, gSKI::Error* pError)
{
	// We store additional information for SML in the AgentSML structure, so look that up.
	AgentSML* pAgentSML = GetAgentSML(pAgent) ;

	bool addingToInputLink = true ;
	IWorkingMemory* pInputWM = pAgent->GetInputLink()->GetInputLinkMemory(pError) ;

	// First get the object which will own this new wme
	// Because we build from the top down, this should always exist by the
	// time we wish to add structure beneath it.
	IWMObject* pParentObject = NULL ;
	pInputWM->GetObjectById(pID, &pParentObject) ;

	// Soar also allows the environment to modify elements on the output link.
	// This is a form of backdoor input, so we need to check on the output side too
	// if we don't find our parent on the input side.
	if (!pParentObject)
	{
		pInputWM = pAgent->GetOutputLink()->GetOutputMemory(pError) ;
		pInputWM->GetObjectById(pID, &pParentObject) ;
		addingToInputLink = false ;
	}

	// Failed to find the parent.
	if (!pParentObject)
		return false ;

	IWme* pWME = NULL ;

	if (IsStringEqual(sml_Names::kTypeString, pType))
	{
		// Add a WME with a string value
		pWME = pInputWM->AddWmeString(pParentObject, pAttribute, pValue, pError) ;
	}
	else if (IsStringEqual(sml_Names::kTypeID, pType))
	{
		// There are two cases here.  We're either adding a new identifier
		// or we're adding a new wme that has an existing identifier as it's value.

		// Convert the value (itself an identifier) from client to kernel
		std::string value ;
		pAgentSML->ConvertID(pValue, &value) ;

		// See if we can find an object with this id (if so we're not adding a new identifier)
		IWMObject* pLinkObject = NULL ;
		pInputWM->GetObjectById(value.c_str(), &pLinkObject) ;

		if (pLinkObject)
		{
			// Create a new wme with the same value as an existing wme
			pWME = pInputWM->AddWmeObjectLink(pParentObject, pAttribute, pLinkObject, pError) ;
			pLinkObject->Release() ;
		}
		else
		{
			// Add a WME with an identifier value
			pWME = pInputWM->AddWmeNewObject(pParentObject, pAttribute, pError) ;
			
			if (pWME)
			{
				// We need to record the id that the kernel assigned to this object and match it against the id the
				// client is using, so that in future we can map the client's id to the kernel's.
				char const* pKernelID = pWME->GetValue()->GetString() ;
				pAgentSML->RecordIDMapping(pValue, pKernelID) ;
			}
		}
	}
	else if (IsStringEqual(sml_Names::kTypeInt, pType))
	{
		// Add a WME with an int value
		int value = atoi(pValue) ;
		pWME = pInputWM->AddWmeInt(pParentObject, pAttribute, value, pError) ;
	}
	else if (IsStringEqual(sml_Names::kTypeDouble, pType))
	{
		// Add a WME with a float value
		double value = atof(pValue) ;
		pWME = pInputWM->AddWmeDouble(pParentObject, pAttribute, value, pError) ;
	}

	if (!pWME)
	{
		pParentObject->Release() ;
		return false ;
	}

	// Well here's a surprise.  The kernel doesn't support a direct lookup from timeTag to wme.
	// So we need to maintain our own map out here so we can find the WME's quickly for removal.
	// So where we had planned to map from client time tag to kernel time tag, we'll instead
	// map from client time tag to IWme*.
	// That means we need to be careful to delete the IWme* objects later.
	if (pWME)
		pAgentSML->RecordTimeTag(pTimeTag, pWME) ;

	// We'll release this when the table of time tags is eventually destroyed or
	// when the wme is deleted.
//	pWME->Release() ;

	pParentObject->Release() ;

	// If we have an error object, check that it hasn't been set by an earlier call.
	if (pError)
		return !gSKI::isError(*pError) ;

	return true ;
}

bool KernelSML::RemoveInputWME(gSKI::IAgent* pAgent, char const* pTimeTag, gSKI::Error* pError)
{
	// We store additional information for SML in the AgentSML structure, so look that up.
	AgentSML* pAgentSML = GetAgentSML(pAgent) ;

	IWorkingMemory* pInputWM = pAgent->GetInputLink()->GetInputLinkMemory(pError) ;

	// Get the wme that matches this time tag
	IWme* pWME = pAgentSML->ConvertTimeTag(pTimeTag) ;

	// Failed to find the wme--that shouldn't happen.
	if (!pWME)
		return false ;

	// If this is an identifier, need to remove it from the ID mapping table too.
	if (pWME->GetValue()->GetType() == gSKI_OBJECT)
	{
		// Get the kernel-side identifier
		std::string id = pWME->GetValue()->GetString() ;

		// Look up the wmobject for this id
		//IWMObject* pObject = NULL ;
		//pInputWM->GetObjectById(id.c_str(), &pObject, pError) ;

		//assert(!pError || !gSKI::isError(*pError)) ;
		//pInputWM->RemoveObject(pObject) ;

		// Remove it from the id mapping table
		pAgentSML->RemoveID(id.c_str()) ;
	}

	// Remove the wme from working memory
	pInputWM->RemoveWme(pWME, pError) ;

	// The semantics of calling RemoveWme have now been clarified
	// to where the wme is released internally (by gSKI) once the kernel wme
	// is actually removed, which is in the next input phase.
	// So we no longer need to explicitly release it here.
	//pWME->Release() ;

	// Remove the object from the time tag table because
	// we no longer own it.
	pAgentSML->RemoveTimeTag(pTimeTag) ;

	// If we have an error object, check that it hasn't been set by an earlier call.
	if (pError)
		return !gSKI::isError(*pError) ;

	return true ;
}

static char const* GetValueType(egSKISymbolType type)
{
	switch (type)
	{
	case gSKI_DOUBLE: return sml_Names::kTypeDouble ;
	case gSKI_INT:	  return sml_Names::kTypeInt ;
	case gSKI_STRING: return sml_Names::kTypeString ;
	case gSKI_OBJECT: return sml_Names::kTypeID ;
	default: return NULL ;
	}
}

static bool AddWmeChildrenToXML(gSKI::IWMObject* pRoot, ElementXML* pTagResult, std::list<IWMObject*> *pTraversedList)
{
	if (!pRoot || !pTagResult)
		return false ;

	gSKI::tIWmeIterator* iter = pRoot->GetWMEs() ;

	while (iter->IsValid())
	{
		gSKI::IWme* pWME = iter->GetVal() ;

		// In some cases, wmes either haven't been added yet or have already been removed from the kernel
		// but still exist in gSKI.  In both cases, we can't (naturally) get a correct time tag for the wme
		// so I think we should skip these wmes.  That's clearly correct if the wme has been removed, but I'm
		// less sure if it's in the process of getting added.
		if (pWME->HasBeenRemoved())
		{
			pWME->Release() ;
			iter->Next() ;

			continue ;
		}

		TagWme* pTagWme = new TagWme() ;

		// Sometimes gSKI's owning object links are null -- esp. on the output side so I'm adding
		// a workaround to use the root object's ID.
		if (pWME->GetOwningObject())
			pTagWme->SetIdentifier(pWME->GetOwningObject()->GetId()->GetString()) ;
		else
			pTagWme->SetIdentifier(pRoot->GetId()->GetString()) ;

		pTagWme->SetAttribute(pWME->GetAttribute()->GetString()) ;
		pTagWme->SetValue(pWME->GetValue()->GetString(), GetValueType(pWME->GetValue()->GetType())) ;
		pTagWme->SetTimeTag(pWME->GetTimeTag()) ;
		pTagWme->SetActionAdd() ;

		// Add this wme into the result
		pTagResult->AddChild(pTagWme) ;

		// If this is an identifier then add all of its children too
		if (pWME->GetValue()->GetType() == gSKI_OBJECT)
		{
			gSKI::IWMObject* pChild = pWME->GetValue()->GetObject() ;

			// Check that we haven't already added this identifier before
			// (there can be cycles).
			if (std::find(pTraversedList->begin(), pTraversedList->end(), pChild) == pTraversedList->end())
			{
				pTraversedList->push_back(pChild) ;
				AddWmeChildrenToXML(pChild, pTagResult, pTraversedList) ;
			}
		}

		pWME->Release() ;
		iter->Next() ;
	}

	iter->Release() ;

	return true ;
}

// Send the current state of the input link back to the caller.  (This is not a commonly used method).
bool KernelSML::HandleGetAllInput(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError)
{
	unused(pCommandName) ; unused(pIncoming) ; unused(pConnection) ;

	// Create the result tag
	TagResult* pTagResult = new TagResult() ;

	// Walk the list of wmes on the input link and send them over
	gSKI::IWMObject* pRootObject = NULL ;
	pAgent->GetInputLink()->GetRootObject(&pRootObject, pError) ;

	// We need to keep track of which identifiers we've already added
	// because this is a graph, so we may cycle back.
	std::list<IWMObject*> traversedList ;

	// Add this wme's children to XML
	AddWmeChildrenToXML(pRootObject, pTagResult, &traversedList) ;

	// Add the result tag to the response
	pResponse->AddChild(pTagResult) ;

	if (pRootObject)
		pRootObject->Release() ;

	// Return true to indicate we've filled in all of the result tag we need
	return true ;
}

// Send the current state of the output link back to the caller.  (This is not a commonly used method).
bool KernelSML::HandleGetAllOutput(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError)
{
	unused(pCommandName) ; unused(pIncoming) ; unused(pConnection) ;

	// Build the SML message we're doing to send which in this case is an output command
	// (just like one you'd get if the agent was generating output rather than being queried for its output link)
	TagCommand* pTagResult = new TagCommand() ;
	pTagResult->SetName(sml_Names::kCommand_Output) ;

	// Walk the list of wmes on the input link and send them over
	gSKI::IWMObject* pRootObject = NULL ;
	pAgent->GetOutputLink()->GetRootObject(&pRootObject, pError) ;

	// Create the wme tag for the output link itself
	TagWme* pTagWme = new TagWme() ;

	pTagWme->SetIdentifier("I1") ;	// I don't see how to get this value in gSKI (i.e. the value of ^io <io>) but we don't actually care what's passed here)
	pTagWme->SetAttribute(sml_Names::kOutputLinkName) ;	// Again, can't see how to ask gSKI for this
	pTagWme->SetValue(pRootObject->GetId()->GetString(), GetValueType(gSKI_OBJECT)) ;
	pTagWme->SetTimeTag(5) ;	// Again, no way to get the real value, but I don't think it matters.  In current version of Soar this is always 5.
	pTagWme->SetActionAdd() ;

	// Add this wme into the result
	pTagResult->AddChild(pTagWme) ;

	// We need to keep track of which identifiers we've already added
	// because this is a graph, so we may cycle back.
	std::list<IWMObject*> traversedList ;

	// Add this wme's children to XML
	AddWmeChildrenToXML(pRootObject, pTagResult, &traversedList) ;

	// Add the message to the response
	pResponse->AddChild(pTagResult) ;

#ifdef _DEBUG
	// Set a break point in here to look at the message as a string
	char *pStr = pResponse->GenerateXMLString(true) ;
	pResponse->DeleteString(pStr) ;
#endif

	if (pRootObject)
		pRootObject->Release() ;

	// Return true to indicate we've filled in all of the result tag we need
	return true ;
}

// Add or remove a list of wmes we've been sent
bool KernelSML::HandleInput(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError)
{
	unused(pCommandName) ; unused(pResponse) ; unused(pConnection) ;

	// Flag to control printing debug information about the input link
#ifdef _DEBUG
	bool kDebugInput = false ;
#else
	bool kDebugInput = false ;
#endif

	if (!pAgent)
		return false ;

	// Get the command tag which contains the list of wmes
	ElementXML const* pCommand = pIncoming->GetCommandTag() ;

	int nChildren = pCommand->GetNumberChildren() ;

	ElementXML wmeXML(NULL) ;
	ElementXML* pWmeXML = &wmeXML ;

	AgentSML* pAgentSML = GetAgentSML(pAgent) ;

	bool ok = true ;

	if (kDebugInput)
		PrintDebugFormat("--------- %s starting input ----------", pAgent->GetName()) ;

	for (int i = 0 ; i < nChildren ; i++)
	{
		pCommand->GetChild(&wmeXML, i) ;

		// Ignore tags that aren't wmes.
		if (!pWmeXML->IsTag(sml_Names::kTagWME))
			continue ;

		// Find out if this is an add or a remove
		char const* pAction = pWmeXML->GetAttribute(sml_Names::kWME_Action) ;

		if (!pAction)
			continue ;

		bool add = IsStringEqual(pAction, sml_Names::kValueAdd) ;
		bool remove = IsStringEqual(pAction, sml_Names::kValueRemove) ;

		if (add)
		{
			char const* pID			= pWmeXML->GetAttribute(sml_Names::kWME_Id) ;	// May be a client side id value (e.g. "o3" not "O3")
			char const* pAttribute  = pWmeXML->GetAttribute(sml_Names::kWME_Attribute) ;
			char const* pValue		= pWmeXML->GetAttribute(sml_Names::kWME_Value) ;
			char const* pType		= pWmeXML->GetAttribute(sml_Names::kWME_ValueType) ;	// Can be NULL (=> string)
			char const* pTimeTag	= pWmeXML->GetAttribute(sml_Names::kWME_TimeTag) ;	// May be a client side time tag (e.g. -3 not +3)

			// Set the default value
			if (!pType)
				pType = sml_Names::kTypeString ;

			// Check we got everything we need
			if (!pID || !pAttribute || !pValue || !pTimeTag)
				continue ;

			// Map the ID from client side to kernel side (if the id is already a kernel side id it's returned unchanged)
			std::string id ;
			pAgentSML->ConvertID(pID, &id) ;

			if (kDebugInput)
			{
				PrintDebugFormat("%s Add %s ^%s %s (type %s tag %s)", pAgent->GetName(), id.c_str(), pAttribute, pValue, pType, pTimeTag) ;
			}

			// Add the wme
			ok = AddInputWME(pAgent, id.c_str(), pAttribute, pValue, pType, pTimeTag, pError) && ok ;
		}
		else if (remove)
		{
			char const* pTimeTag = pWmeXML->GetAttribute(sml_Names::kWME_TimeTag) ;	// May be (will be?) a client side time tag (e.g. -3 not +3)

			if (kDebugInput)
			{
				PrintDebugFormat("%s Remove tag %s", pAgent->GetName(), pTimeTag) ;
			}

			// Remove the wme
			ok = RemoveInputWME(pAgent, pTimeTag, pError) && ok ;
		}
	}

	// Echo back the list of wmes received, so other clients can see what's been added (rarely used).
	pAgentSML->FireInputReceivedEvent(pCommand) ;

	if (kDebugInput)
		PrintDebugFormat("--------- %s ending input ----------", pAgent->GetName()) ;

	// Returns false if any of the adds/removes fails
	return ok ;
}

// Executes a generic command line for a specific agent
bool KernelSML::HandleCommandLine(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError)
{
	unused(pCommandName) ;
	unused(pError);

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
	ElementXML const* pCommand = pIncoming->GetCommandTag() ;
	const char* pCommandOutput = pCommand->GetAttribute(sml_Names::kCommandOutput) ;

	if (pCommandOutput)
		rawOutput = (strcmp(pCommandOutput, sml_Names::kRawOutput) == 0) ;

	if (!pLine)
	{
		return InvalidArg(pConnection, pResponse, pCommandName, "Command line missing") ;
	}

	if (kDebugCommandLine)
		PrintDebugFormat("Executing %s", pLine) ;

	// If we're echoing the results, also echo the command we're executing
	if (echoResults)
	{
		AgentSML* pAgentSML = this->GetAgentSML(pAgent) ;

		if (pAgentSML)
			pAgentSML->FireEchoEvent(pConnection, pLine) ;
	}

	// Send this command line through anyone registered filters.
	// If there are no filters (or this command requested not to be filtered), this copies the original line into the filtered line unchanged.
	std::string filteredLine ;

	if (!noFiltering)
	{
		this->SendFilterMessage(pAgent, pLine, &filteredLine) ;

		// If a filter consumed the entire command, there's no more work for us to do.
		// Doing this after the echo step, so the original command is still echo'd to the user.
		if (filteredLine.empty())
			return true ;
	}
	else
	{
		// Filtering is off for this call, so just use the original command line
		filteredLine = pLine ;
	}

	// Make the call.
	m_CommandLineInterface.SetRawOutput(rawOutput);
	bool result = m_CommandLineInterface.DoCommand(pConnection, pAgent, filteredLine.c_str(), echoResults, pResponse) ;

	if (kDebugCommandLine)
		PrintDebugFormat("Completed %s", pLine) ;

	return result ;
}

// Expands a command line's aliases and returns it without executing it.
bool KernelSML::HandleExpandCommandLine(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError)
{
	unused(pCommandName) ;
	unused(pAgent) ; 	// Agent should be NULL
	unused(pError);

	// Get the parameters
	char const* pLine = pIncoming->GetArgString(sml_Names::kParamLine) ;

	if (!pLine)
	{
		return InvalidArg(pConnection, pResponse, pCommandName, "Command line missing") ;
	}

	// Make the call.
	return m_CommandLineInterface.ExpandCommand(pConnection, pLine, pResponse) ;
}
