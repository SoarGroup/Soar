#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Constants.h"

using namespace cli;

bool CommandLineInterface::ParsePopD(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	unused(pAgent);

	// No arguments
	if (argv.size() != 1) {
		return SetError(CLIError::kTooManyArgs);
	}
	return DoPopD();
}

bool CommandLineInterface::DoPopD() {

	// There must be a directory on the stack to pop
	if (m_DirectoryStack.empty()) return SetError(CLIError::kDirectoryStackEmpty);

	// Change to the directory
	if (!DoCD(&(m_DirectoryStack.top()))) return false;	// error handled in DoCD

	// Pop the directory stack
	m_DirectoryStack.pop();
	return true;
}

