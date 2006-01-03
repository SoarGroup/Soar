#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

/////////////////////////////////////////////////////////////////
// KernelSML class file.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : August 2004
//
// This class represents the translation point between SML and gSKI.
//
// It will maintain the information needed to communicate with gSKI
// and send and receive messages to the client (a tool or simulation).
//
/////////////////////////////////////////////////////////////////

#include "sml_KernelSML.h"
#include "sml_AgentSML.h"
#include "sml_Connection.h"
#include "sml_StringOps.h"
#include "sml_OutputListener.h"
#include "sml_ConnectionManager.h"
#include "sml_Events.h"
#include "sml_RunScheduler.h"
#include "KernelSMLDirect.h"

#include "thread_Lock.h"
#include "thread_Thread.h"

#include "gSKI.h"
#include <iostream>
#include <fstream>
#include <map>
#include <stdlib.h>

#include "IgSKI_KernelFactory.h"
#include "gSKI_Stub.h"
#include "IgSKI_Kernel.h"
#include "../../gSKI/src/gSKI_Error.h"
#include "gSKI_ErrorIds.h"
#include "gSKI_Enumerations.h"
#include "gSKI_Events.h"
#include "IgSKI_AgentManager.h"
#include "IgSKI_Agent.h"
#include "IgSKI_ProductionManager.h"
#include "IgSKI_OutputProcessor.h"
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

// Singleton instance of the kernel object
KernelSML* KernelSML::s_pKernel = NULL ;

// On Windows this is set to the DLL's hModule handle.
void* KernelSML::s_hModule = NULL ;

static ElementXML* AddErrorMsg(Connection* pConnection, ElementXML* pResponse, char const* pErrorMsg, int errorCode = -1)
{
	pConnection->AddErrorToSMLResponse(pResponse, pErrorMsg, errorCode) ;
	return pResponse ;
}

/*************************************************************
* @brief	Creates the singleton kernel object.
*************************************************************/
KernelSML* KernelSML::CreateKernelSML(unsigned short portToListenOn)
{
	// Failed to create KernelSML because it already exists!
	// This is really a sign of a bug.
	if (s_pKernel != NULL)
		return s_pKernel ;

	s_pKernel = new KernelSML(portToListenOn) ;

	return s_pKernel ;
}

/*************************************************************
* @brief	Returns the singleton kernel object.
*************************************************************/
KernelSML* KernelSML::GetKernelSML()
{
	return s_pKernel ;
}

KernelSML::KernelSML(unsigned short portToListenOn)
{
	// Initalize the event map
	m_pEventMap = new Events() ;

	// Create a Kernel Factory
	m_pKernelFactory = gSKI_CreateKernelFactory();
   
	// Create the kernel instance
	m_pIKernel = m_pKernelFactory->Create();

	// Give the command line interface a reference to the kernel interface
	m_CommandLineInterface.SetKernel(m_pIKernel, m_pKernelFactory->GetKernelVersion(), this);

	// Create the map from command name to handler function
	BuildCommandMap() ; 

	// Start listening for incoming connections
	m_pConnectionManager = new ConnectionManager(portToListenOn) ;

	// Start the kernel listener listening for events from gSKI
	m_AgentListener.Init(this);
	m_RhsListener.Init(this);
	m_SystemListener.Init(this);
	m_UpdateListener.Init(this) ;
	m_StringListener.Init(this) ;

	// We'll use this to make sure only one connection is executing commands
	// in the kernel at a time.
	m_pKernelMutex = new soar_thread::Mutex() ;

	m_SuppressSystemStart = false ;
	m_SuppressSystemStop  = false ;
	m_RequireSystemStop   = false ;

	m_pRunScheduler = new RunScheduler(this) ;

	m_pSystemStopListener = NULL ;
	m_EchoCommands = false ;
}

/** Deletes all agents and optionally waits until this has actually happened (if the agent is running there may be a delay) */
void KernelSML::DeleteAllAgents(bool waitTillDeleted)
{
	// We don't want to walk the map.iterator() while we're deleting elements.
	// Instead we just "pop" elements out of the map and delete them.
	while (m_AgentMap.size() != 0)
	{
		AgentSML* pAgentSML = m_AgentMap.begin()->second ;
		size_t agentCount = m_AgentMap.size() ;

		// gSKI's RemoveAgent call isn't guaranteed to complete immediately.
		// Instead it's a request (if the agent is running we have to stop it first and wait for that to happen
		// before the delete it honored).  So we register for this notification that the agent is actually about
		// to be deleted and then we release the data we have on this agent.
		// It's also important that we register this listener now so that it's added to the *end* of the list of listeners.
		// That's required as we want to send out calls to our clients for this before_agent_destroyed event and then
		// clean up our agent information.  This will only work if our clean up comes last, which it will be because
		// we're adding it immediately prior to the notification (although if the listener implementation is changed to not use push_back
		// this will break).
		pAgentSML->RegisterForBeforeAgentDestroyedEvent() ;

		// Make the call to actually delete the agent
		// This will trigger a call to our m_pBeforeDestroyedListener
		GetKernel()->GetAgentManager()->RemoveAgent(pAgentSML->GetIAgent(), NULL) ;

		// Now wait for the agent to be deleted (if we were requested to do so)
		int maxTries = 100 ;	// Wait for a second then abort anyway
		while (waitTillDeleted && agentCount == m_AgentMap.size() && maxTries > 0)
		{
			soar_thread::Thread::SleepStatic(0, 10) ;
			maxTries-- ;
		}
	}
}

KernelSML::~KernelSML()
{
	// Shutdown the connection manager while all data is still valid.
	// (This should have already been done before we get to the destructor,
	//  but this is a safety valve).
	Shutdown() ;

	DeleteAllAgents(true) ;

	m_SystemListener.Clear();
	m_AgentListener.Clear();
	m_RhsListener.Clear();
	m_UpdateListener.Clear() ;
	m_StringListener.Clear() ;

	if (m_pKernelFactory && m_pIKernel)
		m_pKernelFactory->DestroyKernel(m_pIKernel);

	delete m_pKernelFactory ;

	delete m_pConnectionManager ;

	delete m_pKernelMutex ;

	delete m_pEventMap ;
}

/*************************************************************
* @brief	Shutdown any connections and sockets in preparation
*			for the kernel process exiting.
*************************************************************/
void KernelSML::Shutdown()
{
	m_pConnectionManager->Shutdown() ;
}

/*************************************************************
* @brief	Send this message out to any clients that are listening.
*			These messages are from one client to another--kernelSML is just
*			facilitating the message passing process without knowing/caring what is being passed.
*************************************************************/
std::string KernelSML::SendClientMessage(gSKI::IAgent* pAgent, char const* pMessageType, char const* pMessage)
{
	char response[10000] ;
	response[0] = 0 ;

	bool ok = m_RhsListener.HandleEvent(gSKIEVENT_CLIENT_MESSAGE, pAgent, false, pMessageType, pMessage, sizeof(response), response) ;
	if (!ok)
	{
		// There was listening to this message
		strcpy(response, "**NOBODY RESPONDED**") ;
	}

	return response ;
}

/*************************************************************
* @brief Convert from a string version of an event to the int (enum) version.
*		 Returns smlEVENT_INVALID_EVENT (== 0) if the string is not recognized.
*************************************************************/
int KernelSML::ConvertStringToEvent(char const* pStr)
{
	return m_pEventMap->ConvertToEvent(pStr) ;
}

/*************************************************************
* @brief Convert from int version of an event to the string form.
*		 Returns NULL if the id is not recognized.
*************************************************************/
char const* KernelSML::ConvertEventToString(int id)
{
	return m_pEventMap->ConvertToString(id) ;
}

/*************************************************************
* @brief	Add a new connection to the list of connections
*			we're aware of to this soar kernel.
*************************************************************/
void KernelSML::AddConnection(Connection* pConnection)
{
	m_pConnectionManager->AddConnection(pConnection) ;
}

/*************************************************************
* @brief	Receive and process any messages from remote connections
*			that are waiting on a socket.
*			Returning false indicates we should stop checking
*			for more messages (and presumably shutdown completely).
*************************************************************/
bool KernelSML::ReceiveAllMessages()
{
	return m_pConnectionManager->ReceiveAllMessages() ;
}

/*************************************************************
* @brief	There should always be exactly one local connection
*			to us (the process that loaded us).
*************************************************************/
Connection* KernelSML::GetEmbeddedConnection()
{
	int index = 0 ;
	for (Connection* pConnection = m_pConnectionManager->GetConnectionByIndex(index) ; pConnection != NULL ; index++)
	{
		if (!pConnection->IsRemoteConnection())
			return pConnection ;
	}

	return NULL ;
}

/*************************************************************
* @brief	Stop the thread that is used to receive messages
*			from remote connections.  We do this when we're
*			using a "synchronized" embedded connection, which
*			means commands execute in the client's thread instead
*			of the receiver thread.
*************************************************************/
void KernelSML::StopReceiverThread()
{
	m_pConnectionManager->StopReceiverThread() ;
}

/*************************************************************
* @brief Turning this on means we'll start dumping output about messages
*		 being sent and received.  Currently this only applies to remote connections.
*************************************************************/
void KernelSML::SetTraceCommunications(bool state)
{
	m_pConnectionManager->SetTraceCommunications(state) ;
}

bool KernelSML::IsTracingCommunications()
{
	return m_pConnectionManager->IsTracingCommunications() ;
}

/*************************************************************
* @brief	Look up our additional SML information for a specific agent.
*
*			This will always return an AgentSML object.
*			If the IAgent* is new, this call will record a new AgentSML
*			object in the m_AgentMap and return a pointer to it.
*			We do this, so we can easily support connecting up to
*			agents that were created before a connection is established
*			through SML to the kernel (e.g. when attaching a debugger).
*	
*************************************************************/
AgentSML* KernelSML::GetAgentSML(gSKI::IAgent* pAgent)
{
	if (!pAgent)
		return NULL ;

	AgentSML* pResult = NULL ;

	// See if we already have an object in our map
	AgentMapIter iter = m_AgentMap.find(pAgent) ;

	if (iter == m_AgentMap.end())
	{
		// If not in the map, add it.
		pResult = new AgentSML(this, pAgent) ;
		m_AgentMap[pAgent] = pResult ;
	}
	else
	{
		// If in the map, return it.
		pResult = iter->second ;
	}

	return pResult ;
}

/*************************************************************
* @brief	Remove any event listeners for this connection.
*************************************************************/	
void KernelSML::RemoveAllListeners(Connection* pConnection)
{
	// Remove any agent specific listeners
	for (AgentMapIter iter = m_AgentMap.begin() ; iter != m_AgentMap.end() ; iter++)
	{
		AgentSML* pAgent = iter->second ;

		pAgent->RemoveAllListeners(pConnection) ;
	}

	// Remove any kernel event listeners
	m_AgentListener.RemoveAllListeners(pConnection);
	m_RhsListener.RemoveAllListeners(pConnection);
	m_SystemListener.RemoveAllListeners(pConnection);
	m_UpdateListener.RemoveAllListeners(pConnection) ;
	m_StringListener.RemoveAllListeners(pConnection) ;
}

/*************************************************************
* @brief	Delete the agent sml object for this agent.
*************************************************************/	
bool KernelSML::DeleteAgentSML(gSKI::IAgent* pAgent)
{
	// See if we already have an object in our map
	AgentMapIter iter = m_AgentMap.find(pAgent) ;

	if (iter == m_AgentMap.end())
		return false ;

	// Delete the agent sml information
	AgentSML* pResult = iter->second ;
	delete pResult ;

	// Erase the entry from the map
	m_AgentMap.erase(iter) ;

	return true ;
}

/*************************************************************
* @brief	Return a string to the caller.
*
* @param pResult	This is the string to be returned.
* @returns	False if the string is NULL.
*************************************************************/
bool KernelSML::ReturnResult(Connection* pConnection, ElementXML* pResponse, char const* pResult)
{
	if (!pResult)
		return false ;

	pConnection->AddSimpleResultToSMLResponse(pResponse, pResult) ;

	return true ;
}

/*************************************************************
* @brief	Return an integer result to the caller.
*************************************************************/
bool KernelSML::ReturnIntResult(Connection* pConnection, ElementXML* pResponse, int result)
{
	char buffer[kMinBufferSize] ;
	Int2String(result, buffer, kMinBufferSize) ;

	pConnection->AddSimpleResultToSMLResponse(pResponse, buffer) ;

	return true ;
}

/*************************************************************
* @brief	Return a boolean result to the caller.
*************************************************************/
bool KernelSML::ReturnBoolResult(Connection* pConnection, ElementXML* pResponse, bool result)
{
	char const* pResult = result ? sml_Names::kTrue : sml_Names::kFalse ;
	pConnection->AddSimpleResultToSMLResponse(pResponse, pResult) ;
	return true ;
}

/*************************************************************
* @brief	Return an invalid argument error to the caller.
*************************************************************/
bool KernelSML::InvalidArg(Connection* pConnection, ElementXML* pResponse, char const* pCommandName, char const* pErrorDescription)
{
	std::string msg = "Invalid arguments for command : " ;
	msg += pCommandName ;
	msg += pErrorDescription ;

	AddErrorMsg(pConnection, pResponse, msg.c_str()) ;
	
	// Return true because we've already added the error message.
	return true ;
}

/*************************************************************
* @brief	Look up an agent from its name.
*************************************************************/
IAgent* KernelSML::GetAgent(char const* pAgentName)
{
	if (!pAgentName)
		return NULL ;

	IAgent* pAgent = GetKernel()->GetAgentManager()->GetAgent(pAgentName) ;
	return pAgent ;
}

/*************************************************************
* @brief	Defines which phase we stop before when running by decision.
*			E.g. Pass input phase to stop just after generating output and before receiving input.
*			This is a setting which modifies the future behavior of "run <n> --decisions" commands.
*************************************************************/	
void KernelSML::SetStopBefore(egSKIPhaseType phase)
{
	m_pRunScheduler->SetStopBefore(phase) ;
	GetKernel()->FireSystemPropertyChangedEvent() ;
}

egSKIPhaseType KernelSML::GetStopBefore()
{
	return m_pRunScheduler->GetStopBefore() ;
}

/*************************************************************
* @brief	Take an incoming command and call the appropriate
*			handler to process it.
*************************************************************/
bool KernelSML::ProcessCommand(char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse)
{
	// Look up the function that handles this command
	CommandFunction pFunction = m_CommandMap[pCommandName] ;

	if (!pFunction)
	{
		// There is no handler for this command
		std::string msg = "Command " ;
		msg += pCommandName ;
		msg += " is not recognized by the kernel" ;

		AddErrorMsg(pConnection, pResponse, msg.c_str()) ;
		return false ;
	}

	// Look up the agent name parameter (most commands have this)
	char const* pAgentName = pIncoming->GetArgString(sml_Names::kParamAgent) ;

	IAgent* pAgent = NULL ;

	if (pAgentName)
	{
		pAgent = GetAgent(pAgentName) ;

		if (!pAgent)
		{
			// Failed to find this agent
			std::string msg = "Could not find an agent with name: " ;
			msg += pAgentName ;
			AddErrorMsg(pConnection, pResponse, msg.c_str()) ;
			return false ;
		}
	}

	// Create a blank error code
	gSKI::Error error ;
	ClearError(&error) ;

	// Call to the handler (this is a pointer to member call so it's a bit odd)
	bool result = (this->*pFunction)(pAgent, pCommandName, pConnection, pIncoming, pResponse, &error) ;

	// If we return false, we report a generic error about the call.
	if (!result)
	{
		std::string msg = "The call " ;
		msg += pCommandName ;
		msg += " failed to execute correctly." ;
		if (isError(error))
		{
			msg += "gSKI error was: " ;
			msg += error.Text ;
			msg += " details: " ;
			msg += error.ExtendedMsg ;
		}

		AddErrorMsg(pConnection, pResponse, msg.c_str()) ;
		return false ;
	}

	return true ;
}

/*************************************************************
* @brief	Enable/disable the print callback for a given agent.
*			This allows us to use the print callback within the
*			kernel without forwarding that output to clients
*			(useful for capturing the output from some commands).
*************************************************************/
void KernelSML::DisablePrintCallback(gSKI::IAgent* pAgent)
{
	GetAgentSML(pAgent)->DisablePrintCallback() ;
}

void KernelSML::EnablePrintCallback(gSKI::IAgent* pAgent)
{
	GetAgentSML(pAgent)->EnablePrintCallback() ;
}

/*************************************************************
* @brief	Takes an incoming SML message and responds with
*			an appropriate response message.
*
* @param pConnection	The connection this message came in on.
* @param pIncoming		The incoming message
*************************************************************/
ElementXML* KernelSML::ProcessIncomingSML(Connection* pConnection, ElementXML* pIncomingMsg)
{
	if (!pIncomingMsg || !pConnection)
		return NULL ;

	// Make sure only one thread is executing commands in the kernel at a time.
	// This is really just an insurance policy as I don't think we'll ever execute
	// commands on different threads within kernelSML because we
	// only allow one embedded connection to the kernel, but it's nice to be sure.
	soar_thread::Lock lock(m_pKernelMutex) ;

#ifdef DEBUG
	// For debugging, it's helpful to be able to look at the incoming message as an XML string
	char* pIncomingXML = pIncomingMsg->GenerateXMLString(true) ;
#endif

	ElementXML* pResponse = pConnection->CreateSMLResponse(pIncomingMsg) ;

	// Fatal error creating the response
	if (!pResponse)
		return NULL ;

	// Analyze the message and find important tags
	AnalyzeXML msg ;
	msg.Analyze(pIncomingMsg) ;

	// Get the "name" attribute from the <command> tag
	char const* pCommandName = msg.GetCommandName() ;

	if (pCommandName)
	{
		ProcessCommand(pCommandName, pConnection, &msg, pResponse) ;
	}
	else
	{
		// The message wasn't something we recognize.
		if (!msg.GetCommandTag())
			AddErrorMsg(pConnection, pResponse, "Incoming message did not contain a <command> tag") ;
		else
			AddErrorMsg(pConnection, pResponse, "Incoming message did not contain a name attribute in the <command> tag") ;
	}

#ifdef DEBUG
	// For debugging, it's helpful to be able to look at the response as XML
	char* pResponseXML = pResponse->GenerateXMLString(true) ;

	// Set a break point on this next line if you wish to see the incoming
	// and outgoing as XML before they get deleted.
	ElementXML::DeleteString(pIncomingXML) ;
	ElementXML::DeleteString(pResponseXML) ;
#endif

	return pResponse ;
}

/////////////////////////////////////////////////////////////////
// KernelSMLDirect methods.
// 
// These provide a higher speed access to a few methods that we
// need for I/O with an embedded connection.
// These just give us a way to optimize performance on the most
// time critical part of the interface.
/////////////////////////////////////////////////////////////////

// Unfortunately we need to keep track of all wmes as they are created and destroyed
// so that we can release them properly if the user issues an "init-soar".
// Under gSKI's current memory model this is the client's responsibility or the agent
// won't release its existing WMEs.  Also the reason we go to the extend of passing in
// the clientTimeTag is because wme->GetTimeTag() is only valid once the wme has actually
// been added to the kernel's working memory--which won't happen until Soar is next run
// and we pass through an input-phase.
static inline void RecordWME_Map(IWorkingMemory* wm, IWme* wme, long clientTimeTag)
{
	AgentSML* pAgentSML = KernelSML::GetKernelSML()->GetAgentSML(wm->GetAgent()) ;

	pAgentSML->RecordLongTimeTag( clientTimeTag, wme ) ;

//	KernelSML::DebugPrint(wme->GetValue()->GetString(), clientTimeTag, "Recorded\n") ;
}

static inline void RemoveWME_Map(IWorkingMemory* wm, IWme* wme, long clientTimeTag)
{
	unused(wme) ;
	AgentSML* pAgentSML = KernelSML::GetKernelSML()->GetAgentSML(wm->GetAgent()) ;

	pAgentSML->RemoveLongTimeTag( clientTimeTag ) ;

//	KernelSML::DebugPrint(wme->GetValue()->GetString(), clientTimeTag), "Removed\n") ;
}

/*************************************************************
* @brief	Add a wme.
* @param input		True if adding to input link.  False if adding to output link
* @param parent		NULL if adding to the root, otherwise the identifier (WMObject) we're adding to.
* @param pAttribute The attribute name to use
* @param value		The value to use
*************************************************************/
EXPORT Direct_WME_Handle sml_DirectAddWME_String(Direct_WorkingMemory_Handle wm, Direct_WMObject_Handle parent, long clientTimeTag, char const* pAttribute, char const* value)
{
	Direct_WME_Handle wme = (Direct_WME_Handle)((IWorkingMemory*)wm)->AddWmeString((IWMObject*)parent, pAttribute, value) ;
	RecordWME_Map((IWorkingMemory*)wm, (IWme*)wme, clientTimeTag) ;

	return wme ;
}

EXPORT Direct_WME_Handle sml_DirectAddWME_Int(Direct_WorkingMemory_Handle wm, Direct_WMObject_Handle parent, long clientTimeTag, char const* pAttribute, int value)
{
	Direct_WME_Handle wme = (Direct_WME_Handle)((IWorkingMemory*)wm)->AddWmeInt((IWMObject*)parent, pAttribute, value) ;
	RecordWME_Map((IWorkingMemory*)wm, (IWme*)wme, clientTimeTag) ;

	return wme ;
}

EXPORT Direct_WME_Handle sml_DirectAddWME_Double(Direct_WorkingMemory_Handle wm, Direct_WMObject_Handle parent, long clientTimeTag, char const* pAttribute, double value)
{
	Direct_WME_Handle wme = (Direct_WME_Handle)((IWorkingMemory*)wm)->AddWmeDouble((IWMObject*)parent, pAttribute, value) ;
	RecordWME_Map((IWorkingMemory*)wm, (IWme*)wme, clientTimeTag) ;

	return wme ;
}

/*************************************************************
* @brief	Remove a wme.  This function also releases the IWme*
*			making it no longer valid.
* @param wm			The working memory object (either input or output)
* @param wme		The wme we're removing
*************************************************************/
EXPORT void sml_DirectRemoveWME(Direct_WorkingMemory_Handle wm, Direct_WME_Handle wme, long clientTimeTag)
{
	// Remove this from the list of wme's we're tracking.  Do this before removing it from working memory
	RemoveWME_Map((IWorkingMemory*)wm, (IWme*)wme, clientTimeTag) ;

	// Remove the wme from working memory
	((IWorkingMemory*)wm)->RemoveWme((IWme*)wme) ;

	// We used to release after the call to RemoveWme, but we've clarified the definition
	// of remove wme so that it release the wme after it is actually deleted (which won't occur
	// until the next input phase).  This makes our clean up safer and easier.
    //	((IWme*)wme)->Release() ;
}

/*************************************************************
* @brief	Creates a new identifier (parent ^attribute <new-id>).
* @param wm			The working memory object (either input or output)
* @param parent		The identifier (WMObject) we're adding to.
* @param pAttribute	The attribute to add
*************************************************************/
EXPORT Direct_WME_Handle sml_DirectAddID(Direct_WorkingMemory_Handle wm, Direct_WMObject_Handle parent, long clientTimeTag, char const* pAttribute)
{
	Direct_WME_Handle wme = (Direct_WME_Handle)((IWorkingMemory*)wm)->AddWmeNewObject((IWMObject*)parent, pAttribute) ;
	RecordWME_Map((IWorkingMemory*)wm, (IWme*)wme, clientTimeTag) ;

	return wme ;
}

/*************************************************************
* @brief	Creates a new wme with an existing id as its value
*			(parent ^attribute <existing-id>)
* @param wm			The working memory object (either input or output)
* @param parent		The identifier (WMObject) we're adding to.
* @param pAttribute	The attribute to add
* @param orig		The original identifier (whose value we are copying)
*************************************************************/
EXPORT Direct_WME_Handle sml_DirectLinkID(Direct_WorkingMemory_Handle wm, Direct_WMObject_Handle parent, long clientTimeTag, char const* pAttribute, Direct_WMObject_Handle orig)
{
	Direct_WME_Handle wme = (Direct_WME_Handle)((IWorkingMemory*)wm)->AddWmeObjectLink((IWMObject*)parent, pAttribute, (IWMObject*)orig) ;
	RecordWME_Map((IWorkingMemory*)wm, (IWme*)wme, clientTimeTag) ;

	return wme ;
}

/*************************************************************
* @brief	The AddID methods return a WME, but for gSKI you need
*			a WMObject to work with them, so this gets the identifier
*			(WMObject) from a wme.
*			(parent ^attribute <id>) - returns <id> (not parent)
*************************************************************/
EXPORT Direct_WMObject_Handle sml_DirectGetThisWMObject(Direct_WorkingMemory_Handle wm, Direct_WME_Handle wme)
{
	unused(wm) ;
	Direct_WMObject_Handle wmobject = (Direct_WMObject_Handle)((IWme*)wme)->GetValue()->GetObject() ;
	return wmobject ;
}

EXPORT Direct_WorkingMemory_Handle sml_DirectGetWorkingMemory(char const* pAgentName, bool input)
{
	IAgent* pAgent = KernelSML::GetKernelSML()->GetKernel()->GetAgentManager()->GetAgent(pAgentName) ;

	if (!pAgent)
		return 0 ;

	if (input)
		return (Direct_WorkingMemory_Handle)pAgent->GetInputLink()->GetInputLinkMemory() ;
	else
		return (Direct_WorkingMemory_Handle)pAgent->GetOutputLink()->GetOutputMemory() ;
}

EXPORT Direct_WMObject_Handle sml_DirectGetRoot(char const* pAgentName, bool input)
{
	IAgent* pAgent = KernelSML::GetKernelSML()->GetKernel()->GetAgentManager()->GetAgent(pAgentName) ;

	if (!pAgent)
		return 0 ;

	IWMObject* pRoot = NULL ;
	if (input)
	{
		pAgent->GetInputLink()->GetRootObject(&pRoot) ;
		KernelSML::GetKernelSML()->GetAgentSML(pAgent)->SetInputLinkRoot(pRoot) ;
	}
	else
	{
		pAgent->GetOutputLink()->GetRootObject(&pRoot) ;
		KernelSML::GetKernelSML()->GetAgentSML(pAgent)->SetOutputLinkRoot(pRoot) ;
	}

	return (Direct_WMObject_Handle)pRoot ;
}

// A fully direct run would be a call straight to gSKI but supporting that is too dangerous
// due to the extra events and control logic surrounding the SML RunScheduler.
// So we compromise with a call directly to that scheduler, boosting performance over the standard "run" path
// which goes through the command line processor.
EXPORT void sml_DirectRun(char const* pAgentName, bool forever, int stepSize, int count)
{
	// Decide on the type of run.
	egSKIRunType runType ;
	if (forever)
		runType = gSKI_RUN_FOREVER ;
	else
	{
		switch ((smlRunStepSize)stepSize)
		{
		case sml_PHASE:       runType = gSKI_RUN_PHASE ; break ;
		case sml_ELABORATION: runType = gSKI_RUN_ELABORATION_CYCLE ; break ;
		case sml_DECISION:    runType = gSKI_RUN_DECISION_CYCLE ; break ;
		case sml_UNTIL_OUTPUT:runType = gSKI_RUN_UNTIL_OUTPUT ; break ;
		default: assert(0) ; return ;
		}
	}

	KernelSML* pKernelSML = KernelSML::GetKernelSML() ;

	RunScheduler* pScheduler = pKernelSML->GetRunScheduler() ;
	smlRunFlags runFlags = sml_NONE ;

	if (pAgentName)
	{
		gSKI::IAgent* pAgent = pKernelSML->GetAgent(pAgentName) ;
		
		if (!pAgent)
			return ;

		AgentSML* pAgentSML = pKernelSML->GetAgentSML(pAgent) ;
		runFlags = (smlRunFlags)(runFlags | sml_RUN_SELF) ;

		// Schedule just this one agent to run
		pScheduler->ScheduleAllAgentsToRun(false) ;
		pScheduler->ScheduleAgentToRun(pAgentSML, true) ;
	}
	else
	{
		runFlags = (smlRunFlags)(runFlags | sml_RUN_ALL) ;

		// Ask all agents to run
		pScheduler->ScheduleAllAgentsToRun(true) ;
	}

	// Decide how large of a step to run each agent before switching to the next agent
	// By default, we run one phase per agent but this isn't always appropriate.
	egSKIRunType interleaveStepSize = gSKI_RUN_PHASE ;

	egSKIInterleaveType interleave  = pScheduler->DefaultInterleaveStepSize(runType) ;

	switch (runType)
	{
		// If the entire system is running by elaboration cycles, then we need to run each agent by elaboration cycles (they're usually
		// smaller than a phase).
		case gSKI_RUN_ELABORATION_CYCLE: interleaveStepSize = gSKI_RUN_ELABORATION_CYCLE ; break ;

		// If we're running the system to output we want to run each agent until it generates output.  This can be many decisions.
		// The reason is actually to do with stopping the agent after n decisions (default 15) if no output occurs.
		// DJP -- We need to rethink this design so using phase interleaving until we do.
		// case gSKI_RUN_UNTIL_OUTPUT: interleaveStepSize = gSKI_RUN_UNTIL_OUTPUT ; break ;

		default: interleaveStepSize = gSKI_RUN_PHASE ; break ;
	}

	pScheduler->VerifyStepSizeForRunType( runType, interleave) ;

	// If we're running by decision cycle synchronize up the agents to the same phase before we start
	bool synchronizeAtStart = (runType == gSKI_RUN_DECISION_CYCLE) ;

	// Do the run

#ifdef USE_OLD_SCHEDULER
	egSKIRunResult runResult = pScheduler->RunScheduledAgents(runType, count, runFlags, interleaveStepSize, synchronizeAtStart, NULL) ;
#endif
#ifdef USE_NEW_SCHEDULER
	egSKIRunResult runResult = pScheduler->RunScheduledAgents(runType, count, runFlags, interleave, synchronizeAtStart, NULL) ;
#endif

	unused(runResult) ;

	return ;
}

EXPORT void sml_DirectReleaseWME(Direct_WorkingMemory_Handle wm, Direct_WME_Handle wme, long clientTimeTag)
{
	// Remove this from the list of wme's we're tracking.  Do this before we release it
	RemoveWME_Map((IWorkingMemory*)wm, (IWme*)wme, clientTimeTag) ;

	((IWme*)wme)->Release() ;
}

EXPORT void sml_DirectReleaseWMObject(Direct_WMObject_Handle parent)
{
	((IWMObject*)parent)->Release() ;
}

/*
EXPORT Direct_Agent_Handle sml_DirectGetAgent(char const* pAgentName)
{
	IAgent* pAgent = KernelSML::GetKernelSML()->GetKernel()->GetAgentManager()->GetAgent(pAgentName) ;
	return (Direct_Agent_Handle)pAgent ;
}
*/
