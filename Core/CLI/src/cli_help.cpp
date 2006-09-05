/////////////////////////////////////////////////////////////////
// help command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"

#include <iostream>
#include <fstream>

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseHelp(gSKI::Agent* pAgent, std::vector<std::string>& argv) {
	unused(pAgent);

	if (argv.size() > 2) {
		SetErrorDetail("Pass only the command name you would like help with.");
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
		helpFile = m_LibraryDirectory + "/CLIHelp/command-names";
	} else {
		// check for aliases
		if (m_Aliases.IsAlias(*pCommand)) {
			m_Result << *pCommand << " is an alias.  Type 'alias " << *pCommand 
				<< "' to find out what command it is an alias for and look up help on that command.";
			return true;
		}

		helpFile = m_LibraryDirectory + "/CLIHelp/" + *pCommand;
	}

	if (!GetHelpString(helpFile)) return false;
	return true;
}

bool CommandLineInterface::GetHelpString(const std::string& helpFile) {

	std::ifstream helpFileStream(helpFile.c_str());
	if (!helpFileStream) {
		SetErrorDetail("Error opening help file: " + helpFile);
		return SetError(CLIError::kNoHelpFile);
	}

	char ch;
	while(helpFileStream.get(ch)) m_Result.put(ch);

	if (!helpFileStream.eof() || !m_Result) return SetError(CLIError::kHelpFileError);
	return true;
}

