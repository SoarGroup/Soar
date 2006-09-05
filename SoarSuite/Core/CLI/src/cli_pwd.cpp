/////////////////////////////////////////////////////////////////
// pwd command file.
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
#include "sml_Names.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParsePWD(gSKI::Agent* pAgent, std::vector<std::string>& argv) {
	unused(pAgent);

	// No arguments to print working directory
	if (argv.size() != 1) {
		SetErrorDetail("pwd takes no arguments.");
		return SetError(CLIError::kTooManyArgs);
	}
	return DoPWD();
}

bool CommandLineInterface::DoPWD() {

	std::string directory;
	bool ret = GetCurrentWorkingDirectory(directory);

	if (directory.size()) {
		if (m_RawOutput) {
			m_Result << directory;
		} else {
			AppendArgTagFast(sml_Names::kParamDirectory, sml_Names::kTypeString, directory.c_str());
		}
	}

	return ret;
}

