/////////////////////////////////////////////////////////////////
// popd command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include <portability.h>

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"

using namespace cli;

bool CommandLineInterface::ParsePopD(gSKI::Agent* pAgent, std::vector<std::string>& argv) {
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

	// If we're sourcing, this will be non-negative
	if (m_SourceDirDepth >= 0) --m_SourceDirDepth; // And if it is, decrement it for each dir removed from the stack

	// Pop the directory stack
	m_DirectoryStack.pop();
	return true;
}

