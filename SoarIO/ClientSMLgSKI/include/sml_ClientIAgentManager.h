/////////////////////////////////////////////////////////////////
// AgentManager class
//
// Author: Cory Dunham, Devvan Stokes, University of Michigan
// Date  : August 2004
//
//
//
//
/////////////////////////////////////////////////////////////////

#ifndef SML_CLIENT_IAGENT_MANAGER
#define SML_CLIENT_IAGENT_MANAGER

#include "sml_ClientObject.h"

enum sml_AgentManagerErrorCode
{
	AGENTMANAGER_ERROR_NONE = 0,
	AGENTMANAGER_ERROR_ONCREATE,
	AGENTMANAGER_ERROR_AGENT_NOT_FOUND,
	AGENTMANAGER_ERROR_AGENT_DELETION_FAILED,
	AGENTMANAGER_ERROR_AGENT_CREATION_FAILED,
	AGENTMANAGER_ERROR_ERROR_AGENT_RETRIEVAL_FAILED,
	AGENTMANAGER_ERROR_NULL_RESPONSE,
	AGENTMANAGER_ERROR_MALFORMEDXML
};

namespace sml
{

class IAgent ;

class IAgentManager : public ClientObject
{
public:
	virtual ~IAgentManager() { } ;

	virtual sml::IAgent* AddAgent(const char*       name, 
							const char*       prodFileName = 0, 
							bool              learningOn   = false,
							egSKIOSupportMode oSupportMode = gSKI_O_SUPPORT_MODE_4,
							gSKI::Error*            err          = 0)= 0 ;

	virtual sml::IAgent* GetAgent(const char* name, gSKI::Error* err = 0)= 0 ;

//	virtual gSKI::tIAgentIterator* GetAgentIterator(gSKI::Error* err = 0)= 0 ;

	virtual int GetNumberOfAgents() const = 0 ;

	virtual void RemoveAgent(sml::IAgent* agent)= 0 ;

//	virtual void RemoveAgent(sml::IAgent agent, gSKI::Error* err = 0)= 0 ;

//	virtual void RemoveAgentByName(const char* name, gSKI::Error* err = 0)= 0 ;
};

}//closes namespace

#endif //SML_CLIENT_AGENT_MANAGER