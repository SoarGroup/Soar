#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Constants.h"
#include "cli_GetOpt.h"

#include "sml_Names.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseEcho(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	unused(pAgent);

	static struct GetOpt::option longOptions[] = {
		{"nonewline",	0, 0, 'n'},
		{0, 0, 0, 0}
	};

	GetOpt::optind = 0;
	GetOpt::opterr = 0;

	bool noNewLine = false;

	for (;;) {
		int option = m_pGetOpt->GetOpt_Long(argv, "n", longOptions, 0);
		if (option == -1) break;

		switch (option) {
			case 'n':
				noNewLine = true;
				break;
			case '?':
				return m_Error.SetError(CLIError::kUnrecognizedOption);
			default:
				return m_Error.SetError(CLIError::kGetOptError);
		}
	}

	if (noNewLine) {
		argv.erase(++(argv.begin()));
	}

	return DoEcho(argv, noNewLine);
}

bool CommandLineInterface::DoEcho(std::vector<std::string>& argv, bool noNewLine) {

	std::string message;

	// Concatenate arguments (spaces between arguments are lost unless enclosed in quotes)
	for (unsigned i = 1; i < argv.size(); ++i) {
		message += argv[i];
		message += ' ';
	}

	message = message.substr(0, message.length() - 1);

	if (!noNewLine) message += '\n';

	if (m_RawOutput) {
		AppendToResult(message);
	} else {
		AppendArgTag(sml_Names::kParamMessage, sml_Names::kTypeString, message.c_str());
	}
	return true;
}

