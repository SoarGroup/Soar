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
* @param pResult BADBAD: I don't know how else to return a string to the client
*                so I'm currently returning an STL string.
*************************************************************/
bool Kernel::ProcessCommandLine(char const* pCommandLine, char const* pAgentName, char* pResult, size_t resultSize) {

	AnalyzeXML response;
	std::string result;
	bool ret = GetConnection()->SendAgentCommand(&response, sml_Names::kCommand_CommandLine, pAgentName, sml_Names::kParamLine, pCommandLine);

	if (pResult) {
		result = response.GetResultString();
	}

	if (!ret) {
		if (pResult) {
			result += "\nError Data:\n";
			result += response.GetErrorTag()->GetCharacterData();
		}
	}

	pResult = strncpy(pResult, result.c_str(), resultSize);
	*(pResult+resultSize-1) = NULL; //make sure string is NULL terminated; this doesn't happen automatically if the string is truncated

	return ret;
}
