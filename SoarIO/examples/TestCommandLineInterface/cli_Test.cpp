#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_Test.h"

#include <iostream>

#include <assert.h>

#include "sml_Connection.h"
#include "sml_Client.h"
#include "sml_AnalyzeXML.h"
#include "sml_ElementXML.h"
#include "sml_ClientEvents.h"
//#include "sml_ClientAgent.h"

using namespace std;

// globals & constants
const char			AGENT_NAME[] = "test";
CommandProcessor*	g_pCommandProcessor = 0;
const int			HISTORY_SIZE = 10;

// callback functions
void PrintCallbackHandler(sml::smlPrintEventId id, void* pUserData, sml::Agent* pAgent, char const* pMessage) {
	cout << pMessage;
}

void RunCallbackHandler(sml::smlRunEventId id, void* pUserData, sml::Agent* pAgent, sml::smlPhase phase) {
	if (cin.rdbuf()->in_avail()) {
		g_pCommandProcessor->ProcessCharacter();
	}
}

// Command Processor class
CommandProcessor::CommandProcessor(sml::Kernel* pKernel) {

	this->pKernel = pKernel;

	inputChar = 0;
	previousResult = true;
	raw = true;

	// History variables
	historyIndex = 0;
	temporaryHistoryIndex = 0;
	pHistory = new std::string[HISTORY_SIZE];
}

CommandProcessor::~CommandProcessor() {
	delete [] pHistory;
}

void CommandProcessor::DisplayPrompt() {
	commandLine.clear();
	cout << previousResult;
	cout << (raw ? " (raw)" : " (structured)") << " " << AGENT_NAME << "> ";
	cout.flush();
}

bool CommandProcessor::ProcessCharacter() {

	// Read stdin
	if (!cin.get(inputChar)) {
		return false;
	}

	switch (inputChar) {

		// Process returns
		case '\n':
		case '\r':
			return ProcessLine();

		case '\b':
			Backspace();
			break;

		case -32:
			// BUGBUG: An arrow meta-char on windows, I think, need to figure out exactly what this is, seems broken on linux
			if (!cin.get(inputChar)) {
				return false;
			}

			switch (inputChar) {
				case 72:
					// Up arrow
					while (commandLine.size()) {
						Backspace(); // remove the echo from the line ?
					}
					temporaryHistoryIndex = temporaryHistoryIndex ? (temporaryHistoryIndex - 1) : (HISTORY_SIZE - 1);
					commandLine = pHistory[temporaryHistoryIndex];
					cout << commandLine;
					break;
				case 80:
					// Down Arrow
					while (commandLine.size()) {
						Backspace(); // remove the echo from the line ?
					}
					temporaryHistoryIndex = ++temporaryHistoryIndex % HISTORY_SIZE;
					commandLine = pHistory[temporaryHistoryIndex];
					cout << commandLine;
					break;
				default:
					// Ignore others
					break;
			}
			break;

		default:
			// Add the character to the line
			commandLine += inputChar;
			break;
	}
	return true;
}

void CommandProcessor::Backspace() {
	if (commandLine.size()) {
		cout << "\b \b";
		cout.flush();
		commandLine = commandLine.substr(0, commandLine.length() - 1);
	}
}

bool CommandProcessor::ProcessLine() {

	previousResult = true;

	if (!commandLine.size()) {
		DisplayPrompt();
		return true;
	}

	pHistory[historyIndex++] = commandLine;
	historyIndex %= HISTORY_SIZE;
	temporaryHistoryIndex = historyIndex;

	// Process meta-commands first
	if (commandLine == "raw") {
		raw = true;
		DisplayPrompt();
		return true;

	}
	if (commandLine == "structured") {
		raw = false;
		DisplayPrompt();
		return true;
	} 

	// Process command line
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

	// Get rid of that pesky extra newline
	if (output.size()) {
		if (output[output.size() - 1] == '\n') {
			output = output.substr(0, output.size() - 1);
		}
		cout << output << endl;
	}

	// If this string is seen, exit
	if (output.find("Goodbye.") != std::string::npos) {
		return false;
	}

	DisplayPrompt();
	return true;
}


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

	g_pCommandProcessor->DisplayPrompt();
	while (g_pCommandProcessor->ProcessCharacter()) {}

	pAgent->UnregisterForPrintEvent(sml::smlEVENT_PRINT, callbackID2);
	pAgent->UnregisterForRunEvent(sml::smlEVENT_BEFORE_DECISION_CYCLE, callbackID1);

	// Don't delete agent, owned by kernel
	delete pKernel ;
	return 0;
}
