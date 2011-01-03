/////////////////////////////////////////////////////////////////
// soarnews command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

// SEE INIT_SOAR.CPP:1531 for old soarnews

#include <portability.h>

#include "sml_Utils.h"
#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"

#include "sml_Names.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoSoarNews() {

	m_Result << "This is Soar ";
	
	// Make DoVersion print its result to the result string
	bool rawOutputSave = m_RawOutput;
	m_RawOutput = true;
	DoVersion();
	m_RawOutput = rawOutputSave;

	m_Result << ".\n";
	m_Result << "Please visit the Soar home page at\n\thttp://sitemaker.umich.edu/soar/\n";
	m_Result << "Please see the announce.txt file included in the Documentation folder with this release for more information.";

	return true;
}

