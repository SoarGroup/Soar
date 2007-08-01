/////////////////////////////////////////////////////////////////
// predict command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com, 
//         Nate Derbinsky <nlderbin@umich.edu>
// Date  : 2007
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParsePredict(gSKI::Agent* pAgent, std::vector<std::string>& argv) {

	// No arguments to print working directory
	if (argv.size() != 1) {
		SetErrorDetail("predict takes no arguments.");
		return SetError(CLIError::kTooManyArgs);
	}
	return DoPredict(pAgent);
}

bool CommandLineInterface::DoPredict(gSKI::Agent* pAgent) {
	if (!RequireAgent(pAgent)) return false;

	return SetError(CLIError::kCommandNotImplemented);
}
