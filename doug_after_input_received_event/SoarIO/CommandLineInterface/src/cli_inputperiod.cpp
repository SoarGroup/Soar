/////////////////////////////////////////////////////////////////
// input-period command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2005
//
/////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Constants.h"

#include "sml_Names.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseInputPeriod(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	unused(pAgent);
	unused(argv);
	return SetError(CLIError::kNotImplemented);
}

bool CommandLineInterface::DoInputPeriod(gSKI::IAgent* pAgent) {
	unused(pAgent);
	return SetError(CLIError::kNotImplemented);
}

