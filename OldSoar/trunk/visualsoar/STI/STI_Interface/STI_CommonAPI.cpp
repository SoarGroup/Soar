
#include "STI_CommonAPI.h"
#include "Constants.h"
#include "STI_CommandIDs.h"
#include "../SoarSocket/Check.h"
#include "../SoarSocket/ServerSocket.h"
#include "../SoarSocket/ClientSocket.h"
#include "../SoarSocket/Command.h"
#include "ServerData.h"
#include "ServerList.h"

// Keep a list of all active servers
// so we can check close calls are valid and
// also close down all connections left open when
// app shuts down (through the destructor).
CTServerList g_ServerList ;

/////////////////////////////////////////////////////////////////////
// Function name  : STI_InitInterfaceLibrary
// 
// Return type    : STI_COMMON_API STI_Handle 	// true if succeeded
// Argument		  : char const* pName			// A name for the agent/tool
// Argument       : bool bIsRuntime				// Initializing a runtime or a tool?
// 
// Description	  : Call once at startup to initialize everything.
//					Also creates a socket to listen for connections on.
//
/////////////////////////////////////////////////////////////////////
STI_COMMON_API STI_Handle STI_InitInterfaceLibrary(char const* pName, bool bIsRuntime)
{
	CTServerData* pServer = new CTServerData ;

	if (pServer)
	{
		bool ok = pServer->InitInterfaceLibrary(pName, bIsRuntime) ;

		// If there's an error initializing then delete the object
		// and return null.
		if (!ok)
		{
			delete pServer ;
			pServer = NULL ;
		}
	}

	// Return a "handle" which is of course really a pointer
	// to the data structure.
	STI_Handle hServer = (STI_Handle)pServer ;

	// Keep a list of all active servers
	if (hServer)
		g_ServerList.AddServer(hServer) ;

	return hServer ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : STI_InitListenPort
// 
// Return type    : STI_COMMON_API bool 	// true if succeeded
// Argument		  : char const* pName		// A name for the agent/tool
// Argument       : bool bIsRuntime			// Initializing a runtime or a tool?
// 
// Description	  : Call this to initialize the listening port
//
/////////////////////////////////////////////////////////////////////
STI_COMMON_API bool	STI_InitListenPort(STI_Handle hServer)
{
	CHECK_RET_FALSE(hServer) ;

	return ((CTServerData*)hServer)->InitListenPort();
}

// BADBAD: Need to be able to call this more than once--
//         should we close existing connections and reconnect
// RemoteIPAddress can be NULL -- which means connect to the local host.

/////////////////////////////////////////////////////////////////////
// Function name  : STI_EstablishConnections
// 
// Return type    : STI_COMMON_API bool 	
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
STI_COMMON_API bool STI_EstablishConnections(STI_Handle hServer, char const* pRemoteIPAddress, bool bStopOnFirstNotFound)
{
	CHECK_RET_FALSE(hServer) ;

	return ((CTServerData*)hServer)->EstablishConnections(pRemoteIPAddress, bStopOnFirstNotFound) ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : STI_GetNumberConnections
// 
// Return type    : STI_COMMON_API STI_Handle 	
// 
// Description	  : Get the number of current connections to other libraries
//
/////////////////////////////////////////////////////////////////////
STI_COMMON_API long STI_GetNumberConnections(STI_Handle hServer)
{
	CHECK_RET_ZERO(hServer) ;

	return ((CTServerData*)hServer)->GetNumberConnections() ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : const* STI_GetConnectionName
// 
// Return type    : STI_COMMON_API char const*	
// Argument       : STI_Handle connection	
// 
// Description	  : Get the name of a specific connection.
//					Returns NULL for an out of range connection.
//
/////////////////////////////////////////////////////////////////////
STI_COMMON_API char const* STI_GetConnectionName(STI_Handle hServer, long index)
{
	CHECK_RET_NULL(hServer) ;

	char const* pName = ((CTServerData*)hServer)->GetConnections().GetSocketName(index) ;
	return pName ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : STI_GetConnectionPort
// 
// Return type    : STI_COMMON_API long 	
// Argument       : STI_Handle connection	
// 
// Description	  : Get the port of a specific connnecton.
//					Returns -1 for an out of range connection.
//
/////////////////////////////////////////////////////////////////////
STI_COMMON_API long STI_GetConnectionPort(STI_Handle hServer, long index)
{
	CHECK_RET_ZERO(hServer) ;

	short port = ((CTServerData*)hServer)->GetConnections().GetSocketPort(index) ;
	return port ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : STI_GetListenPort
// 
// Return type    : STI_COMMON_API long 	
// 
// Description	  : Get the port we're listening on
//
/////////////////////////////////////////////////////////////////////
STI_COMMON_API long STI_GetListenPort(STI_Handle hServer)
{
	CHECK_RET_ZERO(hServer) ;

	return ((CTServerData*)hServer)->GetListenPort() ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : STI_GetConnectionIndexByName
// 
// Return type    : long				// -1 if no match
// Argument       : char const* pName	
// 
// Description	  : Returns the index (position in the list of connections)
//					by matching based on name.
//
/////////////////////////////////////////////////////////////////////
STI_COMMON_API long	STI_GetConnectionIndexByName(STI_Handle hServer, char const* pName)
{
	CHECK_RET_ZERO(hServer) ;

	long index = ((CTServerData*)hServer)->GetConnections().GetIndexByName(pName) ;

	return index ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : STI_GetConnectionIndexByPort
// 
// Return type    : long			// -1 if no match
// Argument       : long port	
// 
// Description	  : Returns the index (position in the list of connections)
//					by matching based on port.
//
/////////////////////////////////////////////////////////////////////
STI_COMMON_API long	STI_GetConnectionIndexByPort(STI_Handle hServer, long port)
{
	CHECK_RET_ZERO(hServer) ;

	long index = ((CTServerData*)hServer)->GetConnections().GetIndexByPort((short)port) ;

	return index ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : STI_EnableConnectionByIndex
// 
// Return type    : 	
// Argument       : STI_Handle hServer	// Handle to STI data
// Argument       : long index			// Position of socket in list of sockets
// Argument       : bool bEnable		// Enable/disable communication over this socket
// 
// Description	  : Enables/disables communication over a specific connection.
//
/////////////////////////////////////////////////////////////////////
STI_COMMON_API void	STI_EnableConnectionByIndex(STI_Handle hServer, long index, bool bEnable)
{
	CHECK(hServer) ;

	CTSocket* pSocket = ((CTServerData*)hServer)->GetConnections().GetSocketByIndex(index) ;

	if (pSocket)
		pSocket->Enable(bEnable) ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : STI_EnableConnectionByName
// 
// Return type    : 	
// Argument       : STI_Handle hServer	// Handle to STI data
// Argument       : char const* pName	// The name of the remote socket (e.g. agent's name)
// Argument       : bool bEnable		// Enable/disable communication over this socket 
// 
// Description	  : Enables/disables communication over a specific connection.
//
/////////////////////////////////////////////////////////////////////
STI_COMMON_API void	STI_EnableConnectionByName(STI_Handle hServer, char const* pName, bool bEnable)
{
	CHECK(hServer) ;

	CTSocket* pSocket = ((CTServerData*)hServer)->GetConnections().GetSocketByName(pName) ;

	if (pSocket)
		pSocket->Enable(bEnable) ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : STI_EnableConnectionByPort
// 
// Return type    : 	
// Argument       : STI_Handle hServer	// Handle to STI data
// Argument       : long port			// The port number that the remote socket listens on
// Argument       : bool bEnable		// Enable/disable communication over this socket 
// 
// Description	  : Enables/disables communication over a specific connection.
//
/////////////////////////////////////////////////////////////////////
STI_COMMON_API void	STI_EnableConnectionByPort(STI_Handle hServer, long port, bool bEnable)
{
	CHECK(hServer) ;

	CTSocket* pSocket = ((CTServerData*)hServer)->GetConnections().GetSocketByPort((short)port) ;

	if (pSocket)
		pSocket->Enable(bEnable) ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : STI_IsConnectionEnabledByIndex
// 
// Return type    : 	
// Argument       : STI_Handle hServer	// Handle to STI data
// Argument       : long index			// The position of the socket in the list of sockets
// 
// Description	  : Returns true if this connection is currently active.
//
/////////////////////////////////////////////////////////////////////
STI_COMMON_API bool	STI_IsConnectionEnabledByIndex(STI_Handle hServer, long index)
{
	CHECK_RET_FALSE(hServer) ;

	CTSocket* pSocket = ((CTServerData*)hServer)->GetConnections().GetSocketByIndex(index) ;

	if (!pSocket)
		return false ;

	return pSocket->IsEnabled() ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : STI_IsConnectionEnabledByName
// 
// Return type    : 	
// Argument       : STI_Handle hServer	// Handle to STI data
// Argument       : char const* pName	// The name of the remote socket (e.g. agent's name)
// 
// Description	  : Returns true if this connection is currently active.
//
/////////////////////////////////////////////////////////////////////
STI_COMMON_API bool	STI_IsConnectionEnabledByName(STI_Handle hServer, char const* pName)
{
	CHECK_RET_FALSE(hServer) ;

	CTSocket* pSocket = ((CTServerData*)hServer)->GetConnections().GetSocketByName(pName) ;

	if (!pSocket)
		return false ;

	return pSocket->IsEnabled() ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : STI_EnableAllConnections
// 
// Return type    : 	
// Argument       : STI_Handle hServer	
// Argument       : bool bEnable	
// 
// Description	  : Enable or disable all connections.
//					When a connection is disabled no data is sent to through
//					that connection when a command is being "sent".
//
/////////////////////////////////////////////////////////////////////
STI_COMMON_API void	STI_EnableAllConnections(STI_Handle hServer, bool bEnable)
{
	CHECK(hServer) ;

	((CTServerData*)hServer)->GetConnections().EnableAll(bEnable) ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : STI_PumpMessages
// 
// Return type    : STI_COMMON_API bool					// true if no errors	
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
STI_COMMON_API bool STI_PumpMessages(STI_Handle hServer, bool bProcessAllPendingMessages)
{
	CHECK_RET_FALSE(hServer) ;

	return ((CTServerData*)hServer)->PumpMessages(bProcessAllPendingMessages) ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : STI_IsIncomingCommandAvailable
// 
// Return type    : STI_COMMON_API bool 	// true if incoming command is available
// 
// Description	  : Call after pumping messages to see if a new
//					command is waiting in the incoming queue.
//
/////////////////////////////////////////////////////////////////////
STI_COMMON_API bool STI_IsIncomingCommandAvailable(STI_Handle hServer)
{
	CHECK_RET_FALSE(hServer) ;

	return ((CTServerData*)hServer)->GetIncomingCommandQueue().IsCommandAvailable() ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : STI_GetIncomingCommand1
// 
// Return type    : STI_COMMON_API bool 	// false if no incoming command
// Argument       : long* pCommandID
// Argument       : long* pCommandFlags	
// Argument       : etc.
// 
// Description	  : Gets all numeric values from front incoming command.
//					Note: Does not pop this command from the queue.
//
/////////////////////////////////////////////////////////////////////
STI_COMMON_API bool STI_GetIncomingCommand1(STI_Handle hServer, long* pCommandID, long* pCommandFlags,
					long* pDataSize, long* pSystemMsg,
					long* pParam1, long* pParam2, long* pParam3,
					long* pParam4, long* pParam5, long* pParam6)
{
	CHECK_RET_FALSE(hServer) ;

	((CTServerData*)hServer)->GetIncomingCommandQueue().Lock() ;

	CTCommand const* pCommand = ((CTServerData*)hServer)->GetIncomingCommandQueue().GetFrontCommandPointer() ;
	
	if (!pCommand)
	{
		((CTServerData*)hServer)->GetIncomingCommandQueue().Unlock() ;
		return false ;
	}

	*pCommandID		= pCommand->GetCommandID() ;
	*pCommandFlags	= pCommand->GetCommandID() ;
	*pDataSize		= pCommand->GetDataSize() ;
	*pSystemMsg		= pCommand->GetSystemMsg() ;
	*pParam1		= pCommand->GetParam(1) ;
	*pParam2		= pCommand->GetParam(2) ;
	*pParam3		= pCommand->GetParam(3) ;
	*pParam4		= pCommand->GetParam(4) ;
	*pParam5		= pCommand->GetParam(5) ;
	*pParam6		= pCommand->GetParam(6) ;

	((CTServerData*)hServer)->GetIncomingCommandQueue().Unlock() ;

	return true ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : STI_GetIncomingCommandStringParam1
// 
// Return type    : STI_COMMON_API char const*	// NULL if no command
// 
// Description	  : Returns command string param1.
//					The returned buffer is not owned by the caller.
//					Copy it to keep it.
//
/////////////////////////////////////////////////////////////////////
STI_COMMON_API char const* STI_GetIncomingCommandStringParam1(STI_Handle hServer)
{
	CHECK_RET_NULL(hServer) ;

	((CTServerData*)hServer)->GetIncomingCommandQueue().Lock() ;

	CTCommand const* pCommand = ((CTServerData*)hServer)->GetIncomingCommandQueue().GetFrontCommandPointer() ;
	
	if (!pCommand)
	{
		((CTServerData*)hServer)->GetIncomingCommandQueue().Unlock() ;
		return NULL ;
	}

	char const* pStringParam = pCommand->GetStringParam1() ;

	((CTServerData*)hServer)->GetIncomingCommandQueue().Unlock() ;

	return pStringParam ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : STI_GetIncomingCommandData
// 
// Return type    : STI_COMMON_API char const*	// NULL if no command	
// 
// Description	  : 
//
// Returns NULL if there is no incoming command available.
// The returned buffer is not owned by the caller--copy to keep it.
// Use memcpy to copy with the size returned in long *pDataSize of GetIncomingCommand1
// to support embedded null's in the stream.
//
/////////////////////////////////////////////////////////////////////
STI_COMMON_API char const* STI_GetIncomingCommandData(STI_Handle hServer)
{
	CHECK_RET_NULL(hServer) ;

	((CTServerData*)hServer)->GetIncomingCommandQueue().Lock() ;

	CTCommand const* pCommand = ((CTServerData*)hServer)->GetIncomingCommandQueue().GetFrontCommandPointer() ;
	
	if (!pCommand)
	{
		((CTServerData*)hServer)->GetIncomingCommandQueue().Unlock() ;
		return NULL ;
	}

	char const* pDataParam = pCommand->GetData() ;

	((CTServerData*)hServer)->GetIncomingCommandQueue().Unlock() ;

	return pDataParam ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : STI_PopIncomingCommand
// 
// Return type    : STI_COMMON_API void 	
// 
// Description	  : Remove the command at the front of the incoming queue.
//
/////////////////////////////////////////////////////////////////////
STI_COMMON_API void STI_PopIncomingCommand(STI_Handle hServer)
{
	CHECK(hServer) ;

	((CTServerData*)hServer)->GetIncomingCommandQueue().PopFrontCommand() ;
}

// Send a command by putting it in the queue.  It is sent later
// when PumpMessages is called.
STI_COMMON_API bool STI_SendCommandAsynch1(STI_Handle hServer, long commandID, long commandFlags, long dataSize,
									   long param1, long param2, long param3, long param4,
									   long param5, long param6, char const* pStringParam1,
									   char const* pDataBuffer)
{
	CHECK_RET_FALSE(hServer) ;

	// Add the command to the outgoing message queue.
	bool res = ((CTServerData*)hServer)->SendCommandAsynch(commandID, commandFlags, dataSize, param1, param2,
					 param3, param4, param5, param6, pStringParam1, pDataBuffer) ;

	// Call PumpMessages to push the data out over the socket.
	res = res && ((CTServerData*)hServer)->PumpMessages(true /* ProcessAllPendingMessages */) ;

	return res ;
}


// Send a command now and wait for a response.
// BUGBUG: This command is not yet implemented--for now the command goes out asynchronously.
STI_COMMON_API bool STI_SendCommandSynch1(STI_Handle hServer, long commandID, long commandFlags, long dataSize,
									  long param1, long param2, long param3, long param4,
									  long param5, long param6, char const* pStringParam1,
									  char const* pDataBuffer)
{
	CHECK_RET_FALSE(hServer) ;

	return ((CTServerData*)hServer)->SendCommandSynch(commandID, commandFlags, dataSize, param1, param2,
					 param3, param4, param5, param6, pStringParam1, pDataBuffer) ;
}

/////////////////////////////////////////////////////////////////////
// Function name  : STI_TerminateInterfaceLibrary
// 
// Return type    : STI_COMMON_API bool 	
// 
// Description	  : Call once at shutdown to close all connections cleanly.
//
/////////////////////////////////////////////////////////////////////
STI_COMMON_API bool STI_TerminateInterfaceLibrary(STI_Handle hServer)
{
	CHECK_RET_FALSE(hServer) ;

	// Make sure this server is in the list of servers
	// This protects against calling to close the same server twice.
	if (!g_ServerList.HasServer(hServer))
		return false ;

	// Take it out of the server list
	g_ServerList.RemoveServer(hServer) ;

	// Close the server down.
	return ((CTServerData*)hServer)->CloseAll() ;
}

