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

