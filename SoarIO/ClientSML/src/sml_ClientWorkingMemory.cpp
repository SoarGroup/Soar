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

#include "sml_ClientWorkingMemory.h"
#include "sml_ClientSoarId.h"
#include "sml_ClientAgent.h"
#include "sml_Connection.h"

using namespace sml ;

WorkingMemory::WorkingMemory()
{
	m_InputLink = NULL ;
	m_Agent		= NULL ;
}

WorkingMemory::~WorkingMemory()
{
	delete m_InputLink ;
}

Connection* WorkingMemory::GetConnection() const
{
	return GetAgent()->GetConnection() ;
}

char const* WorkingMemory::GetAgentName() const
{
	return GetAgent()->GetName() ;
}

SoarId* WorkingMemory::GetInputLink()
{
	if (!m_InputLink)
	{
		AnalyzeXML response ;

		if (GetConnection()->SendAgentCommand(&response, sml_Names::kCommand_GetInputLink, GetAgentName()))
		{
			m_InputLink = new SoarId(GetAgent(), response.GetResultString()) ;
		}
	}

	return m_InputLink ;
}
