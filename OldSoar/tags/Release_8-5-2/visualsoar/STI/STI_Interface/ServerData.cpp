#include "Constants.h"
#include "STI_CommandIDs.h"
#include "../SoarSocket/SocketLib.h"
#include "../SoarSocket/ServerSocket.h"
#include "../SoarSocket/ClientSocket.h"
#include "../SoarSocket/Check.h"
#include "../SoarSocket/Command.h"
#include "../SoarSocket/CommandQueue.h"
#include "../SoarSocket/Debug.h"
#include "../SoarSocket/Connections.h"

#include "ServerData.h"

/////////////////////////////////////////////////////////////////////
// Function name  : CTServerData::InitInterfaceLibrary
// 
// Return type    : bool					// true if succeeded
// Argument		  : char const* pName		// A name for the agent/tool
// Argument       : bool bIsRuntime			// Initializing a runtime or a tool?
// 
// Description	  : Call once at startup to initialize everything.
//					Also creates a socket to listen for connections on.
//
/////////////////////////////////////////////////////////////////////
bool CTServerData::InitInterfaceLibrary(char const* pName, bool bIsRuntime)
{
	CTDEBUG_ENTER_METHOD("CTServerData::InitInterfaceLibrary");

	// This is used to initialize PrintDebug too.
	PrintDebug("Starting InitializeInterfaceLibrary") ;

	// Initialize WinSock (on Windows)
	m_pOSSocketLib = new CTSocketLib() ;

	m_pListener = new CTServerSocket() ;

	// Store whether this is a runtime or a tool
	m_bIsRuntime = bIsRuntime ;

	// Store the name
	SafeStrncpy(m_Name, pName, sizeof(m_Name)) ;

	// Listen on one port
 	bool ok = !!m_pOSSocketLib && !!m_pListener;

	if (ok)
		PrintDebug("Init Succeeded") ;
	else
		PrintDebug("Error: Init Failed") ;

	return (m_pOSSocketLib && m_pListener && ok) ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : CTServerData::InitListenPort
// 
// Return type    : bool 	
// 
// Description	  : Initializes our listening port
//
/////////////////////////////////////////////////////////////////////
bool CTServerData::InitListenPort()
{
	CTDEBUG_ENTER_METHOD("CTServerData::InitListenPort");

	// Determine if we are a runtime
	bool bIsRuntime=m_bIsRuntime;

	// Decide which port we'll listen on
	short listenPort = bIsRuntime ? kBaseRuntimePort : kBaseToolPort ;
	short maxPort    = bIsRuntime ? kMaxRuntimePort  : kMaxToolPort ;
	
	// Find an open port
	CTClientSocket client ;
	bool portOpen = false ;

	while (!portOpen && listenPort < maxPort)
	{
		// If we can connect to the port--it's not open.
		bool connect = client.ConnectToServer(NULL, listenPort) ;
		portOpen = !connect ;

		if (connect)
		{
			// Close the connection--we're just checking if we can connect
			client.CloseSocket() ;

			// If this port's in use, try the next one.
			listenPort++ ;
		}
	}

	if (!portOpen)
	{
		PrintDebug("Error: Could not find an open port to listen on") ;
		return false ;
	}

	PrintDebugFormat("Listening on port %d",listenPort) ;

	m_ListenPort = listenPort ;

	// Listen on one port
	bool ok = m_pListener ? m_pListener->CreateListener(listenPort) : false ;
	return ok;
}

/////////////////////////////////////////////////////////////////////
// Function name  : CTServerData::EstablishConnections
// 
// Return type    : bool 	
// Argument       : char const* pRemoteIPAddress	
// Argument       : bStopOnFirstNotFound	
// 
// Argument       : char const* pRemoteIPAddress	// eg "12.3.12.45" or NULL for local.  IP address to scan for connections
// Argument       : bool bStopOnFirstNotFound		// true => stop looking for new connections at first port to fail
// 
// Description	  : Search for existing tools or runtimes (as appropriate)
//					on the specified machine (NULL for local machine).
//
//					The search is a sequential search through a range of possible ports.
//					This can take a few seconds to scan all of the ports we allow.
//					Therefore the caller can ask to stop on the first failure.
//					This will *usually* find all connections--but not always.
//					If you start tool1, tool2, tool3 and then shutdown tool2
//					then a search stopping at the first failure will fail to find tool3.
//
/////////////////////////////////////////////////////////////////////
bool CTServerData::EstablishConnections(char const* pRemoteIPAddress, bool bStopOnFirstNotFound)
{
	CTDEBUG_ENTER_METHOD("CTServerData::EstablishConnections");

	// Make sure called Initialize first (and succeeded)
	CHECK_RET_FALSE(this->m_pOSSocketLib && this->m_pListener) ;

	// If this instance is a runtime, connect to tools and
	// vice versa
	short checkPort  = this->m_bIsRuntime ? kBaseToolPort : kBaseRuntimePort ;
	short maxPort    = this->m_bIsRuntime ? kMaxToolPort  : kMaxRuntimePort ;

	// Try to connect to existing servers.
	// There may be none out there.
	// Check each port that could have a listener
	for (short port = checkPort ; port <= maxPort ; port++)
	{
		// Allocate our client socket
		CTClientSocket* pClient = new CTClientSocket ;

		// See if we can connect
		bool connect = pClient->ConnectToServer(pRemoteIPAddress, port) ;

		if (connect)
		{
			PrintDebugFormat("Connected to client on port %d",port) ; 

			this->AddConnection(pClient) ;
			SendNameCommand(pClient, m_Name, m_ListenPort) ;
		}
		else
		{
			PrintDebugFormat("Failed to connect on port %d",port) ; 

			// Destroy our client socket object
			delete pClient;
			pClient=NULL;

			// If we failed to connect and we've been asked to stop
			// looking after the first failure (for speed) we're done.
			if (bStopOnFirstNotFound)
				break ;
		}
	}

	return true ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : CTServerData::ReceiveMessage
// 
// Return type    : bool 	
// Argument       : CTSocket* pSocket	
// Argument       : CTCommand& command
// 
// Description	  : Receive a message on the socket and
//					return it in command.
//
//					Will return false if no data was
//					available to be read.
//
//					Best to check there is data available first
//					before calling here.
//
/////////////////////////////////////////////////////////////////////
bool CTServerData::ReceiveMessage(CTSocket* pSocket, CTCommand& command)
{
	CTDEBUG_ENTER_METHOD("CTServerData::ReceiveMessage");

	PrintDebug("Starting to read message") ;

	// Read the incoming message
	CTMsg msg ;
	bool ok = pSocket->ReceiveMsg(msg) ;

	if (ok)
	{
		PrintDebug("Message received") ;

		// Convert the message to a command and add it to the
		// incoming command queue where the client can retrieve it.
		command.CopyFromMsg(msg) ;

		// Print out the command we just received.
		command.Dump() ;
	}
	else
	{
		// The socket can close down cleanly and we come here
		// because receive message will return false in that case.
		// The way to tell if an error occured is to see if the connection
		// is still alive (it won't be if this was a clean shutdown).
		if (pSocket->IsAlive())
		{
			// If an error occured, shut down the socket so we don't keep failing.
			PrintDebug("Error: Message failed.  Closing down socket.") ;
			pSocket->CloseSocket() ;
		}
		else
		{
			// The other side closed the connection cleanly.
			PrintDebug("Socket has closed.") ;
		}
	}

	return ok ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : CTServerData::SendMessage
// 
// Return type    : bool 	
// Argument       : CTSocket* pSocket	
// Argument		  : CTCommand& command
// 
// Description	  : Send the command out over this socket.
//
/////////////////////////////////////////////////////////////////////
bool CTServerData::SendMessage(CTSocket* pSocket, CTCommand& command)
{
	CTDEBUG_ENTER_METHOD("CTServerData::SendMessage");

	PrintDebug("Starting to send message") ;

	// Print out the command we're sending
	command.Dump() ;

	// Convert the command to a message
	CTMsg msg ;
	msg.CopyFromCommand(command) ;

	// Send the message out
	bool ok = pSocket->SendMsg(msg) ;

	if (ok)
		PrintDebug("Send succeeded") ;
	else
		PrintDebug("Error: Send failed") ;

	return ok ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : CTServerData::ProcessNameCommand
// 
// Return type    : bool 	
// Argument       : CTSocket* pSocket	
// Argument       : CTCommand const& command	
// 
// Description	  : Handle an incoming Name command.
//
/////////////////////////////////////////////////////////////////////
bool CTServerData::ProcessNameCommand(CTSocket* pSocket, CTCommand const& command)
{
	CTDEBUG_ENTER_METHOD("CTServerData::ProcessNameCommand");

	CHECK_RET_FALSE(command.GetCommandID() == STI_kSystemNameCommand) ;

	char const* pName = command.GetStringParam1() ;
	short       port  = (short)command.GetParam(1) ;

	bool bNeedToReply = (!pSocket->IsNameSent()) ;

	pSocket->SetName(pName) ;
	pSocket->SetPort(port) ;

	PrintDebugFormat("Received name %s and port %d for a connection", pName, port) ;

	// If we haven't already sent our name to them we need to reply.
	if (bNeedToReply)
	{
		SendNameCommand(pSocket, m_Name, m_ListenPort) ;
	}

	return true ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : CTServerData::ProcessSystemCommand
// 
// Return type    : bool 	
// Argument       : CTSocket* pSocket	
// Argument       : CTCommand const& command	
// 
// Description	  : Handle an incoming system command.
//					System commands are status/control commands
//					not initiated by the user.
//
/////////////////////////////////////////////////////////////////////
bool CTServerData::ProcessSystemCommand(CTSocket* pSocket, CTCommand const& command)
{
	CTDEBUG_ENTER_METHOD("CTServerData::ProcessSystemCommand");

	switch (command.GetCommandID())
	{
	case STI_kSystemNameCommand:
		return (ProcessNameCommand(pSocket, command)) ;
	default:
		{
			assert(false) ;
			PrintDebugFormat("Error: Unrecognized system command %d", command.GetCommandID()) ;
		}
	}

	return false ;
}

bool CTServerData::SendCommandForSocket(CTSocket* pConnection, CTCommand& command)
{
	CHECK_RET_FALSE(pConnection) ;

	// If the connection has died (remote side closed)
	// caller will need to delete.
	if (!pConnection->IsAlive())
	{
		// Returning true because this is not really an error condition--
		// a clean shutdown will come here.
		return true ;
	}

	// By this point we should have a live connection
	CHECK_RET_FALSE(pConnection && pConnection->IsAlive()) ;

	bool ok = SendMessage(pConnection, command) ;

	return ok ;
}

bool CTServerData::ReceiveCommandsForSocket(CTSocket* pConnection, bool bProcessAllPendingMessages)
{
	CHECK_RET_FALSE(pConnection) ;

	// If the connection has died (remote side closed)
	// caller will need to delete.
	if (!pConnection->IsAlive())
	{
		// Returning true because this is not really an error condition--
		// a clean shutdown will come here.
		return true ;
	}

	// By this point we should have a live connection
	CHECK_RET_FALSE(pConnection && pConnection->IsAlive()) ;

	bool ok = true ;
	CTCommand command ;

	// Read data until there are no more messages
	while (pConnection &&
		   pConnection->IsAlive() &&
		   pConnection->IsReadDataAvailable())
	{
		// Receive the new message
		ok = ReceiveMessage(pConnection, command) && ok ;

		// If the command is received ok, but isn't valid (we don't understand it)
		// then just ignore it.  Otherwise, add it to the command queue.
		if (ok && command.IsValid())
		{
			if (command.IsSystemMsg())
			{
				// System commands are internal--not sent by the user
				ProcessSystemCommand(pConnection, command) ;
			}

			// Add the new command to the incoming queue
			// Note: System commands are added too so client can process if they wish
			GetIncomingCommandQueue().CopyCommandAndPushBack(command) ;
		}

		// If there's an error or we've been asked to only read one message, stop reading messages.
		if (!ok || !bProcessAllPendingMessages)
			break ;
	}

	return ok ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : CTServerData::PumpMessages
// 
// Return type    : bool 	
// Argument       : bool bProcessAllPendingMessages		// If true send/recv all messages; false just do one send+recv
// 
// Description	  : Sends and receives all messages for all sockets.
//
// Call regularly to send and receive connection requests/messages
// This could be put into a separate thread later.
// It sends messages that have been queued and reads messages waiting to come in.
// It also creates accepts new connections that have arrived at the listener socket.
//
/////////////////////////////////////////////////////////////////////
bool CTServerData::PumpMessages(bool bProcessAllPendingMessages)
{
	CTDEBUG_ENTER_METHOD("CTServerData::PumpMessages");

	CHECK_RET_FALSE(m_pOSSocketLib && m_pListener) ;

	// Make sure we're not already in this function.
	// Timer could theoritically call here twice.
	static bool bPumpingMessages = false ;
	if (bPumpingMessages)
		return true ;

	bPumpingMessages = true ;

	// See if anyone new has connected.
	CTSocket* pNewConnection = m_pListener->CheckForClientConnection() ;

	if (pNewConnection)
	{
		PrintDebug("Made a new connection") ;
		AddConnection(pNewConnection) ;
	}

	// If no connections have been made, nothing more to do.
	if (this->GetConnectionList().size() == 0)
	{
		// PrintDebug("No connection exists") ;
		bPumpingMessages = false ;
		return true ;
	}

	bool ok = true ;

	// Go through all outgoing commands and send them.
	// Need to loop over the commands first so we can pop them off the stack
	while (GetOutgoingCommandQueue().IsCommandAvailable())
	{
		CTCommand command ;
		GetOutgoingCommandQueue().GetFrontCommand(command) ;

		for (SocketIter i = this->GetConnectionList().begin() ;
			 i != this->GetConnectionList().end() ; i++)
		{
			CTSocket* pSocket = *i ;

			// Don't send the command out if this socket is disabled.
			if (pSocket && pSocket->IsEnabled())
				SendCommandForSocket(pSocket, command) ;
		}

		GetOutgoingCommandQueue().PopFrontCommand() ;

		if (!bProcessAllPendingMessages)
			break ;
	}

	// Now go through all connections and receive all incoming messages
	for (SocketIter i = this->GetConnectionList().begin() ;
		 i != this->GetConnectionList().end() ; i++)
	{
		CTSocket* pSocket = *i ;

		ReceiveCommandsForSocket(pSocket, bProcessAllPendingMessages) ;
	}

	// Now check if any sockets have closed (do this after pumping
	// as we may discover it when trying to communicate)
	for (SocketIter j = this->GetConnectionList().begin() ;
		 j != this->GetConnectionList().end() ;)
	{
		CTSocket* pSocket = *j ;
		j++ ;	// We may delete pSocket, so need to avoid pointing to invalid item

		// If the connection has died (remote side closed),
		// then delete the connection.
		if (!pSocket->IsAlive())
		{
			this->RemoveConnection(pSocket) ;
			delete pSocket ;
		}
	}

	bPumpingMessages = false ;
	return ok ;
}

bool CTServerData::SendCommandAsynch(long commandID, long commandFlags, long dataSize,
									 long param1, long param2, long param3, long param4,
									 long param5, long param6, char const* pStringParam1,
									 char const* pDataBuffer)
{
	CTCommand command ;

	// Create a new unique command ID (used for synch commands mainly)
	m_CommandCount++ ;

	command.SetFromPort(m_ListenPort) ;
	command.SetUniqueID(m_CommandCount) ;

	command.SetCommandID(commandID) ;
	command.SetCommandFlags(commandFlags) ;
	command.SetDataFromCopy(pDataBuffer, dataSize) ;
	command.SetStringParam1(pStringParam1) ;
	command.SetParam(1, param1) ;
	command.SetParam(2, param2) ;
	command.SetParam(3, param3) ;
	command.SetParam(4, param4) ;
	command.SetParam(5, param5) ;
	command.SetParam(6, param6) ;

	bool ok = GetOutgoingCommandQueue().CopyCommandAndPushBack(command) ;

	return ok ;
}

// Send a command now and wait for a response.
bool CTServerData::SendCommandSynch(long commandID, long commandFlags, long dataSize,
									long param1, long param2, long param3, long param4,
									long param5, long param6, char const* pStringParam1,
									char const* pDataBuffer)
{
	CTCommand command ;

	// Create a new unique command ID (used for synch commands mainly)
	m_CommandCount++ ;

	command.SetFromPort(m_ListenPort) ;
	command.SetUniqueID(m_CommandCount) ;

	command.SetCommandID(commandID) ;
	command.SetCommandFlags(commandFlags) ;
	command.SetDataFromCopy(pDataBuffer, dataSize) ;
	command.SetStringParam1(pStringParam1) ;
	command.SetParam(1, param1) ;
	command.SetParam(2, param2) ;
	command.SetParam(3, param3) ;
	command.SetParam(4, param4) ;
	command.SetParam(5, param5) ;
	command.SetParam(6, param6) ;

	return GetOutgoingCommandQueue().CopyCommandAndPushBack(command) ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : CTServerData::SendNameCommand
// 
// Return type    : bool 	
// Argument       : CTSocket* pSocket	
// Argument       : const char* pName	
// Argument       : short listenPort	
// 
// Description	  : Send the name command.
//					BADBAD: This is currently sent immediately
//					not sure if that's really necessary at all.
//
/////////////////////////////////////////////////////////////////////
bool CTServerData::SendNameCommand(CTSocket* pSocket, const char* pName, short listenPort)
{
	CTDEBUG_ENTER_METHOD("CTServerData::SendNameCommand");

	CTCommand command ;
	command.SetSystemMsg() ;
	command.SetCommandID(STI_kSystemNameCommand) ;
	command.SetParam(1, listenPort) ;
	command.SetStringParam1(pName) ;

	// Record that we've already sent our name to the other side of this connection
	// so we don't do it again.
	pSocket->SetSentName(true) ;

	PrintDebugFormat("Sending name <%s> port %d",pName, listenPort) ;
	bool ok = this->SendMessage(pSocket, command) ;

	return ok ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : CTServerData::CloseAll
// 
// Return type    : bool 	
// 
// Description	  : Close all connections and shutdown.
//
/////////////////////////////////////////////////////////////////////
bool CTServerData::CloseAll()
{
	CTDEBUG_ENTER_METHOD("CTServerData::CloseAll");

	PrintDebug("Called CloseAll()") ;

	m_Connections.DeleteAllSockets() ;

	delete m_pListener ;
	delete m_pOSSocketLib ;			// Must be last--unload WinSock DLL

	m_pListener    = NULL ;
	m_pOSSocketLib = NULL ;

	return true ;
}
