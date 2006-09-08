/////////////////////////////////////////////////////////////////
// echo command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include <portability.h>

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"

#include "sml_Names.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseEcho(gSKI::Agent* pAgent, std::vector<std::string>& argv) {
	unused(pAgent);
	Options optionsData[] = {
		{'n', "no-newline", 0},
		{0, 0, 0}
	};

	bool echoNewline(true);

	for (;;) {
		if (!ProcessOptions(argv, optionsData)) return false;
		if (m_Option == -1) break;

		switch (m_Option) {
			case 'n':
				echoNewline = false;
				break;
			default:
				return SetError(CLIError::kGetOptError);
		}
	}

	std::vector<std::string> newArgv;
	newArgv.push_back(argv[0]);
	unsigned optind = m_Argument - m_NonOptionArguments;
	for (unsigned i = optind; i < argv.size(); ++i) {
		newArgv.push_back(argv[i]);
	}
	return DoEcho(newArgv, echoNewline);
}

bool CommandLineInterface::DoEcho(const std::vector<std::string>& argv, bool echoNewline) {

	std::string message;

	// Concatenate arguments (spaces between arguments are lost unless enclosed in quotes)
	for (unsigned i = 1; i < argv.size(); ++i) {
		message += argv[i];
		message += ' ';
	}

	// remove trailing space
	message = message.substr(0, message.length() - 1);

	// Convert backslash characters
	for(std::string::size_type pos = 0; (pos = message.find('\\', pos)) != std::string::npos; ++pos) {
		if (message.size() <= pos + 1) break;
		// Found a backslash with a character after it, remove it
		message.erase(pos, 1);
		switch (message[pos]) {
			case '\\':
				break;
			case 'b': // backspace
				message[pos] = '\b';
				break;
			case 'c': // supress trailing newline
				message.erase(pos, 1);
				--pos;
				echoNewline = false;
				break;
			case 'f': // form feed
				message[pos] = '\f';
				break;
			case 'n': // newline
				message[pos] = '\n';
				break;
			case 'r': // carriage return
				message[pos] = '\r';
				break;
			case 't': // horizontal tab
				message[pos] = '\t';
				break;
			case 'v': // vertical tab
				message[pos] = '\v';
				break;
			default: // error
				return SetError(CLIError::kInvalidBackslashEscapeCharacter);
		} 
	}

	// Add newline if applicable
	if (echoNewline) {
		message += '\n';
	}

	if (m_RawOutput) {
		m_Result << message;
	} else {
		AppendArgTagFast(sml_Names::kParamMessage, sml_Names::kTypeString, message.c_str());
	}
	return true;
}

