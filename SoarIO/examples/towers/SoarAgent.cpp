#include "SoarAgent.h"
#include <string>
#include <iostream> //just fore testing
#include <cassert>
using std::cout; using std::endl;

#ifdef USE_GSKI_DIRECT_NOT_SML
	//gSKI Directives

	#include "IgSKI_OutputProcessor.h"
	#include "IgSKI_InputLink.h"
	#include "IgSKI_OutputLink.h"

	using namespace gSKI;
#else
	//SML Directives
	#include "sml_Client.h"
	using namespace sml;
#endif

using std::string;

SoarAgent::SoarAgent(IAgent* inAgent, HanoiWorld* inWorld) : m_Agent(inAgent), m_World(inWorld)
{
	// get input link
	IInputLink* pILink = m_Agent->GetInputLink();

	m_Agent->GetOutputLink()->SetAutomaticUpdate(true);
}

SoarAgent::~SoarAgent(){}

void SoarAgent::ProcessOutput(IWorkingMemory* wMemory, IWMObject* moveIdentifier)
{
cout << "Processing Output....." << endl;
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
cout << "\tSource tower is: " << sourceTowerString << endl;
		// Get the col value
		wmeValue = destinationWMEIterator->GetVal();
		destinationTowerString = wmeValue->GetValue()->GetString();
cout << "\tDestination tower is: " << destinationTowerString << endl;
		if(destinationTowerString == "A")
		{
			destinationTowerNum = 0;
		}
		else if(destinationTowerString == "B")
		{
			destinationTowerNum = 1;
		}
		else
			destinationTowerNum = 2;

		if(sourceTowerString == "A")
		{
			sourceTowerNum = 0;
		}
		else if(sourceTowerString == "B")
		{
			sourceTowerNum = 1;
		}
		else
			sourceTowerNum = 2;

		// move disk in game environment
		if(m_World->MoveDisk(sourceTowerNum, destinationTowerNum) == true)
			wMemory->AddWmeString(moveIdentifier, "status", "complete");
		else
		{
			wMemory->AddWmeString(moveIdentifier, "status", "error");
cout << "SENT STATUS ERROR========================================" << endl;
		}
	}
	else
	{
		wMemory->AddWmeString(moveIdentifier, "status", "error");
cout << "SENT STATUS ERROR========================================" << endl;
	}

	m_World->Print();
}

void SoarAgent::MakeMove()
{										//gSKI_RUN_UNTIL_OUTPUT		//gSKI_RUN_DECISION_CYCLE //
	egSKIRunResult runResult = m_Agent->RunInClientThread(gSKI_RUN_UNTIL_OUTPUT, 1);
	cout << "Run result is: " <<  runResult << endl;
	assert(runResult != gSKI_RUN_ERROR);
}