#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Constants.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseHelp(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	unused(pAgent);

	if (argv.size() > 2) {
		return SetError(CLIError::kTooManyArgs);
	}

	if (argv.size() == 2) {
		return DoHelp(&(argv[1]));
	}
	return DoHelp();
}

bool CommandLineInterface::DoHelp(std::string* pCommand) {
	unused(pCommand);
	AppendToResult("Help deprecated until release, please see\n\thttp://winter.eecs.umich.edu/soarwiki");
	return SetError(CLIError::kNoUsageFile);
	//std::string output;

	//if (!m_pConstants->IsUsageFileAvailable()) return SetError(CLIError::kNoUsageFile);

	//if (pCommand) {
	//	if (!m_pConstants->GetUsageFor(*pCommand, output)) return SetError(CLIError::kNoUsageInfo);
	//	AppendToResult(output);
	//	return true;
	//}

	//AppendToResult("Help is available for the following commands:\n");
	//std::list<std::string> commandList = m_pConstants->GetCommandList();
	//std::list<std::string>::const_iterator iter = commandList.begin();

	//int i = 0;
	//int tabs;
	//while (iter != commandList.end()) {
	//	AppendToResult(*iter);
	//	if (m_CommandMap.find(*iter) == m_CommandMap.end()) {
	//		AppendToResult('*');
	//	} else {
	//		AppendToResult(' ');
	//	}
	//	tabs = (40 - (*iter).length() - 2) / 8; 
	//	if (i % 2) {
	//		AppendToResult("\n");
	//	} else {
	//		do {
	//			AppendToResult('\t');
	//		} while (--tabs > 0);
	//	}
	//	++iter;
	//	++i;
	//}
	//if (i % 2) {
	//	AppendToResult('\n');
	//}
	//AppendToResult("Type 'help' followed by the command name for help on a specific command.\n");
	//AppendToResult("A Star (*) indicates the command is not yet implemented.");
	//return true;
}

