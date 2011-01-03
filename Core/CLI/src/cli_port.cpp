/////////////////////////////////////////////////////////////////
// version command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "sml_Utils.h"
#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"
#include "sml_Names.h"
#include "sml_KernelSML.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoPort() {

	int port = m_pKernelSML->GetListenerPort();

	if (m_RawOutput) {
		m_Result << port;
	} else {
		std::string temp;
		AppendArgTag(sml_Names::kParamPort, sml_Names::kTypeInt, to_string(port, temp));
	}
	return true;
}

