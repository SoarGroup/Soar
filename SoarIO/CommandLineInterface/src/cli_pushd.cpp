#include "cli_CommandLineInterface.h"

using namespace cli;

bool CommandLineInterface::ParsePushD(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	unused(pAgent);

	// Only takes one argument, the directory to change into
	if (argv.size() != 2) {
		return HandleSyntaxError(Constants::kCLIPushD, Constants::kCLITooManyArgs);
	}
	return DoPushD(argv[1]);
}

bool CommandLineInterface::DoPushD(std::string& directory) {
	
	// Target directory required, checked in DoCD call.

	// Save the current (soon to be old) directory
	std::string oldDirectory;
	if (!GetCurrentWorkingDirectory(oldDirectory)) {
		// Error message added in function
		return false;
	}

	// Change to the new directory.
	if (!DoCD(&directory)) {
		return false;
	}

	// If we're sourcing, this will be non-negative
	if (m_SourceDirDepth >= 0) {
		// And if it is, increment it for each dir placed on the stack
		++m_SourceDirDepth;
	}

	// Directory change successful, store old directory and move on
	m_DirectoryStack.push(oldDirectory);
	return true;
}

