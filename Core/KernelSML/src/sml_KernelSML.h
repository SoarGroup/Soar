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

#ifndef SML_KERNEL_SML_H
#define SML_KERNEL_SML_H

typedef struct wme_struct wme;

#include <map>
#include <list>

#include "cli_CommandLineInterface.h"
#include "sml_SystemListener.h"
#include "sml_RhsListener.h"
#include "sml_AgentListener.h"
#include "sml_UpdateListener.h"
#include "sml_StringListener.h"
#include "sml_Utils.h"
#include "sml_Events.h"
#include "init_soar.h"

namespace soar_thread
{
	class Mutex ;
}

namespace soarxml
{
	class ElementXML ;
}

namespace sml {

// Forward declarations
class Connection ;
class AnalyzeXML ;
class KernelSML ;
class OutputListener ;
class AgentSML; 
class ConnectionManager ;
class Events ;
class RunScheduler ;
class KernelHelpers ;

// Define the CommandFunction which we'll call to process commands
typedef bool (KernelSML::*CommandFunction)(AgentSML*, char const*, Connection*, AnalyzeXML*, soarxml::ElementXML*);

// Used to store a map from command name to function handler for that command
typedef std::map< std::string, CommandFunction >	CommandMap ;
typedef CommandMap::iterator					CommandMapIter ;
typedef CommandMap::const_iterator				CommandMapConstIter ;

// Map from agent names to information we keep for SML about those agents.
typedef std::map< std::string, AgentSML* >	AgentMap ;
typedef AgentMap::iterator					AgentMapIter ;
typedef AgentMap::const_iterator			AgentMapConstIter ;

// Map from kernel agent pointers to information we keep for SML about those agents.
typedef std::map< agent*, AgentSML* >			KernelAgentMap ;
typedef KernelAgentMap::iterator				KernelAgentMapIter ;
typedef KernelAgentMap::const_iterator			KernelAgentMapConstIter ;

class KernelSML
{
	// Allow the kernel listener to execute command lines directly
	friend class AgentListener;
	friend class RhsListener;
	friend class RunScheduler ;
	friend class AgentSML ;

protected:

	// On Windows this is set to the DLL's hModule handle.
	static void*		s_hModule ;

	// Map from command name to function to handle it
	CommandMap	m_CommandMap ;

	// Map from agent names to AgentSML objects, where we keep additional information
	// required for SML about each agent.
	AgentMap		m_AgentMap ;
	KernelAgentMap	m_KernelAgentMap ;

	// Command line interface module
	cli::CommandLineInterface m_CommandLineInterface ;

	// A listener socket and the list of connections to the kernel
	ConnectionManager* m_pConnectionManager ;

	// We'll use a mutex to serialize execution of commands within the kernel.
	// This is really just an insurance policy as I don't think we'll ever execute
	// commands on different threads within kernelSML because we
	// only allow one embedded connection to the kernel, but it's nice to be sure.
	soar_thread::Mutex*	m_pKernelMutex ;

	// Used to map event IDs to and from strings
	Events*			m_pEventMap ;

	// Used to listen for kernel events that are kernel based (not for a specific agent)
	SystemListener	m_SystemListener;
	RhsListener		m_RhsListener;
	AgentListener	m_AgentListener;
	UpdateListener	m_UpdateListener ;
	StringListener	m_StringListener ;

	// We can suppress system start and system stop events
	// (allowing us to Run Soar without running a connected simulation).
	bool			m_SuppressSystemStart ;
	bool			m_SuppressSystemStop ;

	// When we really issue a stop command we have to be sure we'll send the event
	// so this overrides any suppression setting.  (Use a different flag so it can't
	// be overridden by another call to suppress system stop).
	bool			m_RequireSystemStop ;

	RunScheduler*	m_pRunScheduler ;

	// If true, whenever a user issues a command that changes the state of the kernel in some manner
	// the command and its results are echoed to anyone listening.  This is useful when two users
	// are debugging the same kernel (and should be off at other times).
	bool			m_EchoCommands ;

	int				m_InterruptCheckRate;
	smlPhase		m_StopPoint ;

	// The library location is the parent of the executable or dll, depending on the system. Helps
	// some programs find the share folder for resources should they need them.
	// Note that when running Java applications, the location of the Jar should be used because 
	// this might get the parent directory of the Java executable, which will probably be wrong.
	std::string		m_LibraryDirectory;
	void			InitializeLibraryLocation();

public:

	const char* GetLibraryLocation();		// Hopefully parent of bin, lib, share
	void SetLibraryLocation(const std::string& location);

	void SetStopPoint(bool forever, smlRunStepSize runStepSize, smlPhase m_StopBeforePhase);
	smlPhase GetStopPoint() { return m_StopPoint; }

	int GetInterruptCheckRate() { return m_InterruptCheckRate; }

	/*************************************************************
	* @brief	Creates the singleton kernel object
	*			and starts listening for incoming commands on the
	*			given port.
	*************************************************************/
	static KernelSML* CreateKernelSML(int portToListenOn) ;

	int GetListenerPort();

	/*************************************************************
	* @brief	Handy utility function for dumping output to the
	*			debugger.
	*************************************************************/
	static void DebugPrint(char const* pFilename, int line, char const* pMsg)
	{
#ifdef _DEBUG
		_CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG );
		_CrtSetReportFile( _CRT_WARN, _CRTDBG_FILE_STDOUT );

		_CrtDbgReport(_CRT_WARN, pFilename, line, "KernelSML", pMsg);
#else
		unused(pFilename) ;
		unused(line) ;
		unused(pMsg) ;
#endif // _DEBUG
	}

public:
	~KernelSML(void);

	/*************************************************************
	* @brief	Shutdown any connections and sockets in preparation
	*			for the kernel process exiting.
	*************************************************************/
	void Shutdown() ;

	/*************************************************************
	* @brief	Add a new connection to the list of connections
	*			we're aware of to this soar kernel.
	*************************************************************/
	void AddConnection(Connection* pConnection) ;
	
	/*************************************************************
	* @brief	Add or remove a connection from the list listening
	*			for a particular event in the kernel.
	*************************************************************/
	void AddSystemListener(smlSystemEventId eventID, Connection* pConnection)	 { m_SystemListener.AddListener(eventID, pConnection) ; }
	void AddAgentListener(smlAgentEventId eventID, Connection* pConnection)	 { m_AgentListener.AddListener(eventID, pConnection) ; }
	void AddUpdateListener(smlUpdateEventId eventID, Connection* pConnection)	 { m_UpdateListener.AddListener(eventID, pConnection) ; }
	void AddStringListener(smlStringEventId eventID, Connection* pConnection)  { m_StringListener.AddListener(eventID, pConnection) ; }
	void RemoveSystemListener(smlSystemEventId eventID, Connection* pConnection) { m_SystemListener.RemoveListener(eventID, pConnection) ; }
	void RemoveAgentListener(smlAgentEventId eventID, Connection* pConnection)   { m_AgentListener.RemoveListener(eventID, pConnection) ; }
	void RemoveUpdateListener(smlUpdateEventId eventID, Connection* pConnection) { m_UpdateListener.RemoveListener(eventID, pConnection) ; }
	void RemoveStringListener(smlStringEventId eventID, Connection* pConnection) { m_StringListener.RemoveListener(eventID, pConnection) ; }

	/*************************************************************
	* @brief	Notify listeners that this event has occured.
	*************************************************************/
	void FireUpdateListenerEvent(smlUpdateEventId eventID, int runFlags)	
	{ 
		m_UpdateListener.OnKernelEvent(eventID, 0, &runFlags) ; 
	}

	/*************************************************************
	* @brief	Notify listeners that this event has occured.
	*************************************************************/
	void FireSystemEvent(smlSystemEventId eventID)						
	{ 
		m_SystemListener.OnKernelEvent(eventID, 0 , 0) ; 
	}

	/*************************************************************
	* @brief	Notify listeners that this event has occured.
	*************************************************************/
	void FireAgentEvent(AgentSML* pAgentSML, smlAgentEventId eventID)		{ m_AgentListener.OnEvent(eventID, pAgentSML) ; }

	/*************************************************************
	* @brief	Notify listeners that this event has occured.
	*************************************************************/
	std::string FireEditProductionEvent(char const* pProduction);

	/*************************************************************
	* @brief	Notify listeners that this event has occured.
	*************************************************************/
	std::string FireLoadLibraryEvent(char const* pLibraryCommand);

	/*************************************************************
	* @brief	Add or remove a connection from the list implementing
	*			a particular rhs function in the kernel.
	*************************************************************/
	void AddRhsListener(char const* pFunctionName, Connection* pConnection)	   { m_RhsListener.AddRhsListener(pFunctionName, pConnection) ; }
	void RemoveRhsListener(char const* pFunctionName, Connection* pConnection) { m_RhsListener.RemoveRhsListener(pFunctionName, pConnection) ; }
	bool FireRhsEvent(AgentSML* pAgentSML, smlRhsEventId eventID, std::string const& functionName, std::string const& arguments, std::string* pResult) {
		 return m_RhsListener.ExecuteRhsCommand(pAgentSML, eventID, functionName, arguments, pResult) ; }

	/*************************************************************
	* @brief	Send this message out to any clients that are listening.
	*			These messages are from one client to another--kernelSML is just
	*			facilitating the message passing process without knowing/caring what is being passed.
	*************************************************************/
	std::string SendClientMessage(AgentSML* pAgentSML, char const* pMessageType, char const* pMessage) ;

	/*************************************************************
	* @brief	Send this command line out to all clients that have
	*			registered a filter.  The result is the processed
	*			version of the command line.
	*************************************************************/
	bool SendFilterMessage(AgentSML* pAgent, char const* pCommandLine, std::string* pResult) ;

	/*************************************************************
	* @brief	Returns true if at least one filter is registered.
	*************************************************************/
	bool HasFilterRegistered() ;

	/*************************************************************
	* @brief Convert from a string version of an event to the int (enum) version.
	*		 Returns smlEVENT_INVALID_EVENT (== 0) if the string is not recognized.
	*************************************************************/
	int ConvertStringToEvent(char const* pStr) ;

	/*************************************************************
	* @brief Convert from int version of an event to the string form.
	*		 Returns NULL if the id is not recognized.
	*************************************************************/
	char const* ConvertEventToString(int id) ;

	/*************************************************************
	* @brief Flags used to suppress the firing of system start and
	*		 system stop events.
	*************************************************************/
	void SetSuppressSystemStart(bool state) { m_SuppressSystemStart = state ; }	
	void SetSuppressSystemStop(bool state)  { m_SuppressSystemStop = state ; }	

	void RequireSystemStop(bool state)		{ m_RequireSystemStop = state ; }

	bool IsSystemStartSuppressed() { return m_SuppressSystemStart ; }
	bool IsSystemStopSuppressed()  { return m_SuppressSystemStop && !m_RequireSystemStop ; }

	/*************************************************************
	* @brief	Remove any events that this connection was listening to.
	*			Generally do this just prior to deleting the connection.
	*************************************************************/
	void RemoveAllListeners(Connection* pConnection) ;

	/*************************************************************
	* @brief	Receive and process any messages from remote connections
	*			that are waiting on a socket.
	*			Returning false indicates we should stop checking
	*			for more messages (and presumably shutdown completely).
	*************************************************************/
	bool ReceiveAllMessages() ;

	/*************************************************************
	* @brief	Stop the thread that is used to receive messages
	*			from remote connections.  We do this when we're
	*			using a "synchronized" embedded connection, which
	*			means commands execute in the client's thread instead
	*			of the receiver thread.
	*************************************************************/
	void StopReceiverThread() ;

	/*************************************************************
	* @brief Turning this on means we'll start dumping output about messages
	*		 being sent and received.  Currently this only applies to remote connections.
	*************************************************************/
	void SetTraceCommunications(bool state) ;
	bool IsTracingCommunications() ;

	/*************************************************************
	* @brief	Takes an incoming SML message and responds with
	*			an appropriate response message.
	*
	* @param pConnection	The connection this message came in on.
	* @param pIncoming		The incoming message
	*************************************************************/
	soarxml::ElementXML* ProcessIncomingSML(Connection* pConnection, soarxml::ElementXML* pIncoming) ;

	/*************************************************************
	* @brief	Look up an agent from its name.
	*************************************************************/
	AgentSML* GetAgentSML(char const* pAgentName) ;

	/*************************************************************
	* @brief	Returns the number of agents.
	*************************************************************/	
	int			GetNumberAgents() ;

	/*************************************************************
	* @brief	Delete the agent sml object for this agent.
	*			This object stores the data SML uses when working
	*			with the underlying gSKI agent.
	*************************************************************/	
	bool DeleteAgentSML( const char* agentName ) ;
	//bool DeleteAgentSML(agent* pAgent) ;

	/*************************************************************
	* @brief	Stops and deletes all agents.  Generally called
	*			just prior to shutdown.
	*************************************************************/	
	void DeleteAllAgents(bool waitTillDeleted) ;

	/*************************************************************
	* @brief	The run scheduler is responsible for deciding which
	*			agents to include in a run and actually performing
	*			that run.
	*************************************************************/	
	RunScheduler*	GetRunScheduler() { return m_pRunScheduler ; }

	/*************************************************************
	* @brief	Defines which phase we stop before when running by decision.
	*			E.g. Pass input phase to stop just after generating output and before receiving input.
	*			This is a setting which modifies the future behavior of "run <n> --decisions" commands.
	*************************************************************/	
	void SetStopBefore(smlPhase phase) ;
	smlPhase GetStopBefore() ;
	top_level_phase ConvertSMLToSoarPhase( smlPhase phase ) ;

	/*************************************************************
	* @brief	If true, whenever a user issues a command that changes the state of the kernel in some manner
	*			the command and its results are echoed to anyone listening.  This is useful when two users
	*			are debugging the same kernel (and should be off at other times).
	*************************************************************/	
	void SetEchoCommands(bool state) { m_EchoCommands = state ; }
	bool GetEchoCommands()			 { return m_EchoCommands ; }

	/*************************************************************
	* @brief	Request that all agents stop soon
	*************************************************************/	
	void InterruptAllAgents(smlStopLocationFlags stopLoc) ;
	void ClearAllInterrupts() ;

	// A set of helper functions for tracing kernel wmes
	static void			Symbol2String(Symbol* pSymbol, 	bool refCounts, std::ostringstream& buffer);
	static std::string	Wme2String(wme* pWME, bool refCounts);
	static void			PrintDebugWme(char const* pMsg, wme* pWME, bool refCounts = false);
	static void			PrintDebugSymbol(Symbol* pSymbol, bool refCounts = false);

protected:
	KernelSML(int portToListenOn);

protected:
	/*************************************************************
	* @brief	Return an object* to the caller.
	*************************************************************/
	bool ReturnResult(Connection* pConnection, soarxml::ElementXML* pResponse, char const* pResult) ;

	/*************************************************************
	* @brief	Return an integer result to the caller.
	*************************************************************/
	bool ReturnIntResult(Connection* pConnection, soarxml::ElementXML* pResponse, int64_t result) ;

	/*************************************************************
	* @brief	Return a boolean result to the caller.
	*************************************************************/
	bool ReturnBoolResult(Connection* pConnection, soarxml::ElementXML* pResponse, bool result) ;

	/*************************************************************
	* @brief	Return an invalid argument error to the caller.
	*************************************************************/
	bool InvalidArg(Connection* pConnection, soarxml::ElementXML* pResponse, char const* pCommandName, char const* pErrorDescription) ;

	void BuildCommandMap() ;

	bool ProcessCommand(char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, soarxml::ElementXML* pResponse) ;

	// There should always be exactly one local connection to the kernel (the process that loaded us).
	Connection* GetEmbeddedConnection() ;

	// Our command handlers

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
	bool HandleCreateAgent(AgentSML* pAgentSML, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, soarxml::ElementXML* pResponse) ;
	bool HandleGetInputLink(AgentSML* pAgentSML, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, soarxml::ElementXML* pResponse) ;
	bool HandleInput(AgentSML* pAgentSML, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, soarxml::ElementXML* pResponse) ;
	bool HandleCommandLine(AgentSML* pAgentSML, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, soarxml::ElementXML* pResponse) ;
	bool HandleExpandCommandLine(AgentSML* pAgentSML, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, soarxml::ElementXML* pResponse) ;
	bool HandleCheckForIncomingCommands(AgentSML* pAgentSML, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, soarxml::ElementXML* pResponse) ;
	bool HandleDestroyAgent(AgentSML* pAgentSML, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, soarxml::ElementXML* pResponse) ;
	bool HandleGetAgentList(AgentSML* pAgentSML, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, soarxml::ElementXML* pResponse) ;
	bool HandleSetInterruptCheckRate(AgentSML* pAgentSML, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, soarxml::ElementXML* pResponse) ;
	bool HandleFireEvent(AgentSML* pAgentSML, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, soarxml::ElementXML* pResponse) ;
	bool HandleSuppressEvent(AgentSML* pAgentSML, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, soarxml::ElementXML* pResponse) ;
	bool HandleGetVersion(AgentSML* pAgentSML, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, soarxml::ElementXML* pResponse) ;
	bool HandleShutdown(AgentSML* pAgentSML, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, soarxml::ElementXML* pResponse) ;
	bool HandleIsSoarRunning(AgentSML* pAgentSML, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, soarxml::ElementXML* pResponse) ;
	bool HandleSetConnectionInfo(AgentSML* pAgentSML, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, soarxml::ElementXML* pResponse) ;
	bool HandleGetConnections(AgentSML* pAgentSML, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, soarxml::ElementXML* pResponse) ;
	bool HandleGetAllInput(AgentSML* pAgentSML, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, soarxml::ElementXML* pResponse) ;
	bool HandleGetAllOutput(AgentSML* pAgentSML, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, soarxml::ElementXML* pResponse) ;
	bool HandleGetRunState(AgentSML* pAgentSML, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, soarxml::ElementXML* pResponse) ;
	bool HandleIsProductionLoaded(AgentSML* pAgentSML, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, soarxml::ElementXML* pResponse) ;
	bool HandleSendClientMessage(AgentSML* pAgentSML, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, soarxml::ElementXML* pResponse) ;
	bool HandleWasAgentOnRunList(AgentSML* pAgentSML, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, soarxml::ElementXML* pResponse) ;
	bool HandleGetResultOfLastRun(AgentSML* pAgentSML, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, soarxml::ElementXML* pResponse) ;
	bool HandleGetInitialTimeTag(AgentSML* pAgentSML, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, soarxml::ElementXML* pResponse) ;
	bool HandleConvertIdentifier(AgentSML* pAgentSML, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, soarxml::ElementXML* pResponse) ;
	bool HandleGetListenerPort(AgentSML* pAgentSML, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, soarxml::ElementXML* pResponse) ;
	bool HandleGetLibraryLocation(AgentSML* pAgentSML, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, soarxml::ElementXML* pResponse) ;

	// Note: Register and unregister are both sent to this one handler
	bool HandleRegisterForEvent(AgentSML* pAgentSML, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, soarxml::ElementXML* pResponse) ;

};

}

#endif // SML_KERNEL_H
