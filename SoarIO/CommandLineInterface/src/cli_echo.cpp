#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Constants.h"
#include "cli_GetOpt.h"

using namespace cli;

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
				return HandleSyntaxError(Constants::kCLIEcho, Constants::kCLIUnrecognizedOption);
			default:
				return HandleGetOptError((char)option);
		}
	}

	if (noNewLine) {
		argv.erase(++(argv.begin()));
	}

	return DoEcho(argv, noNewLine);
}

bool CommandLineInterface::DoEcho(std::vector<std::string>& argv, bool noNewLine) {

	// Concatenate arguments (spaces between arguments are lost unless enclosed in quotes)
	for (unsigned i = 1; i < argv.size(); ++i) {
		AppendToResult(argv[i]);
		AppendToResult(' ');
	}

	if (!noNewLine) AppendToResult('\n');
	return true;
}

