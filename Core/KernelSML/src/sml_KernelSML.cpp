#include <portability.h>

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

#include "sml_Utils.h"
#include "sml_AgentSML.h"
#include "sml_Connection.h"
#include "sml_OutputListener.h"
#include "sml_ConnectionManager.h"
#include "sml_Events.h"
#include "sml_RunScheduler.h"
#include "sml_KernelHelpers.h"
#include "KernelSMLDirect.h"

#include "thread_Lock.h"
#include "thread_Thread.h"

#include <iostream>
#include <fstream>
#include <map>
#include <stdlib.h>

#include "KernelHeaders.h"

using namespace sml ;

// Singleton instance of the kernel object
KernelSML* KernelSML::s_pKernel = NULL ;

// On Windows this is set to the DLL's hModule handle.
void* KernelSML::s_hModule = NULL ;

static soarxml::ElementXML* AddErrorMsg(Connection* pConnection, soarxml::ElementXML* pResponse, char const* pErrorMsg, int errorCode = -1)
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

/*************************************************************
* @brief	Return pointer to the class that handles the command
*			line parsing and execution of commands.
*************************************************************/
cli::CommandLineInterface* KernelSML::GetCommandLineInterface()
{
	if (!s_pKernel)
		return NULL ;

	return &s_pKernel->m_CommandLineInterface ;
}

KernelSML::KernelSML(unsigned short portToListenOn)
{
	// Initalize the event map
	m_pEventMap = new Events() ;

	// Give the command line interface a reference to the kernel interface
	m_CommandLineInterface.SetKernel(this);

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

	m_pKernelHelpers = new KernelHelpers() ;

	// We'll use this to make sure only one connection is executing commands
	// in the kernel at a time.
	m_pKernelMutex = new soar_thread::Mutex() ;

	m_SuppressSystemStart = false ;
	m_SuppressSystemStop  = false ;
	m_RequireSystemStop   = false ;

	m_pRunScheduler = new RunScheduler(this) ;

	m_EchoCommands = false ;

	m_InterruptCheckRate = 10;
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

		HandleDestroyAgent( pAgentSML, 0, 0, 0, 0 );

		// Now wait for the agent to be deleted (if we were requested to do so)
		int maxTries = 100 ;	// Wait for a second then abort anyway
		while (waitTillDeleted && agentCount == m_AgentMap.size() && maxTries > 0)
		{
			sml::Sleep(0, 10) ;
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

	delete m_pKernelHelpers ;

	delete m_pConnectionManager ;

	delete m_pKernelMutex ;

	delete m_pEventMap ;

	delete m_pRunScheduler;
}

/*************************************************************
* @brief	Shutdown any connections and sockets in preparation
*			for the kernel process exiting.
*************************************************************/
void KernelSML::Shutdown()
{
	m_pConnectionManager->Shutdown() ;
}

void KernelSML::SetStopPoint(bool forever, smlRunStepSize runStepSize, smlPhase m_StopBeforePhase)
{
	if ((sml_DECISION == runStepSize) || forever) {
		m_StopPoint = m_StopBeforePhase ;
	} else  {
		m_StopPoint = sml_INPUT_PHASE ;
	}
}


//////////////////////////////////////
/*************************************************************
* @brief	Notify listeners that this event has occured.
*************************************************************/
std::string KernelSML::FireEditProductionEvent(char const* pProduction) { 

	const int kBufferLength = 10000; 
	char response[kBufferLength] ;
	response[0] = 0 ;

	StringListenerCallbackData callbackData;

	callbackData.pData = pProduction;
	callbackData.maxLengthReturnStringBuffer = kBufferLength;
	callbackData.pReturnStringBuffer = response;

	m_StringListener.OnKernelEvent( smlEVENT_EDIT_PRODUCTION, 0, &callbackData );

	// This next bit of code is completely redundant, the strcpy below is basically a no-op:

	//if ( response[0] == 0 )
	//{
	//	// return zero length string for success
	//	strcpy(response, "") ;
	//}

	return response ;
}

/*************************************************************
* @brief	Notify listeners that this event has occured.
*************************************************************/
std::string KernelSML::FireLoadLibraryEvent(char const* pLibraryCommand) { 

	const int kBufferLength = 10000; 
	char response[kBufferLength] ;
	response[0] = 0 ;

	StringListenerCallbackData callbackData;

	callbackData.pData = pLibraryCommand;
	callbackData.maxLengthReturnStringBuffer = kBufferLength;
	callbackData.pReturnStringBuffer = response;

	m_StringListener.OnKernelEvent( smlEVENT_LOAD_LIBRARY, 0, &callbackData );

	// This next bit of code is completely redundant, the strcpy below is basically a no-op:

	//if ( response[0] == 0 )
	//{
	//	// return zero length string for success
	//	strcpy(response, "") ;
	//}

	return response ;

}
//////////////////////////////////////

/*************************************************************
* @brief	Send this message out to any clients that are listening.
*			These messages are from one client to another--kernelSML is just
*			facilitating the message passing process without knowing/caring what is being passed.
*************************************************************/
std::string KernelSML::SendClientMessage(AgentSML* pAgentSML, char const* pMessageType, char const* pMessage)
{
	char response[10000] ;
	response[0] = 0 ;

	bool ok = m_RhsListener.HandleEvent(smlEVENT_CLIENT_MESSAGE, pAgentSML, false, pMessageType, pMessage, sizeof(response), response) ;
	if (!ok)
	{
		// There was listening to this message
		strcpy(response, "**NOBODY RESPONDED**") ;
	}

	return response ;
}

/*************************************************************
* @brief	Send this command line out to all clients that have
*			registered a filter.  The result is the processed
*			version of the command line.
*			Returns true if at least one filter was registered.
*************************************************************/
bool KernelSML::SendFilterMessage(AgentSML* pAgent, char const* pCommandLine, std::string* pResult)
{
	char response[10000] ;
	response[0] = 0 ;

	bool ok = m_RhsListener.HandleFilterEvent(smlEVENT_FILTER, pAgent, pCommandLine, sizeof(response), response) ;

	if (!ok)
	{
		// Nobody was listening, so just return the original command line
		*pResult = pCommandLine ;
		return false ;
	}
	else
	{
		// Somebody filtered this command, so return the results of that filtering
		// (this can be "")
		*pResult = response ;
		return true ;
	}
}

/*************************************************************
* @brief	Returns true if at least one filter is registered.
*************************************************************/
bool KernelSML::HasFilterRegistered()
{
	ConnectionList* pListeners = m_RhsListener.GetRhsListeners(sml_Names::kFilterName) ;

	return (pListeners && pListeners->size() > 0) ;
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

	// Notify listeners that we have a new connection.
	this->FireSystemEvent(smlEVENT_AFTER_CONNECTION) ;
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
* @brief	Returns the number of agents.
*************************************************************/	
int	KernelSML::GetNumberAgents()
{
	// FIXME: this function should return unsigned
	return static_cast<int>( m_AgentMap.size() );
}

/*************************************************************
* @brief	Remove any event listeners for this connection.
*************************************************************/	
void KernelSML::RemoveAllListeners(Connection* pConnection)
{
	// Remove any agent specific listeners
	for (AgentMapIter iter = m_AgentMap.begin() ; iter != m_AgentMap.end() ; iter++)
	{
		AgentSML* pAgentSML = iter->second ;

		pAgentSML->RemoveAllListeners(pConnection) ;
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
bool KernelSML::DeleteAgentSML( const char* agentName )
{
	// See if we already have an object in our map
	AgentMapIter iter = m_AgentMap.find( agentName ) ;

	if (iter == m_AgentMap.end())
		return false ;

	KernelAgentMapIter kiter = m_KernelAgentMap.find( iter->second->GetSoarAgent() ) ;
	if (kiter == m_KernelAgentMap.end())
	{
		// Why is the agent in the gSKI map but not in the kernel map?
		assert(false) ;
		return false ;
	}

	// Delete the agent sml information
	// This is instead left up to the caller to do
	//AgentSML* pResult = iter->second ;
	//delete pResult ;

	// Erase the entry from the map
	m_AgentMap.erase(iter) ;
	m_KernelAgentMap.erase(kiter) ;

	return true ;
}

/*************************************************************
* @brief	Return a string to the caller.
*
* @param pResult	This is the string to be returned.
* @returns	False if the string is NULL.
*************************************************************/
bool KernelSML::ReturnResult(Connection* pConnection, soarxml::ElementXML* pResponse, char const* pResult)
{
	if (!pResult)
		return false ;

	pConnection->AddSimpleResultToSMLResponse(pResponse, pResult) ;

	return true ;
}

/*************************************************************
* @brief	Return an integer result to the caller.
*************************************************************/
bool KernelSML::ReturnIntResult(Connection* pConnection, soarxml::ElementXML* pResponse, int result)
{
	std::string temp;
	pConnection->AddSimpleResultToSMLResponse( pResponse, to_string( result, temp ).c_str() ) ;

	return true ;
}

/*************************************************************
* @brief	Return a boolean result to the caller.
*************************************************************/
bool KernelSML::ReturnBoolResult(Connection* pConnection, soarxml::ElementXML* pResponse, bool result)
{
	char const* pResult = result ? sml_Names::kTrue : sml_Names::kFalse ;
	pConnection->AddSimpleResultToSMLResponse(pResponse, pResult) ;
	return true ;
}

/*************************************************************
* @brief	Return an invalid argument error to the caller.
*************************************************************/
bool KernelSML::InvalidArg(Connection* pConnection, soarxml::ElementXML* pResponse, char const* pCommandName, char const* pErrorDescription)
{
	std::stringstream msg;
	msg << "Invalid arguments for command : " << pCommandName << pErrorDescription ;

	AddErrorMsg(pConnection, pResponse, msg.str().c_str()) ;
	
	// Return true because we've already added the error message.
	return true ;
}

/*************************************************************
* @brief	Look up an agent from its name.
*************************************************************/
AgentSML* KernelSML::GetAgentSML(char const* pAgentName)
{
	if (!pAgentName)
		return NULL ;
	
	AgentMapIter iter = m_AgentMap.find( pAgentName ) ;

	if (iter == m_AgentMap.end())
	{
		return NULL ;
	}
	else
	{
		// If in the map, return it.
		return iter->second ;
	}
}

/*************************************************************
* @brief	Defines which phase we stop before when running by decision.
*			E.g. Pass input phase to stop just after generating output and before receiving input.
*			This is a setting which modifies the future behavior of "run <n> --decisions" commands.
*************************************************************/	
void KernelSML::SetStopBefore(smlPhase phase)
{
	m_pRunScheduler->SetStopBefore(phase) ;
	this->FireSystemEvent(smlEVENT_SYSTEM_PROPERTY_CHANGED) ;
}

smlPhase KernelSML::GetStopBefore()
{
	return m_pRunScheduler->GetStopBefore() ;
}

top_level_phase KernelSML::ConvertSMLToSoarPhase( smlPhase phase ) 
{
	// check a few
	assert( INPUT_PHASE == static_cast< top_level_phase >( sml_INPUT_PHASE ) );
	assert( PREFERENCE_PHASE == static_cast< top_level_phase >( sml_PREFERENCE_PHASE ) );
	
	// just cast
	return static_cast< top_level_phase >( phase );
}


/*************************************************************
* @brief	Request that all agents stop soon
*************************************************************/	
void KernelSML::InterruptAllAgents(smlStopLocationFlags stopLoc)
{
	for (AgentMapIter iter = m_AgentMap.begin() ; iter != m_AgentMap.end() ; iter++)
	{
		AgentSML* pAgentSML = iter->second ;
		pAgentSML->Interrupt(stopLoc) ;
	}
}

void KernelSML::ClearAllInterrupts()
{
	for (AgentMapIter iter = m_AgentMap.begin() ; iter != m_AgentMap.end() ; iter++)
	{
		AgentSML* pAgentSML = iter->second ;
		pAgentSML->ClearInterrupts() ;
	}
}

/*************************************************************
* @brief	Take an incoming command and call the appropriate
*			handler to process it.
*************************************************************/
bool KernelSML::ProcessCommand(char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, soarxml::ElementXML* pResponse)
{
	// Look up the function that handles this command
	CommandFunction pFunction = m_CommandMap[pCommandName] ;

	if (!pFunction)
	{
		// There is no handler for this command
		std::stringstream msg;
		msg << "Command " << pCommandName << " is not recognized by the kernel" ;

		AddErrorMsg(pConnection, pResponse, msg.str().c_str()) ;
		return false ;
	}

	// Look up the agent name parameter (most commands have this)
	char const* pAgentName = pIncoming->GetArgString(sml_Names::kParamAgent) ;

	AgentSML* pAgentSML = NULL ;

	if (pAgentName)
	{
		pAgentSML = GetAgentSML( pAgentName ) ;

		if (!pAgentSML)
		{
			// Failed to find this agent
			std::stringstream msg;
			msg << "Could not find an agent with name: " << pAgentName ;
			AddErrorMsg(pConnection, pResponse, msg.str().c_str()) ;
			return false ;
		}
	}

	// Call to the handler (this is a pointer to member call so it's a bit odd)
	bool result = (this->*pFunction)(pAgentSML, pCommandName, pConnection, pIncoming, pResponse) ;

	// If we return false, we report a generic error about the call.
	if (!result)
	{
		std::stringstream msg;
		msg << "The call " << pCommandName << " failed to execute correctly." ;
		AddErrorMsg(pConnection, pResponse, msg.str().c_str()) ;
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
soarxml::ElementXML* KernelSML::ProcessIncomingSML(Connection* pConnection, soarxml::ElementXML* pIncomingMsg)
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

	soarxml::ElementXML* pResponse = pConnection->CreateSMLResponse(pIncomingMsg) ;

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
	soarxml::ElementXML::DeleteString(pIncomingXML) ;
	soarxml::ElementXML::DeleteString(pResponseXML) ;
#endif

	return pResponse ;
}

void KernelSML::Symbol2String(Symbol* pSymbol, 	bool refCounts, std::ostringstream& buffer) {
	if (pSymbol->common.symbol_type==IDENTIFIER_SYMBOL_TYPE) {
		buffer << pSymbol->id.name_letter ;
		buffer << pSymbol->id.name_number ;
	}
	else if (pSymbol->common.symbol_type==VARIABLE_SYMBOL_TYPE) {
		buffer << pSymbol->var.name ;
	}
	else if (pSymbol->common.symbol_type==SYM_CONSTANT_SYMBOL_TYPE) {
		buffer << pSymbol->sc.name ;
	}
	else if (pSymbol->common.symbol_type==INT_CONSTANT_SYMBOL_TYPE) {
		buffer << pSymbol->ic.value ;
	}
	else if (pSymbol->common.symbol_type==FLOAT_CONSTANT_SYMBOL_TYPE) {
		buffer << pSymbol->fc.value ;
	}

	if (refCounts)
		buffer << "[" << pSymbol->common.reference_count << "]" ;
}

std::string KernelSML::Wme2String(wme* pWME, bool refCounts) {
	std::ostringstream buffer ;

	buffer << pWME->timetag << ":" ;

	Symbol2String(pWME->id, refCounts, buffer) ;
	buffer << " ^" ;
	Symbol2String(pWME->attr, refCounts, buffer) ;
	buffer << " " ;
	Symbol2String(pWME->value, refCounts, buffer) ;

	return buffer.str() ;
}

void KernelSML::PrintDebugWme(char const* pMsg, wme* pWME, bool refCounts ) {
	std::string str = Wme2String(pWME, refCounts) ;
	PrintDebugFormat("%s %s", pMsg, str.c_str()) ;
}

void KernelSML::PrintDebugSymbol(Symbol* pSymbol, bool refCounts ) {
	std::ostringstream buffer ;
	Symbol2String(pSymbol, refCounts, buffer) ;
	std::string str = buffer.str() ;

	PrintDebugFormat("%s", str.c_str()) ;
}


/////////////////////////////////////////////////////////////////
// KernelSMLDirect methods.
// 
// These provide a higher speed access to a few methods that we
// need for I/O with an embedded connection.
// These just give us a way to optimize performance on the most
// time critical part of the interface.
/////////////////////////////////////////////////////////////////

/*************************************************************
* @brief	Add a wme.
* @param input		True if adding to input link.  False if adding to output link
* @param parent		NULL if adding to the root, otherwise the identifier (WMObject) we're adding to.
* @param pAttribute The attribute name to use
* @param value		The value to use
*************************************************************/
EXPORT void sml_DirectAddWME_String(Direct_AgentSML_Handle pAgentSMLIn, char const* pId, char const* pAttribute, char const* pValue, long clientTimetag)
{
	AgentSML* pAgentSML = reinterpret_cast<AgentSML*>(pAgentSMLIn);
	assert(pAgentSML);

	//std::stringstream timetagString;
	//timetagString << clientTimetag;

	//pAgentSML->AddInputWME( pId, pAttribute, pValue, sml_Names::kTypeString, timetagString.str().c_str() );
   pAgentSML->AddStringInputWME( pId, pAttribute, pValue, clientTimetag );
}

EXPORT void sml_DirectAddWME_Int(Direct_AgentSML_Handle pAgentSMLIn, char const* pId, char const* pAttribute, int value, long clientTimetag)
{
	AgentSML* pAgentSML = reinterpret_cast<AgentSML*>(pAgentSMLIn);
	assert(pAgentSML);

	// BADBAD conversion happening twice
	//std::stringstream valueString;
	//valueString << value;

	//std::stringstream timetagString;
	//timetagString << clientTimetag;

	//pAgentSML->AddInputWME( pId, pAttribute, valueString.str().c_str(), sml_Names::kTypeInt, timetagString.str().c_str() );
   pAgentSML->AddIntInputWME( pId, pAttribute, value, clientTimetag );
}

EXPORT void sml_DirectAddWME_Double(Direct_AgentSML_Handle pAgentSMLIn, char const* pId, char const* pAttribute, double value, long clientTimetag)
{
	AgentSML* pAgentSML = reinterpret_cast<AgentSML*>(pAgentSMLIn);
	assert(pAgentSML);

	// BADBAD conversion happening twice
	//std::stringstream valueString;
	//valueString << value;

	//std::stringstream timetagString;
	//timetagString << clientTimetag;

	//pAgentSML->AddInputWME( pId, pAttribute, valueString.str().c_str(), sml_Names::kTypeDouble, timetagString.str().c_str() );
   pAgentSML->AddDoubleInputWME( pId, pAttribute, value, clientTimetag );
}

/*************************************************************
* @brief	Remove a wme.  This function also releases the IWme*
*			making it no longer valid.
* @param wm			The working memory object (either input or output)
* @param wme		The wme we're removing
*************************************************************/
EXPORT void sml_DirectRemoveWME(Direct_AgentSML_Handle pAgentSMLIn, long clientTimetag)
{
	AgentSML* pAgentSML = reinterpret_cast<AgentSML*>(pAgentSMLIn);
	assert(pAgentSML);

//	std::stringstream timetagString;
//	timetagString << clientTimetag;

	pAgentSML->RemoveInputWME( clientTimetag );
}

/*************************************************************
* @brief	Creates a new identifier (parent ^attribute <new-id>).
* @param wm			The working memory object (either input or output)
* @param parent		The identifier (WMObject) we're adding to.
* @param pAttribute	The attribute to add
*************************************************************/
EXPORT void sml_DirectAddID(Direct_AgentSML_Handle pAgentSMLIn, char const* pId, char const* pAttribute, char const* pValueId, long clientTimetag)
{
	AgentSML* pAgentSML = reinterpret_cast<AgentSML*>(pAgentSMLIn);
	assert(pAgentSML);

	//std::stringstream timetagString;
	//timetagString << clientTimetag;

	//pAgentSML->AddInputWME( pId, pAttribute, pValueId, sml_Names::kTypeID, timetagString.str().c_str() );
   pAgentSML->AddIdInputWME( pId, pAttribute, pValueId, clientTimetag );
}

EXPORT Direct_AgentSML_Handle sml_DirectGetAgentSMLHandle(char const* pAgentName) 
{
	KernelSML* pKernelSML = KernelSML::GetKernelSML() ;

	AgentSML* pAgentSML = pKernelSML->GetAgentSML( pAgentName );

	return reinterpret_cast<Direct_AgentSML_Handle>(pAgentSML);
}

// A fully direct run would be a call straight to gSKI but supporting that is too dangerous
// due to the extra events and control logic surrounding the SML RunScheduler.
// So we compromise with a call directly to that scheduler, boosting performance over the standard "run" path
// which goes through the command line processor.
EXPORT void sml_DirectRun(char const* pAgentName, bool forever, int stepSize, int interleaveSizeIn, int count)
{
	smlRunStepSize interleaveSize = static_cast<smlRunStepSize>(interleaveSizeIn);
	KernelSML* pKernelSML = KernelSML::GetKernelSML() ;

	RunScheduler* pScheduler = pKernelSML->GetRunScheduler() ;
	smlRunFlags runFlags = sml_NONE ;

	// Decide on the type of run.
	smlRunStepSize runType = (forever) ? sml_DECISION : static_cast<smlRunStepSize>(stepSize) ;

	// Decide how large of a step to run each agent before switching to the next agent
	pScheduler->VerifyStepSizeForRunType( forever, runType, interleaveSize) ;

	if (pAgentName)
	{
		AgentSML* pAgentSML = pKernelSML->GetAgentSML( pAgentName );
		if (!pAgentSML)
			return ;

		runFlags = smlRunFlags(runFlags | sml_RUN_SELF) ;

		// Schedule just this one agent to run
		pScheduler->ScheduleAllAgentsToRun(false) ;
		pScheduler->ScheduleAgentToRun(pAgentSML, true) ;
	}
	else
	{
		runFlags = smlRunFlags(runFlags | sml_RUN_ALL) ;

		// Ask all agents to run
		pScheduler->ScheduleAllAgentsToRun(true) ;
	}

	// If we're running by decision cycle synchronize up the agents to the same phase before we start
	bool synchronizeAtStart = (runType == sml_DECISION) ;

	// Do the run
	pScheduler->RunScheduledAgents(forever, runType, count, runFlags, smlRunStepSize(interleaveSize), synchronizeAtStart) ;
	return ;
}

