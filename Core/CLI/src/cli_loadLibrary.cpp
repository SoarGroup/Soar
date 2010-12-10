/////////////////////////////////////////////////////////////////
// load-library command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com, Bob Marinier, rmarinie@umich.edu
// Date  : 2007
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "sml_Utils.h"
#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"

#include "sml_Names.h"
#include "sml_KernelSML.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseLoadLibrary(std::vector<std::string>& argv) {
	// command-name library-name [library-args ...]

	if (argv.size() < 2) {
		SetErrorDetail("Library command expected.");
		return SetError(kTooFewArgs);
	}

	// strip the command name, combine the rest
	std::string libraryCommand(argv[1]);
	for(std::string::size_type i = 2; i < argv.size(); ++i) {
		libraryCommand += " ";
		libraryCommand += argv[i];
	}

	return DoLoadLibrary(libraryCommand);
}

bool CommandLineInterface::DoLoadLibrary(const std::string& libraryCommand) {

	std::string result = this->m_pKernelSML->FireLoadLibraryEvent(libraryCommand.c_str());

	// zero length is success
	if (result.size() == 0) {
		return true;
	}

	SetErrorDetail(result);
	return SetError(kLoadLibraryError);
}

