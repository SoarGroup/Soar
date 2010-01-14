/////////////////////////////////////////////////////////////////
// quit command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include <iostream>
#include <fstream>

#include "sml_Utils.h"
#include "cli_CommandLineInterface.h"

#include "sml_Names.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseQuit(std::vector<std::string>&) {
	// Quit needs no help
	return DoQuit();
}

bool CommandLineInterface::DoQuit() {
	// Stop soar
	DoStopSoar(false, 0);

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

