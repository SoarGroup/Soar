#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "smlTowersSoarAgent.h"
#include "smlTowers.h"

#include <cassert>

//testing directives, fixme 
#include <iostream>
#include <string>
using std::cout; using std::endl; using std::string;

/*
#include "sgio_command.h"

using sgio::Agent;
using sgio::WorkingMemory;
*/
//using sgio_towers::SoarAgent;
//namespace sgio_towers
//{

SoarAgent::SoarAgent(Agent* inAgent, Agent* inWMemory, HanoiWorld* inWorld) : pAgent(inAgent), pWMemory(inWMemory),
																						pWorld(inWorld)
{

}

SoarAgent::~SoarAgent()
{
	// We need to explicitly destroy the agent.
	// Otherwise, if we're using a remote connection it will keep running
	// (by design, as the embedded connection will still be alive).
	// If this is an embedded connection it doesn't matter if we destroy this
	// or not as we're just about to unload the kernel itself, shutting everything down.
	pAgent->GetKernel()->DestroyAgent(pAgent) ;

	pAgent = 0;
	pWMemory = 0;
	pWorld = 0;
}


int TowerStringToInt(string& sourceString)
{
	if(sourceString == "A" || sourceString == "|A|")
		return 0;
	else if(sourceString == "B" || sourceString == "|B|")
		return 1;
	else if(sourceString == "C" || sourceString == "|C|")
		return 2;

	else cout << "got unexpected tower name: " << sourceString << endl;
	assert(0);
	return -99;
}

void MyPrintEventHandler(smlPrintEventId id, void* pUserData, Agent* pAgent, char const* pMessage)
{
	// In this case the user data is a string we're building up
	std::string* pTrace = (std::string*)pUserData ;

	(*pTrace) += pMessage ;
}

void SoarAgent::MakeMove()
{
	// SGIO takes no arguments.  SML pass in max decisions before stop (so it's clear this is how RunTilOutput works)
//	pAgent->RunTilOutput();

	const bool collectDebugInfo = false ;
	int callbackp ;

	// When debugging this it can be nice to capture the trace output to see what's happening.
	if (collectDebugInfo)
	{
		std::string trace ;	// We'll pass this into the handler and build up the output in it
		callbackp = pAgent->RegisterForPrintEvent(smlEVENT_PRINT, MyPrintEventHandler, &trace) ;
		pAgent->ExecuteCommandLine("watch 1") ;
	}

	std::string output = pAgent->RunSelfTilOutput();

	if (collectDebugInfo)
		pAgent->UnregisterForPrintEvent(callbackp) ;

	assert(pAgent->Commands());

	if (!pAgent->Commands())
	{
		cout << "Failed to receive command from Soar when one was expected." << endl ;
		return ;
	}

	string sourceTower, destinationTower = "";

	// SGIO uses a specific command object.  SML returns a standard WME for the identifier that represents the command.
//	std::auto_ptr<sgio::Command>cmd = pAgent->GetCommand();
	Identifier* cmd = pAgent->GetCommand(0) ;

	std::string name = cmd->GetCommandName();

	// SGIO returns std::string objects.  SML returns a char const*.
//	sourceTower = cmd->GetParameterValue("source-peg").c_str();
//	destinationTower = cmd->GetParameterValue("destination-peg").c_str();
	sourceTower = cmd->GetParameterValue("source-peg");
	destinationTower = cmd->GetParameterValue("destination-peg");

	if(name == "move-disk" && sourceTower != "" && destinationTower != "")
		cmd->AddStatusComplete();
	else
		cmd->AddStatusError();

	pWorld->MoveDisk(TowerStringToInt(sourceTower), TowerStringToInt(destinationTower));
}

//}//closes namespace
