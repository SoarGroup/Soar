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
#include "sock_SocketLib.h"
#include "thread_Thread.h"	// To get to sleep
#include "EmbeddedSMLInterface.h" // for static reference

#include <assert.h>

using namespace sml ;

Kernel::Kernel(Connection* pConnection)
{
	m_Connection     = pConnection ;
	m_TimeTagCounter = 0 ;
	m_IdCounter      = 0 ;
	m_SocketLibrary  = NULL ;
	m_LastError		 = Error::kNoError ;

#ifdef LINUX_STATIC_LINK
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
	sml_ProcessMessage(0,0,0);
#endif
}

Kernel::~Kernel(void)
{
	// When the agent map is deleted, it will delete its contents (the Agent objects)
	// Do this before we delete the connection, in case we need to send things to the kernel
	// during clean up.
	m_AgentMap.clear() ;

	// We also need to close the connection
	if (m_Connection)
		m_Connection->CloseConnection() ;

	delete m_Connection ;

	// Deleting this shuts down the socket library if we were using it.
	delete m_SocketLibrary ;
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
* @brief This function is called (indirectly) when we receive a "call" SML
*		 message from the kernel.
*************************************************************/
ElementXML* Kernel::ProcessIncomingSML(Connection* pConnection, ElementXML* pIncomingMsg)
{
	// Analyze the message and find important tags
	AnalyzeXML msg ;
	msg.Analyze(pIncomingMsg) ;

	// Create a reply
	ElementXML* pResponse = pConnection->CreateSMLResponse(pIncomingMsg) ;

	// Get the "name" attribute from the <command> tag
	char const* pCommandName = msg.GetCommandName() ;

	// Look up the agent name parameter (most commands have this)
	char const* pAgentName = msg.GetArgValue(sml_Names::kParamAgent) ;

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

	return pResponse ;
}

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
Kernel* Kernel::CreateEmbeddedConnection(char const* pLibraryName, bool clientThread, bool optimized, int portToListenOn)
{
	ErrorCode errorCode = 0 ;
	Connection* pConnection = Connection::CreateEmbeddedConnection(pLibraryName, clientThread, optimized, portToListenOn, &errorCode) ;

	if (!pConnection)
		return NULL ;

	Kernel* pKernel = new Kernel(pConnection) ;

	// Transfer any errors over to the kernel object, so the caller can retrieve them.
	pKernel->SetError(pConnection->GetLastError()) ;

	// Register for "calls" from the client.
	pConnection->RegisterCallback(ReceivedCall, pKernel, sml_Names::kDocType_Call, true) ;

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
*
* @returns A new kernel object which is used to communicate with the kernel (or NULL if an error occurs)
*************************************************************/
Kernel* Kernel::CreateRemoteConnection(bool sharedFileSystem, char const* pIPaddress, int port)
{
	ErrorCode errorCode = 0 ;

	// Initialize the socket library before attempting to create a connection
	sock::SocketLib* pLib = new sock::SocketLib() ;

	// Connect to the remote socket
	Connection* pConnection = Connection::CreateRemoteConnection(sharedFileSystem, pIPaddress, (unsigned short)port, &errorCode) ;

	// BUGBUG: We need to return a decent error message here as this can easily fail.
	if (!pConnection)
	{
		delete pLib ;
		return NULL ;
	}

	Kernel* pKernel = new Kernel(pConnection) ;
	pKernel->SetSocketLib(pLib) ;

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
					pAgent = new Agent(this, pAgentName) ;

					// Record this in our list of agents
					m_AgentMap.add(pAgent->GetAgentName(), pAgent) ;
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
* @brief Looks up an agent by name (from our list of known agents).
*
* @returns A pointer to the agent (or NULL if not found).  This object
*		   is owned by the kernel and will be destroyed when the
*		   kernel is destroyed.
*************************************************************/
Agent* Kernel::GetAgent(char const* pAgentName)
{
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
	assert(GetConnection());
	if (GetConnection()->SendAgentCommand(&response, sml_Names::kCommand_CreateAgent, NULL, sml_Names::kParamName, pAgentName))
	{
		agent = new Agent(this, pAgentName) ;

		// Record this in our list of agents
		m_AgentMap.add(agent->GetAgentName(), agent) ;
	}

	// Set our error state based on what happened during this call.
	SetError(GetConnection()->GetLastError()) ;

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

	// BUGBUG? Should we delete the local object anyway even if destroy agent failed?
	return false ;
}

/*************************************************************
* @brief Process a command line command
*
* @param pCommandLine Command line string to process.
* @param pAgentName Agent name to apply the command line to.
* @returns The string form of output from the command.
*************************************************************/
char const* Kernel::ExecuteCommandLine(char const* pCommandLine, char const* pAgentName)
{
	AnalyzeXML response;
	bool wantRawOutput = true ;

	// Send the command line to the kernel
	m_CommandLineSucceeded = GetConnection()->SendAgentCommand(&response, sml_Names::kCommand_CommandLine, pAgentName, sml_Names::kParamLine, pCommandLine, wantRawOutput);

	if (m_CommandLineSucceeded)
	{
		// Get the result as a string
		m_CommandLineResult = response.GetResultString();
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
* @brief Execute a command line command and return the result
*		 as an XML object.
*
* @param pCommandLine Command line string to process.
* @param pAgentName Agent name to apply the command line to.
* @returns True if the command succeeds.
*************************************************************/
bool Kernel::ExecuteCommandLineXML(char const* pCommandLine, char const* pAgentName, AnalyzeXML* pResponse)
{
	if (!pCommandLine || !pResponse)
		return false ;

	m_CommandLineSucceeded = GetConnection()->SendAgentCommand(pResponse, sml_Names::kCommand_CommandLine, pAgentName, sml_Names::kParamLine, pCommandLine);

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
* @brief This is a utility wrapper to let us sleep the entire client process
*		 for a period of time.
*************************************************************/
void Kernel::Sleep(long milliseconds)
{
	soar_thread::Thread::SleepStatic(milliseconds) ;
}

