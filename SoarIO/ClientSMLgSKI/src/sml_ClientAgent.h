//=======================================================================
//Agent Class
//
//Author:	Cory Dunham, Devvan Stokes, University of Michigan
//Date:		August 2004
//
//This class represents an interface to a Soar agent on the client
//side of an SML stream
//========================================================================
#ifndef SML_AGENT_H
#define SML_AGENT_H

#include "sml_ClientWME.h"
#include "sml_ClientIAgent.h"

namespace sml
{
	class ISymbol ;

class Agent : public IAgent
{
protected:
	IInputLink*		m_pInputLink ;
	IOutputLink*	m_pOutputLink ;

	egSKIRunResult RunInSeparateThread(egSKIRunType runLength = gSKI_RUN_FOREVER, 
                                         unsigned long count = 1,
                                         gSKI::Error* err = 0);

public:
	//really only want this to be accessible to Kernel, so it should probably be a friend
	Agent(const char* pID, ClientSML* pClientSML);

	virtual ~Agent();

	const char* GetName(gSKI::Error* err = 0) const {return GetId() ;}

	//this is only going to be available through the AgentManager
	//void LoadProductions(const char* fileName);
	
	void Run(unsigned long n, enum unit u);//fixme, remove this?

	egSKIRunResult RunInClientThread(egSKIRunType runLength = gSKI_RUN_FOREVER, 
                                       unsigned long count = 1,
                                       gSKI::Error* err = 0);

	void Stop(char* reason);
	
	void SetWatchLevel(int watchLevel);

	void AddProduction(char* prod);

	void RemoveProduction(char* name);

	void RemoveProductions(enum productionType pt);

	void LoadRete(const char* fileName);

	void SaveRete(const char* fileName);
	
	void SendClassCommand(char const* pCommand);

//	ElementXML* GetResponse(bool wait);

//	int RegisterEventHandler(Handler* function, enum Event event, optional int id);

	void UnregisterEventHandler(enum Event event, int id);

	virtual IInputLink* GetInputLink(gSKI::Error* err = 0);

	virtual IOutputLink* GetOutputLink(gSKI::Error* err = 0) ;

/*
	sml::WME* CreateWME(IdentifierSymbol* id, Symbol* attribute, Symbol* value);

	sml::Symbol*   GetAttributes(IdentifierSymbol* id) const;
	
	sml::Symbol*   GetValues(IdentifierSymbol* id, Symbol* attribute) const;
	
	sml::IdentifierSymbol* GetInputLinkID() const;
	
	sml::IdentifierSymbol* GetOutputLinkID() const;
	
	sml::Symbol* GetParents(sml::WME* wme) const;
	
	sml::WME* AddWME(IdentifierSymbol* id, Symbol* attribute, Symbol* value);
	
	void AddWME(sml::WME* wme);
	
	void RemoveWME(unsigned long inTimeTag);
	
	void RemoveWME(sml::WME* wme);

	void UpdateWMEValue(sml::WME* wme, Symbol* newValue);
	
	void Commit();
	
	void SetInputLink(sml::WME* inputLink);

	void SetOutputLink(sml::WME* outputLink);
*/
};

}//closes namespace

#endif //SML_AGENT_H