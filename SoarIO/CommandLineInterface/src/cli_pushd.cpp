#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Constants.h"

using namespace cli;

bool CommandLineInterface::ParsePushD(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	unused(pAgent);

	// Only takes one argument, the directory to change into
	if (argv.size() != 2) {
		return SetError(CLIError::kTooManyArgs);
	}
	return DoPushD(argv[1]);
}

bool CommandLineInterface::DoPushD(std::string& directory) {
	
	// Target directory required, checked in DoCD call.

	// Save the current (soon to be old) directory
	std::string oldDirectory;
	if (!GetCurrentWorkingDirectory(oldDirectory)) return false;// Error message handled in function

	// Change to the new directory.
	if (!DoCD(&directory)) return false;// Error message handled in function

	// If we're sourcing, this will be non-negative
	if (m_SourceDirDepth >= 0) ++m_SourceDirDepth;// And if it is, increment it for each dir placed on the stack

	// Directory change successful, store old directory and move on
	m_DirectoryStack.push(oldDirectory);
	return true;
}

