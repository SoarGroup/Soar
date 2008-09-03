//=======================================================================
//Agent Class
//
//Author:	Cory Dunham, Devvan Stokes, University of Michigan
//Date:		August 2004
//
//This class represents an interface to a Soar agent on the client
//side of an SML stream
//========================================================================
#ifndef SML_IAGENT_H
#define SML_IAGENT_H

#include "sml_ClientObject.h"

enum sml_AgentErrorCode
{
	AGENT_ERROR_NONE = 0,
	AGENT_ERROR_ONCREATE,
	AGENT_ERROR_MALFORMEDXML
};

namespace sml
{
class IInputLink ;
class IOutputLink ;

enum Event { foo }; // fixme , placeholder

class IAgent : public ClientObject
{
/*
	egSKIRunResult RunInSeparateThread(egSKIRunType runLength = gSKI_RUN_FOREVER, 
                                         unsigned long count = 1,
                                         gSKI::Error* err = 0);
*/

public:
	virtual ~IAgent() { } ;

	virtual const char* GetName(gSKI::Error* err = 0) const = 0 ;

	//this is only going to be available through the AgentManager
	//void LoadProductions(const char* fileName);
	
	virtual void Run(unsigned long n, enum unit u) = 0 ;//fixme, remove this?

	virtual egSKIRunResult RunInClientThread(egSKIRunType runLength = gSKI_RUN_FOREVER, 
                                       unsigned long count = 1,
                                       gSKI::Error* err = 0)= 0 ;

	virtual void Stop(char* reason)= 0 ;
	
	virtual void SetWatchLevel(int watchLevel)= 0 ;

	virtual void AddProduction(char* prod)= 0 ;

	virtual void RemoveProduction(char* name)= 0 ;

	virtual void RemoveProductions(enum productionType pt)= 0 ;

	virtual void LoadRete(const char* fileName)= 0 ;

	virtual void SaveRete(const char* fileName)= 0 ;
	
	virtual void SendClassCommand(char const* pCommand)= 0 ;

//	ElementXML* GetResponse(bool wait)= 0 ;

//	int RegisterEventHandler(Handler* function, enum Event event, optional int id)= 0 ;

	virtual void UnregisterEventHandler(enum Event event, int id)= 0 ;

	virtual sml::IInputLink* GetInputLink(gSKI::Error* err = 0)= 0 ;

	virtual sml::IOutputLink* GetOutputLink(gSKI::Error* err = 0)= 0 ;

	/*	
	virtual sml::WME* CreateWME(IdentifierSymbol* id, Symbol* attribute, Symbol* value)= 0 ;

	virtual sml::Symbol*   GetAttributes(IdentifierSymbol* id) const= 0 ;
	
	virtual sml::Symbol*   GetValues(IdentifierSymbol* id, Symbol* attribute) const= 0 ;
	
	virtual sml::IdentifierSymbol* GetInputLinkID() const= 0 ;
	
	virtual sml::IdentifierSymbol* GetOutputLinkID() const= 0 ;
	
	virtual sml::Symbol* GetParents(sml::WME* wme) const= 0 ;
	
	virtual sml::WME* AddWME(IdentifierSymbol* id, Symbol* attribute, Symbol* value)= 0 ;
	
	virtual void AddWME(sml::WME* wme)= 0 ;
	
	virtual void RemoveWME(unsigned long inTimeTag)= 0 ;
	
	virtual void RemoveWME(sml::WME* wme)= 0 ;

	virtual void UpdateWMEValue(sml::WME* wme, Symbol* newValue)= 0 ;
	
	virtual void Commit()= 0 ;
	
	virtual void SetInputLink(sml::WME* inputLink)= 0 ;

	virtual void SetOutputLink(sml::WME* outputLink)= 0 ;
*/
};

}//closes namespace

#endif //SML_IAGENT_H