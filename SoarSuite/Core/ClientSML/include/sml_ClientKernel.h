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
#ifndef SML_KERNEL_H
#define SML_KERNEL_H

#include <string>

#include "sml_ObjectMap.h"
#include "sml_ClientErrors.h"
#include "sml_ClientEvents.h"
#include "sml_ListMap.h"

// Forward declaratiokn for ElementXML_Handle.
struct ElementXML_InterfaceStructTag ;
typedef struct ElementXML_InterfaceStructTag *ElementXML_Handle ;

namespace sock
{
	class SocketLib ;
}

namespace sml {

// Forward declarations
class Agent ;
class Connection ;
class ElementXML ;
class EventThread ;
class Events ;
class AnalyzeXML ;
class ClientAnalyzedXML ;

class SystemEventHandlerPlusData : public EventHandlerPlusData
{
public:
	SystemEventHandler  m_Handler ;

	SystemEventHandlerPlusData(int eventID, SystemEventHandler handler, void* userData, int callbackID) : EventHandlerPlusData(eventID, userData, callbackID)
	{
		m_Handler = handler ;
	}
} ;

class UpdateEventHandlerPlusData : public EventHandlerPlusData
{
public:
	UpdateEventHandler  m_Handler ;

	UpdateEventHandlerPlusData(int eventID, UpdateEventHandler handler, void* userData, int callbackID) : EventHandlerPlusData(eventID, userData, callbackID)
	{
		m_Handler = handler ;
	}
} ;

class StringEventHandlerPlusData : public EventHandlerPlusData
{
public:
	StringEventHandler  m_Handler ;

	StringEventHandlerPlusData(int eventID, StringEventHandler handler, void* userData, int callbackID) : EventHandlerPlusData(eventID, userData, callbackID)
	{
		m_Handler = handler ;
	}
} ;

class AgentEventHandlerPlusData : public EventHandlerPlusData
{
public:
	AgentEventHandler m_Handler ;

	AgentEventHandlerPlusData(int eventID, AgentEventHandler handler, void* userData, int callbackID) : EventHandlerPlusData(eventID, userData, callbackID)
	{
		m_Handler = handler ;
	}
} ;

class RhsEventHandlerPlusData : public EventHandlerPlusData
{
public:
	RhsEventHandler	m_Handler ;
	std::string		m_FunctionName ;

	RhsEventHandlerPlusData(int eventID, char const* pFunctionName, RhsEventHandler handler, void* userData, int callbackID) : EventHandlerPlusData(eventID, userData, callbackID)
	{
		m_Handler = handler ;

		if (pFunctionName)
			m_FunctionName = pFunctionName ;
	}
} ;

class ConnectionInfo
{
protected:
	std::string m_ID ;
	std::string m_Name ;
	std::string m_Status ;
	std::string m_AgentStatus ;

public:
	ConnectionInfo(char const* pID, char const* pName, char const* pStatus, char const* pAgentStatus)
	{
		m_ID	 = pID ;
		m_Name   = (pName ? pName : "unknown-name") ;
		m_Status = (pStatus ? pStatus : "unknown-status") ;
		m_AgentStatus = (pAgentStatus ? pAgentStatus : "unknown-status") ;
	}

	/** The ID is a unique identifier for this connection (machine created) */
	char const* GetID()	const		{ return m_ID.c_str() ; }

	/** The name may be set by the owner of the connection (or not) */
	char const* GetName() const		{ return m_Name.c_str() ; }

	/** The connection status is a string from a known set of values, again set by the owner of the connection */
	char const* GetConnectionStatus() const	{ return m_Status.c_str() ; }

	/** The agent status is a string from a known set of values, again set by the owner of the connection and refers to the most recently created agent */
	char const* GetAgentStatus() const	{ return m_AgentStatus.c_str() ; }
} ;

class Kernel : public ClientErrors
{
	// Allow the agent to call to get the connection from the kernel.
	friend class Agent ;
	friend class WorkingMemory ;	// Access to generate next ID methods

public:
	enum { kDefaultSMLPort = 12121 } ;

protected:
	long		m_TimeTagCounter ;	// Used to generate time tags (we do them in the kernel not the agent, so ids are unique for all agents)
	long		m_IdCounter ;		// Used to generate unique id names
	int			m_CallbackIDCounter ;	// Used to generate unique callback IDs

	// The mapping from event number to a list of handlers to call when that event fires
	typedef sml::ListMap<smlSystemEventId, SystemEventHandlerPlusData>			SystemEventMap ;
	typedef sml::ListMap<smlAgentEventId, AgentEventHandlerPlusData>			AgentEventMap ;
	typedef sml::ListMap<std::string, RhsEventHandlerPlusData>					RhsEventMap ;
	typedef sml::ListMap<smlUpdateEventId, UpdateEventHandlerPlusData>			UpdateEventMap ;
	typedef sml::ListMap<smlStringEventId, StringEventHandlerPlusData>			StringEventMap ;

	Connection*			m_Connection ;
	ObjectMap<Agent*>	m_AgentMap ;
	std::string			m_CommandLineResult;
	bool				m_CommandLineSucceeded ;
	sock::SocketLib*	m_SocketLibrary ;

	// Info about all connections (have to explicitly request this)
	std::list<ConnectionInfo*> m_ConnectionInfoList ;
	typedef std::list<ConnectionInfo*>::iterator ConnectionListIter ;
	bool				m_ConnectionInfoChanged ;

	// Which handler functions to call when an event comes in
	SystemEventMap		m_SystemEventMap ;
	AgentEventMap		m_AgentEventMap ;
	RhsEventMap			m_RhsEventMap ;
	UpdateEventMap		m_UpdateEventMap ;
	StringEventMap		m_StringEventMap ;

	// Class used to map events ids to and from strings
	Events*				m_pEventMap ;

	// Utility classes used to test for values in the event maps
	class TestSystemCallbackFull ;
	class TestSystemCallback ;
	class TestAgentCallbackFull ;
	class TestAgentCallback ;
	class TestRhsCallbackFull ;
	class TestRhsCallback ;
	class TestUpdateCallbackFull ;
	class TestUpdateCallback ;
	class TestStringCallbackFull ;
	class TestStringCallback ;

	// Keep a local copy of this flag so we can report
	// information directly about what the client is sending/receiving
	bool m_bTracingCommunications ;
	bool m_bShutdown ;

	// If true, don't register to get output link events
	bool m_bIgnoreOutput ;

	// This thread is used to check for incoming events when the client goes to sleep
	// It ensures the client stays "alive" and is optional (there are other ways for clients to keep themselves
	// responsive).
	EventThread*		m_pEventThread ;

	// To create a kernel object, use one of the static methods, e.g. Kernel::CreateEmbeddedConnection().
	Kernel(Connection* pConnection);

	void InitEvents() ;

	/*************************************************************
	* @brief Register for a particular event with the kernel.
	*		 (This is a primitive function, should call one of the
	*		  higher level methods which will call here if needed)
	*************************************************************/
	void	RegisterForEventWithKernel(int id, char const* pAgentName) ;

	/*************************************************************
	* @brief Unregister for a particular event with the kernel.
	*		 (This is a primitive function, should call one of the
	*		  higher level methods which will call here if needed)
	*************************************************************/
	void	UnregisterForEventWithKernel(int id, char const* pAgentName) ;

	/*************************************************************
	* @brief Creates a new Agent* object (not to be confused
	*		 with actually creating a Soar agent -- see CreateAgent for that)
	*************************************************************/
	Agent* MakeAgent(char const* pAgentName) ;

	/*************************************************************
	* @brief Returns the connection information for this kernel
	*		 which is how we communicate with the kernel (e.g. embedded,
	*		 remotely over a socket etc.)
	*************************************************************/
	Connection* GetConnection() const { return m_Connection ; }

	void SetSocketLib(sock::SocketLib* pLibrary) { m_SocketLibrary = pLibrary ; }

	long	GenerateNextID()		{ return ++m_IdCounter ; }
	long	GenerateNextTimeTag()	{ return --m_TimeTagCounter ; }	// Count down so different from Soar kernel

	/***
	***   RHS functions and message event handlers use the same internal logic, although they look rather different to the user
	***/
	int	InternalAddRhsFunction(smlRhsEventId id, char const* pRhsFunctionName, RhsEventHandler handler, void* pUserData, bool addToBack) ;
	bool InternalRemoveRhsFunction(smlRhsEventId id, int callbackID) ;

	/*************************************************************
	* @brief This function is called when an event is received
	*		 from the Soar kernel.
	*
	* @param pIncoming	The event command
	* @param pResponse	The reply (no real need to fill anything in here currently)
	*************************************************************/
	void ReceivedEvent(AnalyzeXML* pIncoming, ElementXML* pResponse) ;
	void ReceivedSystemEvent(smlSystemEventId id, AnalyzeXML* pIncoming, ElementXML* pResponse) ;
	void ReceivedAgentEvent(smlAgentEventId id, AnalyzeXML* pIncoming, ElementXML* pResponse) ;
	void ReceivedRhsEvent(smlRhsEventId id, AnalyzeXML* pIncoming, ElementXML* pResponse) ;
	void ReceivedUpdateEvent(smlUpdateEventId id, AnalyzeXML* pIncoming, ElementXML* pResponse) ;
	void ReceivedStringEvent(smlStringEventId id, AnalyzeXML* pIncoming, ElementXML* pResponse) ;

	/*************************************************************
	* @brief If this message is an XML trace message returns
	*		 the agent pointer this message is for.
	*		 Otherwise returns NULL.
	*		 This function is just to boost performance on trace messages
	*		 which are really performance critical.
	*************************************************************/
	Agent* IsXMLTraceEvent(ElementXML* pIncomingMsg) ;

public:
	/*************************************************************
	* @brief Creates a connection to the Soar kernel that is embedded
	*        within the same process as the caller.
	*
	*		 If you're not sure which method to use, you generally want "InNewThread".
	*		 That extra thread usually makes your life easier.
	*
	*		 Creating in "current thread" will produce maximum performance but requires a little more work for the developer
	*		 (you must call CheckForIncomingCommands() periodically and you should not register for events and then go to sleep).
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
	static Kernel* CreateKernelInCurrentThread(char const* pLibraryName, bool optimized = false, int portToListenOn = kDefaultSMLPort) ;
	static Kernel* CreateKernelInNewThread(char const* pLibraryName, int portToListenOn = kDefaultSMLPort) ;

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
	* @returns A new kernel object which is used to communicate with the kernel
	*		   If an error occurs a Kernel object is still returned.  Call "HadError()" and "GetLastErrorDescription()" on it.
	*************************************************************/
	static Kernel* CreateRemoteConnection(bool sharedFileSystem, char const* pIPaddress, int port = kDefaultSMLPort, bool ignoreOutput = false) ;

	/*************************************************************
	* @brief Returns the default port we use for remote connections.
	*************************************************************/
	static int GetDefaultPort() { return kDefaultSMLPort ; }

	/*************************************************************
	* @brief Preparation for deleting the kernel.
	*		 Agents are destroyed at this point (if we own the kernel)
	*		 After calling shutdown the kernel cannot be restarted
	*		 it must be deleted.
	*		 This is separated from delete to ensure that messages
	*		 relating to system shutdown can be sent in a more stable
	*		 state (while the kernel object still exists).
	*************************************************************/
	void Shutdown() ;

	/*************************************************************
	* @brief Delete the kernel (or our connection to the kernel)
	*		 releasing all memory owned by the kernel.
	*		 Users should call "Shutdown" prior to calling delete
	*		 to ensure a clean shutdown.
	*************************************************************/
	virtual ~Kernel();

	/*************************************************************
	* @brief Turning this on means we'll start dumping output about messages
	*		 being sent and received.
	*************************************************************/
	void SetTraceCommunications(bool state) ;
	bool IsTracingCommunications() ;

	/*************************************************************
	* @brief Creates a new Soar agent with the given name.
	*
	* @returns A pointer to the agent (or NULL if not found).  This object
	*		   is owned by the kernel and will be destroyed when the
	*		   kernel is destroyed.
	*************************************************************/
	Agent* CreateAgent(char const* pAgentName) ;

	/*************************************************************
	* @brief Get the list of agents currently active in the kernel
	*		 and create local Agent objects for each one (if we
	*		 don't already have that agent registered).
	*************************************************************/
	void UpdateAgentList() ;

	/*************************************************************
	* @brief Returns the number of agents (from our list of known agents).
	*************************************************************/
	int GetNumberAgents() ;

	/*************************************************************
	* @brief Destroys an agent in the kernel (and locally).
	*
	*		 This call is not guaranteed to delete the agent immediately.
	*		 If the agent is running it will be asked to halt and
	*		 once it does it will be deleted.
	*
	*		 If you need to know precisely when the agent is deleted
	*		 you can register for the smlEVENT_BEFORE_AGENT_DESTROYED
	*		 event which is called immediately before the deletion occurs.
	*************************************************************/
	bool DestroyAgent(Agent* pAgent) ;

	/*************************************************************
	* @brief Looks up an agent by name (from our list of known agents).
	*
	* @returns A pointer to the new agent structure.  This object
	*		   is owned by the kernel and will be destroyed when the
	*		   kernel is destroyed.
	*************************************************************/
	Agent* GetAgent(char const* pAgentName) ;

	/*************************************************************
	* @brief Returns the n-th agent from our list of known agents.
	*		 This is slower than GetAgent(pAgentName).
	*************************************************************/
	Agent* GetAgentByIndex(int index) ;

	/*************************************************************
	* @brief Returns true if this agent pointer is still valid and
	*		 can be used.
	*************************************************************/
	bool	IsAgentValid(Agent* pAgent) ;

	/*************************************************************
	* @brief Process a command line command
	*
	* @param pCommandLine Command line string to process.
	* @param pAgentName   Agent name to apply the command line to (can be NULL)
	* @param echoResults  If true the results are also echoed through the smlEVENT_ECHO event, so they can appear in a debugger (or other listener)
	* @returns The string form of output from the command.
	*************************************************************/
	char const* ExecuteCommandLine(char const* pCommandLine, char const* pAgentName, bool echoResults = false) ;

	/*************************************************************
	* @brief Execute a command line command and return the result
	*		 as an XML object.
	*
	* @param pCommandLine Command line string to process.
	* @param pAgentName   Agent name to apply the command line to.
	* @param pResponse    The XML response will be returned within this object.
	*                     The caller should allocate this and pass it in.
	* @returns True if the command succeeds.
	*************************************************************/
	bool ExecuteCommandLineXML(char const* pCommandLine, char const* pAgentName, ClientAnalyzedXML* pResponse) ;

	/*************************************************************
	* @brief   Run Soar for the specified number of decisions
	*
	* This command will run all agents.
	*
	* @returns The result of executing the run command.
	*		   The output from during the run is sent to a different callback.
	*************************************************************/
	char const* RunAllAgents(unsigned long numberSteps, smlRunStepSize stepSize = sml_DECISION) ;
	char const* RunAllAgentsForever() ;

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
	char const* RunAllTilOutput() ;

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
	char const* StopAllAgents() ;

	/*************************************************************
	* @brief Returns true if one or more agents are currently running.
	*
	* Can be used in conjunction with StopAllAgents() to make sure
	* all agents have actually terminated their runs.
	*************************************************************/
	bool		IsSoarRunning() ;

	/*************************************************************
	* @brief Ask the kernel for the current list of connections
	*		 and their status information.
	*		 This is a snapshot which can then be interrogated through
	*		 the other methods.
	* @returns true if the list of information has changed since the
	*		   last time this method was called.
	*************************************************************/
	bool					GetAllConnectionInfo() ;

	/*************************************************************
	* @brief Report number of connections and info about those connections.
	*		 These methods report information cached in the last
	*		 "GetAllConnectionInfo" call.
	*		 HasConnectionInfoChanged() returns false if no info changed
	*		 in the last call to GetAllConnectionInfo().
	*************************************************************/
	int						GetNumberConnections() ;
	bool					HasConnectionInfoChanged() ;
	ConnectionInfo const*	GetConnectionInfo(int i) ;
	char const*				GetConnectionStatus(char const* pConnectionName) ;
	char const*				GetAgentStatus(char const* pConnectionName) ;

	/*************************************************************
	* @brief Sets the name and current status of this connection.
	*		 This information is sent to the kernel and can be requested
	*		 by other clients.
	*		 The connection status refers to the overall state of the connection.
	*		 The agent status refers to the status with respect to the most
	*		 reccently created agent (and is set to "created" by the kernel when
	*		 an agent is created).
	*
	* These values can be used to signal to other clients when a given connection
	* (e.g. the debugger) is fully configured and ready to process events/display
	* information from a client.  It's a fairly simple system but sufficient for
	* many purposes.
	*
	* Current recommend values for status:
	*  "created" - connection exists but client may still be configuring
	*  "ready"   - client has completed its setup and is ready
	*  "closing" - client is in the act of shutting down
	*************************************************************/
	bool SetConnectionInfo(char const* pName, char const* pConnectionStatus, char const* pAgentStatus) ;

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
	bool FireStartSystemEvent() ;

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
	bool FireStopSystemEvent() ;

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
	bool SuppressSystemStop(bool state) ;

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
	char const* ExpandCommandLine(char const* pCommandLine) ;

	/*************************************************************
	* @brief Get last command line result
	*
	* @returns True if the last command line call succeeded.
	*************************************************************/
	bool GetLastCommandLineResult();

	/*************************************************************
	* @brief Returns true if this command line is a run command
	*************************************************************/
	bool IsRunCommand(char const* pCommandLine) ;

	/*************************************************************
	* @brief Returns true if this command line is a stop command
	*************************************************************/
	bool IsStopCommand(char const* pCommandLine) ;

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
	bool CheckForIncomingCommands() ;

	/*************************************************************
	* @brief Start/stop the event thread.
	*
	* This thread can be used to make sure the client remains responsive
	* if it registers for some events and then goes to sleep.
	* (E.g. in a keyboard input handler or a GUI message loop).
	*
	* This thread is started by default for remote connections
	* and embedded connections in a new thread.  A client could
	* reasonably choose to turn it off so we'll expose the methods
	* for starting and stopping.
	*
	* (Once stopped you can't currently start it again).
	*************************************************************/
	bool StartEventThread() ;
	bool StopEventThread() ;

	/*************************************************************
	* @brief This is a utility wrapper to let us sleep the entire client process
	*		 for a period of time.
	*************************************************************/
	void Sleep(long milliseconds) ;

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
	int	RegisterForSystemEvent(smlSystemEventId id, SystemEventHandler handler, void* pUserData, bool addToBack = true) ;

	/*************************************************************
	* @brief Unregister for a particular event
	* @returns True if succeeds
	*************************************************************/
	bool	UnregisterForSystemEvent(int callbackID) ;

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
	bool SetInterruptCheckRate(int newRate) ;

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
	int	AddRhsFunction(char const* pRhsFunctionName, RhsEventHandler handler, void* pUserData, bool addToBack = true) ;

	/*************************************************************
	* @brief Unregister for a particular rhs function callback
	*        using the ID passed back from AddRhsFunction().
	* @returns True if succeeds
	*************************************************************/
	bool RemoveRhsFunction(int callbackID) ;

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
	* @param handler			A function that will be called when the event happens
	* @param pUserData			Arbitrary data that will be passed back to the handler function when the event happens.
	* @param addToBack			If true add this handler is called after existing handlers.  If false, called before existing handlers.
	*
	* @returns Unique ID for this callback.  Required when unregistering this callback.
	*************************************************************/
	int RegisterForClientMessageEvent(char const* pClientName, ClientMessageHandler handler, void* pUserData, bool addToBack = true) ;

	/*************************************************************
	* @brief Unregister for a particular client message
	*        using the ID passed back from RegisterForClientMessageEvent().
	* @returns True if succeeds
	*************************************************************/
	bool UnregisterForClientMessageEvent(int callbackID) ;

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
	std::string SendClientMessage(Agent* pAgent, char const* pClientName, char const* pMessage) ;

	/*************************************************************
	* @brief Register for an "AgentEvent".
	*		 Multiple handlers can be registered for the same event.
	* @param smlEventId		The event we're interested in (see the list below for valid values)
	* @param handler		A function that will be called when the event happens
	* @param pUserData		Arbitrary data that will be passed back to the handler function when the event happens.
	* @param addToBack		If true add this handler is called after existing handlers.  If false, called before existing handlers.
	*
	* This event is registered with the kernel because they relate to the creation and destruction of agents (amongst other things).
	* If we associated it with the agent there'd be no way to register for "agent creation" events.
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
	int	RegisterForAgentEvent(smlAgentEventId id, AgentEventHandler handler, void* pUserData, bool addToBack = true) ;

	/*************************************************************
	* @brief Unregister for a particular event
	* @returns True if succeeds
	*************************************************************/
	bool	UnregisterForAgentEvent(int callbackID) ;

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
	int	RegisterForUpdateEvent(smlUpdateEventId id, UpdateEventHandler handler, void* pUserData, bool addToBack = true) ;

	/*************************************************************
	* @brief Unregister for a particular event
	* @returns True if succeeds
	*************************************************************/
	bool	UnregisterForUpdateEvent(int callbackID) ;

	/*************************************************************
	* @brief Register for a string event.
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
	int	RegisterForStringEvent(smlStringEventId id, StringEventHandler handler, void* pUserData, bool addToBack = true) ;

	/*************************************************************
	* @brief Unregister for a particular event
	* @returns True if succeeds
	*************************************************************/
	bool	UnregisterForStringEvent(int callbackID) ;

	/*************************************************************
	* @brief Get the current value of the "set-library-location" path variable.
	*
	* This points to the location where the kernelSML library was loaded
	* (unless it has been changed since the load).
	*************************************************************/
	std::string GetLibraryLocation() ;

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
	std::string GetSoarKernelVersion() ;
	static std::string GetSoarClientVersion() ;
	static std::string GetSMLVersion() ;

	/*************************************************************
	* @brief Calls Commit() for all agents -- sending any queued I/O operations
	*		 over to the kernel for processing.
	*************************************************************/
	void CommitAll() ;

protected:
	/*************************************************************
	* @brief This function is called when we receive a "call" SML
	*		 message from the kernel.
	*************************************************************/
	static ElementXML* ReceivedCall(Connection* pConnection, ElementXML* pIncoming, void* pUserData) ;

	/*************************************************************
	* @brief This function is called (indirectly) when we receive a "call" SML
	*		 message from the kernel.
	*************************************************************/
	ElementXML* ProcessIncomingSML(Connection* pConnection, ElementXML* pIncoming) ;

	/*************************************************************
	* @brief The workhorse function to create an embedded connection.
	*		 The public methods hide a few of these parameters.
	*************************************************************/
	static Kernel* CreateEmbeddedConnection(char const* pLibraryName, bool clientThread, bool optimized, int portToListenOn) ;

};

}//closes namespace

#endif //SML_KERNEL_H
