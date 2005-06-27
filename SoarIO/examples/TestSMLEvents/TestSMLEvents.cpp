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
NamedEventDataList* CreateRunEventData();
NamedEventDataList* CreateProductionEventData();
NamedEventDataList* CreatePrintEventData();
NamedEventDataList* CreateXMLEventData();
void RegisterForEvents(Kernel* k, Agent* a, DataList* dataList);
void PrintEventData(DataList* dataList);
void ResetEventCounts(DataList* dataList);

void MyRunEventHandler(smlRunEventId id, void* pUserData, Agent* pAgent, smlPhase phase) {
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

void MyProductionEventHandler(smlProductionEventId id, void* pUserData, Agent* pAgent, char const* pProdName, char const* pInstantion) {
	EventData* eventData = static_cast<EventData*>(pUserData);
	eventData->count++;

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


void main() {

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
		char command[1000];
		do {
			cout << agent->GetAgentName() << "> ";
			cin.getline(command, 999);

			// check for special commands
			if(!strncmp(command, "reset", 999)) {
				ResetEventCounts(dataList);
				cout << "counts reset" << endl;
			} else if(!strncmp(command, "counts", 999)) {
				PrintEventData(dataList);
			} else {
				cout << kernel->ExecuteCommandLine(command, agent->GetAgentName()) << endl;
			}
		} while(strncmp(command,"quit",999) && strncmp(command, "exit", 999));
		
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
	
}

DataList* CreateEventTestData() {
	DataList* dataList = new DataList();

	NamedEventDataList* runEventData = CreateRunEventData();
	NamedEventDataList* productionEventData = CreateProductionEventData();
	NamedEventDataList* printEventData = CreatePrintEventData();
	NamedEventDataList* xmlEventData = CreateXMLEventData();

	dataList->push_back(runEventData);
	dataList->push_back(productionEventData);
	dataList->push_back(printEventData);
	dataList->push_back(xmlEventData);

	return dataList;
}

NamedEventDataList* CreateRunEventData() {
	NamedEventDataList* runEventData = new NamedEventDataList();
	runEventData->name = "Run Events";
	
	runEventData->eventData.push_back(new EventData(smlEVENT_BEFORE_SMALLEST_STEP, "smlEVENT_BEFORE_SMALLEST_STEP"));
	runEventData->eventData.push_back(new EventData(smlEVENT_AFTER_SMALLEST_STEP, "smlEVENT_AFTER_SMALLEST_STEP"));
	runEventData->eventData.push_back(new EventData(smlEVENT_BEFORE_ELABORATION_CYCLE, "smlEVENT_BEFORE_ELABORATION_CYCLE"));
	runEventData->eventData.push_back(new EventData(smlEVENT_AFTER_ELABORATION_CYCLE, "smlEVENT_AFTER_ELABORATION_CYCLE"));
	runEventData->eventData.push_back(new EventData(smlEVENT_BEFORE_PHASE_EXECUTED, "smlEVENT_BEFORE_PHASE_EXECUTED"));
	runEventData->eventData.push_back(new EventData(smlEVENT_AFTER_PHASE_EXECUTED, "smlEVENT_AFTER_PHASE_EXECUTED"));
	runEventData->eventData.push_back(new EventData(smlEVENT_BEFORE_DECISION_CYCLE, "smlEVENT_BEFORE_DECISION_CYCLE"));
	runEventData->eventData.push_back(new EventData(smlEVENT_AFTER_DECISION_CYCLE, "smlEVENT_AFTER_DECISION_CYCLE"));
	runEventData->eventData.push_back(new EventData(smlEVENT_AFTER_INTERRUPT, "smlEVENT_AFTER_INTERRUPT"));
	runEventData->eventData.push_back(new EventData(smlEVENT_BEFORE_RUN_STARTS, "smlEVENT_BEFORE_RUN_STARTS"));
	runEventData->eventData.push_back(new EventData(smlEVENT_AFTER_RUN_ENDS, "smlEVENT_AFTER_RUN_ENDS"));
	runEventData->eventData.push_back(new EventData(smlEVENT_BEFORE_RUNNING, "smlEVENT_BEFORE_RUNNING"));
	runEventData->eventData.push_back(new EventData(smlEVENT_AFTER_RUNNING, "smlEVENT_AFTER_RUNNING"));

	return runEventData;
}

NamedEventDataList* CreateProductionEventData() {
	NamedEventDataList* productionEventData = new NamedEventDataList();
	productionEventData->name = "Production Events";
	
	productionEventData->eventData.push_back(new EventData(smlEVENT_AFTER_PRODUCTION_ADDED, "smlEVENT_AFTER_PRODUCTION_ADDED"));
	productionEventData->eventData.push_back(new EventData(smlEVENT_BEFORE_PRODUCTION_REMOVED, "smlEVENT_BEFORE_PRODUCTION_REMOVED"));
	productionEventData->eventData.push_back(new EventData(smlEVENT_AFTER_PRODUCTION_FIRED, "smlEVENT_AFTER_PRODUCTION_FIRED"));
	productionEventData->eventData.push_back(new EventData(smlEVENT_BEFORE_PRODUCTION_RETRACTED, "smlEVENT_BEFORE_PRODUCTION_RETRACTED"));

	return productionEventData;
}

NamedEventDataList* CreatePrintEventData() {
	NamedEventDataList* printEventData = new NamedEventDataList();
	printEventData->name = "Print Events";
	
	printEventData->eventData.push_back(new EventData(smlEVENT_LOG_ERROR, "smlEVENT_LOG_ERROR"));
	printEventData->eventData.push_back(new EventData(smlEVENT_LOG_WARNING, "smlEVENT_LOG_WARNING"));
	printEventData->eventData.push_back(new EventData(smlEVENT_LOG_INFO, "smlEVENT_LOG_INFO"));
	printEventData->eventData.push_back(new EventData(smlEVENT_LOG_DEBUG, "smlEVENT_LOG_DEBUG"));
	printEventData->eventData.push_back(new EventData(smlEVENT_ECHO, "smlEVENT_ECHO"));
	printEventData->eventData.push_back(new EventData(smlEVENT_PRINT, "smlEVENT_PRINT"));

	return printEventData;
}

NamedEventDataList* CreateXMLEventData() {
	NamedEventDataList* xmlEventData = new NamedEventDataList();
	xmlEventData->name = "XML Events";
	
	xmlEventData->eventData.push_back(new EventData(smlEVENT_XML_TRACE_OUTPUT, "smlEVENT_XML_TRACE_OUTPUT"));

	return xmlEventData;
}

void RegisterForEvents(Kernel* k, Agent* a, DataList* dataList) {
	// at the moment we don't use the Kernel object passed in here because we're only registering for agent events
	for(DataList::iterator i = dataList->begin(); i != dataList->end(); i++) {
		for(EventDataList::iterator j = (*i)->eventData.begin(); j != (*i)->eventData.end(); j++) {
			if(IsRunEventID((*j)->eventId)) {
				a->RegisterForRunEvent(static_cast<smlRunEventId>((*j)->eventId), MyRunEventHandler, (*j));
			} else if(IsProductionEventID((*j)->eventId)) {
				a->RegisterForProductionEvent(static_cast<smlProductionEventId>((*j)->eventId), MyProductionEventHandler, (*j));
			} else if(IsPrintEventID((*j)->eventId)) {
				a->RegisterForPrintEvent(static_cast<smlPrintEventId>((*j)->eventId), MyPrintEventHandler, (*j));
			} else if(IsXMLEventID((*j)->eventId)) {
				a->RegisterForXMLEvent(static_cast<smlXMLEventId>((*j)->eventId), MyXMLEventHandler, (*j));
			} else { cout << "Unrecognized event id: " << (*j)->eventId << endl; }
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

void PrintEventData(DataList* dataList) {
	for(DataList::iterator i = dataList->begin(); i != dataList->end(); i++) {

		cout << endl << endl << "*****   " << (*i)->name << "   *****" << endl << endl;
		
		for(EventDataList::iterator j = (*i)->eventData.begin(); j != (*i)->eventData.end(); j++) {
			cout << (*j)->eventId << "\t" << (*j)->eventName << "\t" << (*j)->count << endl;
		}
	}
}