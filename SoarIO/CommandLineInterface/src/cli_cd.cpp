#include "cli_CommandLineInterface.h"

using namespace cli;

#ifdef WIN32
#include <direct.h>
#endif // WIN32

// ____                      ____ ____
//|  _ \ __ _ _ __ ___  ___ / ___|  _ \
//| |_) / _` | '__/ __|/ _ \ |   | | | |
//|  __/ (_| | |  \__ \  __/ |___| |_| |
//|_|   \__,_|_|  |___/\___|\____|____/
//
bool CommandLineInterface::ParseCD(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	unused(pAgent);

	// Only takes one optional argument, the directory to change into
	if (argv.size() > 2) {
		return HandleSyntaxError(Constants::kCLICD, Constants::kCLITooManyArgs);
	}
	if (argv.size() > 1) {
		return DoCD(&(argv[1]));
	}
	return DoCD();
}

// ____         ____ ____
//|  _ \  ___  / ___|  _ \
//| | | |/ _ \| |   | | | |
//| |_| | (_) | |___| |_| |
//|____/ \___/ \____|____/
//
bool CommandLineInterface::DoCD(std::string* pDirectory) {

	// If cd is typed by itself, return to original (home) directory
	if (!pDirectory) {

		// Home dir set in constructor
		if (chdir(m_HomeDirectory.c_str())) {
			return HandleError("Could not change to home directory: " + m_HomeDirectory);
		}
		return true;
	}

	// Chop of quotes if they are there, chdir doesn't like them
	if ((pDirectory->length() > 2) && ((*pDirectory)[0] == '\"') && ((*pDirectory)[pDirectory->length() - 1] == '\"')) {
		*pDirectory = pDirectory->substr(1, pDirectory->length() - 2);
	}

	// Change to passed directory
	if (chdir(pDirectory->c_str())) {
		return HandleError("Could not change to directory: " + *pDirectory);
	}
	return true;
}

