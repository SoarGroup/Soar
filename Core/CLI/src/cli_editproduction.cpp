/////////////////////////////////////////////////////////////////
// edit-production command file.
//
// Author: Douglas Pearson
// Date  : 2005
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "sml_Utils.h"
#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"
#include "cli_CLIError.h"

#include "sml_KernelSML.h"

using namespace cli;

bool CommandLineInterface::ParseEditProduction(std::vector<std::string>& argv) {
	if (argv.size() != 2) {
		SetErrorDetail("Need to include the name of the production to edit.");
		return SetError(CLIError::kTooFewArgs);
	}

	return DoEditProduction(argv[1]);
}

bool CommandLineInterface::DoEditProduction(std::string production) {
	m_pKernelSML->FireEditProductionEvent(production.c_str()) ;
	return true;
}

