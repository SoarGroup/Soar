#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include <portability.h>

/////////////////////////////////////////////////////////////////
// Kernel class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : Sept 2004
//
// This class is used by a client app (e.g. an environment) to represent
// the top level connection to the Soar kernel.  You start by creating
// one of these and then creating agents through it etc.
//
/////////////////////////////////////////////////////////////////
#include "sml_ClientKernel.h"
#include "sml_ClientAgent.h"
#include "sml_Connection.h"
#include "sml_Errors.h"
#include "sml_StringOps.h"
#include "sml_EventThread.h"
#include "sml_Events.h"
#include "sml_ClientAnalyzedXML.h"

#include "sock_SocketLib.h"
#include "thread_Thread.h"	// To get to sleep
#include "EmbeddedSMLInterface.h" // for static reference

#include "sml_EmbeddedConnection.h"	// For access to direct methods
#include "sml_ClientDirect.h"

#include <iostream>     
#include <sstream>     
#include <iomanip>

#include <assert.h>

using namespace sml ;

char const* const Kernel::kDefaultLibraryName = "SoarKernelSML" ;

Kernel::Kernel(Connection* pConnection)
{
	m_Connection     = pConnection ;
	m_TimeTagCounter = 0 ;
	m_IdCounter      = 0 ;
	m_SocketLibrary  = NULL ;
	m_LastError		 = Error::kNoError ;
	m_CallbackIDCounter = 0 ;
	m_pEventThread	= 0 ;
	m_pEventMap		= new Events() ;
	m_bTracingCommunications = false ;
	m_bShutdown		= false ;
	m_ConnectionInfoChanged = false ;
	m_bIgnoreOutput = false ;
	m_FilteringEnabled = true ;

	// We're turning on auto commit by default, so clients are a bit slower but easier to write.
	// Power users are free to turn it off and use explicit commit calls.
	m_bAutoCommit   = true ;

	ClearError() ;

	if (pConnection)
	{
		m_pEventThread = new EventThread(pConnection) ;

		// We start the event thread for asynch connections (remote and embedded on a new thread).
		// Synchronous ones don't need it as the kernel can simply call right over to the client directly
		// for those.
		if (pConnection->IsAsynchronous())
			m_pEventThread->Start() ;
	}

/* voigtjr, rmarinie
 *
 * Upon further tinkering, we have discovered that the use of the code within
 * the following symbol on Linux is no longer necessary, and, in fact, causes 
 * problems, but we are leaving it in in case our analysis turns out to be
 * incorrect.
 */
	
//#ifdef LINUX_STATIC_LINK
	// On Linux the linker only makes a single pass through the libraries
	// so if we try to statically link all of the code together, it fails to
	// see sml_ProcessMessage and the other methods that are exported from KernelSML because
	// they are only referenced within EmbeddedConnection (inside ConnectionSML.lib)
	// which has to come later on the linker's command line than KernelSML.
	// A way to resolve this is to make an access from here in ClientSML (which appears
	// before KernelSML on the command line for the linker) to the
	// methods in KernelSML, which will force the linker to pull the code into the final executable.
	//
	// If we're in Windows this is not an issue (because the Windows linker supports these cyclical references)
	// and if we're loading KernelSML dynamically (the normal fashion) this is also not a problem.
//	sml_ProcessMessage(0,0,0);
//	sml_CreateEmbeddedConnection(0,0,0,0);
//#endif
}

/*************************************************************
* @brief Called when an init-soar event happens so we know
*		 to refresh the input/output links.
*************************************************************/
static void InitSoarHandler(smlAgentEventId id, void* pUserData, Agent* pAgent)
{
	unused(pUserData) ;
	unused(id) ;

	pAgent->Refresh() ;
}

/*************************************************************
* @brief Called when an init-soar event happens so we know
*		 to refresh the input/output links.
*************************************************************/
static void LoadLibraryHandler(smlStringEventId id, void* pUserData, Kernel* pKernel, char const* pString)
{
	unused(pUserData) ;
	unused(id) ;

	pKernel->LoadExternalLibrary(pString);
}

void Kernel::InitEvents()
{
	// Register for init-soar events
	RegisterForAgentEvent(smlEVENT_AFTER_AGENT_REINITIALIZED, &InitSoarHandler, NULL) ;

	// Register for load-library events
	RegisterForStringEvent(smlEVENT_LOAD_LIBRARY, &LoadLibraryHandler, NULL);
}

/*************************************************************
* @brief True if our connection to the kernel has been closed.
*		 (Generally used for remote connections)
*************************************************************/
bool Kernel::IsConnectionClosed()
{
	return !GetConnection() || GetConnection()->IsClosed() ;
}

/*************************************************************
* @brief Preparation for deleting the kernel.
*		 Agents are destroyed at this point (if we own the kernel)
*		 After calling shutdown the kernel cannot be restarted
*		 it must be deleted.
*		 This is separated from delete to ensure that messages
*		 relating to system shutdown can be sent in a more stable
*		 state (while the kernel object still exists).
*************************************************************/
void Kernel::Shutdown()
{
	m_bShutdown = true ;

	// Currently we have no work to do on the kernel side before
	// disconnecting a remote connection.
	if (!GetConnection() || GetConnection()->IsRemoteConnection())
	{
		if (GetConnection())
			GetConnection()->CloseConnection() ;

		return ;
	}

	AnalyzeXML response ;
	GetConnection()->SendAgentCommand(&response, sml_Names::kCommand_Shutdown) ;
	GetConnection()->CloseConnection() ;
}

/*************************************************************
* @brief Delete the kernel (or our connection to the kernel)
*		 releasing all memory owned by the kernel.
*		 Users should call "Shutdown" prior to calling delete
*		 to ensure a clean shutdown.
*************************************************************/
Kernel::~Kernel(void)
{
	// If the user didn't call shutdown, we'll do it now.
	// It's better for the user to call it so we have a more stable state
	// for handling messages.
	if (!m_bShutdown)
		Shutdown() ;

	// When the agent map is deleted, it will delete its contents (the Agent objects)
	// Do this before we delete the connection, in case we need to send things to the kernel
	// during clean up.
	m_AgentMap.clear() ;

	// We also need to close the connection
	if (m_Connection)
		m_Connection->CloseConnection() ;

	// Must stop the event thread before deleting the connection
	// as it has a pointer to the connection.
	if (m_pEventThread)
		m_pEventThread->Stop(true) ;

	// Clean up any connection info we have stored
	for (ConnectionListIter iter = m_ConnectionInfoList.begin() ; iter != m_ConnectionInfoList.end() ; iter++)
	{
		ConnectionInfo* pInfo = *iter ;
		delete pInfo ;
	}
	m_ConnectionInfoList.clear() ;

	delete m_pEventThread ;

	delete m_Connection ;

	// Deleting this shuts down the socket library if we were using it.
	delete m_SocketLibrary ;

	delete m_pEventMap ;
}

/*************************************************************
* @brief Start the event thread.
*
* This thread can be used to make sure the client remains responsive
* if it registers for some events and then goes to sleep.
* (E.g. in a keyboard input handler or a GUI message loop).
*
* This thread is started by default for remote connections
* and embedded connections in a new thread.  A client could
* reasonably choose to turn it off so we'll expose the methods
* for starting and stopping.
*************************************************************/
bool Kernel::StartEventThread()
{
	// This thread is used to listen for events from the kernel
	// when the client is sleeping
	if (!m_pEventThread)
		return false ;

	m_pEventThread->Start() ;

	return true ;
}

/*************************************************************
* @brief Stop the event thread.
*
* This thread can be used to make sure the client remains responsive
* if it registers for some events and then goes to sleep.
* (E.g. in a keyboard input handler or a GUI message loop).
*
* This thread is started by default for remote connections
* and embedded connections in a new thread.  A client could
* reasonably choose to turn it off so we'll expose the methods
* for starting and stopping.
*************************************************************/
bool Kernel::StopEventThread()
{
	// Shut down the event thread
	if (!m_pEventThread)
		return false ;

	m_pEventThread->Stop(true) ;

	return true ;
}

/*************************************************************
* @brief Turning this on means we'll start dumping output about messages
*		 being sent and received.
*************************************************************/
void Kernel::SetTraceCommunications(bool state)
{
	if (m_Connection)
		m_Connection->SetTraceCommunications(state) ;

	// We keep a local copy of this value so we can check it without
	// calling anywhere.
	m_bTracingCommunications = state ;
}

bool Kernel::IsTracingCommunications()
{
	return m_bTracingCommunications ;
}

/*************************************************************
* @brief This function is called when we receive a "call" SML
*		 message from the kernel.
*
* This is a static method and it's only function is to call
* a normal member function.
*************************************************************/
ElementXML* Kernel::ReceivedCall(Connection* pConnection, ElementXML* pIncoming, void* pUserData)
{
	Kernel* pKernel = (Kernel*)pUserData ;

	return pKernel->ProcessIncomingSML(pConnection, pIncoming) ;
}

/*************************************************************
* @brief If this message is an XML trace message returns
*		 the agent pointer this message is for.
*		 Otherwise returns NULL.
*		 This function is just to boost performance on trace messages
*		 which are really performance critical.
*************************************************************/
Agent* Kernel::IsXMLTraceEvent(ElementXML* pIncomingMsg)
{
	//	The message we're looking for has this structure:
	//	<sml><command></command><trace></trace></sml>
	// This is deliberately unusual so this simple test screens out
	// almost all messages in one go.  It does make us more brittle (for detecting
	// xml trace messages) but I think that's a fair trade-off.
	if (pIncomingMsg->GetNumberChildren() != 2)
		return NULL ;

	ElementXML command(NULL) ;
	ElementXML trace(NULL) ;
	pIncomingMsg->GetChild(&command, 0) ;
	pIncomingMsg->GetChild(&trace, 1) ;

	if (trace.IsTag(sml_Names::kTagTrace) && command.IsTag(sml_Names::kTagCommand) && command.GetNumberChildren() > 0)
	{
		ElementXML agentArg(NULL) ;
		command.GetChild(&agentArg, 0) ;

#ifdef _DEBUG
		char const* pParam = agentArg.GetAttribute(sml_Names::kArgParam) ;
		assert (pParam && strcmp(pParam, sml_Names::kParamAgent) == 0) ;
#endif
		// Get the agent's name
		char const* pAgentName = agentArg.GetCharacterData() ;

		if (!pAgentName || pAgentName[0] == 0)
			return NULL ;

		// Look up the agent
		Agent* pAgent = GetAgent(pAgentName) ;

		// If this fails, we got a trace event for a now deleted agent
		// (must have been flushed after the agent was destroyed).
		// Returning null is probably as good as we do here so
		// always return pAgent (even if it's null).
		return pAgent ;
	}

	return NULL ;
}

/*************************************************************
* @brief This function is called (indirectly) when we receive a "call" SML
*		 message from the kernel.
*************************************************************/
ElementXML* Kernel::ProcessIncomingSML(Connection* pConnection, ElementXML* pIncomingMsg)
{
	// Create a reply
	ElementXML* pResponse = pConnection->CreateSMLResponse(pIncomingMsg) ;

	// Make sure the connection hasn't been closed along the way
	if (pConnection->IsClosed())
		return pResponse ;

	// Special case.  We want to intercept XML trace messages and pass them directly to the handler
	// without analyzing them.  This is just to boost performance for these messages as speed is critical here
	// as they're used for trace output.
	Agent* pAgent = IsXMLTraceEvent(pIncomingMsg) ;
	if (pAgent)
	{
		pAgent->ReceivedXMLTraceEvent(smlEVENT_XML_TRACE_OUTPUT, pIncomingMsg, pResponse) ;
		return pResponse ;
	}

	// Analyze the message and find important tags
	AnalyzeXML msg ;
	msg.Analyze(pIncomingMsg) ;

	// Get the "name" attribute from the <command> tag
	char const* pCommandName = msg.GetCommandName() ;

	// Look up the agent name parameter (most commands have this)
	char const* pAgentName = msg.GetArgString(sml_Names::kParamAgent) ;

	// Find the client agent structure that matches this agent
	if (pAgentName && pCommandName)
	{
		Agent* pAgent = GetAgent(pAgentName) ;

		// If this is a command for a known agent and it's an "output" command
		// then we're interested in it.
		if (pAgent && strcmp(sml_Names::kCommand_Output, pCommandName) == 0)
		{
			// Pass the incoming message over to the agent
			pAgent->ReceivedOutput(&msg, pResponse) ;
		}

		if (pAgent && strcmp(sml_Names::kCommand_Event, pCommandName) == 0)
		{
			// This is an event specific to an agent, so handle it there.
			pAgent->ReceivedEvent(&msg, pResponse) ;
		}
	}
	else
	{
		// If this is a mesage for the kernel itself process it here
		if (!pAgentName)
		{
			if (strcmp(sml_Names::kCommand_Event, pCommandName) == 0)
			{
				// This is an event that is not agent specific
				this->ReceivedEvent(&msg, pResponse) ;
			}
		}
	}

	return pResponse ;
}

void Kernel::ReceivedEvent(AnalyzeXML* pIncoming, ElementXML* pResponse)
{
	char const* pEventName = pIncoming->GetArgString(sml_Names::kParamEventID) ;

	// This event had no event id field
	if (!pEventName)
	{
		return ;
	}

	// Convert from the string to an event ID
	int id = m_pEventMap->ConvertToEvent(pEventName) ;

	if (IsSystemEventID(id))
	{
		ReceivedSystemEvent((smlSystemEventId)id, pIncoming, pResponse) ;
	} else if (IsAgentEventID(id))
	{
		ReceivedAgentEvent((smlAgentEventId)id, pIncoming, pResponse) ;
	} else if (IsRhsEventID(id))
	{
		ReceivedRhsEvent((smlRhsEventId)id, pIncoming, pResponse) ;
	} else if (IsUpdateEventID(id))
	{
		ReceivedUpdateEvent((smlUpdateEventId)id, pIncoming, pResponse) ;
	} else if (IsStringEventID(id))
	{
		ReceivedStringEvent((smlStringEventId)id, pIncoming, pResponse) ;
	}
}

/*************************************************************
* @brief This function is called when an event is received
*		 from the Soar kernel.
*
* @param pIncoming	The event command
* @param pResponse	The reply (no real need to fill anything in here currently)
*************************************************************/
void Kernel::ReceivedSystemEvent(smlSystemEventId id, AnalyzeXML* pIncoming, ElementXML* pResponse)
{
	unused(pResponse) ;
	unused(pIncoming) ;

	// Look up the handler(s) from the map
	SystemEventMap::ValueList* pHandlers = m_SystemEventMap.getList(id) ;

	if (!pHandlers)
		return ;

	// Go through the list of event handlers calling each in turn
	for (SystemEventMap::ValueListIter iter = pHandlers->begin() ; iter != pHandlers->end() ;)
	{
		SystemEventHandlerPlusData handlerWithData = *iter ;
		iter++ ;

		SystemEventHandler handler = handlerWithData.m_Handler ;
		void* pUserData = handlerWithData.getUserData() ;

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
void Kernel::ReceivedUpdateEvent(smlUpdateEventId id, AnalyzeXML* pIncoming, ElementXML* pResponse)
{
	unused(pResponse) ;

	// Retrieve the event arguments
	smlRunFlags runFlags = (smlRunFlags)pIncoming->GetArgInt(sml_Names::kParamValue, 0) ;

	// Look up the handler(s) from the map
	UpdateEventMap::ValueList* pHandlers = m_UpdateEventMap.getList(id) ;

	if (!pHandlers)
		return ;

	// Go through the list of event handlers calling each in turn
	for (UpdateEventMap::ValueListIter iter = pHandlers->begin() ; iter != pHandlers->end() ;)
	{
		UpdateEventHandlerPlusData handlerWithData = *iter ;
		iter++ ;

		UpdateEventHandler handler = handlerWithData.m_Handler ;
		void* pUserData = handlerWithData.getUserData() ;

		// Call the handler
		handler(id, pUserData, this, runFlags) ;
	}
}

/*************************************************************
* @brief This function is called when an event is received
*		 from the Soar kernel.
*
* @param pIncoming	The event command
* @param pResponse	The reply (no real need to fill anything in here currently)
*************************************************************/
void Kernel::ReceivedStringEvent(smlStringEventId id, AnalyzeXML* pIncoming, ElementXML* pResponse)
{
	unused(pResponse) ;

	char const* pValue = pIncoming->GetArgString(sml_Names::kParamValue) ;

	// Look up the handler(s) from the map
	StringEventMap::ValueList* pHandlers = m_StringEventMap.getList(id) ;

	if (!pHandlers)
		return ;

	// Go through the list of event handlers calling each in turn
	for (StringEventMap::ValueListIter iter = pHandlers->begin() ; iter != pHandlers->end() ;)
	{
		StringEventHandlerPlusData handlerWithData = *iter ;
		iter++ ;

		StringEventHandler handler = handlerWithData.m_Handler ;
		void* pUserData = handlerWithData.getUserData() ;

		// Call the handler
		handler(id, pUserData, this, pValue) ;
	}
}

/*************************************************************
* @brief This function is called when an event is received
*		 from the Soar kernel.
*
* @param pIncoming	The event command
* @param pResponse	The reply (no real need to fill anything in here currently)
*************************************************************/
void Kernel::ReceivedAgentEvent(smlAgentEventId id, AnalyzeXML* pIncoming, ElementXML* pResponse)
{
	unused(pResponse) ;

	// Get the name of the agent this event refers to.
	char const* pAgentName = pIncoming->GetArgString(sml_Names::kParamName) ;

	// Look up the handler(s) from the map
	AgentEventMap::ValueList* pHandlers = m_AgentEventMap.getList(id) ;

	if (!pHandlers)
		return ;

	// See if we already have an Agent* for this agent.
	// We may not, because "agent created" events are included in the list that come here.
	Agent* pAgent = GetAgent(pAgentName) ;

	// Agent name can be null for some agent manager events
	if (!pAgent && pAgentName)
	{
		// Create a new client side agent object
		// We have to do this now because we'll be passing it back to the caller in a minute
		pAgent = MakeAgent(pAgentName) ;
	}

	// Go through the list of event handlers calling each in turn
	for (AgentEventMap::ValueListIter iter = pHandlers->begin() ; iter != pHandlers->end() ; iter++)
	{
		AgentEventHandlerPlusData handlerPlus = *iter ;
		AgentEventHandler handler = handlerPlus.m_Handler ;
		void* pUserData = handlerPlus.getUserData() ;

		// Call the handler
		handler(id, pUserData, pAgent) ;
	}
}

/*************************************************************
* @brief This function is called when an event is received
*		 from the Soar kernel.
*
* @param pIncoming	The event command
* @param pResponse	The reply (the result of executing this rhs function)
*************************************************************/
void Kernel::ReceivedRhsEvent(smlRhsEventId id, AnalyzeXML* pIncoming, ElementXML* pResponse)
{
	// Get the function name and the argument to the function
	// (We pass a single string but it could be parsed further to other values by the client)
	char const* pFunctionName = pIncoming->GetArgString(sml_Names::kParamFunction) ;
	char const* pArgument     = pIncoming->GetArgString(sml_Names::kParamValue) ;
	char const* pAgentName	  = pIncoming->GetArgString(sml_Names::kParamName) ;
	int maxLength			  = pIncoming->GetArgInt(sml_Names::kParamLength, 0) ;

	if (!pFunctionName || maxLength == 0)
	{
		// Should always include a function name
		SetError(Error::kInvalidArgument) ;
		return ;
	}

	// Look up the handler(s) from the map
	RhsEventMap::ValueList* pHandlers = m_RhsEventMap.getList(pFunctionName) ;

	if (!pHandlers)
		return ;

	// Look up the agent
	Agent* pAgent = pAgentName ? GetAgent(pAgentName) : NULL ;

	// Go through the list of event handlers calling each in turn...except
	// we only execute the first handler (registering multipler handlers for the same RHS function is not supported
	// because these functions return a value -- it wouldn't be clear which to use.  We could change this to call all
	// registered handlers and only use the first or last value returned.)
	RhsEventMap::ValueListIter iter = pHandlers->begin() ;

	if (iter == pHandlers->end())
		return ;

	RhsEventHandlerPlusData handlerWithData = *iter ;

	RhsEventHandler handler = handlerWithData.m_Handler ;
	void* pUserData = handlerWithData.getUserData() ;

	// Call the handler
	std::string result = handler(id, pUserData, pAgent, pFunctionName, pArgument) ;

	// If we got back a result then fill in the value in the response message.
	GetConnection()->AddSimpleResultToSMLResponse(pResponse, result.c_str()) ;
}

/*************************************************************
* @brief Creates a connection to the Soar kernel that is embedded
*        within the same process as the caller.
*
*		 Creating in "current thread" will produce maximum performance but requires a little more work for the developer
*		 (you need to call CheckForIncomingCommands() periodically and you should not register for events and then go to sleep).
*
*		 Creating in "new thread" is simpler for the developer but will be slower (around a factor 2).
*		 (It's simpler because there's no need to call CheckForIncomingCommands() periodically as this happens in a separate
*		  thread running inside the kernel and incoming events are handled by another thread in the client).
*
* @param pLibraryName	The name of the library to load, without an extension (e.g. "SoarKernelSML").  Case-sensitive (to support Linux).
*						This library will be dynamically loaded and connected to.
* @param Optimized		If this is a current thread connection, we can short-circuit parts of the messaging system for sending input and
*						running Soar.  If this flag is true we use those short cuts.  If you're trying to debug the SML libraries
*						you may wish to disable this option (so everything goes through the standard paths).  Not available if running in a new thread.
* @param port			The port number the kernel should use to receive remote connections.  The default port for SML is 12121 (picked at random).
*						Passing 0 means no listening port will be created (so it will be impossible to make remote connections to the kernel).
*
* @returns A new kernel object which is used to communicate with the kernel.
*		   If an error occurs a Kernel object is still returned.  Call "HadError()" and "GetLastErrorDescription()" on it.
*************************************************************/
Kernel* Kernel::CreateKernelInCurrentThread(char const* pLibraryName, bool optimized, int portToListenOn)
{
	return CreateEmbeddedConnection(pLibraryName, true, optimized, portToListenOn) ;
}

Kernel* Kernel::CreateKernelInNewThread(char const* pLibraryName, int portToListenOn)
{
	return CreateEmbeddedConnection(pLibraryName, false, false, portToListenOn) ;
}

/*************************************************************
* @brief Creates a connection to the Soar kernel that is embedded
*        within the same process as the caller.
*		 (This method is protected - clients should use the two methods above that are more explicitly named).
*
* @param pLibraryName	The name of the library to load, without an extension (e.g. "SoarKernelSML").  Case-sensitive (to support Linux).
*						This library will be dynamically loaded and connected to.
* @param ClientThread	If true, Soar will run in the client's thread and the client must periodically call over to the
*						kernel to check for incoming messages on remote sockets.
*						If false, Soar will run in a thread within the kernel and that thread will check the incoming sockets itself.
*						However, this kernel thread model requires a context switch whenever commands are sent to/from the kernel.
* @param Optimized		If this is a client thread connection, we can short-circuit parts of the messaging system for sending input and
*						running Soar.  If this flag is true we use those short cuts.  If you're trying to debug the SML libraries
*						you may wish to disable this option (so everything goes through the standard paths).  Has no affect if not running on client thread.
* @param port			The port number the kernel should use to receive remote connections.  The default port for SML is 12121 (picked at random).
*
* @returns A new kernel object which is used to communicate with the kernel.
*		   If an error occurs a Kernel object is still returned.  Call "HadError()" and "GetLastErrorDescription()" on it.
*************************************************************/
Kernel* Kernel::CreateEmbeddedConnection(char const* pLibraryName, bool clientThread, bool optimized, int portToListenOn)
{
	ErrorCode errorCode = 0 ;
	Connection* pConnection = Connection::CreateEmbeddedConnection(pLibraryName, clientThread, optimized, portToListenOn, &errorCode) ;

	// Even if pConnection is NULL, we still build a kernel object, so we have
	// a clean way to pass the error code back to the caller.
	Kernel* pKernel = new Kernel(pConnection) ;

	// Transfer any errors over to the kernel object, so the caller can retrieve them.
	pKernel->SetError(errorCode) ;

	// Register for "calls" from the kernel.
	if (pConnection)
	{
		pConnection->RegisterCallback(ReceivedCall, pKernel, sml_Names::kDocType_Call, true) ;

		pKernel->InitializeTimeTagCounter() ;

		pKernel->InitEvents() ;
	}

	return pKernel ;
}

/*************************************************************
* @brief Creates a connection to a receiver that is in a different
*        process.  The process can be on the same machine or a different machine.
*
* @param sharedFileSystem	If true the local and remote machines can access the same set of files.
*					For example, this means when loading a file of productions, sending the filename is
*					sufficient, without actually sending the contents of the file.
*					(NOTE: It may be a while before we really support passing in 'false' here)
* @param pIPaddress The IP address of the remote machine (e.g. "202.55.12.54").
*                   Pass "127.0.0.1" or NULL to create a connection between two processes on the same machine.
* @param port		The port number to connect to.  The default port for SML is 12121 (picked at random).
* @param ignoreOutput Setting this to true means output link changes won't be sent to this client (improving performance if you aren't interested in output)
*
* @returns A new kernel object which is used to communicate with the kernel (or NULL if an error occurs)
*************************************************************/
Kernel* Kernel::CreateRemoteConnection(bool sharedFileSystem, char const* pIPaddress, int port, bool ignoreOutput)
{
	ErrorCode errorCode = 0 ;

	// Initialize the socket library before attempting to create a connection
	sock::SocketLib* pLib = new sock::SocketLib() ;

	// Connect to the remote socket
	Connection* pConnection = Connection::CreateRemoteConnection(sharedFileSystem, pIPaddress, (unsigned short)port, &errorCode) ;

	// Even if pConnection is NULL, we still build a kernel object, so we have
	// a clean way to pass the error code back to the caller.
	Kernel* pKernel = new Kernel(pConnection) ;
	pKernel->SetSocketLib(pLib) ;
	pKernel->SetError(errorCode) ;

	// If this client isn't interested in getting output we can speed things up a bit for them.
	pKernel->m_bIgnoreOutput = ignoreOutput ;

	// If we had an error creating the connection, abort before
	// we try to get the current agent list
	if (pKernel->HadError())
		return pKernel ;

	// Register for "calls" from the kernel.
	pConnection->RegisterCallback(ReceivedCall, pKernel, sml_Names::kDocType_Call, true) ;

	// Register for important events
	pKernel->InitEvents() ;

	// Initialize our time tags
	pKernel->InitializeTimeTagCounter() ;

	// Get the current list of active agents
	pKernel->UpdateAgentList() ;

	return pKernel ;
}

/*************************************************************
* @brief Returns the number of agents (from our list of known agents).
*************************************************************/
int Kernel::GetNumberAgents()
{
	return m_AgentMap.size() ;
}

/*************************************************************
* @brief A client uses its own set of time tag values when it's
*		 talking to the kernel.  This is more efficient than using the
*		 kernel's timetags as that would require a timetag to be passed back
*		 to the client for each addition of a wme.
*
*		 However, we're now allowing multiple clients to modify the input
*		 link at the same time.  Because of this we need a way to ensure that
*		 two clients don't use the same client side time tag...or they'll get
*		 confused.  The current solution is to ask the kernel to assign a starting
*		 point for the time tag counter--so we can keep each client in a separate part
*		 of the time tag space (and a long still allows hundreds of millions of unique
*		 wmes for each client before they'll start to run into each other).
*************************************************************/
void Kernel::InitializeTimeTagCounter()
{
	AnalyzeXML response ;
	if (GetConnection()->SendAgentCommand(&response, sml_Names::kCommand_GetInitialTimeTag))
	{
		int initialTimeTag = response.GetResultInt(0) ;

		// Client side time tags are negative (to distinguish them from kernel side ones)
		assert(initialTimeTag <= 0) ;

		// Start time tags from this value
		m_TimeTagCounter = initialTimeTag ;

		// Start IDs from this value too, so they don't collide
		m_IdCounter = -initialTimeTag ;		
	}
}

/*************************************************************
* @brief Get the list of agents currently active in the kernel
*		 and create local Agent objects for each one (if we
*		 don't already have that agent registered).
*************************************************************/
void Kernel::UpdateAgentList()
{
	AnalyzeXML response ;
	if (GetConnection()->SendAgentCommand(&response, sml_Names::kCommand_GetAgentList))
	{
		ElementXML const* pResult = response.GetResultTag() ;
		ElementXML child(NULL) ;
		
		// Keep a record of the agents we find, so we can delete any that have been removed.
		std::list<Agent*>	inUse ;

		for (int i = 0 ; i < pResult->GetNumberChildren() ; i++)
		{
			pResult->GetChild(&child, i) ;

			// Look for the <name> tags
			if (child.IsTag(sml_Names::kTagName))
			{
				// Get the agent's name
				char const* pAgentName = child.GetCharacterData() ;

				// If we don't know about this agent already, then add it to our list.
				Agent* pAgent = m_AgentMap.find(pAgentName) ;

				if (!pAgent)
				{
					pAgent = MakeAgent(pAgentName) ;
				}

				inUse.push_back(pAgent) ;
			}
		}

		// Any agents that are in our map but not in the "inuse" list we should delete
		// as they no longer exist.
		m_AgentMap.keep(&inUse) ;
	}
}

/*************************************************************
* @brief Ask the kernel for the current list of connections
*		 and their status information.
*		 This is a snapshot which can then be interrogated through
*		 the other methods.
*************************************************************/
bool Kernel::GetAllConnectionInfo()
{
	std::list<ConnectionInfo*> previousList ;

	for (ConnectionListIter iter = m_ConnectionInfoList.begin() ; iter != m_ConnectionInfoList.end() ; iter++)
	{
		ConnectionInfo* pInfo = *iter ;
		previousList.push_back(pInfo) ;
	}
	m_ConnectionInfoList.clear() ;

	size_t previousConnectionCount = previousList.size() ;

	AnalyzeXML response ;
	if (GetConnection()->SendAgentCommand(&response, sml_Names::kCommand_GetConnections))
	{
		ElementXML const* pResult = response.GetResultTag() ;
		ElementXML child(NULL) ;
		
		for (int i = 0 ; i < pResult->GetNumberChildren() ; i++)
		{
			pResult->GetChild(&child, i) ;

			// Look for the <connection> tags
			if (child.IsTag(sml_Names::kTagConnection))
			{
				// Get the connection's id, name and status
				char const* pID     = child.GetAttribute(sml_Names::kConnectionId) ;
				char const* pName   = child.GetAttribute(sml_Names::kConnectionName) ;
				char const* pStatus = child.GetAttribute(sml_Names::kConnectionStatus) ;
				char const* pAgentStatus = child.GetAttribute(sml_Names::kAgentStatus) ;

				// If this info is on the previous list move it back to the current list
				bool foundMatch = false ;
				for (ConnectionListIter iter = previousList.begin() ; iter != previousList.end() ; iter++)
				{
					ConnectionInfo* pPrevInfo = *iter ;
					if (strcmp(pPrevInfo->GetID(), pID) == 0 &&
						strcmp(pPrevInfo->GetName(), pName) == 0 &&
						strcmp(pPrevInfo->GetConnectionStatus(), pStatus) == 0 &&
						strcmp(pPrevInfo->GetAgentStatus(), pAgentStatus) == 0)
					{
						m_ConnectionInfoList.push_back(pPrevInfo) ;
						previousList.erase(iter) ;
						foundMatch = true ;
						break ;
					}
				}

				if (!foundMatch)
				{
					ConnectionInfo* info = new ConnectionInfo(pID, pName, pStatus, pAgentStatus) ;
					m_ConnectionInfoList.push_back(info) ;
				}
			}
		}
	}

	// If we deleted all of the items from the previous list, then each connection we found matched
	// an existing one.
	if (previousList.size() == 0 && previousConnectionCount == m_ConnectionInfoList.size())
		m_ConnectionInfoChanged = false ;
	else
		m_ConnectionInfoChanged = true ;

	// Clean up any left over information
	for (ConnectionListIter iter = previousList.begin() ; iter != previousList.end() ; iter++)
	{
		ConnectionInfo* pInfo = *iter ;
		delete pInfo ;
	}

	return m_ConnectionInfoChanged ;
}

bool Kernel::HasConnectionInfoChanged()
{
	return m_ConnectionInfoChanged ;
}

int	Kernel::GetNumberConnections()
{
	return (int)m_ConnectionInfoList.size() ;
}

ConnectionInfo const* Kernel::GetConnectionInfo(int i)
{
	for (ConnectionListIter iter = m_ConnectionInfoList.begin() ; iter != m_ConnectionInfoList.end() ; iter++)
	{
		ConnectionInfo* pInfo = *iter ;
		if (i == 0)
			return pInfo ;
		i-- ;
	}

	return NULL ;
}

char const*	Kernel::GetConnectionStatus(char const* pName)
{
	for (ConnectionListIter iter = m_ConnectionInfoList.begin() ; iter != m_ConnectionInfoList.end() ; iter++)
	{
		ConnectionInfo* pInfo = *iter ;
		if (pInfo->GetName() && strcmp(pInfo->GetName(), pName) == 0)
			return pInfo->GetConnectionStatus() ;
	}

	return NULL ;
}

char const*	Kernel::GetAgentStatus(char const* pName)
{
	for (ConnectionListIter iter = m_ConnectionInfoList.begin() ; iter != m_ConnectionInfoList.end() ; iter++)
	{
		ConnectionInfo* pInfo = *iter ;
		if (pInfo->GetName() && strcmp(pInfo->GetName(), pName) == 0)
			return pInfo->GetAgentStatus() ;
	}

	return NULL ;
}

/*************************************************************
* @brief Sets the name and current status of this connection.
*		 This information is sent to the kernel and can be requested
*		 by other clients.
*************************************************************/
bool Kernel::SetConnectionInfo(char const* pName, char const* pConnectionStatus, char const* pAgentStatus)
{
	AnalyzeXML response ;
	bool ok = GetConnection()->SendAgentCommand(&response, sml_Names::kCommand_SetConnectionInfo, NULL, sml_Names::kConnectionName, pName, sml_Names::kConnectionStatus, pConnectionStatus, sml_Names::kAgentStatus, pAgentStatus) ;
	return ok ;
}

/*************************************************************
* @brief Looks up an agent by name (from our list of known agents).
*
* @returns A pointer to the agent (or NULL if not found).  This object
*		   is owned by the kernel and will be destroyed when the
*		   kernel is destroyed.
*************************************************************/
Agent* Kernel::GetAgent(char const* pAgentName)
{
	if (!pAgentName)
		return NULL ;

	return m_AgentMap.find(pAgentName) ;
}

/*************************************************************
* @brief Returns the n-th agent from our list of known agents.
*		 This is slower than GetAgent(pAgentName).
*************************************************************/
Agent* Kernel::GetAgentByIndex(int index)
{
	return m_AgentMap.getIndex(index) ;
}

/*************************************************************
* @brief Returns true if this agent pointer is still valid and
*		 can be used.
*************************************************************/
bool Kernel::IsAgentValid(Agent* pAgent)
{
	// We check the current list of agent pointers and see if this value is in that list
	// to determine if it is still valid.
	return m_AgentMap.contains(pAgent) ;
}

/*************************************************************
* @brief Creates a new Soar agent with the given name.
*
* @returns A pointer to the new agent structure.  This object
*		   is owned by the kernel and will be destroyed when the
*		   kernel is destroyed.
*************************************************************/
Agent* Kernel::CreateAgent(char const* pAgentName)
{
	AnalyzeXML response ;
	Agent* agent = NULL ;
	
	// See if this agent already exists
	agent = GetAgent(pAgentName) ;

	// If so, trying to create it fails.
	// (We could pass back agent, but that would hide this error from the client).
	if (agent)
	{
		SetError(Error::kAgentExists) ;
		return NULL ;
	}

	assert(GetConnection());
	if (GetConnection()->SendAgentCommand(&response, sml_Names::kCommand_CreateAgent, NULL, sml_Names::kParamName, pAgentName))
	{
		agent = MakeAgent(pAgentName) ;
	}

	// Set our error state based on what happened during this call.
	SetError(GetConnection()->GetLastError()) ;

	return agent ;
}

/*************************************************************
* @brief Creates a new Agent* object (not to be confused
*		 with actually creating a Soar agent -- see CreateAgent for that)
*************************************************************/
Agent* Kernel::MakeAgent(char const* pAgentName)
{
	if (!pAgentName)
		return NULL ;

	// If we already have an agent structure for this name just
	// return it.
	Agent* agent = GetAgent(pAgentName) ;

	if (agent)
		return agent ;

	// Make a new client side agent object
	agent = new Agent(this, pAgentName) ;

	// Record this in our list of agents
	m_AgentMap.add(agent->GetAgentName(), agent) ;

	// Register to get output link events.  These won't come back as standard events.
	// Instead we'll get "output" messages which are handled in a special manner.
	if (!m_bIgnoreOutput)
		RegisterForEventWithKernel(smlEVENT_OUTPUT_PHASE_CALLBACK, agent->GetAgentName()) ;

	return agent ;
}

/*************************************************************
* @brief Destroys an agent in the kernel (and locally).
*************************************************************/
bool Kernel::DestroyAgent(Agent* pAgent)
{
	AnalyzeXML response ;

	if (GetConnection()->SendAgentCommand(&response, sml_Names::kCommand_DestroyAgent, pAgent->GetAgentName()))
	{
		// Remove the object from our map and delete it.
		m_AgentMap.remove(pAgent->GetAgentName(), true) ;
		return true ;
	}

	return false ;
}

/*************************************************************
* @brief If filtering is disabled, that means all commands
*		 sent from this client will not be filtered (sent to
*		 external processes that have registered a filter).
*		 The default is that filtering is enabled.
*************************************************************/
void Kernel::EnableFiltering(bool state)
{
	m_FilteringEnabled = state ;
}

/*************************************************************
* @brief Process a command line command
*
* @param pCommandLine Command line string to process.
* @param pAgentName   Agent name to apply the command line to (can be NULL)
* @param echoResults  If true the results are also echoed through the smlEVENT_ECHO event, so they can appear in a debugger (or other listener)
* @param noFilter	  If true this command line by-passes any external filters that have been registered (this is not common) and is executed immediately.
* @returns The string form of output from the command.
*************************************************************/
char const* Kernel::ExecuteCommandLine(char const* pCommandLine, char const* pAgentName, bool echoResults, bool noFilter)
{
	AnalyzeXML response;
	bool wantRawOutput = true ;

	// Send the command line to the kernel
	m_CommandLineSucceeded = GetConnection()->SendAgentCommand(&response,
		sml_Names::kCommand_CommandLine, pAgentName,
		sml_Names::kParamLine, pCommandLine,
		sml_Names::kParamEcho, echoResults ? sml_Names::kTrue : sml_Names::kFalse,
		sml_Names::kParamNoFiltering, !m_FilteringEnabled || noFilter ? sml_Names::kTrue : sml_Names::kFalse,
		wantRawOutput);

	if (m_CommandLineSucceeded)
	{
		// Get the result as a string
		char const *pResult = response.GetResultString();
		m_CommandLineResult = (pResult == NULL)? "" : pResult ;
	}
	else
	{
		// Get the error message
		m_CommandLineResult = "\nError: ";
		if (response.GetErrorTag()) {
			m_CommandLineResult += response.GetErrorTag()->GetCharacterData();
		} else {
			m_CommandLineResult += "<No error message returned by command>";
		}
	}

	return m_CommandLineResult.c_str();
}

/*************************************************************
* @brief Execute a command line command and return the result
*		 as an XML object.
*
* @param pCommandLine Command line string to process.
* @param pAgentName Agent name to apply the command line to.
* @param pResponse The XML returned by the agent for this command.
*        The caller should pass in an AnalyzeXML object which is then
*		 filled out by the agent during the call.
* @returns True if the command succeeds.
*************************************************************/
bool Kernel::ExecuteCommandLineXML(char const* pCommandLine, char const* pAgentName, ClientAnalyzedXML* pResponse)
{
	if (!pCommandLine || !pResponse)
		return false ;

	m_CommandLineSucceeded = GetConnection()->SendAgentCommand(pResponse->GetAnalyzeXML(), sml_Names::kCommand_CommandLine, pAgentName,
							sml_Names::kParamLine, pCommandLine, sml_Names::kParamNoFiltering, sml_Names::kTrue);

	return m_CommandLineSucceeded ;
}

/*************************************************************
* @brief Calls Commit() for all agents -- sending any queued I/O operations
*		 over to the kernel for processing.
*************************************************************/
void Kernel::CommitAll()
{
	int numberAgents = GetNumberAgents() ;

	for (int i = 0 ; i < numberAgents ; i++)
	{
		Agent* pAgent = GetAgentByIndex(i) ;
		pAgent->Commit() ;
	}
}

/*************************************************************
* @brief Returns true if at least one agent has uncommitted changes.
*************************************************************/
bool Kernel::IsCommitRequired()
{
	int numberAgents = GetNumberAgents() ;

	for (int i = 0 ; i < numberAgents ; i++)
	{
		Agent* pAgent = GetAgentByIndex(i) ;
		if (pAgent->GetWM()->IsCommitRequired())
			return true ;
	}

	return false ;
}

/*************************************************************
* @brief   Run Soar for the specified number of decisions
*
* This command will currently run all agents.
*
* @returns The result of executing the run command.
*		   The output from during the run is sent to a different callback.
*************************************************************/
char const* Kernel::RunAllAgents(unsigned long numberSteps, smlRunStepSize stepSize, smlInterleaveStepSize interleaveStepSize)
{
	if (IsCommitRequired())
	{
		assert(false) ;
		return "Need to commit changes before calling a run method" ;
	}

#ifdef SML_DIRECT
		if (GetConnection()->IsDirectConnection())
		{
			((EmbeddedConnection*)GetConnection())->DirectRun(NULL, false, stepSize, interleaveStepSize, (int)numberSteps) ;
			return "DirectRun completed" ;
		}
#endif

	// Convert int to a string
	std::ostringstream ostr ;
	ostr << numberSteps ;

	// Create the command line for the run command
	std::string step ;
	
	switch (stepSize)
	{
		case sml_DECISION:		step = "-d" ; break ;
		case sml_PHASE:			step = "-p" ; break ;
		case sml_ELABORATION:	step = "-e" ; break ;
		case sml_UNTIL_OUTPUT:	step = "-o" ; break ;
		default: return "Unrecognized step size parameter passed to RunAllAgents" ;
	}

	std::string interleave ;

	switch (interleaveStepSize)
	{
		case sml_INTERLEAVE_ELABORATION:	interleave = "-i e" ; break ;
		case sml_INTERLEAVE_PHASE:			interleave = "-i p" ; break ;
		case sml_INTERLEAVE_DECISION:		interleave = "-i d" ; break ;
		case sml_INTERLEAVE_UNTIL_OUTPUT:	interleave = "-i o" ; break ;
		default: return "Unrecognized interleave size parameter passed to RunAllAgents" ;
	}

	std::string cmd = "run " + step + " " + interleave + " " + ostr.str() ;

	// The command line currently requires an agent in order
	// to execute a run command, so we provide one (which one should make no difference).
	if (GetNumberAgents() == 0)
		return "There are no agents to run" ;

	Agent* pFirstAgent = GetAgentByIndex(0) ;

	// Execute the run command.
	char const* pResult = ExecuteCommandLine(cmd.c_str(), pFirstAgent->GetAgentName()) ;
	return pResult ;
}

char const* Kernel::RunAllAgentsForever(smlInterleaveStepSize interleaveStepSize)
{
	if (IsCommitRequired())
	{
		assert(false) ;
		return "Need to commit changes before calling a run method" ;
	}

#ifdef SML_DIRECT
		if (GetConnection()->IsDirectConnection())
		{
			((EmbeddedConnection*)GetConnection())->DirectRun(NULL, true, sml_DECISION, interleaveStepSize, 1) ;
			return "DirectRun completed" ;
		}
#endif

	std::string interleave ;

	switch (interleaveStepSize)
	{
		case sml_INTERLEAVE_ELABORATION:	interleave = "-i e" ; break ;
		case sml_INTERLEAVE_PHASE:			interleave = "-i p" ; break ;
		case sml_INTERLEAVE_DECISION:		interleave = "-i d" ; break ;
		case sml_INTERLEAVE_UNTIL_OUTPUT:	interleave = "-i o" ; break ;
		default: return "Unrecognized interleave size parameter passed to RunAllAgents" ;
	}

	std::string cmd = "run " + interleave ;

	// The command line currently requires an agent in order
	// to execute a run command, so we provide one (which one should make no difference).
	if (GetNumberAgents() == 0)
		return "There are no agents to run" ;

	Agent* pFirstAgent = GetAgentByIndex(0) ;

	// Execute the run command.
	char const* pResult = ExecuteCommandLine(cmd.c_str(), pFirstAgent->GetAgentName()) ;
	return pResult ;
}

/*************************************************************
* @brief   Run Soar until either output is generated or
*		   the maximum number of decisions is reached.  This is done
*		   on an agent by agent basis, so the entire system stops only
*		   after each agent has generated output (or has reached the decision limit).
*
* This function also calls "ClearOutputLinkChanges" so methods
* like "IsJustAdded" will refer to the changes that occur as a result of
* this run.
*
* We don't generally want Soar to just run until it generates
* output without any limit as an error in the AI logic might cause
* it to never return control to the environment, so there is a maximum
* decision count (currently 15) and if an agent fails to produce output
* before then that agent will stop running.  (This value can be changed with the
* max-nil-output-cycles command).
*************************************************************/
char const* Kernel::RunAllTilOutput(smlInterleaveStepSize interleaveStepSize)
{
	if (IsCommitRequired())
	{
		assert(false) ;
		return "Need to commit changes before calling a run method" ;
	}

#ifdef SML_DIRECT
		if (GetConnection()->IsDirectConnection())
		{
			((EmbeddedConnection*)GetConnection())->DirectRun(NULL, false, sml_UNTIL_OUTPUT, interleaveStepSize, 1) ;
			return "DirectRun completed" ;
		}
#endif

	std::string interleave ;

	switch (interleaveStepSize)
	{
		case sml_INTERLEAVE_ELABORATION:	interleave = "-i e" ; break ;
		case sml_INTERLEAVE_PHASE:			interleave = "-i p" ; break ;
		case sml_INTERLEAVE_DECISION:		interleave = "-i d" ; break ;
		case sml_INTERLEAVE_UNTIL_OUTPUT:	interleave = "-i o" ; break ;
		default: return "Unrecognized interleave size parameter passed to RunAllAgents" ;
	}

	// Run all agents until each has generated output.  Each agent will stop at the point
	// it has generated output, so they may run for different numbers of decisions.
	// For now, maxDecisions is being ignored.  We should make this a separate call
	// to set this parameter.
	std::string cmd = "run --output " + interleave ;

	// The command line currently requires an agent in order
	// to execute a run command, so we provide one (which one should make no difference).
	if (GetNumberAgents() == 0)
		return "There are no agents to run" ;

	Agent* pFirstAgent = GetAgentByIndex(0) ;

	// Execute the run command.
	char const* pResult = ExecuteCommandLine(cmd.c_str(), pFirstAgent->GetAgentName()) ;
	return pResult ;
}

/*************************************************************
* @brief Interrupt the currently running Soar agent.
*
* Call this after calling "Run" in order to stop all Soar agents.
* The usual way to do this is to register for an event (e.g. AFTER_DECISION_CYCLE)
* and in that event handler decide if the user wishes to stop soar.
* If so, call to this method inside that handler.
*
* The request to Stop may not be honored immediately.
* Soar will stop at the next point it is considered safe to do so.
*************************************************************/
char const* Kernel::StopAllAgents()
{
	std::string cmd = "stop-soar" ;

	// The command line currently requires an agent in order
	// to execute a stop-soar command, so we provide one (which one should make no difference).
	if (GetNumberAgents() == 0)
		return "There are no agents to stop" ;

	Agent* pFirstAgent = GetAgentByIndex(0) ;

	// Execute the command.
	char const* pResult = ExecuteCommandLine(cmd.c_str(), pFirstAgent->GetAgentName()) ;
	return pResult ;
}

/*************************************************************
* @brief Returns true if one or more agents are currently running.
*
* Can be used in conjunction with StopAllAgents() to make sure
* all agents have actually terminated their runs.
*************************************************************/
bool Kernel::IsSoarRunning()
{
	AnalyzeXML response ;

	bool ok = (GetConnection()->SendAgentCommand(&response, sml_Names::kCommand_IsSoarRunning)) ;

	if (ok)
	{
		return response.GetResultBool(false) ;
	}

	return ok ;
}

/*************************************************************
* @brief   Causes the kernel to issue a SYSTEM_START event.
*
*		   The expectation is that a simulation will be listening
*		   for this event and on receiving this event it will start
*		   running the Soar agents.
*
*		   Thus calling this method will generally lead to Soar running
*		   but indirectly through a simulation.
*************************************************************/
bool Kernel::FireStartSystemEvent()
{
	AnalyzeXML response ;

	bool ok = GetConnection()->SendAgentCommand(&response, sml_Names::kCommand_FireEvent, NULL, sml_Names::kParamEventID, m_pEventMap->ConvertToString(smlEVENT_SYSTEM_START)) ;
	return ok ;
}

/*************************************************************
* @brief   Causes the kernel to issue a SYSTEM_STOP event.
*
*		   A running simulation should listen for this event
*		   and stop running Soar when this event is fired.
*
*		   NOTE: Calling "StopAllAgents()" also fires this
*		   event, so it's not clear there's ever a need to fire
*		   this event independently.  It's included in the API
*		   for completeness in case we find a use or change the
*		   semantics for "stop-soar".
*************************************************************/
bool Kernel::FireStopSystemEvent()
{
	AnalyzeXML response ;

	bool ok = GetConnection()->SendAgentCommand(&response, sml_Names::kCommand_FireEvent, NULL, sml_Names::kParamEventID, m_pEventMap->ConvertToString(smlEVENT_SYSTEM_STOP)) ;
	return ok ;
}

/*************************************************************
* @brief   Prevents the kernel from sending an smlEVENT_SYSTEM_STOP
*		   event at the of a run.
*
*		   A simulation may issue a series of short run commands to
*		   Soar that to the user looks like a single continues run.
*		   In that case, they should suppress the system stop event
*		   to prevent other tools from thinking the entire system
*		   has terminated.
*
*		   When the simulation's run completes, it can then manually
*		   request that the kernel fire the event.
*************************************************************/
bool Kernel::SuppressSystemStop(bool state)
{
	AnalyzeXML response ;

	bool ok = GetConnection()->SendAgentCommand(&response, sml_Names::kCommand_SuppressEvent, NULL, sml_Names::kParamEventID, m_pEventMap->ConvertToString(smlEVENT_SYSTEM_STOP), sml_Names::kParamValue, state ? sml_Names::kTrue : sml_Names::kFalse) ;
	return ok ;
}

/*************************************************************
* @brief Takes a command line and expands all the aliases within
*		 it without executing it.
*
*		 This can be useful when trying to determine what a command
*		 is about to do.  It does NOT need to be called before
*		 calling ExecuteCommandLine() as that expands aliases automatically.
*
* @param pCommandLine Command line string to process.
* @returns The expanded command line.
*************************************************************/
char const* Kernel::ExpandCommandLine(char const* pCommandLine)
{
	AnalyzeXML response;

	// Send the command line to the kernel
	m_CommandLineSucceeded = GetConnection()->SendAgentCommand(&response, sml_Names::kCommand_ExpandCommandLine, NULL, sml_Names::kParamLine, pCommandLine);

	if (m_CommandLineSucceeded)
	{
		// Get the result as a string
		char const *pResult = response.GetResultString();
		m_CommandLineResult = (pResult == NULL)? "<null>" : pResult ;
	}
	else
	{
		// Get the error message
		m_CommandLineResult = "\nError: ";
		if (response.GetErrorTag()) {
			m_CommandLineResult += response.GetErrorTag()->GetCharacterData();
		} else {
			m_CommandLineResult += "Unknown error.";
		}
	}

	return m_CommandLineResult.c_str();
}

/*************************************************************
* @brief Returns true if this command line is a run command
*************************************************************/
bool Kernel::IsRunCommand(char const* pCommandLine)
{
	// Expand any aliases
	char const* pExpandedLine = ExpandCommandLine(pCommandLine) ;

	if (!pExpandedLine)
		return false ;

	std::string line = pExpandedLine ;

	// We might have a list of these one day but after alias expansion right
	// now it's just one command that should start "run xxx"
	std::string runCommand = "run" ;

	if (line.size() < runCommand.size())
		return false ;

	// Check if the expanded command line starts with "run" or not.
	// (Technically this could be wrong if we added a runxxx command but that's not the case now and
	//  not likely in the future).
	if (line.substr(0, runCommand.size()).compare(runCommand) == 0)
		return true ;

	// We also have a few compound commands ("e.g. time x") which will also cause Soar to run.
	// These may be harder to handle correctly.  For now I'll handle them here but if they become
	// complex we should expose support for recognizing these from inside the command line module
	// in the kernel (much like the way ExpandCommandLine works) as the command line has all of this logic built in.
	std::string compounds[] = { "time", "command-to-file" } ;
	std::string spaceRunCommand = " run" ;	// Leading space

	for (int i = 0 ; i < 2 ; i++)
	{
		if (line.size() < compounds[i].size() + spaceRunCommand.size())
			continue ;

		// If starts with the compound command
		if (line.substr(0, compounds[i].size()).compare(compounds[i]) == 0)
		{
			// Look for " run" anywhere in the remaining command
			// to simplify the problems caused by other arguments and white space may cause
			if (line.find(spaceRunCommand) != std::string::npos)
				return true ;
		}
	}

	return false ;
}

/*************************************************************
* @brief Returns true if this command line is a stop command
*************************************************************/
bool Kernel::IsStopCommand(char const* pCommandLine)
{
	// Expand any aliases
	char const* pExpandedLine = ExpandCommandLine(pCommandLine) ;

	if (!pExpandedLine)
		return false ;

	std::string line = pExpandedLine ;

	// We might have a list of these one day but after alias expansion right
	// now it's just one command that should start "stop-soar xxx"
	std::string stopCommand = "stop-soar" ;

	if (line.size() < stopCommand.size())
		return false ;

	// Check if the expanded command line starts with "stop-soar" or not.
	if (line.substr(0, stopCommand.size()).compare(stopCommand) == 0)
		return true ;

	return false ;
}

/*************************************************************
* @brief Get last command line result
*
* @returns True if the last command line call succeeded.
*************************************************************/
bool Kernel::GetLastCommandLineResult()
{
	return m_CommandLineSucceeded ;
}

/*************************************************************
* @brief Get the current value of the "set-library-location" path variable.
*
* This points to the location where the kernelSML library was loaded
* (unless it has been changed since the load).
*************************************************************/
std::string Kernel::GetLibraryLocation()
{
	ClientAnalyzedXML response ;

	std::string cmd = "set-library-location" ;	// Without params this does a get

	bool ok = ExecuteCommandLineXML(cmd.c_str(), NULL, &response) ;

	if (!ok)
		return "" ;

	std::string path = response.GetArgString(sml_Names::kParamDirectory) ;
	return path ;
}

/*************************************************************
* @brief If this is an embedded connection using "synchronous execution"
*		 then we need to call this periodically to look for commands
*		 coming in from remote sockets.
*		 If this is a remote connection or an embedded connection
*		 with asynch execution, commands are executed on a different
*		 thread inside the kernel, so that thread checks for these
*		 incoming commands automatically (without the client
*		 having to call this).
*************************************************************/
bool Kernel::CheckForIncomingCommands()
{
	AnalyzeXML response ;
	if (GetConnection()->SendAgentCommand(&response, sml_Names::kCommand_CheckForIncomingCommands))
	{
		return response.GetResultBool(false) ;
	}

	return false ;
}

/*************************************************************
* @brief See if there are any incoming messages waiting to
*		 be dispatched to event handlers and if so process them.
*
*		 This call is intended to be used by remote connections that
*		 have shut down the event thread and are taking over event handling directly.
*		 (A common reason is to ensure the callbacks from events are on the main
*		  execution thread of the caller).
*
*		 This is very similar to CheckForIncomingCommands()
*		 but doesn't pass a command over to the kernel.
*
* @return True if processes at least one event
*************************************************************/
bool Kernel::CheckForIncomingEvents()
{
	return (GetConnection()->ReceiveMessages(true)) ;
}

/*************************************************************
* @brief This is a utility wrapper to let us sleep the entire client process
*		 for a period of time.
*************************************************************/
void Kernel::Sleep(long milliseconds)
{
	soar_thread::Thread::SleepStatic(0, milliseconds) ;
}

/*************************************************************
* @brief Register for a particular event at the kernel
*************************************************************/
void Kernel::RegisterForEventWithKernel(int id, char const* pAgentName)
{
	AnalyzeXML response ;

	char const* pEvent = m_pEventMap->ConvertToString(id) ;

	// Send the register command
	GetConnection()->SendAgentCommand(&response, sml_Names::kCommand_RegisterForEvent, pAgentName, sml_Names::kParamEventID, pEvent) ;
}

/*************************************************************
* @brief Unregister for a particular event at the kernel
*************************************************************/
void Kernel::UnregisterForEventWithKernel(int id, char const* pAgentName)
{
	AnalyzeXML response ;

	char const* pEvent = m_pEventMap->ConvertToString(id) ;

	// Send the unregister command
	GetConnection()->SendAgentCommand(&response, sml_Names::kCommand_UnregisterForEvent, pAgentName, sml_Names::kParamEventID, pEvent) ;
}

class Kernel::TestSystemCallback : public SystemEventMap::ValueTest
{
private:
	int m_ID ;
public:
	TestSystemCallback(int id) { m_ID = id ; }

	bool isEqual(SystemEventHandlerPlusData handler)
	{
		return handler.m_CallbackID == m_ID ;
	}
} ;

class Kernel::TestUpdateCallback : public UpdateEventMap::ValueTest
{
private:
	int m_ID ;
public:
	TestUpdateCallback(int id) { m_ID = id ; }

	bool isEqual(UpdateEventHandlerPlusData handler)
	{
		return handler.m_CallbackID == m_ID ;
	}
} ;

class Kernel::TestStringCallback : public StringEventMap::ValueTest
{
private:
	int m_ID ;
public:
	TestStringCallback(int id) { m_ID = id ; }

	bool isEqual(StringEventHandlerPlusData handler)
	{
		return handler.m_CallbackID == m_ID ;
	}
} ;

class Kernel::TestStringCallbackFull : public StringEventMap::ValueTest
{
private:
	int				m_EventID ;
	StringEventHandler m_Handler ;
	void*			m_UserData ;

public:
	TestStringCallbackFull(int id, StringEventHandler handler, void* pUserData)
	{ m_EventID = id ; m_Handler = handler ; m_UserData = pUserData ; }

	bool isEqual(StringEventHandlerPlusData handlerPlus)
	{
		return handlerPlus.m_EventID == m_EventID &&
			   handlerPlus.m_Handler == m_Handler &&
			   handlerPlus.m_UserData == m_UserData ;
	}
} ;

class Kernel::TestSystemCallbackFull : public SystemEventMap::ValueTest
{
private:
	int				m_EventID ;
	SystemEventHandler m_Handler ;
	void*			m_UserData ;

public:
	TestSystemCallbackFull(int id, SystemEventHandler handler, void* pUserData)
	{ m_EventID = id ; m_Handler = handler ; m_UserData = pUserData ; }

	bool isEqual(SystemEventHandlerPlusData handlerPlus)
	{
		return handlerPlus.m_EventID == m_EventID &&
			   handlerPlus.m_Handler == m_Handler &&
			   handlerPlus.m_UserData == m_UserData ;
	}
} ;

class Kernel::TestUpdateCallbackFull : public UpdateEventMap::ValueTest
{
private:
	int				m_EventID ;
	UpdateEventHandler m_Handler ;
	void*			m_UserData ;

public:
	TestUpdateCallbackFull(int id, UpdateEventHandler handler, void* pUserData)
	{ m_EventID = id ; m_Handler = handler ; m_UserData = pUserData ; }

	bool isEqual(UpdateEventHandlerPlusData handlerPlus)
	{
		return handlerPlus.m_EventID == m_EventID &&
			   handlerPlus.m_Handler == m_Handler &&
			   handlerPlus.m_UserData == m_UserData ;
	}
} ;

class Kernel::TestAgentCallback : public AgentEventMap::ValueTest
{
private:
	int m_ID ;
public:
	TestAgentCallback(int id) { m_ID = id ; }

	bool isEqual(AgentEventHandlerPlusData handler)
	{
		return handler.m_CallbackID == m_ID ;
	}
} ;

class Kernel::TestAgentCallbackFull : public AgentEventMap::ValueTest
{
private:
	int				m_EventID ;
	AgentEventHandler m_Handler ;
	void*			m_UserData ;

public:
	TestAgentCallbackFull(int id, AgentEventHandler handler, void* pUserData)
	{ m_EventID = id ; m_Handler = handler ; m_UserData = pUserData ; }

	bool isEqual(AgentEventHandlerPlusData handlerPlus)
	{
		return handlerPlus.m_EventID == m_EventID &&
			   handlerPlus.m_Handler == m_Handler &&
			   handlerPlus.m_UserData == m_UserData ;
	}
} ;

class Kernel::TestRhsCallback : public RhsEventMap::ValueTest
{
private:
	int m_ID ;
public:
	TestRhsCallback(int id) { m_ID = id ; }

	bool isEqual(RhsEventHandlerPlusData handler)
	{
		return handler.m_CallbackID == m_ID ;
	}
} ;

class Kernel::TestRhsCallbackFull : public RhsEventMap::ValueTest
{
private:
	int				m_EventID ;
	std::string		m_FunctionName ;
	RhsEventHandler m_Handler ;
	void*			m_UserData ;

public:
	TestRhsCallbackFull(int eventID, char const* functionName, RhsEventHandler handler, void* pUserData)
	{ m_EventID = eventID ; m_FunctionName = functionName ; m_Handler = handler ; m_UserData = pUserData ; }

	virtual ~TestRhsCallbackFull() { } ;

	bool isEqual(RhsEventHandlerPlusData handlerPlus)
	{
		return handlerPlus.m_FunctionName.compare(m_FunctionName) == 0 &&
			   handlerPlus.m_EventID == m_EventID &&
			   handlerPlus.m_Handler == m_Handler &&
			   handlerPlus.m_UserData == m_UserData ;
	}
} ;

/*************************************************************
* @brief Register for a "SystemEvent".
*		 Multiple handlers can be registered for the same event.
* @param smlEventId		The event we're interested in (see the list below for valid values)
* @param handler		A function that will be called when the event happens
* @param pUserData		Arbitrary data that will be passed back to the handler function when the event happens.
* @param addToBack		If true add this handler is called after existing handlers.  If false, called before existing handlers.
*
* Current set is:
* smlEVENT_BEFORE_SHUTDOWN,
* smlEVENT_AFTER_CONNECTION_LOST,
* smlEVENT_BEFORE_RESTART,
* smlEVENT_AFTER_RESTART,
* smlEVENT_BEFORE_RHS_FUNCTION_ADDED,
* smlEVENT_AFTER_RHS_FUNCTION_ADDED,
* smlEVENT_BEFORE_RHS_FUNCTION_REMOVED,
* smlEVENT_AFTER_RHS_FUNCTION_REMOVED,
* smlEVENT_BEFORE_RHS_FUNCTION_EXECUTED,
* smlEVENT_AFTER_RHS_FUNCTION_EXECUTED,
*
* @returns Unique ID for this callback.  Required when unregistering this callback.
*************************************************************/
int Kernel::RegisterForSystemEvent(smlSystemEventId id, SystemEventHandler handler, void* pUserData, bool addToBack)
{
	// Start by checking if this id, handler, pUSerData combination has already been registered
	TestSystemCallbackFull test(id, handler, pUserData) ;

	// See if this handler is already registered
	SystemEventHandlerPlusData plus(0,0,0,0) ;
	bool found = m_SystemEventMap.findFirstValueByTest(&test, &plus) ;

	if (found && plus.m_Handler != 0)
		return plus.getCallbackID() ;

	// If we have no handlers registered with the kernel, then we need
	// to register for this event.  No need to do this multiple times.
	if (m_SystemEventMap.getListSize(id) == 0)
	{
		RegisterForEventWithKernel(id, NULL) ;
	}

	// Record the handler
	// We use a struct rather than a pointer to a struct, so there's no need to new/delete
	// everything as the objects are added and deleted.
	m_CallbackIDCounter++ ;

	SystemEventHandlerPlusData handlerPlus(id, handler, pUserData, m_CallbackIDCounter) ;
	m_SystemEventMap.add(id, handlerPlus, addToBack) ;

	// Return the ID.  We use this later to unregister the callback
	return m_CallbackIDCounter ;
}

/*************************************************************
* @brief Register for an "UpdateEvent".
*		 Multiple handlers can be registered for the same event.
* @param smlEventId		The event we're interested in (see the list below for valid values)
* @param handler		A function that will be called when the event happens
* @param pUserData		Arbitrary data that will be passed back to the handler function when the event happens.
* @param addToBack		If true add this handler is called after existing handlers.  If false, called before existing handlers.
*
* This event is registered with the kernel because they relate to events we think may be useful to use to trigger updates
* in synchronous environments.
*
* @returns A unique ID for this callback (used to unregister the callback later) 
*************************************************************/
int	Kernel::RegisterForUpdateEvent(smlUpdateEventId id, UpdateEventHandler handler, void* pUserData, bool addToBack)
{
	// Start by checking if this id, handler, pUSerData combination has already been registered
	TestUpdateCallbackFull test(id, handler, pUserData) ;

	// See if this handler is already registered
	UpdateEventHandlerPlusData plus(0,0,0,0) ;
	bool found = m_UpdateEventMap.findFirstValueByTest(&test, &plus) ;

	if (found && plus.m_Handler != 0)
		return plus.getCallbackID() ;

	// If we have no handlers registered with the kernel, then we need
	// to register for this event.  No need to do this multiple times.
	if (m_UpdateEventMap.getListSize(id) == 0)
	{
		RegisterForEventWithKernel(id, NULL) ;
	}

	// Record the handler
	// We use a struct rather than a pointer to a struct, so there's no need to new/delete
	// everything as the objects are added and deleted.
	m_CallbackIDCounter++ ;

	UpdateEventHandlerPlusData handlerPlus(id, handler, pUserData, m_CallbackIDCounter) ;
	m_UpdateEventMap.add(id, handlerPlus, addToBack) ;

	// Return the ID.  We use this later to unregister the callback
	return m_CallbackIDCounter ;
}

/*************************************************************
* @brief Register for an "StringEvent".
*		 Multiple handlers can be registered for the same event.
* @param smlEventId		The event we're interested in (see the list below for valid values)
* @param handler		A function that will be called when the event happens
* @param pUserData		Arbitrary data that will be passed back to the handler function when the event happens.
* @param addToBack		If true add this handler is called after existing handlers.  If false, called before existing handlers.
*
* This event is registered with the kernel because they relate to events we think may be useful to use to trigger updates
* in synchronous environments.
*
* @returns A unique ID for this callback (used to unregister the callback later) 
*************************************************************/
int	Kernel::RegisterForStringEvent(smlStringEventId id, StringEventHandler handler, void* pUserData, bool addToBack)
{
	// Start by checking if this id, handler, pUserData combination has already been registered
	TestStringCallbackFull test(id, handler, pUserData) ;

	// See if this handler is already registered
	StringEventHandlerPlusData plus(0,0,0,0) ;
	bool found = m_StringEventMap.findFirstValueByTest(&test, &plus) ;

	if (found && plus.m_Handler != 0)
		return plus.getCallbackID() ;

	// If we have no handlers registered with the kernel, then we need
	// to register for this event.  No need to do this multiple times.
	if (m_StringEventMap.getListSize(id) == 0)
	{
		RegisterForEventWithKernel(id, NULL) ;
	}

	// Record the handler
	// We use a struct rather than a pointer to a struct, so there's no need to new/delete
	// everything as the objects are added and deleted.
	m_CallbackIDCounter++ ;

	StringEventHandlerPlusData handlerPlus(id, handler, pUserData, m_CallbackIDCounter) ;
	m_StringEventMap.add(id, handlerPlus, addToBack) ;

	// Return the ID.  We use this later to unregister the callback
	return m_CallbackIDCounter ;
}

/*************************************************************
* @brief Register for an "AgentEvent".
*		 Multiple handlers can be registered for the same event.
* @param smlEventId		The event we're interested in (see the list below for valid values)
* @param handler		A function that will be called when the event happens
* @param pUserData		Arbitrary data that will be passed back to the handler function when the event happens.
* @param addToBack		If true add this handler is called after existing handlers.  If false, called before existing handlers.
*
* Current set is:
* // Agent manager
* smlEVENT_AFTER_AGENT_CREATED,
* smlEVENT_BEFORE_AGENT_DESTROYED,
* smlEVENT_BEFORE_AGENT_REINITIALIZED,
* smlEVENT_AFTER_AGENT_REINITIALIZED,
*
* @returns A unique ID for this callback (used to unregister the callback later) 
*************************************************************/
int Kernel::RegisterForAgentEvent(smlAgentEventId id, AgentEventHandler handler, void* pUserData, bool addToBack)
{
	// Start by checking if this id, handler, pUSerData combination has already been registered
	TestAgentCallbackFull test(id, handler, pUserData) ;

	// See if this handler is already registered
	AgentEventHandlerPlusData plus(0,0,0,0) ;
	bool found = m_AgentEventMap.findFirstValueByTest(&test, &plus) ;

	if (found && plus.m_Handler != 0)
		return plus.getCallbackID() ;

	// If we have no handlers registered with the kernel, then we need
	// to register for this event.  No need to do this multiple times.
	if (m_AgentEventMap.getListSize(id) == 0)
	{
		RegisterForEventWithKernel(id, NULL) ;
	}

	// Record the handler
	m_CallbackIDCounter++ ;
	AgentEventHandlerPlusData handlerPlus(id, handler, pUserData, m_CallbackIDCounter) ;
	m_AgentEventMap.add(id, handlerPlus, addToBack) ;

	// Return the ID.  We use this later to unregister the callback
	return m_CallbackIDCounter ;
}

/***
***   RHS functions and message event handlers use the same internal logic, although they look rather different to the user
***/
int	Kernel::InternalAddRhsFunction(smlRhsEventId id, char const* pRhsFunctionName, RhsEventHandler handler, void* pUserData, bool addToBack)
{
	// Start by checking if this functionName, handler, pUSerData combination has already been registered
	TestRhsCallbackFull test(id, pRhsFunctionName, handler, pUserData) ;

	// See if this handler is already registered
	RhsEventHandlerPlusData plus(0,0,0,0,0) ;
	bool found = m_RhsEventMap.findFirstValueByTest(&test, &plus) ;

	if (found && plus.m_Handler != 0)
		return plus.getCallbackID() ;

	// If we have no handlers registered with the kernel, then we need
	// to register for this event.  No need to do this multiple times.
	if (m_RhsEventMap.getListSize(pRhsFunctionName) == 0)
	{
		AnalyzeXML response ;

		// The event ID is always the same for RHS functions
		char const* pEvent = m_pEventMap->ConvertToString(id) ;

		// Send the register command for this event and this function name
		GetConnection()->SendAgentCommand(&response, sml_Names::kCommand_RegisterForEvent, NULL, sml_Names::kParamEventID, pEvent, sml_Names::kParamName, pRhsFunctionName) ;
	}

	// Record the handler
	m_CallbackIDCounter++ ;
	RhsEventHandlerPlusData handlerPlus(id, pRhsFunctionName, handler, pUserData, m_CallbackIDCounter) ;
	m_RhsEventMap.add(pRhsFunctionName, handlerPlus, addToBack) ;

	// Return the ID.  We use this later to unregister the callback
	return m_CallbackIDCounter ;
}

bool Kernel::InternalRemoveRhsFunction(smlRhsEventId id, int callbackID) 
{
	// Build a test object for the callbackID we're interested in
	TestRhsCallback test(callbackID) ;

	// Find the function for this callbackID (for RHS functions the key is a function name not an event id)
	std::string functionName = m_RhsEventMap.findFirstKeyByTest(&test, "") ;

	if (functionName.length() == 0)
		return false ;

	// Remove the handler from our map
	m_RhsEventMap.removeAllByTest(&test) ;

	// If we just removed the last handler, then unregister from the kernel for this event
	if (m_RhsEventMap.getListSize(functionName) == 0)
	{
		AnalyzeXML response ;

		char const* pEvent = m_pEventMap->ConvertToString(id) ;

		// Send the unregister command
		GetConnection()->SendAgentCommand(&response, sml_Names::kCommand_UnregisterForEvent, NULL, sml_Names::kParamEventID, pEvent, sml_Names::kParamName, functionName.c_str()) ;
	}

	return true ;
}

/*************************************************************
* @brief Register a handler for a RHS (right hand side) function.
*		 This function can be called in the RHS of a production firing
*		 allowing a user to quickly extend Soar with custom methods added to the client.
*
*		 The methods should only operate on the incoming argument list and return a
*		 result without access to other external information to remain with the theory of a Soar agent.
*
*		 Multiple handlers can be registered for the same function but only one will ever be called.
*		 This will be the first handler registered in the local process (where the Kernel is executing).
*		 If no handler is available there then the first handler registered in an external process will be called.
*		 (The latter case could obviously be quite slow).
*
*		 The function is implemented by providing a handler (a RhsEventHandler).  This will be passed a single string
*		 and returns a string.  The incoming argument string can contain arguments that the client should parse
*		 (e.g. passing a coordinate as "12 56").  The format of the string is up to the implementor of the specific RHS function.
*
* @param pRhsFunctionName	The name of the method we are implementing (case-sensitive)
* @param handler			A function that will be called when the event happens
* @param pUserData			Arbitrary data that will be passed back to the handler function when the event happens.
* @param addToBack			If true add this handler is called after existing handlers.  If false, called before existing handlers.
*
* @returns Unique ID for this callback.  Required when unregistering this callback.
*************************************************************/
int	Kernel::AddRhsFunction(char const* pRhsFunctionName, RhsEventHandler handler, void* pUserData, bool addToBack)
{
	smlRhsEventId id = smlEVENT_RHS_USER_FUNCTION ;

	return InternalAddRhsFunction(id, pRhsFunctionName, handler, pUserData, addToBack) ;
}

/*************************************************************
* @brief Unregister for a particular RHS function
*************************************************************/
bool Kernel::RemoveRhsFunction(int callbackID)
{
	smlRhsEventId id = smlEVENT_RHS_USER_FUNCTION ;

	return InternalRemoveRhsFunction(id, callbackID) ;
}

/*************************************************************
* @brief Register a handler for receiving generic messages sent from another client.
*		 The content of the messages are up to the client and really aren't related to Soar, but providing the
*		 ability to send a message from any client to any other client is sometimes useful.
*
*		 When the original client sends a message, the RHS function handler is called to process and (optionally) return
*		 a message to the caller.
*
*		 Multiple handlers can be registered for a given message type and the results will be concatenated together and returned
*		 to the original caller.  (This is expected to be an usual situation).
*		 
*		 A RHS (right hand side) function handler is used just to reduce the number of types in the system and because it is sufficient
*		 for this purpose.
*
*		 The function is implemented by providing a handler (a RhsEventHandler).  This will be passed a single string
*		 and returns a string.  The incoming argument string can contain arguments that the client should parse
*		 (e.g. passing a coordinate as "12 56").  The format of the string is up to the implementor of the specific RHS function.
*
* @param pClientName		The name of the client that this message relates to (e.g. "TankSoar" for messages sent to/from tank soar).  The clients sending/receiving just need to agree on this string.
*							Due to the way this is implemented, the type here must not be the same as the name of a RHS function registered with Soar.
* @param pUserData			Arbitrary data that will be passed back to the handler function when the event happens.
* @param addToBack			If true add this handler is called after existing handlers.  If false, called before existing handlers.
*
* @returns Unique ID for this callback.  Required when unregistering this callback.
*************************************************************/
int Kernel::RegisterForClientMessageEvent(char const* pClientName, ClientMessageHandler handler, void* pUserData, bool addToBack)
{
	smlRhsEventId id = smlEVENT_CLIENT_MESSAGE ;

	// We actually use the RHS function code internally to process this message (since it's almost exactly like calling a RHS function that's
	// processed on a client).
	return InternalAddRhsFunction(id, pClientName, (RhsEventHandler)handler, pUserData, addToBack) ;
}

/*************************************************************
* @brief Unregister for a particular client message
*        using the ID passed back from RegisterForClientMessageEvent().
* @returns True if succeeds
*************************************************************/
bool Kernel::UnregisterForClientMessageEvent(int callbackID)
{
	smlRhsEventId id = smlEVENT_CLIENT_MESSAGE ;

	return InternalRemoveRhsFunction(id, callbackID) ;
}

/*************************************************************
* @brief Send a message to another client (not the Soar kernel).
*		 The other client must have registered for this message to receive it.
*
*		 This mechanism allows one client to send an arbitrary string to another client and
*		 get a response (as a string).  The contents of the string are up to the clients to agree upon.
*		 Passing a string is sufficient to support either simple functionality (passing just a single value) up to
*		 complex functionality (passing a complete XML message).
*
*		 If multiple other clients register for this message, their responses are concatenated together in the result.
*		 (This is not expected to be a common usage).
*
* @param pAgent				The originating agent (can be NULL), if this message is specific to an agent.
* @param pClientName		The name of the client that this message relates to (e.g. "TankSoar" for messages sent to/from tank soar).  The clients sending/receiving just need to agree on this string.
*							Due to the way this is implemented, the type here must not be the same as the name of a RHS function registered with Soar.
* @param pMessage			The message being sent.
* @returns The response (if any) from the receiving client.  The string "**NONE**" is reserved to indicate nobody was registered for this event.
*************************************************************/
std::string Kernel::SendClientMessage(Agent* pAgent, char const* pClientName, char const* pMessage)
{
	AnalyzeXML response ;

	bool ok = GetConnection()->SendAgentCommand(&response, sml_Names::kCommand_SendClientMessage, pAgent ? pAgent->GetAgentName() : NULL, sml_Names::kParamName, pClientName, sml_Names::kParamMessage, pMessage) ;

	if (ok)
	{
		const char* pResponse = response.GetResultString() ;
		return pResponse ? pResponse : "" ;
	}

	return GetLastErrorDescription() ;
}

/*************************************************************
* @brief Unregister for a particular event
*************************************************************/
bool Kernel::UnregisterForSystemEvent(int callbackID)
{
	// Build a test object for the callbackID we're interested in
	TestSystemCallback test(callbackID) ;

	// Find the event ID for this callbackID
	smlSystemEventId id = m_SystemEventMap.findFirstKeyByTest(&test, (smlSystemEventId)-1) ;

	if (id == -1)
		return false ;

	// Remove the handler from our map
	m_SystemEventMap.removeAllByTest(&test) ;

	// If we just removed the last handler, then unregister from the kernel for this event
	if (m_SystemEventMap.getListSize(id) == 0)
	{
		UnregisterForEventWithKernel(id, NULL) ;
	}

	return true ;
}

/*************************************************************
* @brief Unregister for a particular event
* @returns True if succeeds
*************************************************************/
bool Kernel::UnregisterForStringEvent(int callbackID)
{
	// Build a test object for the callbackID we're interested in
	TestStringCallback test(callbackID) ;

	// Find the event ID for this callbackID
	smlStringEventId id = m_StringEventMap.findFirstKeyByTest(&test, (smlStringEventId)-1) ;

	if (id == -1)
		return false ;

	// Remove the handler from our map
	m_StringEventMap.removeAllByTest(&test) ;

	// If we just removed the last handler, then unregister from the kernel for this event
	if (m_StringEventMap.getListSize(id) == 0)
	{
		UnregisterForEventWithKernel(id, NULL) ;
	}

	return true ;
}

/*************************************************************
* @brief Unregister for a particular event
* @returns True if succeeds
*************************************************************/
bool Kernel::UnregisterForUpdateEvent(int callbackID)
{
	// Build a test object for the callbackID we're interested in
	TestUpdateCallback test(callbackID) ;

	// Find the event ID for this callbackID
	smlUpdateEventId id = m_UpdateEventMap.findFirstKeyByTest(&test, (smlUpdateEventId)-1) ;

	if (id == -1)
		return false ;

	// Remove the handler from our map
	m_UpdateEventMap.removeAllByTest(&test) ;

	// If we just removed the last handler, then unregister from the kernel for this event
	if (m_UpdateEventMap.getListSize(id) == 0)
	{
		UnregisterForEventWithKernel(id, NULL) ;
	}

	return true ;
}

/*************************************************************
* @brief Unregister for a particular event
*************************************************************/
bool Kernel::UnregisterForAgentEvent(int callbackID)
{
	// Build a test object for the callbackID we're interested in
	TestAgentCallback test(callbackID) ;

	// Find the event ID for this callbackID
	smlAgentEventId id = m_AgentEventMap.findFirstKeyByTest(&test, (smlAgentEventId)-1) ;

	if (id == -1)
		return false ;

	// Remove the handler from our map
	m_AgentEventMap.removeAllByTest(&test) ;

	// If we just removed the last handler, then unregister from the kernel for this event
	if (m_AgentEventMap.getListSize(id) == 0)
	{
		UnregisterForEventWithKernel(id, NULL) ;
	}

	return true ;
}

/*************************************************************
* @brief The smlEVENT_INTERRUPT_CHECK event fires every n-th
*		 step (phase) during a run.  The n is controlled by
*		 this rate.  By setting a larger value there is less
*		 overhead (checking to see if we wish to interrupt the run)
*		 but response time to an interrupt will go down.
*
*		 Setting this rate does not register you for the event.
*		 You should call RegisterForSystemEvent to do that.
*		 Also, any other event can be used as the basis for an
*		 interruption (e.g. registering for each decision cycle)
*		 but those don't offer the same throttle control as this event.
*
* @param newRate >= 1
*************************************************************/
bool Kernel::SetInterruptCheckRate(int newRate)
{
	// Reject invalid rates
	if (newRate <= 0)
		return false ;

	AnalyzeXML response ;

	// Convert int to a string
	std::ostringstream ostr ;
	ostr << newRate ;

	bool ok = GetConnection()->SendAgentCommand(&response, sml_Names::kCommand_SetInterruptCheckRate, NULL, sml_Names::kParamValue, ostr.str().c_str()) ;

	return ok ;
}

/*************************************************************
* @brief The Soar kernel version is based on sending a request
*		 to the kernel asking for its version and returning the
*		 result.
*
*		 The Soar client version reports the version of Soar this
*		 client was compiled for.  It's based on a constant
*		 stored in the sml_Names list.
*
*		 The SML version also reports a constant for the version
*		 of SML in use and is again based on a constant in the
*		 sml_Names list.
*
*		 All versions are of the form Major.Minor.Release
*		 E.g. 8.6.1
*************************************************************/
std::string Kernel::GetSoarKernelVersion()
{
	AnalyzeXML response ;
	if (GetConnection()->SendAgentCommand(&response, sml_Names::kCommand_GetVersion))
	{
		return response.GetResultString() ;
	}
	else
	{
		return "Error: Unable to retrieve the version from the kernel" ;
	}
}

std::string Kernel::GetSoarClientVersion()
{
	return sml_Names::kSoarVersionValue ;
}

std::string Kernel::GetSMLVersion()
{
	return sml_Names::kSMLVersionValue ;
}

#ifdef _WIN32
#include "Windows.h"	// Needed for load library
#undef SendMessage		// Windows defines this as a macro.  Yikes!

#elif defined(HAVE_DLFCN_H)
#include <dlfcn.h>      // Needed for dlopen and dlsym
#define GetProcAddress dlsym

#endif // _WIN32

typedef void (*InitLibraryFunction)(Kernel*, void*);

void Kernel::LoadExternalLibrary(const char *pLibraryName) {
		// Make a copy of the library name so we can work on it.
		std::string libraryName = pLibraryName ;

		// We shouldn't be passed something with an extension
		// but if we are, we'll try to strip the extension to be helpful
		size_t pos = libraryName.find_last_of('.') ;
		if (pos != std::string::npos)
		{
			libraryName.erase(pos) ;
		}

#ifdef _WIN32
		// The windows shared library
		libraryName = libraryName + ".dll";
		
		// Now load the library itself.
		HMODULE hLibrary = LoadLibrary(libraryName.c_str()) ;

#else
		std::string newLibraryName = "lib" + libraryName + ".so";
		void* hLibrary = 0;
		hLibrary = dlopen(newLibraryName.c_str(), RTLD_LAZY);
		if (!hLibrary) {
			// Try again with mac extention
			newLibraryName = "lib" + libraryName + ".dylib";
			hLibrary = dlopen(newLibraryName.c_str(), RTLD_LAZY);
		}
		// FIXME error details can be returned by a call to dlerror()
#endif
		InitLibraryFunction pInitLibraryFunction = (InitLibraryFunction)GetProcAddress(hLibrary, "sml_InitLibrary") ;

		pInitLibraryFunction(this, 0);
}