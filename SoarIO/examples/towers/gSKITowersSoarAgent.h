#ifndef GSKI_HANOI_SOAR_AGENT
#define GSKI_HANOI_SOAR_AGENT

#include "gSKITowers.h"
#include "IgSKI_OutputProcessor.h"
#include "IgSKI_Agent.h"

class HanoiWorld;

class SoarAgent : public gSKI::IOutputProcessor
{
public:
	SoarAgent(gSKI::IAgent* inAgent, HanoiWorld* inWorld);

	~SoarAgent();

	void ProcessOutput(gSKI::IWorkingMemory* wmemory, gSKI::IWMObject* object);

	void MakeMove();

private:
	gSKI::IAgent* m_Agent;
	HanoiWorld* m_World;
	SoarAgent(const SoarAgent&);
	SoarAgent operator=(const SoarAgent&);
	SoarAgent();
};

#endif //GSKI_HANOI_SOAR_AGENT