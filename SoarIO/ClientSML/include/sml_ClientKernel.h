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
#include "sml_ListMap.h"
#include "sml_ClientEvents.h"

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
class AnalyzeXML ;
class ElementXML ;

struct SystemEventHandlerPlusData
{
	SystemEventHandler  m_Handler ;
	void*				m_UserData ;
	int					m_CallbackID ;

	SystemEventHandlerPlusData(SystemEventHandler handler, void* userData, int callbackID)
	{
		m_Handler = handler ;
		m_UserData = userData ;
		m_CallbackID = callbackID ;
	}
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
	typedef sml::ListMap<smlSystemEventId, SystemEventHandlerPlusData>		SystemEventMap ;

	Connection*			m_Connection ;
	ObjectMap<Agent*>	m_AgentMap ;
	std::string			m_CommandLineResult;
	bool				m_CommandLineSucceeded ;
	sock::SocketLib*	m_SocketLibrary ;

	SystemEventMap		m_SystemEventMap ;

	// To create a kernel object, use one of the static methods, e.g. Kernel::CreateEmbeddedConnection().
	Kernel(Connection* pConnection);

	/*************************************************************
	* @brief Returns the connection information for this kernel
	*		 which is how we communicate with the kernel (e.g. embedded,
	*		 remotely over a socket etc.)
	*************************************************************/
	Connection* GetConnection() const { return m_Connection ; }

	void SetSocketLib(sock::SocketLib* pLibrary) { m_SocketLibrary = pLibrary ; }

	long	GenerateNextID()		{ return ++m_IdCounter ; }
	long	GenerateNextTimeTag()	{ return --m_TimeTagCounter ; }	// Count down so different from Soar kernel

	/*************************************************************
	* @brief This function is called when an event is received
	*		 from the Soar kernel.
	*
	* @param pIncoming	The event command
	* @param pResponse	The reply (no real need to fill anything in here currently)
	*************************************************************/
	void ReceivedEvent(AnalyzeXML* pIncoming, ElementXML* pResponse) ;
	void ReceivedSystemEvent(smlSystemEventId id, AnalyzeXML* pIncoming, ElementXML* pResponse) ;

public:
	/*************************************************************
	* @brief Creates a connection to the Soar kernel that is embedded
	*        within the same process as the caller.
	*
	* @param pLibraryName	The name of the library to load, without an extension (e.g. "KernelSML").  Case-sensitive (to support Linux).
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
	static Kernel* CreateEmbeddedConnection(char const* pLibraryName, bool clientThread, bool optimized = true, int portToListenOn = kDefaultSMLPort) ;

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
	*
	* @returns A new kernel object which is used to communicate with the kernel
	*		   If an error occurs a Kernel object is still returned.  Call "HadError()" and "GetLastErrorDescription()" on it.
	*************************************************************/
	static Kernel* CreateRemoteConnection(bool sharedFileSystem, char const* pIPaddress, int port = kDefaultSMLPort) ;

	/*************************************************************
	* @brief Turning this on means we'll start dumping output about messages
	*		 being sent and received.  Currently this only applies to remote connections.
	*************************************************************/
	void SetTraceCommunications(bool state) ;

	virtual ~Kernel();

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
	* @brief Process a command line command and return the result
	*        as a string.
	*
	* @param pCommandLine Command line string to process.
	* @param pAgentName Agent name to apply the command line to.
	* @returns The string form of output from the command.
	*************************************************************/
	char const* ExecuteCommandLine(char const* pCommandLine, char const* pAgentName) ;

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
	bool ExecuteCommandLineXML(char const* pCommandLine, char const* pAgentName, AnalyzeXML* pResponse) ;

	/*************************************************************
	* @brief Get last command line result
	*
	* @returns True if the last command line call succeeded.
	*************************************************************/
	bool GetLastCommandLineResult();

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
	int	RegisterForSystemEvent(smlSystemEventId id, SystemEventHandler handler, void* pUserData) ;

	/*************************************************************
	* @brief Unregister for a particular event
	*************************************************************/
	void	UnregisterForSystemEvent(smlSystemEventId id, int callbackID) ;

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

};

}//closes namespace

#endif //SML_KERNEL_H
