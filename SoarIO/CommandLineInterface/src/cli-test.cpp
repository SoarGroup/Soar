#include <iostream>
#include <conio.h>

#include "commandLineInterface.h"

using namespace std;
using namespace cli;

int main()
{
	string cmdline;
   string result;
	char input;
   bool previousResult = true;

	CommandLineInterface* cli = new CommandLineInterface();

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

      previousResult = cli->DoCommandInternal(cmdline.c_str());
      cli->GetLastResult(&result);
      cout << result << endl;

		if (cli->QuitCalled()) {
			break;
		}
	}

	exit (0);
}



