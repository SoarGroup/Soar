/////////////////////////////////////////////////////////////////
// replay-input command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2008
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "sml_Utils.h"
#include "cli_CommandLineInterface.h"
#include "cli_CLIError.h"

#include "cli_Commands.h"
#include "sml_KernelSML.h"


using namespace cli;
using namespace sml;

// replay-input --open pathname 
// replay-input [--query]
// replay-input --close
bool CommandLineInterface::ParseReplayInput(std::vector<std::string>& argv) {
	Options optionsData[] = {
		{'c', "close", OPTARG_NONE},
		{'o', "open", OPTARG_REQUIRED},
		{'q', "query", OPTARG_NONE},
		{0, 0, OPTARG_NONE}
	};

	eReplayInputMode mode = REPLAY_INPUT_QUERY;
	std::string pathname;

	for (;;) {
		if (!ProcessOptions(argv, optionsData)) return false;
		if (m_Option == -1) break;

		switch (m_Option) {
			case 'c':
				mode = REPLAY_INPUT_CLOSE;
				break;
			case 'o':
				mode = REPLAY_INPUT_OPEN;
				pathname = m_OptionArgument;
				break;
			case 'q':
				mode = REPLAY_INPUT_QUERY;
				break;
			default:
				return SetError(CLIError::kGetOptError);
		}
	}

	return DoReplayInput(mode, mode == REPLAY_INPUT_OPEN ? &pathname : 0);
}

bool CommandLineInterface::DoReplayInput(eReplayInputMode mode, std::string* pathname) {
	switch (mode) {
		case REPLAY_INPUT_CLOSE:
			// TODO: close code
			return SetError(CLIError::kNotImplemented);
			break;

		case REPLAY_INPUT_OPEN:
			if (!pathname) return SetError(CLIError::kMissingFilenameArg);
			if (!pathname->size()) return SetError(CLIError::kMissingFilenameArg);

			StripQuotes(*pathname);

			// TODO: open code
			return SetError(CLIError::kNotImplemented);
			break;

		case REPLAY_INPUT_QUERY:
			// TODO: query code
			return SetError(CLIError::kNotImplemented);
			break;
	}

	return true;
}

