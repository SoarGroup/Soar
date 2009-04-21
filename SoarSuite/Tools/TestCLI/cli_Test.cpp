#include <portability.h>

#include "sml_Utils.h"
#include "cli_Test.h"

#include <iostream>
#include <assert.h>
#include <queue>

#include "sml_Connection.h"
#include "sml_Client.h"
#include "sml_AnalyzeXML.h"
#include "ElementXML.h"
#include "sml_Events.h"
#include "sml_Names.h"

#include "thread_Lock.h"
#include "thread_Event.h"

using namespace std;

// globals & constants
const char			AGENT_NAME[] = "test";		// test agent name
CommandProcessor*	g_pCommandProcessor = 0;	// pointer to the command processor singleton
const int			HISTORY_SIZE = 10;			// number of commands to keep in history
queue<char>*		g_pInputQueue = 0;
soar_thread::Mutex*	g_pInputQueueMutex = 0;
soar_thread::Event*	g_pInputQueueWriteEvent = 0;
soar_thread::Event*	g_pWaitForInput = 0;
InputThread*		g_pInputThread = 0;
int					g_TraceCallbackID = 0;

InputThread::InputThread() {
}

InputThread::~InputThread() {
}

void InputThread::Run() {

	soar_thread::Lock* lock = 0;

	// Get input
	while ((!this->m_QuitNow) && (cin.get(m_C))) {
		// serialize access to this queue
		lock = new soar_thread::Lock(g_pInputQueueMutex);

		// push char on to queue
		g_pInputQueue->push(m_C);

		// signal write event
		g_pInputQueueWriteEvent->TriggerEvent();

		// unlock queue
		delete lock;

		sml::Sleep(0,10);
		g_pWaitForInput->WaitForEventForever();
		sml::Sleep(0,10);
	}

 	if (cin.bad()) {
		// BAD BAD BAD
		cout << "cin.get() failed!  Aborting." << endl;
		exit(1);
	}
}

// callback functions
void PrintCallbackHandler(sml::smlPrintEventId id, void* pUserData, sml::Agent* pAgent, char const* pMessage) {
	cout << pMessage;	// simply display whatever comes back through the event
}

void XMLCallbackHandler(sml::smlXMLEventId id, void* pUserData, sml::Agent* pAgent, sml::ClientXML* pXML) {
	char* message = pXML->GenerateXMLString(true, true);
	cout << message;
	pXML->DeleteString(message);
}

void RunCallbackHandler(sml::smlRunEventId id, void* pUserData, sml::Agent* pAgent, sml::smlPhase phase) {
	char c = getKey(false);

	while (c) {
		g_pCommandProcessor->ProcessCharacter(c);
		c = getKey(false);
	}
}

char getKey(bool block) {

	char ret = 0;	// default to 0 (no input)

	// lock the queue
	soar_thread::Lock* lock = new soar_thread::Lock(g_pInputQueueMutex);

	// blocking?
	if (block) {
		// yes, blocking, follow the following loop
		// lock, check size (break if true (input available)), unlock, wait for event, repeat
		while (!g_pInputQueue->size()) {
			// unlock queue
			delete lock;

			// Wait for write event
			g_pInputQueueWriteEvent->WaitForEventForever();

			// lock queue
			lock = new soar_thread::Lock(g_pInputQueueMutex);
		}
	} else {
		// not blocking, if there is nothing in the queue, return immediately
		if (!g_pInputQueue->size()) {
			// unlock queue
			delete lock;
			return ret;
		}
	}

	// we have a locked queue with stuff in it
	ret = g_pInputQueue->front();
	g_pInputQueue->pop();

	// unlock queue
	delete lock;
	return ret;
}

// Command Processor class
CommandProcessor::CommandProcessor(sml::Kernel* pKernel) {

	// Save kernel pointer
	this->pKernel = pKernel;

	// Init members to sane values
	raw = true;
	meta = false;

	// Init history variables
	historyIndex = 0;
	temporaryHistoryIndex = 0;

	// Create circular buffer
	pHistory = new std::string[HISTORY_SIZE];
}

CommandProcessor::~CommandProcessor() {
	// Delete circular buffer
	delete [] pHistory;
}

void CommandProcessor::DisplayPrompt(bool previousResult) {
	// Display the boolean (soon to be int?) result of the previous command
	// then the output format, then the agent name, and flush
	cout << previousResult;
	cout << (raw ? " (raw)" : " (structured)") << " " << AGENT_NAME << "> ";
	cout.flush();
}

bool CommandProcessor::ProcessCharacter(int c) {

	std::string lineCopy;
	switch (c) {
		// Ignore null
		case 0:
			break;

		// If the input was enter, process the command line
		case '\n':
		case '\r':
			lineCopy = line;
			line.clear();
			return ProcessLine(lineCopy);

		// Backspaces are tricky, make them look right
		case '\b':
			//Backspace();
			g_pWaitForInput->TriggerEvent();
			break;

		case 224:
			// Windows meta char
			meta = true;
			g_pWaitForInput->TriggerEvent();
			break;

		case 72:
			if (!meta) {
				// Add other, non-special characters to the line
				line += c;
				g_pWaitForInput->TriggerEvent();
				break;
			}
			meta = false;
			// Up Arrow
			while (line.size()) {
				Backspace(); // Erase current line
			}

			// Retrieve something from history, overwrite command line, echo it
			temporaryHistoryIndex = temporaryHistoryIndex ? (temporaryHistoryIndex - 1) : (HISTORY_SIZE - 1);
			line = pHistory[temporaryHistoryIndex];
			cout << line;
			g_pWaitForInput->TriggerEvent();
			break;

		case 80:
			if (!meta) {
				// Add other, non-special characters to the line
				line += c;
				g_pWaitForInput->TriggerEvent();
				break;
			}
			meta = false;
			// Down Arrow
			while (line.size()) {
				Backspace(); // Erase current line
			}

			// Retrieve something from history, overwrite command line, echo it
			temporaryHistoryIndex = ++temporaryHistoryIndex % HISTORY_SIZE;
			line = pHistory[temporaryHistoryIndex];
			cout << line;
			g_pWaitForInput->TriggerEvent();
			break;

		default:
			// Add other, non-special characters to the line
			line += c;
			g_pWaitForInput->TriggerEvent();
			break;
	}

	return true;
}

void CommandProcessor::Backspace() {
	// Handle local echo of backspaces
	if (line.size()) {
		cout << "\b \b";
		cout.flush();
		line = line.substr(0, line.length() - 1);
	}
}

bool CommandProcessor::ProcessLine(std::string& commandLine) {

	// Nothing on the command line
	if (!commandLine.size()) {
		DisplayPrompt(true);
		g_pWaitForInput->TriggerEvent();
		return true;
	}

	// Store the command line into the history buffer
	pHistory[historyIndex++] = commandLine;
	historyIndex %= HISTORY_SIZE;
	temporaryHistoryIndex = historyIndex;

	// Process meta-commands first, since they return
	if (!raw && commandLine == "raw") {
		sml::Agent* pAgent = pKernel->GetAgent( AGENT_NAME );
		pAgent->UnregisterForXMLEvent(g_TraceCallbackID);
		g_TraceCallbackID = pAgent->RegisterForPrintEvent( sml::smlEVENT_PRINT, PrintCallbackHandler, 0 );
		raw = true;
		DisplayPrompt(true);
		g_pWaitForInput->TriggerEvent();
		return true;

	}
	if (raw && commandLine == "structured") {
		sml::Agent* pAgent = pKernel->GetAgent( AGENT_NAME );
		pAgent->UnregisterForPrintEvent(g_TraceCallbackID);
		g_TraceCallbackID = pAgent->RegisterForXMLEvent( sml::smlEVENT_XML_TRACE_OUTPUT, XMLCallbackHandler, 0 );
		raw = false;
		DisplayPrompt(true);
		g_pWaitForInput->TriggerEvent();
		return true;
	} 
	if (commandLine == "spawn-debugger") {
		sml::Agent* pAgent = pKernel->GetAgent( AGENT_NAME );
		if (!pAgent->SpawnDebugger()) {
			std::cout << "Debugger spawn failed." << std::endl;
		}
		DisplayPrompt(true);
		g_pWaitForInput->TriggerEvent();
		return true;
	} 
	if (commandLine == "kill-debugger") {
		sml::Agent* pAgent = pKernel->GetAgent( AGENT_NAME );
		if (!pAgent->KillDebugger()) {
			std::cout << "Debugger kill failed." << std::endl;
		}
		DisplayPrompt(true);
		g_pWaitForInput->TriggerEvent();
		return true;
	} 

	bool quit = false;
	if (g_pInputThread) {
		string expandedCommandLine = pKernel->ExpandCommandLine(commandLine.c_str());
		if (commandLine == "quit" || expandedCommandLine == "quit") {
			g_pInputThread->Stop(false);
			g_pWaitForInput->TriggerEvent();
			g_pInputThread->Stop(true);
			quit = true;
		} else {
			g_pWaitForInput->TriggerEvent();
		}
	}

	// Process command line differently depending on the type of output
	bool previousResult;
	std::string output;
	if (raw) {
		output = pKernel->ExecuteCommandLine(commandLine.c_str(), AGENT_NAME);
		previousResult = pKernel->GetLastCommandLineResult() ;

	} else {
		sml::ClientAnalyzedXML* pStructuredResponse = new sml::ClientAnalyzedXML();
		previousResult = pKernel->ExecuteCommandLineXML(commandLine.c_str(), AGENT_NAME, pStructuredResponse);
		const soarxml::ElementXML* pResultTag = pStructuredResponse->GetResultTag();

		if (pResultTag) {
			char* pOutput = pResultTag->GenerateXMLString(true, true);
			if (pOutput) {
				output = pOutput;	// overwrite last command's output
			}
			pResultTag->DeleteString(pOutput);
		}

		const soarxml::ElementXML* pErrorTag = pStructuredResponse->GetErrorTag();
		if (pErrorTag) {
			char* pOutput = pErrorTag->GenerateXMLString(true, true);
			if (pOutput) {
				output += pOutput;	// append to result tag output
			}
			pErrorTag->DeleteString(pOutput);
		}

		delete pStructuredResponse;
	}

	// Display output if any
	if (output.size()) {
		if (output[output.size() - 1] == '\n') {
			// Get rid of that pesky extra newline
			output = output.substr(0, output.size() - 1);
		}
		cout << output << endl;
	}

	if (quit) {
		return false;
	}

	DisplayPrompt(previousResult);
	//g_pWaitForInput->TriggerEvent();
	return true;
}

// Main program
int main(int argc, char** argv)
{
#ifdef _DEBUG
	//_crtBreakAlloc = 2168;
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF ); 
#endif // _DEBUG

	{ // create local scope to prevent scriptFile from being reported as a memory leak (occurs when script passed in as arg)
		int scriptArg = 0;
		bool remote = false;
		if (argc > 1) {
			if (argv[1] == std::string("--remote")) {
				remote = true;
				if (argc > 2) {
					scriptArg = 2;
				}
			} else {
				scriptArg = 1;
			}
		}

		if (argc > 2) {
			cout << "Too many args." << endl;
			exit(1);
		}

		string scriptFile;
		if (scriptArg) {
			scriptFile = "source ";
			scriptFile += argv[scriptArg];
			scriptFile += '\n';
		}
		string::iterator scriptIter = scriptFile.begin();

		// Create an embedded connection to the kernel
		// Passing false here so we don't execute Soar in our thread
		// which means we can handle incoming remote connections automatically.
		sml::Kernel* pKernel = remote ? sml::Kernel::CreateRemoteConnection() : sml::Kernel::CreateKernelInNewThread();
		
		// Check for kernel creation errors
		// Note that even if there are errors in the kernel's creation
		// there should still be a kernel object (so one can query for errors)
		assert(pKernel);
		if(pKernel->HadError()) {
			cout << "Error: " << pKernel->GetLastErrorDescription() << endl;
			exit(1);
		}

		// Create agent
		// NOTE: We don't delete the agent pointer.  It's owned by the kernel
		sml::Agent* pAgent;
		pAgent = pKernel->CreateAgent(AGENT_NAME) ;
		assert(pAgent);

		assert(!g_pInputQueue);
		g_pInputQueue = new queue<char>;

		// Create command processor
		assert(!g_pCommandProcessor);	// singleton
		g_pCommandProcessor = new CommandProcessor(pKernel);

		// Set up synchronization stuff
		g_pInputQueueMutex = new soar_thread::Mutex();
		g_pInputQueueWriteEvent = new soar_thread::Event();
		g_pWaitForInput = new soar_thread::Event();

		cout << "Use the meta-commands 'raw' and 'structured' to switch output style" << endl;

		// Register for necessary callbacks
		int callbackID1 = pAgent->RegisterForRunEvent(sml::smlEVENT_BEFORE_DECISION_CYCLE, RunCallbackHandler, 0);
		g_TraceCallbackID = pAgent->RegisterForPrintEvent(sml::smlEVENT_PRINT, PrintCallbackHandler, 0);

		// Do script if any
		bool good = true;
		if (scriptFile.size()) {
			while (good && (scriptIter != scriptFile.end())) {
				good = g_pCommandProcessor->ProcessCharacter(*scriptIter);
				++scriptIter;
			}
		}
	
		// a good spot to insert a test command line, keep in mind output will not be printed
		//pKernel->ExecuteCommandLine("sp {test #bug\n   (state <s> ^superstate nil)\n-->\n   (<s> ^foo bar)\n}", AGENT_NAME);

		if (good) {
			// Create and start input thread
			g_pInputThread = new InputThread();
			g_pInputThread->Start();

			// Main command loop
			g_pCommandProcessor->DisplayPrompt(true);
			while (g_pCommandProcessor->ProcessCharacter(getKey(true))) {}
		}

		pAgent->UnregisterForPrintEvent(g_TraceCallbackID);
		pAgent->UnregisterForRunEvent(callbackID1);

		// Don't delete agent, owned by kernel
		delete pKernel ;

		// Delete input thread
		delete g_pInputThread;

		// Delete synchronization stuff
		delete g_pWaitForInput;
		delete g_pInputQueueWriteEvent;
		delete g_pInputQueueMutex;

		delete g_pCommandProcessor;
		delete g_pInputQueue;
	} // end local scope
	return 0;
}

//int main(int argc, char** argv)
//{
//#ifdef _DEBUG
//	//_crtBreakAlloc = 1441;
//	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF ); 
//#endif // _DEBUG
//
//	{ // create local scope to prevent scriptFile from being reported as a memory leak (occurs when script passed in as arg)
//		sml::Kernel* pKernel = sml::Kernel::CreateKernelInNewThread("SoarKernelSML") ;
//		assert(pKernel);
//		if(pKernel->HadError()) {
//			cout << "Error: " << pKernel->GetLastErrorDescription() << endl;
//			exit(1);
//		}
//
//		sml::Agent* pAgent;
//		pAgent = pKernel->CreateAgent(AGENT_NAME) ;
//		assert(pAgent);
//
//		g_TraceCallbackID = pAgent->RegisterForPrintEvent( sml::smlEVENT_PRINT, PrintCallbackHandler, 0 );
//		pAgent->UnregisterForPrintEvent(g_TraceCallbackID);
//		g_TraceCallbackID = pAgent->RegisterForXMLEvent( sml::smlEVENT_XML_TRACE_OUTPUT, XMLCallbackHandler, 0 );
//
//		// Don't delete agent, owned by kernel
//		delete pKernel ;
//
//	} // end local scope
//	return 0;
//}
