/////////////////////////////////////////////////////////////////
// cd command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"

using namespace cli;

#ifdef WIN32
#include <direct.h>
#define chdir _chdir
#endif // WIN32

bool CommandLineInterface::ParseCD(gSKI::Agent* pAgent, std::vector<std::string>& argv) {
	unused(pAgent);

	// Only takes one optional argument, the directory to change into
	if (argv.size() > 2) return SetError(CLIError::kTooManyArgs);

	if (argv.size() > 1) {
		return DoCD(&(argv[1]));
	}
	return DoCD();
}

bool CommandLineInterface::DoCD(const std::string* pDirectory) {

	// if directory 0, return SoarLibrary/bin
	if (!pDirectory) {
		std::string binDir = m_LibraryDirectory + "/bin";
		if (chdir(binDir.c_str())) {
			SetErrorDetail("Error changing to " + binDir);
			return SetError(CLIError::kchdirFail);
		}
		return true;
	}
   
    std::string dir = *pDirectory;
    StripQuotes(dir);

	// Change to directory
	if (chdir(dir.c_str())) {
		SetErrorDetail("Error changing to " + dir);
		return SetError(CLIError::kchdirFail);
	}
	return true;
}

