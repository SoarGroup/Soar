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

#ifdef _WIN32
#include <crtdbg.h>
#endif

#include "sml_KernelSML.h"
#include "sml_AgentSML.h"
#include "sml_Connection.h"
#include "sml_StringOps.h"
#include "sml_OutputListener.h"
#include "sml_ConnectionManager.h"

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
#include "IgSKI_WME.h"

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

static ElementXML* AddErrorMsg(Connection* pConnection, ElementXML* pResponse, char const* pErrorMsg, int errorCode = -1)
{
	pConnection->AddErrorToSMLResponse(pResponse, pErrorMsg, errorCode) ;
	return pResponse ;
}

/*************************************************************
* @brief	Returns the singleton kernel object.
*************************************************************/
KernelSML* KernelSML::GetKernelSML()
{
	if (s_pKernel == NULL)
		s_pKernel = new KernelSML() ;

	return s_pKernel ;
}

KernelSML::KernelSML()
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
	m_pConnectionManager = new ConnectionManager() ;

	// We'll use this to make sure only one connection is executing commands
	// in the kernel at a time.
	m_pMutex = new soar_thread::Mutex() ;
}

KernelSML::~KernelSML()
{
	// Clean up any agent specific data we still own.
	// We do this before deleting the kernel itself, so we get
	// clean stats on whether we've released all memory in gSKI or not.
	// (Check the gski_unreleased.txt file in the app's exe directory for a debug build).
	for (AgentMapIter iter = m_AgentMap.begin() ; iter != m_AgentMap.end() ; iter++)
	{
		AgentSML* pAgentSML = iter->second ;
		delete pAgentSML ;
	}

	if (m_pKernelFactory && m_pIKernel)
		m_pKernelFactory->DestroyKernel(m_pIKernel);

	delete m_pConnectionManager ;

	delete m_pMutex ;
}

/*************************************************************
* @brief	Shutdown any connections and sockets in preparation
*			for the kernel process exiting.
*************************************************************/
void KernelSML::Shutdown()
{
	if (m_pConnectionManager)
		m_pConnectionManager->Shutdown() ;
}

/*************************************************************
* @brief	Add a new connection to the list of connections
*			we're aware of to this soar kernel.
*************************************************************/
void KernelSML::AddConnection(Connection* pConnection)
{
	if (m_pConnectionManager)
		m_pConnectionManager->AddConnection(pConnection) ;
}

/*************************************************************
* @brief	Receive and process any messages from remote connections
*			that are waiting on a socket.
*************************************************************/
void KernelSML::ReceiveAllMessages()
{
	if (m_pConnectionManager)
		m_pConnectionManager->ReceiveAllMessages() ;
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

