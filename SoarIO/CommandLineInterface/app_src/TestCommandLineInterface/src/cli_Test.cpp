#include <iostream>
#include <conio.h>
#include <crtdbg.h>

#include "sml_Connection.h"
#include "sml_Client.h"
#include "sml_AnalyzeXML.h"
#include "sml_ElementXML.h"

using namespace std;

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
	sml::Kernel* pKernel;
	pKernel = sml::Kernel::CreateEmbeddedConnection("KernelSML") ;
	cout << "Kernel created." << endl;

	// NOTE: We don't delete the agent pointer.  It's owned by the kernel
	sml::Agent* pAgent;
	const char AGENT_NAME[] = "test";
	pAgent = pKernel->CreateAgent(AGENT_NAME) ;
	cout << "Agent 'test' created." << endl;

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
				input = getch();
			}

			switch (input) {
				case '`':
					raw = !raw;
					cmdline.clear();
					input = '\n';
					// falls through

				case '\n':
				case '\r':
					process = true;
					break;

				case '\b':
					backspace(cmdline);
					break;

				case -32:
					switch (getch()) {
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
					cout << input;
					cmdline += input;
					break;
			}
			if (process) break;
		}
		cout << endl;

		if (!cmdline.size()) {
			continue;
		}

		history[historyIndex++] = cmdline;
		historyIndex %= HISTORY_SIZE;

		if (raw) {
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

		cout << output << endl;

		if (output.find("Goodbye.") != std::string::npos) {
			break;
		}
	}
	delete pKernel ;
	return 0;
}
