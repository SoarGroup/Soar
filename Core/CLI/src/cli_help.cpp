/////////////////////////////////////////////////////////////////
// help command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2010
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "cli_CommandLineInterface.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoHelp() {

	m_Result << "Please view help online: http://code.google.com/p/soar/wiki/CommandLineInterface";
	return true;
}
