#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Constants.h"

#include "sml_Names.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseEcho(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	unused(pAgent);
	return DoEcho(argv);
}

bool CommandLineInterface::DoEcho(const std::vector<std::string>& argv) {

	std::string message;

	// Concatenate arguments (spaces between arguments are lost unless enclosed in quotes)
	for (unsigned i = 1; i < argv.size(); ++i) {
		message += argv[i];
		message += ' ';
	}

	// remove trailing space
	message = message.substr(0, message.length() - 1);

	if (m_RawOutput) {
		m_Result << message;
	} else {
		AppendArgTagFast(sml_Names::kParamMessage, sml_Names::kTypeString, message.c_str());
	}
	return true;
}

