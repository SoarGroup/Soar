/////////////////////////////////////////////////////////////////
// command-to-file command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2006
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "cli_CommandLineInterface.h"

#include <fstream>

#include <assert.h>

#include "cli_Commands.h"
#include "cli_CommandData.h"

#include "sml_Names.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseCommandToFile(std::vector<std::string>& argv) {
	// Not going to use normal option parsing in this case because I do not want to disturb the other command on the line
	if (argv.size() < 3) {
		return SetError(kTooFewArgs);
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
			return SetError(kUnrecognizedOption);
		}

		if (append) {
			mode = LOG_NEWAPPEND;

			// Index of command in argv:  command-to-file -a filename command ...
			if (argv.size() < 4) {
				return SetError(kTooFewArgs);
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

	std::string oldResult(m_Result.str());
	m_Result.str("");
	std::string wtf(m_Result.str());

	// Fire off command
	bool ret = HandleCommand(newArgv);

	std::string ctfOutput;
	if (ret)
	{
		ctfOutput.assign(m_Result.str());
		m_Result.str("");
	} 
	else 
	{
		ctfOutput = GenerateErrorString();
	}

	if (!DoCLog(mode, &filename, 0, true))
	{
		return false;
	}

	if (!DoCLog(LOG_ADD, 0, &ctfOutput, true))
	{
		return false;
	}

	if (!DoCLog(LOG_CLOSE, 0, 0, true))
	{
		return false;
	}

	m_Result << oldResult;
	return ret;
}
