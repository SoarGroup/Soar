#include "sgioTowersSoarAgent.h"
#include "sgioTowers.h"

#include <cassert>
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



void SoarAgent::MakeMove()
{
	pAgent->RunTilOutput();
	assert(pAgent->Commands());

	int sourceTower, destinationTower = -22;

	std::auto_ptr<sgio::Command>cmd = pAgent->GetCommand();
	std::string name = cmd->GetCommandName();
	sourceTower = atoi(cmd->GetParameterValue("source-peg").c_str());
	destinationTower = atoi(cmd->GetParameterValue("destination-peg").c_str());

	if(name == "move-disk" && sourceTower != -22 && destinationTower != -22)
		cmd->AddStatusComplete();
	else
		cmd->AddStatusError();

	pWorld->MoveDisk(sourceTower, destinationTower);
}

//}//closes namespace