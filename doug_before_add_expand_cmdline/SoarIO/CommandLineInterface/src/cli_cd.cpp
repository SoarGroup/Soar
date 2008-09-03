#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Constants.h"

using namespace cli;

#ifdef WIN32
#include <direct.h>
#endif // WIN32

bool CommandLineInterface::ParseCD(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	unused(pAgent);

	// Only takes one optional argument, the directory to change into
	if (argv.size() > 2) {
		return m_Error.SetError(CLIError::kTooManyArgs);
	}
	if (argv.size() > 1) {
		return DoCD(&(argv[1]));
	}
	return DoCD();
}

bool CommandLineInterface::DoCD(std::string* pDirectory) {

	// If cd is typed by itself, return to original (home) directory
	if (!pDirectory) {

		// Home dir set in constructor
		if (chdir(m_HomeDirectory.c_str())) {
			return m_Error.SetError(CLIError::kchdirFail);
		}
		return true;
	}

	// Chop of quotes if they are there, chdir doesn't like them
	if ((pDirectory->length() > 2) && ((*pDirectory)[0] == '\"') && ((*pDirectory)[pDirectory->length() - 1] == '\"')) {
		*pDirectory = pDirectory->substr(1, pDirectory->length() - 2);
	}

	// Change to passed directory
	if (chdir(pDirectory->c_str())) {
		return m_Error.SetError(CLIError::kchdirFail);
	}
	return true;
}

