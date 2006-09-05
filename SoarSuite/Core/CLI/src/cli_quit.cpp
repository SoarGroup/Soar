/////////////////////////////////////////////////////////////////
// quit command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "sml_Names.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseQuit(gSKI::Agent* pAgent, std::vector<std::string>& argv) {
	unused(pAgent);
	unused(argv);

	// Quit needs no help
	return DoQuit();
}

bool CommandLineInterface::DoQuit() {
	// Stop soar
	DoStopSoar(0, false, 0);

	// Stop log
	if (m_pLogFile) {
		(*m_pLogFile) << "Log file closed due to shutdown." << std::endl;
		delete m_pLogFile;
		m_pLogFile = 0;
	}

	if (m_RawOutput) {
		m_Result << "Goodbye.";
	} else {
		AppendArgTagFast(sml_Names::kParamMessage, sml_Names::kTypeString, "Goodbye.");
	}
	return true;
}

