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

#ifndef SML_CLIENT_AGENT_MANAGER
#define SML_CLIENT_AGENT_MANAGER

#include "sml_ClientError.h"
#include "sml_ClientAgent.h"
#include "sml_ClientKernel.h"
#include "sml_ClientIAgentManager.h"
#include "sml_IdMap.h"

#include "IgSKI_Iterator.h"
#include "gSKI_Structures.h"
#include "gSKI_Enumerations.h"

namespace sml
{

class AgentManager : public IAgentManager
{
public:

	AgentManager(const char* pID, ClientSML* pClientSML);

	virtual ~AgentManager();

	sml::IAgent* AddAgent(const char*       name, 
							const char*       prodFileName = 0, 
							bool              learningOn   = false,
							egSKIOSupportMode oSupportMode = gSKI_O_SUPPORT_MODE_4,
							gSKI::Error*            err          = 0);

	sml::IAgent* GetAgent(const char* name, gSKI::Error* err = 0);

//	gSKI::tIAgentIterator* GetAgentIterator(gSKI::Error* err = 0);

	int GetNumberOfAgents() const {	return m_AgentMap.size() ; }

	virtual void RemoveAgent(sml::IAgent* agent);

//	void RemoveAgent(sml::Agent agent, gSKI::Error* err = 0);

//	void RemoveAgentByName(const char* name, gSKI::Error* err = 0);

protected:
	void CleanUp() ;

	IdMap	m_AgentMap ;

//	sml::IKernel* const pKernel;
};

}//closes namespace

#endif //SML_CLIENT_AGENT_MANAGER