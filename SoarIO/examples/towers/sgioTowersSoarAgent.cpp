#include "sgioTowersSoarAgent.h"

using sgio::Agent;
//using sgio_towers::SoarAgent;
//namespace sgio_towers
//{

SoarAgent::SoarAgent(Agent* inAgent, HanoiWorld* inWorld)
{


}

SoarAgent::~SoarAgent()
{
	//m_Agent->


}



void SoarAgent::MakeMove()
{
	m_Agent->RunTilOutput();
}

//}//closes namespace