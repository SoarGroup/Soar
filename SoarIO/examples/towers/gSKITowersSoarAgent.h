#ifndef GSKI_HANOI_SOAR_AGENT
#define GSKI_HANOI_SOAR_AGENT

#include "Towers.h"

//gSKI directives
#ifdef USE_GSKI_DIRECT_NOT_SML

	#include "IgSKI_OutputProcessor.h"
	#include "IgSKI_Agent.h"
	#include "IgSKI_WorkingMemory.h"
	using namespace gSKI;

#else
	//SML Directives
	#include "sml_Client.h"
	using namespace sml;

#endif


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