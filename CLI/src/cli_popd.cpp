/////////////////////////////////////////////////////////////////
// popd command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "sml_Utils.h"
#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"

using namespace cli;

bool CommandLineInterface::ParsePopD(std::vector<std::string>& argv) {
	// No arguments
	if (argv.size() != 1) {
		return SetError(kTooManyArgs);
	}
	return DoPopD();
}

bool CommandLineInterface::DoPopD() {

	// There must be a directory on the stack to pop
	if (m_DirectoryStack.empty()) return SetError(kDirectoryStackEmpty);

	// Change to the directory
	if (!DoCD(&(m_DirectoryStack.top()))) return false;	// error handled in DoCD

	// Pop the directory stack
	m_DirectoryStack.pop();
	return true;
}

