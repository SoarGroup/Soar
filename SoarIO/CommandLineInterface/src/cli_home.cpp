#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include <iostream>
#include <fstream>

#include "cli_Constants.h"
#include "cli_Aliases.h"

using namespace cli;
using namespace std;

bool CommandLineInterface::ParseHome(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	unused(pAgent);

	// Only takes one optional argument, the directory to change home to
	if (argv.size() > 2) {
		return SetError(CLIError::kTooManyArgs);
	}
	if (argv.size() > 1) {
		return DoHome(&(argv[1]));
	}
	return DoHome();
}

EXPORT bool CommandLineInterface::DoHome(std::string* pDirectory) {

	if (pDirectory) {
		// Change to the passed directory if any to make sure it is valid
		// also need usage/aliases files from that directory
		if (!DoCD(pDirectory)) {
			return false;
		}

	}
	// Set Home to current directory
	if (!GetCurrentWorkingDirectory(m_HomeDirectory)) {
		return false;
	}

	// Load aliases from file
	ifstream aliasesFile("aliases.txt");
	if (aliasesFile) {
		string line;
		string::iterator iter;
		vector<string> args;
		string arg;

		// For each line in the file
		while (getline(aliasesFile, line)) {

			// Skip if pounded
			if (line[0] == '#') continue;
			
			// Count args per line
			args.clear();

			// Read all args
			while(line.length()) {

				// Remove leading whitespace
				iter = line.begin();
				while (isspace(*iter)) {
					line.erase(iter);
					if (!line.length()) break; // Nothing but space left
					
					// Next character
					iter = line.begin();
				}

				// Was it actually trailing whitespace?
				if (!line.length()) break;	// Nothing left to do
				
				// We have at least one, read the word
				arg.clear();
				while (!isspace(*iter)) {
					arg += *iter;
					line.erase(iter);
					iter = line.begin();
					if (iter == line.end()) break;
				}

				// Add to args
				args.push_back(arg);
			}
			
			// Add to aliases if we have at least one arg and one in the args vector
			if (args.size() && arg.size()) {
				args.pop_back();
				m_Aliases.RemoveAlias(arg);
				m_Aliases.NewAlias(args, arg);
			}
		}
	}
	return true;
}

