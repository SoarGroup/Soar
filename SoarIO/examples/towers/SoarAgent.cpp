#include "SoarAgent.h"


#ifdef USE_GSKI_DIRECT_NOT_SML
	//gSKI Directives
	//#include "IgSKI_InputProducer.h"
	#include "IgSKI_OutputProcessor.h"
	//#include "IgSKI_SymbolFactory.h"
	#include "IgSKI_InputLink.h"
	#include "IgSKI_OutputLink.h"
	//#include "IgSKI_WorkingMemory.h"
	//#include "gSKI.h"

	using namespace gSKI;
#else
	//SML Directives
	#include "sml_Client.h"
	using namespace sml;
#endif

SoarAgent::SoarAgent(IAgent* inAgent, HanoiWorld* inWorld) : m_Agent(inAgent), m_World(inWorld)
{
	// get input link
	IInputLink* pILink = m_Agent->GetInputLink();

	m_Agent->GetOutputLink()->SetAutomaticUpdate(true);
}

SoarAgent::~SoarAgent(){}

void SoarAgent::ProcessOutput(IWorkingMemory* wmemory, IWMObject* object)
{

	//read the the params

	//move disks
	//m_World->MoveDisk(,);

	m_World->Print();
}
