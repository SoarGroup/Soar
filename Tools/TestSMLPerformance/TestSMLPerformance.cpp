// This test has two subtests.  For each, it creates 4 agents and runs them for 1000 decision cycles. 
// On each input-phase, 25 wmes are changed on the input-link.  The agents just execute the wait operator the entire time.
// The two subtests differ in the events they register for and what those events do.
// In the first subtest, the kernel is registered for the smlEVENT_AFTER_ALL_OUTPUT_PHASES update event,
// in which the environment and agents are both updated.
// In the second subtest, the kernel is still registered for the same update event, but only the environment is updated during that event.
// The agents are also registed for the smlEVENT_BEFORE_INPUT_PHASE event, which handles the agent updating.
// Thus, in the first subtest, a total of 999 events fire, whereas in the second subtest 3999 events fire.
//
// The number of agents to create, wmes to update, and decision cycles to run can be changed at the beginning of the main function.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include <portability.h>

#ifdef _MSC_VER
// Use Visual C++'s memory checking functionality
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif // _MSC_VER

#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include "sml_Client.h"
#include "sml_StringOps.h"

using namespace sml;
using namespace std;

void UpdateAgent(smlRunEventId id, void* pUserData, Agent* pAgent, smlPhase phase);
void PrintCallbackHandler(sml::smlPrintEventId id, void* pUserData, sml::Agent* pAgent, char const* pMessage);

class TestAgent {
	Agent* agent;
	vector<IntElement*>* wmes;
	int numWmes;

public:
	TestAgent(Kernel* kernel, char const* name, int nw) {
		numWmes = nw;
		wmes = new vector<IntElement*>;

		agent = kernel->CreateAgent(name);
		if(kernel->HadError()) {
			cout << kernel->GetLastErrorDescription();
		}

		// Register print event on the first agent so we'll see any errors that occur (for debugging)
		if(!strcmp(name,"0")) {
			agent->RegisterForPrintEvent(sml::smlEVENT_PRINT, PrintCallbackHandler, 0);
		}

		agent->LoadProductions("../Tests/TestSMLPerformance.soar");
		agent->ExecuteCommandLine("watch 0");

		// Create wmes
		Identifier* pInputLink = agent->GetInputLink();
		for(int i=0; i<numWmes; i++) {
			char buffer[25];
			Int2String(i, buffer, sizeof(buffer)) ;
			wmes->push_back(agent->CreateIntWME(pInputLink, buffer, 0));
		}
	}

	Agent* GetAgent() { return agent; }

	void Update(vector<int>* input) {
		// we always want to update
		agent->SetBlinkIfNoChange(true);

		// update each wme based on the new input
		for(int i=0; i<numWmes; i++) {
			agent->Update((*wmes)[i], (*input)[i]);
		}
		agent->Commit();
	}

	~TestAgent() {
		delete wmes;
	}
};

class Environment {
	vector<TestAgent*>* pAgents;
	vector< vector<int>* > inputs;
	int numAgents;
	int numWmes;

public:
	Environment(Kernel *kernel, int na, int nw) {
		numAgents = na;
		numWmes = nw;

		// Create agents
		pAgents = new vector<TestAgent*>();
		for(int i=0; i<numAgents; i++) {
			char buffer[25];
			Int2String(i, buffer, sizeof(buffer)) ;
			pAgents->push_back(new TestAgent(kernel, buffer, numWmes));
		}

		for(int agent=0; agent<numAgents; agent++) {
			vector<int>* input = new vector<int>();
			for(int wme=0; wme<numWmes; wme++) {
				input->push_back(rand());
			}
			inputs.push_back(input);
		}
	}

	void RegisterAgentsForInputEvent() {
		for(int i=0; i<numAgents; i++) {
			(*pAgents)[i]->GetAgent()->RegisterForRunEvent(smlEVENT_BEFORE_INPUT_PHASE, ::UpdateAgent, this);
		}
	}

	void UpdateEnvironment() {
		for(int agent=0; agent<numAgents; agent++) {
			for(int wme=0; wme<numWmes; wme++) {
				(*(inputs[agent]))[wme] = rand();
			}
		}
	}

	void UpdateAgent(int agentIndex) {
		(*pAgents)[agentIndex]->Update(inputs[agentIndex]);
	}

	void UpdateAll() {
		UpdateEnvironment();
		for(int i=0; i<numAgents; i++) {
			UpdateAgent(i);
		}
	}

	~Environment() {
		for(int i=0; i<numAgents; i++) {
			delete (*pAgents)[i];
			delete inputs[i];
		}
		delete pAgents;
	}

};

// we'll count the number of times these events occur
int numUpdateEvents = 0;
int numInputEvents = 0;

void ResetEventCounts() {
	numUpdateEvents = 0;
	numInputEvents = 0;
}

void UpdateEnvironmentAndAgents(smlUpdateEventId id, void* pUserData, Kernel* pKernel, smlRunFlags runFlags) {
	numUpdateEvents++;
	Environment* env = static_cast<Environment*>(pUserData);
	env->UpdateAll();
}

void UpdateEnvironment(smlUpdateEventId id, void* pUserData, Kernel* pKernel, smlRunFlags runFlags) {
	numUpdateEvents++;
	Environment* env = static_cast<Environment*>(pUserData);
	env->UpdateEnvironment();
}

void UpdateAgent(smlRunEventId id, void* pUserData, Agent* pAgent, smlPhase phase) {
	numInputEvents++;
	Environment* env = static_cast<Environment*>(pUserData);
	// the agent's name is it's index
	env->UpdateAgent(atoi(pAgent->GetAgentName()));
}

void PrintCallbackHandler(sml::smlPrintEventId id, void* pUserData, sml::Agent* pAgent, char const* pMessage) {
	cout << pMessage;
}

void RunTest1(int numAgents, int numWmes, int numCycles) {
	Kernel* kernel = Kernel::CreateKernelInNewThread("SoarKernelSML");
	if(kernel->HadError()) {
		cout << "Error: " << kernel->GetLastErrorDescription() << endl;
	}

	// go for maximum performance
	kernel->SetAutoCommit(false);

	// Create environment
	Environment* pEnv = new Environment(kernel, numAgents, numWmes);

	kernel->RegisterForUpdateEvent(smlEVENT_AFTER_ALL_OUTPUT_PHASES, UpdateEnvironmentAndAgents, pEnv);

	ostringstream oss;
	oss << "time run " << numCycles;
	const char* result = kernel->ExecuteCommandLine(oss.str().c_str(), "0");
	cout << "Time: " << result << endl;
	cout << "Num Update Events: " << numUpdateEvents << endl;
	cout << "Num Input Events : " << numInputEvents << endl;

	delete pEnv;

	kernel->Shutdown();
	delete kernel;
}

void RunTest2(int numAgents, int numWmes, int numCycles) {
	Kernel* kernel = Kernel::CreateKernelInNewThread("SoarKernelSML");
	if(kernel->HadError()) {
		cout << "Error: " << kernel->GetLastErrorDescription() << endl;
	}

	// go for maximum performance
	kernel->SetAutoCommit(false);

	// Create environment
	Environment* pEnv = new Environment(kernel, numAgents, numWmes);

	kernel->RegisterForUpdateEvent(smlEVENT_AFTER_ALL_OUTPUT_PHASES, UpdateEnvironment, pEnv);
	pEnv->RegisterAgentsForInputEvent();

	ostringstream oss;
	oss << "time run " << numCycles;
	const char* result = kernel->ExecuteCommandLine(oss.str().c_str(), "0");
	cout << "Time: " << result << endl;
	cout << "Num Update Events: " << numUpdateEvents << endl;
	cout << "Num Input Events : " << numInputEvents << endl;

	delete pEnv;

	kernel->Shutdown();
	delete kernel;
}

int main() {
	// When we have a memory leak, set this variable to
	// the allocation number (e.g. 122) and then we'll break
	// when that allocation occurs.
	//_crtBreakAlloc = 1053 ;

	{ // create local scope to allow for local memory cleanup before we check at end
		int numAgents = 4;
		int numWmes = 25;
		int numCycles = 1000;

		srand( (unsigned)time( NULL ) );

		RunTest1(numAgents, numWmes, numCycles);
		ResetEventCounts();
		RunTest2(numAgents, numWmes, numCycles);

		cout << endl << endl << "Press enter to exit.";
		cin.get();
	}

#ifdef _MSC_VER
	// Set the memory checking output to go to Visual Studio's debug window (so we have a copy to keep)
	// and to stdout so we can see it immediately.
	_CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG );
	_CrtSetReportFile( _CRT_WARN, _CRTDBG_FILE_STDOUT );

	// Now check for memory leaks.
	// This will only detect leaks in objects that we allocate within this executable and static libs.
	// If we allocate something in a DLL then this call won't see it because it works by overriding the
	// local implementation of malloc.
	_CrtDumpMemoryLeaks();
#endif // _MSC_VER

	return 0;
}