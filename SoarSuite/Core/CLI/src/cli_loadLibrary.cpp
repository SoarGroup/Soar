/////////////////////////////////////////////////////////////////
// load-library command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com, Bob Marinier, rmarinie@umich.edu
// Date  : 2007
//
/////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include <portability.h>

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"

#include "sml_Names.h"
#include "sml_KernelSML.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseLoadLibrary(gSKI::Agent* pAgent, std::vector<std::string>& argv) {
	unused(pAgent);

	// command-name library-name [library-args ...]
	// TODO: library-args not implemented yet, currently an error

	if (argv.size() < 2) {
		SetErrorDetail("Library name expected.");
		return SetError(CLIError::kTooFewArgs);
	} else if (argv.size() > 2) {
		// TODO: this shouldn't be an error in the future (should be library args)
		SetErrorDetail("Library arguments not implemented yet.");
		return SetError(CLIError::kTooManyArgs);
	}

	return DoLoadLibrary(argv[1]);
}

bool CommandLineInterface::DoLoadLibrary(const std::string& argv) {

	this->m_pKernelSML->FireLoadLibraryEvent(argv.c_str());

	return true;
}

