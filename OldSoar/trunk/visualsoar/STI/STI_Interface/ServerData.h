#ifndef __USE_CTSERVERDATA__
#define __USE_CTSERVERDATA__

class CTSocketLib ;
class CTServerSocket ;

#include "../SoarSocket/CommandQueue.h"
#include "../SoarSocket/Connections.h"

class CTServerData
{
protected:
	// Used to initialize local OS socket library
	CTSocketLib* m_pOSSocketLib ;

	// The socket we're listening on for new connections
	CTServerSocket* m_pListener ;

	// The list of existing socket connections
	CTConnections	m_Connections ;

	// true if this is a runtime instance.  False if this is a tool.
	bool			m_bIsRuntime ;

	// The number of the port we're listening on
	short			m_ListenPort ;

	// The name for this tool/runtime
	char			m_Name[256] ;

	// Command counter -- used to mark messages with unique ID
	long			m_CommandCount ;

	// The queue of commands coming in (received) and going out (waiting to be sent).
	CTCommandQueue	m_IncomingCommandQueue ;
	CTCommandQueue	m_OutgoingCommandQueue ;

public:
	// Constructor
	CTServerData()
	{
		m_pOSSocketLib  = NULL ;
		m_pListener     = NULL ;
		m_CommandCount	= 0 ;
	}

	// Destructor
	~CTServerData()
	{
		CloseAll() ;
	}

	// Accessors
	CTConnections&	GetConnections()		  { return m_Connections ; }
	SocketList&     GetConnectionList()       { return m_Connections.GetSocketList() ; }
	long			GetNumberConnections()	  { return m_Connections.GetNumberConnections() ; }
	CTCommandQueue& GetIncomingCommandQueue() { return m_IncomingCommandQueue ; } 
	CTCommandQueue& GetOutgoingCommandQueue() { return m_OutgoingCommandQueue ; } 
	short			GetListenPort()			  { return m_ListenPort ; }

	// Initialize the library
	bool		InitInterfaceLibrary(char const* pName, bool bIsRuntime) ;

	// Initialize our listening port
	bool		InitListenPort();

	// Establish connections to any existing tools or runtimes as appropriate.  RemoteIPAddress can be NULL for local.
	bool		EstablishConnections(char const* pRemoteIPAddress, bool bStopOnFirstNotFound) ;

	// Send and receive any waiting messages (or just one of each if bProcessAll is false).
	// This pumps messages on all active connections and establishes new connections through the listener socket.
	bool		PumpMessages(bool bProcessAllPendingMessages) ;

	// Send a command by putting it in the queue.  It is sent later
	// when PumpMessages is called.
	bool SendCommandAsynch(long commandID, long commandFlags, long dataSize,
						   long param1, long param2, long param3, long param4,
						   long param5, long param6, char const* pStringParam1,
						   char const* pDataBuffer) ;

	// Send a command now and wait for a response.
	bool SendCommandSynch(long commandID, long commandFlags, long dataSize,
						  long param1, long param2, long param3, long param4,
						  long param5, long param6, char const* pStringParam1,
						  char const* pDataBuffer) ;

	// Close all connections
	bool		CloseAll() ;

protected:
	// Add a new connection to the list of connections.
	void	AddConnection(CTSocket* pSocket)
	{
		GetConnectionList().push_back(pSocket) ;
	}

	// Remove a connection from the list of connections.
	// Note: pSocket not deleted here--should it be?
	void	RemoveConnection(CTSocket* pSocket)
	{
		GetConnectionList().remove(pSocket) ;
	}

	// Send the name and port we're listening on
	bool SendNameCommand(CTSocket* pSocket, char const* pName, short listenPort) ;

	// Receive a command defining the name for a specific connection
	bool ProcessNameCommand(CTSocket* pSocket, CTCommand const& command) ;

	// Handle system commands
	bool ProcessSystemCommand(CTSocket* pSocket, CTCommand const& command) ;

	// Send the command at the front of the outgoing queue out through this connection
	bool SendMessage(CTSocket* pSocket, CTCommand& command) ;

	// Check for a new incoming message, get it and add it to the back of the incoming queue
	bool ReceiveMessage(CTSocket* pSocket, CTCommand& command) ;

	// Process all of the messages for a specific socket
	bool SendCommandForSocket(CTSocket* pConnection, CTCommand& command) ;
	bool ReceiveCommandsForSocket(CTSocket* pConnection, bool bProcessAllPendingMessages) ;
} ;

#endif