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
#include "sml_ClientEvents.h"
#include "sml_Events.h"

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
	   //m_KernelSML->ReceiveAllMessages() ;
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
	m_CommandMap[sml_Names::kCommand_StopOnOutput]		= &sml::KernelSML::HandleStopOnOutput ;
	m_CommandMap[sml_Names::kCommand_CommandLine]		= &sml::KernelSML::HandleCommandLine ;
	m_CommandMap[sml_Names::kCommand_ExpandCommandLine]	= &sml::KernelSML::HandleExpandCommandLine ;
	m_CommandMap[sml_Names::kCommand_CheckForIncomingCommands] = &sml::KernelSML::HandleCheckForIncomingCommands ;
	m_CommandMap[sml_Names::kCommand_GetAgentList]		= &sml::KernelSML::HandleGetAgentList ;
	m_CommandMap[sml_Names::kCommand_RegisterForEvent]	= &sml::KernelSML::HandleRegisterForEvent ;
	m_CommandMap[sml_Names::kCommand_UnregisterForEvent]= &sml::KernelSML::HandleRegisterForEvent ;	// Note -- both register and unregister go to same handler
	m_CommandMap[sml_Names::kCommand_FireEvent]			= &sml::KernelSML::HandleFireEvent ;
	m_CommandMap[sml_Names::kCommand_SuppressEvent]		= &sml::KernelSML::HandleSuppressEvent ;
	m_CommandMap[sml_Names::kCommand_SetInterruptCheckRate] = &sml::KernelSML::HandleSetInterruptCheckRate ;
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
	char const* pName = pIncoming->GetArgValue(sml_Names::kParamName) ;

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
	char const* pEventName = pIncoming->GetArgValue(sml_Names::kParamEventID) ;

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
		char const* pRhsFunctionName = pIncoming->GetArgValue(sml_Names::kParamName) ;

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
	} else if(IsPrintEventID(id)) {

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

// Set a flag so Soar will break when it next generates output.  This allows us to
// run for "n decisions" OR "until output" which we can't do with raw gSKI calls.
bool KernelSML::HandleStopOnOutput(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError)
{
	unused(pCommandName) ; unused(pResponse) ; unused(pConnection) ; unused(pError) ;

	// Get the parameters
	bool state = pIncoming->GetArgBool(sml_Names::kParamValue, true) ;

	// Make the call.
	bool ok = GetAgentSML(pAgent)->SetStopOnOutput(state) ;

	return ok ;
}

// Fire a particular event at the request of the client.
bool KernelSML::HandleFireEvent(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError)
{
	unused(pResponse) ; unused(pConnection) ; unused(pError) ; 

	// Get the parameters
	char const* pEventName = pIncoming->GetArgValue(sml_Names::kParamEventID) ;

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

// Prevent a particular event from firing when it next would normally do so
bool KernelSML::HandleSuppressEvent(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError)
{
	unused(pResponse) ; unused(pConnection) ; unused(pError) ; unused(pAgent) ;

	// Get the parameters
	char const* pEventName = pIncoming->GetArgValue(sml_Names::kParamEventID) ;
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
	char const* pFilename = pIncoming->GetArgValue(sml_Names::kParamFilename) ;

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
			GetAgentSML(pAgent)->ConvertID(pID, &id) ;

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

	// Get the parameters
	char const* pLine = pIncoming->GetArgValue(sml_Names::kParamLine) ;

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

	// Make the call.
	m_CommandLineInterface.SetRawOutput(rawOutput);
	return m_CommandLineInterface.DoCommand(pConnection, pAgent, pLine, pResponse) ;
}

// Expands a command line's aliases and returns it without executing it.
bool KernelSML::HandleExpandCommandLine(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError)
{
	unused(pCommandName) ;
	unused(pAgent) ; 	// Agent should be NULL
	unused(pError);

	// Get the parameters
	char const* pLine = pIncoming->GetArgValue(sml_Names::kParamLine) ;

	if (!pLine)
	{
		return InvalidArg(pConnection, pResponse, pCommandName, "Command line missing") ;
	}

	// Make the call.
	return m_CommandLineInterface.ExpandCommand(pConnection, pLine, pResponse) ;
}
