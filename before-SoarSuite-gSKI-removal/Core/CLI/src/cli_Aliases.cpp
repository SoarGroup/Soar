/////////////////////////////////////////////////////////////////
// Aliases class file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
// This class handles aliases for the command line interface.
//
/////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include <portability.h>

#include "cli_Aliases.h"

#include <string>
#include <assert.h>

using namespace cli;
using namespace std;

Aliases::Aliases() {
	// add-wme aw
	{
		string commandToSubstitute("aw");
		std::vector<std::string> substitution;
		substitution.push_back(string("add-wme"));
		NewAlias(substitution, commandToSubstitute);
	}
	// alias a
	{
		string commandToSubstitute("a");
		std::vector<std::string> substitution;
		substitution.push_back(string("alias"));
		NewAlias(substitution, commandToSubstitute);
	}
	// cd chdir
	{
		string commandToSubstitute("chdir");
		std::vector<std::string> substitution;
		substitution.push_back(string("cd"));
		NewAlias(substitution, commandToSubstitute);
	}
	// command-to-file ctf
	{
		string commandToSubstitute("ctf");
		std::vector<std::string> substitution;
		substitution.push_back(string("command-to-file"));
		NewAlias(substitution, commandToSubstitute);
	}
	// default-wme-depth set-default-depth
	{
		string commandToSubstitute("set-default-depth");
		std::vector<std::string> substitution;
		substitution.push_back(string("default-wme-depth"));
		NewAlias(substitution, commandToSubstitute);
	}
	// excise ex
	{
		string commandToSubstitute("ex");
		std::vector<std::string> substitution;
		substitution.push_back(string("excise"));
		NewAlias(substitution, commandToSubstitute);
	}
	// explain-backtraces eb
	{
		string commandToSubstitute("eb");
		std::vector<std::string> substitution;
		substitution.push_back(string("explain-backtraces"));
		NewAlias(substitution, commandToSubstitute);
	}
	// firing-counts fc
	{
		string commandToSubstitute("fc");
		std::vector<std::string> substitution;
		substitution.push_back(string("firing-counts"));
		NewAlias(substitution, commandToSubstitute);
	}
	// format-watch fw
	//{
	//	string commandToSubstitute("fw");
	//	std::vector<std::string> substitution;
	//	substitution.push_back(string("format-watch"));
	//	NewAlias(substitution, commandToSubstitute);
	//}
	// gds-print gds_print
	{
		string commandToSubstitute("gds_print");
		std::vector<std::string> substitution;
		substitution.push_back(string("gds-print"));
		NewAlias(substitution, commandToSubstitute);
	}
	// help h
	{
		string commandToSubstitute("h");
		std::vector<std::string> substitution;
		substitution.push_back(string("help"));
		NewAlias(substitution, commandToSubstitute);
	}
	// help man
	{
		string commandToSubstitute("man");
		std::vector<std::string> substitution;
		substitution.push_back(string("help"));
		NewAlias(substitution, commandToSubstitute);
	}
	// help ?
	{
		string commandToSubstitute("?");
		std::vector<std::string> substitution;
		substitution.push_back(string("help"));
		NewAlias(substitution, commandToSubstitute);
	}
	// indifferent-selection inds
	{
		string commandToSubstitute("inds");
		std::vector<std::string> substitution;
		substitution.push_back(string("indifferent-selection"));
		NewAlias(substitution, commandToSubstitute);
	}
	// init-soar init
	{
		string commandToSubstitute("init");
		std::vector<std::string> substitution;
		substitution.push_back(string("init-soar"));
		NewAlias(substitution, commandToSubstitute);
	}
	// init-soar is
	{
		string commandToSubstitute("is");
		std::vector<std::string> substitution;
		substitution.push_back(string("init-soar"));
		NewAlias(substitution, commandToSubstitute);
	}
	// learn l
	{
		string commandToSubstitute("l");
		std::vector<std::string> substitution;
		substitution.push_back(string("learn"));
		NewAlias(substitution, commandToSubstitute);
	}
	// ls dir
	{
		string commandToSubstitute("dir");
		std::vector<std::string> substitution;
		substitution.push_back(string("ls"));
		NewAlias(substitution, commandToSubstitute);
	}
	// print p
	{
		string commandToSubstitute("p");
		std::vector<std::string> substitution;
		substitution.push_back(string("print"));
		NewAlias(substitution, commandToSubstitute);
	}
	// print --chunks pc
	{
		string commandToSubstitute("pc");
		std::vector<std::string> substitution;
		substitution.push_back(string("print"));
		substitution.push_back(string("--chunks"));
		NewAlias(substitution, commandToSubstitute);
	}
	// preferences pr
	{
		string commandToSubstitute("pr");
		std::vector<std::string> substitution;
		substitution.push_back(string("preferences"));
		NewAlias(substitution, commandToSubstitute);
	}
	// pwd topd
	{
		string commandToSubstitute("topd");
		std::vector<std::string> substitution;
		substitution.push_back(string("pwd"));
		NewAlias(substitution, commandToSubstitute);
	}
	// pwatch pw
	{
		string commandToSubstitute("pw");
		std::vector<std::string> substitution;
		substitution.push_back(string("pwatch"));
		NewAlias(substitution, commandToSubstitute);
	}
	// quit exit
	{
		string commandToSubstitute("exit");
		std::vector<std::string> substitution;
		substitution.push_back(string("quit"));
		NewAlias(substitution, commandToSubstitute);
	}
	// run -d step
	{
		string commandToSubstitute("step");
		std::vector<std::string> substitution;
		substitution.push_back(string("run"));
		substitution.push_back(string("-d"));
		NewAlias(substitution, commandToSubstitute);
	}
	// run -d d
	{
		string commandToSubstitute("d");
		std::vector<std::string> substitution;
		substitution.push_back(string("run"));
		substitution.push_back(string("-d"));
		NewAlias(substitution, commandToSubstitute);
	}
	// run -e e
	{
		string commandToSubstitute("e");
		std::vector<std::string> substitution;
		substitution.push_back(string("run"));
		substitution.push_back(string("-e"));
		NewAlias(substitution, commandToSubstitute);
	}
	// remove-wme rw
	{
		string commandToSubstitute("rw");
		std::vector<std::string> substitution;
		substitution.push_back(string("remove-wme"));
		NewAlias(substitution, commandToSubstitute);
	}
	// rete-net rn
	{
		string commandToSubstitute("rn");
		std::vector<std::string> substitution;
		substitution.push_back(string("rete-net"));
		NewAlias(substitution, commandToSubstitute);
	}
	// soarnews sn
	{
		string commandToSubstitute("sn");
		std::vector<std::string> substitution;
		substitution.push_back(string("soarnews"));
		NewAlias(substitution, commandToSubstitute);
	}
	// stats st
	{
		string commandToSubstitute("st");
		std::vector<std::string> substitution;
		substitution.push_back(string("stats"));
		NewAlias(substitution, commandToSubstitute);
	}
	// start-system start
	{
		string commandToSubstitute("start");
		std::vector<std::string> substitution;
		substitution.push_back(string("start-system"));
		NewAlias(substitution, commandToSubstitute);
	}
	// stop-soar stop
	{
		string commandToSubstitute("stop");
		std::vector<std::string> substitution;
		substitution.push_back(string("stop-soar"));
		NewAlias(substitution, commandToSubstitute);
	}
	// stop-soar ss
	{
		string commandToSubstitute("ss");
		std::vector<std::string> substitution;
		substitution.push_back(string("stop-soar"));
		NewAlias(substitution, commandToSubstitute);
	}
	// stop-soar interrupt
	{
		string commandToSubstitute("interrupt");
		std::vector<std::string> substitution;
		substitution.push_back(string("stop-soar"));
		NewAlias(substitution, commandToSubstitute);
	}
		// preferences support
	{
		string commandToSubstitute("support");
		std::vector<std::string> substitution;
		substitution.push_back(string("preferences"));
		substitution.push_back(string("--object"));
		NewAlias(substitution, commandToSubstitute);
	}

	// unalias un
	{
		string commandToSubstitute("un");
		std::vector<std::string> substitution;
		substitution.push_back(string("unalias"));
		NewAlias(substitution, commandToSubstitute);
	}
	// watch w
	{
		string commandToSubstitute("w");
		std::vector<std::string> substitution;
		substitution.push_back(string("watch"));
		NewAlias(substitution, commandToSubstitute);
	}
	// print -i wmes
	{
		string commandToSubstitute("wmes");
		std::vector<std::string> substitution;
		substitution.push_back(string("print"));
		substitution.push_back(string("-i"));
		NewAlias(substitution, commandToSubstitute);
	}
	// print -v -d 100 varprint
	{
		string commandToSubstitute("varprint");
		std::vector<std::string> substitution;
		substitution.push_back(string("print"));
		substitution.push_back(string("-v"));
		substitution.push_back(string("-d"));
		substitution.push_back(string("100"));
		NewAlias(substitution, commandToSubstitute);
	}
}

bool Aliases::IsAlias(const std::string& command) {
	return m_AliasMap.find(command) != m_AliasMap.end();
}

void Aliases::NewAlias(const std::vector<std::string>& substitution, const std::string& commandToSubstitute) {
	if (IsAlias(commandToSubstitute)) {
		RemoveAlias(commandToSubstitute);
	}
	m_AliasMap[commandToSubstitute] = substitution;
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
