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
#ifndef SML_AGENT_H
#define SML_AGENT_H

#include "sml_ClientWorkingMemory.h"

#include <string>

namespace sml {

class Kernel ;
class Connection ;

class Agent
{
protected:
	// We maintain a local copy of working memory so we can just send changes
	WorkingMemory	m_WorkingMemory ;

	// The kernel that owns this agent
	Kernel*			m_Kernel ;

	// The name of this agent
	std::string		m_Name ;

public:
	Agent(Kernel* pKernel, char const* pAgentName);

	virtual ~Agent();

	Connection* GetConnection() const ;
	char const* GetName() const		{ return m_Name.c_str() ; }
	WorkingMemory* GetWM() 			{ return &m_WorkingMemory ; } 

	/*************************************************************
	* @brief Load a set of productions from a file.
	*
	* The file must currently be on a filesystem that the kernel can
	* access (i.e. can't send to a remote PC unless that PC can load
	* this file).
	*
	* @returns True if finds file to load successfully.
	*************************************************************/
	bool LoadProductions(char const* pFilename) ;

	/*************************************************************
	* @brief Returns the id object for the input link.
	*		 The agent retains ownership of this object.
	*************************************************************/
	SoarId*		   GetInputLink() ;

//	StringElement* CreateStringWME(SoarId* parent, char const* pAttribute, char const* pValue);

};

}//closes namespace

#endif //SML_AGENT_H