#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include <iostream>

#ifdef _MSC_VER
#include <crtdbg.h>
#endif 

#include <assert.h>

#include "sml_Connection.h"
#include "sml_Client.h"
#include "sml_AnalyzeXML.h"
#include "sml_ElementXML.h"
#include "sml_ClientEvents.h"
//#include "sml_ClientAgent.h"

using namespace std;

void PrintCallbackHandler(sml::smlEventId id, void* pUserData, sml::Agent* pAgent, char const* pMessage) {
	cout << pMessage;
}

void backspace(string& cmdline) {
	if (cmdline.size()) {
		cout << "\b \b";
		cout.flush();
		cmdline = cmdline.substr(0, cmdline.length() - 1);
	}
}

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
	cout << "Kernel created." << endl;

#ifdef _DEBUG
	// Comment this in or out to enable tracing of remote SML messages as they come in and out
	// pKernel->SetTraceCommunications(true) ;
#endif

	// NOTE: We don't delete the agent pointer.  It's owned by the kernel
	sml::Agent* pAgent;
	const char AGENT_NAME[] = "test";
	pAgent = pKernel->CreateAgent(AGENT_NAME) ;
	assert(pAgent);
	cout << "Agent 'test' created.\n";
	cout << "Use the meta-commands 'raw' and 'structured' to switch output style" << endl;

	// Register for print callbacks
	pAgent->RegisterForPrintEvent(sml::smlEVENT_PRINT, PrintCallbackHandler, 0);

	string cmdline;
	string output ;
	char input;
	bool previousResult = true;
	bool process;
	int historyIndex = 0;
	int temporaryHistoryIndex = 0;
	const int HISTORY_SIZE = 10;
	string history[HISTORY_SIZE];
	bool raw = true;
	bool cinFail = false;
	sml::AnalyzeXML* pStructuredResponse;
	
	string scriptFile;
	if (argc > 1) {
		scriptFile = "source ";
		scriptFile += argv[1];
		scriptFile += '\n';
	}
	string::iterator sfIter = scriptFile.begin();

	for (;;) {
		cout << previousResult;
		if (raw) {
			cout << " (raw)";
		} else {
			cout << " (structured)";
		}
		cout << " " << AGENT_NAME << "> ";
		cout.flush();
		cmdline.clear();
		output.clear();
		temporaryHistoryIndex = historyIndex;
		process = false;

		for (;;) {
			
			if (sfIter != scriptFile.end()) {
				input = *sfIter;
				++sfIter;
			} else {
				if (!cin.get(input)) {
					cinFail = true;
					break;
				}
			}

			switch (input) {
				case '\n':
				case '\r':
					process = true;
					break;

				case '\b':
					backspace(cmdline);
					break;

				case -32:
					if (!cin.get(input)) {
						cinFail = true;
						process = true;
						break;
					}
					switch (input) {
						case 72:
							// Up arrow
							while (cmdline.size()) {
								backspace(cmdline);
							}
							temporaryHistoryIndex = temporaryHistoryIndex ? (temporaryHistoryIndex - 1) : (HISTORY_SIZE - 1);
							cmdline = history[temporaryHistoryIndex];
							cout << cmdline;
							break;
						case 80:
							// Down Arrow
							while (cmdline.size()) {
								backspace(cmdline);
							}
							temporaryHistoryIndex = ++temporaryHistoryIndex % HISTORY_SIZE;
							cmdline = history[temporaryHistoryIndex];
							cout << cmdline;
							break;
						default:
							// Ignore others
							break;
					}
					break;

				default:
					//cout << input;
					cmdline += input;
					break;
			}
			if (process) break;
		}

		if (cinFail) {
			break;
		}

		//cout << endl;

		if (!cmdline.size()) {
			continue;
		}

		history[historyIndex++] = cmdline;
		historyIndex %= HISTORY_SIZE;

		if (cmdline == "raw") {
			raw = true;
		} else if (cmdline == "structured") {
			raw = false;
		} else if (raw) {
			output = pKernel->ExecuteCommandLine(cmdline.c_str(), AGENT_NAME);
			previousResult = pKernel->GetLastCommandLineResult() ;
		} else {
			pStructuredResponse = new sml::AnalyzeXML();
			previousResult = pKernel->ExecuteCommandLineXML(cmdline.c_str(), AGENT_NAME, pStructuredResponse);
			const sml::ElementXML* pResultTag = pStructuredResponse->GetResultTag();
			if (pResultTag) {
				char* pOutput = pResultTag->GenerateXMLString(true);
				if (pOutput) {
					output += pOutput;
				}
				pResultTag->DeleteString(pOutput);
			}

			const sml::ElementXML* pErrorTag = pStructuredResponse->GetErrorTag();
			if (pErrorTag) {
				char* pOutput = pErrorTag->GenerateXMLString(true);
				if (pOutput) {
					output += pOutput;
				}
				pErrorTag->DeleteString(pOutput);
			}

			delete pStructuredResponse;
		}
		if (output.size()) {
			if (output[output.size() - 1] == '\n') {
				output = output.substr(0, output.size() - 1);
			}
			cout << output << endl;
		}


		if (output.find("Goodbye.") != std::string::npos) {
			break;
		}
	}
	pAgent->UnregisterForPrintEvent(sml::smlEVENT_PRINT, PrintCallbackHandler, 0);
	delete pKernel ;
	return 0;
}
