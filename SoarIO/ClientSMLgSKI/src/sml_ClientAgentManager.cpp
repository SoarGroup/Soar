#include "sml_ClientAgentManager.h"
#include "sml_StringOps.h"
#include "sml_Connection.h"

#include <iostream>
#include <string>

using std::cout; using std::endl; using std::string;

using namespace sml;

AgentManager::AgentManager(const char* pID, ClientSML* pClientSML)
{
	cout << "AgentManager::constructor beginning..." << endl;

	SetId(pID);
	SetClientSML(pClientSML) ;
}

AgentManager::~AgentManager()
{
	CleanUp() ;
	ClearError();
}

void AgentManager::CleanUp()
{
	// We'll delete each agent that's still in our map
	// Following gSKI accurately, we shouldn't do this and the user must release each agent explicitly,
	// but this seems to make sense for now.
	// Have to be careful with how we do this deletion, because the DestroyAgent calls
	// will modify the kernel map, so we'll do it like this:
	IAgent *pAgent = (IAgent*)m_AgentMap.getFirst() ;

	while (pAgent)
	{
		RemoveAgent(pAgent) ;
		pAgent = (IAgent*)m_AgentMap.getFirst() ;
	}

	// Make sure all are gone
	m_AgentMap.clear() ;
}

IAgent* AgentManager::AddAgent(const char* name,  const char* prodFileName, bool learningOn,
							   egSKIOSupportMode oSupportMode, gSKI::Error*)
{
	IAgent* existingAgent = GetAgent(name);

	if(existingAgent)
	{
		smlErrorCode = AGENTMANAGER_ERROR_AGENT_CREATION_FAILED;
		smlErrorDescription = "Creation failed because agent already existed.\n";
		return 0;//this agent already exists, and will not be created.
	}

	//create an agent interface
	else
	{
cout << "AgentManager::AddAgent:  Agent did not already exist..." << endl;

		AnalyzeXML response ;

		// BUGBUG: Need to include learningOn and oSupportMode in the parameter list here still.
		if (!GetConnection()->SendClassCommand(&response, sml_Names::kgSKI_IAgentManager_AddAgent,
			GetId(),
			sml_Names::kParamName, name,
			sml_Names::kParamFilename, prodFileName))
			return NULL ;

		//Create agent
		existingAgent = new Agent(response.GetResultString(), GetClientSML());

		cout << "AgentManager::CreateAgent: successful Agent construction..." << endl;

		// Only add to the map if we are properly constructed
		if (existingAgent->GetName())
			m_AgentMap.add(existingAgent) ;
	}

	return existingAgent;
}


IAgent* AgentManager::GetAgent(const char* agentName, gSKI::Error* err)
{
	return (IAgent*)m_AgentMap.lookup(agentName) ;
}


void AgentManager::RemoveAgent(IAgent* agent)
{
	// We have to delete our local IAgent first, so it can do any clean up it
	// wishes (possibly cleaning up real objects in the kernel) and then
	// we can ask the kernel to remove the real IAgent object.
	std::string id = agent->GetName() ;

	m_AgentMap.remove(id.c_str()) ;

	AnalyzeXML response ;
	GetConnection()->SendClassCommand(&response, sml_Names::kgSKI_IAgentManager_RemoveAgent, GetId(),
		sml_Names::kParamAgent, id.c_str()) ;
}
