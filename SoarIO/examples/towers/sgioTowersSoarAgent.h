#ifndef SGIO_HANOI_SOAR_AGENT
#define SGIO_HANOI_SOAR_AGENT

#include "sgioTowers.h"

#include "sgio_agent.h"

class SGIOTowersSoarAgent
{
public:
	SGIOTowersSoarAgent(sgio::Agent* inAgent, HanoiWorld* inWorld);

	~SGIOTowersSoarAgent();

	void ProcessOutput(/*IWorkingMemory* wmemory, IWMObject* object*/);

	void MakeMove();

private:
	//IAgent* m_Agent;
	HanoiWorld* m_World;
	SGIOTowersSoarAgent(const SGIOTowersSoarAgent&);
	SGIOTowersSoarAgent operator=(const SGIOTowersSoarAgent&);
	SGIOTowersSoarAgent();
};

#endif //SGIO_HANOI_SOAR_AGENT