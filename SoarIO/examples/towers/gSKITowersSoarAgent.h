#ifndef GSKI_HANOI_SOAR_AGENT
#define GSKI_HANOI_SOAR_AGENT

#include "AgnosticTowers.h"
#include "gSKI_OutputProcessor.h"

class IAgent;
class HanoiWorld;
class IWMObject;
class IWorkingMemory;


class SoarAgent : public IOutputProcessor
{
public:
	SoarAgent(IAgent* inAgent, HanoiWorld* inWorld);

	~SoarAgent();

	void ProcessOutput(IWorkingMemory* wmemory, IWMObject* object);

	void MakeMove();

private:
	IAgent* m_Agent;
	HanoiWorld* m_World;
	SoarAgent(const SoarAgent&);
	SoarAgent operator=(const SoarAgent&);
	SoarAgent();
};

#endif //GSKI_HANOI_SOAR_AGENT