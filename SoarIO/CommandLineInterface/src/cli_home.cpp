#include "cli_CommandLineInterface.h"

#include "cli_Constants.h"
#include "cli_Aliases.h"

using namespace cli;

bool CommandLineInterface::ParseHome(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	unused(pAgent);

	// Only takes one optional argument, the directory to change home to
	if (argv.size() > 2) {
		return HandleSyntaxError(Constants::kCLIHome, Constants::kCLITooManyArgs);
	}
	if (argv.size() > 1) {
		return DoHome(&(argv[1]));
	}
	return DoHome();
}

bool CommandLineInterface::DoHome(std::string* pDirectory) {

	if (pDirectory) {
		// Change to the passed directory if any to make sure it is valid
		// also need usage/aliases files from that directory
		if (!DoCD(pDirectory)) {
			return false;
		}

	}
	// Set Home to current directory
	if (!GetCurrentWorkingDirectory(m_HomeDirectory)) {
		return false;
	}

	// Reload aliases and constants
	if (m_pAliases) {
		delete m_pAliases;
		m_pAliases = 0;
	}

	if (m_pConstants) {
		delete m_pConstants;
		m_pConstants = 0;
	}

	m_pAliases = new Aliases();
	m_pConstants = new Constants();
	return true;
}

