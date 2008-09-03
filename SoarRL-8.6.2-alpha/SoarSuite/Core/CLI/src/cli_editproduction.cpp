/////////////////////////////////////////////////////////////////
// edit-production command file.
//
// Author: Douglas Pearson
// Date  : 2005
//
/////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"

#include "IgSKI_Kernel.h"
#include "sml_KernelSML.h"

using namespace cli;

bool CommandLineInterface::ParseEditProduction(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	unused(pAgent);
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

