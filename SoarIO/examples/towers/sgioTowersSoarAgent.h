#ifndef SGIO_HANOI_SOAR_AGENT
#define SGIO_HANOI_SOAR_AGENT

class HanoiWorld;
#include "sgio_agent.h"

//namespace sgio_towers
//{

class SoarAgent
{
public:
	SoarAgent(sgio::Agent* inAgent, HanoiWorld* inWorld);

	~SoarAgent();

	void MakeMove();

private:
	sgio::Agent* m_Agent;
	HanoiWorld* m_World;
	SoarAgent(const SoarAgent&);
	SoarAgent operator=(const SoarAgent&);
	SoarAgent();
};


//}// closes namesapce

#endif //SGIO_HANOI_SOAR_AGENT