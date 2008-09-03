#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "SoarListener.h"

#include <iostream>
#include <conio.h>
#include <assert.h>

#include "sml_Connection.h"
#include "sml_Client.h"
#include "sml_AnalyzeXML.h"
#include "sml_ElementXML.h"
#include "sml_ClientEvents.h"
//#include "sml_ClientAgent.h"


// Define a sleep
#ifdef _WIN32
#define _WINSOCKAPI_
#include <Windows.h>
#define SLEEP Sleep
#else
#include <unistd.h>
#define SLEEP usleep
#endif

using namespace std;

// globals & constants
const char			AGENT_NAME[] = "test";		// test agent name
CommandProcessor*	g_pCommandProcessor = 0;	// pointer to the command processor singleton
const int			HISTORY_SIZE = 10;			// number of commands to keep in history

/*
// callback functions
void PrintCallbackHandler(sml::smlPrintEventId id, void* pUserData, sml::Agent* pAgent, char const* pMessage) {
	cout << pMessage;	// simply display whatever comes back through the event
}

void RunCallbackHandler(sml::smlRunEventId id, void* pUserData, sml::Agent* pAgent, sml::smlPhase phase) {
	g_pCommandProcessor->ProcessCharacter(getKey(false));
}

int getKey(bool block) {

#ifdef _WIN32
	// WINDOWS VERSION
	if (!block) {
		if (!kbhit()) return 0;
	}
	return getch();

#else // _WIN32

	// OTHER VERSIONS
	return 0;
#endif
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
			cout << endl;
			lineCopy = line;
			line.clear();
			return ProcessLine(lineCopy);

		// Backspaces are tricky, make them look right
		case '\b':
			Backspace();
			break;

		case 224:
			// Windows meta char
			meta = true;
			break;

		case 72:
			if (!meta) break;
			meta = false;
			// Up Arrow
			while (line.size()) {
				Backspace(); // Erase current line
			}

			// Retrieve something from history, overwrite command line, echo it
			temporaryHistoryIndex = temporaryHistoryIndex ? (temporaryHistoryIndex - 1) : (HISTORY_SIZE - 1);
			line = pHistory[temporaryHistoryIndex];
			cout << line;
			break;

		case 80:
			if (!meta) break;
			meta = false;
			// Down Arrow
			while (line.size()) {
				Backspace(); // Erase current line
			}

			// Retrieve something from history, overwrite command line, echo it
			temporaryHistoryIndex = ++temporaryHistoryIndex % HISTORY_SIZE;
			line = pHistory[temporaryHistoryIndex];
			cout << line;
			break;

		default:
			// Add other, non-special characters to the line
			line += c;
			cout << (char)c;
			cout.flush();
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
		return true;
	}

	// Store the command line into the history buffer
	pHistory[historyIndex++] = commandLine;
	historyIndex %= HISTORY_SIZE;
	temporaryHistoryIndex = historyIndex;

	// Process meta-commands first, since they return
	if (commandLine == "raw") {
		raw = true;
		DisplayPrompt(true);
		return true;

	}
	if (commandLine == "structured") {
		raw = false;
		DisplayPrompt(true);
		return true;
	} 

	// Process command line differently depending on the type of output
	bool previousResult;
	std::string output;
	if (raw) {
		output = pKernel->ExecuteCommandLine(commandLine.c_str(), AGENT_NAME);
		previousResult = pKernel->GetLastCommandLineResult() ;

	} else {
		sml::AnalyzeXML* pStructuredResponse = new sml::AnalyzeXML();
		previousResult = pKernel->ExecuteCommandLineXML(commandLine.c_str(), AGENT_NAME, pStructuredResponse);
		const sml::ElementXML* pResultTag = pStructuredResponse->GetResultTag();

		if (pResultTag) {
			char* pOutput = pResultTag->GenerateXMLString(true);
			if (pOutput) {
				output = pOutput;	// overwrite last command's output
			}
			pResultTag->DeleteString(pOutput);
		}

		const sml::ElementXML* pErrorTag = pStructuredResponse->GetErrorTag();
		if (pErrorTag) {
			char* pOutput = pErrorTag->GenerateXMLString(true);
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

	// If this string is seen, exit
	if (output.find("Goodbye.") != std::string::npos) {
		return false; // BUGBUG: this false is a normal, non erroneous exit, no way to tell apart from error exits
	}

	DisplayPrompt(previousResult);
	return true;
}*/

// Main program
int main(int argc, char** argv)
{
	if (argc > 2) {
		cout << "Too many args." << endl;
		exit(1);
	}

	// Create an embedded connection to the kernel
	// Passing false here so we don't execute Soar in our thread
	// which means we can handle incoming remote connections automatically.
	sml::Kernel* pKernel = sml::Kernel::CreateEmbeddedConnection("KernelSML", false) ;
	assert(pKernel);

	while(true)
	{
		SLEEP(1000);
	}
/*
	// Create agent
	// NOTE: We don't delete the agent pointer.  It's owned by the kernel
	sml::Agent* pAgent;
	pAgent = pKernel->CreateAgent(AGENT_NAME) ;
	assert(pAgent);

	// Create command processor
	assert(!g_pCommandProcessor);	// singleton
	g_pCommandProcessor = new CommandProcessor(pKernel);

	cout << "Use the meta-commands 'raw' and 'structured' to switch output style" << endl;

	// Register for necessary callbacks
	int callbackID1 = pAgent->RegisterForRunEvent(sml::smlEVENT_BEFORE_DECISION_CYCLE, RunCallbackHandler, 0);
	int callbackID2 = pAgent->RegisterForPrintEvent(sml::smlEVENT_PRINT, PrintCallbackHandler, 0);

	// Main command loop
	g_pCommandProcessor->DisplayPrompt(true);
	while (g_pCommandProcessor->ProcessCharacter(getKey(true))) {}

	pAgent->UnregisterForPrintEvent(sml::smlEVENT_PRINT, callbackID2);
	pAgent->UnregisterForRunEvent(sml::smlEVENT_BEFORE_DECISION_CYCLE, callbackID1);

	// Don't delete agent, owned by kernel*/
	delete pKernel ;
	return 0;
}
