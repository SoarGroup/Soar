#include "cli_Constants.h"

#include <iostream>
#include <fstream>

using namespace cli;
using namespace std;

char const* Constants::kCLISyntaxError		= "Command syntax error.";
char const* Constants::kCLINoUsageInfo		= "Usage information not found for that command.";

char const* Constants::kCLICD				= "cd";			
char const* Constants::kCLIEcho				= "echo";
char const* Constants::kCLIExcise			= "excise";
char const* Constants::kCLIHelp				= "help";
char const* Constants::kCLIInitSoar			= "init-soar";
char const* Constants::kCLILearn			= "learn";
char const* Constants::kCLILog				= "log";
char const* Constants::kCLILS				= "ls";
char const* Constants::kCLIMultiAttributes	= "multi-attributes";
char const* Constants::kCLIPopD				= "popd";
char const* Constants::kCLIPrint			= "print";
char const* Constants::kCLIPushD			= "pushd";
char const* Constants::kCLIPWD				= "pwd";
char const* Constants::kCLIQuit				= "quit";
char const* Constants::kCLIRun				= "run";
char const* Constants::kCLISource			= "source";
char const* Constants::kCLISP				= "sp";
char const* Constants::kCLIStopSoar			= "stop-soar";
char const* Constants::kCLITime				= "time";
char const* Constants::kCLIWatch			= "watch";

Constants::Constants() {
	ifstream usageFile ("usage.txt");

	m_UsageFileAvailable = usageFile ? true : false;

	if (m_UsageFileAvailable) {
		LoadUsage(usageFile);
	}
	usageFile.close();
}

Constants::~Constants() {
	// TODO: empty map?
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
		output = m_UsageMap[command];
		return true;
	}

	output = "Help not available (no usage.txt file found).";
	return false;
}

void Constants::LoadUsage(ifstream& usageFile) {

	string line;
	while (getline(usageFile, line)) {
		if (!line.length() || (line[0] == '#')) {
			continue;
		}

		string debug = GetUsage(usageFile);
		if (line.length()) {
			m_UsageMap[line] = debug;
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
