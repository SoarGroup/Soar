#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_Aliases.h"

#include <iostream>
#include <fstream>
#include <string>

using namespace cli;
using namespace std;

Aliases::Aliases() {
}

bool Aliases::IsAlias(const std::string& command) {
	return m_AliasMap.find(command) != m_AliasMap.end();
}

bool Aliases::NewAlias(const std::vector<std::string>& substitution, const std::string& commandToSubstitute) {
	if (IsAlias(commandToSubstitute)) return false;
	m_AliasMap[commandToSubstitute] = substitution;
	return true;
}

bool Aliases::RemoveAlias(const std::string& command) {
	return m_AliasMap.erase(command) ? true : false;
}

std::string Aliases::List() {
	std::string result;
	for (AliasMapIter i = m_AliasMap.begin(); i != m_AliasMap.end(); ++i) {
		result += i->first;
		result += '=';
		for (vector<string>::iterator j = i->second.begin(); j != i->second.end(); ++j) {
			result += *j;
			result += ' ';
		}
		result += '\n';
	}	
	// remove trailing newline
	result = result.substr(0, result.size()-1);
	return result;
}

AliasMap::const_iterator Aliases::GetAliasMapBegin() {
	return m_AliasMap.begin();
}

AliasMap::const_iterator Aliases::GetAliasMapEnd() {
	return m_AliasMap.end();
}

bool Aliases::Translate(vector<string>& argv) {

	if (!IsAlias(argv[0])) return false;

	// Copy the alias out of the alias map
	vector<string> newArgv = (m_AliasMap.find(argv[0]))->second;

	// Add the args from the passed argv to the alias
	vector<string>::iterator vIter = argv.begin();

	// ... skipping the first command (substituting it with the alias from the alias map
	while (++vIter != argv.end()) {
		// ... copy each element to the end
		newArgv.push_back(*vIter);
	}

	// save the new argv
	argv = newArgv;

	// return true since we did a substitution
	return true;
}
