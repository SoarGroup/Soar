#ifndef HANOI_SOAR_AGENT
#define HANOI_SOAR_AGENT

#include "Towers.h"

//gSKI directives
#ifdef USE_GSKI_DIRECT_NOT_SML
	//#include "IgSKI_InputProducer.h"
	#include "IgSKI_OutputProcessor.h"
	#include "IgSKI_Agent.h"
	//#include "IgSKI_SymbolFactory.h"
	//#include "IgSKI_InputLink.h"
	//#include "IgSKI_OutputLink.h"
	#include "IgSKI_WorkingMemory.h"
	//#include "gSKI.h"
	using namespace gSKI;
//	using gSKI::IOutputProcessor;


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

private:
	IAgent* m_Agent;
	HanoiWorld* m_World;
	SoarAgent(const SoarAgent&);
	SoarAgent operator=(const SoarAgent&);
	SoarAgent();
};

#endif //HANOI_SOAR_AGENT