#include "sgioTowersSoarAgent.h"
#include "sgioTowers.h"

#include <cassert>

//testing directives, fixme 
#include <iostream>
#include <string>
using std::cout; using std::endl; using std::string;

#include "sgio_command.h"
using sgio::Agent;
using sgio::WorkingMemory;
//using sgio_towers::SoarAgent;
//namespace sgio_towers
//{

SoarAgent::SoarAgent(Agent* inAgent, WorkingMemory* inWMemory, HanoiWorld* inWorld) : pAgent(inAgent), pWMemory(inWMemory),
																						pWorld(inWorld)
{

}

SoarAgent::~SoarAgent()
{
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

void SoarAgent::MakeMove()
{

	pAgent->RunTilOutput();

	assert(pAgent->Commands());

	string sourceTower, destinationTower = "";

	std::auto_ptr<sgio::Command>cmd = pAgent->GetCommand();
	std::string name = cmd->GetCommandName();
	sourceTower = cmd->GetParameterValue("source-peg").c_str();
	destinationTower = cmd->GetParameterValue("destination-peg").c_str();

	if(name == "move-disk" && sourceTower != "" && destinationTower != "")
		cmd->AddStatusComplete();
	else
		cmd->AddStatusError();

	pWorld->MoveDisk(TowerStringToInt(sourceTower), TowerStringToInt(destinationTower));
}

//}//closes namespace