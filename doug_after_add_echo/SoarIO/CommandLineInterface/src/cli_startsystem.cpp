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

using namespace cli;

bool CommandLineInterface::ParseStartSystem(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	unused(pAgent);
	unused(argv);
	return DoStartSystem();
}

bool CommandLineInterface::DoStartSystem() {
	m_pKernel->FireSystemStart();
	return true;
}

