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
#include "KernelSMLDirect.h"

#include "thread_Lock.h"

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
	// Create a Kernel Factory
	m_pKernelFactory = gSKI_CreateKernelFactory();
   
	// Create the kernel instance
	m_pIKernel = m_pKernelFactory->Create();

	// Give the command line interface a reference to the kernel interface
	m_CommandLineInterface.SetKernel(m_pIKernel);

#ifdef USE_TCL_DEBUGGER
	m_Debugger = NULL ;
#endif

	// Create the map from command name to handler function
	BuildCommandMap() ; 

	// Start listening for incoming connections
	m_pConnectionManager = new ConnectionManager(portToListenOn) ;

	// We'll use this to make sure only one connection is executing commands
	// in the kernel at a time.
	m_pMutex = new soar_thread::Mutex() ;
}

KernelSML::~KernelSML()
{
	// Shutdown the connection manager while all data is still valid.
	// (This should have already been done before we get to the destructor,
	//  but this is a safety valve).
	Shutdown() ;

	// Clean up any agent specific data we still own.
	// We do this before deleting the kernel itself, so we get
	// clean stats on whether we've released all memory in gSKI or not.
	// (Check the gski_unreleased.txt file in the app's exe directory for a debug build).
	for (AgentMapIter iter = m_AgentMap.begin() ; iter != m_AgentMap.end() ; iter++)
	{
		AgentSML* pAgentSML = iter->second ;

		// Release any wmes or other objects we're keeping
		pAgentSML->Clear() ;

		// Make the call.
		GetKernel()->GetAgentManager()->RemoveAgent(pAgentSML->GetIAgent()) ;

		// Now delete the agent information we're keeping.
		delete pAgentSML ;
	}

	if (m_pKernelFactory && m_pIKernel)
		m_pKernelFactory->DestroyKernel(m_pIKernel);

	delete m_pKernelFactory ;

	delete m_pConnectionManager ;

	delete m_pMutex ;
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
	m_KernelListener.RemoveAllListeners(pConnection) ;
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
	char const* pAgentName = pIncoming->GetArgValue(sml_Names::kParamAgent) ;

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
	// This allows us to have a separate thread for managing remote connections
	// and yet not need the kernel itself to support multi-threaded access.
	soar_thread::Lock lock(m_pMutex) ;

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
// won't release its existing WMEs.
static inline void RecordWME_Map(IWorkingMemory* wm, IWme* wme)
{
	AgentSML* pAgentSML = KernelSML::GetKernelSML()->GetAgentSML(wm->GetAgent()) ;

	// DJP: I'm disabling this for now as there are still bugs around which objects
	// to release prior to init-soar.  Until I've figured out what to release and what not to
	// it's safer to leave this out (which means init-soar will fail, but nothing will crash).
	pAgentSML->RecordLongTimeTag( wme->GetTimeTag(), wme ) ;

//	KernelSML::DebugPrint(wme->GetValue()->GetString(), wme->GetTimeTag(), "Recorded\n") ;
}

static inline void RemoveWME_Map(IWorkingMemory* wm, IWme* wme)
{
	AgentSML* pAgentSML = KernelSML::GetKernelSML()->GetAgentSML(wm->GetAgent()) ;

	// DJP: I'm disabling this for now as there are still bugs around which objects
	// to release prior to init-soar.  Until I've figured out what to release and what not to
	// it's safer to leave this out (which means init-soar will fail, but nothing will crash).
	pAgentSML->RemoveLongTimeTag( wme->GetTimeTag() ) ;

//	KernelSML::DebugPrint(wme->GetValue()->GetString(), wme->GetTimeTag(), "Removed\n") ;
}

/*************************************************************
* @brief	Add a wme.
* @param input		True if adding to input link.  False if adding to output link
* @param parent		NULL if adding to the root, otherwise the identifier (WMObject) we're adding to.
* @param pAttribute The attribute name to use
* @param value		The value to use
*************************************************************/
EXPORT Direct_WME_Handle sml_DirectAddWME_String(Direct_WorkingMemory_Handle wm, Direct_WMObject_Handle parent, char const* pAttribute, char const* value)
{
	Direct_WME_Handle wme = (Direct_WME_Handle)((IWorkingMemory*)wm)->AddWmeString((IWMObject*)parent, pAttribute, value) ;
	RecordWME_Map((IWorkingMemory*)wm, (IWme*)wme) ;

	return wme ;
}

EXPORT Direct_WME_Handle sml_DirectAddWME_Int(Direct_WorkingMemory_Handle wm, Direct_WMObject_Handle parent, char const* pAttribute, int value)
{
	Direct_WME_Handle wme = (Direct_WME_Handle)((IWorkingMemory*)wm)->AddWmeInt((IWMObject*)parent, pAttribute, value) ;
	RecordWME_Map((IWorkingMemory*)wm, (IWme*)wme) ;

	return wme ;
}

EXPORT Direct_WME_Handle sml_DirectAddWME_Double(Direct_WorkingMemory_Handle wm, Direct_WMObject_Handle parent, char const* pAttribute, double value)
{
	Direct_WME_Handle wme = (Direct_WME_Handle)((IWorkingMemory*)wm)->AddWmeDouble((IWMObject*)parent, pAttribute, value) ;
	RecordWME_Map((IWorkingMemory*)wm, (IWme*)wme) ;

	return wme ;
}

/*************************************************************
* @brief	Remove a wme.  This function also releases the IWme*
*			making it no longer valid.
* @param wm			The working memory object (either input or output)
* @param wme		The wme we're removing
*************************************************************/
EXPORT void sml_DirectRemoveWME(Direct_WorkingMemory_Handle wm, Direct_WME_Handle wme)
{
	// Remove this from the list of wme's we're tracking.  Do this before removing it from working memory
	RemoveWME_Map((IWorkingMemory*)wm, (IWme*)wme) ;

	// Remove the wme from working memory
	((IWorkingMemory*)wm)->RemoveWme((IWme*)wme) ;

	// Release the object so the client doesn't have to call back in and do that.
	((IWme*)wme)->Release() ;
}

/*************************************************************
* @brief	Creates a new identifier (parent ^attribute <new-id>).
* @param wm			The working memory object (either input or output)
* @param parent		The identifier (WMObject) we're adding to.
* @param pAttribute	The attribute to add
*************************************************************/
EXPORT Direct_WME_Handle sml_DirectAddID(Direct_WorkingMemory_Handle wm, Direct_WMObject_Handle parent, char const* pAttribute)
{
	Direct_WME_Handle wme = (Direct_WME_Handle)((IWorkingMemory*)wm)->AddWmeNewObject((IWMObject*)parent, pAttribute) ;
	RecordWME_Map((IWorkingMemory*)wm, (IWme*)wme) ;

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
EXPORT Direct_WME_Handle sml_DirectLinkID(Direct_WorkingMemory_Handle wm, Direct_WMObject_Handle parent, char const* pAttribute, Direct_WMObject_Handle orig)
{
	Direct_WME_Handle wme = (Direct_WME_Handle)((IWorkingMemory*)wm)->AddWmeObjectLink((IWMObject*)parent, pAttribute, (IWMObject*)orig) ;
	RecordWME_Map((IWorkingMemory*)wm, (IWme*)wme) ;

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

EXPORT void sml_DirectRunTilOutput(char const* pAgentName)
{
	IAgent* pAgent = KernelSML::GetKernelSML()->GetKernel()->GetAgentManager()->GetAgent(pAgentName) ;

	if (!pAgent)
		return ;

	pAgent->RunInClientThread(gSKI_RUN_UNTIL_OUTPUT) ;
}

EXPORT void sml_DirectReleaseWME(Direct_WorkingMemory_Handle wm, Direct_WME_Handle wme)
{
	// Remove this from the list of wme's we're tracking.  Do this before we release it
	RemoveWME_Map((IWorkingMemory*)wm, (IWme*)wme) ;

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
