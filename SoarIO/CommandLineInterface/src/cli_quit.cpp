#include "cli_CommandLineInterface.h"

using namespace cli;

bool CommandLineInterface::ParseQuit(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	unused(pAgent);

	// Quit needs no help
	argv.clear();
	return DoQuit();
}

bool CommandLineInterface::DoQuit() {
	// Simply flip the quit flag
	m_QuitCalled = true; 

	// Toodles!
	AppendToResult("Goodbye.");
	return true;
}

