#include "cli_CommandLineInterface.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseHelp(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	unused(pAgent);

	if (argv.size() > 2) {
		return HandleSyntaxError(Constants::kCLIHelp, Constants::kCLITooManyArgs);
	}

	if (argv.size() == 2) {
		return DoHelp(&(argv[1]));
	}
	return DoHelp();
}

bool CommandLineInterface::DoHelp(std::string* pCommand) {
	std::string output;

	if (!m_Constants.IsUsageFileAvailable()) {
		return HandleError(Constants::kCLINoUsageFile);
	}

	if (pCommand) {
		if (!m_Constants.GetUsageFor(*pCommand, output)) {
			return HandleError("Help for command '" + *pCommand + "' not found.");
		}
		AppendToResult(output);
		return true;
	}
	AppendToResult("Help is available for the following commands:\n");
	std::list<std::string> commandList = m_Constants.GetCommandList();
	std::list<std::string>::const_iterator iter = commandList.begin();

	int i = 0;
	int tabs;
	while (iter != commandList.end()) {
		AppendToResult(*iter);
		if (m_CommandMap.find(*iter) == m_CommandMap.end()) {
			AppendToResult('*');
		} else {
			AppendToResult(' ');
		}
		tabs = (40 - (*iter).length() - 2) / 8; 
		if (i % 2) {
			AppendToResult("\n");
		} else {
			do {
				AppendToResult('\t');
			} while (--tabs > 0);
		}
		++iter;
		++i;
	}
	if (i % 2) {
		AppendToResult('\n');
	}
	AppendToResult("Type 'help' followed by the command name for help on a specific command.\n");
	AppendToResult("A Star (*) indicates the command is not yet implemented.");
	return true;
}

