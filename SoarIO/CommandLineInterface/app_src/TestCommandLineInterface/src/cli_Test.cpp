#include <iostream>
#include <conio.h>
#include <crtdbg.h>

#include "sml_Connection.h"
#include "sml_Client.h"

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
	
	string scriptFile;
	if (argc > 1) {
		scriptFile = "source ";
		scriptFile += argv[1];
		scriptFile += '\n';
	}
	string::iterator sfIter = scriptFile.begin();

	for (;;) {
		cout << previousResult << " " << AGENT_NAME << "> ";
		cout.flush();
		cmdline.clear();
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

		history[historyIndex++] = cmdline;
		historyIndex %= HISTORY_SIZE;

		output = pKernel->ExecuteCommandLine(cmdline.c_str(), AGENT_NAME);
		previousResult = pKernel->GetLastCommandLineResult() ;
		cout << output << endl;

		if (output == "Goodbye.") {
			break;
		}
	}
	delete pKernel ;
	exit (0);
}
