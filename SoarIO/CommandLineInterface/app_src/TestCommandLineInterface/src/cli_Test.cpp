#include <iostream>
#include <conio.h>
#include <crtdbg.h>

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

	//CommandLineInterface* cli;
	sml::Kernel* pKernel;
	sml::Agent* pAgent;

	//gSKI::
	//gSKI::IKernel* pgKernel;
	//gSKI::IAgent* pgAgent;

	if (useSML) {

		// Create an embedded connection to the kernel
		pKernel = sml::Kernel::CreateEmbeddedConnection("KernelSML") ;
		cout << "Kernel created." << endl;

		// NOTE: We don't delete the agent pointer.  It's owned by the kernel
		pAgent = pKernel->CreateAgent(AGENT_NAME) ;
		cout << "Agent 'test' created." << endl;

	} else {
		//cli = new CommandLineInterface();
		//pgKernel = 
		//cli->SetKernel(pgKernel);
	}

	string cmdline;
	string output ;
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
			
			output = pKernel->ExecuteCommandLine(cmdline.c_str(), AGENT_NAME);
			previousResult = pKernel->GetLastCommandLineResult() ;
		    cout << output << endl;

			if (output == "Goodbye.") {
				break;
			}

		} else {
			//char const* pOutput;
			//cli->DoCommand(pAgent, cmdline.c_str(), pOutput, pError);
		 //   cout << output << endl;

			//if (std::string(pOutput) == "Goodbye.") {
			//	break;
			//}
		}
	}

	if (useSML) {
		delete pKernel ;

	//} else {
	//	delete cli;
	}

	exit (0);
}
