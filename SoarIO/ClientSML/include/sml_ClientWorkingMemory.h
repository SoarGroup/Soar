/////////////////////////////////////////////////////////////////
// WorkingMemory class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : Sept 2004
//
// This class is used to represent Soar's working memory.
// We maintain a copy of this on the client so we can just
// send changes over to the kernel.
//
/////////////////////////////////////////////////////////////////
#ifndef SML_WORKING_MEMORY_H
#define SML_WORKING_MEMORY_H

#include "sml_ObjectMap.h"

namespace sml {

// Forward declarations
class Agent ;
class Connection ;
class StringElement ;
class SoarId ;

class WorkingMemory
{
protected:
	Agent*		m_Agent ;
	SoarId*		m_InputLink ;

public:
	WorkingMemory() ;

	virtual ~WorkingMemory();

	void			SetAgent(Agent* pAgent)	{ m_Agent = pAgent ; }
	Agent*			GetAgent() const		{ return m_Agent ; }
	char const*		GetAgentName() const ;
	Connection*		GetConnection()	const ;

	SoarId*		   GetInputLink() ;
	StringElement* CreateStringWME(SoarId* parent, char const* pAttribute, char const* pValue);

private:

};

}//closes namespace

#endif //SML_WORKING_MEMORY_H
