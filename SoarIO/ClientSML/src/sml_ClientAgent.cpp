/////////////////////////////////////////////////////////////////
// Agent class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : Sept 2004
//
// This class is used by a client app (e.g. an environment) to represent
// a Soar agent and to send commands and I/O to and from that agent.
//
/////////////////////////////////////////////////////////////////

#include "sml_ClientAgent.h"
#include "sml_ClientKernel.h"
#include "sml_Connection.h"
#include "sml_ClientIdentifier.h"

#include <cassert>
#include <string>

using namespace sml;

Agent::Agent(Kernel* pKernel, char const* pName)
{
	m_Kernel = pKernel ;
	m_Name	 = pName ;
	m_WorkingMemory.SetAgent(this) ;
}

Agent::~Agent()
{
}

Connection* Agent::GetConnection() const
{
	return m_Kernel->GetConnection() ;
}

/*************************************************************
* @brief Load a set of productions from a file.
*
* The file must currently be on a filesystem that the kernel can
* access (i.e. can't send to a remote PC unless that PC can load
* this file).
*************************************************************/
bool Agent::LoadProductions(char const* pFilename)
{
	AnalyzeXML response ;

	bool ok = GetConnection()->SendAgentCommand(&response, sml_Names::kCommand_LoadProductions, GetName(), sml_Names::kParamFilename, pFilename) ;
	return ok ;
}

/*************************************************************
* @brief Returns the id object for the input link.
*		 The agent retains ownership of this object.
*************************************************************/
Identifier* Agent::GetInputLink()
{
	return GetWM()->GetInputLink() ;
}

/*************************************************************
* @brief Builds a new WME that has a string value.
*		 The agent retains ownership of this object.
*		 The returned object is valid until the caller
*		 deletes the parent identifier.
*************************************************************/
StringElement* Agent::CreateStringWME(Identifier* parent, char const* pAttribute, char const* pValue)
{
	if (!parent)
		return NULL ;

	return GetWM()->CreateStringWME(parent, pAttribute, pValue) ;
}

/*************************************************************
* @brief Send the most recent list of changes to working memory
*		 over to the kernel.
*************************************************************/
bool Agent::Commit()
{
	return GetWM()->Commit() ;
}


