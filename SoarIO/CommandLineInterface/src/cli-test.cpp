#include <iostream>
#include <conio.h>

#include "sml_Connection.h"
#include "sml_Client.h"

#include "commandLineInterface.h"

using namespace std;
using namespace cli;

int main(int argc, char** argv)
{
	bool useSML = true;

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

		// NOTE: We don't delete the agent pointer.  It's owned by the kernel
		pAgent = pKernel->CreateAgent("test") ;

	} else {
		cli = new CommandLineInterface();
	}

	// Simple command line client that just uses CommandLineInterface directly
	string cmdline;
	string result;
	char input;
	bool previousResult = true;

	for (;;) {
		cout << '\n' << previousResult << " cli> ";
		cout.flush();
		cmdline.clear();
		for (;;) {
			input = getche();
			if (input == '\n' || input == '\r') {
				break;
			} else if (input == '\b') {
				cout << " \b";
				cout.flush();
				cmdline = cmdline.substr(0, cmdline.length() - 1);
			} else {
				cmdline += input;
			}
		}
		cout << endl;

		if (useSML) {
			
			std::string result; 
			previousResult = pKernel->ProcessCommandLine(cmdline.c_str(), "test", &result);
			cout << result << endl;

			if (result == "Goodbye.") {
				break;
			}

		} else {

			previousResult = cli->DoCommandInternal(cmdline.c_str());
			cli->GetLastResult(&result);
			cout << result << endl;

			if (cli->QuitCalled()) {
				break;
			}
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
