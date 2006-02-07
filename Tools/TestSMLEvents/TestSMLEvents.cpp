/////////////////////////////////////////////////////////////////////////
//
// TestSMLEvents.cpp: Reports how many times various events were called
//
// Bob Marinier, University of Michigan
// June 2005
//
// This program registers for a bunch of events, and then counts how
//  often they get called. It provides a rudimentary command line
//  interface so the user can do whatever Soar stuff they want to test.
// If the user wants to reset the counters during the run, use the 
//  special command "reset".  If the user wants to print the event
//  counts "so far", use the special command "counts".  When the user
//  wants to quit, type "quit" or "exit".  The final counts are printed
//  at the end.
//
// More on special command "counts"
//  If you want to see all event counts, use "counts" by itself.
//  If you want to see only the counts for a particular kind of event,
//   you can specify the event type with "counts <type>".
//  To see a list of possible types, use "counts help".
//
// If you want to see where a particular event comes from (i.e. the
//  callstack), uncomment the "if" statement in My<type>EventHandler and
//  modify it to check for the particular event you want to check, and set
//  a breakpoint on the cout line in the "if" body.
//
// How it works:
// We build a list of EventData objects.  Each of these contains the
//  eventId it's tracking, a plain-text name for the event, and a count.
// We group events of the same type (i.e. run, production, etc) into a
//  single list and label it with the name of the event type (this is
//  what the NamedEventDataList structure is for).
// We put all of these lists into a master list which we then use to
//  register the events.  When we register for an event, we pass in the
//  associated EventData object as the userData for that event.  When
//  the event fires, we increment the count on that object.
// To add new events to existing types, simply add the new event to the
//  list in the Create<type>EventData function.
// To add a new type, create a Create<type>EventData function, and call
//  it from CreateEventTestData.  Then modify RegisterForEvents to
//  register for the new type.
/////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

// Use Visual C++'s memory checking functionality
#ifdef _MSC_VER
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif // _MSC_VER

#include <string>
#include <list>
#include <iostream>
#include "sml_Client.h"

using namespace std;
using namespace sml;

struct EventData {
	int eventId;
	string eventName;
	int count;

	EventData(int i, string n) : eventId(i), eventName(n) { count=0; }
};

typedef list<EventData*> EventDataList;

struct NamedEventDataList {
	string name;
	EventDataList eventData;

	~NamedEventDataList() {
		for(EventDataList::iterator i = eventData.begin(); i != eventData.end(); i++) {
			delete (*i);
		}
		eventData.clear();
	}
};

typedef list<NamedEventDataList*> DataList;


DataList* CreateEventTestData();
NamedEventDataList* CreateSystemEventData();
NamedEventDataList* CreateRunEventData();
NamedEventDataList* CreateProductionEventData();
NamedEventDataList* CreateAgentEventData();
NamedEventDataList* CreatePrintEventData();
NamedEventDataList* CreateXMLEventData();
NamedEventDataList* CreateUpdateEventData();
NamedEventDataList* CreateStringEventData();

void RegisterForEvents(Kernel* k, Agent* a, DataList* dataList);
void PrintEventData(DataList* dataList, string type="");
void ResetEventCounts(DataList* dataList);


void MySystemEventHandler(smlSystemEventId id, void* pUserData, Kernel* pKernel) {
	EventData* eventData = static_cast<EventData*>(pUserData);
	eventData->count++;

	// if you want to see where a particular event is coming from, set a breakpoint here
	/*if(eventData->eventId == 123) {
		cout << "Got event " << eventId << endl;
	}*/
}

void MyRunEventHandler(smlRunEventId id, void* pUserData, Agent* pAgent, smlPhase phase) {
	EventData* eventData = static_cast<EventData*>(pUserData);
	eventData->count++;

	// if you want to see where a particular event is coming from, set a breakpoint here
	/*if(eventData->eventId == 123) {
		cout << "Got event " << eventId << endl;
	}*/
}

void MyProductionEventHandler(smlProductionEventId id, void* pUserData, Agent* pAgent, char const* pProdName, char const* pInstantion) {
	EventData* eventData = static_cast<EventData*>(pUserData);
	eventData->count++;

	// if you want to see where a particular event is coming from, set a breakpoint here
	/*if(eventData->eventId == 123) {
		cout << "Got event " << eventId << endl;
	}*/
}

void MyAgentEventHandler(smlAgentEventId id, void* pUserData, Agent* pAgent) {
	EventData* eventData = static_cast<EventData*>(pUserData);
	eventData->count++;

	// if you want to see where a particular event is coming from, set a breakpoint here
	/*if(eventData->eventId == 123) {
		cout << "Got event " << eventId << endl;
	}*/
}

void MyPrintEventHandler(smlPrintEventId id, void* pUserData, Agent* pAgent, char const* pMessage) {
	EventData* eventData = static_cast<EventData*>(pUserData);
	eventData->count++;
	cout << pMessage << endl;
	
	// if you want to see where a particular event is coming from, set a breakpoint here
	/*if(eventData->eventId == 123) {
		cout << "Got event " << eventId << endl;
	}*/
}

void MyXMLEventHandler(smlXMLEventId id, void* pUserData, Agent* pAgent, ClientXML* pXML) {
	EventData* eventData = static_cast<EventData*>(pUserData);
	eventData->count++;

	// if you want to see where a particular event is coming from, set a breakpoint here
	/*if(eventData->eventId == 123) {
		cout << "Got event " << eventId << endl;
	}*/
}

void MyUpdateEventHandler(smlUpdateEventId id, void* pUserData, Kernel* pKernel, smlRunFlags runFlags) {
	EventData* eventData = static_cast<EventData*>(pUserData);
	eventData->count++;

	// if you want to see where a particular event is coming from, set a breakpoint here
	/*if(eventData->eventId == 123) {
		cout << "Got event " << eventId << endl;
	}*/
}

void MyStringEventHandler(smlStringEventId id, void* pUserData, Kernel* pKernel, char const* pData) {
	EventData* eventData = static_cast<EventData*>(pUserData);
	eventData->count++;

	// if you want to see where a particular event is coming from, set a breakpoint here
	/*if(eventData->eventId == 123) {
		cout << "Got event " << eventId << endl;
	}*/
}


int main() {

	// When we have a memory leak, set this variable to
	// the allocation number (e.g. 122) and then we'll break
	// when that allocation occurs.
	//_crtBreakAlloc = 73 ;

	{ // create local scope to allow for local memory cleanup before we check at end


		//
		// create the test data
		//
		DataList* dataList = CreateEventTestData();
		
		//
		// set up Soar
        //
		Kernel* kernel = Kernel::CreateKernelInNewThread("SoarKernelSML");
		Agent* agent = kernel->CreateAgent("Soar1");

		RegisterForEvents(kernel, agent, dataList);

		//
		// let the user do whatever they want
		//
		string command;
		do {
			cout << agent->GetAgentName() << "> ";
			getline(cin, command);

			// check for special commands
			if(command == "reset") {
				ResetEventCounts(dataList);
				cout << "counts reset" << endl;
			} else if(!command.compare(0, 6, "counts")) {
				string type = "";
				if(command != "counts") { type = command.substr(7,command.length()-7); }
				PrintEventData(dataList, type);
			} else {
				cout << kernel->ExecuteCommandLine(command.c_str(), agent->GetAgentName()) << endl;
			}
		} while( (command != "quit") && (command != "exit") );
		
		kernel->Shutdown();
		delete kernel;

		//
		// Show results
		//
		PrintEventData(dataList);

		string dummy;
		cout << endl << endl << "Press any key and enter to continue";
		cin >> dummy;

		//
		// clean up
		//
		for(DataList::iterator i = dataList->begin(); i != dataList->end(); i++) {
			delete (*i);
		}
		delete dataList;

	} // end local scope

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

DataList* CreateEventTestData() {
	DataList* dataList = new DataList();

	NamedEventDataList* systemEventData = CreateSystemEventData();
	NamedEventDataList* runEventData = CreateRunEventData();
	NamedEventDataList* productionEventData = CreateProductionEventData();
	NamedEventDataList* agentEventData = CreateAgentEventData();
	NamedEventDataList* printEventData = CreatePrintEventData();
	NamedEventDataList* xmlEventData = CreateXMLEventData();
	NamedEventDataList* updateEventData = CreateUpdateEventData();
	NamedEventDataList* stringEventData = CreateStringEventData();

	dataList->push_back(systemEventData);
	dataList->push_back(runEventData);
	dataList->push_back(productionEventData);
	dataList->push_back(agentEventData);
	dataList->push_back(printEventData);
	dataList->push_back(xmlEventData);
	dataList->push_back(updateEventData);
	dataList->push_back(stringEventData);

	return dataList;
}

NamedEventDataList* CreateSystemEventData() {
	NamedEventDataList* namedEventData = new NamedEventDataList();
	namedEventData->name = "system";
	
	namedEventData->eventData.push_back(new EventData(smlEVENT_BEFORE_SHUTDOWN, "smlEVENT_BEFORE_SHUTDOWN"));
	namedEventData->eventData.push_back(new EventData(smlEVENT_AFTER_CONNECTION, "smlEVENT_AFTER_CONNECTION"));
	namedEventData->eventData.push_back(new EventData(smlEVENT_AFTER_CONNECTION_LOST, "smlEVENT_AFTER_CONNECTION_LOST"));
	namedEventData->eventData.push_back(new EventData(smlEVENT_BEFORE_RESTART, "smlEVENT_BEFORE_RESTART"));
	namedEventData->eventData.push_back(new EventData(smlEVENT_AFTER_RESTART, "smlEVENT_AFTER_RESTART"));
	namedEventData->eventData.push_back(new EventData(smlEVENT_SYSTEM_START, "smlEVENT_SYSTEM_START"));
	namedEventData->eventData.push_back(new EventData(smlEVENT_SYSTEM_STOP, "smlEVENT_SYSTEM_STOP"));
	namedEventData->eventData.push_back(new EventData(smlEVENT_INTERRUPT_CHECK, "smlEVENT_INTERRUPT_CHECK"));
	namedEventData->eventData.push_back(new EventData(smlEVENT_BEFORE_RHS_FUNCTION_ADDED, "smlEVENT_BEFORE_RHS_FUNCTION_ADDED"));
	namedEventData->eventData.push_back(new EventData(smlEVENT_AFTER_RHS_FUNCTION_ADDED, "smlEVENT_AFTER_RHS_FUNCTION_ADDED"));
	namedEventData->eventData.push_back(new EventData(smlEVENT_BEFORE_RHS_FUNCTION_REMOVED, "smlEVENT_BEFORE_RHS_FUNCTION_REMOVED"));
	namedEventData->eventData.push_back(new EventData(smlEVENT_AFTER_RHS_FUNCTION_REMOVED, "smlEVENT_AFTER_RHS_FUNCTION_REMOVED"));
	namedEventData->eventData.push_back(new EventData(smlEVENT_BEFORE_RHS_FUNCTION_EXECUTED, "smlEVENT_BEFORE_RHS_FUNCTION_EXECUTED"));
	namedEventData->eventData.push_back(new EventData(smlEVENT_AFTER_RHS_FUNCTION_EXECUTED, "smlEVENT_AFTER_RHS_FUNCTION_EXECUTED"));

	return namedEventData;
}

NamedEventDataList* CreateRunEventData() {
	NamedEventDataList* namedEventData = new NamedEventDataList();
	namedEventData->name = "run";
	
	namedEventData->eventData.push_back(new EventData(smlEVENT_BEFORE_SMALLEST_STEP, "smlEVENT_BEFORE_SMALLEST_STEP"));
	namedEventData->eventData.push_back(new EventData(smlEVENT_AFTER_SMALLEST_STEP, "smlEVENT_AFTER_SMALLEST_STEP"));
	namedEventData->eventData.push_back(new EventData(smlEVENT_BEFORE_ELABORATION_CYCLE, "smlEVENT_BEFORE_ELABORATION_CYCLE"));
	namedEventData->eventData.push_back(new EventData(smlEVENT_AFTER_ELABORATION_CYCLE, "smlEVENT_AFTER_ELABORATION_CYCLE"));
	namedEventData->eventData.push_back(new EventData(smlEVENT_BEFORE_PHASE_EXECUTED, "smlEVENT_BEFORE_PHASE_EXECUTED"));

	namedEventData->eventData.push_back(new EventData(smlEVENT_BEFORE_INPUT_PHASE, "smlEVENT_BEFORE_INPUT_PHASE"));
	namedEventData->eventData.push_back(new EventData(smlEVENT_BEFORE_PROPOSE_PHASE, "smlEVENT_BEFORE_PROPOSE_PHASE"));
	namedEventData->eventData.push_back(new EventData(smlEVENT_BEFORE_DECISION_PHASE, "smlEVENT_BEFORE_DECISION_PHASE"));
	namedEventData->eventData.push_back(new EventData(smlEVENT_BEFORE_APPLY_PHASE, "smlEVENT_BEFORE_APPLY_PHASE"));
	namedEventData->eventData.push_back(new EventData(smlEVENT_BEFORE_OUTPUT_PHASE, "smlEVENT_BEFORE_OUTPUT_PHASE"));
	namedEventData->eventData.push_back(new EventData(smlEVENT_BEFORE_PREFERENCE_PHASE, "smlEVENT_BEFORE_PREFERENCE_PHASE"));// Soar-7 mode only
	namedEventData->eventData.push_back(new EventData(smlEVENT_BEFORE_WM_PHASE, "smlEVENT_BEFORE_WM_PHASE"));// Soar-7 mode only

	namedEventData->eventData.push_back(new EventData(smlEVENT_AFTER_INPUT_PHASE, "smlEVENT_AFTER_INPUT_PHASE")); 
	namedEventData->eventData.push_back(new EventData(smlEVENT_AFTER_PROPOSE_PHASE, "smlEVENT_AFTER_PROPOSE_PHASE")); 
	namedEventData->eventData.push_back(new EventData(smlEVENT_AFTER_DECISION_PHASE, "smlEVENT_AFTER_DECISION_PHASE")); 
	namedEventData->eventData.push_back(new EventData(smlEVENT_AFTER_APPLY_PHASE, "smlEVENT_AFTER_APPLY_PHASE"));
	namedEventData->eventData.push_back(new EventData(smlEVENT_AFTER_OUTPUT_PHASE, "smlEVENT_AFTER_OUTPUT_PHASE")); 
	namedEventData->eventData.push_back(new EventData(smlEVENT_AFTER_PREFERENCE_PHASE, "smlEVENT_AFTER_PREFERENCE_PHASE"));// Soar-7 mode only
	namedEventData->eventData.push_back(new EventData(smlEVENT_AFTER_WM_PHASE, "smlEVENT_AFTER_WM_PHASE"));// Soar-7 mode only

	namedEventData->eventData.push_back(new EventData(smlEVENT_AFTER_PHASE_EXECUTED, "smlEVENT_AFTER_PHASE_EXECUTED"));
	namedEventData->eventData.push_back(new EventData(smlEVENT_BEFORE_DECISION_CYCLE, "smlEVENT_BEFORE_DECISION_CYCLE"));
	namedEventData->eventData.push_back(new EventData(smlEVENT_AFTER_DECISION_CYCLE, "smlEVENT_AFTER_DECISION_CYCLE"));
	namedEventData->eventData.push_back(new EventData(smlEVENT_AFTER_INTERRUPT, "smlEVENT_AFTER_INTERRUPT"));
	namedEventData->eventData.push_back(new EventData(smlEVENT_BEFORE_RUN_STARTS, "smlEVENT_BEFORE_RUN_STARTS"));
	namedEventData->eventData.push_back(new EventData(smlEVENT_AFTER_RUN_ENDS, "smlEVENT_AFTER_RUN_ENDS"));
	namedEventData->eventData.push_back(new EventData(smlEVENT_BEFORE_RUNNING, "smlEVENT_BEFORE_RUNNING"));
	namedEventData->eventData.push_back(new EventData(smlEVENT_AFTER_RUNNING, "smlEVENT_AFTER_RUNNING"));

	return namedEventData;
}

NamedEventDataList* CreateProductionEventData() {
	NamedEventDataList* namedEventData = new NamedEventDataList();
	namedEventData->name = "production";
	
	namedEventData->eventData.push_back(new EventData(smlEVENT_AFTER_PRODUCTION_ADDED, "smlEVENT_AFTER_PRODUCTION_ADDED"));
	namedEventData->eventData.push_back(new EventData(smlEVENT_BEFORE_PRODUCTION_REMOVED, "smlEVENT_BEFORE_PRODUCTION_REMOVED"));
	namedEventData->eventData.push_back(new EventData(smlEVENT_AFTER_PRODUCTION_FIRED, "smlEVENT_AFTER_PRODUCTION_FIRED"));
	namedEventData->eventData.push_back(new EventData(smlEVENT_BEFORE_PRODUCTION_RETRACTED, "smlEVENT_BEFORE_PRODUCTION_RETRACTED"));

	return namedEventData;
}

NamedEventDataList* CreateAgentEventData() {
	NamedEventDataList* namedEventData = new NamedEventDataList();
	namedEventData->name = "agent";
	
	namedEventData->eventData.push_back(new EventData(smlEVENT_AFTER_AGENT_CREATED, "smlEVENT_AFTER_AGENT_CREATED"));
	namedEventData->eventData.push_back(new EventData(smlEVENT_BEFORE_AGENT_DESTROYED, "smlEVENT_BEFORE_AGENT_DESTROYED"));
	namedEventData->eventData.push_back(new EventData(smlEVENT_BEFORE_AGENTS_RUN_STEP, "smlEVENT_BEFORE_AGENTS_RUN_STEP"));
	namedEventData->eventData.push_back(new EventData(smlEVENT_BEFORE_AGENT_REINITIALIZED, "smlEVENT_BEFORE_AGENT_REINITIALIZED"));
	namedEventData->eventData.push_back(new EventData(smlEVENT_AFTER_AGENT_REINITIALIZED, "smlEVENT_AFTER_AGENT_REINITIALIZED"));

	return namedEventData;
}

NamedEventDataList* CreatePrintEventData() {
	NamedEventDataList* namedEventData = new NamedEventDataList();
	namedEventData->name = "print";
	
	//namedEventData->eventData.push_back(new EventData(smlEVENT_LOG_ERROR, "smlEVENT_LOG_ERROR"));
	//namedEventData->eventData.push_back(new EventData(smlEVENT_LOG_WARNING, "smlEVENT_LOG_WARNING"));
	//namedEventData->eventData.push_back(new EventData(smlEVENT_LOG_INFO, "smlEVENT_LOG_INFO"));
	//namedEventData->eventData.push_back(new EventData(smlEVENT_LOG_DEBUG, "smlEVENT_LOG_DEBUG"));
	namedEventData->eventData.push_back(new EventData(smlEVENT_ECHO, "smlEVENT_ECHO"));
	namedEventData->eventData.push_back(new EventData(smlEVENT_PRINT, "smlEVENT_PRINT"));

	return namedEventData;
}

NamedEventDataList* CreateXMLEventData() {
	NamedEventDataList* namedEventData = new NamedEventDataList();
	namedEventData->name = "xml";
	
	namedEventData->eventData.push_back(new EventData(smlEVENT_XML_TRACE_OUTPUT, "smlEVENT_XML_TRACE_OUTPUT"));

	return namedEventData;
}

NamedEventDataList* CreateUpdateEventData() {
	NamedEventDataList* namedEventData = new NamedEventDataList();
	namedEventData->name = "update";
	
	namedEventData->eventData.push_back(new EventData(smlEVENT_AFTER_ALL_OUTPUT_PHASES, "smlEVENT_AFTER_ALL_OUTPUT_PHASES"));
	namedEventData->eventData.push_back(new EventData(smlEVENT_AFTER_ALL_GENERATED_OUTPUT, "smlEVENT_AFTER_ALL_GENERATED_OUTPUT"));

	return namedEventData;
}

NamedEventDataList* CreateStringEventData() {
	NamedEventDataList* namedEventData = new NamedEventDataList();
	namedEventData->name = "string";
	
	namedEventData->eventData.push_back(new EventData(smlEVENT_EDIT_PRODUCTION, "smlEVENT_EDIT_PRODUCTION"));

	return namedEventData;
}

void RegisterForEvents(Kernel* k, Agent* a, DataList* dataList) {
	for(DataList::iterator i = dataList->begin(); i != dataList->end(); i++) {
		for(EventDataList::iterator j = (*i)->eventData.begin(); j != (*i)->eventData.end(); j++) {
			if(IsSystemEventID((*j)->eventId)) {
				k->RegisterForSystemEvent(static_cast<smlSystemEventId>((*j)->eventId), MySystemEventHandler, (*j));
			} else if(IsRunEventID((*j)->eventId)) {
				a->RegisterForRunEvent(static_cast<smlRunEventId>((*j)->eventId), MyRunEventHandler, (*j));
			} else if(IsProductionEventID((*j)->eventId)) {
				a->RegisterForProductionEvent(static_cast<smlProductionEventId>((*j)->eventId), MyProductionEventHandler, (*j));
			} else if(IsAgentEventID((*j)->eventId)) {
				k->RegisterForAgentEvent(static_cast<smlAgentEventId>((*j)->eventId), MyAgentEventHandler, (*j));
			} else if(IsPrintEventID((*j)->eventId)) {
				a->RegisterForPrintEvent(static_cast<smlPrintEventId>((*j)->eventId), MyPrintEventHandler, (*j));
			} else if(IsXMLEventID((*j)->eventId)) {
				a->RegisterForXMLEvent(static_cast<smlXMLEventId>((*j)->eventId), MyXMLEventHandler, (*j));
			} else if(IsUpdateEventID((*j)->eventId)) {
				k->RegisterForUpdateEvent(static_cast<smlUpdateEventId>((*j)->eventId), MyUpdateEventHandler, (*j));
			} else if(IsStringEventID((*j)->eventId)) {
				k->RegisterForStringEvent(static_cast<smlStringEventId>((*j)->eventId), MyStringEventHandler, (*j));
			} else { cout << "TestSMLEvents error: Unrecognized event id: " << (*j)->eventId << endl; }
		}
	}
}

void ResetEventCounts(DataList* dataList) {
	for(DataList::iterator i = dataList->begin(); i != dataList->end(); i++) {
		for(EventDataList::iterator j = (*i)->eventData.begin(); j != (*i)->eventData.end(); j++) {
			(*j)->count = 0;
		}
	}
}

void PrintEventData(DataList* dataList, string type) {

	bool isValidType = false;

	for(DataList::iterator i = dataList->begin(); i != dataList->end(); i++) {

		// only print the desired type, or all type if none specified
		if(type == "" || (*i)->name == type) {
			
			isValidType = true;

			cout << endl << "*****   " << (*i)->name << " events   *****" << endl << endl;
			
			for(EventDataList::iterator j = (*i)->eventData.begin(); j != (*i)->eventData.end(); j++) {
				cout.width(4);
				cout << (*j)->eventId << " ";
				cout.width(40);
				cout << (*j)->eventName << " ";
				cout << (*j)->count << endl;
			}
		}

		// if the user is asking for help, give them a list of possible event type
		if(type == "help") {
			isValidType = true;
			cout << (*i)->name << endl;
		}
	}

	if(!isValidType) {
		cout << "Invalid type '" << type << "' specified." << endl;
		cout << "Try 'counts help' to see the list of valid types." << endl;
		cout << "Not specifying a type will print all types." << endl;
	}
}
