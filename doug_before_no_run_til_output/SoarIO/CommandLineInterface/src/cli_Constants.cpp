#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_Constants.h"

#include <iostream>
#include <fstream>

using namespace cli;
using namespace std;

char const* Constants::kCLIAddWME					= "add-wme";			
char const* Constants::kCLIAlias					= "alias";			
char const* Constants::kCLICD						= "cd";			
char const* Constants::kCLIDirs						= "dirs";
char const* Constants::kCLIEcho						= "echo";
char const* Constants::kCLIExcise					= "excise";
char const* Constants::kCLIFiringCounts				= "firing-counts";
char const* Constants::kCLIGDSPrint					= "gds-print";
char const* Constants::kCLIHelp						= "help";
char const* Constants::kCLIHelpEx					= "helpex";
char const* Constants::kCLIHome						= "home";
char const* Constants::kCLIIndifferentSelection		= "indifferent-selection";
char const* Constants::kCLIInitSoar					= "init-soar";
char const* Constants::kCLILearn					= "learn";
char const* Constants::kCLILog						= "log";
char const* Constants::kCLILS						= "ls";
char const* Constants::kCLIMatches					= "matches";
char const* Constants::kCLIMaxChunks				= "max-chunks";
char const* Constants::kCLIMaxElaborations			= "max-elaborations";
char const* Constants::kCLIMemories					= "memories";
char const* Constants::kCLIMultiAttributes			= "multi-attributes";
char const* Constants::kCLINumericIndifferentMode	= "numeric-indifferent-mode";
char const* Constants::kCLIOSupportMode				= "o-support-mode";
char const* Constants::kCLIPopD						= "popd";
char const* Constants::kCLIPrint					= "print";
char const* Constants::kCLIProductionFind			= "production-find";
char const* Constants::kCLIPushD					= "pushd";
char const* Constants::kCLIPWD						= "pwd";
char const* Constants::kCLIQuit						= "quit";
char const* Constants::kCLIRemoveWME				= "remove-wme";
char const* Constants::kCLIReteNet					= "rete-net";
char const* Constants::kCLIRun						= "run";
char const* Constants::kCLISoar8					= "soar8";
char const* Constants::kCLISource					= "source";
char const* Constants::kCLISP						= "sp";
char const* Constants::kCLIStats					= "stats";
char const* Constants::kCLIStopSoar					= "stop-soar";
char const* Constants::kCLITime						= "time";
char const* Constants::kCLITimers					= "timers";
char const* Constants::kCLIVersion					= "version";
char const* Constants::kCLIWaitSNC					= "waitsnc";
char const* Constants::kCLIWarnings					= "warnings";
char const* Constants::kCLIWatch					= "watch";

Constants::Constants() {
	ifstream usageFile("usage.txt");

	m_UsageFileAvailable = usageFile ? true : false;

	if (m_UsageFileAvailable) {
		LoadUsage(usageFile);
	}
	usageFile.close();
}

std::list<std::string> Constants::GetCommandList() {
	UsageMapConstIter iter = m_UsageMap.begin();
	std::list<std::string> result;
	while (iter != m_UsageMap.end()) {
		result.push_back(iter->first);
		++iter;
	}
	return result;
}

bool Constants::GetUsageFor(const std::string& command, std::string& output) {
	if (m_UsageFileAvailable) {
		if (m_UsageMap.find(command) == m_UsageMap.end()) {
			return false;
		}
		//output = m_UsageMap[command];
		// FIXME: update for release
		output = "Usage file depricated until release, please see\n\t http://winter.eecs.umich.edu/soarwiki/Soar_Command_Line_Interface \nfor help on the command line.";
		return true;
	}
	return false;
}

bool Constants::GetExtendedUsageFor(const std::string& command, std::string& output) {
	if (m_UsageFileAvailable) {
		if (m_ExtendedUsageMap.find(command) == m_ExtendedUsageMap.end()) {
			return false;
		}
		//output = m_ExtendedUsageMap[command];
		// FIXME: update for release
		output = "Usage file depricated until release, please see\n\t http://winter.eecs.umich.edu/soarwiki/Soar_Command_Line_Interface \nfor help on the command line.";
		return true;
	}
	return false;
}

bool Constants::IsUsageFileAvailable() {
	return m_UsageFileAvailable;
}

void Constants::LoadUsage(ifstream& usageFile) {

	string line;
	while (getline(usageFile, line)) {
		if (!line.length() || (line[0] == '#')) {
			continue;
		}

		if (line.length()) {
			m_UsageMap[line] = GetUsage(usageFile);
			m_ExtendedUsageMap[line] = GetExtendedUsage(usageFile);
		}
	}
}

string Constants::GetUsage(ifstream& usageFile) {
	string line, usage;
	while (getline(usageFile, line)) {

		if (line.length()) {
			if (line[0] == '#') {
				continue;
			}
			if (line[0] == '~') {
				break;
			}
		}
		usage += line;
		usage += '\n';
	}
	// TODO: remove extra newline on end?
	return usage;
}

string Constants::GetExtendedUsage(ifstream& usageFile) {
	string line, usage;
	while (getline(usageFile, line)) {

		if (line.length()) {
			if (line[0] == '#') {
				continue;
			}
			if (line[0] == '*') {
				break;
			}
		}
		usage += line;
		usage += '\n';
	}
	// TODO: remove extra newline on end?
	return usage;
}
