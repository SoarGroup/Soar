#include "cli_Constants.h"

#include <iostream>
#include <fstream>

using namespace cli;
using namespace std;

char const* Constants::kCLISyntaxError		= "Command syntax error.\n";

char const* Constants::kCLIAddWME			= "add-wme";
char const* Constants::kCLICD				= "cd";			
char const* Constants::kCLIDir				= "dir";		// alias for ls
char const* Constants::kCLIEcho				= "echo";
char const* Constants::kCLIExcise			= "excise";
char const* Constants::kCLIExit				= "exit";		// alias for quit
char const* Constants::kCLIInit				= "init";		// alias for init-soar
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
std::string Constants::GetUsageFor(const std::string& command) {
	if (m_UsageFileAvailable) {
		return m_UsageMap[command];

	}

	return "Help not available (no usage.txt file found).";
}

void Constants::LoadUsage(ifstream& usageFile) {

	string line;
	while (getline(usageFile, line)) {
		string debug = GetUsage(usageFile);
		if (line.length()) {
			m_UsageMap[line] = debug;
		}
	}
}

string Constants::GetUsage(ifstream& usageFile) {
	string line, usage;
	while (getline(usageFile, line)) {
		if (line == "***") {
			break;
		}
		usage += line;
		usage += '\n';
	}
	return usage;
}
