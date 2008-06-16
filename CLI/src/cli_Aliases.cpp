/////////////////////////////////////////////////////////////////
// Aliases class file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
// This class handles aliases for the command line interface.
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "cli_Aliases.h"

#include <assert.h>

using namespace cli;

Aliases::Aliases() {
	// add-wme aw
	{
		std::string commandToSubstitute("aw");
		std::vector<std::string> substitution;
		substitution.push_back(std::string("add-wme"));
		NewAlias(substitution, commandToSubstitute);
	}
	// alias a
	{
		std::string commandToSubstitute("a");
		std::vector<std::string> substitution;
		substitution.push_back(std::string("alias"));
		NewAlias(substitution, commandToSubstitute);
	}
	// cd chdir
	{
		std::string commandToSubstitute("chdir");
		std::vector<std::string> substitution;
		substitution.push_back(std::string("cd"));
		NewAlias(substitution, commandToSubstitute);
	}
	// command-to-file ctf
	{
		std::string commandToSubstitute("ctf");
		std::vector<std::string> substitution;
		substitution.push_back(std::string("command-to-file"));
		NewAlias(substitution, commandToSubstitute);
	}
	// default-wme-depth set-default-depth
	{
		std::string commandToSubstitute("set-default-depth");
		std::vector<std::string> substitution;
		substitution.push_back(std::string("default-wme-depth"));
		NewAlias(substitution, commandToSubstitute);
	}
	// excise ex
	{
		std::string commandToSubstitute("ex");
		std::vector<std::string> substitution;
		substitution.push_back(std::string("excise"));
		NewAlias(substitution, commandToSubstitute);
	}
	// explain-backtraces eb
	{
		std::string commandToSubstitute("eb");
		std::vector<std::string> substitution;
		substitution.push_back(std::string("explain-backtraces"));
		NewAlias(substitution, commandToSubstitute);
	}
	// firing-counts fc
	{
		std::string commandToSubstitute("fc");
		std::vector<std::string> substitution;
		substitution.push_back(std::string("firing-counts"));
		NewAlias(substitution, commandToSubstitute);
	}
	// format-watch fw
	//{
	//	std::string commandToSubstitute("fw");
	//	std::vector<std::string> substitution;
	//	substitution.push_back(std::string("format-watch"));
	//	NewAlias(substitution, commandToSubstitute);
	//}
	// gds-print gds_print
	{
		std::string commandToSubstitute("gds_print");
		std::vector<std::string> substitution;
		substitution.push_back(std::string("gds-print"));
		NewAlias(substitution, commandToSubstitute);
	}
	// help h
	{
		std::string commandToSubstitute("h");
		std::vector<std::string> substitution;
		substitution.push_back(std::string("help"));
		NewAlias(substitution, commandToSubstitute);
	}
	// help man
	{
		std::string commandToSubstitute("man");
		std::vector<std::string> substitution;
		substitution.push_back(std::string("help"));
		NewAlias(substitution, commandToSubstitute);
	}
	// help ?
	{
		std::string commandToSubstitute("?");
		std::vector<std::string> substitution;
		substitution.push_back(std::string("help"));
		NewAlias(substitution, commandToSubstitute);
	}
	// indifferent-selection inds
	{
		std::string commandToSubstitute("inds");
		std::vector<std::string> substitution;
		substitution.push_back(std::string("indifferent-selection"));
		NewAlias(substitution, commandToSubstitute);
	}
	// init-soar init
	{
		std::string commandToSubstitute("init");
		std::vector<std::string> substitution;
		substitution.push_back(std::string("init-soar"));
		NewAlias(substitution, commandToSubstitute);
	}
	// init-soar is
	{
		std::string commandToSubstitute("is");
		std::vector<std::string> substitution;
		substitution.push_back(std::string("init-soar"));
		NewAlias(substitution, commandToSubstitute);
	}
	// learn l
	{
		std::string commandToSubstitute("l");
		std::vector<std::string> substitution;
		substitution.push_back(std::string("learn"));
		NewAlias(substitution, commandToSubstitute);
	}
	// ls dir
	{
		std::string commandToSubstitute("dir");
		std::vector<std::string> substitution;
		substitution.push_back(std::string("ls"));
		NewAlias(substitution, commandToSubstitute);
	}
	// print p
	{
		std::string commandToSubstitute("p");
		std::vector<std::string> substitution;
		substitution.push_back(std::string("print"));
		NewAlias(substitution, commandToSubstitute);
	}
	// print --chunks pc
	{
		std::string commandToSubstitute("pc");
		std::vector<std::string> substitution;
		substitution.push_back(std::string("print"));
		substitution.push_back(std::string("--chunks"));
		NewAlias(substitution, commandToSubstitute);
	}
	// preferences pr
	{
		std::string commandToSubstitute("pr");
		std::vector<std::string> substitution;
		substitution.push_back(std::string("preferences"));
		NewAlias(substitution, commandToSubstitute);
	}
	// pwd topd
	{
		std::string commandToSubstitute("topd");
		std::vector<std::string> substitution;
		substitution.push_back(std::string("pwd"));
		NewAlias(substitution, commandToSubstitute);
	}
	// pwatch pw
	{
		std::string commandToSubstitute("pw");
		std::vector<std::string> substitution;
		substitution.push_back(std::string("pwatch"));
		NewAlias(substitution, commandToSubstitute);
	}
	// quit exit
	{
		std::string commandToSubstitute("exit");
		std::vector<std::string> substitution;
		substitution.push_back(std::string("quit"));
		NewAlias(substitution, commandToSubstitute);
	}
	// run -d step
	{
		std::string commandToSubstitute("step");
		std::vector<std::string> substitution;
		substitution.push_back(std::string("run"));
		substitution.push_back(std::string("-d"));
		NewAlias(substitution, commandToSubstitute);
	}
	// run -d d
	{
		std::string commandToSubstitute("d");
		std::vector<std::string> substitution;
		substitution.push_back(std::string("run"));
		substitution.push_back(std::string("-d"));
		NewAlias(substitution, commandToSubstitute);
	}
	// run -e e
	{
		std::string commandToSubstitute("e");
		std::vector<std::string> substitution;
		substitution.push_back(std::string("run"));
		substitution.push_back(std::string("-e"));
		NewAlias(substitution, commandToSubstitute);
	}
	// remove-wme rw
	{
		std::string commandToSubstitute("rw");
		std::vector<std::string> substitution;
		substitution.push_back(std::string("remove-wme"));
		NewAlias(substitution, commandToSubstitute);
	}
	// rete-net rn
	{
		std::string commandToSubstitute("rn");
		std::vector<std::string> substitution;
		substitution.push_back(std::string("rete-net"));
		NewAlias(substitution, commandToSubstitute);
	}
	// soarnews sn
	{
		std::string commandToSubstitute("sn");
		std::vector<std::string> substitution;
		substitution.push_back(std::string("soarnews"));
		NewAlias(substitution, commandToSubstitute);
	}
	// stats st
	{
		std::string commandToSubstitute("st");
		std::vector<std::string> substitution;
		substitution.push_back(std::string("stats"));
		NewAlias(substitution, commandToSubstitute);
	}
	// start-system start
	{
		std::string commandToSubstitute("start");
		std::vector<std::string> substitution;
		substitution.push_back(std::string("start-system"));
		NewAlias(substitution, commandToSubstitute);
	}
	// stop-soar stop
	{
		std::string commandToSubstitute("stop");
		std::vector<std::string> substitution;
		substitution.push_back(std::string("stop-soar"));
		NewAlias(substitution, commandToSubstitute);
	}
	// stop-soar ss
	{
		std::string commandToSubstitute("ss");
		std::vector<std::string> substitution;
		substitution.push_back(std::string("stop-soar"));
		NewAlias(substitution, commandToSubstitute);
	}
	// stop-soar interrupt
	{
		std::string commandToSubstitute("interrupt");
		std::vector<std::string> substitution;
		substitution.push_back(std::string("stop-soar"));
		NewAlias(substitution, commandToSubstitute);
	}
		// preferences support
	{
		std::string commandToSubstitute("support");
		std::vector<std::string> substitution;
		substitution.push_back(std::string("preferences"));
		substitution.push_back(std::string("--object"));
		NewAlias(substitution, commandToSubstitute);
	}

	// unalias un
	{
		std::string commandToSubstitute("un");
		std::vector<std::string> substitution;
		substitution.push_back(std::string("unalias"));
		NewAlias(substitution, commandToSubstitute);
	}
	// watch w
	{
		std::string commandToSubstitute("w");
		std::vector<std::string> substitution;
		substitution.push_back(std::string("watch"));
		NewAlias(substitution, commandToSubstitute);
	}
	// print -i wmes
	{
		std::string commandToSubstitute("wmes");
		std::vector<std::string> substitution;
		substitution.push_back(std::string("print"));
		substitution.push_back(std::string("-i"));
		NewAlias(substitution, commandToSubstitute);
	}
	// print -v -d 100 varprint
	{
		std::string commandToSubstitute("varprint");
		std::vector<std::string> substitution;
		substitution.push_back(std::string("print"));
		substitution.push_back(std::string("-v"));
		substitution.push_back(std::string("-d"));
		substitution.push_back(std::string("100"));
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
		for (std::vector<std::string>::iterator j = i->second.begin(); j != i->second.end(); ++j) {
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

bool Aliases::Translate(std::vector<std::string>& argv) {

	if (!IsAlias(argv[0])) return false;

	// Copy the alias out of the alias map
	std::vector<std::string> newArgv = (m_AliasMap.find(argv[0]))->second;

	// Add the args from the passed argv to the alias
	std::vector<std::string>::iterator vIter = argv.begin();

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
