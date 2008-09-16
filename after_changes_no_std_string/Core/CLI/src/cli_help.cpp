/////////////////////////////////////////////////////////////////
// help command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "sml_Utils.h"
#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"
#include "cli_CLIError.h"

#include <iostream>
#include <fstream>

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseHelp(std::vector<std::string>& argv) {
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
		m_Result << "Help is available for the following topics:\n";
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
	if (!helpFileStream) 
	{
		// this should exist, if not then we have a library location problem.
		std::string runHelpFile = m_LibraryDirectory + "/CLIHelp/run";
		std::ifstream runHelpFileStream( runHelpFile.c_str() );
		if ( !runHelpFileStream )
		{
			SetErrorDetail("The library location appears to be incorrect, please use set-library-location <path> where path leads to SoarLibrary.");
			return SetError(CLIError::kNoHelpFile);
		}

		SetErrorDetail("No help is available for " + helpFile);
		return SetError(CLIError::kNoHelpFile);
	}

	char ch;
	while(helpFileStream.get(ch)) m_Result.put(ch);

	if (!helpFileStream.eof() || !m_Result) return SetError(CLIError::kHelpFileError);
	return true;
}

