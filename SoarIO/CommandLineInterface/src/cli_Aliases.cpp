#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_Aliases.h"

#include <string>
#include <assert.h>

using namespace cli;
using namespace std;

Aliases::Aliases() {
	// alias -d unalias
	{
		string commandToSubstitute("unalias");
		std::vector<std::string> substitution;
		substitution.push_back(string("alias"));
		substitution.push_back(string("-d"));
		assert(NewAlias(substitution, commandToSubstitute));
	}
	// cd chdir
	{
		string commandToSubstitute("chdir");
		std::vector<std::string> substitution;
		substitution.push_back(string("cd"));
		assert(NewAlias(substitution, commandToSubstitute));
	}
	// default-wme-depth set-default-depth
	{
		string commandToSubstitute("set-default-depth");
		std::vector<std::string> substitution;
		substitution.push_back(string("default-wme-depth"));
		assert(NewAlias(substitution, commandToSubstitute));
	}
	// firing-counts fc
	{
		string commandToSubstitute("fc");
		std::vector<std::string> substitution;
		substitution.push_back(string("firing-counts"));
		assert(NewAlias(substitution, commandToSubstitute));
	}
	// gds-print gds_print
	{
		string commandToSubstitute("gds_print");
		std::vector<std::string> substitution;
		substitution.push_back(string("gds-print"));
		assert(NewAlias(substitution, commandToSubstitute));
	}
	// help man
	{
		string commandToSubstitute("man");
		std::vector<std::string> substitution;
		substitution.push_back(string("help"));
		assert(NewAlias(substitution, commandToSubstitute));
	}
	// help ?
	{
		string commandToSubstitute("?");
		std::vector<std::string> substitution;
		substitution.push_back(string("help"));
		assert(NewAlias(substitution, commandToSubstitute));
	}
	// init-soar init
	{
		string commandToSubstitute("init");
		std::vector<std::string> substitution;
		substitution.push_back(string("init-soar"));
		assert(NewAlias(substitution, commandToSubstitute));
	}
	// ls dir
	{
		string commandToSubstitute("dir");
		std::vector<std::string> substitution;
		substitution.push_back(string("ls"));
		assert(NewAlias(substitution, commandToSubstitute));
	}
	// print p
	{
		string commandToSubstitute("p");
		std::vector<std::string> substitution;
		substitution.push_back(string("print"));
		assert(NewAlias(substitution, commandToSubstitute));
	}
	// pwd topd
	{
		string commandToSubstitute("topd");
		std::vector<std::string> substitution;
		substitution.push_back(string("pwd"));
		assert(NewAlias(substitution, commandToSubstitute));
	}
	// quit exit
	{
		string commandToSubstitute("exit");
		std::vector<std::string> substitution;
		substitution.push_back(string("quit"));
		assert(NewAlias(substitution, commandToSubstitute));
	}
	// run -d step
	{
		string commandToSubstitute("step");
		std::vector<std::string> substitution;
		substitution.push_back(string("run"));
		substitution.push_back(string("-d"));
		assert(NewAlias(substitution, commandToSubstitute));
	}
	// run -d d
	{
		string commandToSubstitute("d");
		std::vector<std::string> substitution;
		substitution.push_back(string("run"));
		substitution.push_back(string("-d"));
		assert(NewAlias(substitution, commandToSubstitute));
	}
	// run -e e
	{
		string commandToSubstitute("e");
		std::vector<std::string> substitution;
		substitution.push_back(string("run"));
		substitution.push_back(string("-e"));
		assert(NewAlias(substitution, commandToSubstitute));
	}
	// stop-soar stop
	{
		string commandToSubstitute("stop");
		std::vector<std::string> substitution;
		substitution.push_back(string("stop-soar"));
		assert(NewAlias(substitution, commandToSubstitute));
	}
	// stop-soar interrupt
	{
		string commandToSubstitute("interrupt");
		std::vector<std::string> substitution;
		substitution.push_back(string("stop-soar"));
		assert(NewAlias(substitution, commandToSubstitute));
	}
	// watch w
	{
		string commandToSubstitute("w");
		std::vector<std::string> substitution;
		substitution.push_back(string("watch"));
		assert(NewAlias(substitution, commandToSubstitute));
	}
	// print -i wmes
	{
		string commandToSubstitute("wmes");
		std::vector<std::string> substitution;
		substitution.push_back(string("print"));
		substitution.push_back(string("-i"));
		assert(NewAlias(substitution, commandToSubstitute));
	}
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

std::string Aliases::List(const std::string* pCommand) {
	std::string result;

	for (AliasMapIter i = m_AliasMap.begin(); i != m_AliasMap.end(); ++i) {
		if (pCommand) {
			if (i->first != *pCommand) continue;
		}
		result += i->first;
		result += '=';
		for (vector<string>::iterator j = i->second.begin(); j != i->second.end(); ++j) {
			result += *j;
			result += ' ';
		}
		if (pCommand) return result;
		result += '\n';
	}

	// didn't find one?
	if (pCommand) return result;

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
