#include "cli_CommandLineInterface.h"

using namespace cli;

// ____                     ____             ____
//|  _ \ __ _ _ __ ___  ___|  _ \ ___  _ __ |  _ \
//| |_) / _` | '__/ __|/ _ \ |_) / _ \| '_ \| | | |
//|  __/ (_| | |  \__ \  __/  __/ (_) | |_) | |_| |
//|_|   \__,_|_|  |___/\___|_|   \___/| .__/|____/
//                                    |_|
bool CommandLineInterface::ParsePopD(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	unused(pAgent);

	// No arguments
	if (argv.size() != 1) {
		return HandleSyntaxError(Constants::kCLIPopD, Constants::kCLITooManyArgs);
	}
	return DoPopD();
}

// ____        ____             ____
//|  _ \  ___ |  _ \ ___  _ __ |  _ \
//| | | |/ _ \| |_) / _ \| '_ \| | | |
//| |_| | (_) |  __/ (_) | |_) | |_| |
//|____/ \___/|_|   \___/| .__/|____/
//                       |_|
bool CommandLineInterface::DoPopD() {

	// There must be a directory on the stack to pop
	if (m_DirectoryStack.empty()) {
		return HandleError("Directory stack empty, no directory to change to.");
	}

	// Change to the directory
	if (!DoCD(&(m_DirectoryStack.top()))) {
		return false;
	}

	// If we're sourcing, this will be non-negative
	if (m_SourceDirDepth >= 0) {
		// And if it is, decrement it for each dir removed from the stack
		--m_SourceDirDepth;
	}

	// Pop the directory stack
	m_DirectoryStack.pop();
	return true;
}

