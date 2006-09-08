/////////////////////////////////////////////////////////////////
// command-to-file command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2006
//
/////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include <portability.h>

#include "cli_CommandLineInterface.h"

#include <fstream>

#include <assert.h>

#include "cli_Commands.h"
#include "cli_CommandData.h"

#include "sml_Names.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseCommandToFile(gSKI::Agent* pAgent, std::vector<std::string>& argv) {
	// Not going to use normal option parsing in this case because I do not want to disturb the other command on the line
	if (argv.size() < 3) {
		return SetError(CLIError::kTooFewArgs);
	}

	// Index of command in argv:  command-to-file filename command ...
	// Unless append option is present, which is handled later.
	int startOfCommand = 2;
	eLogMode mode = LOG_NEW;
	std::string filename = argv[1];

	// Parse out option.
	for (int i = 1; i < 3; ++i) {
		bool append = false;
		bool unrecognized = false;
		std::string arg = argv[i];
		if (arg[0] == '-') {
			if (arg[1] == 'a') {
				append = true;
			} else if (arg[1] == '-') {
				if (arg[2] == 'a') {
					append = true;
				} else {
					unrecognized = true;
				}
			} else {
				unrecognized = true;
			}
		}
		
		if (unrecognized) {
			SetErrorDetail(arg);
			return SetError(CLIError::kUnrecognizedOption);
		}

		if (append) {
			mode = LOG_NEWAPPEND;

			// Index of command in argv:  command-to-file -a filename command ...
			if (argv.size() < 4) {
				return SetError(CLIError::kTooFewArgs);
			}
			startOfCommand = 3;

			// Re-set filename if necessary
			if (i == 1) {
				filename = argv[2];
			}
			break;
		}
	}

	// Restructure argv
	std::vector<std::string> newArgv;
	for (std::vector<int>::size_type i = startOfCommand; i < argv.size(); ++i) {
		newArgv.push_back(argv[i]);
	}

	// Open log
	if (!DoCLog(pAgent, mode, &filename)) {
		return false;
	}

	// Set flag to close log after output:
	m_CloseLogAfterOutput = true;

	// Fire off command
	return DoCommandInternal(pAgent, newArgv);
}
