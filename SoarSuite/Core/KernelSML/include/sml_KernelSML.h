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

#ifdef _WIN32
#include <crtdbg.h>
#endif

// Forward declarations
namespace gSKI {
	class IKernelFactory ;
	class IKernel ;
	class IAgent ;
	class IInputProducer ;
	class IOutputProcessor ;
	class OutputListener ;
	class IWme ;
	struct Error ;
}

#ifdef _MSC_VER
#pragma warning (disable : 4702)  // warning C4702: unreachable code, need to disable for VS.NET 2003 due to STL "bug" in certain cases
#endif
#include <map>
#ifdef _MSC_VER
#pragma warning (default : 4702)
#endif
#include <list>


#include "cli_CommandLineInterface.h"
#include "sml_SystemListener.h"
#include "sml_RhsListener.h"
#include "sml_AgentListener.h"
#include "sml_UpdateListener.h"
#include "sml_UntypedListener.h"

namespace soar_thread
{
	class Mutex ;
}

namespace sml {

// Forward declarations
class ElementXML ;
class Connection ;
class AnalyzeXML ;
class KernelSML ;
class OutputListener ;
class AgentSML; 
class ConnectionManager ;
class Events ;
class RunScheduler ;

// Define the CommandFunction which we'll call to process commands
typedef bool (KernelSML::*CommandFunction)(gSKI::IAgent*, char const*, Connection*, AnalyzeXML*, ElementXML*, gSKI::Error*);

// Used to store a map from command name to function handler for that command
typedef std::map< std::string, CommandFunction >	CommandMap ;
typedef CommandMap::iterator					CommandMapIter ;
typedef CommandMap::const_iterator				CommandMapConstIter ;

// List of input producers that we need to delete
typedef std::list< gSKI::IInputProducer* >	InputProducerList_t ;
typedef InputProducerList_t::iterator		InputProducerListIter_t ;

// List of output producers that we need to delete
typedef std::list< gSKI::IOutputProcessor* >	OutputProcessorList_t ;
typedef OutputProcessorList_t::iterator		OutputProcessorListIter_t ;

// List of output listeners that we need to delete
//typedef std::list< OutputListener* >			OutputListenerList_t ;
//typedef OutputListenerList_t::iterator		OutputListenerListIter_t ;

// Map from agent pointers to information we keep for SML about those agents.
typedef std::map< gSKI::IAgent*, AgentSML* >	AgentMap ;
typedef AgentMap::iterator					AgentMapIter ;
typedef AgentMap::const_iterator			AgentMapConstIter ;

class KernelSML
{
	// Allow the kernel listener to execute command lines directly
	//friend class KernelListener ;
	friend class AgentListener;
	friend class RhsListener;
	friend class RunScheduler ;

protected:
	// The singleton kernel object
	static KernelSML*	s_pKernel ;

	// On Windows this is set to the DLL's hModule handle.
	static void*		s_hModule ;

	// Map from command name to function to handle it
	CommandMap	m_CommandMap ;

	// Map from gSKI agent pointers to AgentSML objects, where we keep additional information
	// required for SML about each agent.
	AgentMap		m_AgentMap ;

	// Command line interface module
	cli::CommandLineInterface m_CommandLineInterface ;

	// The gSKI kernel objects
	gSKI::IKernelFactory*	m_pKernelFactory ;   
	gSKI::IKernel*			m_pIKernel ;

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

	// Used to shutdown a running system.  Not sure we really need to support this
	// but this is an attempt.
	class OnSystemStopDeleteAll ;
	OnSystemStopDeleteAll*	m_pSystemStopListener ;

	// If true, whenever a user issues a command that changes the state of the kernel in some manner
	// the command and its results are echoed to anyone listening.  This is useful when two users
	// are debugging the same kernel (and should be off at other times).
	bool			m_EchoCommands ;

public:
	/*************************************************************
	* @brief	Returns the singleton kernel object.
	*************************************************************/
	static KernelSML* GetKernelSML() ;

	/*************************************************************
	* @brief	Creates the singleton kernel object
	*			and starts listening for incoming commands on the
	*			given port.
	*************************************************************/
	static KernelSML* CreateKernelSML(unsigned short portToListenOn) ;

	/*************************************************************
	* @brief	Delete the singleton kernel object
	*************************************************************/
	static void DeleteSingleton()
	{
		if (s_pKernel)
			delete s_pKernel ;

		s_pKernel = 0 ;
	}

	/*************************************************************
	* @brief	The module handle (only set on Windows) can be
	*			used to determine the path to this DLL.
	*************************************************************/
	static void SetModuleHandle(void* hModule)	{ s_hModule = hModule ; }
	static void* GetModuleHandle()				{ return s_hModule ; }

	/*************************************************************
	* @brief	Handy utility function for dumping output to the
	*			debugger.
	*************************************************************/
	static void DebugPrint(char const* pFilename, int line, char const* pMsg)
	{
#ifdef _WIN32
#ifdef _DEBUG
		_CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG );
		_CrtSetReportFile( _CRT_WARN, _CRTDBG_FILE_STDOUT );

		_CrtDbgReport(_CRT_WARN, pFilename, line, "KernelSML", pMsg);
#else
		unused(pFilename) ;
		unused(line) ;
		unused(pMsg) ;
#endif
#endif
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
	void AddSystemListener(egSKISystemEventId eventID, Connection* pConnection)	 { m_SystemListener.AddListener(eventID, pConnection) ; }
	void AddAgentListener(egSKIAgentEventId eventID, Connection* pConnection)	 { m_AgentListener.AddListener(eventID, pConnection) ; }
	void AddUpdateListener(egSKIUpdateEventId eventID, Connection* pConnection)	 { m_UpdateListener.AddListener(eventID, pConnection) ; }
	void AddStringListener(egSKIStringEventId eventID, Connection* pConnection)  { m_StringListener.AddListener(eventID, pConnection) ; }
	void RemoveSystemListener(egSKISystemEventId eventID, Connection* pConnection) { m_SystemListener.RemoveListener(eventID, pConnection) ; }
	void RemoveAgentListener(egSKIAgentEventId eventID, Connection* pConnection)   { m_AgentListener.RemoveListener(eventID, pConnection) ; }
	void RemoveUpdateListener(egSKIUpdateEventId eventID, Connection* pConnection) { m_UpdateListener.RemoveListener(eventID, pConnection) ; }
	void RemoveStringListener(egSKIStringEventId eventID, Connection* pConnection) { m_StringListener.RemoveListener(eventID, pConnection) ; }

	/*************************************************************
	* @brief	Notify listeners that this event has occured.
	*************************************************************/
	void FireUpdateListenerEvent(egSKIUpdateEventId eventID, int runFlags)	{ m_UpdateListener.HandleEvent(eventID, runFlags) ; }

	/*************************************************************
	* @brief	Notify listeners that this event has occured.
	*************************************************************/
	void FireSystemEvent(egSKISystemEventId eventID)						{ m_SystemListener.HandleEvent(eventID, GetKernel()) ; }

	/*************************************************************
	* @brief	Notify listeners that this event has occured.
	*************************************************************/
	void FireEditProductionEvent(char const* pProduction) { m_StringListener.HandleEvent(gSKIEVENT_EDIT_PRODUCTION, pProduction) ; }

	/*************************************************************
	* @brief	Add or remove a connection from the list implementing
	*			a particular rhs function in the kernel.
	*************************************************************/
	void AddRhsListener(char const* pFunctionName, Connection* pConnection)	   { m_RhsListener.AddRhsListener(pFunctionName, pConnection) ; }
	void RemoveRhsListener(char const* pFunctionName, Connection* pConnection) { m_RhsListener.RemoveRhsListener(pFunctionName, pConnection) ; }

	/*************************************************************
	* @brief	Send this message out to any clients that are listening.
	*			These messages are from one client to another--kernelSML is just
	*			facilitating the message passing process without knowing/caring what is being passed.
	*************************************************************/
	std::string SendClientMessage(gSKI::IAgent* pAgent, char const* pMessageType, char const* pMessage) ;

	/*************************************************************
	* @brief	Send this command line out to all clients that have
	*			registered a filter.  The result is the processed
	*			version of the command line.
	*************************************************************/
	void KernelSML::SendFilterMessage(gSKI::IAgent* pAgent, char const* pCommandLine, std::string* pResult) ;

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
	ElementXML* ProcessIncomingSML(Connection* pConnection, ElementXML* pIncoming) ;

	/*************************************************************
	* @brief	Look up an agent from its name.
	*************************************************************/
	gSKI::IAgent* GetAgent(char const* pAgentName) ;

	/*************************************************************
	* @brief	Get the kernel object.
	*************************************************************/
	gSKI::IKernel* GetKernel() { return m_pIKernel ; }

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
	AgentSML*	GetAgentSML(gSKI::IAgent* pAgent) ;

	/*************************************************************
	* @brief	Returns the number of agents.
	*************************************************************/	
	int			GetNumberAgents() ;

	/*************************************************************
	* @brief	Delete the agent sml object for this agent.
	*			This object stores the data SML uses when working
	*			with the underlying gSKI agent.
	*************************************************************/	
	bool DeleteAgentSML(gSKI::IAgent* pAgent) ;

	/*************************************************************
	* @brief	Stops and deletes all agents.  Generally called
	*			just prior to shutdown.
	*************************************************************/	
	void DeleteAllAgents(bool waitTillDeleted) ;

	/*************************************************************
	* @brief	Enable/disable the print callback for a given agent.
	*			This allows us to use the print callback within the
	*			kernel without forwarding that output to clients
	*			(useful for capturing the output from some commands).
	*************************************************************/
	void DisablePrintCallback(gSKI::IAgent* pAgent) ;
	void EnablePrintCallback(gSKI::IAgent* pAgent)  ;

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
	void SetStopBefore(egSKIPhaseType phase) ;
	egSKIPhaseType GetStopBefore() ;

	/*************************************************************
	* @brief	If true, whenever a user issues a command that changes the state of the kernel in some manner
	*			the command and its results are echoed to anyone listening.  This is useful when two users
	*			are debugging the same kernel (and should be off at other times).
	*************************************************************/	
	void SetEchoCommands(bool state) { m_EchoCommands = state ; }
	bool GetEchoCommands()			 { return m_EchoCommands ; }

protected:
	KernelSML(unsigned short portToListenOn);

protected:
	/*************************************************************
	* @brief	Get the kernel factory object.
	*************************************************************/
	gSKI::IKernelFactory* GetKernelFactory() { return m_pKernelFactory ; }

	/*************************************************************
	* @brief	Return an object* to the caller.
	*************************************************************/
	bool ReturnResult(Connection* pConnection, ElementXML* pResponse, char const* pResult) ;

	/*************************************************************
	* @brief	Return an integer result to the caller.
	*************************************************************/
	bool ReturnIntResult(Connection* pConnection, ElementXML* pResponse, int result) ;

	/*************************************************************
	* @brief	Return a boolean result to the caller.
	*************************************************************/
	bool ReturnBoolResult(Connection* pConnection, ElementXML* pResponse, bool result) ;

	/*************************************************************
	* @brief	Return an invalid argument error to the caller.
	*************************************************************/
	bool InvalidArg(Connection* pConnection, ElementXML* pResponse, char const* pCommandName, char const* pErrorDescription) ;

	void BuildCommandMap() ;

	bool ProcessCommand(char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse) ;

	// Add a value to working memory
	bool AddInputWME(gSKI::IAgent* pAgent, char const* pID, char const* pAttribute, char const* pValue, char const* pType, char const* pTimeTag, gSKI::Error* pError) ;

	// Remove a value from working memory.  The time tag is the string form of an int.
	bool RemoveInputWME(gSKI::IAgent* pAgent, char const* pTimeTag, gSKI::Error* pError) ;

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
	bool KernelSML::HandleCreateAgent(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError) ;
	bool KernelSML::HandleLoadProductions(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError) ;
	bool KernelSML::HandleGetInputLink(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError) ;
	bool KernelSML::HandleInput(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError) ;
	bool KernelSML::HandleCommandLine(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError) ;
	bool KernelSML::HandleExpandCommandLine(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError) ;
	bool KernelSML::HandleCheckForIncomingCommands(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError) ;
	bool KernelSML::HandleDestroyAgent(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError) ;
	bool KernelSML::HandleGetAgentList(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError) ;
	bool KernelSML::HandleSetInterruptCheckRate(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError) ;
	bool KernelSML::HandleFireEvent(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError) ;
	bool KernelSML::HandleSuppressEvent(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError) ;
	bool KernelSML::HandleGetVersion(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError) ;
	bool KernelSML::HandleShutdown(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError) ;
	bool KernelSML::HandleIsSoarRunning(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError) ;
	bool KernelSML::HandleSetConnectionInfo(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError) ;
	bool KernelSML::HandleGetConnections(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError) ;
	bool KernelSML::HandleGetAllInput(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError) ;
	bool KernelSML::HandleGetAllOutput(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError) ;
	bool KernelSML::HandleGetRunState(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError) ;
	bool KernelSML::HandleIsProductionLoaded(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError) ;
	bool KernelSML::HandleSendClientMessage(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError) ;
	bool KernelSML::HandleWasAgentOnRunList(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError) ;
	bool KernelSML::HandleGetResultOfLastRun(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError) ;

	// Note: Register and unregister are both sent to this one handler
	bool KernelSML::HandleRegisterForEvent(gSKI::IAgent* pAgent, char const* pCommandName, Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse, gSKI::Error* pError) ;

};

}

#endif // SML_KERNEL_H
