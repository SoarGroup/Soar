#include "sml_ClientAgent.h"
#include "sml_StringOps.h"
#include "sml_Connection.h"
#include "sml_ClientInputLink.h"
#include "sml_ClientOutputLink.h"

#include <cassert>
#include <string>

using namespace sml;
using std::string;

//devvan testing includes/using
#include <iostream>
using std::endl; using std::cout;

Agent::Agent(const char* pID, ClientSML* clientSML)
{
	SetId(pID) ;

	SetClientSML(clientSML) ;

	m_pInputLink  = NULL ;
	m_pOutputLink = NULL ;
}

Agent::~Agent()
{
	delete m_pInputLink ;
	delete m_pOutputLink ;

	ClearError();
}


/*void Agent::LoadProductions(const char* fileName)
{
 // fixme implement me
}*/

 // fixme implement me, or remove in favor of the two gski versions
void Agent::Run(unsigned long n, enum unit u)
{

}


egSKIRunResult Agent::RunInClientThread(egSKIRunType runLength, unsigned long count, gSKI::Error*)
{
	// Build up the run command.
	// NOTE: Don't copy this pattern for building a command -- it's very unusual
	// because this one takes int parameters.
	ElementXML* pMsg = GetConnection()->CreateSMLCommand(sml_Names::kgSKI_IAgent_RunInClientThread);

	//add the 'this' pointer parameter
	GetConnection()->AddParameterToSMLCommand(pMsg, sml_Names::kParamThis, GetId());

	// Add the runLength and count value.
	char buffer[100] ;
	sprintf(buffer, "%d", runLength) ;
	GetConnection()->AddParameterToSMLCommand(pMsg, sml_Names::kParamLength, buffer);

	sprintf(buffer, "%d", count) ;
	GetConnection()->AddParameterToSMLCommand(pMsg, sml_Names::kParamCount, buffer);

	// With this call, in an embedded connection, Soar runs for <n> cycles.
	// This function won't return until Soar stops running.
	// (On a remote connection it will return immediately).
	AnalyzeXML response ;
	egSKIRunResult result = gSKI_RUN_COMPLETED ;

	if (GetConnection()->SendMessageGetResponse(&response, pMsg))
	{
		char const* pID = response.GetResultString() ;

		if (pID)
		{
			result = (egSKIRunResult)atoi(pID) ;
		}
	}

	delete pMsg ;

 // fixme implement me
	return result; 
}

void Agent::Stop(char* reason)
{
 // fixme implement me
}

void Agent::SetWatchLevel(int watchLevel)
{
 // fixme implement me
}

void Agent::AddProduction(char* prod)
{
 // fixme implement me
}

void Agent::RemoveProduction(char* name)
{
 // fixme implement me
}

void Agent::RemoveProductions(enum productionType pt)
{
 // fixme implement me
}

void Agent::LoadRete(const char* fileName)
{
 // fixme implement me
}

void Agent::SaveRete(const char* fileName)
{
 // fixme implement me
}

void Agent::SendClassCommand(char const* pCommand)
{
 // fixme implement me
}
/*
ElementXML* Agent::GetResponse(bool wait)
{
	return 0;//fixme
}
*/
//	int RegisterEventHandler(Handler* function, enum Event event, optional int id);

void Agent::UnregisterEventHandler(enum Event event, int id)
{

}
/*
WME* Agent::CreateWME(IdentifierSymbol* id, Symbol* attribute, Symbol* value)
{
	return 0;//fixme
}

Symbol*   Agent::GetAttributes(IdentifierSymbol* id) const
{
	return 0;//fixme
}

Symbol*   Agent::GetValues(IdentifierSymbol* id, Symbol* attribute) const
{
	return 0;//fixme
}
*/
IInputLink* Agent::GetInputLink(gSKI::Error* pError)
{
	// Return the cached value if we've already got one.
	if (m_pInputLink)
		return m_pInputLink ;

	AnalyzeXML response ;
	
	if (GetConnection()->SendClassCommand(&response, sml_Names::kgSKI_IAgent_GetInputLink, GetId()))
	{
		m_pInputLink = new InputLink(response.GetResultString(), GetClientSML()) ;
	}

	return m_pInputLink ;
}

IOutputLink* Agent::GetOutputLink(gSKI::Error* pError)
{
	// Return the cached value if we've already got one.
	if (m_pOutputLink)
		return m_pOutputLink ;

	AnalyzeXML response ;
	if (GetConnection()->SendClassCommand(&response, sml_Names::kgSKI_IAgent_GetOutputLink, GetId()))
	{
		m_pOutputLink = new OutputLink(response.GetResultString(), GetClientSML()) ;
	}

	return m_pOutputLink ;
}

/*
IdentifierSymbol* Agent::GetInputLinkID() const
{
	return 0;//fixme
}

IdentifierSymbol* Agent::GetOutputLinkID() const
{
	return 0;//fixme
}

Symbol* Agent::GetParents(WME* wme) const
{
	return 0;//fixme
}

WME* Agent::AddWME(IdentifierSymbol* id, Symbol* attribute, Symbol* value)
{
	return 0;//fixme
}

void Agent::AddWME(WME* wme)
{

}

void Agent::RemoveWME(unsigned long inTimeTag)
{

}

void Agent::RemoveWME(WME* wme)
{

}

void Agent::UpdateWMEValue(WME* wme, Symbol* newValue)
{

}

void Agent::Commit()
{

}

void Agent::SetInputLink(WME* inputLink)
{

}

void Agent::SetOutputLink(WME* outputLink)
{

}
*/
