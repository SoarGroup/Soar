#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

using namespace cli;

bool CommandLineInterface::ParseQuit(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	unused(pAgent);

	// Quit needs no help
	argv.clear();
	return DoQuit();
}

bool CommandLineInterface::DoQuit() {
	// Stop any running thread
	if (m_pRunForever) {
		m_pRunForever->Stop(true);
		delete m_pRunForever;
		m_pRunForever = 0;
	}

	// Simply flip the quit flag
	m_QuitCalled = true; 

	// Toodles!
	AppendToResult("Goodbye.");
	return true;
}

