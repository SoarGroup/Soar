/////////////////////////////////////////////////////////////////
// unalias command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2006
//
/////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include <portability.h>

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"

#include "sml_Names.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseUnalias(gSKI::Agent* pAgent, std::vector<std::string>& argv) {
	unused(pAgent);

	// Need exactly one argument
	if (argv.size() < 2) {
		SetErrorDetail("Need exactly one command to unalias. See also: alias");
		return SetError(CLIError::kTooFewArgs);
	} else if (argv.size() > 2) {
		SetErrorDetail("Need exactly one command to unalias. See also: alias");
		return SetError(CLIError::kTooManyArgs);
	}
	return DoAlias(&(argv[1]));
}
