#include "cli_Aliases.h"

#include <iostream>
#include <fstream>
#include <string>

using namespace cli;
using namespace std;

Aliases::Aliases() {
	ifstream aliasesFile("aliases.txt");
	if (aliasesFile) {
		LoadAliases(aliasesFile);
		aliasesFile.close();
	}
}

void Aliases::LoadAliases(ifstream& aliasesFile) {

	string line;
	string::iterator iter;
	vector<string> args;
	string arg;

	// For each line in the file
	while (getline(aliasesFile, line)) {

		// Skip if pounded
		if (line[0] == '#') {
			continue;
		}
		
		// Count args per line
		args.clear();

		// Read all args
		while(line.length()) {

			// Remove leading whitespace
			iter = line.begin();
			while (isspace(*iter)) {
				line.erase(iter);

				if (!line.length()) {
					// Nothing but space left
					break;
				}
				
				// Next character
				iter = line.begin();
			}

			// Was it actually trailing whitespace?
			if (!line.length()) {
				// Nothing left to do
				break;
			}
			
			// We have at least one, read the word
			arg.clear();
			while (!isspace(*iter)) {
				arg += *iter;
				line.erase(iter);
				iter = line.begin();
				if (iter == line.end()) {
					break;
				}
			}

			// Add to args
			args.push_back(arg);
		}
		
		// Add to alias map if we have at least one arg and one in the args vector
		if (args.size() && arg.size()) {
			args.pop_back();
			aliasMap[arg] = args;
		}
	}

}

bool Aliases::Translate(vector<string>& argv) {

	// Look up the command to see if it is in the alias map
	AliasMapIter amIter = aliasMap.find(argv[0]);
	if (amIter == aliasMap.end()) {
		// It isn't
		return false;
	}

	// Copy the alias out of the alias map
	vector<string> newArgv = amIter->second;

	// Add the args from the passed argv to the alias
	vector<string>::iterator vIter = argv.begin();

	// ... skipping the first command (substituting it with the alias from the alias map
	++vIter;
	while (vIter != argv.end()) {
		// ... copy each element to the end
		newArgv.push_back(*vIter);
		++vIter;
	}

	// debug
	//vIter = argv.begin();
	//while (vIter != argv.end()) {
	//	cout << *vIter << " ";
	//	++vIter;
	//}
	//cout << "==> ";
	//vIter = newArgv.begin();
	//while (vIter != newArgv.end()) {
	//	cout << *vIter << " ";
	//	++vIter;
	//}
	//cout << endl;

	// save the new argv
	argv = newArgv;

	// return true signifying we did a substitution
	return true;
}
