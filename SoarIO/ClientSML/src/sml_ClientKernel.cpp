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

using namespace sml ;

Kernel::Kernel(Connection* pConnection)
{
	m_Connection = pConnection ;
	m_TimeTagCounter = 0 ;
	m_IdCounter = 0 ;
}

Kernel::~Kernel(void)
{
	// When the agent map is deleted, it will delete its contents (the Agent objects)

	// We also need to close the connection
	if (m_Connection)
		m_Connection->CloseConnection() ;

	delete m_Connection ;
}

/*************************************************************
* @brief Creates a connection to the Soar kernel that is embedded
*        within the same process as the caller.
*
* @param pLibraryName	The name of the library to load, without an extension (e.g. "KernelSML").  Case-sensitive (to support Linux).
*						This library will be dynamically loaded and connected to.
*
* @returns A new kernel object which is used to communicate with the kernel (or NULL if an error occurs)
*************************************************************/
Kernel* Kernel::CreateEmbeddedConnection(char const* pLibraryName)
{
	ErrorCode errorCode = 0 ;
	Connection* pConnection = Connection::CreateEmbeddedConnection(pLibraryName, &errorCode) ;

	if (!pConnection)
		return NULL ;

	Kernel* pKernel = new Kernel(pConnection) ;
	return pKernel ;
}

/*************************************************************
* @brief Creates a connection to a receiver that is in a different
*        process.  The process can be on the same machine or a different machine.
*
* @param pIPaddress The IP address of the remote machine (e.g. "202.55.12.54").
*                   Pass "127.0.0.1" to create a connection between two processes on the same machine.
* @param port		The port number to connect to.  The default port for SML is 35353 (picked at random).
*
* @returns A new kernel object which is used to communicate with the kernel (or NULL if an error occurs)
*************************************************************/
Kernel* Kernel::CreateRemoteConnection(char const* pIPaddress, int port)
{
	ErrorCode errorCode = 0 ;

	// The remote part is still to be written.
	//Connection* pConnection = Connection::CreateRemoteConnection(pIPaddress, port, &errorCode) ;
	Connection* pConnection = NULL ;

	if (!pConnection)
		return NULL ;

	Kernel* pKernel = new Kernel(pConnection) ;
	return pKernel ;
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
	return (Agent*)m_AgentMap.find(pAgentName) ;
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
	if (GetConnection()->SendAgentCommand(&response, sml_Names::kCommand_CreateAgent, NULL, sml_Names::kParamName, pAgentName))
	{
		agent = new Agent(this, pAgentName) ;

		// Record this in our list of agents
		m_AgentMap.add(agent->GetName(), agent) ;
	}

	return agent ;
}

/*************************************************************
* @brief Process a command line command
*
* @param pCommandLine Command line string to process.
* @param pAgentName Agent name to apply the command line to.
* @returns The string form of output from the command.
*************************************************************/
const char* Kernel::ProcessCommandLine(char const* pCommandLine, char const* pAgentName)
{
	AnalyzeXML response;
	m_CommandLineSucceeded = GetConnection()->SendAgentCommand(&response, sml_Names::kCommand_CommandLine, pAgentName, sml_Names::kParamLine, pCommandLine);

	if (m_CommandLineSucceeded)
	{
		m_CommandLineResult = response.GetResultString();
	}
	else
	{
		m_CommandLineResult += "\nError Data:\n";
		m_CommandLineResult += response.GetErrorTag()->GetCharacterData();
	}

	return m_CommandLineResult.c_str();
}

/*************************************************************
* @brief Execute a command line command and return the result
*		 as an XML object.
*
* @param pCommandLine Command line string to process.
* @param pAgentName Agent name to apply the command line to.
* @returns The XML form of output from the command.
*          The caller must release this handle.
*************************************************************/
ElementXML_Handle Kernel::ProcessCommandLineXML(char const* pCommandLine, char const* pAgentName)
{
	return NULL ;
}

/*************************************************************
* @brief Execute a command line command and return the result
*		 as an XML object.
*
* @param pCommandLine Command line string to process.
* @param pAgentName Agent name to apply the command line to.
* @returns True if the command succeeds.
*************************************************************/
bool Kernel::ProcessCommandLineXML(char const* pCommandLine, char const* pAgentName, AnalyzeXML* pResponse)
{
	m_CommandLineSucceeded = GetConnection()->SendAgentCommand(pResponse, sml_Names::kCommand_CommandLine, pAgentName, sml_Names::kParamLine, pCommandLine);

	if (m_CommandLineSucceeded)
	{
		m_CommandLineResult = pResponse->GetResultString();
	}
	else
	{
		m_CommandLineResult += "\nError Data:\n";
		m_CommandLineResult += pResponse->GetErrorTag()->GetCharacterData();
	}

	return m_CommandLineSucceeded ;
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

