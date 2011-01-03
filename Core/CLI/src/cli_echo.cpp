/////////////////////////////////////////////////////////////////
// echo command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "cli_CommandLineInterface.h"
#include "sml_Utils.h"
#include "sml_Names.h"

using namespace cli;
using namespace sml;

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
		switch (message[pos + 1]) {
			case '\\':
				message.erase(pos, 1);
				break;
			case 'b': // backspace
				message.erase(pos, 1);
				message[pos] = '\b';
				break;
			case 'c': // supress trailing newline
				message.erase(pos, 1);
				message.erase(pos, 1);
				--pos;
				echoNewline = false;
				break;
			case 'f': // form feed
				message.erase(pos, 1);
				message[pos] = '\f';
				break;
			case 'n': // newline
				message.erase(pos, 1);
				message[pos] = '\n';
				break;
			case 'r': // carriage return
				message.erase(pos, 1);
				message[pos] = '\r';
				break;
			case 't': // horizontal tab
				message.erase(pos, 1);
				message[pos] = '\t';
				break;
			case 'v': // vertical tab
				message.erase(pos, 1);
				message[pos] = '\v';
				break;
			default: // ignore it
				break;
		} 
	}

	// Add newline if applicable
	if (echoNewline) {
		message += '\n';
	}

	if (m_RawOutput) {
		m_Result << message;
	} else {
		AppendArgTagFast(sml_Names::kParamMessage, sml_Names::kTypeString, message);
	}
	return true;
}

