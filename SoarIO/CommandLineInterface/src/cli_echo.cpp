#include "cli_CommandLineInterface.h"

#include "cli_Constants.h"

using namespace cli;

bool CommandLineInterface::ParseEcho(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	unused(pAgent);

	if (argv.size() != 2) {
		return HandleSyntaxError(Constants::kCLIEcho);
	}

	return DoEcho(argv);
}

bool CommandLineInterface::DoEcho(std::vector<std::string>& argv) {

	// Concatenate arguments (spaces between arguments are lost unless enclosed in quotes)
	for (unsigned i = 1; i < argv.size(); ++i) {
		AppendToResult(argv[i]);
		AppendToResult(' ');
	}

	// Chop off that last space we just added in the loop
	m_Result = m_Result.substr(0, m_Result.length() - 1);
	return true;
}

