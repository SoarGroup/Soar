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
	// Don't want users creating and destroying agent objects without
	// going through the kernel
	friend Kernel ;
	friend WorkingMemory ;
	friend ObjectMap<Agent*> ;	// So can delete agent
	friend WMElement ;

protected:
	// We maintain a local copy of working memory so we can just send changes
	WorkingMemory	m_WorkingMemory ;

	// The kernel that owns this agent
	Kernel*			m_Kernel ;

	// The name of this agent
	std::string		m_Name ;

protected:
	Agent(Kernel* pKernel, char const* pAgentName);

	virtual ~Agent();

	Connection* GetConnection() const ;
	char const* GetName() const		{ return m_Name.c_str() ; }
	WorkingMemory* GetWM() 			{ return &m_WorkingMemory ; } 
	Kernel*		GetKernel() const	{ return m_Kernel ; }

public:

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
	Identifier* GetInputLink() ;

	/*************************************************************
	* @brief Builds a new WME that has a string value and schedules
	*		 it for addition to Soar's input link.
	*
	*		 The agent retains ownership of this object.
	*		 The returned object is valid until the caller
	*		 deletes the parent identifier.
	*		 The WME is not added to Soar's input link until the
	*		 client calls "Commit".
	*************************************************************/
	StringElement* CreateStringWME(Identifier* parent, char const* pAttribute, char const* pValue);

	/*************************************************************
	* @brief Same as CreateStringWME but for a new WME that has
	*		 an int as its value.
	*************************************************************/
	IntElement* CreateIntWME(Identifier* parent, char const* pAttribute, int value) ;

	/*************************************************************
	* @brief Same as CreateStringWME but for a new WME that has
	*		 a floating point value.
	*************************************************************/
	FloatElement* CreateFloatWME(Identifier* parent, char const* pAttribute, double value) ;

	/*************************************************************
	* @brief Same as CreateStringWME but for a new WME that has
	*		 an identifier as its value.
	*		 The identifier value is generated here and will be
	*		 different from the value Soar assigns in the kernel.
	*		 The kernel will keep a map for translating back and forth.
	*************************************************************/
	Identifier*		CreateIdWME(Identifier* parent, char const* pAttribute) ;

	/*************************************************************
	* @brief Update the value of an existing WME.
	*		 The value is not actually sent to the kernel
	*		 until "Commit" is called.
	*************************************************************/
	void	Update(StringElement* pWME, char const* pValue) ;
	void	Update(IntElement* pWME, int value) ;
	void	Update(FloatElement* pWME, double value) ;

	/*************************************************************
	* @brief Schedules a WME from deletion from the input link and removes
	*		 it from the client's model of working memory.
	*
	*		 The caller should not access this WME after calling
	*		 DestroyWME().
	*		 The WME is not removed from the input link until
	*		 the client calls "Commit"
	*************************************************************/
	bool	DestroyWME(WMElement* pWME) ;

	/*************************************************************
	* @brief Send the most recent list of changes to working memory
	*		 over to the kernel.
	*************************************************************/
	bool Commit() ;
};

}//closes namespace

#endif //SML_AGENT_H