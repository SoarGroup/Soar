#include "gSKITowers.h"
#include "gSKITowersSoarAgent.h"

#include <string>

#include <cassert>

#define testingGTSAgent
#ifdef testingGTSAgent
	#include <iostream>
	using std::cout; using std::endl;
#endif //testingGTSAgent

using std::string;

#include "IgSKI_OutputProcessor.h"
#include "IgSKI_InputLink.h"
#include "IgSKI_OutputLink.h"
#include "IgSKI_Agent.h"
#include "IgSKI_WMObject.h"
#include "IgSKI_Wme.h"
#include "IgSKI_Symbol.h"
#include "IgSKI_WorkingMemory.h"

using namespace gSKI;
//using namespace gski_towers;

SoarAgent::SoarAgent(IAgent* inAgent, HanoiWorld* inWorld) : m_Agent(inAgent), m_World(inWorld)
{
	// get input link
	IInputLink* pILink = m_Agent->GetInputLink();

	m_Agent->GetOutputLink()->SetAutomaticUpdate(true);
}

SoarAgent::~SoarAgent()
{
	// fixme todo implement this 
}

void SoarAgent::ProcessOutput(IWorkingMemory* wMemory, gSKI::IWMObject* moveIdentifier)
{
	string sourceTowerString;
	string destinationTowerString;
	int sourceTowerNum;
	int destinationTowerNum;
	IWme* wmeValue;

	//read the the params
	tIWmeIterator* sourceWMEIterator = moveIdentifier->GetWMEs("source-peg");
	tIWmeIterator* destinationWMEIterator = moveIdentifier->GetWMEs("destination-peg");

	// Get the agent's output decision
	if (sourceWMEIterator->IsValid() && destinationWMEIterator->IsValid())
	{
		// Get the row value
		wmeValue = sourceWMEIterator->GetVal();
		sourceTowerString = wmeValue->GetValue()->GetString();
		// Get the col value
		wmeValue = destinationWMEIterator->GetVal();
		destinationTowerString = wmeValue->GetValue()->GetString();
		if(destinationTowerString == "A")
			destinationTowerNum = 0;
		else if(destinationTowerString == "B")
			destinationTowerNum = 1;
		else
			destinationTowerNum = 2;

		if(sourceTowerString == "A")
			sourceTowerNum = 0;
		else if(sourceTowerString == "B")
			sourceTowerNum = 1;
		else
			sourceTowerNum = 2;

		// move disk in game environment
		if(m_World->MoveDisk(sourceTowerNum, destinationTowerNum) == true)
			wMemory->AddWmeString(moveIdentifier, "status", "complete");
		else
			wMemory->AddWmeString(moveIdentifier, "status", "error");
	}
	else
	{
		wMemory->AddWmeString(moveIdentifier, "status", "error");
	}
}

void SoarAgent::MakeMove()
{										//gSKI_RUN_UNTIL_OUTPUT		//gSKI_RUN_DECISION_CYCLE //
	egSKIRunResult runResult = m_Agent->RunInClientThread(gSKI_RUN_UNTIL_OUTPUT, 1);
	assert(runResult != gSKI_RUN_ERROR);
}