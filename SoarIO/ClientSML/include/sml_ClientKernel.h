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

class Kernel
{
	// Allow the agent to call to get the connection from the kernel.
	friend class Agent ;

public:
	enum { kDefaultSMLPort = 12121 } ;

protected:
	long		m_TimeTagCounter ;	// Used to generate time tags (we do them in the kernel not the agent, so ids are unique for all agents)
	long		m_IdCounter ;		// Used to generate unique id names

	Connection*			m_Connection ;
	ObjectMap<Agent*>	m_AgentMap ;
	std::string			m_CommandLineResult;
	bool				m_CommandLineSucceeded ;
	sock::SocketLib*	m_SocketLibrary ;

	// To create a kernel object, use one of the static methods, e.g. Kernel::CreateEmbeddedConnection().
	Kernel(Connection* pConnection);

	/*************************************************************
	* @brief Returns the connection information for this kernel
	*		 which is how we communicate with the kernel (e.g. embedded,
	*		 remotely over a socket etc.)
	*************************************************************/
	Connection* GetConnection() const { return m_Connection ; }

	void SetSocketLib(sock::SocketLib* pLibrary) { m_SocketLibrary = pLibrary ; }

public:
	/*************************************************************
	* @brief Creates a connection to the Soar kernel that is embedded
	*        within the same process as the caller.
	*
	* @param pLibraryName	The name of the library to load, without an extension (e.g. "KernelSML").  Case-sensitive (to support Linux).
	*						This library will be dynamically loaded and connected to.
	* @param SynchronousExecution	If true, Soar will run in the client's thread and the client must periodically call over to the
	*								kernel to check for incoming messages on remote sockets.
	*								If false, Soar will run in a thread within the kernel and that thread will check the incoming sockets itself.
	*								However, this asynchronous model requires a context switch whenever commands are sent to/from the kernel.
	*
	* @returns A new kernel object which is used to communicate with the kernel (or NULL if an error occurs)
	*************************************************************/
	static Kernel* CreateEmbeddedConnection(char const* pLibraryName, bool synchronousExecution) ;

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
	* @returns A new kernel object which is used to communicate with the kernel (or NULL if an error occurs)
	*************************************************************/
	static Kernel* CreateRemoteConnection(bool sharedFileSystem, char const* pIPaddress, int port = kDefaultSMLPort) ;

	virtual ~Kernel();

	long	GenerateNextID()		{ return ++m_IdCounter ; }
	long	GenerateNextTimeTag()	{ return --m_TimeTagCounter ; }	// Count down so different from Soar kernel

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

	// This call gives some cycles to the Tcl debugger.  It should come out eventually.
	bool CheckForIncomingCommands() ;

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
