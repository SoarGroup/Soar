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
	// Flip the quit flag
	m_QuitCalled = true; 

	// Stop soar
	DoStopSoar(0, false, std::string("Quit called."));

	// Stop log
	if (m_pLogFile) {
		(*m_pLogFile) << "Log file closed due to shutdown." << std::endl;
		delete m_pLogFile;
	}

	AppendToResult("Goodbye.");
	return true;
}

