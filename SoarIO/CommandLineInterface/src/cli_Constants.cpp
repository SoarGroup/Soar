#include "cli_Constants.h"

#include <iostream>
#include <fstream>

using namespace cli;
using namespace std;

char const* Constants::kCLISyntaxError		= "Command syntax error.";
char const* Constants::kCLINoUsageInfo		= "Usage information not found for that command.";
char const* Constants::kCLINoUsageFile		= "Usage file not available (error opening file).";

char const* Constants::kCLICD				= "cd";			
char const* Constants::kCLIEcho				= "echo";
char const* Constants::kCLIExcise			= "excise";
char const* Constants::kCLIHelp				= "help";
char const* Constants::kCLIHelpEx			= "helpex";
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
char const* Constants::kCLIStats			= "stats";
char const* Constants::kCLIStopSoar			= "stop-soar";
char const* Constants::kCLITime				= "time";
char const* Constants::kCLIWatch			= "watch";

//  ____                _              _
// / ___|___  _ __  ___| |_ __ _ _ __ | |_ ___
//| |   / _ \| '_ \/ __| __/ _` | '_ \| __/ __|
//| |__| (_) | | | \__ \ || (_| | | | | |_\__ \
// \____\___/|_| |_|___/\__\__,_|_| |_|\__|___/
//
Constants::Constants() {
	ifstream usageFile("usage.txt");

	m_UsageFileAvailable = usageFile ? true : false;

	if (m_UsageFileAvailable) {
		LoadUsage(usageFile);
	}
	usageFile.close();
}

//  ____      _    ____                                          _ _     _     _
// / ___| ___| |_ / ___|___  _ __ ___  _ __ ___   __ _ _ __   __| | |   (_)___| |_
//| |  _ / _ \ __| |   / _ \| '_ ` _ \| '_ ` _ \ / _` | '_ \ / _` | |   | / __| __|
//| |_| |  __/ |_| |__| (_) | | | | | | | | | | | (_| | | | | (_| | |___| \__ \ |_
// \____|\___|\__|\____\___/|_| |_| |_|_| |_| |_|\__,_|_| |_|\__,_|_____|_|___/\__|
//
std::list<std::string> Constants::GetCommandList() {
	UsageMapConstIter iter = m_UsageMap.begin();
	std::list<std::string> result;
	while (iter != m_UsageMap.end()) {
		result.push_back(iter->first);
		++iter;
	}
	return result;
}

//  ____      _   _   _                      _____
// / ___| ___| |_| | | |___  __ _  __ _  ___|  ___|__  _ __
//| |  _ / _ \ __| | | / __|/ _` |/ _` |/ _ \ |_ / _ \| '__|
//| |_| |  __/ |_| |_| \__ \ (_| | (_| |  __/  _| (_) | |
// \____|\___|\__|\___/|___/\__,_|\__, |\___|_|  \___/|_|
//                                |___/
bool Constants::GetUsageFor(const std::string& command, std::string& output) {
	if (m_UsageFileAvailable) {
		if (m_UsageMap.find(command) == m_UsageMap.end()) {
			return false;
		}
		output = m_UsageMap[command];
		return true;
	}
	return false;
}

bool Constants::GetExtendedUsageFor(const std::string& command, std::string& output) {
	if (m_UsageFileAvailable) {
		if (m_ExtendedUsageMap.find(command) == m_ExtendedUsageMap.end()) {
			return false;
		}
		output = m_ExtendedUsageMap[command];
		return true;
	}
	return false;
}

// ___     _   _                      _____ _ _         _             _ _       _     _
//|_ _|___| | | |___  __ _  __ _  ___|  ___(_) | ___   / \__   ____ _(_) | __ _| |__ | | ___
// | |/ __| | | / __|/ _` |/ _` |/ _ \ |_  | | |/ _ \ / _ \ \ / / _` | | |/ _` | '_ \| |/ _ \
// | |\__ \ |_| \__ \ (_| | (_| |  __/  _| | | |  __// ___ \ V / (_| | | | (_| | |_) | |  __/
//|___|___/\___/|___/\__,_|\__, |\___|_|   |_|_|\___/_/   \_\_/ \__,_|_|_|\__,_|_.__/|_|\___|
//                         |___/
bool Constants::IsUsageFileAvailable() {
	return m_UsageFileAvailable;
}

// _                    _ _   _
//| |    ___   __ _  __| | | | |___  __ _  __ _  ___
//| |   / _ \ / _` |/ _` | | | / __|/ _` |/ _` |/ _ \
//| |__| (_) | (_| | (_| | |_| \__ \ (_| | (_| |  __/
//|_____\___/ \__,_|\__,_|\___/|___/\__,_|\__, |\___|
//                                        |___/
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

//  ____      _   _   _
// / ___| ___| |_| | | |___  __ _  __ _  ___
//| |  _ / _ \ __| | | / __|/ _` |/ _` |/ _ \
//| |_| |  __/ |_| |_| \__ \ (_| | (_| |  __/
// \____|\___|\__|\___/|___/\__,_|\__, |\___|
//                                |___/
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
