#include <iostream>
#include <conio.h>

#include "sml_Connection.h"
#include "sml_Client.h"

#include "cli_CommandLineInterface.h"

using namespace std;
using namespace cli;

void backspace(string& cmdline) {
	if (cmdline.size()) {
		cout << "\b \b";
		cout.flush();
		cmdline = cmdline.substr(0, cmdline.length() - 1);
	}
}

int main(int argc, char** argv)
{
	bool useSML = true;
	const char AGENT_NAME[] = "test";

	const int HISTORY_SIZE = 10;
	string history[HISTORY_SIZE];
	int historyIndex = 0;
	int temporaryHistoryIndex = 0;

	// check to see if we should use SML
	if (argc > 1) {
		string argv1 = argv[1];
		if (argv1 == "-e") {
			useSML = false;
		}
	}

	CommandLineInterface* cli;
	sml::Connection* pConnection;
	sml::Kernel* pKernel;
	sml::Agent* pAgent;

	if (useSML) {

		// Create a connection
		ErrorCode error;
		pConnection = sml::Connection::CreateEmbeddedConnection("KernelSML", &error) ;

		pKernel = new sml::Kernel(pConnection) ;
		cout << "Kernel created." << endl;

		// NOTE: We don't delete the agent pointer.  It's owned by the kernel
		pAgent = pKernel->CreateAgent(AGENT_NAME) ;
		cout << "Agent 'test' created." << endl;

	} else {
		cli = new CommandLineInterface();
	}

	// Simple command line client that just uses CommandLineInterface directly
	string cmdline;
	char input;
	bool previousResult = true;
	bool process;

	for (;;) {
		cout << previousResult << " " << AGENT_NAME << "> ";
		cout.flush();
		cmdline.clear();
		temporaryHistoryIndex = historyIndex;
		process = false;

		for (;;) {
			input = getch();

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

		if (useSML) {
			
			previousResult = pKernel->ProcessCommandLine(cmdline.c_str(), AGENT_NAME);
			if (pKernel->GetLastCommandLineResult()) {
				cout << pKernel->GetLastCommandLineResult() << endl;
			}

			if (string(pKernel->GetLastCommandLineResult()) == "Goodbye.") {
				break;
			}

		} else {

			cout << "Not implemented." << endl;
			break;
			//previousResult = cli->DoCommandInternal(cmdline.c_str());
			//cli->GetLastResult(&result);
			//cout << result << endl;

			//if (cli->QuitCalled()) {
			//	break;
			//}
		}
	}

	if (useSML) {
		delete pKernel ;

		////destroy connection
		//cout << "Closing connection..." << endl << endl;

		pConnection->CloseConnection();
		delete pConnection ;
	} else {
		delete cli;
	}

	exit (0);
}
