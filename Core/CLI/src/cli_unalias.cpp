/////////////////////////////////////////////////////////////////
// unalias command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2006
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "sml_Utils.h"
#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"

#include "sml_Names.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseUnalias(std::vector<std::string>& argv) {
	// Need exactly one argument
	if (argv.size() < 2) {
		SetErrorDetail("Need exactly one command to unalias. See also: alias");
		return SetError(kTooFewArgs);
	} else if (argv.size() > 2) {
		SetErrorDetail("Need exactly one command to unalias. See also: alias");
		return SetError(kTooManyArgs);
	}
	return DoAlias(&(argv[1]));
}
