#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Constants.h"

#include "IgSKI_Kernel.h"

using namespace cli;

bool CommandLineInterface::ParseStartSimulation(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	unused(pAgent);
	unused(argv);
	return DoStartSimulation();
}

bool CommandLineInterface::DoStartSimulation() {
	m_pKernel->FireSystemStart();
	return true;
}

