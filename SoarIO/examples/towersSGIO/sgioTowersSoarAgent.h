#ifndef SGIO_HANOI_SOAR_AGENT
#define SGIO_HANOI_SOAR_AGENT

#include "sgio_agent.h"
#include "sgio_wmemem.h"

class HanoiWorld;
//namespace sgio_towers
//{

class SoarAgent
{
public:
	SoarAgent(sgio::Agent* inAgent, sgio::WorkingMemory* inWMemory, HanoiWorld* inWorld);

	~SoarAgent();

	void MakeMove();

private:

	SoarAgent(const SoarAgent&);
	SoarAgent operator=(const SoarAgent&);
	SoarAgent();

	sgio::Agent* pAgent;
	sgio::WorkingMemory* pWMemory;
	HanoiWorld* pWorld;
};


//}// closes namespace

#endif //SGIO_HANOI_SOAR_AGENT