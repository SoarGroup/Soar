#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include <iostream>
#include <fstream>

#include "cli_Constants.h"
#include "cli_Aliases.h"

using namespace cli;
using namespace std;

bool CommandLineInterface::ParseHome(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	unused(pAgent);

	// Only takes one optional argument, the directory to change home to
	if (argv.size() > 2) return SetError(CLIError::kTooManyArgs);
	if (argv.size() > 1) return DoHome(&(argv[1]));
	return DoHome();
}

bool CommandLineInterface::DoHome(const std::string* pDirectory) {

	if (pDirectory) {
		// Change to the passed directory if any to make sure it is valid
		// also need usage file from that directory
		if (!DoCD(pDirectory)) return false;
	}
	// Set Home to current directory
	if (!GetCurrentWorkingDirectory(m_HomeDirectory)) return false;
	return true;
}

