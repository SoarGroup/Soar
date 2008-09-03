#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Constants.h"

#include <iostream>
#include <fstream>

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

bool CommandLineInterface::DoHelp(const std::string* pCommand) {

	std::string helpFile;
	if (!pCommand || !pCommand->size()) {
		m_Result << "Help is available for the following commands:\n";
		helpFile = m_HomeDirectory + "/command-names";
	} else {
		// check for aliases
		if (m_Aliases.IsAlias(*pCommand)) {
			m_Result << *pCommand << " is an alias.  Type 'alias " << *pCommand 
				<< "' to find out what command it is an alias for and look up help on that command.";
			return true;
		}

		helpFile = m_HomeDirectory + "/help/" + *pCommand;
	}

	std::ifstream helpFileStream(helpFile.c_str());
	if (!helpFileStream) return SetError(CLIError::kNoHelpFile);

	char ch;
	while(helpFileStream.get(ch)) m_Result.put(ch);

	if (!helpFileStream.eof() || !m_Result) return SetError(CLIError::kHelpFileError);

	return true;
}

