/////////////////////////////////////////////////////////////////
// start-system command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Constants.h"

#include "IgSKI_Kernel.h"
#include "sml_KernelSML.h"

using namespace cli;

bool CommandLineInterface::ParseStartSystem(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	unused(pAgent);
	if (argv.size() != 2) {
		SetErrorDetail("Need to include the name of the production to edit.");
		return SetError(CLIError::kTooFewArgs);
	}

	return DoStartSystem(argv[1]);
}

bool CommandLineInterface::DoStartSystem(std::string production) {
	m_pKernelSML->FireEditProductionEvent(production.c_str()) ;
	return true;
}

